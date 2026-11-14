/**-----------------------------------------------------------------------*
 * src/client/render/test.slang
 *
 * Copyright (C) 2026 @oxeylie
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *-----------------------------------------------------------------------**/

#include <shader/test.h>

StructuredBuffer<uint> buffer;
RWStructuredBuffer<uint> result;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 threadId: SV_DispatchThreadID) {
  uint index = threadId.x;
  result[index] = twice(buffer[index]);
}

