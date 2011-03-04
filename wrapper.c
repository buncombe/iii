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

#define FIND_ELEM(a, b) (((a) - (b)) / 2 + 1)

static size_t	valhost(char *);
static int	valport(char *);
static size_t	valsysarg(char *);
static void	usage(void);

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
				errx(1, "sh(1) meta-characters and quotation"
				    " marks are not allowed in `ii arg'.");
			iiarg = optarg;
			break;
		case 's':
			if (valsysarg(optarg))
				errx(1, "sh(1) meta-characters and quotation"
				    " marks are not allowed in `sh arg'.");
			sharg = optarg;
			break;
		default:
			usage();
			/* NOTREACHED */
		}

	for (i = 1; i < argc - 2; i++)
		if (!strcmp(argv[i], "--"))
			break;
	i++; /* argv[i] is now the first hostname, hopefully: */
	if ((argc - i) & 1)
		usage();

	for (j = i; j < argc; j++) /* Validate the hosts and ports. */
		if ((j - i) & 1) {
			if ((rv = valport(argv[j])) < 0)
				errx(1, "Port number `%lu' is not in range.",
				    (unsigned long)FIND_ELEM(j, i));
			else if (rv > 0)
				errx(1, "Character `%lu' in port `%lu' is not"
				    " allowed.", (unsigned long)rv,
				    (unsigned long)FIND_ELEM(j, i));
		} else
			if ((rv = valhost(argv[j])) != NULL)
				errx(1, "Character `%lu' in hostname `%lu' is"
				    " not allowed.", (unsigned long)rv,
				    (unsigned long)FIND_ELEM(j, i));

	if (iiarg)
		printf("ii arg: `%s'.\n", iiarg);
	if (sharg)
		printf("sh arg: `%s'.\n", sharg);
	for (j = i; j < argc - 1; j += 2)
		printf("Server[%lu]: %s:%d\n", (unsigned long)FIND_ELEM(j, i),
		    argv[j], atoi(argv[j + 1]));

	iipid = fork();
	if (iipid < 0)
		err(1, "Failed to create child process.");
	else if (iipid > 0)
		exit(0);

	umask(0);

	sid = setsid();
	if (sid < 0)
		_exit(1);

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	srandom(time(NULL));

	for (;;) {
		if (sharg && shpid > 0) {
			if (kill(shpid, SIGKILL) && errno != ESRCH)
				_exit(1);
			/* FIXME: see SA_NOCLDWAIT in sigaction(3). */
			waitpid(0, NULL, 0);
		}

		sleep(LOOP_SLEEP);

		/* http://eternallyconfuzzled.com/arts/jsw_art_rand.aspx */
		j = i + 1 + random() * 1.0 / ( RAND_MAX + 1.0 ) * (argc - i - 1);
		if ((j - i) & 1)
			j--;
		size = strlen(iiarg) + strlen(argv[j]) + strlen(argv[j + 1]) +
		    11 + strlen(II_EXEC);

		if ((iicmd = calloc(size, 1)) == NULL)
			_exit(1);
		strlcpy(iicmd, II_EXEC" ", size);
		strlcat(iicmd, iiarg, size);
		strlcat(iicmd, " -s ", size);
		strlcat(iicmd, argv[j], size);
		strlcat(iicmd, " -p ", size);
		strlcat(iicmd, argv[j + 1], size);

		if (sharg) {
			shpid = fork();
			if (shpid < 0) {
				_exit(1);
			} else if (shpid > 0) {
				system(iicmd); /* FIXME: Check return value. */
				free(iicmd);
				continue;
			}

			sleep(CHLD_SLEEP);
			system(sharg); /* FIXME: Check return value. */
			_exit(0);
		} else {
			system(iicmd);  /* FIXME: Check return value. */
			free(iicmd);
		}
	}
}

static size_t
valhost(char *h)
{
	size_t i;

	/*
	 * Permitted characters in a hostname:
	 * 	a-z	(0x61 <= c <= 0x7a)
	 *	0-9	(0x30 <= c <= 0x39)
	 *	A-Z	(0x41 <= c <= 0x5a)
	 *	.-
	 */
	/* FIXME: Is too restrictive in its current shape. */
	for (i = 0; i < strlen(h); i++)
		if (((h[i] < 0x61 || h[i] > 0x7a) && (h[i] < 0x30 ||
		    h[i] > 0x39) && (h[i] < 0x41 || h[i] > 0x5a))
		    && h[i] != '.' && h[i] != '-')
			return i + 1;
	return 0;
}

static int
valport(char *p)
{
	size_t i;

	if (atoi(p) < 1 || atoi(p) > 65535)
		return -1;

	for (i = 0; i < strlen(p); i++)
		if (p[i] < 0x30 || p[i] > 0x39)
			return i + 1;
	return 0;
}

static size_t
valsysarg(char *s)
{
	size_t i;

	for (i = 0; i < strlen(s); i++)
		if (s[i] == '<' || s[i] == '>' || s[i] == '|' || s[i] == ';' ||
		    s[i] == '(' || s[i] == ')' || s[i] == '&' || s[i] == '"' ||
		    s[i] == '\'')
			if (i && s[i - 1] != '\\')
				return i + 1;
	return 0;
}

static void
usage(void)
{
	extern char *__progname;

	(void)fprintf(stderr, "usage: %s [-i ii arg] [-s sh arg] \\\n\t --"
	    " host1 port1 [host2 port2 ...]\n", __progname);
	exit(1);
}
