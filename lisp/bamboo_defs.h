/**
 * bamboo_defs.h
 * Global definitions of Bamboo Lisp's internal types.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef BAMBOO_LISP_BAMBOO_DEFS_H
#define BAMBOO_LISP_BAMBOO_DEFS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Parser return values.
typedef enum {
    BAMBOO_PAREN_QUOTE_END = -5,
    BAMBOO_PAREN_END       = -4,
    BAMBOO_QUOTE_END       = -3,
    BAMBOO_COMMENT         = -2,
    BAMBOO_EMPTY_LINE      = -1,
    BAMBOO_OK              = 0,
    BAMBOO_ERROR_SYNTAX,
    BAMBOO_ERROR_EMPTY,
    BAMBOO_ERROR_UNBOUND,
    BAMBOO_ERROR_ARGUMENTS,
    BAMBOO_ERROR_WRONG_TYPE,
    BAMBOO_ERROR_NUM_OVERFLOW,
    BAMBOO_ERROR_NUM_UNDERFLOW,
    BAMBOO_ERROR_ALLOCATION,
    BAMBOO_ERROR_UNKNOWN
} bamboo_error_t;

// Atom types.
typedef enum {
    ATOM_TYPE_NIL,
    ATOM_TYPE_SYMBOL,
    ATOM_TYPE_INTEGER,
    ATOM_TYPE_FLOAT,
    ATOM_TYPE_BOOLEAN,
    ATOM_TYPE_STRING,
    ATOM_TYPE_PAIR,
    ATOM_TYPE_BUILTIN,
    ATOM_TYPE_CLOSURE,
    ATOM_TYPE_MACRO,
    ATOM_TYPE_POINTER
} atom_type_t;

// Atom structures typedefs.
typedef struct pair_s pair_t;
typedef struct atom_s atom_t;
typedef atom_t env_t;

// Built-in function prototype typedef.
// Template: bamboo_error_t func_builtin(atom_t args, atom_t *result);
typedef bamboo_error_t (*builtin_func_t)(atom_t, atom_t*);

// Atom structure.
struct atom_s {
    atom_type_t type;
    union {
        pair_t *pair;
        char **symbol;
        char **str;
        int64_t integer;
        long double dfloat;
        bool boolean;
        builtin_func_t builtin;
        void *pointer;
    } value;
};

// Atom pair structure.
struct pair_s {
    atom_t atom[2];
};

// Universal nil atom.
static const atom_t nil = { ATOM_TYPE_NIL };

#ifdef __cplusplus
}
#endif

#endif //BAMBOO_LISP_BAMBOO_DEFS_H
