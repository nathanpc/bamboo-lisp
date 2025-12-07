/**
 * bamboo.c
 * A small and purpose-built Lisp dialect focused on scientific problem solving.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "bamboo.h"
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#ifndef _WIN32_WCE
	#include <errno.h>
#endif  // _WIN32_WCE
#include <limits.h>
#include <float.h>
#include <math.h>

// Convenience macros.
#define IF_ERROR(err)        IF_BAMBOO_ERROR(err)
#define IF_SPECIAL_COND(err) IF_BAMBOO_SPECIAL_COND(err)
#define IF_NOT_ERROR(err)    if ((err) <= BAMBOO_OK)

// Make Visual C++ 6.0 not complain about passing NULL to _sntprintf.
#if defined(_MSC_VER) && (_MSC_VER <= 1400)
	#define I64_MAX_DIGITS        128
	#define LONGDOUBLE_MAX_DIGITS 128
#endif

// HUGE_VALL is C99, so let's just make sure we cater to the previous decade.
#ifndef HUGE_VALL
	#define HUGE_VALL LDBL_MAX
#endif  // HUGE_VALL

// Make Microsoft's Visual C++ 6.0 happy about our limits.
#ifndef LLONG_MAX
	#define LLONG_MAX _I64_MAX
#endif  // LLONG_MAX
#ifndef LLONG_MIN
	#define LLONG_MIN _I64_MIN
#endif  // LLONG_MIN

// Token structure.
typedef struct {
	const char *start;
	const char *end;
} token_t;

// Stack frame definitions.
typedef atom_t frame_t;
typedef enum {
	STACK_PARENT_INDEX = 0,
	STACK_ENV_INDEX,
	STACK_EVAL_OP_INDEX,
	STACK_PENDING_ARGS_INDEX,
	STACK_EVAL_ARGS_INDEX,
	STACK_BODY_INDEX
} frame_idx_t;

// Garbage collection definitions.
typedef enum {
	GC_TO_FREE = 0,
	GC_IN_USE
} gc_mark_t;
typedef enum {
	ALLOCATION_TYPE_PAIR = 0,
	ALLOCATION_TYPE_STRING
} alloc_type_t;
typedef struct allocation_s allocation_t;
struct allocation_s {
	pair_t pair;
	char *str;
	alloc_type_t type;
	gc_mark_t mark;
	allocation_t *next;
};

// Private variables.
static allocation_t *bamboo_allocations = NULL;
static uint32_t bamboo_gc_iter_counter = 0;

// Private methods.
void putstr(const char *str);
void putstrerr(const char *str);
char* strcpyse(const char *start, const char *end);
bool contains_point(const char *str);
bool atom_boolean_val(atom_t atom);
void gc_mark(atom_t root);
void gc(bool respect_marks);
atom_t shallow_copy_list(atom_t list);
bamboo_error_t lex(const char *str, token_t *token);
bamboo_error_t parse_hash_expr(const token_t *token, const char **end,
	atom_t *atom);
bamboo_error_t parse_primitive(const token_t *token, const char **end,
	atom_t *atom);
bamboo_error_t parse_string(const token_t *token, const char **end,
	atom_t *atom);
bamboo_error_t parse_list(const char *input, const char **end, atom_t *atom);
bamboo_error_t parse_comment(const token_t *token, const char **end,
	atom_t *atom);
frame_t new_stack_frame(frame_t parent, env_t env, atom_t tail);
bamboo_error_t eval_expr_exec(frame_t *stack, atom_t *expr, env_t *env);
bamboo_error_t eval_expr_bind(frame_t *stack, atom_t *expr, env_t *env);
bamboo_error_t eval_expr_apply(frame_t *stack, atom_t *expr, env_t *env);
bamboo_error_t eval_expr_return(frame_t *stack, atom_t *expr, env_t *env,
	atom_t *result);

/**
 * Initializes the Bamboo interpreter environment.
 *
 * @param env Pointer to the root environment of the interpreter.
 *
 * @return BAMBOO_OK if everything went fine.
 */
bamboo_error_t bamboo_init(env_t *env) {
	bamboo_error_t err;

	// Display a pretty welcome message.
	putstr("Bamboo Lisp v0.1a\n\n");

	// Make sure the error message string is properly terminated.
	bamboo_error_msg[0] = '\0';
	bamboo_error_msg[ERROR_MSG_STR_LEN] = '\0';

	// Make sure the garbage collection iteration counter is zeroed out.
	bamboo_gc_iter_counter = 0;

	// Initialize the root environment.
	*env = bamboo_env_new(nil);
	bamboo_root_env = env;

	// Populate the environment with our built-in functions.
	err = populate_builtins(env);
	IF_ERROR(err)
		return err;

	return BAMBOO_OK;
}

/**
 * Destroys an Bamboo interpreter environment.
 *
 * @param env Pointer to the root environment of the interpreter.
 *
 * @return BAMBOO_OK if everything went fine.
 */
bamboo_error_t bamboo_destroy(env_t *env) {
	gc(false);
	return BAMBOO_OK;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                           List Atom Manipulation                           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/**
 * Builds a pair atom from two other atoms.
 *
 * @param _car Left-hand side of the atom pair.
 * @param _cdr Right-hand side of the atom pair.
 *
 * @return Atom pair.
 */
atom_t cons(atom_t _car, atom_t _cdr) {
	allocation_t *alloc;
	atom_t pair;

	// Create a new allocation.
	alloc = (allocation_t *)malloc(sizeof(allocation_t));
	if (alloc == NULL) {
		bamboo_error_fatal(BAMBOO_ERROR_ALLOCATION, "Can't allocate structure for "
		                                            "garbage collector allocation tracking");
		return nil;
	}

	// Fill up the new allocation and push the linked list forward.
	alloc->mark = GC_TO_FREE;
	alloc->type = ALLOCATION_TYPE_PAIR;
	alloc->next = bamboo_allocations;
	bamboo_allocations = alloc;

	// Set up the pair atom.
	pair.type = ATOM_TYPE_PAIR;
	pair.value.pair = &alloc->pair;

	// Populate the pair.
	car(pair) = _car;
	cdr(pair) = _cdr;

	return pair;
}

/**
 * Checks if an atom is a list.
 *
 * @param expr Expression to be checked.
 *
 * @return TRUE if the expression is a valid list.
 */
bool listp(atom_t expr) {
	// Iterate over the expression until we reach the final nil atom.
	while (!nilp(expr)) {
		// If every atom of a list is not a pair, then it's not a list.
		if (expr.type != ATOM_TYPE_PAIR)
			return false;

		// Go to the next item.
		expr = cdr(expr);
	}

	// We've successfully iterated over the whole list, so it must be a list.
	return true;
}

/**
 * Calls a built-in function or a closure with the supplied arguments and get
 * the result.
 *
 * @param func   Built-in function atom.
 * @param args   Arguments to be passed to the function.
 * @param result Pointer to the result of the operation.
 *
 * @return BAMBOO_OK if the function call was successful.
 */
bamboo_error_t apply(atom_t func, atom_t args, atom_t *result) {
	env_t env;
	atom_t arg_names;
	atom_t body;

	// Check if we have a valid type.
	if (func.type == ATOM_TYPE_BUILTIN) {
		// Call the built-in function.
		return (*func.value.builtin)(args, result);
	} else if (func.type != ATOM_TYPE_CLOSURE) {
		return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
			"Function atom must be of type built-in or closure");
	}

	// Create a local environment for the closure and get its different parts.
	env = bamboo_env_new(car(func));
	arg_names = car(cdr(func));
	body = cdr(cdr(func));

	// Bind the local environment argument values.
	while (!nilp(arg_names)) {
		// Check if we have a variadic function and are dealing with the
		// argument that will hold the rest list.
		if (arg_names.type == ATOM_TYPE_SYMBOL) {
			bamboo_env_set(env, arg_names, args);
			args = nil;
			break;
		}

		// Check if the argument value list ends prematurely.
		if (nilp(args)) {
			return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
				"Argument value list ended prematurely");
		}

		// Assign the value to the argument.
		bamboo_env_set(env, car(arg_names), car(args));

		// Go to the next argument.
		arg_names = cdr(arg_names);
		args = cdr(args);
	}

	// Check if we still have argument values that weren't assigned.
	if (!nilp(args)) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"Too many argument values passed to the closure");
	}

	// Evaluate the body of the closure with our local environment.
	while (!nilp(body)) {
		bamboo_error_t err = bamboo_eval_expr(car(body), env, result);
		IF_ERROR(err)
			return err;

		// Go to the next element of the body.
		body = cdr(body);
	}

	return BAMBOO_OK;
}

