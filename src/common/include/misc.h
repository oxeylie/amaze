/**-----------------------------------------------------------------------*
 * Miscellaneous common utilities for client / server
 *
 * Copyright (C) 2026 @oxeylie - AGPL v3.0
 *-----------------------------------------------------------------------**/

#ifndef _MISC_H
#define _MISC_H

#include <stddef.h>
#include <stdint.h>
#include <time.h>

// Argument function pointer
typedef size_t (*arg_func_t)(char** argv, size_t argc);

// Argument definition structure
struct arg_def {
  char* name;        // Name of the arguement eg: "-c"
  char* description; // Argument description
  arg_func_t func; // Associated function
};

// Eval arguments in the context of the given argument table
void evalArgsContext(char** argv, size_t argc, const struct arg_def* arglv, size_t arglc);
// Transforms nanoseconds to timespec
struct timespec nsToTimespec(uint64_t ns);
// Transforms timespec to nanoseconds
uint64_t timespecToNs(struct timespec time_spec);

#endif // _MISC_H
