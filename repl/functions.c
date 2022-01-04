/**
 * functions.c
 * Built-in functions specially for the REPL.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "functions.h"
#include <stdio.h>
#include <stdlib.h>

// Built-in function prototypes.
bamboo_error_t builtin_quit(atom_t args, atom_t *result);

/**
 * Populates the environment with our built-in functions.
 *
 * @param  env Pointer to the environment to be populated.
 * @return     BAMBOO_OK if the population was successful.
 */
bamboo_error_t repl_populate_builtins(env_t *env) {
	bamboo_error_t err;

	// Basic pair operations.
	err = bamboo_env_set_builtin(*env, _T("QUIT"), builtin_quit);
	IF_BAMBOO_ERROR(err)
		return err;

	return BAMBOO_OK;
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
	arg1 = car(args);

	// Check if its the right type of argument.
	if (arg1.type != ATOM_TYPE_INTEGER)
		return BAMBOO_ERROR_WRONG_TYPE;

	// Exit with the specified return value.
	*result = arg1;
	_tprintf(_T("Bye!") LINEBREAK);
	return (bamboo_error_t)BAMBOO_REPL_QUIT;
}