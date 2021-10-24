/**
 * bamboo.h
 * The amazingly embeddable Lisp.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef _BAMBOO_H
#define _BAMBOO_H

#include <stdint.h>
#include <stdbool.h>

// Global definitions.
#define LINEBREAK "\r\n"

// Parser return values.
typedef enum {
    BAMBOO_OK = 0,
    BAMBOO_ERROR_SYNTAX
} bamboo_error_t;

// Atom types.
typedef enum {
    ATOM_TYPE_NIL,
    ATOM_TYPE_PAIR,
    ATOM_TYPE_SYMBOL,
    ATOM_TYPE_INTEGER
} atom_type_t;

// Atom structure.
typedef struct pair_s pair_t;
typedef struct atom_s atom_t;
struct atom_s {
    atom_type_t type;
    union {
        pair_t *pair;
        const char *symbol;
        long integer;
    } value;
};

// Atom pair structure.
struct pair_s {
    atom_t atom[2];
};

// Universal atoms.
static const atom_t nil = { ATOM_TYPE_NIL };

// Core list manipulation.
#define car(p)     ((p).value.pair->atom[0])
#define cdr(p)     ((p).value.pair->atom[1])
#define nilp(atom) ((atom).type == ATOM_TYPE_NIL)
atom_t cons(atom_t _car, atom_t _cdr);

// Primitive creation.
atom_t bamboo_int(long num);
atom_t bamboo_symbol(const char *name);

// Debugging.
void bamboo_print_expr(atom_t atom);

#endif  // _BAMBOO_H