/**
 * Creates a shallow copy of a list.
 *
 * @param list List to be copied.
 *
 * @return Shallow copy of the list.
 */
atom_t shallow_copy_list(atom_t list) {
	atom_t copied;
	atom_t tmp;

	// Check if we actually got nil.
	if (nilp(list))
		return nil;

	// Copy first item of the list and preserve the list root.
	copied = cons(car(list), nil);
	tmp = copied;

	// Iterate through the list copying each item into our own list.
	list = cdr(list);
	while (!nilp(list)) {
		cdr(tmp) = cons(car(list), nil);
		tmp = cdr(tmp);
		list = cdr(list);
	}

	return copied;
}

/**
 * Counts the number of elements in a list.
 *
 * @param list List atom to have its elements counted.
 *
 * @return Number of elements in the list. 0 if it isn't a valid list.
 */
uint16_t bamboo_list_count(atom_t list) {
	uint16_t count = 0;

	// Iterate over the list until we reach the final nil atom.
	while (!nilp(list)) {
		// If every atom of a list is not a pair, then it's not a list.
		if (list.type != ATOM_TYPE_PAIR)
			return 0;

		// Go to the next item.
		list = cdr(list);
		count++;
	}

	return count;
}

/**
 * Gets an element at an index from a list. Just like 'list-ref' in Scheme.
 *
 * @param list  List you want the element from.
 * @param index Index of the element you want.
 *
 * @return Element at the specified index of the list.
 */
atom_t bamboo_list_ref(atom_t list, uint16_t index) {
	// Iterate through the list.
	while (index--)
		list = cdr(list);

	// Get our element.
	return car(list);
}

/**
 * Sets the value of an element at an specific index in a list. Just like
 * 'list-set!' in Scheme.
 *
 * @param list  List that you want to edit.
 * @param index Index of the element you want to change.
 * @param value New element you want placed at the specified index of the list.
 */
void bamboo_list_set(atom_t list, uint16_t index, atom_t value) {
	// Iterate through the list.
	while (index--)
		list = cdr(list);

	// Set the new element's value.
	car(list) = value;
}

/**
 * Reverses the elements in a list.
 *
 * @param list Pointer to the list that will have its elements reversed.
 */
