/**
 * BambooWrapper.h
 * A C++ wrapper for the Bamboo Lisp interpreter C library.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef _BAMBOOWRAPPER_H
#define _BAMBOOWRAPPER_H

#include <exception>
#include <vector>
#include "bamboo.h"

namespace Bamboo {

	/**
	 * Bamboo exception abstraction to handle errors.
	 */
#ifdef _MSC_VER
	class BambooException : public exception {
#else
	class BambooException : public std::exception {
#endif  // _MSC_VER
	protected:
		bamboo_error_t m_err;
		TCHAR *m_type_str;

	public:
		// Classic implementation.
		BambooException(bamboo_error_t err);
		virtual ~BambooException();
		virtual const char* what() const throw();

		// Information collection.
		bamboo_error_t error_code();
		TCHAR* error_type();
		const TCHAR* error_detail();
	};

	/**
	 * Bamboo environment abstraction class.
	 */
	class Environment {
	protected:
		env_t m_env;

	public:
		// Filtering enumerator.
		enum ListFilter {
			FilterNothing = 0,
			FilterUserGenerated,
			FilterClosuresAndMacros,
			FilterPrimitives,
			FilterBuiltins
		};

		// Constructors and destructors.
		Environment();
		Environment(env_t& parent);

		// Getters.
		env_t& env();
		std::vector<pair_t> list(ListFilter filter);

		// Environment manipulation.
		atom_t get(atom_t symbol);
		void set(atom_t symbol, atom_t value);
		void set_builtin(const TCHAR *name, builtin_func_t func);
	};

	/**
	 * Bamboo wrapper class.
	 */
	class Lisp {
	protected:
		Environment m_env;

	public:
		Lisp();
		virtual ~Lisp();

		// Parsing and evaluation.
		atom_t parse_expr(const TCHAR *input);
		atom_t parse_expr(const TCHAR *input, const TCHAR **end);
		atom_t eval_expr(atom_t expr);

		// Information.
		TCHAR* expr_str(atom_t atom);

		// Environment.
		Environment& env();
	};

}

#endif  // _BAMBOOWRAPPER_H
