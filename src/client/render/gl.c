/**-----------------------------------------------------------------------*
 * src/client/render/gl.c
 *
 * Copyright (C) 2026 @oxeylie
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *-----------------------------------------------------------------------**/

#include <render/gl.h>

#include <errno.h>
#include <math.h>

#if defined(GL_USE_VULKAN)
#include <compat/vulkan.h>
#elif defined(GL_USE_METAL)
#include <compat/metal.h>
#include <compat/objc.h>
#endif

struct gldev_t {
#if defined(GL_USE_VULKAN)
#elif defined(GL_USE_METAL)
  id dev_obj;
#endif
};

static bool _did_init;

/// GL Setup
int init_gl() {
  if (_did_init) {
    errno = EEXIST;
    return -1;
  }

#if !defined(GPU_SUPPORT)
  errno = ENOENT; // No library avail
  return -1;

#elif defined(GL_USE_VULKAN)
#elif defined(GL_USE_METAL)
#endif

  _did_init = true;
  return 0;
}

int free_gl() {
  if (!_did_init) {
    errno = ENOENT;
    return -1;
  }

#if defined(GL_USE_VULKAN)
#elif defined(GL_USE_METAL)
#endif

  return 0;
}

/// GL devices operations
int init_gldevs(gldev_t** devices, unsigned int count) {
#if defined(GL_USE_VULKAN)
#elif defined(GL_USE_METAL)
  id device_array = MTLCopyAllDevices();
  NSUInteger device_count = fmin(objc_CALLMTD(NSUInteger, device_array, objc_SEL(count)), count);

  for (NSUInteger i = 0; i < device_count; i++) {
    devices[i] = malloc(sizeof(gldev_t));
    devices[i]->dev_obj = objc_CALLMTD(id, device_array, objc_SEL(objectAtIndex), (NSUInteger)i);
  }

  objc_CALLMTD(void, device_array, objc_SEL(release));
  return device_count;

#endif
  return 0;
}

void free_gldev(gldev_t* device) {
#if defined(GL_USE_VULKAN)
#elif defined(GL_USE_METAL)
  objc_CALLMTD(void, device->dev_obj, objc_SEL(release));
  free(device);
#endif
}
