#include <sys/stat.h>
#include <sys/types.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define EXECUTABLE "ii"

/*
 * Takes user specified hostnames and port numbers and randomly selects one pair[1]
 * before it passes them as arguments to EXECUTABLE via system(3). Whenever system(3)
 * dies, a new pair is selected[2].
 * [2] is done in the background while [1] is not.
 */
int
main(int argc, char **argv)
{
	const unsigned int uargc = (unsigned int)argc;
	char *init = NULL;
	size_t i, j, r, rv = 0, s;
	pid_t pid, sid;

	/* Validate the user input. */
	if (uargc % 2 != 0 || uargc < 4)
		err(1, "\"<ii args>\" <host> <port> <host1> <port1> ...\n");

	/*
	 * sh(1) meta-characters: `<', `>', `|', `;', `(', `)', and `&'.
	 * system(3) caveat:
	 * http://www.openbsd.org/cgi-bin/man.cgi?query=system&apropos=0&sektion=0&manpath=OpenBSD+Current&arch=i386&format=html#CAVEATS
	 */
	for (i = 0; i < strlen(argv[1]); i++)
		if (argv[1][i] == '<' || argv[1][i] == '>' || argv[1][i] == '|'
		|| argv[1][i] == ';' || argv[1][i] == '(' || argv[1][i] == ')'
		|| argv[1][i] == '&' || argv[1][i] == '"' || argv[1][i] == '\'')
			err(1, "sh(1) meta-characters and quotation marks are"
			" not allowed.\n");

	for (i = 2; i < uargc; i++)
		if (i % 2 == 1) {
			if (atoi(argv[i]) < 1 || atoi(argv[i]) > 65535)
				err(1, "Port number `%lu' is not in range."
				" (Counting from zero.)\n", (unsigned long)i / 2 - 1);
			for (j = 0; j < strlen(argv[i]); j++)
				if (argv[i][j] < 0x30 || argv[i][j] > 0x39) /* !0-9 */
					err(1, "Character `%lu' in port `%lu'"
					" is not allowed."
					" (Counting from zero.)\n",
					(unsigned long)j, (unsigned long)i / 2 - 1);
		} else {
			for (j = 0; j < strlen(argv[i]); j++)
				if (((argv[i][j] < 0x61 || argv[i][j] > 0x7a) /* !a-z */
				&& (argv[i][j] < 0x30 || argv[i][j] > 0x39) /* !0-9 */
				&& (argv[i][j] < 0x41 || argv[i][j] > 0x5a)) /* !A-Z */
				&& argv[i][j] != 0x2d && argv[i][j] != 0x2e) /* !. !- */
					err(1, "Character `%lu' in hostname `%lu'"
					" is not allowed."
					" (Counting from zero.)\n",
					(unsigned long)j, (unsigned long)i / 2 - 1);
		}

	/* Tell the user what we have got. */
	for (i = 2; i < uargc; i += 2)
		printf("[%lu]: %s:%d\n", (unsigned long)(i - 1) / 2, argv[i],
		atoi(argv[i + 1]));

	/* Lets continue in the background. */
	pid = fork();
	if (pid < 0) {
		rv = 1;
		goto clean;
	} else if (pid > 0)
		goto clean;

	umask(0);

	sid = setsid();
	if (sid < 0) {
		rv = 1;
		goto clean;
	}

	if ((chdir("/")) < 0) {
		rv = 1;
		goto clean;
	}

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	srandom(time(NULL));

	while (1) {
		/*
		 * Calculate a random number in the range 0-(i/2 - 1). 
		 * http://eternallyconfuzzled.com/arts/jsw_art_rand.aspx
		 */
		r = random() * 1.0 / ( RAND_MAX + 1.0 ) * (i / 2 - 1);

		s = strlen(argv[1]) + strlen(argv[r * 2 + 2]) +
		strlen(argv[r * 2 + 3]) + 10 + sizeof(EXECUTABLE);

		if ((init = calloc(s, 1)) == NULL) {
			rv = 1;
			goto clean;
		}
		strlcpy(init, EXECUTABLE" ", s);
		strlcat(init, argv[1], s);
		strlcat(init, " -s ", s);
		strlcat(init, argv[r * 2 + 2], s);
		strlcat(init, " -p ", s);
		strlcat(init, argv[r * 2 + 3], s);

		system(init);
		free(init);
	}

clean:
	if (init != NULL)
		free(init);
	return rv;
}
