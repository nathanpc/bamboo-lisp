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

/**
 * Program's main entry point.
 *
 * @return 0 if everything went fine.
 */
int main(void) {
    printf("Bamboo LISP v0.1a" LINEBREAK);

    bamboo_print_expr(bamboo_int(1234));
    printf(LINEBREAK);
    bamboo_print_expr(bamboo_symbol("asymbol"));
    printf(LINEBREAK);
    bamboo_print_expr(cons(bamboo_symbol("foo"), bamboo_symbol("bar")));
    printf(LINEBREAK);
    bamboo_print_expr(cons(bamboo_int(1),
        cons(bamboo_int(2), cons(bamboo_int(3), nil))));
    printf(LINEBREAK);

    printf("Press any key to continue..." LINEBREAK);
    getch();
    return 0;
}
