/**
 * bamboo.h
 * A small and purpose-built Lisp dialect focused on scientific problem solving.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef BAMBOO_LISP_BAMBOO_H
#define BAMBOO_LISP_BAMBOO_H

#include "bamboo_defs.h"
#include "primitives.h"
#include "environment.h"
#include "error.h"
#include "builtins.h"

#ifdef __cplusplus
extern "C" {
#endif

// Global definitions.
#ifndef GC_ITER_COUNT_SWEEP
	#define GC_ITER_COUNT_SWEEP 10000
#endif  // GC_ITER_COUNT_SWEEP

// Core list manipulation.
#define car(p)	   ((p).value.pair->atom[0])
#define cdr(p)	   ((p).value.pair->atom[1])
#define nilp(atom) ((atom).type == ATOM_TYPE_NIL)
atom_t cons(atom_t _car, atom_t _cdr);
bool listp(atom_t expr);
bamboo_error_t apply(atom_t func, atom_t args, atom_t *result);

// Initialization and destruction.
bamboo_error_t bamboo_init(env_t *env);
bamboo_error_t bamboo_destroy(env_t *env);

// Parsing and evaluation.
bamboo_error_t bamboo_parse_expr(const char *input, const char **end,
                                 atom_t *atom);
bamboo_error_t bamboo_eval_expr(atom_t expr, env_t env, atom_t *result);

// List manipulation.
uint16_t bamboo_list_count(atom_t list);
atom_t bamboo_list_ref(atom_t list, uint16_t index);
void bamboo_list_set(atom_t list, uint16_t index, atom_t value);
void bamboo_list_reverse(atom_t *list);

// Debugging.
void bamboo_expr_str(char **buf, atom_t atom);
void bamboo_print_expr(atom_t atom);
void bamboo_print_tokens(const char *str);

#ifdef __cplusplus
}
#endif

#endif  // BAMBOO_LISP_BAMBOO_H
