/**
 * builtins.c
 * Core functions of the language written in C instead of Lisp.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "builtins.h"
#include "environment.h"

/**
 * Populates the environment with our built-in functions.
 *
 * @param env Pointer to the environment to be populated.
 *
 * @return BAMBOO_OK if the population was successful.
 */
bamboo_error_t populate_builtins(env_t *env) {
	bamboo_error_t err;

	// Basic pair operations.
	err = bamboo_env_set_builtin(*env, "CAR", builtin_car);
	IF_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, "CDR", builtin_cdr);
	IF_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, "CONS", builtin_cons);
	IF_ERROR(err)
		return err;

	// Arithmetic operations.
	err = bamboo_env_set_builtin(*env, "+", builtin_sum);
	IF_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, "-", builtin_subtract);
	IF_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, "*", builtin_multiply);
	IF_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, "/", builtin_divide);
	IF_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, "MOD", builtin_modulo);
	IF_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, "EXPT", builtin_expt);
	IF_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, "FLOOR", builtin_floor);
	IF_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, "ROUND", builtin_round);
	IF_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, "CEIL", builtin_ceil);
	IF_ERROR(err)
		return err;

	// Boolean operations.
	err = bamboo_env_set_builtin(*env, "NOT", builtin_not);
	IF_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, "AND", builtin_and);
	IF_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, "OR", builtin_or);
	IF_ERROR(err)
		return err;

	// Predicates for numbers.
	err = bamboo_env_set_builtin(*env, "=", builtin_numeq);
	IF_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, "<", builtin_lt);
	IF_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, ">", builtin_gt);
	IF_ERROR(err)
		return err;

	// Atom testing.
	err = bamboo_env_set_builtin(*env, "EQ?", builtin_eq);
	IF_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, "NIL?", builtin_nilp);
	IF_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, "PAIR?", builtin_pairp);
	IF_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, "SYMBOL?", builtin_symbolp);
	IF_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, "INTEGER?", builtin_integerp);
	IF_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, "FLOAT?", builtin_floatp);
	IF_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, "NUMERIC?", builtin_numericp);
	IF_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, "BOOLEAN?", builtin_booleanp);
	IF_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, "BUILTIN?", builtin_builtinp);
	IF_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, "CLOSURE?", builtin_closurep);
	IF_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, "MACRO?", builtin_macrop);
	IF_ERROR(err)
		return err;

	// Console I/O.
	err = bamboo_env_set_builtin(*env, "DISPLAY", builtin_display);
	IF_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, "CONCAT", builtin_concat);
	IF_ERROR(err)
		return err;
	err = bamboo_env_set_builtin(*env, "NEWLINE", builtin_newline);
	IF_ERROR(err)
		return err;

	// Misc.
	err = bamboo_env_set_builtin(*env, "DISPLAY-ENV", builtin_display_env);
	IF_ERROR(err)
		return err;

	// Mathematical constants.
	err = bamboo_env_set(*env, bamboo_symbol("E"), bamboo_float(M_E));
	err = bamboo_env_set(*env, bamboo_symbol("LOG2E"), bamboo_float(M_LOG2E));
	err = bamboo_env_set(*env, bamboo_symbol("LOG10E"), bamboo_float(M_LOG10E));
	err = bamboo_env_set(*env, bamboo_symbol("LN2"), bamboo_float(M_LN2));
	err = bamboo_env_set(*env, bamboo_symbol("LN10"), bamboo_float(M_LN10));
	err = bamboo_env_set(*env, bamboo_symbol("PI"), bamboo_float(M_PI));
	err = bamboo_env_set(*env, bamboo_symbol("PI/2"), bamboo_float(M_PI_2));
	err = bamboo_env_set(*env, bamboo_symbol("PI/4"), bamboo_float(M_PI_4));
	err = bamboo_env_set(*env, bamboo_symbol("SQRT2"), bamboo_float(M_SQRT2));
	err = bamboo_env_set(*env, bamboo_symbol("SQRT1/2"), bamboo_float(M_SQRT1_2));

	return BAMBOO_OK;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                              List Operations                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// (car pair) -> atom
bamboo_error_t builtin_car(atom_t args, atom_t *result) {
	// Check if we have the right number of arguments.
	if (bamboo_list_count(args) != 1) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects a single argument");
	}

	// Get the first element of a pair otherwise just return nil.
	if (car(args).type == ATOM_TYPE_PAIR) {
		*result = car(car(args));
	} else {
		*result = nil;
	}

	return BAMBOO_OK;
}