void bamboo_list_reverse(atom_t *list) {
	atom_t tail = nil;

	// Iterate over the list reversing its elements into tail.
	while (!nilp(*list)) {
		atom_t tmp;

		tmp = cdr(*list);
		cdr(*list) = tail;
		tail = *list;
		*list = tmp;
	}

	*list = tail;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                             Lexing and Parsing                             //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/**
 * A very simple lexer to find the beginning and the end of tokens in a string.
 *
 * @param str   String to be scanned for tokens.
 * @param token Pointer to the token structure that will hold the beginning and
 *              the end of a token string.
 *
 * @return BAMBOO_OK if a token was found. BAMBOO_ERROR_SYNTAX if we've reached
 *         the end of the string without finding any tokens.
 */
bamboo_error_t lex(const char *str, token_t *token) {
	const char *tmp = str;
	const char *wspace = " \t\r\n";
	const char *delim = "()\"; \t\r\n";
	const char *prefix = "()\'`\";";

	// Skip any leading whitespace.
	tmp += strspn(tmp, wspace);

	// Check if this was an empty line.
	if (tmp[0] == '\0') {
		token->start = tmp;
		token->end = tmp;

		return BAMBOO_EMPTY_LINE;
	}

	// Set the starting point of our token.
	token->start = tmp;

	// Check if the token is just a parenthesis or unquotation.
	if (strchr(prefix, tmp[0]) != NULL) {
		token->end = tmp + 1;
		return BAMBOO_OK;
	} else if (tmp[0] == ',') {
		// Detect the end of an unquote or unquote splicing.
		token->end = tmp + (tmp[1] == '@' ? 2 : 1);

		return BAMBOO_OK;
	}

	// Find the end of the token.
	token->end = tmp + strcspn(tmp, delim);
	return BAMBOO_OK;
}

/**
 * Parses an generic expression.
 *
 * @param input Expression as a string.
 * @param end   Pointer that will hold the point where the parsing stopped.
 * @param atom  Pointer to the atom object generated from the expression.
 *
 * @return BAMBOO_OK if the parsing was successful.
 */
bamboo_error_t bamboo_parse_expr(const char *input, const char **end,
								 atom_t *atom) {
	token_t token;
	bamboo_error_t err;

	// Pass the input through the lexer to get a token.
	err = lex(input, &token);
	IF_ERROR(err)
		return err;

	// Handle some special conditions.
	IF_SPECIAL_COND(err) {
		switch (err) {
		case BAMBOO_EMPTY_LINE:
			*atom = nil;
			return err;
		}
	}

	// Try to parse the toke we've found.
	switch (token.start[0]) {
	case '\"':
		// String
		return parse_string(&token, end, atom);
	case '(':
		// List beginning.
		return parse_list(token.end, end, atom);
	case ')':
		// List ending.
		return BAMBOO_PAREN_END;
	case '\'':
		// Parse quoted body.
		*atom = cons(bamboo_symbol("QUOTE"), cons(nil, nil));
		return bamboo_parse_expr(token.end, end, &car(cdr(*atom)));
	case '`':
		// Quasiquote
		*atom = cons(bamboo_symbol("QUASIQUOTE"), cons(nil, nil));
		return bamboo_parse_expr(token.end, end, &car(cdr(*atom)));
	case ',':
		// Unquote
		*atom = cons(bamboo_symbol(token.start[1] == '@' ?
			"UNQUOTE-SPLICING" : "UNQUOTE"), cons(nil, nil));
		return bamboo_parse_expr(token.end, end, &car(cdr(*atom)));
	case ';':
		// Comment ahead.
		return parse_comment(&token, end, atom);
	default:
		// Primitive it is.
		return parse_primitive(&token, end, atom);
	}

	// Well, something truly weird must have happened.
	return BAMBOO_ERROR_UNKNOWN;
}

/**
 * Parses primitives from a given token.
 *
 * @param token Pointer to token structure that holds the beginning and the end
 *              of a token string.
 * @param end   Pointer to the end of the last parsed part of the expression.
 * @param atom  Pointer to an atom structure that will hold the parsed atom.
 *
 * @return BAMBOO_OK if we were able to parse the token correctly.
 */
bamboo_error_t parse_primitive(const token_t *token, const char **end,
							   atom_t *atom) {
	char *buf;
	char *buftmp;
	const char *tmp;
#ifndef _tcstold
	int cret = 0;
#endif

	// Check if we are dealing with a hash expression.
	if (token->start[0] == '#')
		return parse_hash_expr(token, end, atom);

	// Check if we are dealing with a number of some kind.
	if (((token->start[0] >= '0') && (token->start[0] <= '9')) ||
			(token->start[0] == '+') || (token->start[0] == '-')) {
		int64_t integer;
		long double dfloat;

#if defined(_MSC_VER) && (_MSC_VER <= 1400)
		// Create a string with only the number.
		buf = strcpyse(token->start, token->end);

		// Check if we just have a simple + or - function call.
		if (((buf[0] == '+') || (buf[0] == '-')) &&
				(buf[1] == '\0')) {
			goto symbolparser;
		}
#endif  // _MSC_VER

		// Try to parse an integer.
#if defined(_MSC_VER) && (_MSC_VER <= 1400)
		if (!contains_point(buf)) {
			integer = _ttoi64(buf);

			free(buf);
			buf = token->end;
		} else {
			free(buf);
			buf = NULL;
		}
#else
		integer = strtoll(token->start, &buf, 0);
#endif  // _MSC_VER
		if (buf == token->end) {
#ifndef _WIN32_WCE
			// Check for overflows/underflows.
			if (errno == ERANGE) {
				*atom = nil;

				if (integer == LLONG_MAX) {
					return bamboo_error(BAMBOO_ERROR_NUM_OVERFLOW,
						"An integer overflow occured while parsing");
				} else if (integer == LLONG_MIN) {
					return bamboo_error(BAMBOO_ERROR_NUM_UNDERFLOW,
						"An integer underflow occured while parsing");
				}
			}
#endif  // _WIN32_WCE

			// Populate the atom.
			atom->type = ATOM_TYPE_INTEGER;
			atom->value.integer = integer;

			// Set the end pointer.
			*end = token->end;

			return BAMBOO_OK;
		}

		// Try to parse a float.
#ifndef strtold
		cret = sscanf(token->start, "%lg", &dfloat);
		if ((cret != 0) && (cret != EOF)) {
			buf = token->end;
		} else {
			buf = NULL;
		}
#else
		dfloat = strtold(token->start, &buf);
#endif  // strtold
		if (buf == token->end) {
#ifndef _WIN32_WCE
			// Check for overflows/underflows.
			if (errno == ERANGE) {
				*atom = nil;

				if (dfloat == HUGE_VALL) {
					return bamboo_error(BAMBOO_ERROR_NUM_OVERFLOW,
						"An float overflow occured while parsing");
				} else if (dfloat == LDBL_MIN) {
					return bamboo_error(BAMBOO_ERROR_NUM_UNDERFLOW,
						"An float underflow occured while parsing");
				}
			}
#endif  // _WIN32_WCE

			// Populate the atom.
			atom->type = ATOM_TYPE_FLOAT;
			atom->value.dfloat = dfloat;

			// Set the end pointer.
			*end = token->end;

			return BAMBOO_OK;
		}
	}

#if defined(_MSC_VER) && (_MSC_VER <= 1400)
symbolparser:
#endif  // _MSC_VER

	// Allocate string for symbol upper-case conversion.
	buf = (char *)malloc(sizeof(char) * (token->end - token->start + 1));
	if (buf == NULL) {
		return bamboo_error(BAMBOO_ERROR_ALLOCATION, "Can't allocate string "
			"for symbol upper-case conversion");
	}

	// Convert the symbol to upper-case.
	buftmp = buf;
	tmp = token->start;
	while (tmp != token->end)
		*buftmp++ = (char)toupper((int)*tmp++);
	*buftmp = '\0';

	// Check if we are dealing with a NIL symbol.
	if (strcmp(buf, "NIL") == 0) {
		*atom = nil;
	} else {
		// Looks like a regular symbol.
		*atom = bamboo_symbol(buf);
	}

	// Set the end pointer.
	*end = token->end;

	// Clean up and return OK.
	free(buf);
	return BAMBOO_OK;
}

/**
 * Parses primitives that begin with the special hash (#) notation.
 *
 * @param token Pointer to token structure that holds the beginning and the end
 *              of a token string.
 * @param end   Pointer to the end of the last parsed part of the expression.
 * @param atom  Pointer to an atom structure that will hold the parsed atom.
 *
 * @return BAMBOO_OK if we were able to parse the token correctly.
 */
bamboo_error_t parse_hash_expr(const token_t *token, const char **end,
							   atom_t *atom) {
	// Check if we don't have an invalid syntax.
	if ((token->start + 1) == token->end) {
		return bamboo_error(BAMBOO_ERROR_SYNTAX,
			"Special values must have at least one character after the # "
			"character");
	}

	// Check which kind of special value we are dealing with.
	switch (token->start[1]) {
	case 'F':
	case 'f':
		*atom = bamboo_boolean(false);
		*end = token->end;
		return BAMBOO_OK;
	case 'T':
	case 't':
		*atom = bamboo_boolean(true);
		*end = token->end;
		return BAMBOO_OK;
	default:
		return bamboo_error(BAMBOO_ERROR_SYNTAX,
			"Invalid type of hash expression");
	}
}

/**
 * Parses a string from a given token.
 *
 * @param token Pointer to token structure that holds the beginning and the end
 *              of a token string.
 * @param end   Pointer to the end of the last parsed part of the expression.
 * @param atom  Pointer to an atom structure that will hold the parsed atom.
 *
 * @return BAMBOO_OK if we were able to parse the token correctly.
 */
bamboo_error_t parse_string(const token_t *token, const char **end,
							atom_t *atom) {
	size_t len;
	char *buf;
	char *buftmp;
	const char *tmp;

	// Calculate the length of our string.
	tmp = token->end;
	len = 0;
	while (*tmp != '\"') {
		// Check if the string is never terminated.
		if (*tmp == '\0') {
			*end = tmp;
			return bamboo_error(BAMBOO_ERROR_SYNTAX, "String never terminated");
		}

		tmp++;
		len++;
	}

	// Allocate space for our string.
	buf = (char *)malloc((len + 1) * sizeof(char));
	if (buf == NULL) {
		return bamboo_error(BAMBOO_ERROR_ALLOCATION, "Can't allocate string "
			"for string atom");
	}

	// Pre-terminate the string.
	buf[len] = '\0';

	// Copy the string into our buffer.
	tmp = token->end;
	buftmp = buf;
	while (*tmp != '\"') {
		*buftmp = *tmp++;
		buftmp++;
	}

	// Make the atom and free our buffer.
	*end = ++tmp;
	*atom = bamboo_string(buf);
	free(buf);

	return BAMBOO_OK;
}

/**
 * Parses an list expression.
 *
 * @param input Pointer to the list expression string starting at the first
 *              character after the opening parenthesis.
 * @param end   Pointer to the end of the last parsed part of the expression.
 * @param atom  Pointer to the atom object that will hold the result of the
 *              parsing operation.
 *
 * @return BAMBOO_OK if the parsing was successful.
 */
bamboo_error_t parse_list(const char *input, const char **end, atom_t *atom) {
	token_t token;
	bamboo_error_t err;
	atom_t tmp_atom;
	atom_t *last_atom;
	bool is_pair;

	// Reset values.
	*atom = nil;
	tmp_atom = nil;
	last_atom = atom;
	token.end = input;
	is_pair = false;

	while (!(err = lex(token.end, &token))) {
		// Check if we have a pair.
		if (token.start[0] == '.') {
			token_t test_token;

			// Check if the pair separator is the first token in the atom.
			if (nilp(*atom)) {
				return bamboo_error(BAMBOO_ERROR_SYNTAX,
					"Pair delimiter without left-hand atom");
			}

			// Check if we have something after the pair separator.
			err = lex(token.end, &test_token);
			if (err || (test_token.start[0] == ')')) {
				return bamboo_error(BAMBOO_ERROR_SYNTAX,
					"Pair ends without right-hand atom");
			}

			// Move to the next token.
			is_pair = true;
			continue;
		}

		// Parse the next token of list.
		err = bamboo_parse_expr(token.start, &(token.end), &tmp_atom);
		IF_SPECIAL_COND(err) {
			// We are dealing with a special condition.
			switch (err) {
			case BAMBOO_PAREN_END:
				// We've reached the end of a list.
				*end = token.end;
				return BAMBOO_OK;
			case BAMBOO_COMMENT:
			case BAMBOO_EMPTY_LINE:
				continue;
			default:
				// We haven't implemented this new special condition apparently.
				return bamboo_error(err, "Unknown special condition");
			}
		} else if (err) {
			// Looks like we've errored out.
			return err;
		}

		// Concatenate the atom to the list.
		if (nilp(*atom)) {
			// First item in a list.
			*atom = cons(tmp_atom, nil);
			last_atom = &cdr(*atom);
		} else {
			// Check if we are trying to append something to a pair.
			if (!nilp(*last_atom)) {
				return bamboo_error(BAMBOO_ERROR_SYNTAX,
					"Tried to append an atom to a pair");
			}

			// Check if we are dealing with a pair.
			if (is_pair) {
				*last_atom = tmp_atom;
				is_pair = false;

				continue;
			}

			// Append a new item to the list.
			*last_atom = cons(tmp_atom, nil);
			last_atom = &cdr(*last_atom);
		}
	}

	return BAMBOO_OK;
}

/**
 * Parses a comment token.
 *
 * @param token Pointer to token structure that holds the beginning and the end
 *              of a token string.
 * @param end   Pointer to the end of the last parsed part of the expression.
 * @param atom  Pointer to an atom structure that will hold the parsed atom.
 *
 * @return BAMBOO_OK if we were able to parse the token correctly.
 */
bamboo_error_t parse_comment(const token_t *token, const char **end,
							 atom_t *atom) {
	const char *tmp;

	// We already know this thing has no future, so...
	*atom = nil;

	// Skip to the nearest newline character or string terminator.
	tmp = token->end;
	while ((*tmp != '\0') && (*tmp != '\n')) {
		tmp++;
	}

	// We've reached the end of the comment.
	*end = tmp;

	return BAMBOO_COMMENT;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                                 Evaluation                                 //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/**
 * Evaluates an expression in a given environment.
 *
 * This function used to be so simple, but used recursion and caused stack
 * overflows in deep recursions, so it had to be re-written, for the older
 * version check the commit 0d1bc6c.
 *
 * @param expr   Expression to be evaluated.
 * @param env    Environment list to use for this evaluation.
 * @param result Pointer to the resulting atom of the evaluation.
 *
 * @return BAMBOO_OK if the evaluation was successful.
 *
 * @see https://lwh.jp/lisp/continuations.html
 */
bamboo_error_t bamboo_eval_expr(atom_t expr, env_t env, atom_t *result) {
	frame_t stack;
	bamboo_error_t err;

	// Clean slate.
	err = BAMBOO_OK;
	stack = nil;
	*result = nil;

	do {
		// Should we trigger the garbage collector?
		if (++bamboo_gc_iter_counter == GC_ITER_COUNT_SWEEP) {
			// Mark our current parameters as in use for good measure.
			gc_mark(expr);
			gc_mark(env);
			gc_mark(stack);

			// Collect the garbage and reset the iteration counter.
			gc(true);
			bamboo_gc_iter_counter = 0;
		}

		// Check if the expression is simple and doesn't require manipulation.
		if (expr.type == ATOM_TYPE_SYMBOL) {
			// A symbol from the environment was requested.
			err = bamboo_env_get(env, expr, result);
		} else if (expr.type != ATOM_TYPE_PAIR) {
			// Literals always evaluate to themselves.
			*result = expr;
		} else {
			atom_t op;
			atom_t args;

			// Get the op and its arguments.
			op = car(expr);
			args = cdr(expr);

			// Check if it's a special form to be evaluated.
			if (op.type == ATOM_TYPE_SYMBOL) {
				// Check which special form we need to evaluate.
				if (strcmp(*op.value.symbol, "QUOTE") == 0) {
					// Check if we have the single required arguments.
					if (bamboo_list_count(args) != 1) {
						return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
							"Wrong number of arguments. Expected 1");
					}

					// Return the arguments without evaluating.
					*result = car(args);
				} else if (strcmp(*op.value.symbol, "IF") == 0) {
					// Check if we have the right number of arguments.
					if (bamboo_list_count(args) != 3) {
						return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
							"Wrong number of arguments. Expected 3");
					}

					// Place it in the stack for later evaluation.
					stack = new_stack_frame(stack, env, cdr(args));
					bamboo_list_set(stack, STACK_EVAL_OP_INDEX, op);
					expr = car(args);

					continue;
				} else if (strcmp(*op.value.symbol, "DEFINE") == 0) {
					atom_t symbol;

					// Check if we have both of the required 2 arguments.
					if (bamboo_list_count(args) < 2) {
						return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
							"Wrong number of arguments. Expected at least 2");
					}

					// Get the reference symbol.
					symbol = car(args);
					switch (symbol.type) {
					case ATOM_TYPE_SYMBOL:
						// Defining a simple symbol.
						stack = new_stack_frame(stack, env, nil);
						bamboo_list_set(stack, STACK_EVAL_OP_INDEX, op);
						bamboo_list_set(stack, STACK_EVAL_ARGS_INDEX, symbol);
						expr = car(cdr(args));
						continue;
					case ATOM_TYPE_PAIR:
						// Build a closure since we are using the define lambda
						// shorthand.
						err = bamboo_closure(env, cdr(symbol), cdr(args),
							result);
						symbol = car(symbol);

						// Check if we actually have a symbol for closure name.
						if (symbol.type != ATOM_TYPE_SYMBOL) {
							return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
								"First element of argument 0 list should be a "
								"symbol");
						}

						// Put the symbol in th environment.
						(void)bamboo_env_set(env, symbol, *result);
						*result = symbol;
						break;
					default:
						return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
							"Argument 0 should be of type symbol or pair");
					}
				} else if (strcmp(*op.value.symbol, "LAMBDA") == 0) {
					// Check if we have both of the required 2 arguments.
					if (bamboo_list_count(args) < 2) {
						return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
							"Wrong number of arguments. Expected at least 2");
					}

					// Make the closure.
					err = bamboo_closure(env, car(args), cdr(args), result);
				} else if (strcmp(*op.value.symbol, "DEFINE-MACRO") == 0) {
					atom_t name;
					atom_t macro;

					// Check if we have both of the required 2 arguments.
					if (bamboo_list_count(args) < 2) {
						return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
							"Wrong number of arguments. Expected at least 2");
					}

					// Check if the first argument is defined like a define
					// lambda shorthand.
					if (car(args).type != ATOM_TYPE_PAIR) {
						return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
							"First argument must be a pair or a list like when "
							"defining a function using only define");
					}

					// Get macro name.
					name = car(car(args));
					if (name.type != ATOM_TYPE_SYMBOL) {
						return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
							"Macro name must be of type symbol");
					}

					// Make the macro.
					err = bamboo_closure(env, cdr(car(args)), cdr(args),
						&macro);
					IF_NOT_ERROR(err) {
						// Return the name symbol and add the macro to the
						// environment.
						macro.type = ATOM_TYPE_MACRO;
						*result = name;
						(void)bamboo_env_set(env, name, macro);
					}
				} else if (strcmp(*op.value.symbol, "APPLY") == 0) {
					// Check if we have both of the required 2 arguments.
					if (bamboo_list_count(args) < 2) {
						return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
							"Wrong number of arguments. Expected at least 2");
					}

					// Evaluate the apply by the magic of the stack.
					stack = new_stack_frame(stack, env, cdr(args));
					bamboo_list_set(stack, STACK_EVAL_OP_INDEX, op);
					expr = car(args);
					continue;
				} else {
					goto push;
				}
			} else if (op.type == ATOM_TYPE_BUILTIN) {
				// Execute a built-in function.
				err = (*op.value.builtin)(args, result);
			} else {
				// Handle a closure or macro.
push:
				stack = new_stack_frame(stack, env, args);
				expr = op;
				continue;
			}
		}

		// Are we at the end of the stack?
		if (nilp(stack))
			break;

		// Get the return value from the stack frame evaluation.
		IF_NOT_ERROR(err)
			err = eval_expr_return(&stack, &expr, &env, result);
	} while (err <= BAMBOO_OK);

	return err;
}

