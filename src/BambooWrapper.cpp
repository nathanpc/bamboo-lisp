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

using namespace Bamboo;

/**
 * Initializes a brand new interpreter environment.
 */
Lisp::Lisp() {
	bamboo_error_t err = bamboo_init(&this->m_env.env());
	if (err)
		throw BambooException(err);
}

/**
 * Destroys the interpreter environment.
 */
Lisp::~Lisp() {
	bamboo_error_t err = bamboo_destroy(&this->m_env.env());
	if (err)
		throw BambooException(err);
}

/**
 * Parses an expression into an atom. Throws an exception if the input could
 * not be parsed.
 *
 * @param  input Input string to be parsed.
 * @param  end   Pointer that will store the position where the parser stopped
 *               parsing the input string.
 * @return       Parsed atom.
 */
atom_t Lisp::parse_expr(const TCHAR *input, const TCHAR **end) {
	atom_t atom;
	bamboo_error_t err;

	// Parse the expression.
	err = bamboo_parse_expr(input, end, &atom);
	if (err)
		throw BambooException(err);

	return atom;
}

/**
 * Evaluates an expression atom and get its return value. Throws an exception
 * if an error occured during the evaluation of the expression.
 *
 * @param  expr Expression atom to be evaluated.
 * @return      Return value of the evaluated expression.
 */
atom_t Lisp::eval_expr(atom_t expr) {
	atom_t result;
	bamboo_error_t err;

	// Evaluate the expression.
	err = bamboo_eval_expr(expr, this->m_env.env(), &result);
	if (err)
		throw BambooException(err);

	return result;
}

/**
 * Parses an expression into an atom. Throws an exception if the input could
 * not be parsed.
 *
 * @param  input Input string to be parsed.
 * @return       Parsed atom.
 */
atom_t Lisp::parse_expr(const TCHAR *input) {
	const TCHAR *ignored;
	return this->parse_expr(input, &ignored);
}

/**
 * Gets the interpreter environment.
 *
 * @return Reference to our current environment.
 */
Environment& Lisp::env() {
	return this->m_env;
}

/**
 * Creates a new root Bamboo environment.
 */
Environment::Environment() {
	this->m_env = bamboo_env_new(nil);
}

/**
 * Creates a new Bamboo environment with a parent.
 *
 * @param parent Parent environment.
 */
Environment::Environment(env_t& parent) {
	this->m_env = bamboo_env_new(parent);
}

/**
 * Gets the entire internal environment container.
 *
 * @return Bamboo internal environment container.
 */
env_t& Environment::env() {
	return this->m_env;
}

/**
 * Gets the value of a symbol in the environment. Throws an exception if the
 * symbol wasn't found in the environment.
 *
 * @param  symbol Symbol to be searched for in the environment.
 * @return        Value atom associated with the symbol.
 */
atom_t Environment::get(atom_t symbol) {
	bamboo_error_t err;
	atom_t atom;

	// Get the actual atom.
	err = bamboo_env_get(this->env(), symbol, &atom);
	if (err)
		throw BambooException(err);

	return atom;
}

/**
 * Sets the value of a symbol in the environment. Throws an exception if
 * something went wrong.
 *
 * @param symbol Name of the symbol to have its value changed.
 * @param value  Value for the symbol to have.
 */
void Environment::set(atom_t symbol, atom_t value) {
	bamboo_error_t err;

	// Set the symbol's value.
	err = bamboo_env_set(this->env(), symbol, value);
	if (err)
		throw BambooException(err);
}

/**
 * Adds a built-in function into the environment. Throws an exception if
 * something went wrong.
 *
 * @param name Name of the symbol of the built-in function in the environment.
 * @param func Function to handle the call.
 */
void Environment::set_builtin(const TCHAR *name, builtin_func_t func) {
	bamboo_error_t err;

	// Add the built-in function.
	err = bamboo_env_set_builtin(this->env(), name, func);
	if (err)
		throw BambooException(err);
}

/**
 * Wraps Bamboo's error system into a pretty little C++ exception.
 *
 * @param err Bamboo error.
 */
BambooException::BambooException(bamboo_error_t err) {
	this->m_err = err;
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
bamboo_error_t BambooException::error_code() {
	return this->m_err;
}