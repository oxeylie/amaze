/**-----------------------------------------------------------------------*
 * Amaze mini-game server
 *
 * Copyright (C) 2026 @oxeylie - AGPL v3.0
 *-----------------------------------------------------------------------**/

#include <misc.h>

const struct arg_def arglv[] = {};

int main(int argc, char** argv) {
  evalArgsContext(argv, argc, arglv, sizeof(arglv) / sizeof(const struct arg_def));
  return 0;
}
