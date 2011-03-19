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

#define ELEMENT(i) ((i) / 2 + 1)

int
main(int argc, char **argv)
{
	int i, rv;
	pid_t iipid, shpid = 0;
	size_t size;
	char *iiarg = NULL, *iicmd = NULL, *sharg = NULL;

	if (argc < 3)
		usage();
	while ((rv = getopt(argc, argv, "i:s:")) != -1)
		switch (rv) {
		case 'i':
			if (valsysarg(optarg))
				errx(EXIT_FAILURE, "sh(1) meta-characters and"
				    " quotation marks are not allowed in `ii"
				    " arg'.");
			iiarg = optarg;
			break;
		case 's':
			if (sharg)
				errx(EXIT_FAILURE, "Do not use the -s flag"
				    " multiple times.");
			if (valsysarg(optarg))
				errx(EXIT_FAILURE, "sh(1) meta-characters and"
				    " quotation marks are not allowed in `sh"
				    " arg'.");
			size = strlen(optarg);
			if ((sharg = calloc(size + 3, 1)) == NULL)
				err(EXIT_FAILURE, "Failed to allocate"
				    " memory.");
			memcpy(sharg, optarg, size);
			memcpy(sharg + size, " &", 2);
			break;
		default:
			usage();
			/* NOTREACHED */
		}

	argc -= optind;
	argv += optind;

	if (argc & 1)
		usage();

	/* Validate the hostnames and ports. */
	for (i = 0; i < argc; i++)
		if (i & 1) {
			if (valport(argv[i]))
				errx(EXIT_FAILURE, "Port number `%d' is not"
				    " ok.", ELEMENT(i));
		} else
			if (valhost(argv[i]))
				errx(EXIT_FAILURE, "Hostname `%d' is not ok.",
					ELEMENT(i));

	if (iiarg)
		printf("ii arg: `%s'.\n", iiarg);
	if (sharg)
		printf("sh arg: `%s'.\n", sharg);
	for (i = 0; i < argc - 1; i += 2)
		printf("Server[%d]: %s:%d\n", ELEMENT(i), argv[i],
		    atoi(argv[i + 1]));

	iipid = fork();
	if (iipid < 0)
		err(EXIT_FAILURE, "Failed to create child process.");
	else if (iipid > 0)
		exit(EXIT_SUCCESS);

	umask(0);

	if (setsid() < 0)
		_exit(EXIT_FAILURE);

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	srandom(time(NULL));
	signal(SIGCHLD, SIG_IGN);

	for (;;) {
		/* http://eternallyconfuzzled.com/arts/jsw_art_rand.aspx */
		i = random() * 1.0 / (RAND_MAX + 1.0) * (argc - 1);
		/* If we hit a port number, choose its relating hostname. */
		if (i & 1)
			i--;
		size = strlen(argv[i]) + strlen(argv[i + 1]) + strlen(IIEXEC) +
		    10;
		if (iiarg)
			size += strlen(iiarg);
		if ((iicmd = calloc(size, 1)) == NULL)
			_exit(EXIT_FAILURE);
		strlcpy(iicmd, IIEXEC" ", size);
		if (iiarg)
			strlcat(iicmd, iiarg, size);
		strlcat(iicmd, " -s ", size);
		strlcat(iicmd, argv[i], size);
		strlcat(iicmd, " -p ", size);
		strlcat(iicmd, argv[i + 1], size);

		if (sharg) {
			shpid = fork();
			if (shpid < 0) {
				_exit(EXIT_FAILURE);
			} else if (!shpid) {
				/* Child process with process ID shpid. */
				sleep(CHLDSLEEP);
				/* Will not block. */
				rv = system(sharg);
				if (rv < 0 || WEXITSTATUS(rv) != EXIT_SUCCESS)
					killpg(0, SIGKILL);
				_exit(EXIT_SUCCESS);
			}
		}

		system(iicmd);
		free(iicmd);
		if (shpid && kill(shpid, SIGKILL) && errno != ESRCH)
			_exit(EXIT_FAILURE);
		sleep(LOOPSLEEP);
	}
	/* NOTREACHED */
}

static void
usage(void)
{
	extern char *__progname;

	fprintf(stderr, "usage: %s [-i ii arg] [-s sh arg] \\\n\t"
	    " host1 port1 [host2 port2 ...]\n", __progname);
	exit(EXIT_FAILURE);
}
