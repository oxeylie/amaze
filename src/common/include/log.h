/**-----------------------------------------------------------------------*
 * log.h
 *
 * Copyright (C) 2026 @oxeylie
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *-----------------------------------------------------------------------**/

#ifndef LOG_H
#define LOG_H

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
void print_logf(char* restrict msg_fmt, ...) __attribute__((nonnull));
// Prints error with formated message
void print_errorf(err code, char* restrict err_fmt, ...) __attribute__((nonnull));
// Prints & panic err with formated message
void panic_errorf(err code, char* restrict err_fmt, ...) __attribute__((nonnull));
// Converts an integer error code to an error string
const char* error_str(err code);

#endif // LOG_H
