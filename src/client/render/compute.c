/**-----------------------------------------------------------------------*
 * src/client/render/compute.c
 *
 * Copyright (C) 2026 @oxeylie
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *-----------------------------------------------------------------------**/

#include <render/compute.h>
#include <shader/test.h>

#if defined(GL_USE_VULKAN)
#include <compat/vulkan.h>
#elif defined(GL_USE_METAL)
#include <compat/metal.h>
#include <compat/objc.h>
#endif

#if defined(GL_USE_VULKAN)

#elif defined(GL_USE_METAL)
#endif

#ifdef GPU_SUPPORT
char compute_shader[] = {
#embed _shader_file(test) suffix(, 0)
};
#endif

void* initGPU() {
  void* device;
#if defined(GL_USE_METAL)
  device = MTLCreateSystemDefaultDevice();
#endif
  return device;
}

void* loadShader(void* gpu, char* shader_str) {
  void* shader = NULL;
#if defined(GL_USE_METAL)
  shader = objc_MTD(id, gpu,
                    objc_SEL(newLibraryWithSource), NSString(shader_str),
                    objc_SEL(options), NULL,
                    objc_SEL(error), NULL);
#endif
  return shader;
}

void* getShaderEntryPoint(void* shader, char* symbol_name) {
  void* func_ptr;
#if defined(GL_USE_METAL)
  func_ptr = objc_MTD(id, shader, objc_SEL(newFunctionWithName), NSString(symbol_name));
#endif
  return func_ptr;
}
