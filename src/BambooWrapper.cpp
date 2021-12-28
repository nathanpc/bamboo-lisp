/**
 * BambooWrapper.h
 * A C++ wrapper for the Bamboo Lisp interpreter C library.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "BambooWrapper.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/**
 * Initializes a brand new interpreter environment.
 */
Bamboo::Bamboo() {
	bamboo_error_t err = bamboo_init(&this->env.environment());
	if (err)
		throw BambooException(err);
}

/**
 * Destroys the interpreter environment.
 */
Bamboo::~Bamboo() {
	bamboo_error_t err = bamboo_destroy(&this->env.environment());
	if (err)
		throw BambooException(err);
}

/**
 * Creates a new root Bamboo environment.
 */
BambooEnvironment::BambooEnvironment() {
	this->env = bamboo_env_new(nil);
}

/**
 * Creates a new Bamboo environment with a parent.
 *
 * @param parent Parent environment.
 */
BambooEnvironment::BambooEnvironment(env_t& parent) {
	this->env = bamboo_env_new(parent);
}

/**
 * Gets the entire internal environment container.
 *
 * @return Bamboo internal environment container.
 */
env_t& BambooEnvironment::environment() {
	return this->env;
}

/**
 * Gets the value of a symbol in the environment. Throws an
 * exception if the symbol wasn't found in the environment.
 *
 * @param  symbol Symbol to be searched for in the environment.
 * @return        Value atom associated with the symbol.
 */
atom_t BambooEnvironment::get(atom_t symbol) {
	bamboo_error_t err;
	atom_t atom;

	// Get the actual atom.
	err = bamboo_env_get(this->env, symbol, &atom);
	if (err)
		throw BambooException(err);

	return atom;
}

/**
 * Sets the value of a symbol in the environment. Throws an
 * exception if something went wrong.
 *
 * @param symbol Name of the symbol to have its value changed.
 * @param value  Value for the symbol to have.
 */
void BambooEnvironment::set(atom_t symbol, atom_t value) {
	bamboo_error_t err;

	// Set the symbol's value.
	err = bamboo_env_set(this->env, symbol, value);
	if (err)
		throw BambooException(err);
}

/**
 * Wraps Bamboo's error system into a pretty little C++ exception.
 *
 * @param err Bamboo error.
 */
BambooException::BambooException(bamboo_error_t err) {
	this->err = err;
}

/**
 * Creates the error message according to Bamboo's internal error handling.
 *
 * @return Descriptive error message.
 */
const char* BambooException::what() const throw() {
	return bamboo_error_detail();
}

/**
 * Grabs the internal Bamboo Lisp error code that generated the exception.
 *
 * @return Internal error code.
 */
bamboo_error_t BambooException::ErrorCode() {
	return this->err;
}