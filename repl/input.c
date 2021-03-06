/**
 * input.c
 * Handles the input of expressions in the REPL.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

// Make sure we also have a "T-variant" for int.
#ifdef UNICODE
#include <wchar.h>
typedef wint_t TINT;
#else
typedef int TINT;
#endif  // UNICODE

#include "input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef USE_GNU_READLINE
	#include <readline/readline.h>
	#include <readline/history.h>
#endif  // USE_GNU_READLINE

/**
 * Initializes the REPL.
 */
void repl_init(void) {
	// Placeholder for future things.
}

/**
 * Cleans up any mess created by the REPL.
 */
void repl_destroy(void) {
#ifdef USE_GNU_READLINE
	// Empty readline's history.
	clear_history();
#endif  // USE_GNU_READLINE
}

/**
 * Reads the user input like a command prompt.
 *
 * @param  buf Pointer to the buffer that will hold the user input.
 * @param  len Maximum length to read the user input including NULL terminator.
 * @return     Non-zero to break out of the REPL loop.
 */
int repl_readline(TCHAR *buf, size_t len) {
#ifdef USE_GNU_READLINE
	char *line;
	size_t linelen = 0;

	// Read the line using GNU Readline.
	line = readline("> ");
	if (line == NULL)
		return -1;

	// Check if the line isn't too long.
	linelen = strlen(line);
	if (linelen > len)
		return linelen;

	// Add the line to the history.
	if (linelen > 0)
		add_history(line);

#ifdef UNICODE
	// Convert our string to a wide string for the buffer.
	mbstowcs(buf, line, len);
#else
	// Copy the line to our buffer.
	strcpy(buf, line);
#endif

	// Free our buffer.
	free(line);

	return 0;
#else
	// Implement our own line reader.
	uint16_t i;
	int16_t openparens = 0;
	int16_t paren_index = 0;
	bool instring = false;

	// Put some safe guards in place.
	buf[0] = _T('\0');
	buf[len] = _T('\0');

	// Get user input.
	_tprintf(_T("> "));
	for (i = 0; i < len; i++) {
		// Get character from STDIN.
		TINT c = _gettchar();

		switch (c) {
		case _T('\"'):
			// Opening or closing a string.
			instring = !instring;
			break;
		case _T('('):
			// Opened a parenthesis.
			if (!instring)
				openparens++;
			break;
		case _T(')'):
			// Closed a parenthesis.
			if (!instring)
				openparens--;
			break;
		case _T('\n'):
			// Only return the string if all the parenthesis have been closed.
			if (openparens < 1) {
				buf[i] = _T('\0');
				goto returnstr;
			}

			// Add some indentation into the mix.
			for (paren_index = 0; paren_index <= openparens; paren_index++)
				_tprintf(_T("  "));
			break;
		}

		// Append character to the buffer.
		buf[i] = (TCHAR)c;
	}

returnstr:
	return 0;
#endif  // USE_GNU_READLINE
}
