/**
 * strutils.h
 * Some utility functions to help us play around with strings.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef REPL_STRUTILS_H
#define REPL_STRUTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <wchar.h>
#include "../src/bamboo.h"

// Conversion
char *trunc_wchar(const wchar_t *str);

#ifdef __cplusplus
}
#endif

#endif  // REPL_STRUTILS_H
