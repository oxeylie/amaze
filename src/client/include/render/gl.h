/**-----------------------------------------------------------------------*
 * src/client/include/render/compute.h
 *
 * Copyright (C) 2026 @oxeylie
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *-----------------------------------------------------------------------**/

#ifndef RENDER_GL_H
#define RENDER_GL_H

#include <compat/metal.h>
#include <macros.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if defined(__linux__)
#define GL_USE_VULKAN
#elif defined(__APPLE__)
#define GL_USE_METAL
#endif
#define _shader_file(name) STR(name.SHADER_EXT)

#define GLINST_MAX_DEVS 16
#define GLDEV_MAX_CMDQUEUES 16
#define GLDEV_MAX_SHADERS 64
#define GLDEV_MAX_PIPELINES 64

enum glpipeline_type {
  GLPIPL_COMPUTE = 0
};

typedef struct glpipeline_opdata* glpipeline_t;
typedef struct glshader_opdata* glshader_t;
typedef struct glshaderep_opdata* glshaderep_t;
typedef struct glcmdqueue_opdata* glcmdqueue_t;
typedef struct glinst* glinst_t;
typedef struct gldev* gldev_t;

// Opaque structures
struct glcmdqueue_opdata;
struct glshaderep_opdata;
struct glshaner_opdata;
struct glpipeline_opdata;
struct gldev_opdata;
struct glinst_opdata;

// Public structures
struct gldev {
  // Public
  glinst_t assoc_inst;       // Device's instance
  glcmdqueue_t* cmdqueues_v; // Device's cmdqueues
  uint cmdqueues_c;          // Command queue count
  glshader_t* shaders_v;     // Loaded shaders on device
  uint shaders_c;            // Shader count
  glpipeline_t* pipelines_v; // Loaded pipelines on device
  uint pipelines_c;          // Pipeline count

  /// Opaque data
  struct gldev_opdata* _data; // Implementation-specific data
};
struct glinst {
  // Public
  gldev_t* devs_v; // Available devices on system
  uint devs_c;     // Device count

  /// Opaque data
  struct glinst_opdata* _data; // Implementation-specific data
};

/// GL operations
// - [return] glinst_t on success, NULL on failure + errno
glinst_t init_glinst();
// Runs free for GL
// - instance contains the glinst_t to be freed
void shutdown_glinst(glinst_t instance);

/// GL devices operations
// Lists & alloc GPUs devices on the system
// - instance contains the glinst_t to be populated
// - [return] number of fetched devices on success, -1 on failure
int init_gldevs(glinst_t instance, gldev_t* devices, uint max_devices);
// Frees GPU device
// - device contains the device to be freed
void shutdown_gldev(gldev_t device);
// Inits the device's command queue
// - device contains the GPU from which the command queue must be initialized
// - [return] return glcmdqueue_t on success, NULL on failure + errno
glcmdqueue_t init_glcmdqueue(gldev_t device);

/// Shader operations
// Load a shader on a GPU device
// - device contains the GPU device on which the shader should be loaded
// - shader_data contains the shader to be loaded on the device
// - [return] glshader_t shader on success, NULL on failure + errno
glshader_t create_glshader(gldev_t device, uint8_t* shader_data);
// Return a shader entrypoint
// - shader contains the shader from which the entry point will be fetched
// - symbol_name entry point name
// - [return] glshaderep_t shader entry on success, NULL on failure + errno
glshaderep_t create_glshaderep(glshader_t shader, char* symbol_name);

// Create a compute pipeline on a device
// - device contains the GPU device on which the pipeline should be created
// - entrypoint contains the entrypoint for a shader function to be executed in the pipeline
// - [return] glpipeline_t pipeline on success, NULL on failure + errno
glpipeline_t create_glpipeline(gldev_t device, glshaderep_t entrypoint, enum glpipeline_type pipeline_type);

// void* gpu_create_cmdqueue(void* gpu);
// void* gpu_create_cmdbuf(void* cmdqueue);
// void* gpu_create_databuf(void* gpu, size_t size);

#endif // RENDER_GL_H
