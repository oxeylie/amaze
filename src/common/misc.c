/**-----------------------------------------------------------------------*
 * Miscellaneous common utilities for client/server
 *
 * Copyright (C) 2026 @oxeylie - AGPL v3.0
 *-----------------------------------------------------------------------**/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include <sys/errno.h>

#include <log.h>
#include <misc.h>

void evalArgsContext(char** argv, size_t argc, const struct arg_def* arglv, size_t arglc) {
  for (int arg_idx = 1; arg_idx < argc; arg_idx++) { // Start at i = 1 because we don't care about path
    const struct arg_def* match_arg;
    char* name_arg = argv[arg_idx];
    bool match = false;

    for (int match_idx = 0; match_idx < arglc; match_idx++) { // Search for name match in arguements array
      match_arg = &arglv[match_idx];

      if (strcmp(name_arg, match_arg->name) == 0) { // If name of args matches with an available arg
        match = true;
        break;
      }
    }

    if (!match) panicErrorf(EINVAL, "Evaluating arg \"%s\"", name_arg); // Return error if no match

    int offset;

    offset = match_arg->func(argv + arg_idx + 1, argc - arg_idx - 1);     // Execute associated function and pass all args after arg_name
    if (offset == -1) panicErrorf(errno, "Parsing arg \"%s\"", name_arg); // Return if error

    arg_idx += offset;
  }
}

inline struct timespec nsToTimespec(uint64_t ns) {
  return (struct timespec){
      .tv_sec = ns / 1000000000,
      .tv_nsec = ns % 1000000000};
}

inline uint64_t timespecToNs(struct timespec time_spec) {
  return time_spec.tv_sec * 1000000000 + time_spec.tv_nsec;
}
