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
#define SYMBOL_NIL_STR "NIL"

// Token structure.
typedef struct {
	const char *start;
	const char *end;
} token_t;

// Private variables.
static atom_t symbol_table = { ATOM_TYPE_NIL };

// Private methods.
void putstr(const char *str);
bamboo_error_t lex(const char *str, token_t *token);
bamboo_error_t parse_primitive(const token_t *token, atom_t *atom);
bamboo_error_t parse_list(const char *input, const char **end, atom_t *atom);

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
    tmp = symbol_table;
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
    symbol_table = cons(atom, symbol_table);
    return atom;
}

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
			// Check if the pair separator is the first token in the atom.
			if (nilp(*atom))
				return BAMBOO_ERROR_SYNTAX;

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
			if (!nilp(*last_atom))
				return BAMBOO_ERROR_SYNTAX;

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
		putstr("SYNTAX ERROR");
		break;
	case BAMBOO_ERROR_UNKNOWN:
		putstr("UNKNOWN ERROR");
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
