/**
 * gnuplot.h
 * A GNUplot subsystem for our plotting library.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef PLOTTING_GNUPLOT_H
#define PLOTTING_GNUPLOT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#ifdef _WIN32
	#include <windows.h>
#endif  // _WIN32

// Define the plot structure for this specific subsystem.
typedef struct {
#ifdef _WIN32
	HANDLE hndGplotW;
	HANDLE hndGplotR;
	PROCESS_INFORMATION piProcInfo;
#else
	FILE *gplot;
#endif  // _WIN32
} plot_t;

#include "plot.h"

// GNUplot specific things.
void gnuplot_cmd(plot_t *plt, const TCHAR *cmd, ...);

#ifdef __cplusplus
}
#endif

#endif  // PLOTTING_GNUPLOT_H
