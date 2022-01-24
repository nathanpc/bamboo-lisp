/**
 * fileutils.h
 * Some utility functions to help us with files and paths.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "fileutils.h"
#include <stdio.h>
#include <stdlib.h>
#include "strutils.h"

 /**
  * Checks if a file exists.
  *
  * @param  fpath File path to be checked.
  * @return       TRUE if the file exists.
  */
bool file_exists(const TCHAR *fpath) {
#ifdef _WIN32
	DWORD dwAttrib;

	// Get file attributes and return.
	dwAttrib = GetFileAttributes(fpath);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
#ifdef UNICODE
	char *buf;
	bool exists = false;

	// Get the regular string version of the file path.
	buf = trunc_wchar(fpath);
	if (buf == NULL)
		return false;

	// Check if the file actually exists.
	exists = (access(buf, F_OK) != -1);

	// Clean up and return.
	free(buf);
	return exists;
#else
	return access(fpath, F_OK) != -1;
#endif  // UNICODE
#endif  // _WIN32
}

/**
 * Checks if a file extension is the same as the one specified.
 *
 * @param  fpath File path to be checked.
 * @param  ext   Desired file extension.
 * @return       TRUE if the file has the desired extension.
 */
bool file_ext_match(const TCHAR *fpath, const TCHAR *ext) {
	const TCHAR *fext;
	int i;

	// Go through the file path backwards trying to find a dot.
	fext = fpath;
	for (i = (_tcslen(fpath) - 1); i >= 0; i--) {
		if (fpath[i] == _T('.')) {
			fext = fpath + i + 1;
			break;
		}
	}

	return _tcscmp(fext, ext) == 0;
}

/**
 * Cleans up a path string and converts it to Windows separators if necessary.
 *
 * @param  path File path string.
 * @return      Final size of the cleaned up path.
 */
size_t cleanup_path(TCHAR *path) {
	TCHAR *pos;
	TCHAR *back;

	// Go through string searching for duplicate slashes.
	while ((pos = _tcsstr(path, _T("//"))) != NULL) {
		// Append the rest of the string skipping one character.
		for (back = pos++; *back != _T('\0'); back++) {
			*back = *pos++;
		}
	}

#ifdef _WIN32
	// Fix strings that have a mix of Windows and UNIX slashes together.
	while ((pos = _tcsstr(path, _T("\\/"))) != NULL) {
		// Append the rest of the string skipping one character.
		for (back = pos++; *back != _T('\0'); back++) {
			*back = *pos++;
		}
	}

	// Convert UNIX path separators to Windows.
	pos = path;
	for (; *pos != _T('\0'); pos++) {
		if (*pos == _T('/'))
			*pos = _T('\\');
	}
#endif  // _WIN32

	return _tcslen(path);
}

/**
 * Concatenates an extension to a file path.
 * WARNING: This function allocates its return string, so you're responsible
 *          for freeing it.
 *
 * @param  fpath Path to the file without extension.
 * @param  ext   File extension without the dot.
 * @return       Path to the file with extension appended.
 */
TCHAR* extcat(const TCHAR *fpath, const TCHAR *ext) {
	TCHAR *final_path;

	// Allocate the final path string.
	final_path = (TCHAR *)malloc((_tcslen(fpath) + _tcslen(ext) + 2) *
		sizeof(TCHAR));
	if (final_path == NULL)
		return NULL;

	// Concatenate things.
	_stprintf(final_path, SPEC_STR _T(".") SPEC_STR, fpath, ext);

	return final_path;
}

/**
 * Gets the size of a buffer to hold the whole contents of a file.
 *
 * @param  fname File path.
 * @return       Size of the content of specified file or 0 if an error occured.
 */
size_t file_contents_size(const TCHAR *fname) {
	FILE *fh;
	size_t nbytes;
#ifdef UNICODE
	char *tmp;

	// Get the regular string version of the file path.
	tmp = trunc_wchar(fname);
	if (tmp == NULL)
		return 0;

	// Open file.
	fh = fopen(tmp, "r");
#else
	// Open file.
	fh = fopen(fname, "r");
#endif  // UNICODE
	if (fh == NULL)
		return 0L;

	// Seek file to determine its size.
	fseek(fh, 0L, SEEK_END);
	nbytes = ftell(fh);

#ifdef UNICODE
	// Free our temporary conversion buffer.
	free(tmp);
#endif  // UNICODE

	// Close the file handler and return.
	fclose(fh);
	return nbytes;
}

/**
 * Reads a whole file and stores it into a string.
 * WARNING: Remember to free the returned string allocated by this function.
 *
 * @param  fname File path.
 * @return       String where the file contents are to be stored (will be
 *               allocated by this function).
 */
TCHAR* slurp_file(const TCHAR *fname) {
	FILE *fh;
	size_t fsize;
	int c;
	TCHAR *contents;
	TCHAR *tmp;
#ifdef UNICODE
	char *spath;
#endif  // UNICODE

	// Get file size.
	fsize = file_contents_size(fname);
	if (fsize == 0L)
		return NULL;

	// Allocate the string to hold the contents of the file.
	contents = (TCHAR *)malloc((fsize + 1) * sizeof(TCHAR));
	if (contents == NULL)
		return NULL;

#ifdef UNICODE
	// Get the regular string version of the file path.
	spath = trunc_wchar(fname);
	if (spath == NULL) {
		free(contents);
		return NULL;
	}

	// Open file to read its contents.
	fh = fopen(spath, "r");
#else
	// Open file to read its contents.
	fh = fopen(fname, "r");
#endif  // UNICODE
	if (fh == NULL) {
#ifdef UNICODE
		free(spath);
#endif  // UNICODE
		free(contents);
		return NULL;
	}

#ifdef UNICODE
	// Clean up our temporary file path.
	free(spath);
#endif  // UNICODE

	// Reads the whole file into the buffer.
	tmp = contents;
	c = fgetc(fh);
	while (c != EOF) {
		// Append the character to our contents buffer and advance it.
		*tmp = (TCHAR)c;
		tmp++;

		// Get a character from the file.
		c = fgetc(fh);
	}

	// Close the file handle and make sure our string is properly terminated.
	fclose(fh);
	*tmp = _T('\0');

	return contents;
}
