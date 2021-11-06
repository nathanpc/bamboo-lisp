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
	BAMBOO_PAREN_END,
	BAMBOO_ERROR_SYNTAX,
	BAMBOO_ERROR_UNBOUND,
	BAMBOO_ERROR_ARGUMENTS,
	BAMBOO_ERROR_WRONG_TYPE,
	BAMBOO_ERROR_UNKNOWN
} bamboo_error_t;

// Atom types.
typedef enum {
	ATOM_TYPE_NIL,
	ATOM_TYPE_PAIR,
	ATOM_TYPE_SYMBOL,
	ATOM_TYPE_INTEGER,
	ATOM_TYPE_FLOAT,
	ATOM_TYPE_BUILTIN
} atom_type_t;

// Atom structures typedefs.
typedef struct pair_s pair_t;
typedef struct atom_s atom_t;
typedef atom_t env_t;

// Built-in function prototype typedef.
typedef bamboo_error_t (*builtin_func_t)(atom_t args, atom_t *result);

// Atom structure.
struct atom_s {
	atom_type_t type;
	union {
		pair_t *pair;
		const char *symbol;
		long integer;
		double dfloat;
		builtin_func_t builtin;
	} value;
};

// Atom pair structure.
struct pair_s {
	atom_t atom[2];
};

// Universal atoms.
static const atom_t nil = { ATOM_TYPE_NIL };

// Core list manipulation.
#define car(p)	   ((p).value.pair->atom[0])
#define cdr(p)	   ((p).value.pair->atom[1])
#define nilp(atom) ((atom).type == ATOM_TYPE_NIL)
atom_t cons(atom_t _car, atom_t _cdr);
bool listp(atom_t expr);
bamboo_error_t apply(atom_t func, atom_t args, atom_t *result);

// Initialization.
bamboo_error_t bamboo_init(env_t *env);

// Environment.
env_t bamboo_env_new(env_t parent);
bamboo_error_t bamboo_env_get(env_t env, atom_t symbol, atom_t *atom);
bamboo_error_t bamboo_env_set(env_t env, atom_t symbol, atom_t value);
bamboo_error_t bamboo_env_set_builtin(env_t env, const char *name,
									  builtin_func_t func);

// Primitive creation.
atom_t bamboo_int(long num);
atom_t bamboo_float(double num);
atom_t bamboo_symbol(const char *name);
atom_t bamboo_builtin(builtin_func_t func);

// Parsing and evaluation.
bamboo_error_t parse_expr(const char *input, const char **end,
						  atom_t *atom);
bamboo_error_t bamboo_eval_expr(atom_t expr, env_t env, atom_t *result);

// Debugging.
const char* bamboo_error_detail(void);
void bamboo_print_error(bamboo_error_t err);
void bamboo_print_expr(atom_t atom);
void bamboo_print_tokens(const char *str);

#endif	// _BAMBOO_H
