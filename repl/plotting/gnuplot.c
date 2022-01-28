/**
 * gnuplot.c
 * A GNUplot subsystem for our plotting library.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "gnuplot.h"
#include "plot.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#ifdef _WIN32
	#include "../windows/winutils.h"
#endif  // _WIN32

// Private methods.
void gnuplot_init(plot_t *plt);

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                         Generic Interface Methods                          //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

plot_t* plot_init(void) {
	plot_t *plt;

	// Allocate space for our plotting handle.
	plt = (plot_t *)malloc(sizeof(plot_t));
	if (plt == NULL)
		goto retplt;

	// Start the GNUplot process.
	gnuplot_init(plt);
	if (plt == NULL)
		goto retplt;

retplt:
	return plt;
}

void plot_destroy(plot_t *plt) {
	// Do we even need to do something?
	if (!plt)
		return;

	// Free the GNUplot process.
#ifdef _WIN32
	if (plt->hndGplotW) {
		CloseHandle(plt->hndGplotW);
	}

	if (plt->hndGplotR) {
		CloseHandle(plt->hndGplotR);
	}
#else
	if (plt->gplot) {
		if (pclose(plt->gplot) == -1) {
			_ftprintf(stderr, _T("An error occured while trying to close ")
					_T("GNUplot's process") LINEBREAK);
		}
	}
#endif  // _WIN32
	
	// Free our plottin handle.
	free(plt);
	plt = NULL;
}

void plot_equation(plot_t *plt, const TCHAR *equation) {
	gnuplot_cmd(plt, _T("plot ") SPEC_STR, equation);
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                         Subsystem Specific Methods                         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/**
 * Initializes the GNUplot process.
 *
 * @param plt Plotting handle.
 */
void gnuplot_init(plot_t *plt) {
#ifdef _WIN32
	SECURITY_ATTRIBUTES saAttr;
	STARTUPINFO siStartInfo;
	TCHAR szCmdline[MAX_PATH];

	// Setup the STDIN pipe.
	saAttr.nLength              = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle       = true;
	saAttr.lpSecurityDescriptor = NULL;

	// Open the STDIN pipe.
	plt->hndGplotR = NULL;
	plt->hndGplotW = NULL;
	if (!CreatePipe(&plt->hndGplotR, &plt->hndGplotW, &saAttr, 0)) {
		_ftprintf(stderr, _T("Couldn't open a pipe for GNUplot's STDIN")
			LINEBREAK);
		DisplayError(_T("CreatePipe"));

		plot_destroy(plt);
		return;
	}

	// Ensure the the pipe for STDIN is not inherited.
	if (!SetHandleInformation(plt->hndGplotW, HANDLE_FLAG_INHERIT, 0)) {
		_ftprintf(stderr, _T("Couldn't set the STDIN pipe inheritance")
			LINEBREAK);
		DisplayError(_T("SetHandleInformation"));

		plot_destroy(plt);
		return;
	}

	// Setup the process structures.
	ZeroMemory(&plt->piProcInfo, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.hStdError   = GetStdHandle(STD_ERROR_HANDLE);
	siStartInfo.hStdOutput  = GetStdHandle(STD_OUTPUT_HANDLE);
	siStartInfo.hStdInput   = plt->hndGplotW;
	siStartInfo.dwFlags    |= STARTF_USESTDHANDLES;

	// Create the GNUplot child process.
	_tcscpy(szCmdline, _T("gnuplot"));
	if (!CreateProcess(NULL, szCmdline, NULL, NULL, true, 0, NULL,
			NULL, &siStartInfo, &plt->piProcInfo)) {
		_ftprintf(stderr, _T("Couldn't open a new GNUplot process. Make sure ")
			_T("gnuplot is in your system PATH") LINEBREAK);
		DisplayError(_T("CreateProcess"));

		plot_destroy(plt);
		return;
	}

	// Close some unused handles.
	CloseHandle(plt->piProcInfo.hProcess);
	CloseHandle(plt->piProcInfo.hThread);
	CloseHandle(plt->hndGplotR);
#else
	// Start a new GNUplot process.
	plt->gplot = popen("gnuplot", "w");
	if (plt->gplot == NULL) {
		_ftprintf(stderr, _T("Couldn't open a new GNUplot process. Make sure ")
				_T("gnuplot is in your system PATH") LINEBREAK);

		plot_destroy(plt);
		return;
	}
#endif  // _WIN32
}

/**
 * Sends a raw command to the GNUplot process.
 *
 * @param plt Plotting handle.
 * @param cmd Command to be executed by a GNUplot instance (with printf format
 *            specifiers).
 * @param ... Variables to be substituted in place of the format specifiers.
 */
void gnuplot_cmd(plot_t *plt, const TCHAR *cmd, ...) {
	va_list ap;
#ifdef _WIN32
	DWORD dwWritten;
	LPTSTR lpszBuffer;
#endif  // _WIN32

	// Check if we have a valid handle.
	if (plt == NULL) {
		_ftprintf(stderr, _T("Trying to write a command to an invalid handle")
			LINEBREAK);
		return;
	}

	// Perform a printf command to GNUplot.
	va_start(ap, cmd);
#ifdef _WIN32
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
		cmd, 0, 0, (LPTSTR)&lpszBuffer, 0, &ap);
#else
	_vftprintf(plt->gplot, cmd, ap);
#endif  // _WIN32
	va_end(ap);

#ifdef _WIN32
	// Write the buffer to the handle.
	if (!WriteFile(plt->hndGplotW, lpszBuffer, _tcslen(lpszBuffer), &dwWritten, NULL)) {
		DisplayError(_T("WriteFile command"));
	}

	// Make sure we flush the command we've just sent.
	LocalFree(lpszBuffer);
	if (!WriteFile(plt->hndGplotW, _T("\n"), 1, NULL, NULL)) {
		DisplayError(_T("WriteFile flush"));
	}
#endif  // _WIN32

	// Make sure we flush the command we've just sent.
#ifndef _WIN32
	_fputts(_T("\n"), plt->gplot);
	fflush(plt->gplot);
#endif  // !_WIN32
}
