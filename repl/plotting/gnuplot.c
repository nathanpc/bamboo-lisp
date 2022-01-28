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
	if (plt->gplot) {
		if (pclose(plt->gplot) == -1) {
			_ftprintf(stderr, _T("An error occured while trying to close ")
					_T("GNUplot's process"));
		}
	}
	
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
	// Start a new GNUplot process.
	plt->gplot = popen("gnuplot", "w");
	if (plt->gplot == NULL) {
		_ftprintf(stderr, _T("Couldn't open a new GNUplot process. Make sure ")
				_T("gnuplot is in your system PATH"));

		plot_destroy(plt);
		return;
	}
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

	// Check if we have a valid handle.
	if (plt == NULL) {
		_ftprintf(stderr, _T("Trying to write a command to an invalid handle"));
		return;
	}

	// Perform a printf command to GNUplot.
	va_start(ap, cmd);
	_vftprintf(plt->gplot, cmd, ap);
	va_end(ap);

	// Make sure we flush the command we've just sent.
	_fputts(_T("\n"), plt->gplot);
	fflush(plt->gplot);
}
