/**
 * primitives.c
 * Primitive types of our Lisp dialect.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "primitives.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * Builds an integer atom.
 *
 * @param num Integer number.
 *
 * @return Integer atom.
 */
atom_t bamboo_int(int64_t num) {
	atom_t atom;

	// Populate the atom.
	atom.type = ATOM_TYPE_INTEGER;
	atom.value.integer = num;

	return atom;
}

/**
 * Builds an floating-point atom.
 *
 * @param num Double floating-point number.
 *
 * @return Floating-point atom.
 */
atom_t bamboo_float(long double num) {
	atom_t atom;

	// Populate the atom.
	atom.type = ATOM_TYPE_FLOAT;
	atom.value.dfloat = num;

	return atom;
}

/**
 * Builds an symbol atom.
 *
 * @param name Symbol name.
 *
 * @return Symbol atom.
 */
atom_t bamboo_symbol(const char *name) {
	atom_t atom;
	atom_t tmp;
	allocation_t *alloc;

	// Check if the symbol already exists in the symbol table.
	tmp = bamboo_symbol_table;
	while (!nilp(tmp)) {
		atom = car(tmp);
		if (strcmp(*atom.value.symbol, name) == 0)
			return atom;

		tmp = cdr(tmp);
	}

	// Create a new allocation for the symbol name.
	alloc = (allocation_t *)malloc(sizeof(allocation_t));
	if (alloc == NULL) {
		fatal_error(BAMBOO_ERROR_ALLOCATION, "Can't allocate structure for "
			"garbage collector symbol allocation tracking");
		return nil;
	}

	// Fill up the new allocation and push the linked list forward.
	alloc->mark = GC_TO_FREE;
	alloc->type = ALLOCATION_TYPE_STRING;
	alloc->str = strdup(name);
	alloc->next = bamboo_allocations;
	bamboo_allocations = alloc;

	// Create the new symbol atom.
	atom.type = ATOM_TYPE_SYMBOL;
	atom.value.symbol = &alloc->str;

	// Prepend the symbol atom to the symbol table and return the atom.
	bamboo_symbol_table = cons(atom, bamboo_symbol_table);
	return atom;
}

/**
 * Build an boolean atom.
 *
 * @param value Boolean value for the atom.
 *
 * @return Boolean atom.
 */
atom_t bamboo_boolean(bool value) {
	atom_t atom;

	// Create the boolean atom.
	atom.type = ATOM_TYPE_BOOLEAN;
	atom.value.boolean = value;

	return atom;
}

/**
 * Builds an string atom.
 *
 * @param str String to be stored.
 *
 * @return String atom.
 */
atom_t bamboo_string(const char *str) {
	allocation_t *alloc;
	atom_t atom;

	// Create a new allocation.
	alloc = (allocation_t *)malloc(sizeof(allocation_t));
	if (alloc == NULL) {
		fatal_error(BAMBOO_ERROR_ALLOCATION, "Can't allocate structure for "
			"garbage collector string allocation tracking");
		return nil;
	}

	// Fill up the new allocation and push the linked list forward.
	alloc->mark = GC_TO_FREE;
	alloc->type = ALLOCATION_TYPE_STRING;
	alloc->str = strdup(str);
	alloc->next = bamboo_allocations;
	bamboo_allocations = alloc;

	// Create the new string atom.
	atom.type = ATOM_TYPE_STRING;
	atom.value.str = &alloc->str;

	return atom;
}

/**
 * Builds an built-in function atom.
 *
 * @param func Built-in C function.
 *
 * @return Built-in function atom.
 */
atom_t bamboo_builtin(builtin_func_t func) {
	atom_t atom;

	// Populate the atom.
	atom.type = ATOM_TYPE_BUILTIN;
	atom.value.builtin = func;

	return atom;
}

/**
 * Builds an closure (procedure) atom.
 *
 * @param env    Environment for this closure.
 * @param args   Arguments for the closure.
 * @param body   Body of the closure.
 * @param result Pointer to store the resulting atom.
 *
 * @return BAMBOO_OK if the atom creation was successful.
 */
bamboo_error_t bamboo_closure(env_t env, atom_t args, atom_t body,
		atom_t *result) {
	atom_t tmp;

	// Check if the body is a list.
	if (!listp(body))
		return bamboo_error(BAMBOO_ERROR_SYNTAX, "Closure body must be a list");

	// Check if all argument names are symbols or if it ends in a pair.
	tmp = args;
	while (!nilp(tmp)) {
		// Check if we have a variadic function or an invalid arguments list.
		if (tmp.type == ATOM_TYPE_SYMBOL) {
			// Last argument is a symbol instead of nil. This means we have a
			// variadic function.
			break;
		} else if ((tmp.type != ATOM_TYPE_PAIR) ||
				(car(tmp).type != ATOM_TYPE_SYMBOL)) {
			return bamboo_error(BAMBOO_ERROR_SYNTAX,
				"All arguments must be symbols or a pair at the end");
		}

		// Next argument.
		tmp = cdr(tmp);
	}

	// Make the closure atom.
	*result = cons(env, cons(args, body));
	result->type = ATOM_TYPE_CLOSURE;

	return BAMBOO_OK;
}

/**
 * Builds an pointer atom.
 *
 * @param pointer Pointer to be stored.
 *
 * @return Pointer atom.
 */
atom_t bamboo_pointer(void *pointer) {
	atom_t atom;

	// Populate the atom.
	atom.type = ATOM_TYPE_POINTER;
	atom.value.pointer = pointer;

	return atom;
}
