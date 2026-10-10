/**-----------------------------------------------------------------------*
 * Common error codes for client/server
 *
 * Copyright (C) 2026 @oxeylie - AGPL v3.0
 *-----------------------------------------------------------------------**/

#ifndef _LOG_H
#define _LOG_H

#include <stdio.h>

#include <sys/errno.h>

#ifdef __linux__
#define ELAST 133 // Max errno code on linux
#endif
#define ERR_COUNT (ECOUNT - ELAST)

// Error codes
typedef enum {
  OK = 0,
  EINVLPRM = ELAST + 1,
  ECOUNT,
} err;

// Array of error message corresponding to codes
extern char* error_messages[ERR_COUNT];
// Error log file
extern FILE* error_file;
// Log file
extern FILE* log_file;

// Prints log with formated message
void printLogf(char* restrict msg_fmt, ...);
// Prints error with formated message
void printErrorf(err code, char* restrict err_fmt, ...);
// Prints & panic err with formated message
void panicErrorf(err code, char* restrict err_fmt, ...);
// Converts an integer error code to an error string
const char* errorToStr(err code);

#endif // _LOG_H