/**
 * Creates a brand new virtual stack frame used to allow us to not run into CPU
 * stack overflows while evaluating expressions.
 *
 * To be honest I have no clue how this whole thing is working, I just want to
 * make sure we don't run into stack overflows.
 *
 * This is how the stack frame structure should look like:
 * (parent env evaluated-op (pending-arg...) (evaluated-arg...) (body...))
 *
 * @param parent Parent stack frame.
 * @param env    Environment for the stack frame to be evaluated in.
 * @param tail   Rest of the stack frame to be evaluated later.
 *
 * @return A brand new stack frame atom.
 *
 * @see https://lwh.jp/lisp/continuations.html
 */
frame_t new_stack_frame(frame_t parent, env_t env, atom_t tail) {
	return cons(parent, cons(env, cons(nil /* evaluated-op */, cons(tail,
   		cons(nil /* evaluated-args */, cons(nil /* body */, nil))))));
}

/**
 * Grabs the current expression to be evaluated from the body of the stack
 * frame.
 *
 * To be honest I have no clue how this whole thing is working, I just want to
 * make sure we don't run into stack overflows.
 *
 * @param stack Pointer to the stack frame we are currently evaluating.
 * @param expr  Pointer to the expression that will be grabbed from the stack to
 *              be evaluated later.
 * @param env   Pointer to the environment where the current expression will be
 *              evaluated in. This will also come from our stack frame.
 *
 * @return BAMBOO_OK if everything went fine.
 *
 * @see https://lwh.jp/lisp/continuations.html
 */
