/**
 * builtins.h
 * Core functions of the language written in C instead of Lisp.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef BAMBOO_LISP_BUILTINS_H
#define BAMBOO_LISP_BUILTINS_H

#include "bamboo_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

// Built-ins initialization.
bamboo_error_t populate_builtins(env_t *env);

// List operations.
bamboo_error_t builtin_car(atom_t args, atom_t *result);
bamboo_error_t builtin_cdr(atom_t args, atom_t *result);
bamboo_error_t builtin_cons(atom_t args, atom_t *result);

// Mathematics
bamboo_error_t builtin_sum(atom_t args, atom_t *result);
bamboo_error_t builtin_subtract(atom_t args, atom_t *result);
bamboo_error_t builtin_multiply(atom_t args, atom_t *result);
bamboo_error_t builtin_divide(atom_t args, atom_t *result);
bamboo_error_t builtin_expt(atom_t args, atom_t *result);
bamboo_error_t builtin_modulo(atom_t args, atom_t *result);
bamboo_error_t builtin_floor(atom_t args, atom_t *result);
bamboo_error_t builtin_round(atom_t args, atom_t *result);
bamboo_error_t builtin_ceil(atom_t args, atom_t *result);

// Boolean operations.
bamboo_error_t builtin_not(atom_t args, atom_t *result);
bamboo_error_t builtin_and(atom_t args, atom_t *result);
bamboo_error_t builtin_or(atom_t args, atom_t *result);
bamboo_error_t builtin_eq(atom_t args, atom_t *result);
bamboo_error_t builtin_numeq(atom_t args, atom_t *result);
bamboo_error_t builtin_lt(atom_t args, atom_t *result);
bamboo_error_t builtin_gt(atom_t args, atom_t *result);

// Type checking.
bamboo_error_t builtin_nilp(atom_t args, atom_t *result);
bamboo_error_t builtin_pairp(atom_t args, atom_t *result);
bamboo_error_t builtin_symbolp(atom_t args, atom_t *result);
bamboo_error_t builtin_integerp(atom_t args, atom_t *result);
bamboo_error_t builtin_floatp(atom_t args, atom_t *result);
bamboo_error_t builtin_numericp(atom_t args, atom_t *result);
bamboo_error_t builtin_booleanp(atom_t args, atom_t *result);
bamboo_error_t builtin_builtinp(atom_t args, atom_t *result);
bamboo_error_t builtin_closurep(atom_t args, atom_t *result);
bamboo_error_t builtin_macrop(atom_t args, atom_t *result);

// Display and string operations.
bamboo_error_t builtin_display(atom_t args, atom_t *result);
bamboo_error_t builtin_concat(atom_t args, atom_t *result);
bamboo_error_t builtin_newline(atom_t args, atom_t *result);

// Debugging
bamboo_error_t builtin_display_env(atom_t args, atom_t *result);

#ifdef __cplusplus
}
#endif

#endif  // BAMBOO_LISP_BUILTINS_H
