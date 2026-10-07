/**-----------------------------------------------------------------------*
 * Common error codes for client/server
 *
 * Copyright (C) 2026 @oxeylie - AGPL v3.0
 *-----------------------------------------------------------------------**/

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <misc/log.h>

#define EOFST(pos) (pos - ELAST)

// Local declarations

static void printLog(char* log_str);
static void printErr(err code, char* err_str);
static void printToFile(FILE* file, char* type_str, char* msg_str);

// Global vars

char* error_messages[ERR_COUNT] = {
    [OK] = "Operation succeeded",
    [EOFST(EINVLPRM)] = "Invalid parameter",
};

FILE* error_file = NULL;
FILE* log_file = NULL;

// Global functions

void printLogf(char* restrict msg_fmt, ...) {
  va_list args;
  char buf[256];

  va_start(args, msg_fmt);
  vsnprintf(buf, sizeof(buf), msg_fmt, args); // Pass variatic arg down to snprintf
  va_end(args);

  printLog(buf);
}

void printErrorf(err code, char* restrict err_fmt, ...) {
  va_list args;
  char buf[256];

  va_start(args, err_fmt);
  vsnprintf(buf, sizeof(buf), err_fmt, args); // Pass variatic arg down to snprintf
  va_end(args);

  printErr(code, buf);
}

void panicErrorf(err code, char* restrict err_fmt, ...) {
  va_list args;
  char buf[256];

  va_start(args, err_fmt);
  vsnprintf(buf, sizeof(buf), err_fmt, args); // Pass variatic arg down to snprintf
  va_end(args);

  printErr(code, buf);
  exit(EXIT_FAILURE);
}

char* errorCodeToStr(err code) {
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

static void printLog(char* log_str) {
  printToFile(error_file == NULL ? stdout : error_file, "Log", log_str);
}

static void printErr(err code, char* err_str) {
  if (code != 0) {
    char buf[256];
    snprintf(buf, sizeof(buf), "%s: %s", err_str, errorCodeToStr(code)); // Translate code to human readable error
    printToFile(error_file == NULL ? stderr : error_file, "Error", buf); // Print in configured file
  } else {
    printToFile(error_file == NULL ? stderr : error_file, "Error", err_str); // Print in configured file
  }
}

static void printToFile(FILE* file, char* type_str, char* msg_str) {
  time_t now = time(NULL);
  struct tm* t = localtime(&now);
  fprintf(file, "[%d/%d/%d-%d:%d][%s] %s\n", // Print with timestamp + message type
          t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
          t->tm_hour, t->tm_min,
          type_str, msg_str);
}