bamboo_error_t eval_expr_exec(frame_t *stack, atom_t *expr, env_t *env) {
	atom_t body;

	// Get the different parts of the stack.
	*env = bamboo_list_ref(*stack, STACK_ENV_INDEX);
	body = bamboo_list_ref(*stack, STACK_BODY_INDEX);
	*expr = car(body);

	// Check the next item in line.
	body = cdr(body);
	if (nilp(body)) {
		// We've reached the of this stack. Pop the stack to its parent.
		*stack = car(*stack);
	} else {
		// Advance the body of the atom to the next item in line.
		bamboo_list_set(*stack, STACK_BODY_INDEX, body);
	}

	return BAMBOO_OK;
}

/**
 * Binds function arguments into the new stack frame environment if they haven't
 * been bound already.
 *
 * To be honest I have no clue how this whole thing is working, I just want to
 * make sure we don't run into stack overflows.
 *
 * @param stack Pointer to the stack frame we are currently evaluating.
 * @param expr  Pointer to the expression that will be grabbed from the stack to
 *              be evaluated later.
 * @param env   Pointer to the environment where the current expression will be
 *              evaluated in. This will also come from our stack frame.
 *
 * @return BAMBOO_OK if everything went fine.
 *
 * @see https://lwh.jp/lisp/continuations.html
 */
bamboo_error_t eval_expr_bind(frame_t *stack, atom_t *expr, env_t *env) {
	atom_t op;
	atom_t args;
	atom_t arg_names;
	atom_t body;

	// If we have anything in the body just return it then.
	body = bamboo_list_ref(*stack, STACK_BODY_INDEX);
	if (!nilp(body))
		return eval_expr_exec(stack, expr, env);

	// Get the op and function arguments from the stack frame.
	op = bamboo_list_ref(*stack, STACK_EVAL_OP_INDEX);
	args = bamboo_list_ref(*stack, STACK_EVAL_ARGS_INDEX);

	// Get all parameters from the stack frame.
	*env = bamboo_env_new(car(op));
	arg_names = car(cdr(op));
	body = cdr(cdr(op));
	bamboo_list_set(*stack, STACK_ENV_INDEX, *env);
	bamboo_list_set(*stack, STACK_BODY_INDEX, body);

	// Go through the arguments binding them to the environment.
	while (!nilp(arg_names)) {
		// Looks like we just have a symbol, nothing else to do here.
		if (arg_names.type == ATOM_TYPE_SYMBOL) {
			bamboo_env_set(*env, arg_names, args);
			args = nil;
			break;
		}

		// Not enough argument values given the amount of argument names.
		if (nilp(args)) {
 			return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
 				"Argument value list ended prematurely");
		}

		// Push the argument into the environment.
		bamboo_env_set(*env, car(arg_names), car(args));

		// Move to the next argument.
		arg_names = cdr(arg_names);
		args = cdr(args);
	}

	// Check if we have arguments remaining.
	if (!nilp(args)) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"Arguments left over after iterating through argument names");
	}

	bamboo_list_set(*stack, STACK_EVAL_ARGS_INDEX, nil);
	return eval_expr_exec(stack, expr, env);
}

