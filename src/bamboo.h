/**
 * bamboo.h
 * The amazingly embeddable Lisp.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef _BAMBOO_H
#define _BAMBOO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* Library export prefix definition. */
#ifdef LIBRARY_EXPORTS
	#ifdef _WIN32
		#define BAMBOO_API __declspec(dllexport)
	#else
		#define BAMBOO_API extern
	#endif /* _WIN32 */
#else
	#ifdef _WIN32
		#define BAMBOO_API __declspec(dllimport)
	#else
		#define BAMBOO_API
	#endif /* _WIN32 */
#endif /* LIBRARY_EXPORTS */

// Very basics.
typedef char   TCHAR;
#define _T(x)  x

// Make sure we use the right format specifier for printf and wprintf.
#define SPEC_CHR _T("%c")
#define SPEC_STR _T("%s")

// Global definitions.
#ifndef LINEBREAK
	#define LINEBREAK _T("\r\n")
#endif  // LINEBREAK
#ifndef GC_ITER_COUNT_SWEEP
	#define GC_ITER_COUNT_SWEEP 10000
#endif  // GC_ITER_COUNT_SWEEP

// Error checking macros.
#define IF_BAMBOO_ERROR(err)        if ((err) > BAMBOO_OK)
#define IF_BAMBOO_SPECIAL_COND(err) if ((err) < BAMBOO_OK)

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
		TCHAR **symbol;
		TCHAR **str;
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

// Universal atoms.
static const atom_t nil = { ATOM_TYPE_NIL };

// Core list manipulation.
#define car(p)	   ((p).value.pair->atom[0])
#define cdr(p)	   ((p).value.pair->atom[1])
#define nilp(atom) ((atom).type == ATOM_TYPE_NIL)
BAMBOO_API atom_t cons(atom_t _car, atom_t _cdr);
BAMBOO_API bool listp(atom_t expr);
BAMBOO_API bamboo_error_t apply(atom_t func, atom_t args, atom_t *result);

// Initialization and destruction.
BAMBOO_API bamboo_error_t bamboo_init(env_t *env);
BAMBOO_API bamboo_error_t bamboo_destroy(env_t *env);

// Environment.
BAMBOO_API env_t bamboo_env_new(env_t parent);
BAMBOO_API bamboo_error_t bamboo_env_get(env_t env, atom_t symbol, atom_t *atom);
BAMBOO_API bamboo_error_t bamboo_env_set(env_t env, atom_t symbol, atom_t value);
BAMBOO_API bamboo_error_t bamboo_env_set_builtin(env_t env, const TCHAR *name,
												 builtin_func_t func);
BAMBOO_API env_t *bamboo_get_root_env(void);

// Primitive creation.
BAMBOO_API atom_t bamboo_int(int64_t num);
BAMBOO_API atom_t bamboo_float(long double num);
BAMBOO_API atom_t bamboo_symbol(const TCHAR *name);
BAMBOO_API atom_t bamboo_boolean(bool value);
BAMBOO_API atom_t bamboo_string(const TCHAR *str);
BAMBOO_API atom_t bamboo_builtin(builtin_func_t func);
BAMBOO_API bamboo_error_t bamboo_closure(env_t env, atom_t args, atom_t body,
										 atom_t *result);
BAMBOO_API atom_t bamboo_pointer(void *pointer);

// Parsing and evaluation.
BAMBOO_API bamboo_error_t bamboo_parse_expr(const TCHAR *input, const TCHAR **end,
											atom_t *atom);
BAMBOO_API bamboo_error_t bamboo_eval_expr(atom_t expr, env_t env, atom_t *result);

// Error handling.
BAMBOO_API const TCHAR *bamboo_error_detail(void);
BAMBOO_API bamboo_error_t bamboo_error(bamboo_error_t err, const TCHAR *msg);

// List manipulation.
BAMBOO_API uint16_t bamboo_list_count(atom_t list);
BAMBOO_API atom_t bamboo_list_ref(atom_t list, uint16_t index);
BAMBOO_API void bamboo_list_set(atom_t list, uint16_t index, atom_t value);
BAMBOO_API void bamboo_list_reverse(atom_t *list);

// Debugging.
BAMBOO_API void bamboo_error_type_str(TCHAR **buf, bamboo_error_t err);
BAMBOO_API void bamboo_print_error(bamboo_error_t err);
BAMBOO_API void bamboo_expr_str(TCHAR **buf, atom_t atom);
BAMBOO_API void bamboo_print_expr(atom_t atom);
BAMBOO_API void bamboo_print_tokens(const TCHAR *str);

#ifdef __cplusplus
}
#endif

#endif  // _BAMBOO_H
