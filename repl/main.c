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

	// Make sure Unicode is enabled.
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
	#include "common/tgetopt.h"
#else
	#include <unistd.h>
	#include <getopt.h>
#endif // _WIN32
#include "../src/bamboo.h"
#include "input.h"
#include "functions.h"

// Private definitions.
#define REPL_INPUT_MAX_LEN 512

// Private variables.
static env_t repl_env;
static TCHAR repl_input[REPL_INPUT_MAX_LEN + 1];
static bool env_initialized;

// Private methods.
void enable_unicode(void);
void usage(const TCHAR *pname, int retval);
void parse_args(int argc, TCHAR **argv);
bamboo_error_t init_env(void);
bamboo_error_t destroy_env(void);
void repl(void);
void load_include(const TCHAR *fname, bool terminate);
void run_source(const TCHAR *fname);
void cleanup(void);

/**
 * Program's main entry point.
 *
 * @param  argc Number of command-line arguments passed to the program.
 * @param  argv Command-line arguments passed to the program.
 * @return      0 if everything went fine.
 */
int main(int argc, char *argv[]) {
	bamboo_error_t err;

	// Setup some flags.
	env_initialized = false;

	// Make sure we clean things up.
	atexit(cleanup);

	// Enable Unicode support in the console and parse any given arguments.
	enable_unicode();
	parse_args(argc, argv);

	// Initialize the Lisp environment.
	if (!env_initialized) {
		err = init_env();
		IF_BAMBOO_ERROR(err)
			goto quit;
	}

	// Start the REPL.
	repl();

quit:
	repl_destroy();
	destroy_env();
	return err;
}

/**
 * Initializes the Lisp environment.
 * 
 * @return BAMBOO_OK if everything went fine.
 */
bamboo_error_t init_env(void) {
	bamboo_error_t err;

	// Initialize the interpreter.
	err = bamboo_init(&repl_env);
	IF_BAMBOO_ERROR(err)
		return err;

	// Set our initialized flag.
	env_initialized = true;

	// Add our own REPL-related built-in functions.
	err = repl_populate_builtins(&repl_env);
	IF_BAMBOO_ERROR(err)
		return err;

	return err;
}

/**
 * Destroys our current Lisp environment and quit.
 * 
 * @return BAMBOO_OK if we were able to clean everything up.
 */
bamboo_error_t destroy_env(void) {
	// Do nothing if nothing was initialized.
	if (!env_initialized)
		return BAMBOO_OK;

	// Destroy our environment.
	return bamboo_destroy(&repl_env);
}

/**
 * Performs some basic cleanup before the program exits.
 */
void cleanup(void) {
	repl_destroy();
	destroy_env();
}

/**
 * Creates a classic Read-Eval-Print-Loop.
 */
void repl(void) {
	bamboo_error_t err;
	int retval = 0;

	// Initialize the REPL.
	err = BAMBOO_OK;
	repl_init();

	// Start the REPL loop.
	while (!repl_readline(repl_input, REPL_INPUT_MAX_LEN)) {
		atom_t parsed;
		atom_t result;
		const TCHAR *end = repl_input;

		// Check if all we've got was an empty line.
		if (*repl_input == _T('\0'))
			continue;

		// Check if we've parsed all of the statements in the expression.
		while (*end != _T('\0')) {
#ifdef DEBUG
			// Check out our tokens.
			bamboo_print_tokens(end);
			printf(LINEBREAK);
#endif  // DEBUG

			// Parse the user's input.
			err = bamboo_parse_expr(end, &end, &parsed);
			IF_BAMBOO_ERROR(err) {
				uint8_t spaces;

				// Show where the user was wrong.
				printf(_T("%s %s"), repl_input, LINEBREAK);
				for (spaces = 0; spaces < (end - repl_input); spaces++)
					putchar(_T(' '));
				printf(_T("^ "));

				// Show the error message.
				bamboo_print_error(err);
				goto next;
			}

			// Evaluate the parsed expression.
			err = bamboo_eval_expr(parsed, repl_env, &result);
			IF_BAMBOO_ERROR(err) {
				// Check if we just got a quit situation.
				if (err == (bamboo_error_t)BAMBOO_REPL_QUIT) {
					retval = (int)result.value.integer;
					err = BAMBOO_OK;

					goto quit;
				}

				// Explain the real issue then...
				bamboo_print_error(err);
				goto next;
			}
		}

		// Print the evaluated result.
		bamboo_print_expr(result);
		printf(LINEBREAK);
next:
		;
	}

quit:
	// Return the correct code.
	IF_BAMBOO_ERROR(err)
		exit((int)err);
	exit(retval);
}

