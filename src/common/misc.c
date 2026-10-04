/**-----------------------------------------------------------------------*
 * Miscellaneous common utilities for client/server
 *
 * Copyright (C) 2026 @oxeylie - GPL v3
 *-----------------------------------------------------------------------**/

#include <misc/misc.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/errno.h>
#include <time.h>

void evalArgsContext(char** argv, int argc, const struct programArgument* args) {
  for (int arg_idx = 1; arg_idx < argc; arg_idx++) { // Start at i = 1 because we don't care about path
    const struct programArgument* match_arg;
    char* name_arg = argv[arg_idx];
    bool match = false;

    for (int match_idx = 0; args[match_idx].name != NULL; match_idx++) { // Search for name match in arguements array
      match_arg = &args[match_idx];

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
