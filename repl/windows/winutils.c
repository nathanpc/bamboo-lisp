/**
 * winutils.c
 * Some utility functions to help us cope with the Windows nightmare.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifdef _WIN32

#include "winutils.h"
#include <strsafe.h>
#include <tchar.h>

/**
 * Displays an internal Windows API error that might have occured.
 *
 * @param lpszHint A little hint to add to the message.
 */
void DisplayError(LPCTSTR lpszHint) {
	LPTSTR lpszMessage;
	LPTSTR lpszDisplay;
	DWORD dwErrorCode = GetLastError();

	// Get the error message.
	lpszMessage = NULL;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwErrorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpszMessage, 0, NULL);

	// Allocate and construct our display message.
	lpszDisplay = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,
		(_tcslen(lpszMessage) + _tcslen(lpszHint) + 40) * sizeof(TCHAR));
	StringCchPrintf(lpszDisplay, LocalSize((LPVOID)lpszDisplay) / sizeof(TCHAR),
		_T("%s failed with error %d: %s"),
		lpszHint, dwErrorCode, lpszMessage);
	MessageBox(NULL, lpszDisplay, _T("Windows API Error"),
		MB_OK | MB_ICONEXCLAMATION);

	// Clean up this mess.
	LocalFree(lpszMessage);
	LocalFree(lpszDisplay);
}

#endif  // _WIN32
