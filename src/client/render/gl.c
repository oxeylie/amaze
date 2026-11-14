/**-----------------------------------------------------------------------*
 * src/client/render/gl.c
 *
 * See docs:
 *  - [Metal] https://developer.apple.com/documentation/metal/
 *  - [Vulkan] https://docs.vulkan.org/spec/latest
 *
 * Copyright (C) 2026 @oxeylie
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *-----------------------------------------------------------------------**/

// TODO:
// - Better error handling on metal
// - VK Implementation

#include <misc.h>
#include <objc/objc-api.h>
#include <render/gl.h>

#include <errno.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/errno.h>

#if defined(GL_USE_VULKAN)
#include <compat/vulkan.h>
#elif defined(GL_USE_METAL)
#include <compat/metal.h>
#include <compat/objc.h>
#endif

static glinst_t _alloc_glinst();
static void _free_glinst(glinst_t instance);
static gldev_t _alloc_gldev();
static void _free_gldev(gldev_t device);
static glcmdqueue_t _alloc_glcmdqueue();
static void _free_glcmdqueue(glcmdqueue_t command_queue);
static glshader_t _alloc_glshader();
static void _free_glshader(glshader_t shader);
static glshaderep_t _alloc_glshaderep();
static void _free_glshaderep(glshaderep_t shader_entry);
static glpipeline_t _alloc_glpipeline();
static void _free_glpipeline(glpipeline_t pipeline);

struct gldev_opdata {
#if defined(GL_USE_VULKAN)
#elif defined(GL_USE_METAL)
  id dev_obj;
#endif
};
struct glcmdqueue_opdata {
#if defined(GL_USE_VULKAN)
#elif defined(GL_USE_METAL)
  id cmdqueue_obj;
#endif
};
struct glshader_opdata {
#if defined(GL_USE_VULKAN)
#elif defined(GL_USE_METAL)
  id shader_obj;
#endif
};
struct glshaderep_opdata {
#if defined(GL_USE_VULKAN)
#elif defined(GL_USE_METAL)
  id shaderep_obj;
#endif
};
struct glpipeline_opdata {
#if defined(GL_USE_VULKAN)
#elif defined(GL_USE_METAL)
  id pipeline_obj;
#endif
};

/// GLInstances operations
glinst_t init_glinst() {
#if !defined(GPU_SUPPORT)
  errno = ENOTSUP; // No library avail
  return NULL;
#endif
  glinst_t instance = _alloc_glinst();

  return instance;
}

void shutdown_glinst(glinst_t instance) {
  _free_glinst(instance);
}

/// GL devices operations
int init_gldevs(glinst_t instance, gldev_t* devices, uint max_devices) {
  uint device_count;

#if defined(GL_USE_VULKAN)
#elif defined(GL_USE_METAL)
  id device_array = MTLCopyAllDevices();
  device_count = fmin(objc_CALLMTD(NSUInteger, device_array, objc_SEL(count)), max_devices);

  if (device_count != instance->devs_c) instance->devs_v = realloc(instance->devs_v, device_count * sizeof(gldev_t));

  for (uint i = 0; i < device_count; i++) {
    gldev_t dev = _alloc_gldev();
    dev->assoc_inst = instance;
    dev->_data->dev_obj = objc_CALLMTD(id, device_array, objc_SEL(objectAtIndex), (NSUInteger)i);
    devices[i] = instance->devs_v[i] = dev;
  }

  objc_CALLMTD(void, device_array, objc_SEL(release));
  instance->devs_c = device_count;
#endif

  return device_count;
}
void shutdown_gldev(gldev_t device) {
  glinst_t instance = device->assoc_inst;
  uint dev_idx;
  for (dev_idx = 0; dev_idx < instance->devs_c; dev_idx++) // Search for device in instance's device list
    if (instance->devs_v[dev_idx] == device) break;

  if (dev_idx < instance->devs_c) { // If device found in list
    int ncount = ARRAY_POP(instance->devs_v, dev_idx, instance->devs_c);
    if (ncount >= 0) instance->devs_v = realloc(instance->devs_v, (instance->devs_c = ncount) * sizeof(gldev_t));
  }

#if defined(GL_USE_VULKAN)
#elif defined(GL_USE_METAL)
  objc_CALLMTD(void, device->_data->dev_obj, objc_SEL(release));
#endif

  _free_gldev(device);
}

glcmdqueue_t init_glcmdqueue(gldev_t device) {
  glcmdqueue_t cmdqueue;

#if defined(GL_USE_VULKAN)
#elif defined(GL_USE_METAL)
  id cmdqueue_obj = objc_CALLMTD(id, device->_data->dev_obj,
                                 objc_SEL(newCommandQueue));
  if (cmdqueue_obj == NULL) return NULL; // Error creating queue

  cmdqueue = _alloc_glcmdqueue();
  cmdqueue->cmdqueue_obj = cmdqueue_obj;
#endif

  device->cmdqueues_v = realloc(device->cmdqueues_v, device->cmdqueues_c + 1);
  int res = ARRAY_PUSH(device->cmdqueues_v, -1, cmdqueue, device->shaders_c, GLDEV_MAX_CMDQUEUES);

  if (res > 0) {
    device->cmdqueues_c = res;
    return cmdqueue;
  } else {
    // TODO: free cmdqueue
    return NULL;
  }
}