// (cdr pair) -> atom
bamboo_error_t builtin_cdr(atom_t args, atom_t *result) {
	// Check if we have the right number of arguments.
	if (bamboo_list_count(args) != 1) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects a single argument");
	}

	// Get the second element of a pair otherwise just return nil.
	if (car(args).type == ATOM_TYPE_PAIR) {
		*result = cdr(car(args));
	} else {
		*result = nil;
	}

	return BAMBOO_OK;
}

// (cons car cdr) -> pair
bamboo_error_t builtin_cons(atom_t args, atom_t *result) {
	// Check if we have the right number of arguments.
	if (bamboo_list_count(args) != 2) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects 2 arguments");
	}

	// Create the pair.
	*result = cons(car(args), car(cdr(args)));
	return BAMBOO_OK;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                                Mathematics                                 //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// (+ nums...) -> num
bamboo_error_t builtin_sum(atom_t args, atom_t *result) {
	atom_t num;

	// Initialize the result atom.
	num.type = ATOM_TYPE_INTEGER;
	num.value.integer = 0;

	// Check if we have the right number of arguments.
	if (bamboo_list_count(args) < 2) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects at least 2 arguments");
	}

	// Iterate through the arguments summing them.
	while (!nilp(args)) {
		if (car(args).type == ATOM_TYPE_INTEGER) {
			// Integer argument.
			switch (num.type) {
			case ATOM_TYPE_INTEGER:
				num.value.integer += car(args).value.integer;
				break;
			case ATOM_TYPE_FLOAT:
				num.value.dfloat += car(args).value.integer;
				break;
			default:
				return BAMBOO_ERROR_UNKNOWN;
			}
		} else if (car(args).type == ATOM_TYPE_FLOAT) {
			// Float argument. Check if we should change our atom type first.
			if (num.type == ATOM_TYPE_INTEGER) {
				num.type = ATOM_TYPE_FLOAT;
				num.value.dfloat = (long double)num.value.integer;
			}

			// Sum it up.
			num.value.dfloat += car(args).value.dfloat;
		} else {
			// Non-numeric argument.
			return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
				"Invalid type of argument. This function only accepts "
				"numerics");
		}

		// Go to the next argument.
		args = cdr(args);
	}

	// Return the result atom.
	*result = num;
	return BAMBOO_OK;
}

// (- nums...) -> num
bamboo_error_t builtin_subtract(atom_t args, atom_t *result) {
	atom_t num;
	bool first = true;

	// Initialize the result atom.
	num.type = ATOM_TYPE_INTEGER;
	num.value.integer = 0;

	// Check if we have the right number of arguments.
	if (bamboo_list_count(args) < 2) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects at least 2 arguments");
	}

	// Iterate through the arguments subtracting them.
	while (!nilp(args)) {
		if (car(args).type == ATOM_TYPE_INTEGER) {
			// Integer argument.
			if (first) {
				// First iteration, so let's assign the first value first.
				num.type = ATOM_TYPE_INTEGER;
				num.value.integer = car(args).value.integer;
				first = false;

				goto next;
			}

			switch (num.type) {
			case ATOM_TYPE_INTEGER:
				num.value.integer -= car(args).value.integer;
				break;
			case ATOM_TYPE_FLOAT:
				num.value.dfloat -= car(args).value.integer;
				break;
			default:
				return BAMBOO_ERROR_UNKNOWN;
			}
		} else if (car(args).type == ATOM_TYPE_FLOAT) {
			// Float argument.
			if (first) {
				// First iteration, so let's assign the first value first.
				num.type = ATOM_TYPE_FLOAT;
				num.value.dfloat = car(args).value.dfloat;
				first = false;

				goto next;
			}

			// Check if we should change our atom type first.
			if (num.type == ATOM_TYPE_INTEGER) {
				num.type = ATOM_TYPE_FLOAT;
				num.value.dfloat = (long double)num.value.integer;
			}

			// Subtract it up.
			num.value.dfloat -= car(args).value.dfloat;
		} else {
			// Non-numeric argument.
			return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
				"Invalid type of argument. This function only accepts "
				"numerics");
		}

next:
		// Go to the next argument.
		args = cdr(args);
	}

	// Return the result atom.
	*result = num;
	return BAMBOO_OK;
}

