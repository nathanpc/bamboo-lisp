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

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                       Initialization and Destruction                       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                            Settings and Styling                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/**
 * Sets the title of graph.
 *
 * @param plt   Plotting handle.
 * @param title Graph main title.
 */
void plot_set_title(plot_t *plt, const TCHAR *title);

/**
 * Sets the type of graph that will be used to display the series.
 *
 * @param plt  Plotting handle.
 * @param type Name of the graph type.
 */
void plot_set_type(plot_t *plt, const TCHAR *type);

/**
 * Sets the name of the series to be displayed in the legend.
 *
 * @param plt  Plotting handle.
 * @param name Series name to be displayed in the legend.
 */
void plot_set_series_name(plot_t *plt, const TCHAR *name);

/**
 * Sets the label of the X axis.
 *
 * @param plt   Plotting handle.
 * @param label X axis label.
 */
void plot_set_xlabel(plot_t *plt, const TCHAR *label);

/**
 * Sets the label of the Y axis.
 *
 * @param plt   Plotting handle.
 * @param label Y axis label.
 */
void plot_set_ylabel(plot_t *plt, const TCHAR *label);

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                           Plotting and Graphing                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/**
 * Clears the plots and starts over.
 *
 * @param plt Plotting handle.
 */
void plot_clear(plot_t *plt);

/**
 * Plots a mathematical function.
 *
 * @param plt      Plotting handle.
 * @param equation Mathematical function.
 */
void plot_equation(plot_t *plt, const TCHAR *equation);

/**
 * Plots a series of data points.
 *
 * @param plt Plotting handle.
 * @param len Length of the arrays of values.
 * @param x   Array of X values.
 * @param y   Array of Y values.
 */
void plot_data_l(plot_t *plt, size_t len, long double x[], long double y[]);

#ifdef __cplusplus
}
#endif

#endif  // PLOTTING_PLOT_H
