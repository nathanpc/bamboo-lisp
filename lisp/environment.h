/**
 * environment.h
 * Manages the environment of the interpreter, keeping track of its state,
 * managing all symbols and definitions.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef BAMBOO_LISP_ENVIRONMENT_H
#define BAMBOO_LISP_ENVIRONMENT_H

#include "bamboo_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

env_t bamboo_env_new(env_t parent);
bamboo_error_t bamboo_env_get(env_t env, atom_t symbol, atom_t *atom);
bamboo_error_t bamboo_env_set(env_t env, atom_t symbol, atom_t value);
bamboo_error_t bamboo_env_set_builtin(env_t env, const char *name,
                                      builtin_func_t func);
env_t *bamboo_get_root_env(void);

#ifdef __cplusplus
}
#endif

#endif  // BAMBOO_LISP_ENVIRONMENT_H
