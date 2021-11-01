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

#define TEST_LEN 8

/**
 * Program's main entry point.
 *
 * @return 0 if everything went fine.
 */
int main(void) {
	int i;
	const char *str[TEST_LEN];
	str[0] = "(foo bar)";
	str[1] = "(foo . bar)";
	str[2] = "(foo bar baz)";
	str[3] = "(foo bar . baz)";
	str[4] = "(test (foo . bar) hello)";
	str[5] = "(foo . (bar . nil) baz)";
	str[6] = "(foo (bar baz (test . nil) some (ot . her) thing) another)";
	str[7] = "(s (t . u) v . (w . nil))";

	printf("Bamboo LISP v0.1a" LINEBREAK LINEBREAK);

	for (i = 0; i < TEST_LEN; i++) {
		atom_t atom;
		bamboo_error_t err;

		printf("> %s" LINEBREAK, str[i]);

		bamboo_print_tokens(str[i]);
		printf(LINEBREAK);

		err = parse_expr(str[i], &str[i], &atom);
		if (err)
			bamboo_print_error(err);
		bamboo_print_expr(atom);
		printf(LINEBREAK);
		printf(LINEBREAK);
	}

	system("pause");
	return 0;
}
