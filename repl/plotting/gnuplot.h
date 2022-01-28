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

// Define the plot structure for this specific subsystem.
typedef struct {
	FILE *gplot;
} plot_t;

#include "plot.h"

// GNUplot specific things.
void gnuplot_cmd(plot_t *plt, const TCHAR *cmd, ...);

#ifdef __cplusplus
}
#endif

#endif  // PLOTTING_GNUPLOT_H