// (* nums...) -> num
bamboo_error_t builtin_multiply(atom_t args, atom_t *result) {
	atom_t num;
	bool first = true;

	// Initialize the result atom.
	num.type = ATOM_TYPE_INTEGER;
	num.value.integer = 0;

	// Check if we have the right number of arguments.
	if (bamboo_list_count(args) < 2) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects at least 2 arguments");
	}

	// Iterate through the arguments multiplying them.
	while (!nilp(args)) {
		if (car(args).type == ATOM_TYPE_INTEGER) {
			// Integer argument.
			if (first) {
				// First iteration, so let's assign the first value first.
				num.type = ATOM_TYPE_INTEGER;
				num.value.integer = car(args).value.integer;
				first = false;

				goto next;
			}

			switch (num.type) {
			case ATOM_TYPE_INTEGER:
				num.value.integer *= car(args).value.integer;
				break;
			case ATOM_TYPE_FLOAT:
				num.value.dfloat *= car(args).value.integer;
				break;
			default:
				return BAMBOO_ERROR_UNKNOWN;
			}
		} else if (car(args).type == ATOM_TYPE_FLOAT) {
			// Float argument.
			if (first) {
				// First iteration, so let's assign the first value first.
				num.type = ATOM_TYPE_FLOAT;
				num.value.dfloat = car(args).value.dfloat;
				first = false;

				goto next;
			}

			// Check if we should change our atom type first.
			if (num.type == ATOM_TYPE_INTEGER) {
				num.type = ATOM_TYPE_FLOAT;
				num.value.dfloat = (long double)num.value.integer;
			}

			// Multiply it up.
			num.value.dfloat *= car(args).value.dfloat;
		} else {
			// Non-numeric argument.
			return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
				"Invalid type of argument. This function only accepts "
				"numerics");
		}

next:
		// Go to the next argument.
		args = cdr(args);
	}

	// Return the result atom.
	*result = num;
	return BAMBOO_OK;
}

// (/ nums...) -> num
bamboo_error_t builtin_divide(atom_t args, atom_t *result) {
	atom_t num;
	bool first = true;

	// Initialize the result atom.
	num.type = ATOM_TYPE_FLOAT;
	num.value.dfloat = (long double)0;

	// Check if we have the right number of arguments.
	if (bamboo_list_count(args) < 2) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects at least 2 arguments");
	}

	// Iterate through the arguments dividing them.
	while (!nilp(args)) {
		if (car(args).type == ATOM_TYPE_INTEGER) {
			// Integer argument.
			if (first) {
				// First iteration, so let's assign the first value first.
				num.value.dfloat = (long double)car(args).value.integer;
				first = false;

				goto next;
			}

			num.value.dfloat /= (long double)car(args).value.integer;
		} else if (car(args).type == ATOM_TYPE_FLOAT) {
			// Float argument.
			if (first) {
				// First iteration, so let's assign the first value first.
				num.value.dfloat = car(args).value.dfloat;
				first = false;

				goto next;
			}

			// Check if we should change our atom type first.
			if (num.type == ATOM_TYPE_INTEGER) {
				num.type = ATOM_TYPE_FLOAT;
				num.value.dfloat = (long double)num.value.integer;
			}

			// Divide it up.
			num.value.dfloat /= car(args).value.dfloat;
		} else {
			// Non-numeric argument.
			return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
				"Invalid type of argument. This function only accepts "
				"numerics");
		}

next:
		// Go to the next argument.
		args = cdr(args);
	}

	// Return the result atom.
	*result = num;
	return BAMBOO_OK;
}

// (expt x y) -> num
bamboo_error_t builtin_expt(atom_t args, atom_t *result) {
	atom_t nx;
	atom_t ny;

	// Check if we have the right number of arguments.
	*result = nil;
	if (bamboo_list_count(args) != 2) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects 2 arguments");
	}

	// Get the X argument.
	nx = car(args);
	if (nx.type == ATOM_TYPE_INTEGER) {
		// Looks like we need to convert this to a float atom first.
		nx.type = ATOM_TYPE_FLOAT;
		nx.value.dfloat = (long double)nx.value.integer;
	} else if (nx.type != ATOM_TYPE_FLOAT) {
		// Doesn't look like a numeric to me...
		return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
			"Invalid type of argument. This function only accepts "
			"numerics");
	}

	// Get the Y argument.
	ny = car(cdr(args));
	if (ny.type == ATOM_TYPE_INTEGER) {
		// Looks like we need to convert this to a float atom first.
		ny.type = ATOM_TYPE_FLOAT;
		ny.value.dfloat = (long double)ny.value.integer;
	} else if (ny.type != ATOM_TYPE_FLOAT) {
		// Doesn't look like a numeric to me...
		return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
			"Invalid type of argument. This function only accepts "
			"numerics");
	}

	// Set the result atom.
	result->type = ATOM_TYPE_FLOAT;
	result->value.dfloat = powl(nx.value.dfloat, ny.value.dfloat);

	return BAMBOO_OK;
}

