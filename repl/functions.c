/**
 * functions.c
 * Built-in functions specially for the REPL.
 * 
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "functions.h"
#include <stdio.h>
#include <stdlib.h>
#include "fileutils.h"
#ifdef USE_PLOTTING
	#include "plotting/plot.h"
#endif  // USE_PLOTTING

// Built-in function prototypes.
bamboo_error_t builtin_quit(atom_t args, atom_t *result);
bamboo_error_t builtin_load(atom_t args, atom_t *result);
#ifdef USE_PLOTTING
bamboo_error_t builtin_plot_init(atom_t args, atom_t *result);
bamboo_error_t builtin_plot_destroy(atom_t args, atom_t *result);
bamboo_error_t builtin_plot_clear(atom_t args, atom_t *result);
bamboo_error_t builtin_plot_title(atom_t args, atom_t *result);
bamboo_error_t builtin_plot_xlabel(atom_t args, atom_t *result);
bamboo_error_t builtin_plot_ylabel(atom_t args, atom_t *result);
bamboo_error_t builtin_plot_type(atom_t args, atom_t *result);
bamboo_error_t builtin_plot_name(atom_t args, atom_t *result);
bamboo_error_t builtin_plot_equation(atom_t args, atom_t *result);
bamboo_error_t builtin_plot_data(atom_t args, atom_t *result);
#endif  // USE_PLOTTING

/**
 * Loads the contents of a source file into the given environment.
 * 
 * @param  env    Pointer to the environment for the source to be evaluated in.
 * @param  fname  Path to the file to be loaded.
 * @param  result Return atom of the last evaluated expression in the source.
 * @return        BAMBOO_OK if everything went fine.
 */
bamboo_error_t load_source(env_t *env, const TCHAR *fname, atom_t *result) {
	bamboo_error_t err;
	atom_t parsed;
	TCHAR *contents;
	const TCHAR *end;

	// Start from a clean slate.
	err = BAMBOO_OK;
	*result = nil;

	// Just remind the user of what's happening.
	_tprintf(_T("Loading ") SPEC_STR LINEBREAK, fname);

	// Get the file contents.
	contents = slurp_file(fname);
	if (contents == NULL) {
		return bamboo_error(BAMBOO_ERROR_UNKNOWN,
			_T("Couldn't read the specified file for some reason"));
	}

	// Parse and evaluate the contents of the file.
	end = contents;
	while (*end != _T('\0')) {
#ifdef DEBUG
		// Check out our tokens.
		bamboo_print_tokens(end);
		_tprintf(LINEBREAK);
#endif  // DEBUG

		// Parse the expression.
		err = bamboo_parse_expr(end, &end, &parsed);
		IF_BAMBOO_ERROR(err) {
			// Show the error message.
			bamboo_print_error(err);
			goto stop;
		}

		// Deal with some special conditions from the parsing stage.
		IF_BAMBOO_SPECIAL_COND(err) {
			switch (err) {
			case BAMBOO_EMPTY_LINE:
				// Ignore things that are meant to be ignored.
				end++;
				continue;
			default:
				break;
			}
		}

		// Evaluate the parsed expression.
		err = bamboo_eval_expr(parsed, *env, result);
		IF_BAMBOO_ERROR(err) {
			// Check if we just got a quit situation.
			if (err == (bamboo_error_t)BAMBOO_REPL_QUIT)
				goto stop;

			// Explain the real issue then...
			bamboo_print_error(err);
			goto stop;
		}
	}

stop:
	// Clean up and return.
	free(contents);
	return err;
}

/**
 * Populates the environment with our built-in functions.
 *
 * @param  env Pointer to the environment to be populated.
 * @return     BAMBOO_OK if the population was successful.
 */
bamboo_error_t repl_populate_builtins(env_t *env) {
	bamboo_error_t err;

	// Quit interpreter.
	err = bamboo_env_set_builtin(*env, _T("QUIT"), builtin_quit);
	IF_BAMBOO_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, _T("EXIT"), builtin_quit);
	IF_BAMBOO_ERROR(err)
		return err;

	// Load source file.
	err = bamboo_env_set_builtin(*env, _T("LOAD"), builtin_load);
	IF_BAMBOO_ERROR(err)
		return err;

