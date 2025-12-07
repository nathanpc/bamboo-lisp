/**
 * error.h
 * Internal language error handling and reporting.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef BAMBOO_LISP_ERROR_H
#define BAMBOO_LISP_ERROR_H

#include "bamboo_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

// Error checking macros.
#define IF_BAMBOO_ERROR(err)        if ((err) > BAMBOO_OK)
#define IF_BAMBOO_SPECIAL_COND(err) if ((err) < BAMBOO_OK)

// Error handling.
const char *bamboo_error_detail(void);
bamboo_error_t bamboo_error(bamboo_error_t err, const char *msg);

// Error declarations.
void bamboo_error_set(const char *msg);
void bamboo_error_fatal(bamboo_error_t err, const char *msg);

// Debugging
void bamboo_error_type_str(char **buf, bamboo_error_t err);
void bamboo_error_print(bamboo_error_t err);

#ifdef __cplusplus
}
#endif

#endif  // BAMBOO_LISP_ERROR_H