// (mod x y) -> num
bamboo_error_t builtin_modulo(atom_t args, atom_t *result) {
	*result = nil;

	// Check if we have the right number of arguments.
	if (bamboo_list_count(args) != 2) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects 2 arguments");
	}

	// Perform the calculation.
	if (car(args).type == ATOM_TYPE_INTEGER) {
		// X is an integer.
		result->type = ATOM_TYPE_FLOAT;

		// Actually calculate the modulo.
		if (car(cdr(args)).type == ATOM_TYPE_INTEGER) {
			// Both are integer.
			result->value.dfloat = fmodl((long double)car(args).value.integer,
				(long double)car(cdr(args)).value.integer);
		} else if (car(cdr(args)).type == ATOM_TYPE_FLOAT) {
			// Y is float, so we need to make sure the function returns a float.
			result->value.dfloat = fmodl((long double)result->value.integer,
				car(cdr(args)).value.dfloat);
		}

		return BAMBOO_OK;
	} else if (car(args).type == ATOM_TYPE_FLOAT) {
		// X is an float.
		result->type = ATOM_TYPE_FLOAT;

		// Actually calculate the modulo.
		if (car(cdr(args)).type == ATOM_TYPE_FLOAT) {
			// Both are floats.
			result->value.dfloat = fmodl(car(args).value.dfloat,
				car(cdr(args)).value.dfloat);
		} else if (car(cdr(args)).type == ATOM_TYPE_INTEGER) {
			// Y is an integer.
			result->value.dfloat = fmodl(car(args).value.dfloat,
				(long double)car(cdr(args)).value.integer);
		}

		return BAMBOO_OK;
	}

	// Doesn't look like a numeric to me...
	return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
		"Invalid type of argument. This function only accepts "
		"numerics");
}

// (floor x) -> int
bamboo_error_t builtin_floor(atom_t args, atom_t *result) {
	*result = nil;

	// Check if we have the right number of arguments.
	if (bamboo_list_count(args) != 1) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects a single argument");
	}

	// Round the number.
	result->type = ATOM_TYPE_INTEGER;
	if (car(args).type == ATOM_TYPE_INTEGER) {
		// Number is an integer.
		result->value.integer = car(args).value.integer;
		return BAMBOO_OK;
	} else if (car(args).type == ATOM_TYPE_FLOAT) {
		// Number is an float.
		result->value.integer = (int64_t)floorl(car(args).value.dfloat);
		return BAMBOO_OK;
	}

	// Doesn't look like a numeric to me...
	*result = nil;
	return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
		"Invalid type of argument. This function only accepts "
		"numerics");
}

// (round x) -> int
bamboo_error_t builtin_round(atom_t args, atom_t *result) {
	*result = nil;

	// Check if we have the right number of arguments.
	if (bamboo_list_count(args) != 1) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects a single argument");
	}

	// Round the number.
	result->type = ATOM_TYPE_INTEGER;
	if (car(args).type == ATOM_TYPE_INTEGER) {
		// Number is an integer.
		result->value.integer = car(args).value.integer;
		return BAMBOO_OK;
	} else if (car(args).type == ATOM_TYPE_FLOAT) {
		// Number is an float.
		result->value.integer = (int64_t)roundl(car(args).value.dfloat);
		return BAMBOO_OK;
	}

	// Doesn't look like a numeric to me...
	*result = nil;
	return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
		"Invalid type of argument. This function only accepts "
		"numerics");
}

// (ceil x) -> int
bamboo_error_t builtin_ceil(atom_t args, atom_t *result) {
	*result = nil;

	// Check if we have the right number of arguments.
	if (bamboo_list_count(args) != 1) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects a single argument");
	}

	// Round the number.
	result->type = ATOM_TYPE_INTEGER;
	if (car(args).type == ATOM_TYPE_INTEGER) {
		// Number is an integer.
		result->value.integer = car(args).value.integer;
		return BAMBOO_OK;
	} else if (car(args).type == ATOM_TYPE_FLOAT) {
		// Number is an float.
		result->value.integer = (int64_t)ceill(car(args).value.dfloat);
		return BAMBOO_OK;
	}

	// Doesn't look like a numeric to me...
	*result = nil;
	return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
		"Invalid type of argument. This function only accepts "
		"numerics");
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                            Boolean Operations                              //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// (not bool) -> bool
bamboo_error_t builtin_not(atom_t args, atom_t *result) {
	// Check if we have the right number of arguments.
	if (bamboo_list_count(args) != 1) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects exactly 1 argument");
	}

	// Populate the result atom.
	result->type = ATOM_TYPE_BOOLEAN;
	result->value.boolean = !atom_boolean_val(car(args));

	return BAMBOO_OK;
}

