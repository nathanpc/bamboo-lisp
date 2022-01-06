#include <stdio.h>
#include <stdlib.h>
#include "../src/bamboo.h"

// Custom built-in function prototype.
bamboo_error_t builtin_quit(atom_t args, atom_t *result);

int main(void) {
	bamboo_error_t err;
	env_t env;
	char *input = NULL;
	size_t inputlen = 0;

	// Initialize the interpreter.
	err = bamboo_init(&env);
	IF_BAMBOO_ERROR(err)
		return err;

	// Add our own custom built-in function.
	bamboo_env_set_builtin(env, "QUIT", builtin_quit);

	// Start the REPL loop.
	printf("> ");
	while (getline(&input, &inputlen, stdin) != -1) {
		atom_t parsed;
		atom_t result;
		const char *end = input;

		// Check if we've parsed all of the statements in the expression.
		while (*end != '\0') {
			// Parse the user's input.
			err = bamboo_parse_expr(end, &end, &parsed);
			IF_BAMBOO_ERROR(err) {
				// Show the error message.
				bamboo_print_error(err);
				fprintf(stderr, "\n");

				continue;
			}

			// Evaluate the parsed expression.
			err = bamboo_eval_expr(parsed, env, &result);
			IF_BAMBOO_ERROR(err) {
				bamboo_print_error(err);
				fprintf(stderr, "\n");

				continue;
			}
		}

		// Print the last evaluated result and prompt the user for more.
		bamboo_print_expr(result);
		printf("\n> ");
	}

	// Quit.
	free(input);
	err = bamboo_destroy(&env);
	printf("Bye!" LINEBREAK);

	return err;
}

/**
 * Simple example of how to create a built-in function.
 *
 * (quit [retval])
 *
 * @param  args   List of arguments passed to the function.
 * @param  result Return atom of calling the function.
 * @return        BAMBOO_OK if the call was successful, otherwise check the
 *                bamboo_error_t enum.
 */
bamboo_error_t builtin_quit(atom_t args, atom_t *result) {
	atom_t arg1;
	int retval = 0;

	// Just set the return to nil since we wont use it.
	*result = nil;

	// Check if we don't have any arguments.
	if (nilp(args)) {
		printf("Quitting from a custom built-in function." LINEBREAK);
		retval = 0;

		goto destroy;
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
	printf("Quitting from a custom built-in function with return value %lld."
		LINEBREAK, arg1.value.integer);
	retval = (int)arg1.value.integer;

destroy:
	bamboo_destroy(NULL);
	exit(retval);
	return BAMBOO_OK;
}