#ifdef USE_PLOTTING
	err = bamboo_env_set_builtin(*env, _T("PLOT-INIT"), builtin_plot_init);
	IF_BAMBOO_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, _T("PLOT-CLOSE"),
		builtin_plot_destroy);
	IF_BAMBOO_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, _T("PLOT-CLEAR"), builtin_plot_clear);
	IF_BAMBOO_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, _T("PLOT-TITLE"), builtin_plot_title);
	IF_BAMBOO_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, _T("PLOT-XLABEL"), builtin_plot_xlabel);
	IF_BAMBOO_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, _T("PLOT-YLABEL"), builtin_plot_ylabel);
	IF_BAMBOO_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, _T("PLOT-NAME"), builtin_plot_name);
	IF_BAMBOO_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, _T("PLOT-TYPE"), builtin_plot_type);
	IF_BAMBOO_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, _T("PLOT-STYLE"), builtin_plot_type);
	IF_BAMBOO_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, _T("PLOT-EQN"), builtin_plot_equation);
	IF_BAMBOO_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, _T("PLOT-DATA"), builtin_plot_data);
	IF_BAMBOO_ERROR(err)
		return err;
#endif  // USE_PLOTTING

	return BAMBOO_OK;
}

/**
 * Quits the interpreter
 *
 * (quit [retval])
 *
 * @param retval Value to be returned when exiting the interpreter.
 */
bamboo_error_t builtin_quit(atom_t args, atom_t *result) {
	atom_t retval;

	// Just in case...
	*result = bamboo_int(-1);

	// Check if we don't have any arguments.
	if (nilp(args)) {
		result->value.integer = 0;
		_tprintf(_T("Bye!") LINEBREAK);
		
		return (bamboo_error_t)BAMBOO_REPL_QUIT;
	}

	// Check if we have more than a single argument.
	if (!nilp(cdr(args)))
		return BAMBOO_ERROR_ARGUMENTS;

	// Get the first argument.
	retval = car(args);

	// Check if its the right type of argument.
	if (retval.type != ATOM_TYPE_INTEGER)
		return BAMBOO_ERROR_WRONG_TYPE;

	// Exit with the specified return value.
	*result = retval;
	_tprintf(_T("Bye!") LINEBREAK);
	return (bamboo_error_t)BAMBOO_REPL_QUIT;
}

/**
 * Evaluates the contents of a file.
 *
 * (load fname) -> any?
 *
 * @param  fname Path to the file to be loaded into the environment.
 * @return       Last return value of the evaluated source.
 */
bamboo_error_t builtin_load(atom_t args, atom_t *result) {
	atom_t fname;

	// Just in case...
	*result = nil;

	// Check if we don't have any arguments.
	if (nilp(args)) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			_T("A file path must be supplied to this function"));
	}

	// Check if we have more than a single argument.
	if (!nilp(cdr(args))) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			_T("Only a single file path should be supplied to this function"));
	}

	// Get the file name argument.
	fname = car(args);

	// Check if its the right type of argument.
	if (fname.type != ATOM_TYPE_STRING) {
		return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
			_T("File name atom must be of type string"));
	}

	// Load the file.
	return load_source(bamboo_get_root_env(), *fname.value.str, result);
}

#ifdef USE_PLOTTING
/**
 * Initializes a plotting environment.
 *
 * (plot-init) -> pointer
 *
 * @return Pointer to the plotting handle. NULL if an error occured.
 */
bamboo_error_t builtin_plot_init(atom_t args, atom_t *result) {
	plot_t *plt;

	// Just in case...
	*result = nil;

	// Check if we don't have any arguments.
	if (!nilp(args)) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			_T("No arguments should be supplied to this function"));
	}

	// Initialize the plotting environment.
	plt = plot_init();
	if (plt == NULL)
		return BAMBOO_OK;

	// Set the result atom.
	result->type = ATOM_TYPE_POINTER;
	result->value.pointer = (void *)plt;

	return BAMBOO_OK;
}

