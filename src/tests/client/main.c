/**-----------------------------------------------------------------------*
 * src/tests/client/main.c
 *
 * Copyright (C) 2026 @oxeylie
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *-----------------------------------------------------------------------**/

#include <log.h>

int main() {
  printf("\n");
  print_logf("Info: Launching tests for client");
  unsigned int failed_tests = 0;

  if (failed_tests > 0) print_errorf(0, "Fail: %u tests failed", failed_tests);
  else print_logf("Success: All tests passed");
  return failed_tests > 0 ? -1 : 0;
}
