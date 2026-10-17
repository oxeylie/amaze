/**-----------------------------------------------------------------------*
 * src/tests/client/main.c
 *
 * Copyright (C) 2026 @oxeylie
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *-----------------------------------------------------------------------**/

#include <stdio.h>

int main() {
  printf("[info] Launching tests for client\n");
  unsigned int failed_tests = 0;

  if (failed_tests > 0) printf("[fail] %ud tests failed\n", failed_tests);
  else printf("[success] All tests passed\n");
  return failed_tests > 0 ? -1 : 0;
}