// (and bool...) -> bool
bamboo_error_t builtin_and(atom_t args, atom_t *result) {
	atom_t prev_atom;

	// Check if we have the right number of arguments.
	if (bamboo_list_count(args) < 2) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects at least 2 arguments");
	}

	// Iterate through the arguments checking them.
	prev_atom = car(args);
	args = cdr(args);
	while (!nilp(args)) {
		// We only need a single false value.
		if (atom_boolean_val(prev_atom) != atom_boolean_val(car(args))) {
			*result = bamboo_boolean(false);
			return BAMBOO_OK;
		}

		// Go to the next argument.
		prev_atom = car(args);
		args = cdr(args);
	}

	// Looks like they were all true all along.
	*result = bamboo_boolean(true);
	return BAMBOO_OK;
}

// (or bool...) -> bool
bamboo_error_t builtin_or(atom_t args, atom_t *result) {
	atom_t prev_atom;

	// Check if we have the right number of arguments.
	if (bamboo_list_count(args) < 2) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects at least 2 arguments");
	}

	// Iterate through the arguments checking them.
	prev_atom = car(args);
	args = cdr(args);
	while (!nilp(args)) {
		// We only need a single true value.
		if (atom_boolean_val(prev_atom) || atom_boolean_val(car(args))) {
			*result = bamboo_boolean(true);
			return BAMBOO_OK;
		}

		// Go to the next argument.
		prev_atom = car(args);
		args = cdr(args);
	}

	// Looks like they were all false all along.
	*result = bamboo_boolean(false);
	return BAMBOO_OK;
}

// (= nums...) -> bool
bamboo_error_t builtin_numeq(atom_t args, atom_t *result) {
	atom_t prev_num;

	// Check if we have the right number of arguments.
	if (bamboo_list_count(args) < 2) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects at least 2 arguments");
	}

	// Iterate through the arguments checking them.
	prev_num = car(args);
	args = cdr(args);
	while (!nilp(args)) {
		if (car(args).type == ATOM_TYPE_INTEGER) {
			if (prev_num.type == ATOM_TYPE_INTEGER) {
				// int == int.
				if (prev_num.value.integer == car(args).value.integer)
					goto next;
			} else {
				// float == int.
				if (prev_num.value.dfloat == (long double)car(args).value.integer)
					goto next;
			}

			*result = bamboo_boolean(false);
			return BAMBOO_OK;
		} else if (car(args).type == ATOM_TYPE_FLOAT) {
			if (prev_num.type == ATOM_TYPE_INTEGER) {
				// float == int.
				if ((long double)prev_num.value.integer == car(args).value.dfloat)
					goto next;
			} else {
				// float == float.
				if (prev_num.value.dfloat == car(args).value.dfloat)
					goto next;
			}

			*result = bamboo_boolean(false);
				return BAMBOO_OK;
		} else {
			// Non-numeric argument.
			return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
				"Invalid type of argument. This function only accepts "
				"numerics");
		}

next:
		// Go to the next argument.
		prev_num = car(args);
		args = cdr(args);
	}

	// Looks like they were all equal all along.
	*result = bamboo_boolean(true);
	return BAMBOO_OK;
}

// (< nums...) -> bool
bamboo_error_t builtin_lt(atom_t args, atom_t *result) {
	atom_t prev_num;

	// Check if we have the right number of arguments.
	if (bamboo_list_count(args) < 2) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects at least 2 arguments");
	}

	// Iterate through the arguments checking them.
	prev_num = car(args);
	args = cdr(args);
	while (!nilp(args)) {
		if (car(args).type == ATOM_TYPE_INTEGER) {
			if (prev_num.type == ATOM_TYPE_INTEGER) {
				// int == int.
				if (prev_num.value.integer < car(args).value.integer)
					goto next;
			} else {
				// float == int.
				if (prev_num.value.dfloat < (long double)car(args).value.integer)
					goto next;
			}

			*result = bamboo_boolean(false);
			return BAMBOO_OK;
		} else if (car(args).type == ATOM_TYPE_FLOAT) {
			if (prev_num.type == ATOM_TYPE_INTEGER) {
				// float == int.
				if ((long double)prev_num.value.integer < car(args).value.dfloat)
					goto next;
			} else {
				// float == float.
				if (prev_num.value.dfloat < car(args).value.dfloat)
					goto next;
			}

			*result = bamboo_boolean(false);
				return BAMBOO_OK;
		} else {
			// Non-numeric argument.
			return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
				"Invalid type of argument. This function only accepts "
				"numerics");
		}

next:
		// Go to the next argument.
		prev_num = car(args);
		args = cdr(args);
	}

	// Looks like they were all equal all along.
	*result = bamboo_boolean(true);
	return BAMBOO_OK;
}

