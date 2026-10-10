/**-----------------------------------------------------------------------*
 * Common error codes for client/server
 *
 * Copyright (C) 2026 @oxeylie - AGPL v3.0
 *-----------------------------------------------------------------------**/

#ifndef _MISC_ERROR_H
#define _MISC_ERROR_H

#include <stdio.h>
#include <sys/errno.h>

#ifdef __linux__
#define ELAST 133 // Max errno code on linux
#endif
#define ERR_COUNT (ECOUNT - ELAST)

/// Types

/**
 * Error codes
 */
typedef enum {
  OK = 0,
  EINVLPRM = ELAST + 1,
  ECOUNT,
} err;

/// Global vars

/**
 * Array of error error messages corresponding to error codes
 */
extern char* error_messages[ERR_COUNT];

/**
 * Error file
 */
extern FILE* error_file;

/**
 * Log file
 */
extern FILE* log_file;

/// Global functions

/**
 * Prints log with formated message
 *
 * @param msg_fmt message format
 * @param ... format args
 */
void printLogf(char* restrict msg_fmt, ...);

/**
 * Prints err with formated message
 *
 * @param err_fmt message format
 * @param ... format args
 */
void printErrorf(err code, char* restrict err_fmt, ...);

/**
 * Prints & panic err with formated message
 *
 * @param err_fmt message format
 * @param ... format args
 */
void panicErrorf(err code, char* restrict err_fmt, ...);

/**
 * Converts an integer error code to an error string
 *
 * @param code Error code
 *
 * @return Error string
 */
const char* errorToStr(err code);

#endif // _MISC_ERROR_H
