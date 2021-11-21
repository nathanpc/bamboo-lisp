/**
 * bamboo.c
 * The amazingly embeddable Lisp.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "bamboo.h"
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Convinience macros.
#define IF_ERROR(err)        if ((err) > BAMBOO_OK)
#define IF_SPECIAL_COND(err) if ((err) < BAMBOO_OK)
#define IF_NOT_ERROR(err)    if ((err) <= BAMBOO_OK)
#ifdef _MSC_VER
// Make Microsoft's compiler happy about our use of strdup and strncpy.
#define strdup  _strdup
#endif

// Private definitions.
#define ERROR_MSG_STR_LEN 200

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
static char bamboo_error_msg[ERROR_MSG_STR_LEN + 1];
static atom_t bamboo_symbol_table = { ATOM_TYPE_NIL };
static allocation_t *bamboo_allocations = NULL;
static uint32_t bamboo_gc_iter_counter = 0;

// Private methods.
void putstr(const char *str);
void putstrerr(const char *str);
bool atom_boolean_val(atom_t atom);
void set_error_msg(const char *msg);
void fatal_error(bamboo_error_t err, const char *msg);
void gc_mark(atom_t root);
void gc(void);
uint16_t list_count(atom_t list);
atom_t list_ref(atom_t list, uint16_t index);
void list_set(atom_t list, uint16_t index, atom_t value);
void list_reverse(atom_t *list);
atom_t shallow_copy_list(atom_t list);
bamboo_error_t lex(const char *str, token_t *token);
bamboo_error_t parse_hash_expr(const token_t *token, atom_t *atom);
bamboo_error_t parse_primitive(const token_t *token, atom_t *atom);
bamboo_error_t parse_string(const token_t *token, const char **end,
	atom_t *atom);
bamboo_error_t parse_list(const char *input, const char **end, atom_t *atom);
frame_t new_stack_frame(frame_t parent, env_t env, atom_t tail);
bamboo_error_t eval_expr_exec(frame_t *stack, atom_t *expr, env_t *env);
bamboo_error_t eval_expr_bind(frame_t *stack, atom_t *expr, env_t *env);
bamboo_error_t eval_expr_apply(frame_t *stack, atom_t *expr, env_t *env);
bamboo_error_t eval_expr_return(frame_t *stack, atom_t *expr, env_t *env,
	atom_t *result);

// Built-in functions.
bamboo_error_t builtin_car(atom_t args, atom_t *result);
bamboo_error_t builtin_cdr(atom_t args, atom_t *result);
bamboo_error_t builtin_cons(atom_t args, atom_t *result);
bamboo_error_t builtin_sum(atom_t args, atom_t *result);
bamboo_error_t builtin_subtract(atom_t args, atom_t *result);
bamboo_error_t builtin_multiply(atom_t args, atom_t *result);
bamboo_error_t builtin_divide(atom_t args, atom_t *result);
bamboo_error_t builtin_not(atom_t args, atom_t *result);
bamboo_error_t builtin_and(atom_t args, atom_t *result);
bamboo_error_t builtin_or(atom_t args, atom_t *result);
bamboo_error_t builtin_eq(atom_t args, atom_t *result);
bamboo_error_t builtin_numeq(atom_t args, atom_t *result);
bamboo_error_t builtin_lt(atom_t args, atom_t *result);
bamboo_error_t builtin_gt(atom_t args, atom_t *result);
bamboo_error_t builtin_nilp(atom_t args, atom_t *result);
bamboo_error_t builtin_pairp(atom_t args, atom_t *result);
bamboo_error_t builtin_symbolp(atom_t args, atom_t *result);
bamboo_error_t builtin_integerp(atom_t args, atom_t *result);
bamboo_error_t builtin_floatp(atom_t args, atom_t *result);
bamboo_error_t builtin_numericp(atom_t args, atom_t *result);
bamboo_error_t builtin_booleanp(atom_t args, atom_t *result);
bamboo_error_t builtin_builtinp(atom_t args, atom_t *result);
bamboo_error_t builtin_closurep(atom_t args, atom_t *result);
bamboo_error_t builtin_macrop(atom_t args, atom_t *result);
bamboo_error_t builtin_display(atom_t args, atom_t *result);
bamboo_error_t builtin_newline(atom_t args, atom_t *result);

// Initialization functions.
bamboo_error_t populate_builtins(env_t *env);


/**
 * Initializes the Bamboo interpreter environment.
 *
 * @param  env Pointer to the root environment of the interpreter.
 * @return     BAMBOO_OK if everything went fine.
 */
bamboo_error_t bamboo_init(env_t *env) {
	bamboo_error_t err;
	
	// Display a pretty welcome message.
	printf("Bamboo Lisp v0.1a" LINEBREAK LINEBREAK);

	// Make sure the error message string is properly terminated.
	bamboo_error_msg[0] = '\0';
	bamboo_error_msg[ERROR_MSG_STR_LEN] = '\0';

	// Make sure the garbage collection iteration counter is zeroed out.
	bamboo_gc_iter_counter = 0;

	// Initialize the root environment.
	*env = bamboo_env_new(nil);

	// Populate the environment with our built-in functions.
	err = populate_builtins(env);
	IF_ERROR(err)
		return err;

	return BAMBOO_OK;
}

/**
 * Populates the environment with our built-in functions.
 *
 * @param  env Pointer to the environment to be populated.
 * @return     BAMBOO_OK if the population was successful.
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
	err = bamboo_env_set_builtin(*env, "NEWLINE", builtin_newline);
	IF_ERROR(err)
		return err;

	return BAMBOO_OK;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                           Primitive Constructors                           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/**
 * Builds an integer atom.
 *
 * @param  num Integer number.
 * @return     Integer atom.
 */
atom_t bamboo_int(long num) {
    atom_t atom;

    // Populate the atom.
    atom.type = ATOM_TYPE_INTEGER;
    atom.value.integer = num;

    return atom;
}

