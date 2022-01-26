<h1>
	<img src="https://raw.githubusercontent.com/nathanpc/bamboo-lisp/master/assets/icon/icon-512.png" width="64" height="64" />
	Bamboo Lisp
</h1>

An extremely portable and embeddable (single source/header file) Lisp dialect
written in C.


## Included Items

This project contains a bit more than just a library for you to have an
embeddable Lisp language in your application. It includes a full REPL and
interpreter in case you want to use it as a full blown Lisp environment, the
C library itself of course, and C++ wrapper to the C library so that you can
work with it more natively while in a C++ environment.


## Requirements

This project was built with the ability to be super portable in mind, so you
can probably compile it for almost any platform out there. Here's a list of
compilers this project is proven to build in:

  - Open Watcom v2
  - Microsoft Visual C++ 6.0
  - GCC 8 (Linux)
  - Clang 5.1 (Mac OS X)


## Building the REPL

In order to build the complete REPL and interpreter environment under an UNIX
platform all you need to do is just run `make` in the root of the project and
you'll get a `bamboo` binary built in the `build/` directory.

If you're on Windows a Visual Studio solution has been thoughtfully included in
the `windows/` directory. Just open the `Bamboo.sln` file and compile the
project.


## Example Usage

There are some examples included in the `examples` folder, but here's a very
super simple, bare-bones, REPL so that you can get an idea of how easily it is
to integrate the interpreter into your application and also how to extend the
language with your own functions.

```c
#include <stdio.h>
#include <stdlib.h>
#include "bamboo.h"

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
		const char *end;

		// Parse the user's input.
		err = bamboo_parse_expr(input, &end, &parsed);
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

		// Print the evaluated result and prompt the user for more.
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
```


## Inspiration

I've always wanted to write my own Lisp, and after taking a quick look at
[Building LISP](https://lwh.jp/lisp/) I've decided to finally do it.


## License

This project is licensed under the [MIT License](/LICENSE.txt).
