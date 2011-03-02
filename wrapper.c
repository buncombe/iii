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

#define LOCATE_HP(a, b) (((a) - (b)) / 2 + 1)

static size_t	host_is_valid(char *);
static size_t	is_sh_clear(char *);
static int	port_is_valid(char *);
static void	usage(void);

int
main(int argc, char **argv)
{
	const unsigned int uargc = (unsigned int)argc;
	char *args[2] = {NULL, NULL}, *init = NULL;
	size_t n, o, s;
	int ch, rv;
	pid_t pids[2], sid;

	if (uargc < 3) usage();
	while ((ch = getopt(argc, argv, "i:s:")) != -1)
		switch (ch) {
		case 'i':
			if (is_sh_clear(optarg))
				errx(1, "sh(1) meta-characters and quotation"
				    " marks are not allowed in `ii arg'.");
			args[0] = optarg;
			break;
		case 's':
			if (is_sh_clear(optarg))
				errx(1, "sh(1) meta-characters and quotation"
				    " marks are not allowed in `sh arg'.");
			args[1] = optarg;
			break;
		default:
			usage();
			/* NOTREACHED */
		}

	for (o = 1; o < uargc - 2; o++)
		if (!strcmp(argv[o], "--"))
			break;
	o++; /* argv[o] is now the first hostname, hopefully: */
	if ((uargc - o) & 1) usage();

	for (n = o; n < uargc; n++) /* Validate the hosts and ports. */
		if ((n - o) & 1) {
			if ((rv = port_is_valid(argv[n])) < 0)
				errx(1, "Port number `%lu' is not in range.",
				    (unsigned long)LOCATE_HP(n, o));
			else if (rv > 0)
				errx(1, "Character `%lu' in port `%lu' is not"
				    " allowed.", (unsigned long)rv,
				    (unsigned long)LOCATE_HP(n, o));
		} else
			if ((rv = host_is_valid(argv[n])) != NULL)
				errx(1, "Character `%lu' in hostname `%lu' is"
				    " not allowed.", (unsigned long)rv,
				    (unsigned long)LOCATE_HP(n, o));

	if (args[0]) printf("ii arg: `%s'.\n", args[0]);
	if (args[1]) printf("sh arg: `%s'.\n", args[1]);
	for (n = o; n < uargc - 1; n += 2)
		printf("Server[%lu]: %s:%d\n", (unsigned long)LOCATE_HP(n, o),
		    argv[n], atoi(argv[n + 1]));

	pids[0] = fork();
	if (pids[0] < 0)
		exit(1);
	else if (pids[0] > 0)
		exit(0);

	umask(0);

	sid = setsid();
	if (sid < 0)
		_exit(1);

	if ((chdir("/")) < 0)
		_exit(1);

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	srandom(time(NULL));

	for (;;) {
		if (args[1] && pids[1] > 0) {
			if (kill(pids[1], SIGKILL) && errno != ESRCH)
				_exit(1);
			/* FIXME: see SA_NOCLDWAIT in sigaction(3). */
			waitpid(0, NULL, 0);
		}

		sleep(LOOP_SLEEP);

		/* http://eternallyconfuzzled.com/arts/jsw_art_rand.aspx */
		n = o + 1 + random() * 1.0 / ( RAND_MAX + 1.0 ) * (uargc - o - 1);
		if ((n - o) & 1) n--;
		s = strlen(args[0]) + strlen(argv[n]) + strlen(argv[n + 1]) + 10
		    + sizeof(II_EXEC);

		if ((init = calloc(s, 1)) == NULL)
			_exit(1);
		strlcpy(init, II_EXEC" ", s);
		strlcat(init, args[0], s);
		strlcat(init, " -s ", s);
		strlcat(init, argv[n], s);
		strlcat(init, " -p ", s);
		strlcat(init, argv[n + 1], s);

		if (args[1]) {
			pids[1] = fork();
			if (pids[1] < 0) {
				_exit(1);
			} else if (pids[1] > 0) {
				system(init);
				free(init);
				continue;
			}

			sleep(CHLD_SLEEP);
			system(args[1]);
			_exit(0);
		} else {
			system(init);
			free(init);
		}
	}
}

static size_t
host_is_valid(char *h)
{
	size_t n;

	/*
	 * Permitted characters in a hostname:
	 * 	a-z	(0x61 <= c <= 0x7a)
	 *	0-9	(0x30 <= c <= 0x39)
	 *	A-Z	(0x41 <= c <= 0x5a)
	 *	.-
	 */
	/* FIXME: Is too restrictive in its current shape. */
	for (n = 0; n < strlen(h); n++)
		if (((h[n] < 0x61 || h[n] > 0x7a) && (h[n] < 0x30 ||
		    h[n] > 0x39) && (h[n] < 0x41 || h[n] > 0x5a))
		    && h[n] != '.' && h[n] != '-')
			return n + 1;
	return 0;
}

static size_t
is_sh_clear(char *s)
{
	size_t n;

	for (n = 0; n < strlen(s); n++)
		if (s[n] == '<' || s[n] == '>' || s[n] == '|' || s[n] == ';' ||
		    s[n] == '(' || s[n] == ')' || s[n] == '&' || s[n] == '"' ||
		    s[n] == '\'')
			if (n && s[n - 1] != '\\')
				return n + 1;
	return 0;
}

static int
port_is_valid(char *p)
{
	size_t n;

	if (atoi(p) < 1 || atoi(p) > 65535)
		return -1;

	for (n = 0; n < strlen(p); n++)
		if (p[n] < 0x30 || p[n] > 0x39)
			return n + 1;
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
