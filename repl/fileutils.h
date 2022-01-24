/**
 * fileutils.h
 * Some utility functions to help us with files and paths.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef REPL_FILEUTILS_H
#define REPL_FILEUTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../src/bamboo.h"
#include <sys/types.h>
#include <stddef.h>
#include <stdbool.h>

// Checking.
bool file_exists(const TCHAR *fpath);
bool file_ext_match(const TCHAR *fpath, const TCHAR *ext);

// Path manipulaton.
size_t cleanup_path(TCHAR *path);
TCHAR *extcat(const TCHAR *fpath, const TCHAR *ext);

// File content.
size_t file_contents_size(const TCHAR *fname);
TCHAR *slurp_file(const TCHAR *fname);

#ifdef __cplusplus
}
#endif

#endif  // REPL_FILEUTILS_H