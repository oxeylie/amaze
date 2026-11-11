/**-----------------------------------------------------------------------*
 * src/client/include/shader/test.h
 * 
 * Copyright (C) 2026 @oxeylie
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *-----------------------------------------------------------------------**/

#ifdef __STDC__ // true for a C compiler, not defined by glslang
#include <stdint.h>
#define FN static inline
typedef uint32_t uint;

#else
#define FN

#endif

FN uint twice(uint x) { return x * 2u; }
