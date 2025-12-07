/**
 * error.c
 * Internal language error handling and reporting.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "error.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * Maximum length of our internal error string.
 */
#define ERROR_MSG_STR_LEN 200

/**
 * Internal global error string.
 */
static char bamboo_error_msg[ERROR_MSG_STR_LEN + 1];

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
void bamboo_error_set(const char *msg) {
	strncpy(bamboo_error_msg, msg, ERROR_MSG_STR_LEN);
}

/**
 * Sets the internal error message variable and returns the specified error
 * code.
 *
 * @param err Error code to be returned.
 * @param msg Error message to be set.
 *
 * @return Error code passed in 'err'.
 */
bamboo_error_t bamboo_error(bamboo_error_t err, const char *msg) {
	bamboo_error_set(msg);
	return err;
}

/**
 * Handles a fatal error where the program is required to exit immediately,
 * since it's unrecoverable.
 *
 * @param err Error code to be returned.
 * @param msg Error message to be set.
 */
void bamboo_error_fatal(bamboo_error_t err, const char *msg) {
	// Print the error message.
	bamboo_error_set(msg);
	bamboo_error_print(err);

	// Bye guys!
	exit(err);
}

/**
 * Gets the error type string given an error code.
 *
 * @param buf Pointer to a string that will be allocated by this function which
 *            will return the error type string. NOTE: Remember that you're
 *            responsible for freeing this pointer later.
 * @param err Error code.
 */
void bamboo_error_type_str(char **buf, bamboo_error_t err) {
	// Get the error type string.
	switch (err) {
	case BAMBOO_OK:
		*buf = strdup("OK");
		break;
	case BAMBOO_PAREN_END:
		*buf = strdup("PARENTHESIS ENDED");
		break;
	case BAMBOO_ERROR_SYNTAX:
		*buf = strdup("SYNTAX ERROR");
		break;
	case BAMBOO_ERROR_EMPTY:
		*buf = strdup("EMPTY STATEMENT");
		break;
	case BAMBOO_ERROR_UNBOUND:
		*buf = strdup("UNBOUND SYMBOL ERROR");
		break;
	case BAMBOO_ERROR_ARGUMENTS:
		*buf = strdup("INCORRECT ARGUMENT ERROR");
		break;
	case BAMBOO_ERROR_WRONG_TYPE:
		*buf = strdup("WRONG TYPE ERROR");
		break;
	case BAMBOO_ERROR_NUM_OVERFLOW:
		*buf = strdup("NUMERIC OVERFLOW ERROR");
		break;
	case BAMBOO_ERROR_NUM_UNDERFLOW:
		*buf = strdup("NUMERIC UNDERFLOW ERROR");
		break;
	case BAMBOO_ERROR_ALLOCATION:
		*buf = strdup("MEMORY ALLOCATION ERROR");
		break;
	case BAMBOO_ERROR_UNKNOWN:
		*buf = strdup("UNKNOWN ERROR");
		break;
	default:
		*buf = strdup("I have no clue why you're here, because you shouldn't");
		break;
	}
}

/**
 * Prints the appropriate error message for a given error code.
 *
 * @param err Error code to print the message.
 */
void bamboo_error_print(bamboo_error_t err) {
	char *err_type = NULL;

	// Get the error type string and print it out.
	bamboo_error_type_str(&err_type, err);
	putstrerr(err_type);
	putstrerr(": ");

	// Print the error detail string and a line break.
	putstrerr(bamboo_error_detail());
	putstrerr("\n");

	// Free up our temporary buffer.
	free(err_type);
}