// (> nums...) -> bool
bamboo_error_t builtin_gt(atom_t args, atom_t *result) {
	atom_t prev_num;

	// Check if we have the right number of arguments.
	if (bamboo_list_count(args) < 2) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects at least 2 arguments");
	}

	// Iterate through the arguments checking them.
	prev_num = car(args);
	args = cdr(args);
	while (!nilp(args)) {
		if (car(args).type == ATOM_TYPE_INTEGER) {
			if (prev_num.type == ATOM_TYPE_INTEGER) {
				// int == int.
				if (prev_num.value.integer > car(args).value.integer)
					goto next;
			} else {
				// float == int.
				if (prev_num.value.dfloat > (long double)car(args).value.integer)
					goto next;
			}

			*result = bamboo_boolean(false);
			return BAMBOO_OK;
		} else if (car(args).type == ATOM_TYPE_FLOAT) {
			if (prev_num.type == ATOM_TYPE_INTEGER) {
				// float == int.
				if ((long double)prev_num.value.integer > car(args).value.dfloat)
					goto next;
			} else {
				// float == float.
				if (prev_num.value.dfloat > car(args).value.dfloat)
					goto next;
			}

			*result = bamboo_boolean(false);
				return BAMBOO_OK;
		} else {
			// Non-numeric argument.
			return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
				"Invalid type of argument. This function only accepts "
				"numerics");
		}

next:
		// Go to the next argument.
		prev_num = car(args);
		args = cdr(args);
	}

	// Looks like they were all equal all along.
	*result = bamboo_boolean(true);
	return BAMBOO_OK;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                               Type Checking                                //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// (eq? a b) -> boolean
bamboo_error_t builtin_eq(atom_t args, atom_t *result) {
	atom_t a;
	atom_t b;

	// Check if we have the right number of arguments.
	if (bamboo_list_count(args) != 2) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects 2 arguments");
	}

	// Get the two atoms to be tested.
	a = car(args);
	b = car(cdr(args));

	// If the types are different then it's instantly different.
	if (a.type != b.type) {
		*result = bamboo_boolean(false);
		return BAMBOO_OK;
	}

	// Check for more equality.
	switch (a.type) {
	case ATOM_TYPE_NIL:
		*result = bamboo_boolean(true);
		break;
	case ATOM_TYPE_PAIR:
	case ATOM_TYPE_CLOSURE:
	case ATOM_TYPE_MACRO:
		*result = bamboo_boolean(a.value.pair == b.value.pair);
		break;
	case ATOM_TYPE_SYMBOL:
		*result = bamboo_boolean(*a.value.symbol == *b.value.symbol);
		break;
	case ATOM_TYPE_STRING:
		*result = bamboo_boolean(strcmp(*a.value.str, *b.value.str) == 0);
		break;
	case ATOM_TYPE_BOOLEAN:
		*result = bamboo_boolean(a.value.boolean == b.value.boolean);
		break;
	case ATOM_TYPE_INTEGER:
		*result = bamboo_boolean(a.value.integer == b.value.integer);
		break;
	case ATOM_TYPE_FLOAT:
		*result = bamboo_boolean(a.value.dfloat == b.value.dfloat);
		break;
	case ATOM_TYPE_BUILTIN:
		*result = bamboo_boolean(a.value.builtin == b.value.builtin);
		break;
	case ATOM_TYPE_POINTER:
		*result = bamboo_boolean(a.value.pointer == b.value.pointer);
		break;
	}

	return BAMBOO_OK;
}

// (nil? atom) -> boolean
bamboo_error_t builtin_nilp(atom_t args, atom_t *result) {
	// Check if we have the right number of arguments.
	if (bamboo_list_count(args) != 1) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects 1 argument");
	}

	*result = bamboo_boolean(car(args).type == ATOM_TYPE_NIL);
	return BAMBOO_OK;
}

// (pair? atom) -> boolean
bamboo_error_t builtin_pairp(atom_t args, atom_t *result) {
	// Check if we have the right number of arguments.
	if (bamboo_list_count(args) != 1) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects 1 argument");
	}

	*result = bamboo_boolean(car(args).type == ATOM_TYPE_PAIR);
	return BAMBOO_OK;
}

// (symbol? atom) -> boolean
bamboo_error_t builtin_symbolp(atom_t args, atom_t *result) {
	// Check if we have the right number of arguments.
	if (bamboo_list_count(args) != 1) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects 1 argument");
	}

	*result = bamboo_boolean(car(args).type == ATOM_TYPE_SYMBOL);
	return BAMBOO_OK;
}

// (integer? atom) -> boolean
bamboo_error_t builtin_integerp(atom_t args, atom_t *result) {
	// Check if we have the right number of arguments.
	if (bamboo_list_count(args) != 1) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects 1 argument");
	}

	*result = bamboo_boolean(car(args).type == ATOM_TYPE_INTEGER);
	return BAMBOO_OK;
}

