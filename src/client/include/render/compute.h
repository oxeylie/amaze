/**-----------------------------------------------------------------------*
 * src/client/include/render/compute.h
 *
 * Copyright (C) 2026 @oxeylie
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *-----------------------------------------------------------------------**/

#ifndef RENDER_COMPUTE_H
#define RENDER_COMPUTE_H

#include <misc.h>

#include <stdbool.h>

#if defined(__linux__)
#define GL_USE_VULKAN
#elif defined(__APPLE__)
#define GL_USE_METAL
#endif
#define _shader_file(name) STR(name.SHADER_EXT)

void* initGPU();

#endif // RENDER_COMPUTE_H
