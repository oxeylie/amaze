/**-----------------------------------------------------------------------*
 * src/common/include/misc.h
 *
 * Copyright (C) 2026 @oxeylie
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *-----------------------------------------------------------------------**/

#ifndef MISC_H
#define MISC_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/// Program args operation
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

/// Time operations
// Transforms nanoseconds to timespec
struct timespec ns_to_timespec(uint64_t ns);
// Transforms timespec to nanoseconds
uint64_t timespec_to_ns(struct timespec time_spec);

/// Array ops
#define ARRAY_PUSH(array, index, value, len, max)        \
  __extension__({                                        \
    __typeof__(index) _index = (index);                  \
    _index = _index > 0 ? _index : len + 1 + _index;     \
    __typeof__(max) _max = (max);                        \
    __typeof__(len) _new_len = (len) + 1;                \
    __typeof__(len) _rem_len = (len) - _index;           \
    size_t _size = _rem_len * sizeof(*(array));          \
    __typeof__(*(array))* _rem_array = (array) + _index; \
    if (_rem_len > 0 && _new_len <= max)                 \
      memmove(_rem_array + 1, _rem_array, _size);        \
    if (_new_len <= max) *_rem_array = (value);          \
    _new_len <= _max ? _new_len : -1;                    \
  })
#define ARRAY_POP(array, index, len)                     \
  __extension__({                                        \
    __typeof__(index) _index = (index);                  \
    _index = _index >= 0 ? _index : len + _index;        \
    __typeof__(len) _new_len = (len) - 1;                \
    __typeof__(len) _rem_len = _new_len - _index;        \
    size_t _size = _rem_len * sizeof(*(array));          \
    __typeof__(*(array))* _rem_array = (array) + _index; \
    if (_rem_len >= 0 && _rem_len < len)                 \
      memmove(_rem_array, _rem_array + 1, _size);        \
    _rem_len >= 0 && _rem_len < len ? _new_len : -1;     \
  })

/// Error checks
#define CHECK_PANIC(expr, message) \
  if ((expr) == -1) panic_errorf(errno, message);
#define CHECK_ERROR(expr, message) \
  if ((expr) == -1) print_errorf(errno, message);

#endif // MISC_H
