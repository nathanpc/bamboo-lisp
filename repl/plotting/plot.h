/**
 * plot.h
 * A generic interface to the plotting subsystems.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef PLOTTING_PLOT_H
#define PLOTTING_PLOT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../src/bamboo.h"

// Currently we only have the GNUplot subsystem, so that's the only include
// for now.
#include "gnuplot.h"

//
// Initialization and destruction.
//

/**
 * Initializes the plotting environment.
 *
 * @return Plotting handle structure.
 */
plot_t* plot_init(void);

/**
 * Destroys the plotting environment and frees up resources.
 *
 * @param plt Plotting handle.
 */
void plot_destroy(plot_t *plt);

//
// Plotting and graphing.
//

/**
 * Plots a mathematical function.
 *
 * @param plt      Plotting handle.
 * @param equation Mathematical function.
 */
void plot_equation(plot_t *plt, const TCHAR *equation);

#ifdef __cplusplus
}
#endif

#endif  // PLOTTING_PLOT_H
