/**
 * Bamboo REPL
 * A simple console REPL for the Bamboo Lisp C++ implementation.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include <tchar.h>
#include <string>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <clocale>
#include <fcntl.h>
#include <io.h>
#include "BambooWrapper.h"

// Private definitions.
#define REPL_INPUT_MAX_LEN 512

// Private methods.
int readline(TCHAR *buf, size_t len);
bamboo_error_t builtin_quit(atom_t args, atom_t *result);

/**
 * Program's main entry point.
 *
 * @param  argc Number of command-line arguments passed to the program.
 * @param  argv Command-line arguments passed to the program.
 * @return      0 if everything went fine.
 */
int _tmain(int argc, TCHAR *argv[]) {
	TCHAR *input;
	Bamboo::Lisp bamboo;

#ifdef UNICODE
	// Enable support for unicode in the console.
	(void)_setmode(_fileno(stdout), _O_WTEXT);
	(void)_setmode(_fileno(stdin), _O_WTEXT);
#endif

	// Add our own custom built-in function.
	bamboo.env().set_builtin(_T("QUIT"), builtin_quit);

	// Allocate memory for the REPL input data.
	input = (TCHAR *)malloc(sizeof(TCHAR) * (REPL_INPUT_MAX_LEN + 1));
	if (input == NULL) {
		_ftprintf(stderr, _T("Can't allocate the input string for the REPL")
			LINEBREAK);
		return 1;
	}

	// Start the REPL.
	while (!readline(input, REPL_INPUT_MAX_LEN)) {
		try {
			// Parse the user's input and evaluate the expression.
			atom_t parsed = bamboo.parse_expr(input);
			atom_t result = bamboo.eval_expr(parsed);

			// Print the evaluated result.
			bamboo_print_expr(result);
			std::cout << std::endl;
		} catch (Bamboo::BambooException e) {
			// Print the error encountered.
			std::cerr << e.what() << std::endl;
		}
	}

	// Quit.
	free(input);
	std::cout << _T("Bye!") << std::endl;

	return 0;
}

/**
 * Reads the user input like a command prompt.
 *
 * @param  buf Pointer to the buffer that will hold the user input.
 * @param  len Maximum length to read the user input including NULL terminator.
 * @return     Non-zero to break out of the REPL loop.
 */
int readline(TCHAR *buf, size_t len) {
	uint16_t i;
	int16_t openparens = 0;
	bool instring = false;

	// Put some safe guards in place.
	buf[0] = _T('\0');
	buf[len] = _T('\0');

	// Get user input.
	_tprintf(_T("> "));
	for (i = 0; i < len; i++) {
		// Get character from STDIN.
		wint_t c = _gettchar();

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

			_tprintf(_T("  "));
		}

		// Append character to the buffer.
	    buf[i] = (TCHAR)c;
	}

returnstr:
	return 0;
}

/**
 * Simple example of how to create a built-in function.
 *
 * (quit [retval])
 *
 * @param  args   List of arguments passed to the function.
 * @param  result Return atom of calling the function.
 * @return        BAMBOO_OK if the call was sucessful, otherwise check the
 *                bamboo_error_t enum.
 */
bamboo_error_t builtin_quit(atom_t args, atom_t *result) {
	atom_t arg1;

	// Just set the return to nil since we wont use it.
	*result = nil;

	// Check if we don't have any arguments.
	if (nilp(args)) {
		_tprintf(_T("Bye!") LINEBREAK);
		exit(0);
	}

	// Check if we have more than a single argument.
	if (!nilp(cdr(args)))
		return BAMBOO_ERROR_ARGUMENTS;

	// Get the first argument.
	arg1 = car(args);

	// Check if its the right type of argument.
	if (arg1.type != ATOM_TYPE_INTEGER)
		return BAMBOO_ERROR_WRONG_TYPE;

	_tprintf(_T("Bye!") LINEBREAK);
	exit((int)arg1.value.integer);
	return BAMBOO_OK;
}
