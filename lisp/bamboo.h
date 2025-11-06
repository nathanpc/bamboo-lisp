/**
 * bamboo.h
 * A small and purpose-built Lisp dialect focused on scientific problem solving.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef BAMBOO_LISP_BAMBOO_H
#define BAMBOO_LISP_BAMBOO_H

#include "bamboo_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

// Global definitions.
#ifndef GC_ITER_COUNT_SWEEP
	#define GC_ITER_COUNT_SWEEP 10000
#endif  // GC_ITER_COUNT_SWEEP

// Error checking macros.
#define IF_BAMBOO_ERROR(err)        if ((err) > BAMBOO_OK)
#define IF_BAMBOO_SPECIAL_COND(err) if ((err) < BAMBOO_OK)

// Universal atoms.
static const atom_t nil = { ATOM_TYPE_NIL };

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

// Environment.
env_t bamboo_env_new(env_t parent);
bamboo_error_t bamboo_env_get(env_t env, atom_t symbol, atom_t *atom);
bamboo_error_t bamboo_env_set(env_t env, atom_t symbol, atom_t value);
bamboo_error_t bamboo_env_set_builtin(env_t env, const char *name,
                                      builtin_func_t func);
env_t *bamboo_get_root_env(void);

// Primitive creation.
atom_t bamboo_int(int64_t num);
atom_t bamboo_float(long double num);
atom_t bamboo_symbol(const char *name);
atom_t bamboo_boolean(bool value);
atom_t bamboo_string(const char *str);
atom_t bamboo_builtin(builtin_func_t func);
bamboo_error_t bamboo_closure(env_t env, atom_t args, atom_t body,
                              atom_t *result);
atom_t bamboo_pointer(void *pointer);

// Parsing and evaluation.
bamboo_error_t bamboo_parse_expr(const char *input, const char **end,
                                 atom_t *atom);
bamboo_error_t bamboo_eval_expr(atom_t expr, env_t env, atom_t *result);

// Error handling.
const char *bamboo_error_detail(void);
bamboo_error_t bamboo_error(bamboo_error_t err, const char *msg);

// List manipulation.
uint16_t bamboo_list_count(atom_t list);
atom_t bamboo_list_ref(atom_t list, uint16_t index);
void bamboo_list_set(atom_t list, uint16_t index, atom_t value);
void bamboo_list_reverse(atom_t *list);

// Debugging.
void bamboo_error_type_str(char **buf, bamboo_error_t err);
void bamboo_print_error(bamboo_error_t err);
void bamboo_expr_str(char **buf, atom_t atom);
void bamboo_print_expr(atom_t atom);
void bamboo_print_tokens(const char *str);

#ifdef __cplusplus
}
#endif

#endif  // BAMBOO_LISP_BAMBOO_H