/**
 * Once all arguments have been evaluated, this function is responsible for
 * generating an expression to call a builtin, or delegating to 'eval_do_bind'.
 *
 * To be honest I have no clue how this whole thing is working, I just want to
 * make sure we don't run into stack overflows.
 *
 * @param stack Pointer to the stack frame we are currently evaluating.
 * @param expr  Pointer to the expression that will be grabbed from the stack to
 *              be evaluated later.
 * @param env   Pointer to the environment where the current expression will be
 *              evaluated in. This will also come from our stack frame.
 *
 * @return BAMBOO_OK if everything went fine.
 *
 * @see https://lwh.jp/lisp/continuations.html
 */
bamboo_error_t eval_expr_apply(frame_t *stack, atom_t *expr, env_t *env) {
	atom_t op;
	atom_t args;

	// Get the op and arguments from the stack frame.
	op = bamboo_list_ref(*stack, STACK_EVAL_OP_INDEX);
	args = bamboo_list_ref(*stack, STACK_EVAL_ARGS_INDEX);

	// Reverse the arguments if we have any.
	if (!nilp(args)) {
		bamboo_list_reverse(&args);
		bamboo_list_set(*stack, STACK_EVAL_ARGS_INDEX, args);
	}

	// Handle the apply special form.
	if (op.type == ATOM_TYPE_SYMBOL) {
		if (strcmp(*op.value.symbol, "APPLY") == 0) {
			// Replace the current frame.
			*stack = car(*stack);
			*stack = new_stack_frame(*stack, *env, nil);
			op = car(args);
			args = car(cdr(args));

			// Check if we actually have an arguments list.
			if (!listp(args)) {
				return bamboo_error(BAMBOO_ERROR_SYNTAX,
					"Arguments atom must be of list type");
			}

			// Go to the next one.
			bamboo_list_set(*stack, STACK_EVAL_OP_INDEX, op);
			bamboo_list_set(*stack, STACK_EVAL_ARGS_INDEX, args);
		}
	}

	// Handle built-ins.
	if (op.type == ATOM_TYPE_BUILTIN) {
		*stack = car(*stack);
		*expr = cons(op, args);

		return BAMBOO_OK;
	} else if (op.type != ATOM_TYPE_CLOSURE) {
		// Looks like we don't have anything that's "apply"able.
		return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
			"Applyable op must be either a built-in or a closure");
	}

	return eval_expr_bind(stack, expr, env);
}

/**
 * Once an expression has been evaluated, this function is responsible for
 * storing the result, which is either an op, an argument, or an
 * intermediate body expression, and fetching the next expression to evaluate.
 *
 * To be honest I have no clue how this whole thing is working, I just want to
 * make sure we don't run into stack overflows.
 *
 * @param stack  Pointer to the stack frame we are currently evaluating.
 * @param expr   Pointer to the expression that will be grabbed from the stack
 *               to be evaluated later.
 * @param env    Pointer to the environment where the current expression will be
 *               evaluated in. This will also come from our stack frame.
 * @param result Pointer to the return value of the evaluated stack frame.
 *
 * @return BAMBOO_OK if everything went fine.
 *
 * @see https://lwh.jp/lisp/continuations.html
 */
