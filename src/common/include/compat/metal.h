/**-----------------------------------------------------------------------*
 * src/common/include/compat/metal.h
 *
 * Copyright (C) 2026 @oxeylie
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *-----------------------------------------------------------------------**/

#ifndef COMPAT_METAL_H
#define COMPAT_METAL_H
#ifdef __APPLE__
// Requires -Framework Metal and -Framework Foundation

extern void* MTLCreateSystemDefaultDevice();

#endif
#endif // COMPAT_METAL_H
