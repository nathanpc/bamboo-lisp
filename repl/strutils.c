/**
 * strutils.h
 * Some utility functions to help us play around with strings.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "strutils.h"
#include <stdlib.h>

/**
 * Truncates a wide-character string into a regular string.
 * WARNING: This function allocates its return string, so you're responsible
 *          for freeing it.
 * 
 * @param  str Wide-character string to be converted (truncated).
 * @return     Regular truncated string.
 */
char* trunc_wchar(const wchar_t *str) {
	char *buf;
	char *tmp;
	const wchar_t *wtmp;

	// Allocate space for the string.
	buf = (char *)malloc((wcslen(str) + 1) * sizeof(char));
	if (buf == NULL)
		return NULL;

	// Copy the wide-string while truncating it.
	tmp = buf;
	wtmp = str;
	do {
		// Truncate the wide-character.
		*tmp = (char)(*wtmp & 0xFF);

		// Go to the next character in the string.
		tmp++;
		wtmp++;
	} while (*wtmp != L'\0');

	// Make sure we properly terminate our return string.
	*tmp = '\0';

	return buf;
}
