/**
 * environment.c
 * Manages the environment of the interpreter, keeping track of its state,
 * managing all symbols and definitions.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "environment.h"

#include <stddef.h>
#include "primitives.h"

/**
 * Global root environment holding all definitions.
 */
static env_t *bamboo_root_env = NULL;

/**
 * Global symbol table.
 */
static atom_t bamboo_symbol_table = { ATOM_TYPE_NIL };

/**
 * Creates a new child environment list.
 *
 * @param parent Parent environment to this new child.
 *
 * @return New child environment list.
 */
env_t bamboo_env_new(env_t parent) {
	return cons(parent, nil);
}

/**
 * Gets a symbol definition from an environment list recursively searching
 * through its parents.
 *
 * @param env    Environment list to search for the desired symbol in.
 * @param symbol Symbol you're searching for.
 * @param atom   Pointer to the resulting atom of the symbol definition.
 *
 * @return BAMBOO_OK if the symbol was found. BAMBOO_ERROR_UNBOUND otherwise.
 */
bamboo_error_t bamboo_env_get(env_t env, atom_t symbol, atom_t *atom) {
	env_t parent = car(env);
	env_t current = cdr(env);

	// Clean up the result just in case.
	*atom = nil;

	// Iterate through the symbols in the environment list.
	while (!nilp(current)) {
		// Get symbol-value pair.
		atom_t item = car(current);

		// Take advantage of the fact we can't have different symbols with same
		// name to compare them by pointer instead of having to do strcmp.
		if (*car(item).value.symbol == *symbol.value.symbol) {
			*atom = cdr(item);
			return BAMBOO_OK;
		}

		// Check the next symbol in the list.
		current = cdr(current);
	}

	// Check if we've reached the end of our parent environments to search for.
	if (nilp(parent)) {
		char msg[ERROR_MSG_STR_LEN + 1];

		// Build the error string.
		snprintf(msg, ERROR_MSG_STR_LEN, "Symbol '%s' not found in any of the "
			"environments", *symbol.value.symbol);
		return bamboo_error(BAMBOO_ERROR_UNBOUND, msg);
	}

	// Search for the symbol in the parent.
	return bamboo_env_get(parent, symbol, atom);
}

/**
 * Creates a new symbol inside an environment or changes it if it already
 * exists in the specified environment.
 *
 * @param env    Parent environment where the symbol resides.
 * @param symbol Symbol to be created or edited.
 * @param value  Value attributed to the symbol.
 *
 * @return BAMBOO_OK if the operation was successful.
 */
bamboo_error_t bamboo_env_set(env_t env, atom_t symbol, atom_t value) {
	env_t current = cdr(env);
	atom_t item = nil;

	// Iterate over the symbols in the environment list checking if the symbol
	// already exists in the current environment.
	while (!nilp(current)) {
		// Get a symbol from the list.
		item = car(current);

		// Check if the symbol matches another one in the environment.
		if (*car(item).value.symbol == *symbol.value.symbol) {
			cdr(item) = value;
			return BAMBOO_OK;
		}

		// Go to the next symbol.
		current = cdr(current);
	}

	// Looks like this is a new symbol definition. Create it then...
	item = cons(symbol, value);
	cdr(env) = cons(item, cdr(env));

	return BAMBOO_OK;
}

/**
 * Creates a new built-in function symbol inside an environment or changes it if
 * it already exists in the specified environment.
 *
 * @param env  Parent environment where the built-in function symbol resides.
 * @param name Name of the symbol for the built-in function.
 * @param func Built-in function that will be called for this symbol.
 *
 * @return BAMBOO_OK if the operation was successful.
 */
bamboo_error_t bamboo_env_set_builtin(env_t env, const char *name,
									  builtin_func_t func) {
	return bamboo_env_set(env, bamboo_symbol(name), bamboo_builtin(func));
}

/**
 * Gets the root environment current in use by the interpreter.
 *
 * @return Pointer to the current root environment.
 */
env_t *bamboo_get_root_env(void) {
	return bamboo_root_env;
}