// (float? atom) -> boolean
bamboo_error_t builtin_floatp(atom_t args, atom_t *result) {
	// Check if we have the right number of arguments.
	if (bamboo_list_count(args) != 1) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects 1 argument");
	}

	*result = bamboo_boolean(car(args).type == ATOM_TYPE_FLOAT);
	return BAMBOO_OK;
}

// (numeric? atom) -> boolean
bamboo_error_t builtin_numericp(atom_t args, atom_t *result) {
	// Check if we have the right number of arguments.
	if (bamboo_list_count(args) != 1) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects 1 argument");
	}

	*result = bamboo_boolean((car(args).type == ATOM_TYPE_INTEGER) ||
		(car(args).type == ATOM_TYPE_FLOAT));
	return BAMBOO_OK;
}

// (boolean? atom) -> boolean
bamboo_error_t builtin_booleanp(atom_t args, atom_t *result) {
	// Check if we have the right number of arguments.
	if (bamboo_list_count(args) != 1) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects 1 argument");
	}

	*result = bamboo_boolean(car(args).type == ATOM_TYPE_BOOLEAN);
	return BAMBOO_OK;
}

// (builtin? atom) -> boolean
bamboo_error_t builtin_builtinp(atom_t args, atom_t *result) {
	// Check if we have the right number of arguments.
	if (bamboo_list_count(args) != 1) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects 1 argument");
	}

	*result = bamboo_boolean(car(args).type == ATOM_TYPE_BUILTIN);
	return BAMBOO_OK;
}

// (closure? atom) -> boolean
bamboo_error_t builtin_closurep(atom_t args, atom_t *result) {
	// Check if we have the right number of arguments.
	if (bamboo_list_count(args) != 1) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects 1 argument");
	}

	*result = bamboo_boolean(car(args).type == ATOM_TYPE_CLOSURE);
	return BAMBOO_OK;
}

// (macro? atom) -> boolean
bamboo_error_t builtin_macrop(atom_t args, atom_t *result) {
	// Check if we have the right number of arguments.
	if (bamboo_list_count(args) != 1) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects 1 argument");
	}

	*result = bamboo_boolean(car(args).type == ATOM_TYPE_MACRO);
	return BAMBOO_OK;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                       Display and String Operations                        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// (display any...) -> string
bamboo_error_t builtin_display(atom_t args, atom_t *result) {
	bamboo_error_t err;

	// Use the concat function to help us out here.
	err = builtin_concat(args, result);
	IF_ERROR(err)
		return err;

	// Print the concatenated string.
	putstr(*result->value.str);
	putchar('\n');

	return BAMBOO_OK;
}

