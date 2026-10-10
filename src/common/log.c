/**-----------------------------------------------------------------------*
 * Common error codes for client/server
 *
 * Copyright (C) 2026 @oxeylie - AGPL v3.0
 *-----------------------------------------------------------------------**/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/errno.h>

#include <log.h>

#define EOFST(pos) (pos - ELAST)

// Local declarations

static void print_log(char* log_str);
static void print_err(err code, char* err_str);
static void print_file(FILE* file, char* type_str, char* msg_str);

// Global vars

char* error_messages[ERR_COUNT] = {
    [OK] = "Operation succeeded",
    [EOFST(EINVLPRM)] = "Invalid parameter",
};

FILE* error_file = NULL;
FILE* log_file = NULL;

// Global functions

void print_logf(char* restrict msg_fmt, ...) {
  va_list args;
  char buf[256];

  va_start(args, msg_fmt);
  vsnprintf(buf, sizeof(buf), msg_fmt, args); // Pass variatic arg down to snprintf
  va_end(args);

  print_log(buf);
}

void print_errorf(err code, char* restrict err_fmt, ...) {
  va_list args;
  char buf[256];

  va_start(args, err_fmt);
  vsnprintf(buf, sizeof(buf), err_fmt, args); // Pass variatic arg down to snprintf
  va_end(args);

  print_err(code, buf);
}

void panic_errorf(err code, char* restrict err_fmt, ...) {
  va_list args;
  char buf[256];

  va_start(args, err_fmt);
  vsnprintf(buf, sizeof(buf), err_fmt, args); // Pass variatic arg down to snprintf
  va_end(args);

  print_err(code, buf);
  exit(EXIT_FAILURE);
}

char* error_code_str(err code) {
  if (code > 0 && code <= ELAST) // Code is errno
    return strerror(code);
  else if (code > ELAST && code < ECOUNT) // Code is custom
    return error_messages[code - ELAST];
  else if (code == 0) // Code is 0
    return error_messages[code];
  else
    return "Unknown error";
};

// Local

static void print_log(char* log_str) {
  print_file(error_file == NULL ? stdout : error_file, "Log", log_str);
}

static void print_err(err code, char* err_str) {
  if (code != 0) {
    char buf[256];
    snprintf(buf, sizeof(buf), "%s: %s", err_str, error_code_str(code)); // Translate code to human readable error
    print_file(error_file == NULL ? stderr : error_file, "Error", buf); // Print in configured file
  } else {
    print_file(error_file == NULL ? stderr : error_file, "Error", err_str); // Print in configured file
  }
}

static void print_file(FILE* file, char* type_str, char* msg_str) {
  time_t now = time(NULL);
  struct tm* t = localtime(&now);
  fprintf(file, "[%d/%d/%d-%d:%d][%s] %s\n", // Print with timestamp + message type
          t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
          t->tm_hour, t->tm_min,
          type_str, msg_str);
}
