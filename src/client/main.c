/**-----------------------------------------------------------------------*
 * Amaze mini-game client
 *
 * Copyright (C) 2026 @oxeylie - AGPL v3.0
 *-----------------------------------------------------------------------**/

#include <misc.h>

const struct arg_def argl[] = {};

int main(int argc, char** argv) {
  evalArgsContext(argv, argc, argl, sizeof(argl) / sizeof(const struct arg_def));
  return 0;
}
