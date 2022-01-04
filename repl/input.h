/**
 * input.h
 * Handles the input of expressions in the REPL.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef REPL_INPUT_H
#define REPL_INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../src/bamboo.h"
#include <stddef.h>

void repl_init(void);
int repl_readline(TCHAR *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif  // REPL_INPUT_H