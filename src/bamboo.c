/**
 * bamboo.c
 * The amazingly embeddable Lisp.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "bamboo.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Private definitions.
#define SYMBOL_NIL_STR    "NIL"
#define ERROR_MSG_STR_LEN 100

// Token structure.
typedef struct {
	const char *start;
	const char *end;
} token_t;

// Private variables.
static char bamboo_error_msg[ERROR_MSG_STR_LEN + 1];
static atom_t bamboo_symbol_table = { ATOM_TYPE_NIL };

// Private methods.
void putstr(const char *str);
void set_error_msg(const char *msg);
uint8_t list_count(atom_t list);
atom_t shallow_copy_list(atom_t list);
bamboo_error_t lex(const char *str, token_t *token);
bamboo_error_t parse_primitive(const token_t *token, atom_t *atom);
bamboo_error_t parse_list(const char *input, const char **end, atom_t *atom);

// Built-in functions.
bamboo_error_t builtin_car(atom_t args, atom_t *result);
bamboo_error_t builtin_cdr(atom_t args, atom_t *result);
bamboo_error_t builtin_cons(atom_t args, atom_t *result);

/**
 * Initializes the Bamboo interpreter environment.
 *
 * @param  env Pointer to the root environment of the interpreter.
 * @return     BAMBOO_OK if everything went fine.
 */