/// Shader operations
glshader_t create_glshader(gldev_t device, uint8_t* shader_data) {
  glshader_t shader;

#if defined(GL_USE_VULKAN)
#elif defined(GL_USE_METAL)
  id shader_obj = objc_CALLMTD(id, device->_data->dev_obj, // Compile MSL shader into a metal library
                               objc_SEL(newLibraryWithSource), NSString(shader_data),
                               objc_SEL(options), NULL,
                               objc_SEL(error), NULL);
  if (shader_obj == NULL) return NULL; // Error loading shader

  shader = _alloc_glshader();
  shader->shader_obj = shader_obj;
#endif

  device->shaders_v = realloc(device->shaders_v, device->shaders_c + 1);
  int res = ARRAY_PUSH(device->shaders_v, -1, shader, device->shaders_c, GLDEV_MAX_SHADERS); // Add shader to device list

  if (res > 0) {
    device->shaders_c = res;
    return shader;
  } else {
    // TODO: free shader_obj
    return NULL;
  }
}

glshaderep_t create_glshaderep(glshader_t shader, char* symbol_name) {
  glshaderep_t shaderep;

#if defined(GL_USE_VULKAN)
#elif defined(GL_USE_METAL)
  id shaderep_obj = objc_CALLMTD(id, shader->shader_obj,
                                 objc_SEL(newFunctionWithName), NSString(symbol_name));
  if (shaderep_obj == NULL) return NULL; // Entry point not found

  shaderep = _alloc_glshaderep();
  shaderep->shaderep_obj = shaderep_obj;
#endif

  return shaderep;
}

glpipeline_t create_glpipeline(gldev_t device, glshaderep_t entrypoint, enum glpipeline_type type) {
  glpipeline_t pipeline;

#if defined(GL_USE_VULKAN)
#elif defined(GL_USE_METAL)
  switch (type) {
  case GLPIPL_COMPUTE:
    id pipeline_obj = objc_CALLMTD(id, device->_data->dev_obj,
                                   objc_SEL(newComputePipelineStateWithFunction), entrypoint->shaderep_obj,
                                   objc_SEL(error), NULL);
    if (pipeline_obj == NULL) return NULL; // Error setting up pipeline

    pipeline = _alloc_glpipeline();
    pipeline->pipeline_obj = pipeline_obj;
    break;
  default:
    errno = EINVAL;
    return NULL;
  }
#endif

  device->pipelines_v = realloc(device->pipelines_v, device->pipelines_c + 1);
  int res = ARRAY_PUSH(device->pipelines_v, -1, pipeline, device->pipelines_c, GLDEV_MAX_PIPELINES); // Add shader to device list

  if (res > 0) {
    device->pipelines_c = res;
    return pipeline;
  } else {
    // TODO: free pipeline
    return NULL;
  }
}

/// Local functions
inline glinst_t _alloc_glinst() {
  glinst_t instance = malloc(sizeof(glinst_t));
  instance->devs_v = malloc(sizeof(gldev_t));
  return instance;
}
inline void _free_glinst(glinst_t instance) {
  free(instance->devs_v);
  free(instance);
}

inline gldev_t _alloc_gldev() {
  gldev_t dev = malloc(sizeof(gldev_t));
  dev->_data = malloc(sizeof(struct gldev_opdata));
  dev->shaders_v = malloc(sizeof(glshader_t));
  dev->pipelines_v = malloc(sizeof(glpipeline_t));
  dev->cmdqueues_v = malloc(sizeof(glcmdqueue_t));
  return dev;
}
inline void _free_gldev(gldev_t device) {
  free(device->_data);
  free(device);
}

inline glcmdqueue_t _alloc_glcmdqueue() {
  return malloc(sizeof(glcmdqueue_t));
}
inline void _free_glcmdqueue(glcmdqueue_t command_queue) {
  free(command_queue);
}

inline glshader_t _alloc_glshader() {
  return malloc(sizeof(glshader_t));
}
inline void _free_glshader(glshader_t shader) {
  free(shader);
}

inline glshaderep_t _alloc_glshaderep() {
  return malloc(sizeof(glshaderep_t));
}
inline void _free_glshaderep(glshaderep_t shader_entry) {
  free(shader_entry);
}

inline glpipeline_t _alloc_glpipeline() {
  return malloc(sizeof(glpipeline_t));
}
inline void _free_glpipeline(glpipeline_t pipeline) {
  free(pipeline);
}
