/**
 * functions.h
 * Built-in functions specially for the REPL.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef REPL_FUNCTIONS_H
#define REPL_FUNCTIONS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../src/bamboo.h"

// Custom error codes.
typedef enum {
	BAMBOO_REPL_QUIT = 100
} bamboo_custom_error_t;

// Automatically add all of our built-ins.
bamboo_error_t repl_populate_builtins(env_t *env);

#ifdef __cplusplus
}
#endif

#endif  // REPL_FUNCTIONS_H