/**
 * Destroys a plotting environment.
 *
 * (plot-close plthnd)
 *
 * @param plthnd Plotting handle pointer.
 */
bamboo_error_t builtin_plot_destroy(atom_t args, atom_t *result) {
	atom_t plthnd;
	plot_t *plt;

	// Just in case...
	*result = nil;

	// Check if we don't have any arguments.
	if (nilp(args)) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			_T("A plotting handle must be supplied to this function"));
	}

	// Check if we have more than a single argument.
	if (!nilp(cdr(args))) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			_T("Only a single argument should be supplied to this function"));
	}

	// Check if we have a pointer argument.
	plthnd = car(args);
	if (plthnd.type != ATOM_TYPE_POINTER) {
		return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
			_T("Plotting handle atom must be of type pointer"));
	}

	// Get the plotting handle and promptly destroy it.
	plt = (plot_t *)plthnd.value.pointer;
	plot_destroy(plt);

	return BAMBOO_OK;
}

/**
 * Clears a plotting window.
 *
 * (plot-clear plthnd)
 *
 * @param plthnd Plotting handle pointer.
 */
bamboo_error_t builtin_plot_clear(atom_t args, atom_t *result) {
	atom_t plthnd;
	plot_t *plt;

	// Just in case...
	*result = nil;

	// Check if we don't have any arguments.
	if (nilp(args)) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			_T("A plotting handle must be supplied to this function"));
	}

	// Check if we have more than a single argument.
	if (!nilp(cdr(args))) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			_T("Only a single argument should be supplied to this function"));
	}

	// Check if we have a pointer argument.
	plthnd = car(args);
	if (plthnd.type != ATOM_TYPE_POINTER) {
		return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
			_T("Plotting handle atom must be of type pointer"));
	}

	// Get the plotting handle and promptly destroy it.
	plt = (plot_t *)plthnd.value.pointer;
	plot_clear(plt);

	return BAMBOO_OK;
}

/**
 * Sets the title of a plot.
 *
 * (plot-title plthnd title)
 *
 * @param plthnd Plotting handle pointer.
 * @param title  Title of the plot.
 */
bamboo_error_t builtin_plot_title(atom_t args, atom_t *result) {
	atom_t plthnd;
	atom_t title;
	plot_t *plt;

	// Just in case...
	*result = nil;

	// Check if we don't have any arguments.
	if (nilp(args)) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			_T("A plotting handle must be supplied to this function"));
	}

	// Check if we have more than two arguments.
	if (bamboo_list_count(args) != 2) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			_T("Only 2 arguments should be supplied to this function"));
	}

	// Check if we have a pointer argument.
	plthnd = car(args);
	if (plthnd.type != ATOM_TYPE_POINTER) {
		return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
			_T("Plotting handle atom must be of type pointer"));
	}

	// Check if we have a string argument.
	title = car(cdr(args));
	if (title.type != ATOM_TYPE_STRING) {
		return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
			_T("Plot title atom must be of type string"));
	}

	// Get the plotting handle, the equation, and plot it.
	plt = (plot_t *)plthnd.value.pointer;
	plot_set_title(plt, *title.value.str);

	return BAMBOO_OK;
}

/**
 * Sets the label of the X axis of a plot.
 *
 * (plot-xlabel plthnd label)
 *
 * @param plthnd Plotting handle pointer.
 * @param label  Axis label.
 */
bamboo_error_t builtin_plot_xlabel(atom_t args, atom_t *result) {
	atom_t plthnd;
	atom_t label;
	plot_t *plt;

	// Just in case...
	*result = nil;

	// Check if we don't have any arguments.
	if (nilp(args)) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			_T("A plotting handle must be supplied to this function"));
	}

	// Check if we have more than two arguments.
	if (bamboo_list_count(args) != 2) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			_T("Only 2 arguments should be supplied to this function"));
	}

	// Check if we have a pointer argument.
	plthnd = car(args);
	if (plthnd.type != ATOM_TYPE_POINTER) {
		return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
			_T("Plotting handle atom must be of type pointer"));
	}

	// Check if we have a string argument.
	label = car(cdr(args));
	if (label.type != ATOM_TYPE_STRING) {
		return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
			_T("Axis label atom must be of type string"));
	}

	// Get the plotting handle, the equation, and plot it.
	plt = (plot_t *)plthnd.value.pointer;
	plot_set_xlabel(plt, *label.value.str);

	return BAMBOO_OK;
}

