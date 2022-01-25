/**
 * functions.c
 * Built-in functions specially for the REPL.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "functions.h"
#include <stdio.h>
#include <stdlib.h>
#include "fileutils.h"

// Built-in function prototypes.
bamboo_error_t builtin_quit(atom_t args, atom_t *result);
bamboo_error_t builtin_load(atom_t args, atom_t *result);

/**
 * Loads the contents of a source file into the given environment.
 * 
 * @param  env    Pointer to the environment for the source to be evaluated in.
 * @param  fname  Path to the file to be loaded.
 * @param  result Return atom of the last evaluated expression in the source.
 * @return        BAMBOO_OK if everything went fine.
 */
bamboo_error_t load_source(env_t *env, const TCHAR *fname, atom_t *result) {
	bamboo_error_t err;
	atom_t parsed;
	TCHAR *contents;
	const TCHAR *end;

	// Start from a clean slate.
	err = BAMBOO_OK;
	*result = nil;

	// Just remind the user of what's happening.
	_tprintf(_T("Loading ") SPEC_STR LINEBREAK, fname);

	// Get the file contents.
	contents = slurp_file(fname);
	if (contents == NULL) {
		return bamboo_error(BAMBOO_ERROR_UNKNOWN,
			_T("Couldn't read the specified file for some reason"));
	}

	// Parse and evaluate the contents of the file.
	end = contents;
	while (*end != _T('\0')) {
#ifdef DEBUG
		// Check out our tokens.
		bamboo_print_tokens(end);
		_tprintf(LINEBREAK);
#endif  // DEBUG

		// Parse the expression.
		err = bamboo_parse_expr(end, &end, &parsed);
		IF_BAMBOO_ERROR(err) {
			// Show the error message.
			bamboo_print_error(err);
			_ftprintf(stderr, LINEBREAK);

			goto stop;
		}

		// Evaluate the parsed expression.
		err = bamboo_eval_expr(parsed, *env, result);
		IF_BAMBOO_ERROR(err) {
			// Check if we just got a quit situation.
			if (err == (bamboo_error_t)BAMBOO_REPL_QUIT)
				goto stop;

			// Explain the real issue then...
			bamboo_print_error(err);
			_ftprintf(stderr, LINEBREAK);
			goto stop;
		}
	}

stop:
	// Clean up and return.
	free(contents);
	return err;
}

/**
 * Populates the environment with our built-in functions.
 *
 * @param  env Pointer to the environment to be populated.
 * @return     BAMBOO_OK if the population was successful.
 */
bamboo_error_t repl_populate_builtins(env_t *env) {
	bamboo_error_t err;

	// Quit interpreter.
	err = bamboo_env_set_builtin(*env, _T("QUIT"), builtin_quit);
	IF_BAMBOO_ERROR(err)
		return err;

	// Load source file.
	err = bamboo_env_set_builtin(*env, _T("LOAD"), builtin_load);
	IF_BAMBOO_ERROR(err)
		return err;

	return BAMBOO_OK;
}

/**
 * Quits the interpreter
 *
 * (quit [retval])
 *
 * @param retval Value to be returned when exiting the interpreter.
 */
bamboo_error_t builtin_quit(atom_t args, atom_t *result) {
	atom_t retval;

	// Just in case...
	*result = bamboo_int(-1);

	// Check if we don't have any arguments.
	if (nilp(args)) {
		result->value.integer = 0;
		_tprintf(_T("Bye!") LINEBREAK);
		
		return (bamboo_error_t)BAMBOO_REPL_QUIT;
	}

	// Check if we have more than a single argument.
	if (!nilp(cdr(args)))
		return BAMBOO_ERROR_ARGUMENTS;

	// Get the first argument.
	retval = car(args);

	// Check if its the right type of argument.
	if (retval.type != ATOM_TYPE_INTEGER)
		return BAMBOO_ERROR_WRONG_TYPE;

	// Exit with the specified return value.
	*result = retval;
	_tprintf(_T("Bye!") LINEBREAK);
	return (bamboo_error_t)BAMBOO_REPL_QUIT;
}

/**
 * Evaluates the contents of a file.
 *
 * (load fname) -> any?
 *
 * @param  fname Path to the file to be loaded into the environment.
 * @return       Last return value of the evaluated source.
 */
bamboo_error_t builtin_load(atom_t args, atom_t *result) {
	atom_t fname;

	// Just in case...
	*result = nil;

	// Check if we don't have any arguments.
	if (nilp(args)) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			_T("A file path must be supplied to this function"));
	}

	// Check if we have more than a single argument.
	if (!nilp(cdr(args))) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			_T("Only a single file path should be supplied to this function"));
	}

	// Get the file name argument.
	fname = car(args);

	// Check if its the right type of argument.
	if (fname.type != ATOM_TYPE_STRING) {
		return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
			_T("File name atom must be of type string"));
	}

	// Load the file.
	return load_source(bamboo_get_root_env(), *fname.value.str, result);
}