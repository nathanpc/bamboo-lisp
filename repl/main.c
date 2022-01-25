/**
 * main.c
 * Fully-featured REPL and interpreter for the Bamboo Lisp implementation.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

// Make sure Visual C++ and Open Watcom under Windows are happy.
#ifdef _WIN32
	#include <windows.h>

	// For some reason this is not defined in the standard header.
	#ifndef _O_WTEXT
		#define _O_WTEXT 0x10000
	#endif  // _O_WTEXT

	// Make sure unicode is enabled.
	#ifndef __WATCOMC__
		#ifndef _UNICODE
			#define _UNICODE
		#endif  // _UNICODE
		#ifndef UNICODE
			#define UNICODE
		#endif  // UNICODE
	#endif  // __WATCOMC__
#endif  // _WIN32

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <locale.h>
#ifdef _WIN32
	#include <fcntl.h>
	#include <io.h>
#endif // _WIN32
#include "common/tgetopt.h"
#include "../src/bamboo.h"
#include "input.h"
#include "functions.h"

// Private definitions.
#define REPL_INPUT_MAX_LEN 512

// Private variables.
static env_t repl_env;
static TCHAR repl_input[REPL_INPUT_MAX_LEN + 1];

// Private methods.
void enable_unicode(void);
void usage(const TCHAR *pname, int retval);
void parse_arguments(int argc, TCHAR **argv);

/**
 * Program's main entry point.
 *
 * @param  argc Number of command-line arguments passed to the program.
 * @param  argv Command-line arguments passed to the program.
 * @return      0 if everything went fine.
 */
int _tmain(int argc, TCHAR *argv[]) {
	bamboo_error_t err;
	int retval = 0;

	// Enable Unicode support in the console and parse any given arguments.
	enable_unicode();
	parse_arguments(argc, argv);

	// Initialize the interpreter.
	err = bamboo_init(&repl_env);
	IF_BAMBOO_ERROR(err)
		return err;

	// Add our own REPL-related built-in functions.
	err = repl_populate_builtins(&repl_env);
	IF_BAMBOO_ERROR(err)
		return err;

	// Start the REPL.
	repl_init();
	while (!repl_readline(repl_input, REPL_INPUT_MAX_LEN)) {
		atom_t parsed;
		atom_t result;
		const TCHAR *end = repl_input;

		// Check if we've parsed all of the statements in the expression.
		while (*end != _T('\0')) {
#ifdef DEBUG
			// Check out our tokens.
			bamboo_print_tokens(end);
			_tprintf(LINEBREAK);
#endif  // DEBUG

			// Parse the user's input.
			err = bamboo_parse_expr(end, &end, &parsed);
			IF_BAMBOO_ERROR(err) {
				uint8_t spaces;

				// Show where the user was wrong.
				_tprintf(_T("%s %s"), repl_input, LINEBREAK);
				for (spaces = 0; spaces < (end - repl_input); spaces++)
					_puttchar(_T(' '));
				_tprintf(_T("^ "));

				// Show the error message.
				bamboo_print_error(err);
				_ftprintf(stderr, LINEBREAK);

				continue;
			}

			// Evaluate the parsed expression.
			err = bamboo_eval_expr(parsed, repl_env, &result);
			IF_BAMBOO_ERROR(err) {
				// Check if we just got a quit situation.
				if (err == (bamboo_error_t)BAMBOO_REPL_QUIT) {
					retval = (int)result.value.integer;
					goto quit;
				}

				// Explain the real issue then...
				bamboo_print_error(err);
				_ftprintf(stderr, LINEBREAK);

				continue;
			}
		}

		// Print the evaluated result.
		bamboo_print_expr(result);
		_tprintf(LINEBREAK);
	}

quit:
	// Free up resources.
	err = bamboo_destroy(&repl_env);

	// Return the correct code.
	IF_BAMBOO_ERROR(err)
		return err;
	return retval;
}

/**
 * Program's main entry point.
 *
 * @param  argc Number of command-line arguments passed to the program.
 * @param  argv Command-line arguments passed to the program.
 * @return      0 if everything went fine.
 */
void parse_arguments(int argc, TCHAR **argv) {
	int opt;

	while ((opt = getopt(argc, argv, _T("abX"))) != -1) {
		switch (opt) {
		case _T('a'):
			_tprintf(_T("Option a was provided\r\n"));
			break;
		case _T('b'):
			_tprintf(_T("Option b was provided\r\n"));
			break;
		case _T('X'):
			_tprintf(_T("Option X was provided\r\n"));
			break;
		case _T('?'):
			usage(argv[0], 1);
			break;
		}
	}
}

/**
 * Prints the usage message of the program.
 * 
 * @param pname  Program name.
 * @param retval Return value to be used when exiting.
 */
void usage(const TCHAR *pname, int retval) {
	exit(retval);
}

/**
 * Enables Unicode support in the console if compiled with support.
 */
void enable_unicode(void) {
#ifdef UNICODE
#ifdef _WIN32
	(void)_setmode(_fileno(stdout), _O_WTEXT);
	(void)_setmode(_fileno(stdin), _O_WTEXT);
#else
	setlocale(LC_ALL, "en_US.UTF-8");
#endif  // _WIN32
#endif  // UNICODE
}
