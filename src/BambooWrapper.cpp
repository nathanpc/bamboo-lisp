/**
 * BambooWrapper.h
 * A C++ wrapper for the Bamboo Lisp interpreter C library.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "BambooWrapper.h"
#include <cstdlib>

// Microsoft-specific stuff.
#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
	#define new DEBUG_NEW
#endif

// Make old C++ compilers happy.
#ifndef nullptr
	#define nullptr NULL
#endif  // nullptr

using namespace Bamboo;

/**
 * Initializes a brand new interpreter environment.
 */
Lisp::Lisp() {
	bamboo_error_t err = bamboo_init(&this->m_env.env());
	IF_BAMBOO_ERROR(err)
		throw BambooException(err);
}

/**
 * Destroys the interpreter environment.
 */
Lisp::~Lisp() {
	bamboo_error_t err = bamboo_destroy(&this->m_env.env());
	IF_BAMBOO_ERROR(err)
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
	IF_BAMBOO_ERROR(err)
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
	IF_BAMBOO_ERROR(err)
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
 * Gets the string representation of the contents of an expression.
 * WARNING: Remember that you're responsible for freeing the pointer returned
 * by this function later.
 *
 * @param  atom Atom to have its contents represented.
 * @return      String allocated by this function containing the atom
 *              represented as a string.
 */
TCHAR* Lisp::expr_str(atom_t atom) {
	TCHAR *buf;
	bamboo_expr_str(&buf, atom);

	return buf;
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
	IF_BAMBOO_ERROR(err)
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
	IF_BAMBOO_ERROR(err)
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
	IF_BAMBOO_ERROR(err)
		throw BambooException(err);
}

/**
 * Wraps Bamboo's error system into a pretty little C++ exception.
 *
 * @param err Bamboo error.
 */
BambooException::BambooException(bamboo_error_t err) {
	this->m_err = err;
	this->m_type_str = nullptr;
}

/**
 * Cleans up the mess left behind by the exception message allocations.
 */
BambooException::~BambooException() {
	// Make sure we clean up the allocation mess.
	if (this->m_type_str)
		free(m_type_str);
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

/**
 * Gets the human-friendly error type string that generated the exception.
 *
 * @return Error type message.
 */
TCHAR* BambooException::error_type() {
	// Get the error type string if we haven't fetched it yet.
	if (this->m_type_str == nullptr)
		bamboo_error_type_str(&this->m_type_str, this->error_code());

	return this->m_type_str;
}

/**
 * Gets a human-friendly detailed error message string that generated this
 * exception.
 *
 * @return Detailed error message.
 */
const TCHAR* BambooException::error_detail() {
	return bamboo_error_detail();
}