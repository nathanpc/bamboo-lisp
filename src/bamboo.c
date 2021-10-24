/**
 * bamboo.c
 * The amazingly embeddable Lisp.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "bamboo.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Private variables.
static atom_t symbol_table = { ATOM_TYPE_NIL };

// Private methods.
void putstr(const char *str);

/**
 * Builds an integer atom.
 *
 * @param  num Integer number.
 * @return     Integer atom.
 */
atom_t bamboo_int(long num) {
    atom_t atom;

    // Populate the atom.
    atom.type = ATOM_TYPE_INTEGER;
    atom.value.integer = num;

    return atom;
}

/**
 * Builds an symbol atom.
 *
 * @param  name Symbol name.
 * @return      Symbol atom.
 */
atom_t bamboo_symbol(const char *name) {
    atom_t atom;
    atom_t tmp;

    // Check if the symbol already exists in the symbol table.
    tmp = symbol_table;
    while (!nilp(tmp)) {
        atom = car(tmp);
        if (strcmp(atom.value.symbol, name) == 0)
            return atom;

        tmp = cdr(tmp);
    }

    // Create the new symbol atom.
    atom.type = ATOM_TYPE_SYMBOL;
    atom.value.symbol = strdup(name);

    // Prepend the symbol atom to the symbol table and return the atom.
    symbol_table = cons(atom, symbol_table);
    return atom;
}

/**
 * Builds a pair atom from two other atoms.
 *
 * @param  _car Left-hand side of the atom pair.
 * @param  _cdr Right-hand side of the atom pair.
 * @return      Atom pair.
 */
atom_t cons(atom_t _car, atom_t _cdr) {
    atom_t pair;

    // Setup the pair atom.
    pair.type = ATOM_TYPE_PAIR;
    pair.value.pair = (pair_t *)malloc(sizeof(pair_t));

    // Populate the pair.
    car(pair) = _car;
    cdr(pair) = _cdr;

    return pair;
}

/**
 * Prints the contents of an atom in a standard way.
 *
 * @param atom Atom to have its contents printed.
 */
void bamboo_print_expr(atom_t atom) {
    switch (atom.type) {
    case ATOM_TYPE_NIL:
        putstr("nil");
        break;
    case ATOM_TYPE_SYMBOL:
        printf("%s", atom.value.symbol);
        break;
    case ATOM_TYPE_INTEGER:
        printf("%ld", atom.value.integer);
        break;
    case ATOM_TYPE_PAIR:
        putchar('(');
        bamboo_print_expr(car(atom));
        atom = cdr(atom);

        // Iterate over the right-hand side of the pair since it may be a list.
        while (!nilp(atom)) {
            // Check if we are in a list.
            if (atom.type == ATOM_TYPE_PAIR) {
                putchar(' ');
                bamboo_print_expr(car(atom));
                atom = cdr(atom);
            } else {
                // It was just a simple pair.
                putstr(" . ");
                bamboo_print_expr(atom);
                break;
            }
        }

        putchar(')');
        break;
    }
}

/**
 * Prints a string to stdout. Just like puts but without the newline.
 *
 * @param str String to be printed.
 */
void putstr(const char *str) {
    const char *tmp = str;

    while (*tmp)
        putchar(*tmp++);
}