/**
 * Builds an floating-point atom.
 *
 * @param  num Double floating-point number.
 * @return     Floating-point atom.
 */
atom_t bamboo_float(double num) {
    atom_t atom;

    // Populate the atom.
    atom.type = ATOM_TYPE_FLOAT;
    atom.value.dfloat = num;

    return atom;
}

/**
 * Builds an symbol atom.
 *
 * @param  name Symbol name.
 * @return      Symbol atom.
 */
atom_t bamboo_symbol(const char *name) {
    atom_t atom;
    atom_t tmp;

    // Check if the symbol already exists in the symbol table.
    tmp = bamboo_symbol_table;
    while (!nilp(tmp)) {
        atom = car(tmp);
        if (strcmp(atom.value.symbol, name) == 0)
            return atom;

        tmp = cdr(tmp);
    }

    // Create the new symbol atom.
    atom.type = ATOM_TYPE_SYMBOL;
    atom.value.symbol = strdup(name);

    // Prepend the symbol atom to the symbol table and return the atom.
    bamboo_symbol_table = cons(atom, bamboo_symbol_table);
    return atom;
}

/**
 * Build an boolean atom.
 *
 * @param  value Boolean value for the atom.
 * @return       Boolean atom.
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
 * @param  str String to be stored.
 * @return     String atom.
 */
