/**
 * BambooWrapper.h
 * A C++ wrapper for the Bamboo Lisp interpreter C library.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#if !defined(AFX_BAMBOOLISP_H__758F6A83_C64F_4861_B2E8_9BBFFAB7465B__INCLUDED_)
#define AFX_BAMBOOLISP_H__758F6A83_C64F_4861_B2E8_9BBFFAB7465B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <exception>
#include <tchar.h>
#include "bamboo.h"

namespace Bamboo {

	/**
	 * Bamboo exception abstraction to handle errors.
	 */
	class BambooException : public exception {
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
		// Constructors and destructors.
		Environment();
		Environment(env_t& parent);

		// Getters.
		env_t& env();

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

#endif // !defined(AFX_BAMBOOLISP_H__758F6A83_C64F_4861_B2E8_9BBFFAB7465B__INCLUDED_)