/**
 * Sets the label of the Y axis of a plot.
 *
 * (plot-ylabel plthnd label)
 *
 * @param plthnd Plotting handle pointer.
 * @param label  Axis label.
 */
bamboo_error_t builtin_plot_ylabel(atom_t args, atom_t *result) {
	atom_t plthnd;
	atom_t label;
	plot_t *plt;

	// Just in case...
	*result = nil;

	// Check if we don't have any arguments.
	if (nilp(args)) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			_T("A plotting handle must be supplied to this function"));
	}

	// Check if we have more than two arguments.
	if (bamboo_list_count(args) != 2) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			_T("Only 2 arguments should be supplied to this function"));
	}

	// Check if we have a pointer argument.
	plthnd = car(args);
	if (plthnd.type != ATOM_TYPE_POINTER) {
		return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
			_T("Plotting handle atom must be of type pointer"));
	}

	// Check if we have a string argument.
	label = car(cdr(args));
	if (label.type != ATOM_TYPE_STRING) {
		return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
			_T("Axis label atom must be of type string"));
	}

	// Get the plotting handle, the equation, and plot it.
	plt = (plot_t *)plthnd.value.pointer;
	plot_set_ylabel(plt, *label.value.str);

	return BAMBOO_OK;
}

/**
 * Sets the name of the series to be displayed in the legend of the plot.
 *
 * (plot-name plthnd name)
 *
 * @param plthnd Plotting handle pointer.
 * @param name   Series name.
 */
bamboo_error_t builtin_plot_name(atom_t args, atom_t *result) {
	atom_t plthnd;
	atom_t name;
	plot_t *plt;

	// Just in case...
	*result = nil;

	// Check if we don't have any arguments.
	if (nilp(args)) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			_T("A plotting handle must be supplied to this function"));
	}

	// Check if we have more than two arguments.
	if (bamboo_list_count(args) != 2) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			_T("Only 2 arguments should be supplied to this function"));
	}

	// Check if we have a pointer argument.
	plthnd = car(args);
	if (plthnd.type != ATOM_TYPE_POINTER) {
		return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
			_T("Plotting handle atom must be of type pointer"));
	}

	// Check if we have a string argument.
	name = car(cdr(args));
	if (name.type != ATOM_TYPE_STRING) {
		return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
			_T("Series name atom must be of type string"));
	}

	// Get the plotting handle, the equation, and plot it.
	plt = (plot_t *)plthnd.value.pointer;
	plot_set_series_name(plt, *name.value.str);

	return BAMBOO_OK;
}

/**
 * Sets the type/style of the plot.
 *
 * (plot-type plthnd type)
 *
 * @param plthnd Plotting handle pointer.
 * @param type   Graphing style.
 */
bamboo_error_t builtin_plot_type(atom_t args, atom_t *result) {
	atom_t plthnd;
	atom_t type;
	plot_t *plt;

	// Just in case...
	*result = nil;

	// Check if we don't have any arguments.
	if (nilp(args)) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			_T("A plotting handle must be supplied to this function"));
	}

	// Check if we have more than two arguments.
	if (bamboo_list_count(args) != 2) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			_T("Only 2 arguments should be supplied to this function"));
	}

	// Check if we have a pointer argument.
	plthnd = car(args);
	if (plthnd.type != ATOM_TYPE_POINTER) {
		return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
			_T("Plotting handle atom must be of type pointer"));
	}

	// Check if we have a symbol argument.
	type = car(cdr(args));
	if (type.type != ATOM_TYPE_SYMBOL) {
		return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
			_T("Plot type atom must be of type symbol"));
	}

	// Get the plotting handle, the equation, and plot it.
	plt = (plot_t *)plthnd.value.pointer;
	plot_set_type(plt, *type.value.symbol);

	return BAMBOO_OK;
}

