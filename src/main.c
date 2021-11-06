/**
 * bamboo.c
 * The amazingly embeddable Lisp.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "bamboo.h"

// Private definitions.
#define REPL_INPUT_MAX_LEN 100

// Private methods.
int readline(char *buf, size_t len);
bamboo_error_t builtin_quit(atom_t args, atom_t *result);

/**
 * Program's main entry point.
 *
 * @return 0 if everything went fine.
 */
int main(void) {
	char *input;
	bamboo_error_t err;
	env_t env;

	// Initialize the interpreter.
	err = bamboo_init(&env);
	if (err)
		return err;

	// Add our own custom built-in function.
	bamboo_env_set_builtin(env, "QUIT", builtin_quit);

	// Start the REPL.
	input = (char *)malloc(sizeof(char) * (REPL_INPUT_MAX_LEN + 1));
	while (!readline(input, REPL_INPUT_MAX_LEN)) {
		atom_t parsed;
		atom_t result;

		// Parse the user's input.
		err = parse_expr(input, &input, &parsed);
		if (err) {
			bamboo_print_error(err);
			printf(LINEBREAK);

			continue;
		}

		// Evaluate the parsed expression.
		err = bamboo_eval_expr(parsed, env, &result);
		if (err) {
			bamboo_print_error(err);
			printf(LINEBREAK);

			continue;
		}

		// Print the evaluated result.
		bamboo_print_expr(result);
		printf(LINEBREAK);
	}

	// Quit.
	free(input);
	printf("Bye!" LINEBREAK);
	system("pause");
	return 0;
}

/**
 * Reads the user input like a command prompt.
 *
 * @param  buf Pointer to the buffer that will hold the user input.
 * @param  len Maximum length to read the user input.
 * @return     Non-zero to break out of the REPL loop.
 */
int readline(char *buf, size_t len) {
	uint8_t i;

	// Put some safe guards in place.
	buf[0] = '\0';
	buf[len] = '\0';

	// Get user input.
	printf("> ");
	for (i = 0; i < REPL_INPUT_MAX_LEN; i++) {
		// Get character from STDIN.
		int c = getchar();

		// Did we get a return?
		if (c == '\n') {
			buf[i] = '\0';
			break;
		}

		// Append character to the buffer.
	    buf[i] = (char)c;
	}

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
		printf("Quitting from a custom built-in function." LINEBREAK);
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
	printf("Quitting from a custom built-in function with return value %d."
		   LINEBREAK, arg1.value.integer);
	exit(arg1.value.integer);
	return BAMBOO_OK;
}