atom_t bamboo_string(const char *str) {
	allocation_t *alloc;
    atom_t atom;

	// Create a new allocation.
	alloc = (allocation_t *)malloc(sizeof(allocation_t));
	if (alloc == NULL) {
		fatal_error(BAMBOO_ERROR_ALLOCATION, "Can't allocate structure for "
			"garbage collector allocation tracking");
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
 * @param  func Built-in C function.
 * @return      Built-in function atom.
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
 * @param  env    Environment for this closure.
 * @param  args   Arguments for the closure.
 * @param  body   Body of the closure.
 * @param  result Pointer to store the resulting atom.
 * @return        BAMBOO_OK if the atom creation was successful.
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

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                           List Atom Manipulation                           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/**
 * Builds a pair atom from two other atoms.
 *
 * @param  _car Left-hand side of the atom pair.
 * @param  _cdr Right-hand side of the atom pair.
 * @return      Atom pair.
 */
atom_t cons(atom_t _car, atom_t _cdr) {
	allocation_t *alloc;
    atom_t pair;

	// Create a new allocation.
	alloc = (allocation_t *)malloc(sizeof(allocation_t));
	if (alloc == NULL) {
		fatal_error(BAMBOO_ERROR_ALLOCATION, "Can't allocate structure for "
			"garbage collector allocation tracking");
		return nil;
	}

	// Fill up the new allocation and push the linked list forward.
	alloc->mark = GC_TO_FREE;
	alloc->type = ALLOCATION_TYPE_PAIR;
	alloc->next = bamboo_allocations;
	bamboo_allocations = alloc;

    // Setup the pair atom.
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
 * @param  expr Expression to be checked.
 * @return      TRUE if the expression is a valid list.
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
 * @param  func   Built-in function atom.
 * @param  args   Arguments to be passed to the function.
 * @param  result Pointer to the result of the operation.
 * @return        BAMBOO_OK if the function call was successful.
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
 * @param  list List to be copied.
 * @return      Shallow copy of the list.
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
 * @param  list List atom to have its elements counted.
 * @return      Number of elements in the list. 0 if it isn't a valid list.
 */
uint16_t list_count(atom_t list) {
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
 * @param  list  List you want the element from.
 * @param  index Index of the element you want.
 * @return       Element at the specified index of the list.
 */
atom_t list_ref(atom_t list, uint16_t index) {
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
void list_set(atom_t list, uint16_t index, atom_t value) {
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
void list_reverse(atom_t *list) {
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
 * @param  str   String to be scanned for tokens.
 * @param  token Pointer to the token structure that will hold the beginning and
 *               the end of a token string.
 * @return       BAMBOO_OK if a token was found. BAMBOO_ERROR_SYNTAX if we've
 *               reached the end of the string without finding any tokens.
 */
bamboo_error_t lex(const char *str, token_t *token) {
    const char *tmp = str;
    const char *wspace = " \t\r\n";
    const char *delim = "()\" \t\r\n";
    const char *prefix = "()\'\"";

    // Skip any leading whitespace.
    tmp += strspn(tmp, wspace);

    // Check if this was an empty line.
    if (tmp[0] == '\0') {
        token->start = NULL;
        token->end = NULL;

		return bamboo_error(BAMBOO_ERROR_SYNTAX, "Empty line");
    }

    // Set the starting point of our token.
    token->start = tmp;

    // Check if the token is just a parenthesis.
    if (strchr(prefix, tmp[0]) != NULL) {
        token->end = tmp + 1;
        return BAMBOO_OK;
    }

    // Find the end of the token.
    token->end = tmp + strcspn(tmp, delim);
    return BAMBOO_OK;
}

/**
 * Parses an generic expression.
 *
 * @param  input Expression as a string.
 * @param  end   Pointer that will hold the point where the parsing stopped.
 * @param  atom  Pointer to the atom object generated from the expression.
 * @return       BAMBOO_OK if the parsing was successful.
 */
bamboo_error_t bamboo_parse_expr(const char *input, const char **end,
						  atom_t *atom) {
	token_t token;
	bamboo_error_t err;

	err = lex(input, &token);
	IF_ERROR(err)
		return err;

	switch (token.start[0]) {
	case '\"':
		return parse_string(&token, end, atom);
	case '(':
		return parse_list(token.end, end, atom);
	case ')':
		return BAMBOO_PAREN_END;
	case '\'':
		// Check if we are trying to quote a list.
		if (token.end[0] == '(') {
			// Sadly I'm having issues implementing this, so just error out.
			return bamboo_error(BAMBOO_ERROR_SYNTAX,
				"Can't use the quote shorthand for quoting lists. Please use "
				"the (quote) syntax for quoring lists");
		}

		// Parse quoted body.
		*atom = cons(bamboo_symbol("QUOTE"), cons(nil, nil));
		err = bamboo_parse_expr(token.end, end, &car(cdr(*atom)));
		IF_ERROR(err)
			return err;

		// Check if we've just returned from parsing a quoted list.
		if (err == BAMBOO_PAREN_END)
			return BAMBOO_PAREN_QUOTE_END;

		return BAMBOO_QUOTE_END;
	default:
		return parse_primitive(&token, atom);
	}

	return BAMBOO_ERROR_UNKNOWN;
}

/**
 * Parses primitives from a given token.
 *
 * @param  token Pointer to token structure that holds the beginning and the end
 *               of a token string.
 * @param  atom  Pointer to an atom structure that will hold the parsed atom.
 * @return       BAMBOO_OK if we were able to parse the token correctly.
 */
bamboo_error_t parse_primitive(const token_t *token, atom_t *atom) {
	char *buf;
	char *buftmp;
    const char *tmp;
	const char *start = token->start;
	const char *end = token->end;

	// Check if we are dealing with a hash expression.
	if (start[0] == '#')
		return parse_hash_expr(token, atom);

	// Check if we are dealing with a number of some kind.
	if ((start[0] >= '0') && (start[0] <= '9')) {
		long integer;
		double dfloat;

		// Try to parse an integer.
		integer = strtol(start, &buf, 0);
		if (buf == end) {
			atom->type = ATOM_TYPE_INTEGER;
			atom->value.integer = integer;

			return BAMBOO_OK;
		}

		// Try to parse an float.
		dfloat = strtod(start, &buf);
		if (buf == end) {
			atom->type = ATOM_TYPE_FLOAT;
			atom->value.dfloat = dfloat;

			return BAMBOO_OK;
		}
	}

	// Allocate string for symbol upper-case conversion.
	buf = (char *)malloc(sizeof(char) * (end - start + 1));
	if (buf == NULL) {
		return bamboo_error(BAMBOO_ERROR_ALLOCATION, "Can't allocate string "
			"for symbol upper-case conversion");
	}

	// Convert the symbol to upper-case.
	buftmp = buf;
	tmp = start;
	while (tmp != end)
		*buftmp++ = toupper(*tmp++);
	*buftmp = '\0';

	// Check if we are dealing with a NIL symbol.
	if (strcmp(buf, "NIL") == 0) {
		*atom = nil;
	} else {
		// Looks like a regular symbol.
		*atom = bamboo_symbol(buf);
	}

	// Clean up and return OK.
	free(buf);
	return BAMBOO_OK;
}

/**
 * Parses primitives that begin with the special hash (#) notation.
 *
 * @param  token Pointer to token structure that holds the beginning and the end
 *               of a token string.
 * @param  atom  Pointer to an atom structure that will hold the parsed atom.
 * @return       BAMBOO_OK if we were able to parse the token correctly.
 */
bamboo_error_t parse_hash_expr(const token_t *token, atom_t *atom) {
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
		return BAMBOO_OK;
	case 'T':
	case 't':
		*atom = bamboo_boolean(true);
		return BAMBOO_OK;
	default:
		return bamboo_error(BAMBOO_ERROR_SYNTAX,
			"Invalid type of hash expression");
	}
}

/**
 * Parses a string from a given token.
 *
 * @param  token Pointer to token structure that holds the beginning and the end
 *               of a token string.
 * @param  end   Pointer to the end of the last parsed part of the expression.
 * @param  atom  Pointer to an atom structure that will hold the parsed atom.
 * @return       BAMBOO_OK if we were able to parse the token correctly.
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
 * @param  input Pointer to the list expression string starting at the first
 *               character after the opening parenthesis.
 * @param  end   Pointer to the end of the last parsed part of the expression.
 * @param  atom  Pointer to the atom object that will hold the result of the
 *               parsing operation.
 * @return       BAMBOO_OK if the parsing was sucessful.
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
				// We've reached the end of a list. Move the end of the token
				// in the last stack and return OK.
				*end = token.end;
				return BAMBOO_OK;
			case BAMBOO_QUOTE_END:
				// We've just ended dealing with a quote shorthand. Let's ignore
				// the next token since it has already been dealt with.
				err = lex(token.end, &token);
				IF_ERROR(err)
					return err;
				
				// Let's just continue to the next line. It's fine.
			case BAMBOO_PAREN_QUOTE_END:
				// Move the end of the token in the last stack.
				*end = token.end;
				err = BAMBOO_OK;
				break;
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
 * @param  expr   Expression to be evaluated.
 * @param  env    Environment list to use for this evaluation.
 * @param  result Pointer to the resulting atom of the evaluation.
 * @return        BAMBOO_OK if the evaluation was successful.
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
			gc();
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
			atom_t operator;
			atom_t args;
			
			// Get the operator and its arguments.
			operator = car(expr);
			args = cdr(expr);

			// Check if it's a special form to be evaluated.
			if (operator.type == ATOM_TYPE_SYMBOL) {
				// Check which special form we need to evaluate.
				if (strcmp(operator.value.symbol, "QUOTE") == 0) {
					// Check if we have the single required arguments.
					if (list_count(args) != 1) {
						return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
							"Wrong number of arguments. Expected 1");
					}

					// Return the arguments without evaluating.
					*result = car(args);
				} else if (strcmp(operator.value.symbol, "IF") == 0) {
					// Check if we have the right number of arguments.
					if (list_count(args) != 3) {
						return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
							"Wrong number of arguments. Expected 3");
					}

					// Place it in the stack for later evaluation.
					stack = new_stack_frame(stack, env, cdr(args));
					list_set(stack, STACK_EVAL_OP_INDEX, operator);
					expr = car(args);
					
					continue;
				} else if (strcmp(operator.value.symbol, "DEFINE") == 0) {
					atom_t symbol;

					// Check if we have both of the required 2 arguments.
					if (list_count(args) < 2) {
						return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
							"Wrong number of arguments. Expected at least 2");
					}

					// Get the reference symbol.
					symbol = car(args);
					switch (symbol.type) {
					case ATOM_TYPE_SYMBOL:
						// Defining a simple symbol.
						stack = new_stack_frame(stack, env, nil);
						list_set(stack, STACK_EVAL_OP_INDEX, operator);
						list_set(stack, STACK_EVAL_ARGS_INDEX, symbol);
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
				} else if (strcmp(operator.value.symbol, "LAMBDA") == 0) {
					// Check if we have both of the required 2 arguments.
					if (list_count(args) < 2) {
						return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
							"Wrong number of arguments. Expected at least 2");
					}

					// Make the closure.
					err = bamboo_closure(env, car(args), cdr(args), result);
				} else if (strcmp(operator.value.symbol, "DEFMACRO") == 0) {
					atom_t name;
					atom_t macro;
					
					// Check if we have both of the required 2 arguments.
					if (list_count(args) < 2) {
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
				} else if (strcmp(operator.value.symbol, "APPLY") == 0) {
					// Check if we have both of the required 2 arguments.
					if (list_count(args) < 2) {
						return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
							"Wrong number of arguments. Expected at least 2");
					}

					// Evaluate the apply by the magic of the stack.
					stack = new_stack_frame(stack, env, cdr(args));
					list_set(stack, STACK_EVAL_OP_INDEX, operator);
					expr = car(args);
					continue;
				} else {
					goto push;
				}
			} else if (operator.type == ATOM_TYPE_BUILTIN) {
				// Execute a built-in function.
				err = (*operator.value.builtin)(args, result);
			} else {
				// Handle a closure or macro.
push:
				stack = new_stack_frame(stack, env, args);
				expr = operator;
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
 * @param  parent Parent stack frame.
 * @param  env    Environment for the stack frame to be evaluated in.
 * @param  tail   Rest of the stack frame to be evaluated later.
 * @return        A brand new stack frame atom.
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
 * @param  stack Pointer to the stack frame we are currently evaluating.
 * @param  expr  Pointer to the expression that will be grabbed from the stack
 *               to be evaluated later.
 * @param  env   Pointer to the environment where the current expression will be
 *               evaluated in. This will also come from our stack frame.
 * @return       BAMBOO_OK if everything went fine.
 *
 * @see https://lwh.jp/lisp/continuations.html
 */
bamboo_error_t eval_expr_exec(frame_t *stack, atom_t *expr, env_t *env) {
	atom_t body;

	// Get the different parts of the stack.
	*env = list_ref(*stack, STACK_ENV_INDEX);
	body = list_ref(*stack, STACK_BODY_INDEX);
	*expr = car(body);

	// Check the next item in line.
	body = cdr(body);
	if (nilp(body)) {
		// We've reached the of this stack. Pop the stack to its parent.
		*stack = car(*stack);
	} else {
		// Advance the body of the atom to the next item in line.
		list_set(*stack, STACK_BODY_INDEX, body);
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
 * @param  stack Pointer to the stack frame we are currently evaluating.
 * @param  expr  Pointer to the expression that will be grabbed from the stack
 *               to be evaluated later.
 * @param  env   Pointer to the environment where the current expression will be
 *               evaluated in. This will also come from our stack frame.
 * @return       BAMBOO_OK if everything went fine.
 *
 * @see https://lwh.jp/lisp/continuations.html
 */
bamboo_error_t eval_expr_bind(frame_t *stack, atom_t *expr, env_t *env) {
	atom_t operator;
	atom_t args;
	atom_t arg_names;
	atom_t body;

	// If we have anything in the body just return it then.
	body = list_ref(*stack, STACK_BODY_INDEX);
	if (!nilp(body))
		return eval_expr_exec(stack, expr, env);

	// Get the operator and function arguments from the stack frame.
	operator = list_ref(*stack, STACK_EVAL_OP_INDEX);
	args = list_ref(*stack, STACK_EVAL_ARGS_INDEX);

	// Get all of the parameters from the stack frame.
	*env = bamboo_env_new(car(operator));
	arg_names = car(cdr(operator));
	body = cdr(cdr(operator));
	list_set(*stack, STACK_ENV_INDEX, *env);
	list_set(*stack, STACK_BODY_INDEX, body);

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

	list_set(*stack, STACK_EVAL_ARGS_INDEX, nil);
	return eval_expr_exec(stack, expr, env);
}

/**
 * Once all arguments have been evaluated, this function is responsible for
 * generating an expression to call a builtin, or delegating to 'eval_do_bind'.
 *
 * To be honest I have no clue how this whole thing is working, I just want to
 * make sure we don't run into stack overflows.
 *
 * @param  stack Pointer to the stack frame we are currently evaluating.
 * @param  expr  Pointer to the expression that will be grabbed from the stack
 *               to be evaluated later.
 * @param  env   Pointer to the environment where the current expression will be
 *               evaluated in. This will also come from our stack frame.
 * @return       BAMBOO_OK if everything went fine.
 *
 * @see https://lwh.jp/lisp/continuations.html
 */
bamboo_error_t eval_expr_apply(frame_t *stack, atom_t *expr, env_t *env) {
	atom_t operator;
	atom_t args;

	// Get the operator and arguments from the stack frame.
	operator = list_ref(*stack, STACK_EVAL_OP_INDEX);
	args = list_ref(*stack, STACK_EVAL_ARGS_INDEX);

	// Reverse the arguments if we have any.
	if (!nilp(args)) {
		list_reverse(&args);
		list_set(*stack, STACK_EVAL_ARGS_INDEX, args);
	}

	// Handle the apply special form.
	if (operator.type == ATOM_TYPE_SYMBOL) {
		if (strcmp(operator.value.symbol, "APPLY") == 0) {
			// Replace the current frame.
			*stack = car(*stack);
			*stack = new_stack_frame(*stack, *env, nil);
			operator = car(args);
			args = car(cdr(args));

			// Check if we actually have an arguments list.
			if (!listp(args)) {
				return bamboo_error(BAMBOO_ERROR_SYNTAX,
					"Arguments atom must be of list type");
			}

			// Go to the next one.
			list_set(*stack, STACK_EVAL_OP_INDEX, operator);
			list_set(*stack, STACK_EVAL_ARGS_INDEX, args);
		}
	}

	// Handle built-ins.
	if (operator.type == ATOM_TYPE_BUILTIN) {
		*stack = car(*stack);
		*expr = cons(operator, args);
	
		return BAMBOO_OK;
	} else if (operator.type != ATOM_TYPE_CLOSURE) {
		// Looks like we don't have anything that's "apply"able.
		return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
			"Applyable operator must be either a built-in or a closure");
	}

	return eval_expr_bind(stack, expr, env);
}

/**
 * Once an expression has been evaluated, this function is responsible for
 * storing the result, which is either an operator, an argument, or an
 * intermediate body expression, and fetching the next expression to evaluate.
 *
 * To be honest I have no clue how this whole thing is working, I just want to
 * make sure we don't run into stack overflows.
 *
 * @param  stack  Pointer to the stack frame we are currently evaluating.
 * @param  expr   Pointer to the expression that will be grabbed from the stack
 *                to be evaluated later.
 * @param  env    Pointer to the environment where the current expression will be
 *                evaluated in. This will also come from our stack frame.
 * @param  result Pointer to the return value of the evaluated stack frame.
 * @return        BAMBOO_OK if everything went fine.
 *
 * @see https://lwh.jp/lisp/continuations.html
 */
bamboo_error_t eval_expr_return(frame_t *stack, atom_t *expr, env_t *env,
		atom_t *result) {
	atom_t operator;
	atom_t args;
	atom_t body;

	// Gets the parameters from the stack frame.
	*env = list_ref(*stack, STACK_ENV_INDEX);
	operator = list_ref(*stack, STACK_EVAL_OP_INDEX);
	body = list_ref(*stack, STACK_BODY_INDEX);

	// Check if we are still running a procedure. If so, just ignore the result.
	if (!nilp(body))
		return eval_expr_apply(stack, expr, env);

	// Check in which phase of evaluation we are currently at.
	if (nilp(operator)) {
		// Finished evaluating operator.
		operator = *result;
		list_set(*stack, STACK_EVAL_OP_INDEX, operator);

		// Are we doing a macro?
		if (operator.type == ATOM_TYPE_MACRO) {
			// Don't evaluate macro arguments.
			args = list_ref(*stack, STACK_PENDING_ARGS_INDEX);
			
			*stack = new_stack_frame(*stack, *env, nil);
			operator.type = ATOM_TYPE_CLOSURE;
			list_set(*stack, STACK_EVAL_OP_INDEX, operator);
			list_set(*stack, STACK_EVAL_ARGS_INDEX, args);
			
			return eval_expr_bind(stack, expr, env);
		}
	} else if (operator.type == ATOM_TYPE_SYMBOL) {
		// Finished working on an special form.
		if (strcmp(operator.value.symbol, "DEFINE") == 0) {
			atom_t symbol;

			symbol = list_ref(*stack, STACK_EVAL_ARGS_INDEX);
			(void)bamboo_env_set(*env, symbol, *result);
			*stack = car(*stack);
			*expr = cons(bamboo_symbol("QUOTE"), cons(symbol, nil));
			
			return BAMBOO_OK;
		} else if (strcmp(operator.value.symbol, "IF") == 0) {
			args = list_ref(*stack, STACK_PENDING_ARGS_INDEX);

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
	} else if (operator.type == ATOM_TYPE_MACRO) {
		// Finished evaluating macro.
		*expr = *result;
		*stack = car(*stack);
		
		return BAMBOO_OK;
	} else {
store_argument:
		// Store the evaluated argument.
		args = list_ref(*stack, STACK_EVAL_ARGS_INDEX);
		list_set(*stack, STACK_EVAL_ARGS_INDEX, cons(*result, args));
	}

	// Get the next set of arguments to evaluate.
	args = list_ref(*stack, STACK_PENDING_ARGS_INDEX);
	if (nilp(args)) {
		// No more arguments left to evaluate.
		return eval_expr_apply(stack, expr, env);
	}

	// Evaluate the next argument.
	*expr = car(args);
	list_set(*stack, STACK_PENDING_ARGS_INDEX, cdr(args));
	
	return BAMBOO_OK;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                                Environment                                 //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/**
 * Creates a new child environment list.
 *
 * @param  parent Parent environment to this new child.
 * @return        New child environment list.
 */
env_t bamboo_env_new(env_t parent) {
	return cons(parent, nil);
}

/**
 * Gets a symbol definition from an environment list recursively searching
 * through its parents.
 *
 * @param  env    Environment list to search for the desired symbol in.
 * @param  symbol Symbol you're searching for.
 * @param  atom   Pointer to the resulting atom of the symbol definition.
 * @return        BAMBOO_OK if the symbol was found. BAMBOO_ERROR_UNBOUND
 *                otherwise.
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
		if (car(item).value.symbol == symbol.value.symbol) {
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
			"environments", symbol.value.symbol);
		return bamboo_error(BAMBOO_ERROR_UNBOUND, msg);
	}

	// Search for the symbol in the parent.
	return bamboo_env_get(parent, symbol, atom);
}

/**
 * Creates a new symbol inside an environment or changes it if it already
 * exists in the specified environment.
 *
 * @param  env    Parent environment where the symbol resides.
 * @param  symbol Symbol to be created or edited.
 * @param  value  Value attributed to the symbol.
 * @return        BAMBOO_OK if the operation was successful.
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
		if (car(item).value.symbol == symbol.value.symbol) {
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
 * @param  env  Parent environment where the built-in function symbol resides.
 * @param  name Name of the symbol for the built-in function.
 * @param  func Built-in function that will be called for this symbol.
 * @return      BAMBOO_OK if the operation was successful.
 */
bamboo_error_t bamboo_env_set_builtin(env_t env, const char *name,
									  builtin_func_t func) {
	return bamboo_env_set(env, bamboo_symbol(name), bamboo_builtin(func));
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
 */
void gc(void) {
	allocation_t *alloc;
	allocation_t **tmp;

	// Make sure we don't trash our global symbols list.
	gc_mark(bamboo_symbol_table);

	// Free up all unmarked allocations.
	tmp = &bamboo_allocations;
	while (*tmp != NULL) {
		alloc = *tmp;

		// Check if it's marked to be freed.
		if (alloc->mark == GC_TO_FREE) {
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
 * Prints the contents of an atom in a standard way.
 *
 * @param atom Atom to have its contents printed.
 */
void bamboo_print_expr(atom_t atom) {
    switch (atom.type) {
    case ATOM_TYPE_NIL:
        putstr("nil");
        break;
    case ATOM_TYPE_SYMBOL:
        printf("%s", atom.value.symbol);
        break;
    case ATOM_TYPE_INTEGER:
        printf("%ld", atom.value.integer);
        break;
    case ATOM_TYPE_FLOAT:
        printf("%g", atom.value.dfloat);
        break;
	case ATOM_TYPE_BOOLEAN:
		printf("#%c", (atom.value.boolean) ? 't' : 'f');
		break;
	case ATOM_TYPE_STRING:
		printf("\"%s\"", *atom.value.str);
		break;
    case ATOM_TYPE_PAIR:
        putchar('(');
        bamboo_print_expr(car(atom));
        atom = cdr(atom);

        // Iterate over the right-hand side of the pair since it may be a list.
        while (!nilp(atom)) {
            // Check if we are in a list.
            if (atom.type == ATOM_TYPE_PAIR) {
                putchar(' ');
                bamboo_print_expr(car(atom));
                atom = cdr(atom);
            } else {
                // It was just a simple pair.
                putstr(" . ");
                bamboo_print_expr(atom);
                break;
            }
        }

        putchar(')');
        break;
	case ATOM_TYPE_BUILTIN:
		printf("#<BUILTIN:%p>", atom.value.builtin);
		break;
	case ATOM_TYPE_CLOSURE:
		putstr("#<FUNCTION:");
		if (!nilp(car(cdr(atom))))
			bamboo_print_expr(car(cdr(atom)));
		putstr(" ");
		bamboo_print_expr(cdr(cdr(atom)));
		putstr(">");
		break;
	case ATOM_TYPE_MACRO:
		putstr("#<MACRO:");
		if (!nilp(car(cdr(atom))))
			bamboo_print_expr(car(cdr(atom)));
		putstr(" ");
		bamboo_print_expr(cdr(cdr(atom)));
		putstr(">");
		break;
	default:
		putstr("Unknown type. Don't know how to display this");
    }
}

/**
 * Prints the appropriate error message for a given error code.
 *
 * @param err Error code to print the message.
 */
void bamboo_print_error(bamboo_error_t err) {
	switch (err) {
	case BAMBOO_OK:
		putstrerr("OK");
		break;
	case BAMBOO_PAREN_END:
		putstrerr("PARENTHESIS ENDED");
		break;
	case BAMBOO_ERROR_SYNTAX:
		putstrerr("SYNTAX ERROR: ");
		putstrerr(bamboo_error_detail());
		break;
	case BAMBOO_ERROR_UNBOUND:
		putstrerr("UNBOUND SYMBOL ERROR: ");
		putstrerr(bamboo_error_detail());
		break;
	case BAMBOO_ERROR_ARGUMENTS:
		putstrerr("INCORRECT ARGUMENT ERROR: ");
		putstrerr(bamboo_error_detail());
		break;
	case BAMBOO_ERROR_WRONG_TYPE:
		putstrerr("WRONG TYPE ERROR: ");
		putstrerr(bamboo_error_detail());
		break;
	case BAMBOO_ERROR_UNKNOWN:
		putstrerr("UNKNOWN ERROR: ");
		putstrerr(bamboo_error_detail());
		break;
	default:
		putstrerr("I have no clue why you're here, because you shouldn't: ");
		putstrerr(bamboo_error_detail());
		break;
	}

	putstrerr(LINEBREAK);
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
			fatal_error(BAMBOO_ERROR_ALLOCATION,
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
//                               Error Handling                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/**
 * Gets the last detailed error message from the interpreter.
 *
 * @return Last detailed error message.
 */
const char* bamboo_error_detail(void) {
	return bamboo_error_msg;
}

/**
 * Sets the internal error message variable.
 *
 * @param msg Error message to be set.
 */
void set_error_msg(const char *msg) {
	strncpy(bamboo_error_msg, msg, ERROR_MSG_STR_LEN);
}

/**
 * Sets the internal error message variable and returns the specified error
 * code.
 * 
 * @param  err Error code to be returned.
 * @param  msg Error message to be set.
 * @return     Error code passed in 'err'.
 */
bamboo_error_t bamboo_error(bamboo_error_t err, const char *msg) {
	set_error_msg(msg);
	return err;
}

/**
 * Handles a fatal error where the program is required to exit immediately,
 * since it's unrecoverable.
 * 
 * @param err Error code to be returned.
 * @param msg Error message to be set.
 */
void fatal_error(bamboo_error_t err, const char *msg) {
	// Print the error message.
	set_error_msg(msg);
	bamboo_print_error(err);

	// Bye guys!
	exit(err);
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                             Built-in Functions                             //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// (car pair) -> atom
bamboo_error_t builtin_car(atom_t args, atom_t *result) {
	// Check if we have the right number of arguments.
	if (list_count(args) != 1) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects a single argument");
	}

	// Do what's appropriate for each scenario.
	if (nilp(car(args))) {
		// Is it just nil?
		*result = nil;
	} else if (car(args).type != ATOM_TYPE_PAIR) {
		// Is it actually a pair?
		return bamboo_error(BAMBOO_ERROR_WRONG_TYPE, "Argument must be a pair");
	} else {
		// Just get the first element of the pair.
		*result = car(car(args));
	}

	return BAMBOO_OK;
}

// (cdr pair) -> atom
bamboo_error_t builtin_cdr(atom_t args, atom_t *result) {
	// Check if we have the right number of arguments.
	if (list_count(args) != 1) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects a single argument");
	}

	// Do what's appropriate for each scenario.
	if (nilp(car(args))) {
		// Is it just nil?
		*result = nil;
	} else if (car(args).type != ATOM_TYPE_PAIR) {
		// Is it actually a pair?
		return bamboo_error(BAMBOO_ERROR_WRONG_TYPE, "Argument must be a pair");
	} else {
		// Just get the first element of the pair.
		*result = cdr(car(args));
	}

	return BAMBOO_OK;
}

// (cons car cdr) -> pair
bamboo_error_t builtin_cons(atom_t args, atom_t *result) {
	// Check if we have the right number of arguments.
	if (list_count(args) != 2) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects 2 arguments");
	}

	// Create the pair.
	*result = cons(car(args), car(cdr(args)));
	return BAMBOO_OK;
}

// (+ nums...) -> num
bamboo_error_t builtin_sum(atom_t args, atom_t *result) {
	atom_t num;

	// Initialize the result atom.
	num.type = ATOM_TYPE_INTEGER;
	num.value.integer = 0;

	// Check if we have the right number of arguments.
	if (list_count(args) < 2) {
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
			}
		} else if (car(args).type == ATOM_TYPE_FLOAT) {
			// Float argument. Check if we should change our atom type first.
			if (num.type == ATOM_TYPE_INTEGER) {
				num.type = ATOM_TYPE_FLOAT;
				num.value.dfloat = (double)num.value.integer;
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
	if (list_count(args) < 2) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects at least 2 arguments");
	}

	// Iterate through the arguments subtracting them.
	while (!nilp(args)) {
		if (car(args).type == ATOM_TYPE_INTEGER) {
			// Integer argument.
			if (first) {
				// First iteration, so let's assign the first value first.
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
			}
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
				num.value.dfloat = (double)num.value.integer;
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
	if (list_count(args) < 2) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects at least 2 arguments");
	}

	// Iterate through the arguments multiplying them.
	while (!nilp(args)) {
		if (car(args).type == ATOM_TYPE_INTEGER) {
			// Integer argument.
			if (first) {
				// First iteration, so let's assign the first value first.
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
			}
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
				num.value.dfloat = (double)num.value.integer;
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
	num.value.dfloat = (double)0;

	// Check if we have the right number of arguments.
	if (list_count(args) < 2) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects at least 2 arguments");
	}

	// Iterate through the arguments dividing them.
	while (!nilp(args)) {
		if (car(args).type == ATOM_TYPE_INTEGER) {
			// Integer argument.
			if (first) {
				// First iteration, so let's assign the first value first.
				num.value.dfloat = (double)car(args).value.integer;
				first = false;

				goto next;
			}

			num.value.dfloat /= (double)car(args).value.integer;
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
				num.value.dfloat = (double)num.value.integer;
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

// (not bool) -> bool
bamboo_error_t builtin_not(atom_t args, atom_t *result) {
	// Check if we have the right number of arguments.
	if (list_count(args) != 1) {
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
	if (list_count(args) < 2) {
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
	if (list_count(args) < 2) {
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
	if (list_count(args) < 2) {
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
				if (prev_num.value.dfloat == (double)car(args).value.integer)
					goto next;
			}
			
			*result = bamboo_boolean(false);
			return BAMBOO_OK;
		} else if (car(args).type == ATOM_TYPE_FLOAT) {
			if (prev_num.type == ATOM_TYPE_INTEGER) {
				// float == int.
				if ((double)prev_num.value.integer == car(args).value.dfloat)
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
	if (list_count(args) < 2) {
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
				if (prev_num.value.dfloat < (double)car(args).value.integer)
					goto next;
			}
			
			*result = bamboo_boolean(false);
			return BAMBOO_OK;
		} else if (car(args).type == ATOM_TYPE_FLOAT) {
			if (prev_num.type == ATOM_TYPE_INTEGER) {
				// float == int.
				if ((double)prev_num.value.integer < car(args).value.dfloat)
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
	if (list_count(args) < 2) {
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
				if (prev_num.value.dfloat > (double)car(args).value.integer)
					goto next;
			}
			
			*result = bamboo_boolean(false);
			return BAMBOO_OK;
		} else if (car(args).type == ATOM_TYPE_FLOAT) {
			if (prev_num.type == ATOM_TYPE_INTEGER) {
				// float == int.
				if ((double)prev_num.value.integer > car(args).value.dfloat)
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

// (eq? a b) -> boolean
bamboo_error_t builtin_eq(atom_t args, atom_t *result) {
	atom_t a;
	atom_t b;

	// Check if we have the right number of arguments.
	if (list_count(args) != 2) {
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
		*result = bamboo_boolean(a.value.symbol == b.value.symbol);
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
	}

	return BAMBOO_OK;
}

// (nil? atom) -> boolean
bamboo_error_t builtin_nilp(atom_t args, atom_t *result) {
	// Check if we have the right number of arguments.
	if (list_count(args) != 1) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects 1 argument");
	}

	*result = bamboo_boolean(car(args).type == ATOM_TYPE_NIL);
	return BAMBOO_OK;
}

// (pair? atom) -> boolean
bamboo_error_t builtin_pairp(atom_t args, atom_t *result) {
	// Check if we have the right number of arguments.
	if (list_count(args) != 1) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects 1 argument");
	}

	*result = bamboo_boolean(car(args).type == ATOM_TYPE_PAIR);
	return BAMBOO_OK;
}

// (symbol? atom) -> boolean
bamboo_error_t builtin_symbolp(atom_t args, atom_t *result) {
	// Check if we have the right number of arguments.
	if (list_count(args) != 1) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects 1 argument");
	}

	*result = bamboo_boolean(car(args).type == ATOM_TYPE_SYMBOL);
	return BAMBOO_OK;
}

// (integer? atom) -> boolean
bamboo_error_t builtin_integerp(atom_t args, atom_t *result) {
	// Check if we have the right number of arguments.
	if (list_count(args) != 1) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects 1 argument");
	}

	*result = bamboo_boolean(car(args).type == ATOM_TYPE_INTEGER);
	return BAMBOO_OK;
}

// (float? atom) -> boolean
bamboo_error_t builtin_floatp(atom_t args, atom_t *result) {
	// Check if we have the right number of arguments.
	if (list_count(args) != 1) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects 1 argument");
	}

	*result = bamboo_boolean(car(args).type == ATOM_TYPE_FLOAT);
	return BAMBOO_OK;
}

// (numeric? atom) -> boolean
bamboo_error_t builtin_numericp(atom_t args, atom_t *result) {
	// Check if we have the right number of arguments.
	if (list_count(args) != 1) {
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
	if (list_count(args) != 1) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects 1 argument");
	}

	*result = bamboo_boolean(car(args).type == ATOM_TYPE_BOOLEAN);
	return BAMBOO_OK;
}

// (builtin? atom) -> boolean
bamboo_error_t builtin_builtinp(atom_t args, atom_t *result) {
	// Check if we have the right number of arguments.
	if (list_count(args) != 1) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects 1 argument");
	}

	*result = bamboo_boolean(car(args).type == ATOM_TYPE_BUILTIN);
	return BAMBOO_OK;
}

// (closure? atom) -> boolean
bamboo_error_t builtin_closurep(atom_t args, atom_t *result) {
	// Check if we have the right number of arguments.
	if (list_count(args) != 1) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects 1 argument");
	}

	*result = bamboo_boolean(car(args).type == ATOM_TYPE_CLOSURE);
	return BAMBOO_OK;
}

// (macro? atom) -> boolean
bamboo_error_t builtin_macrop(atom_t args, atom_t *result) {
	// Check if we have the right number of arguments.
	if (list_count(args) != 1) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects 1 argument");
	}

	*result = bamboo_boolean(car(args).type == ATOM_TYPE_MACRO);
	return BAMBOO_OK;
}

// (display any...) -> nil
bamboo_error_t builtin_display(atom_t args, atom_t *result) {
	size_t len;
	char *buf;

	// Check if we have the right number of arguments.
	if (list_count(args) < 1) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects at least 1 argument");
	}

	// Iterate through the arguments printing them them.
	while (!nilp(args)) {
		switch (car(args).type) {
		case ATOM_TYPE_STRING:
			putstr(*car(args).value.str);
			break;
		case ATOM_TYPE_NIL:
	        break;
	    case ATOM_TYPE_SYMBOL:
	        putstr(car(args).value.symbol);
	        break;
	    case ATOM_TYPE_INTEGER:
			// Get the length of the string we'll need to print this number.
	        len = snprintf(NULL, 0, "%ld", car(args).value.integer);
			buf = (char *)malloc((len + 1) * sizeof(char));
			if (buf == NULL) {
				*result = nil;
				return bamboo_error(BAMBOO_ERROR_ALLOCATION, "Can't allocate "
					"string to display integer atom");
			}

			// Create the numeric string, print it, and free it.
			sprintf(buf, "%ld", car(args).value.integer);
			putstr(buf);
			free(buf);
	        break;
	    case ATOM_TYPE_FLOAT:
			// Get the length of the string we'll need to print this number.
	        len = snprintf(NULL, 0, "%g", car(args).value.dfloat);
			buf = (char *)malloc((len + 1) * sizeof(char));
			if (buf == NULL) {
				*result = nil;
				return bamboo_error(BAMBOO_ERROR_ALLOCATION, "Can't allocate "
					"string to display float atom");
			}

			// Create the numeric string, print it, and free it.
			sprintf(buf, "%g", car(args).value.dfloat);
			putstr(buf);
			free(buf);
	        break;
		case ATOM_TYPE_BOOLEAN:
			if (car(args).value.boolean) {
				putstr("TRUE");
			} else {
				putstr("FALSE");
			}
			break;
		default:
			*result = nil;
			return bamboo_error(BAMBOO_ERROR_WRONG_TYPE,
				"Don't know how to display this type of atom");
		}

		// Go to the next argument.
		args = cdr(args);
	}

	putstr(LINEBREAK);
	*result = nil;
	return BAMBOO_OK;
}

// (newline) -> nil
bamboo_error_t builtin_newline(atom_t args, atom_t *result) {
	// Check if we have the right number of arguments.
	if (list_count(args) != 0) {
		return bamboo_error(BAMBOO_ERROR_ARGUMENTS,
			"This function expects no arguments");
	}

	// Print the newline string.
	putstr(LINEBREAK);

	*result = nil;
	return BAMBOO_OK;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                          Miscellaneous Utilities                           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/**
 * Gets the boolean value of a given atom.
 *
 * @param  atom Atom to get its boolean value.
 * @return      TRUE if the atom is of type boolean and is true, or if its any
 *              other type of atom. FALSE only for a false boolean atom.
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
