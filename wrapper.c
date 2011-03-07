/*
 * The author of this work has dedicated it to the public by waiving all of his
 * or her rights to the work under copyright law and all related or neighboring
 * legal rights he or she had in the work, to the extent allowable by law.
 */
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <err.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "val.h"

static void usage(void);

#define FINDELEM(a, b) (((a) - (b)) / 2 + 1)

int
main(int argc, char **argv)
{
	char *iiarg = NULL, *iicmd = NULL, *sharg = NULL;
	size_t size;
	int ch, i, j, rv;
	pid_t iipid, shpid, sid;

	if (argc < 3)
		usage();
	while ((ch = getopt(argc, argv, "i:s:")) != -1)
		switch (ch) {
		case 'i':
			if (valsysarg(optarg))
				errx(EXIT_FAILURE, "sh(1) meta-characters and"
				    " quotation marks are not allowed in `ii"
				    " arg'.");
			iiarg = optarg;
			break;
		case 's':
			if (valsysarg(optarg))
				errx(EXIT_FAILURE, "sh(1) meta-characters and"
				    " quotation marks are not allowed in `sh"
				    " arg'.");
			sharg = optarg;
			break;
		default:
			usage();
			/* NOTREACHED */
		}

	for (i = 1; i < argc - 2; i++)
		if (!strcmp(argv[i], "--"))
			break;
	i++;
	/* argv[i] is now (hopefully) the first hostname. */
	if ((argc - i) & 1)
		usage();

	/* Validate the hostnames and ports. */
	for (j = i; j < argc; j++)
		if ((j - i) & 1) {
			if (valport(argv[j]))
				errx(EXIT_FAILURE, "Port number `%lu' is not"
				    " ok.", (unsigned long)FINDELEM(j, i));
		} else
			if (valhost(argv[j]))
				errx(EXIT_FAILURE, "Hostname `%lu' is not ok.",
				    (unsigned long)FINDELEM(j, i));

	if (iiarg)
		printf("ii arg: `%s'.\n", iiarg);
	if (sharg)
		printf("sh arg: `%s'.\n", sharg);
	for (j = i; j < argc - 1; j += 2)
		printf("Server[%lu]: %s:%d\n", (unsigned long)FINDELEM(j, i),
		    argv[j], atoi(argv[j + 1]));

	iipid = fork();
	if (iipid < 0)
		err(EXIT_FAILURE, "Failed to create child process.");
	else if (iipid > 0)
		exit(EXIT_SUCCESS);

	umask(0);

	sid = setsid();
	if (sid < 0)
		_exit(EXIT_FAILURE);

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	srandom(time(NULL));
	signal(SIGCHLD, SIG_IGN);

	for (;;) {
		if (sharg && shpid > 0)
			if (kill(shpid, SIGKILL) == -1 && errno != ESRCH)
				_exit(EXIT_FAILURE);

		sleep(LOOPSLEEP);

		/* http://eternallyconfuzzled.com/arts/jsw_art_rand.aspx */
		j = i + 1 + random() * 1.0 / (RAND_MAX + 1.0) * (argc - i - 1);
		if ((j - i) & 1)
			j--;
		size = strlen(iiarg) + strlen(argv[j]) + strlen(argv[j + 1]) +
		    strlen(IIEXEC) + 11;

		if ((iicmd = calloc(size, 1)) == NULL)
			_exit(EXIT_FAILURE);
		strlcpy(iicmd, IIEXEC" ", size);
		strlcat(iicmd, iiarg, size);
		strlcat(iicmd, " -s ", size);
		strlcat(iicmd, argv[j], size);
		strlcat(iicmd, " -p ", size);
		strlcat(iicmd, argv[j + 1], size);

		if (sharg) {
			shpid = fork();
			if (shpid < 0) {
				_exit(EXIT_FAILURE);
			} else if (shpid > 0) {
				rv = system(iicmd);
				if (rv < 0 || WEXITSTATUS(rv) == 127)
					killpg(sid, SIGKILL);
				free(iicmd);
				continue;
			}

			sleep(CHLDSLEEP);
			rv = system(sharg);
			if (rv < 0 || WEXITSTATUS(rv) != EXIT_SUCCESS)
				killpg(sid, SIGKILL);
			_exit(EXIT_SUCCESS);
		} else {
			rv = system(iicmd);
			if (rv < 0 || WEXITSTATUS(rv) == 127)
				_exit(EXIT_FAILURE);
			free(iicmd);
		}
	}
	/* NOTREACHED */
}

static void
usage(void)
{
	extern char *__progname;

	(void)fprintf(stderr, "usage: %s [-i ii arg] [-s sh arg] \\\n\t --"
	    " host1 port1 [host2 port2 ...]\n", __progname);
	exit(EXIT_FAILURE);
}