/**
 * Plots an equation.
 *
 * (plot-eqn plthnd eqn)
 *
 * @param plthnd Plotting handle pointer.
 * @param eqn    Equation string to be plotted.
 */
bamboo_error_t builtin_plot_equation(atom_t args, atom_t *result) {
	atom_t plthnd;
	atom_t eqn;
	plot_t *plt;

	// Just in case...
	*result = nil;

	// Check if we don't have any arguments.
	if (nilp(args)) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			_T("A plotting handle must be supplied to this function"));
	}

	// Check if we have more than two arguments.
	if (bamboo_list_count(args) != 2) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			_T("Only 2 arguments should be supplied to this function"));
	}

	// Check if we have a pointer argument.
	plthnd = car(args);
	if (plthnd.type != ATOM_TYPE_POINTER) {
		return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
			_T("Plotting handle atom must be of type pointer"));
	}

	// Check if we have a string argument.
	eqn = car(cdr(args));
	if (eqn.type != ATOM_TYPE_STRING) {
		return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
			_T("Equation atom must be of type string"));
	}

	// Get the plotting handle, the equation, and plot it.
	plt = (plot_t *)plthnd.value.pointer;
	plot_equation(plt, *eqn.value.str);

	return BAMBOO_OK;
}

/**
 * Plots a series of data points.
 *
 * (plot-data plthnd data)
 *
 * @param plthnd Plotting handle pointer.
 * @param data   Data points to plot as a list of (X . Y) pairs.
 */
bamboo_error_t builtin_plot_data(atom_t args, atom_t *result) {
	atom_t plthnd;
	atom_t data;
	plot_t *plt;
	size_t len;
	long double *x;
	long double *y;
	long double *px;
	long double *py;

	// Just in case...
	*result = nil;

	// Check if we don't have any arguments.
	if (nilp(args)) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			_T("A plotting handle must be supplied to this function"));
	}

	// Check if we have more than two arguments.
	if (bamboo_list_count(args) != 2) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			_T("Only 2 arguments should be supplied to this function"));
	}

	// Check if we have a pointer argument.
	plthnd = car(args);
	if (plthnd.type != ATOM_TYPE_POINTER) {
		return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
			_T("Plotting handle atom must be of type pointer"));
	}

	// Check if we have a list argument.
	data = car(cdr(args));
	if (data.type != ATOM_TYPE_PAIR) {
		return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
			_T("Data points atom must be of type list"));
	}

	// Allocate the data points array.
	len = bamboo_list_count(data);
	x = (long double *)malloc(len * sizeof(long double));
	y = (long double *)malloc(len * sizeof(long double));

	// Get the data points.
	px = x;
	py = y;
	while (!nilp(data)) {
		atom_t item = car(data);

		// Check if we have a proper pair.
		if (item.type != ATOM_TYPE_PAIR) {
			free(x);
			free(y);
			return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
				_T("Data points must be a pair"));
		}

		// Get the X value.
		switch (car(item).type) {
		case ATOM_TYPE_INTEGER:
			*px = (long double)car(item).value.integer;
			break;
		case ATOM_TYPE_FLOAT:
			*px = car(item).value.dfloat;
			break;
		default:
			free(x);
			free(y);
			return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
				_T("X data point must be a numeric atom"));
		}

		// Get the Y value.
		switch (cdr(item).type) {
		case ATOM_TYPE_INTEGER:
			*py = (long double)cdr(item).value.integer;
			break;
		case ATOM_TYPE_FLOAT:
			*py = cdr(item).value.dfloat;
			break;
		default:
			free(x);
			free(y);
			return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
				_T("Y data point must be a numeric atom"));
		}

		// Next data point.
		px++;
		py++;
		data = cdr(data);
	}

	// Get the plotting handle and plot the data.
	plt = (plot_t *)plthnd.value.pointer;
	plot_data_l(plt, len, x, y);

	// Clean up our mess.
	free(x);
	free(y);

	return BAMBOO_OK;
}
#endif  // USE_PLOTTING
