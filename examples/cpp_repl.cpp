/**
 * Bamboo REPL
 * A simple console REPL for the Bamboo Lisp C++ implementation.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include <string>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include "../src/BambooWrapper.h"

// Private definitions.
#define REPL_INPUT_MAX_LEN 512

// Private methods.
int readline(char *buf, size_t len);
bamboo_error_t builtin_quit(atom_t args, atom_t *result);

/**
 * Program's main entry point.
 *
 * @param  argc Number of command-line arguments passed to the program.
 * @param  argv Command-line arguments passed to the program.
 * @return      0 if everything went fine.
 */
int main(int argc, char *argv[]) {
	char *input;
	Bamboo::Lisp bamboo;

	// Add our own custom built-in function.
	bamboo.env().set_builtin("QUIT", builtin_quit);

	// Allocate memory for the REPL input data.
	input = (char *)malloc(sizeof(char) * (REPL_INPUT_MAX_LEN + 1));
	if (input == NULL) {
		fprintf(stderr, "Can't allocate the input string for the REPL"
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
		} catch (Bamboo::BambooException& e) {
			// Print the error encountered.
			std::cerr << e.what() << std::endl;
		}
	}

	// Quit.
	free(input);
	std::cout << "Bye!" << std::endl;

	return 0;
}

/**
 * Reads the user input like a command prompt.
 *
 * @param  buf Pointer to the buffer that will hold the user input.
 * @param  len Maximum length to read the user input including NULL terminator.
 * @return     Non-zero to break out of the REPL loop.
 */
int readline(char *buf, size_t len) {
	uint16_t i;
	int16_t openparens = 0;
	bool instring = false;

	// Put some safe guards in place.
	buf[0] = '\0';
	buf[len] = '\0';

	// Get user input.
	printf("> ");
	for (i = 0; i < len; i++) {
		// Get character from STDIN.
		int c = getchar();

		switch (c) {
		case '\"':
			// Opening or closing a string.
			instring = !instring;
			break;
		case '(':
			// Opened a parenthesis.
			if (!instring)
				openparens++;
			break;
		case ')':
			// Closed a parenthesis.
			if (!instring)
				openparens--;
			break;
		case '\n':
			// Only return the string if all the parenthesis have been closed.
			if (openparens < 1) {
				buf[i] = '\0';
				goto returnstr;
			}

			printf("  ");
		}

		// Append character to the buffer.
	    buf[i] = (char)c;
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
		printf("Bye!" LINEBREAK);
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

	printf("Bye!" LINEBREAK);
	exit((int)arg1.value.integer);
	return BAMBOO_OK;
}