/**
 * Loads a source file into the current environment.
 *
 * @param fname     Path to the source file to be loaded.
 * @param terminate Terminate the program after loading the source file?
 */
void load_include(const TCHAR *fname, bool terminate) {
	bamboo_error_t err;
	atom_t result;
	int retval = 0;

	// Initialize the Lisp environment.
	if (!env_initialized) {
		err = init_env();
		IF_BAMBOO_ERROR(err)
			goto quit;
	}

	// Load the file.
	err = load_source(&repl_env, fname, &result);
	IF_BAMBOO_ERROR(err) {
		// Check if we just got a quit situation.
		if (err == (bamboo_error_t)BAMBOO_REPL_QUIT) {
			retval = (int)result.value.integer;
			err = BAMBOO_OK;

			goto quit;
		}

		// Explain the real issue then...
		bamboo_print_error(err);
		fprintf(stderr, LINEBREAK);
		goto quit;
	}

	// Print the evaluated result.
	bamboo_print_expr(result);
	printf(LINEBREAK);

	// Continue the program execution if we want to.
	if (!terminate)
		return;

quit:
	// Return the correct code.
	IF_BAMBOO_ERROR(err)
		exit((int)err);
	exit(retval);
}

/**
 * Runs a source file and quits the application after its finished.
 *
 * @param fname Path to the source file to be executed.
 */
void run_source(const TCHAR *fname) {
	load_include(fname, true);
}

/**
 * Program's main entry point.
 *
 * @param  argc Number of command-line arguments passed to the program.
 * @param  argv Command-line arguments passed to the program.
 * @return      0 if everything went fine.
 */
void parse_args(int argc, TCHAR **argv) {
	int opt;

	while ((opt = getopt(argc, argv, _T("-:r:l:h"))) != -1) {
		switch (opt) {
		case _T('r'):
		case 1:
			// Run a script.
			run_source(optarg);
			break;
		case _T('l'):
			// Load a script into the current environment.
			load_include(optarg, false);
			break;
		case _T('h'):
			// Help
			usage(argv[0], EXIT_SUCCESS);
			break;
		case ':':
			printf(_T("Missing argument for ") SPEC_CHR LINEBREAK, optopt);
			usage(argv[0], EXIT_FAILURE);
			break;
		case _T('?'):
			printf(_T("Unknown option: ") SPEC_CHR LINEBREAK, optopt);
			// Fallthrough...
		default:
			usage(argv[0], EXIT_FAILURE);
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
	printf(_T("Usage: ") SPEC_STR _T(" [[-rl] source]") LINEBREAK LINEBREAK,
		pname);

	printf(_T("Options:") LINEBREAK);
	printf(_T("    -r <source>  Runs the source file and quits.")
		LINEBREAK);
	printf(_T("    -l <source>  Loads the source file before the REPL.")
		LINEBREAK);
	printf(_T("    -h           Displays this message.")
		LINEBREAK);

	printf(LINEBREAK _T("Author: Nathan Campos <nathan@innoveworkshop.com>")
		LINEBREAK);

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
#endif  // _WIN32
#else
	// Force UTF-8 on platforms other than Windows.
	setlocale(LC_ALL, "en_US.UTF-8");
#endif  // UNICODE
}
