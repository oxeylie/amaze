/**-----------------------------------------------------------------------*
 * misc.h
 *
 * Copyright (C) 2026 @oxeylie
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *-----------------------------------------------------------------------**/

#ifndef MISC_H
#define MISC_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define _printf_like(fmt, va) __attribute__((format(printf, fmt, va)))
#define _nonnull(...) __attribute__((nonnull __VA_OPT__((__VA_ARGS__))))
#define _use_res [[nodiscard]]

// Argument function pointer
typedef size_t (*arg_func_t)(char** argv, size_t argc);

// Argument definition structure
struct arg_def {
  char* name;        // Name of the arguement eg: "-c"
  char* description; // Argument description
  arg_func_t func;   // Associated function
};

// Eval arguments in the context of the given argument table
void eval_args(char** argv, size_t argc, const struct arg_def* arglv, size_t arglc);
// Transforms nanoseconds to timespec
struct timespec ns_to_timespec(uint64_t ns);
// Transforms timespec to nanoseconds
uint64_t timespec_to_ns(struct timespec time_spec);

#endif // MISC_H
