/**-----------------------------------------------------------------------*
 * src/common/include/compat/metal.h
 *
 * Copyright (C) 2026 @oxeylie
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *-----------------------------------------------------------------------**/

#ifndef COMPAT_METAL_H
#define COMPAT_METAL_H
#ifdef __APPLE__
#include <compat/objc.h>
// Requires -Framework Metal and -Framework Foundation

// Defined in "Metal.framework/Headers/MTLResource.h"
enum MTLResourceOptions : NSUInteger {
  MTLResourceCPUCacheModeDefaultCache = 0 << 0,
  MTLResourceCPUCacheModeWriteCombined = 1 << 0,
  MTLResourceStorageModeShared = 0 << 4,
  MTLResourceStorageModeManaged = 1 << 4,
  MTLResourceStorageModePrivate = 2 << 4,
  MTLResourceStorageModeMemoryless = 3 << 4,
  MTLResourceHazardTrackingModeDefault = 0 << 8,
  MTLResourceHazardTrackingModeUntracked = 1 << 8,
  MTLResourceHazardTrackingModeTracked = 2 << 8,
};

extern void* MTLCopyAllDevices();

#endif
#endif // COMPAT_METAL_H