bamboo_error_t bamboo_init(env_t *env) {
	// Display a pretty welcome message.
	printf("Bamboo Lisp v0.1a" LINEBREAK LINEBREAK);

	// Make sure the error message string is properly terminated.
	bamboo_error_msg[0] = '\0';
	bamboo_error_msg[ERROR_MSG_STR_LEN] = '\0';

	// Initialize the root environment.
	*env = bamboo_env_new(nil);

	// Populate the environment with our built-in functions.
	bamboo_env_set_builtin(*env, "CAR", builtin_car);
	bamboo_env_set_builtin(*env, "CDR", builtin_cdr);
	bamboo_env_set_builtin(*env, "CONS", builtin_cons);

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
    atom_t pair;

    // Setup the pair atom.
    pair.type = ATOM_TYPE_PAIR;
    pair.value.pair = (pair_t *)malloc(sizeof(pair_t));

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
 * Calls a built-in function with the supplied arguments and get the result.
 *
 * @param  func   Built-in function atom.
 * @param  args   Arguments to be passed to the function.
 * @param  result Pointer to the result of the operation.
 * @return        BAMBOO_OK if the function call was successful.
 */
bamboo_error_t apply(atom_t func, atom_t args, atom_t *result) {
	// Check if we actually have a built-in function.
	if (func.type != ATOM_TYPE_BUILTIN) {
		set_error_msg("Atom should be of type builtin");
		return BAMBOO_ERROR_WRONG_TYPE;
	}

	// Call the built-in function.
	return (*func.value.builtin)(args, result);
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
uint8_t list_count(atom_t list) {
	uint8_t count = 0;
	
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

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                      Lexing, Parsing, and Evaluation                       //
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
    const char *delim = "() \t\n";
    const char *prefix = "()";

    // Skip any leading whitespace.
    tmp += strspn(tmp, wspace);

    // Check if this was an empty line.
    if (tmp[0] == '\0') {
        token->start = NULL;
        token->end = NULL;

		set_error_msg("Empty line");
        return BAMBOO_ERROR_SYNTAX;
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

	// Check if we are dealing with a number of some kind.
	if ((start[0] >= '0') && (start[0] <= '9')) {
		// Try to parse an integer.
		long num = strtol(start, &buf, 0);

		// Check if we were able to parse an integer from the token.
		if (buf == end) {
			atom->type = ATOM_TYPE_INTEGER;
			atom->value.integer = num;

			return BAMBOO_OK;
		}
	}

	// Convert the symbol to upper-case.
	buf = (char *)malloc(sizeof(char) * (end - start + 1));
	buftmp = buf;
	tmp = start;
	while (tmp != end)
		*buftmp++ = toupper(*tmp++);
	*buftmp = '\0';

	// Check if we are dealing with a NIL symbol.
	if (strcmp(buf, SYMBOL_NIL_STR) == 0) {
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
	bool is_pair = false;

	// Reset values.
	*atom = nil;
	tmp_atom = nil;
	last_atom = atom;
	token.end = input;

	while (!(err = lex(token.end, &token))) {
		// Check if we have a pair.
		if (token.start[0] == '.') {
			token_t test_token;

			// Check if the pair separator is the first token in the atom.
			if (nilp(*atom)) {
				set_error_msg("Pair delimiter without left-hand atom");
				return BAMBOO_ERROR_SYNTAX;
			}

			// Check if we have something after the pair separator.
			err = lex(token.end, &test_token);
			if (err || (test_token.start[0] == ')')) {
				set_error_msg("Pair ends without right-hand atom");
				return BAMBOO_ERROR_SYNTAX;
			}

			// Move to the next token.
			is_pair = true;
			continue;
		}

		// Parse the next token of list.
		err = parse_expr(token.start, &(token.end), &tmp_atom);
		if (err) {
			// Move the end of the token in the last stack.
			*end = token.end;

			// Have we just reached the end of a list?
			if (err == BAMBOO_PAREN_END)
				return BAMBOO_OK;

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
				set_error_msg("Tried to append an atom to a pair");
				return BAMBOO_ERROR_SYNTAX;
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
 * Parses an generic expression.
 *
 * @param  input Expression as a string.
 * @param  end   Pointer that will hold the point where the parsing stopped.
 * @param  atom  Pointer to the atom object generated from the expression.
 * @return       BAMBOO_OK if the parsing was successful.
 */
bamboo_error_t parse_expr(const char *input, const char **end,
						  atom_t *atom) {
	token_t token;
	bamboo_error_t err;

	err = lex(input, &token);
	if (err)
		return err;

	switch (token.start[0]) {
	case '(':
		return parse_list(token.end, end, atom);
	case ')':
		return BAMBOO_PAREN_END;
	default:
		return parse_primitive(&token, atom);
	}

	return BAMBOO_ERROR_UNKNOWN;
}

/**
 * Evaluates an expression in a given environment.
 *
 * @param  expr   Expression to be evaluated.
 * @param  env    Environment list to use for this evaluation.
 * @param  result Pointer to the resulting atom of the evaluation.
 * @return        BAMBOO_OK if the evaluation was successful.
 */
bamboo_error_t bamboo_eval_expr(atom_t expr, env_t env, atom_t *result) {
	atom_t operator;
	atom_t args;
	atom_t tmp;
	bamboo_error_t err;

	// Clean slate.
	*result = nil;

	// Check if the expression is simple and doesn't require any manipulation.
	if (expr.type == ATOM_TYPE_SYMBOL) {
		// A symbol from the environment was requested.
		return bamboo_env_get(env, expr, result);
	} else if (expr.type != ATOM_TYPE_PAIR) {
		// Literals always evaluate to themselves.
		*result = expr;
		return BAMBOO_OK;
	}

	// Check if we actually have a list to evaluate.
	if (!listp(expr)) {
		set_error_msg("Expression is not of list type");
		return BAMBOO_ERROR_SYNTAX;
	}

	// Get the operator and its arguments.
	operator = car(expr);
	args = cdr(expr);

	// Check if it's a special forms.
	if (operator.type == ATOM_TYPE_SYMBOL) {
		// Check which special form we need.
		if (strcmp(operator.value.symbol, "QUOTE") == 0) {
			// Check if we have the single required arguments.
			if (list_count(args) != 1) {
				set_error_msg("Wrong number of arguments. Expected 1");
				return BAMBOO_ERROR_ARGUMENTS;
			}

			// Return the arguments without evaluating.
			*result = car(args);
			return BAMBOO_OK;
		} else if (strcmp(operator.value.symbol, "DEFINE") == 0) {
			atom_t symbol;
			atom_t value;

			// Check if we have both of the required 2 arguments.
			if (list_count(args) != 2) {
				set_error_msg("Wrong number of arguments. Expected 2");
				return BAMBOO_ERROR_ARGUMENTS;
			}

			// Get symbol.
			symbol = car(args);
			if (symbol.type != ATOM_TYPE_SYMBOL) {
				set_error_msg("Argument 0 should be of type symbol");
				return BAMBOO_ERROR_WRONG_TYPE;
			}

			// Evaluate value before assigning it to the symbol.
			err = bamboo_eval_expr(car(cdr(args)), env, &value);
			if (err)
				return err;

			// Put the symbol in th environment.
			*result = symbol;
			return bamboo_env_set(env, symbol, value);
		}
	}

	// Evaluate operator.
	err = bamboo_eval_expr(operator, env, &operator);
	if (err)
		return err;

	// Copy the arguments list to allow for future use without being overritten.
	args = shallow_copy_list(args);

	// Evaluate the arguments.
	tmp = args;
	while (!nilp(tmp)) {
		err = bamboo_eval_expr(car(tmp), env, &car(tmp));
		if (err)
			return err;

		// Go to the next element in the list.
		tmp = cdr(tmp);
	}

	// Call a function with the supplied arguments.
	return apply(operator, args, result);
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
		set_error_msg("Symbol not found in any of the environments");
		return BAMBOO_ERROR_UNBOUND;
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
	default:
		putstr("Don't know how to show this");
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
		putstr("OK");
		break;
	case BAMBOO_PAREN_END:
		putstr("PARENTHESIS ENDED");
		break;
	case BAMBOO_ERROR_SYNTAX:
		putstr("SYNTAX ERROR: ");
		putstr(bamboo_error_detail());
		break;
	case BAMBOO_ERROR_UNBOUND:
		putstr("UNBOUND SYMBOL ERROR: ");
		putstr(bamboo_error_detail());
		break;
	case BAMBOO_ERROR_ARGUMENTS:
		putstr("INCORRECT ARGUMENT ERROR: ");
		putstr(bamboo_error_detail());
		break;
	case BAMBOO_ERROR_WRONG_TYPE:
		putstr("WRONG TYPE ERROR: ");
		putstr(bamboo_error_detail());
		break;
	case BAMBOO_ERROR_UNKNOWN:
		putstr("UNKNOWN ERROR: ");
		putstr(bamboo_error_detail());
		break;
	}

	putstr(LINEBREAK);
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

		// Get the token string from the token structure.
		buf = (char *)malloc((token.end - token.start + 1) * sizeof(char));
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

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                             Built-in Functions                             //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

bamboo_error_t builtin_car(atom_t args, atom_t *result) {
	// Check if we have the right number of arguments.
	if (list_count(args) != 1) {
		set_error_msg("This function expects a single argument");
		return BAMBOO_ERROR_ARGUMENTS;
	}

	// Do what's appropriate for each scenario.
	if (nilp(car(args))) {
		// Is it just nil?
		*result = nil;
	} else if (car(args).type != ATOM_TYPE_PAIR) {
		// Is it actually a pair?
		set_error_msg("Argument must be a pair");
		return BAMBOO_ERROR_WRONG_TYPE;
	} else {
		// Just get the first element of the pair.
		*result = car(car(args));
	}

	return BAMBOO_OK;
}

bamboo_error_t builtin_cdr(atom_t args, atom_t *result) {
	// Check if we have the right number of arguments.
	if (list_count(args) != 1) {
		set_error_msg("This function expects a single argument");
		return BAMBOO_ERROR_ARGUMENTS;
	}

	// Do what's appropriate for each scenario.
	if (nilp(car(args))) {
		// Is it just nil?
		*result = nil;
	} else if (car(args).type != ATOM_TYPE_PAIR) {
		// Is it actually a pair?
		set_error_msg("Argument must be a pair");
		return BAMBOO_ERROR_WRONG_TYPE;
	} else {
		// Just get the first element of the pair.
		*result = cdr(car(args));
	}

	return BAMBOO_OK;
}

bamboo_error_t builtin_cons(atom_t args, atom_t *result) {
	// Check if we have the right number of arguments.
	if (list_count(args) != 2) {
		set_error_msg("This function expects 2 arguments");
		return BAMBOO_ERROR_ARGUMENTS;
	}

	// Create the pair.
	*result = cons(car(args), car(cdr(args)));
	return BAMBOO_OK;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                          Miscellaneous Utilities                           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

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
