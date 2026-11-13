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

#if defined(__linux__)
#define GL_USE_VULKAN
#elif defined(__APPLE__)
#define GL_USE_METAL
#endif
#define _shader_file(name) STR(name.SHADER_EXT)

struct gldev_t;
typedef struct gldev_t gldev_t;

// Runs init for GL
// - [return] 0 on success, -1 on failure + errno
int init_gl();
// Runs free for GL
// - [return] 0 on success, -1 on failure + errno
int free_gl();

// Lists & alloc GPUs devices on the system
// - devices contains a list to be populated by devices
// - count contains the maximum number of devices to be fetched
// - [return] number of fetched devices on success, -1 on failure
int init_gldevs(gldev_t** devices, unsigned int count);
// Frees GPU device
// - device conatins the adress of the device to be freed
void free_gldev(gldev_t* device);

// void* gldev_load_shader(void* device, char* shader_str);
// void* shader_get_fnref(void* shader, char* symbol_name);
// void* gpu_create_fnpipeline(void* gpu, void* shader_fn);
// void* gpu_create_cmdqueue(void* gpu);
// void* gpu_create_cmdbuf(void* cmdqueue);
// void* gpu_create_databuf(void* gpu, size_t size);

#endif // RENDER_GL_H
