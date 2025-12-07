/**
 * primitives.h
 * Primitive types of our Lisp dialect.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef BAMBOO_LISP_PRIMITIVES_H
#define BAMBOO_LISP_PRIMITIVES_H

#include <stdint.h>
#include "bamboo_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

atom_t bamboo_int(int64_t num);
atom_t bamboo_float(long double num);
atom_t bamboo_symbol(const char *name);
atom_t bamboo_boolean(bool value);
atom_t bamboo_string(const char *str);
atom_t bamboo_builtin(builtin_func_t func);
bamboo_error_t bamboo_closure(env_t env, atom_t args, atom_t body,
                              atom_t *result);
atom_t bamboo_pointer(void *pointer);

#ifdef __cplusplus
}
#endif

#endif  // BAMBOO_LISP_PRIMITIVES_H
