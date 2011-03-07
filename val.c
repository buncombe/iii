/*
 * The author of this work has dedicated it to the public by waiving all of his
 * or her rights to the work under copyright law and all related or neighboring
 * legal rights he or she had in the work, to the extent allowable by law.
 */

#include <stdlib.h>
#include <string.h>

/*
 * valhost() - Validate hostname
 *
 * Abstract:
 *  According to RFC1123 and RFC952, a hostname can only begin and end with any
 *  character in the ranges A-Z (case-insensitive) and 0-9. At the same time, a
 *  hostname also can contain dots (`.') and hyphens (`-'), although its total
 *  length must not exceed 255 bytes and each label may not be longer than 63
 *  octets.
 *
 *  The function valhost() takes one argument: a pointer to a NUL-terminated
 *  char array h.
 *
 * Return values:
 *  -3    Something is wrong with the labels.
 *  -2    The char array h is too long.
 *  -1    Something is wrong with the prefix or suffix.
 *   0    It is ok.
 *   1<=n Character n-1 (according to the array index) is not allowed.
 *
 * List of character ranges:
 *  a-z (0x61 <= c <= 0x7a)
 *  0-9 (0x30 <= c <= 0x39)
 *  A-Z (0x41 <= c <= 0x5a)
 */
int
valhost(char *h)
{
	size_t i, j;

	/* Test prefixes and suffixes. */
	if (h[0] == '.' || h[0] == '-' || h[strlen(h) - 1] == '-' ||
	    h[strlen(h) - 1] == '.')
		return -1;

	/* Test length. */
	if (strlen(h) > 255)
		return -2;

	/* Test characters and labels. */
	for (i = 0, j = 0; i < strlen(h); i++) {
		if ((h[i] < 0x61 || h[i] > 0x7a) && (h[i] < 0x30 ||
		    h[i] > 0x39) && (h[i] < 0x41 || h[i] > 0x5a)
		    && h[i] != '.' && h[i] != '-')
			return i + 1;

		if (j == 64 || (i && h[i] == '.' && h[i - 1] == '.'))
			return -3;

		if (h[i] != '.')
			j++;
		else
			j = 0;
	}

	return 0;
}

/*
 * valport() - Validate port
 *
 * If the atoi(3) value of the NUL-terminated p, which is passed by reference
 * as a pointer to a char array in the sole argument of the function, is not
 * within the range 0 < i < 65536, then it returns the value -1.
 *
 * If the value of element x in p is not a number, the location of x (according
 * to the array index) added with 1 is returned (thus never 0).
 *
 * In case p passes all the tests, then the value 0 is returned.
 */
int
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

/*
 * valsysarg() - Validate system(3) argument
 *
 * valsysarg() takes a pointer to a char array s and tests whether s contains
 * any non-escaped meta-characters (`<', `>', `|', `;', `(', `)' and `&') and
 * quotation marks (`"' and `\''), in case it does: it returns a value greater
 * than zero which represents the element after the one (i + 1) where the
 * character was found.
 *
 * If s is clean from non-escaped meta-characters and quotation marks,
 * valsysarg() returns 0.
 */
size_t
valsysarg(char *s)
{
	size_t i;

	for (i = 0; i < strlen(s); i++)
		if ((s[i] == '<' || s[i] == '>' || s[i] == '|' ||
		    s[i] == ';' || s[i] == '(' || s[i] == ')' || s[i] == '&' ||
		    s[i] == '"' || s[i] == '\'') && (!i || s[i - 1] != '\\'))
			return i + 1;

	return 0;
}
