/**-----------------------------------------------------------------------*
 * src/common/include/log.h
 * 
 * Copyright (C) 2026 @oxeylie
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *-----------------------------------------------------------------------**/

#ifndef LOG_H
#define LOG_H

#include <misc.h>
#include <stdio.h>
#include <sys/errno.h>

#if defined(DEBUG_V) // Debug_Verbose functions
#define DEBUG_LOGF(strf, ...) print_logf(strf __VA_OPT__(, ) __VA_ARGS__)
#define DEBUG_ERRF(code, strf, ...) print_errf(code, strf __VA_OPT__(, ) __VA_ARGS__)
#else
#define DEBUG_LOGF(strf, ...)
#define DEBUG_ERRF(code, strf, ...)
#endif

#if defined(__linux__)
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
void print_logf(char* const restrict msg_fmt, ...) _printf_like(1, 2) _nonnull(1);
// Prints error with formated message
void print_errorf(err code, const char* restrict err_fmt, ...) _printf_like(2, 3) _nonnull(2);
// Prints & panic err with formated message
void panic_errorf(err code, const char* restrict err_fmt, ...) _printf_like(2, 3) _nonnull(2);
// Converts an integer error code to an error string
const char* error_str(err code);

#endif // LOG_H