// (concat any...) -> string
bamboo_error_t builtin_concat(atom_t args, atom_t *result) {
	size_t buflen = 0;
	size_t tmplen = 0;
	char *tmpbuf = NULL;
	char *buf = NULL;

	// Check if we have the right number of arguments.
	if (bamboo_list_count(args) < 1) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects at least 1 argument");
	}

	// Get a clean slate.
	buf = (char *)malloc(sizeof(char));
	if (buf == NULL) {
		*result = nil;
		return bamboo_error(BAMBOO_ERROR_ALLOCATION, "Can't allocate "
			"new string to begin concatenation");
	}
	buf[0] = '\0';

	// Iterate through the arguments printing them them.
	while (!nilp(args)) {
		switch (car(args).type) {
		case ATOM_TYPE_STRING:
			// Get the argument string length.
			tmpbuf = *car(args).value.str;
			tmplen = strlen(tmpbuf);
			buflen += tmplen;

			// Reallocate the string to fit the new concatenated string.
			buf = (char *)realloc(buf, (buflen + 1) * sizeof(char));
			if (buf == NULL) {
				*result = nil;
				return bamboo_error(BAMBOO_ERROR_ALLOCATION, "Can't allocate "
					"new string to concatenate string atom");
			}

			// Actually concatenate the strings.
			strcat(buf, tmpbuf);
			break;
		case ATOM_TYPE_NIL:
			break;
		case ATOM_TYPE_SYMBOL:
			// Get the argument string length.
			tmplen = strlen(*car(args).value.symbol);
			buflen += tmplen;

			// Reallocate the string to fit the new concatenated string.
			buf = (char *)realloc(buf, (buflen + 1) * sizeof(char));
			if (buf == NULL) {
				*result = nil;
				return bamboo_error(BAMBOO_ERROR_ALLOCATION, "Can't allocate "
					"new string to concatenate symbol atom");
			}

			// Actually concatenate the strings.
			strcat(buf, *car(args).value.symbol);
			break;
		case ATOM_TYPE_INTEGER:
			// Get the length of the string we'll need to concatenate this number.
#if defined(_MSC_VER) && (_MSC_VER <= 1400)
			tmplen = I64_MAX_DIGITS;
#else
			tmplen = snprintf(NULL, 0, "%lld", car(args).value.integer);
#endif  // _MSC_VER
			tmpbuf = (char *)malloc((tmplen + 1) * sizeof(char));
			if (tmpbuf == NULL) {
				*result = nil;
				return bamboo_error(BAMBOO_ERROR_ALLOCATION, "Can't allocate "
					"string to display integer atom");
			}
#if defined(_MSC_VER) && (_MSC_VER <= 1400)
			snprintf(tmpbuf, tmplen + 1, "%I64d", car(args).value.integer);
#else
			snprintf(tmpbuf, tmplen + 1, "%lld", car(args).value.integer);
#endif  // _MSC_VER

			// Reallocate the string to fit the new concatenated string.
			buflen += tmplen;
			buf = (char *)realloc(buf, (buflen + 1) * sizeof(char));
			if (buf == NULL) {
				*result = nil;
				return bamboo_error(BAMBOO_ERROR_ALLOCATION, "Can't allocate "
					"new string to concatenate integer atom");
			}

			// Actually concatenate the strings and free our temporary string.
			strcat(buf, tmpbuf);
			free(tmpbuf);
			break;
		case ATOM_TYPE_FLOAT:
			// Get the length of the string we'll need to concatenate this number.
#if defined(_MSC_VER) && (_MSC_VER <= 1400)
			tmplen = LONGDOUBLE_MAX_DIGITS;
#else
			tmplen = snprintf(NULL, 0, "%Lg", car(args).value.dfloat);
#endif  // _MSC_VER
			tmpbuf = (char *)malloc((tmplen + 1) * sizeof(char));
			if (tmpbuf == NULL) {
				*result = nil;
				return bamboo_error(BAMBOO_ERROR_ALLOCATION, "Can't allocate "
					"string to display float atom");
			}
			snprintf(tmpbuf, tmplen + 1, "%Lg", car(args).value.dfloat);

			// Reallocate the string to fit the new concatenated string.
			buflen += tmplen;
			buf = (char *)realloc(buf, (buflen + 1) * sizeof(char));
			if (buf == NULL) {
				*result = nil;
				return bamboo_error(BAMBOO_ERROR_ALLOCATION, "Can't allocate "
					"new string to concatenate float atom");
			}

			// Actually concatenate the strings and free our temporary string.
			strcat(buf, tmpbuf);
			free(tmpbuf);
			break;
		case ATOM_TYPE_BOOLEAN:
			// Determine the string needed to concatenate depending on value.
			if (car(args).value.boolean) {
				tmpbuf = strdup("TRUE");
				tmplen = 4;
			} else {
				tmpbuf = strdup("FALSE");
				tmplen = 5;
			}

			// Reallocate the string to fit the new concatenated string.
			buflen += tmplen;
			buf = (char *)realloc(buf, (buflen + 1) * sizeof(char));
			if (buf == NULL) {

				*result = nil;
				return bamboo_error(BAMBOO_ERROR_ALLOCATION, "Can't allocate "
					"new string to concatenate boolean atom");
			}

			// Actually concatenate the strings and free our temporary string.
			strcat(buf, tmpbuf);
			free(tmpbuf);
			break;
		default:
			*result = nil;
			return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
				"Don't know how to display this type of atom");
		}

		// Go to the next argument.
		args = cdr(args);
	}

	// Build the string atom to return and free our building buffer.
	*result = bamboo_string(buf);
	free(buf);

	return BAMBOO_OK;
}

// (newline) -> nil
bamboo_error_t builtin_newline(atom_t args, atom_t *result) {
	// Check if we have the right number of arguments.
	if (bamboo_list_count(args) != 0) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects no arguments");
	}

	// Print the newline string.
	putchar('\n');

	*result = nil;
	return BAMBOO_OK;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                                 Debugging                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// (display-env) -> nil
bamboo_error_t builtin_display_env(atom_t args, atom_t *result) {
	env_t current;

	// Check if we have the right number of arguments.
	if (bamboo_list_count(args) != 0) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects no arguments");
	}

	// Grab our root environment.
	current = cdr(*bamboo_root_env);

	// Iterate over the symbols in the environment filtering out built-ins.
	putstr("symbol\t\tvalue\n");
	while (!nilp(current)) {
		atom_t item = car(current);

		// Filter out any built-ins.
		if (cdr(item).type == ATOM_TYPE_BUILTIN)
			goto next_item;

		// Print out the symbol name and its value.
		printf("%s\t\t", *car(item).value.symbol);
		bamboo_print_expr(cdr(item));
		putchar('\n');

next_item:
		// Go to the next item.
		current = cdr(current);
	}

	*result = nil;
	return BAMBOO_OK;
}