bamboo_error_t eval_expr_return(frame_t *stack, atom_t *expr, env_t *env,
								atom_t *result) {
	atom_t op;
	atom_t args;
	atom_t body;

	// Gets the parameters from the stack frame.
	*env = bamboo_list_ref(*stack, STACK_ENV_INDEX);
	op = bamboo_list_ref(*stack, STACK_EVAL_OP_INDEX);
	body = bamboo_list_ref(*stack, STACK_BODY_INDEX);

	// Check if we are still running a procedure. If so, just ignore the result.
	if (!nilp(body))
		return eval_expr_apply(stack, expr, env);

	// Check in which phase of evaluation we are currently at.
	if (nilp(op)) {
		// Finished evaluating op.
		op = *result;
		bamboo_list_set(*stack, STACK_EVAL_OP_INDEX, op);

		// Are we doing a macro?
		if (op.type == ATOM_TYPE_MACRO) {
			// Don't evaluate macro arguments.
			args = bamboo_list_ref(*stack, STACK_PENDING_ARGS_INDEX);

			*stack = new_stack_frame(*stack, *env, nil);
			op.type = ATOM_TYPE_CLOSURE;
			bamboo_list_set(*stack, STACK_EVAL_OP_INDEX, op);
			bamboo_list_set(*stack, STACK_EVAL_ARGS_INDEX, args);

			return eval_expr_bind(stack, expr, env);
		}
	} else if (op.type == ATOM_TYPE_SYMBOL) {
		// Finished working on a special form.
		if (strcmp(*op.value.symbol, "DEFINE") == 0) {
			atom_t symbol;

			symbol = bamboo_list_ref(*stack, STACK_EVAL_ARGS_INDEX);
			(void)bamboo_env_set(*env, symbol, *result);
			*stack = car(*stack);
			*expr = cons(bamboo_symbol("QUOTE"), cons(symbol, nil));

			return BAMBOO_OK;
		} else if (strcmp(*op.value.symbol, "IF") == 0) {
			args = bamboo_list_ref(*stack, STACK_PENDING_ARGS_INDEX);

			// Choose which path to go for an if statement.
			if ((result->type == ATOM_TYPE_BOOLEAN) && (!result->value.boolean)) {
				*expr = car(cdr(args));
			} else {
				*expr = car(args);
			}

			*stack = car(*stack);
			return BAMBOO_OK;
		} else {
			goto store_argument;
		}
	} else if (op.type == ATOM_TYPE_MACRO) {
		// Finished evaluating macro.
		*expr = *result;
		*stack = car(*stack);

		return BAMBOO_OK;
	} else {
store_argument:
		// Store the evaluated argument.
		args = bamboo_list_ref(*stack, STACK_EVAL_ARGS_INDEX);
		bamboo_list_set(*stack, STACK_EVAL_ARGS_INDEX, cons(*result, args));
	}

	// Get the next set of arguments to evaluate.
	args = bamboo_list_ref(*stack, STACK_PENDING_ARGS_INDEX);
	if (nilp(args)) {
		// No more arguments left to evaluate.
		return eval_expr_apply(stack, expr, env);
	}

	// Evaluate the next argument.
	*expr = car(args);
	bamboo_list_set(*stack, STACK_PENDING_ARGS_INDEX, cdr(args));

	return BAMBOO_OK;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                             Garbage Collection                             //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/**
 * Marks a whole tree of pairs as "in use" so that the garbage collector won't
 * free them up.
 *
 * @param root Root of the pair tree to be marked as "in use".
 */
void gc_mark(atom_t root) {
	allocation_t *alloc;

	//  Get the allocation from the atom.
	switch (root.type) {
	case ATOM_TYPE_PAIR:
	case ATOM_TYPE_CLOSURE:
	case ATOM_TYPE_MACRO:
		alloc = (allocation_t *)((size_t)root.value.pair -
			offsetof(allocation_t, pair));
		break;
	case ATOM_TYPE_SYMBOL:
		alloc = (allocation_t *)((size_t)root.value.symbol -
			offsetof(allocation_t, str));
		break;
	case ATOM_TYPE_STRING:
		alloc = (allocation_t *)((size_t)root.value.str -
			offsetof(allocation_t, str));
		break;
	default:
		// Ignore non-"garbage collectable" types.
		return;
	}

	// If it's already marked, then there's nothing to do.
	if (alloc->mark == GC_IN_USE)
		return;

	// Mark it as "in use".
	alloc->mark = GC_IN_USE;

	// Traverse the pair marking everything as "in use".
	gc_mark(car(root));
	gc_mark(cdr(root));
}

/**
 * Go through the allocation linked list collecting the garbage.
 *
 * @param respect_marks Should we respect the "in use" marks in allocations? If
 *                      set to FALSE this will deallocate everything.
 */
void gc(bool respect_marks) {
	allocation_t *alloc;
	allocation_t **tmp;

	// Make sure we don't trash our global symbols list.
	if (respect_marks)
		gc_mark(bamboo_symbol_table);

	// Free up all unmarked allocations.
	tmp = &bamboo_allocations;
	while (*tmp != NULL) {
		alloc = *tmp;

		// Check if it's marked to be freed.
		if ((alloc->mark == GC_TO_FREE) | !respect_marks) {
			// Free it up!
			*tmp = alloc->next;
			if (alloc->type == ALLOCATION_TYPE_STRING)
				free(alloc->str);
			free(alloc);

			continue;
		}

		// Let's go to the next item in the allocation list.
		tmp = &alloc->next;
	}

	// Clear all the marks for the next round.
	alloc = bamboo_allocations;
	while (alloc != NULL) {
		alloc->mark = GC_TO_FREE;
		alloc = alloc->next;
	}
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                                 Debugging                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/**
 * Gets the string representation of the contents of an atom.
 *
 * @param buf  Pointer to a string that will be allocated by this function which
 *             will return the atom representation string. NOTE: Remember that
 *             you're responsible for freeing this pointer later.
 * @param atom Atom to have its contents represented.
 */
void bamboo_expr_str(char **buf, atom_t atom) {
	char *tmp;
	size_t buflen = 0;

	switch (atom.type) {
	case ATOM_TYPE_NIL:
		// nil
		*buf = strdup("nil");
		break;
	case ATOM_TYPE_SYMBOL:
		// Symbol
		*buf = strdup(*atom.value.symbol);
		break;
	case ATOM_TYPE_INTEGER:
		// Integer
#if defined(_MSC_VER) && (_MSC_VER <= 1400)
		buflen = I64_MAX_DIGITS;
#else
		buflen = snprintf(NULL, 0, "%lld", atom.value.integer);
#endif  // _MSC_VER

		*buf = (char *)malloc((buflen + 1) * sizeof(char));
		if (*buf == NULL) {
			bamboo_error_fatal(BAMBOO_ERROR_ALLOCATION, "Can't allocate string to "
			                                            "represent integer atom");
		}

#if defined(_MSC_VER) && (_MSC_VER <= 1400)
		snprintf(*buf, buflen + 1, "%I64d", atom.value.integer);
#else
		snprintf(*buf, buflen + 1, "%lld", atom.value.integer);
#endif  // _MSC_VER
		break;
	case ATOM_TYPE_FLOAT:
		// Float
#if defined(_MSC_VER) && (_MSC_VER <= 1400)
		buflen = LONGDOUBLE_MAX_DIGITS;
#else
		buflen = snprintf(NULL, 0, "%Lg", atom.value.dfloat);
#endif  // _MSC_VER

		*buf = (char *)malloc((buflen + 1) * sizeof(char));
		if (*buf == NULL) {
			bamboo_error_fatal(BAMBOO_ERROR_ALLOCATION, "Can't allocate string to "
			                                            "represent float atom");
		}

		snprintf(*buf, buflen + 1, "%Lg", atom.value.dfloat);
		break;
	case ATOM_TYPE_BOOLEAN:
		// Boolean
		buflen = 2;
		*buf = (char *)malloc((buflen + 1) * sizeof(char));
		if (*buf == NULL) {
			bamboo_error_fatal(BAMBOO_ERROR_ALLOCATION, "Can't allocate string to "
			                                            "represent boolean atom");
		}

		tmp = *buf;
		tmp[0] = '#';
		tmp[1] = (atom.value.boolean) ? 't' : 'f';
		tmp[2] = '\0';
		break;
	case ATOM_TYPE_STRING:
		// String
		buflen = strlen(*atom.value.str) + 2;
		*buf = (char *)malloc((buflen + 1) * sizeof(char));
		if (*buf == NULL) {
			bamboo_error_fatal(BAMBOO_ERROR_ALLOCATION, "Can't allocate string to "
			                                            "represent string atom");
		}

		snprintf(*buf, buflen + 1, "\"%s\"",
			*atom.value.str);
		break;
	case ATOM_TYPE_PAIR:
		// Pair
		buflen = 3;

		// Start by allocating the basic string and adding the leading paren.
		*buf = (char *)malloc(buflen * sizeof(char));
		if (*buf == NULL)
			goto err_alloc_pair_str;
		(*buf)[0] = '(';
		(*buf)[1] = '\0';

		// Grab the first item of the pair and append it to the string.
		bamboo_expr_str(&tmp, car(atom));
		buflen += strlen(tmp);
		*buf = (char *)realloc(*buf, buflen * sizeof(char));
		if (*buf == NULL)
			goto err_alloc_pair_str;
		strcat(*buf, tmp);
		free(tmp);

		// Iterate over the right-hand side of the pair since it may be a list.
		atom = cdr(atom);
		while (!nilp(atom)) {
			// Check if we are in a list.
			if (atom.type == ATOM_TYPE_PAIR) {
				bamboo_expr_str(&tmp, car(atom));

				buflen += strlen(tmp) + 1;
				*buf = (char *)realloc(*buf, buflen * sizeof(char));
				if (*buf == NULL)
					goto err_alloc_pair_str;

				strcat(*buf, " ");
				strcat(*buf, tmp);

				free(tmp);
				atom = cdr(atom);
			} else {
				// It was just a simple pair.
				bamboo_expr_str(&tmp, atom);

				buflen += strlen(tmp) + 3;
				*buf = (char *)realloc(*buf, buflen * sizeof(char));
				if (*buf == NULL)
					goto err_alloc_pair_str;

				strcat(*buf, " . ");
				strcat(*buf, tmp);

				free(tmp);
				break;
			}
		}

		// Append the last paren and terminate the string.
		strcat(*buf, ")");
		break;
err_alloc_pair_str:
	bamboo_error_fatal(BAMBOO_ERROR_ALLOCATION, "Can't allocate string to "
	                                            "represent pair atom");
		break;
	case ATOM_TYPE_BUILTIN:
		// Built-in Function
#if defined(_MSC_VER) && (_MSC_VER <= 1400)
		buflen = 32;
#else
		buflen = snprintf(NULL, 0, "#<BUILTIN:%p>", atom.value.builtin);
#endif  // _MSC_VER

		*buf = (char *)malloc((buflen + 1) * sizeof(char));
		if (*buf == NULL) {
			bamboo_error_fatal(BAMBOO_ERROR_ALLOCATION, "Can't allocate string to "
			                                            "represent built-in function atom");
		}

		snprintf(*buf, buflen + 1, "#<BUILTIN:%p>", atom.value.builtin);
		break;
	case ATOM_TYPE_CLOSURE:
		// Closure
		buflen = 14;

		// Allocate the string and begin with the type identifier.
		*buf = (char *)malloc(buflen * sizeof(char));
		if (*buf == NULL)
			goto err_alloc_closure_str;
		(*buf)[0] = '\0';
		strcat(*buf, "#<FUNCTION:");

		// Do we have arguments?
		if (!nilp(car(cdr(atom)))) {
			bamboo_expr_str(&tmp, car(cdr(atom)));

			buflen += strlen(tmp);
			*buf = (char *)realloc(*buf, buflen * sizeof(char));
			if (*buf == NULL)
				goto err_alloc_closure_str;

			strcat(*buf, tmp);
			free(tmp);
		}

		// Append the closure body.
		strcat(*buf, " ");
		bamboo_expr_str(&tmp, cdr(cdr(atom)));
		buflen += strlen(tmp);
		*buf = (char *)realloc(*buf, buflen * sizeof(char));
		if (*buf == NULL)
			goto err_alloc_closure_str;
		strcat(*buf, tmp);
		free(tmp);

		// Finalize the string.
		strcat(*buf, ">");
		break;
err_alloc_closure_str:
	bamboo_error_fatal(BAMBOO_ERROR_ALLOCATION, "Can't allocate string to "
	                                            "represent closure atom");
		break;
	case ATOM_TYPE_MACRO:
		// Macro
		buflen = 11;

		// Allocate the string and begin with the type identifier.
		*buf = (char *)malloc(buflen * sizeof(char));
		if (*buf == NULL)
			goto err_alloc_macro_str;
		(*buf)[0] = '\0';
		strcat(*buf, "#<MACRO:");

		// Do we have arguments?
		if (!nilp(car(cdr(atom)))) {
			bamboo_expr_str(&tmp, car(cdr(atom)));

			buflen += strlen(tmp);
			*buf = (char *)realloc(*buf, buflen * sizeof(char));
			if (*buf == NULL)
				goto err_alloc_macro_str;

			strcat(*buf, tmp);
			free(tmp);
		}

		// Append the macro body.
		strcat(*buf, " ");
		bamboo_expr_str(&tmp, cdr(cdr(atom)));
		buflen += strlen(tmp);
		*buf = (char *)realloc(*buf, buflen * sizeof(char));
		if (*buf == NULL)
			goto err_alloc_macro_str;
		strcat(*buf, tmp);
		free(tmp);

		// Finalize the string.
		strcat(*buf, ">");
		break;
err_alloc_macro_str:
	bamboo_error_fatal(BAMBOO_ERROR_ALLOCATION, "Can't allocate string to "
	                                            "represent macro atom");
		break;
	case ATOM_TYPE_POINTER:
		// Pointer
#if defined(_MSC_VER) && (_MSC_VER <= 1400)
		buflen = 32;
#else
		buflen = snprintf(NULL, 0, "#<POINTER:%p>", atom.value.pointer);
#endif  // _MSC_VER

		*buf = (char *)malloc((buflen + 1) * sizeof(char));
		if (*buf == NULL) {
			bamboo_error_fatal(BAMBOO_ERROR_ALLOCATION, "Can't allocate string to "
			                                            "represent pointer atom");
		}

		snprintf(*buf, buflen + 1, "#<POINTER:%p>", atom.value.pointer);
		break;
	default:
		// Unknown
		*buf = strdup("Unknown type. Don't know how to display this");
	}
}

/**
 * Prints the contents of an atom in a standard way.
 *
 * @param atom Atom to have its contents printed.
 */
void bamboo_print_expr(atom_t atom) {
	char *buf;

	// Get the atom content as a string and print it.
	bamboo_expr_str(&buf, atom);
	putstr(buf);

	// Free up the allocated string.
	free(buf);
}

/**
 * Prints all of the tokens found in a string.
 *
 * @param str String to debug tokens in.
 */
void bamboo_print_tokens(const char *str) {
	token_t token;
	bamboo_error_t err;

	// Go through tokens in string.
	token.end = str;
	while (!(err = lex(token.end, &token))) {
		char *buf;
		int i;

		// Allocate string for the token string.
		buf = (char *)malloc(((token.end - token.start) + 1) * sizeof(char));
		if (buf == NULL) {
			bamboo_error_fatal(BAMBOO_ERROR_ALLOCATION,
			                   "Can't allocate string for token printing");
			return;
		}

		// Get the token string from the token structure.
		for (i = 0; i < (token.end - token.start); i++) {
			buf[i] = token.start[i];
		}
		buf[i] = '\0';

		// Print the token and free the string.
		printf("'%s' ", buf);
		free(buf);
	}
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                          Miscellaneous Utilities                           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/**
 * Gets the boolean value of a given atom.
 *
 * @param atom Atom to get its boolean value.
 *
 * @return TRUE if the atom is of type boolean and is true, or if its any other
 *         type of atom. FALSE only for a false boolean atom.
 */
bool atom_boolean_val(atom_t atom) {
	// All non-boolean atoms are true.
	if (atom.type != ATOM_TYPE_BOOLEAN)
		return true;

	return atom.value.boolean;
}

/**
 * Prints a string to stdout. Just like puts but without the newline.
 *
 * @param str String to be printed.
 */
void putstr(const char *str) {
	const char *tmp = str;
	while (*tmp)
		putchar(*tmp++);
}

/**
 * Prints a string to stderr. Just like puts stderr, but without the newline.
 *
 * @param str String to be printed.
 */
void putstrerr(const char *str) {
	const char *tmp = str;
	while (*tmp)
		putc(*tmp++, stderr);
}

/**
 * Copies a string from start to a a specific end pointer appending the NULL
 * terminator.
 *
 * @param start Pointer to the beginning of the string.
 * @param end   Pointer to the end of the string.
 *
 * @return Newly allocated and copied string. Remember to free it!
 */
char* strcpyse(const char *start, const char *end) {
	char *buf;
	char *tmp_buf;
	const char *tmp_start;
	size_t len = end - start;

	// Allocate space for the string.
	buf = (char *)malloc((len + 1) * sizeof(char));
	if (buf == NULL) {
		bamboo_error_fatal(BAMBOO_ERROR_ALLOCATION, "Can't allocate string for string"
		                                            "start and end copying");
		return NULL;
	}

	// Copy just part of the string.
	tmp_buf = buf;
	tmp_start = start;
	while (tmp_start != end) {
		*tmp_buf++ = *tmp_start++;
	}
	*tmp_buf = '\0';

	return buf;
}

/**
 * Checks if a string contains a point character.
 *
 * @param str String to be tested.
 *
 * @return TRUE if there was a point character somewhere in the string.
 */
bool contains_point(const char *str) {
	const char *tmp = str;

	while (*tmp) {
		if (*tmp++ == '.')
			return true;
	}

	return false;
}
