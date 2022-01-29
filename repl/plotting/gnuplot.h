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

#include "../../src/bamboo.h"
#include <stdio.h>
#include <stdint.h>

// Public definitions.
#define GNUPLOT_STYLE_MAX_LEN 14
#define GNUPLOT_TITLE_MAX_LEN 20

// Define the plot structure for this specific subsystem.
typedef struct {
	FILE *gplot;

	uint8_t pcount;
	TCHAR sname[GNUPLOT_TITLE_MAX_LEN + 1];
	TCHAR pstyle[GNUPLOT_STYLE_MAX_LEN + 1];
} plot_t;

#include "plot.h"

// GNUplot specific things.
void gnuplot_cmd(plot_t *plt, const TCHAR *cmd, ...);

#ifdef __cplusplus
}
#endif

#endif  // PLOTTING_GNUPLOT_H
