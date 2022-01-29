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

	// Setup the defaults.
	plt->pcount = 0;
	*plt->sname = _T('\0');
	plot_set_type(plt, _T("lines"));

retplt:
	return plt;
}

void plot_destroy(plot_t *plt) {
	// Do we even need to do something?
	if (!plt)
		return;

	// Free the GNUplot process.
	if (plt->gplot) {
#ifdef _WIN32
		if (_pclose(plt->gplot) == -1) {
#else
		if (pclose(plt->gplot) == -1) {
#endif  // _WIN32
			_ftprintf(stderr, _T("An error occured while trying to close ")
					_T("GNUplot's process") LINEBREAK);
		}
	}
	
	// Free our plotting handle.
	free(plt);
	plt = NULL;
}

void plot_set_title(plot_t *plt, const TCHAR *title) {
	gnuplot_cmd(plt, _T("set title \"") SPEC_STR _T("\""), title);
}

void plot_set_xlabel(plot_t *plt, const TCHAR *label) {
	gnuplot_cmd(plt, _T("set xlabel \"") SPEC_STR _T("\""), label);
}

void plot_set_ylabel(plot_t *plt, const TCHAR *label) {
	gnuplot_cmd(plt, _T("set ylabel \"") SPEC_STR _T("\""), label);
}

void plot_set_type(plot_t *plt, const TCHAR *type) {
	uint8_t i;

	// Copy the type string while making sure it's lowercase.
	*plt->pstyle = _T('\0');
	for (i = 0; i < GNUPLOT_STYLE_MAX_LEN; i++) {
		// Copy the character as lowercase.
		plt->pstyle[i] = _totlower(type[i]);

		// Are we there yet?
		if (type[i] == _T('\0')) {
			return;
		}
	}

	// Make sure we terminate the string.
	plt->pstyle[GNUPLOT_STYLE_MAX_LEN] = _T('\0');
}

void plot_set_series_name(plot_t *plt, const TCHAR *name) {
	*plt->sname = _T('\0');
	_tcsncat(plt->sname, name, GNUPLOT_TITLE_MAX_LEN);
}

void plot_clear(plot_t *plt) {
	plt->pcount = 0;
	gnuplot_cmd(plt, _T("clear"));
}

void plot_equation(plot_t *plt, const TCHAR *equation) {
	// Build the plot/replot command.
	gnuplot_cmd(plt, SPEC_STR _T(" ") SPEC_STR _T(" title \"") SPEC_STR
		_T("\" with ") SPEC_STR,
		(plt->pcount == 0) ? _T("plot") : _T("replot"), equation,
		(*plt->sname == _T('\0')) ? equation : plt->sname, plt->pstyle);

	// Increment the plot count.
	plt->pcount++;
}

void plot_data_l(plot_t *plt, size_t len, long double x[], long double y[]) {
	size_t i;

	// Build the plot/replot command.
	gnuplot_cmd(plt, SPEC_STR _T(" '-' using 1:2 title \"") SPEC_STR
		_T("\" with ") SPEC_STR,
		(plt->pcount == 0) ? _T("plot") : _T("replot"), plt->sname, plt->pstyle);

	// Send data points.
	for (i = 0; i < len; i++) {
		gnuplot_cmd(plt, _T("%Lg %Lg"), x[i], y[i]);
	}

	// Finish data points list.
	gnuplot_cmd(plt, _T("e"));

	// Increment the plot count.
	plt->pcount++;
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
	// Start a new GNUplot process.
#ifdef _WIN32
	plt->gplot = _tpopen(_T("gnuplot"), _T("w"));
#else
	plt->gplot = popen("gnuplot", "w");
#endif  // _WIN32
	if (plt->gplot == NULL) {
		_ftprintf(stderr, _T("Couldn't open a new GNUplot process. Make sure ")
				_T("gnuplot is in your system PATH") LINEBREAK);

		plot_destroy(plt);
		return;
	}
}

/**
 * Variable argument version of gnuplot_cmd_cont.
 *
 * @param plt Plotting handle.
 * @param cmd Command to be executed by a GNUplot instance (with printf format
 *            specifiers).
 * @param ap  Variables to be substituted in place of the format specifiers.
 */
void vgnuplot_cmd_cont(plot_t *plt, const TCHAR *cmd, va_list ap) {
#ifdef DEBUG
	_vftprintf(stdout, cmd, ap);
#endif  // DEBUG
	_vftprintf(plt->gplot, cmd, ap);
}

/**
 * Sends a raw command to the GNUplot process without terminating the command
 * line.
 *
 * @param plt Plotting handle.
 * @param cmd Command to be executed by a GNUplot instance (with printf format
 *            specifiers).
 * @param ... Variables to be substituted in place of the format specifiers.
 */
void gnuplot_cmd_cont(plot_t *plt, const TCHAR *cmd, ...) {
	va_list ap;

	// Check if we have a valid handle.
	if (plt == NULL) {
		_ftprintf(stderr, _T("Trying to write a command to an invalid handle")
			LINEBREAK);
		return;
	}

	// Perform a printf command to GNUplot.
	va_start(ap, cmd);
	vgnuplot_cmd_cont(plt, cmd, ap);
	va_end(ap);
}

/**
 * Makes sure a command has been sent to the GNUplot process.
 *
 * @param plt Plotting handle.
 */
void gnuplot_cmd_flush(plot_t *plt) {
#ifdef DEBUG
	_tprintf(LINEBREAK);
#endif  // DEBUG

	_fputts(_T("\n"), plt->gplot);
	fflush(plt->gplot);
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

	// Perform a printf command to GNUplot.
	va_start(ap, cmd);
	vgnuplot_cmd_cont(plt, cmd, ap);
	va_end(ap);

	// Make sure we flush the command we've just sent.
	gnuplot_cmd_flush(plt);
}
