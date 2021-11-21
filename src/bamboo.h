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

// Unicode support.
#ifdef _MSC_VER
#include <tchar.h>
#else
#define UNICODE
#include <wchar.h>
typedef wchar_t TCHAR;
#endif

#ifndef _T
#ifdef UNICODE
#define _T(x) L ## x
#else
#define _T(x) x
#endif
#endif

// Global definitions.
#ifndef LINEBREAK
#define LINEBREAK _T("\r\n")
#endif
#ifndef GC_ITER_COUNT_SWEEP
#define GC_ITER_COUNT_SWEEP 10000
#endif

// Parser return values.
typedef enum {
	BAMBOO_PAREN_QUOTE_END = -3,
	BAMBOO_PAREN_END = -2,
	BAMBOO_QUOTE_END = -1,
	BAMBOO_OK = 0,
	BAMBOO_ERROR_SYNTAX,
	BAMBOO_ERROR_UNBOUND,
	BAMBOO_ERROR_ARGUMENTS,
	BAMBOO_ERROR_WRONG_TYPE,
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
	ATOM_TYPE_MACRO
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
		const TCHAR *symbol;
		TCHAR **str;
		long integer;
		double dfloat;
		bool boolean;
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
bamboo_error_t bamboo_env_set_builtin(env_t env, const TCHAR *name,
									  builtin_func_t func);

// Primitive creation.
atom_t bamboo_int(long num);
atom_t bamboo_float(double num);
atom_t bamboo_symbol(const TCHAR *name);
atom_t bamboo_boolean(bool value);
atom_t bamboo_string(const TCHAR *str);
atom_t bamboo_builtin(builtin_func_t func);
bamboo_error_t bamboo_closure(env_t env, atom_t args, atom_t body,
		atom_t *result);


// Parsing and evaluation.
bamboo_error_t bamboo_parse_expr(const TCHAR *input, const TCHAR **end,
						  atom_t *atom);
bamboo_error_t bamboo_eval_expr(atom_t expr, env_t env, atom_t *result);

// Error handling.
const TCHAR *bamboo_error_detail(void);
bamboo_error_t bamboo_error(bamboo_error_t err, const TCHAR *msg);

// Debugging.
void bamboo_print_error(bamboo_error_t err);
void bamboo_print_expr(atom_t atom);
void bamboo_print_tokens(const TCHAR *str);

#endif	// _BAMBOO_H
