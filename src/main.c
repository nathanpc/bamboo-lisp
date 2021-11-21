/**
 * bamboo.c
 * The amazingly embeddable Lisp.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifdef _MSC_VER
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "bamboo.h"

// Private definitions.
#define REPL_INPUT_MAX_LEN 512

// Private methods.
int readline(TCHAR *buf, size_t len);
bamboo_error_t builtin_quit(atom_t args, atom_t *result);

/**
 * Program's main entry point.
 *
 * @return 0 if everything went fine.
 */
#ifdef _MSC_VER
#ifdef UNICODE
int wmain(void) {
#endif
#else
int main(void) {
#endif
	TCHAR *input;
	bamboo_error_t err;
	env_t env;

#ifdef _MSC_VER
	// Make sure we can use unicode characters inside the Windows console.
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
#endif

	// Initialize the interpreter.
	err = bamboo_init(&env);
	if (err)
		return err;

	// Add our own custom built-in function.
	bamboo_env_set_builtin(env, _T("QUIT"), builtin_quit);

	// Allocate memory for the REPL input data.
	input = (TCHAR *)malloc(sizeof(TCHAR) * (REPL_INPUT_MAX_LEN + 1));
	if (input == NULL) {
#ifdef UNICODE
		fwprintf(stderr, _T("Can't allocate the input string for the REPL")
			LINEBREAK);
#else
		fprintf(stderr, _T("Can't allocate the input string for the REPL")
			LINEBREAK);
#endif
		return 1;
	}

	// Start the REPL.
	while (!readline(input, REPL_INPUT_MAX_LEN)) {
		atom_t parsed;
		atom_t result;
		const TCHAR *end;

		// Parse the user's input.
		err = bamboo_parse_expr(input, &end, &parsed);
		if (err > BAMBOO_OK) {
			uint8_t spaces;
			
			// Show where the user was wrong.
#ifdef UNICODE
			wprintf(input);
			wprintf(LINEBREAK);
#else
			printf(input);
			printf(LINEBREAK);
#endif
			for (spaces = 0; spaces < (end - input); spaces++)
#ifdef UNICODE
					putwchar(_T(' '));
				wprintf(_T("^ "));
#else
				putchar(_T(' '));
			printf(_T("^ "));
#endif
			// Show the error message.
			bamboo_print_error(err);
#ifdef UNICODE
			fwprintf(stderr, LINEBREAK);
#else
			fprintf(stderr, LINEBREAK);
#endif

			continue;
		}

		// Evaluate the parsed expression.
		err = bamboo_eval_expr(parsed, env, &result);
		if (err > BAMBOO_OK) {
			bamboo_print_error(err);
#ifdef UNICODE
			fwprintf(stderr, LINEBREAK);
#else
			fprintf(stderr, LINEBREAK);
#endif

			continue;
		}

		// Print the evaluated result.
		bamboo_print_expr(result);
#ifdef UNICODE
		wprintf(LINEBREAK);
#else
		printf(LINEBREAK);
#endif
	}

	// Quit.
	free(input);
#ifdef UNICODE
	wprintf(_T("Bye!") LINEBREAK);
#else
	printf(_T("Bye!") LINEBREAK);
#endif

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
#ifdef UNICODE
	wprintf(_T("> "));
#else
	printf(_T("> "));
#endif
	for (i = 0; i < len; i++) {
		// Get character from STDIN.
#ifdef UNICODE
		wint_t c = getwchar();
#else
		int c = getchar();
#endif

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

#ifdef UNICODE
			wprintf(_T("  "));
#else
			printf(_T("  "));
#endif
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
#ifdef UNICODE
		wprintf(_T("Quitting from a custom built-in function.") LINEBREAK);
#else
		printf(_T("Quitting from a custom built-in function.") LINEBREAK);
#endif
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

	// Exit with the specified return value.
#ifdef UNICODE
	wprintf(_T("Quitting from a custom built-in function with return value %d.")
		   LINEBREAK, arg1.value.integer);
#else
	printf(_T("Quitting from a custom built-in function with return value %d.")
		   LINEBREAK, arg1.value.integer);
#endif

	exit(arg1.value.integer);
	return BAMBOO_OK;
}
