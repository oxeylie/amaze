/**-----------------------------------------------------------------------*
 * Miscellaneous common utilities for client / server
 *
 * Copyright (C) 2026 @oxeylie - AGPL v3.0
 *-----------------------------------------------------------------------**/

#ifndef _MISC_H
#define _MISC_H

#include <misc/log.h>
#include <stdint.h>
#include <time.h>

/// Argument Parser

/**
 * Executes code given following args
 *
 * @param argv Args following the given argument
 * @param argc Number of args
 *
 * @return Number of args used
 */
typedef int (*argumentFunc)(char** argv, int argc);

/**
 * Program argument struct
 */
struct programArgument {
  char* name;        // Name of the arguement eg: "-c"
  char* description; // Argument description
  argumentFunc func; // Associated function
};

/**
 * Eval arguments in the context of the given argument table
 *
 * @param argv Arguments to parse
 * @param args Argument table for matching (NULL-Terminated)
 *
 * @return Error code
 */
void evalArgsContext(char** argv, int argc, const struct programArgument* args);

struct timespec nsToTimespec(uint64_t ns);
uint64_t timespecToNs(struct timespec time_spec);

#endif // _MISC_H
