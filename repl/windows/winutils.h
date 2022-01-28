/**
 * winutils.h
 * Some utility functions to help us cope with the Windows nightmare.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef REPL_WINUTILS_H
#define REPL_WINUTILS_H
#ifdef _WIN32

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

// Error handling.
void DisplayError(LPCTSTR lpszHint);

#ifdef __cplusplus
}
#endif

#endif  // _WIN32
#endif  // REPL_WINUTILS_H
