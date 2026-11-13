/**-----------------------------------------------------------------------*
 * src/common/include/compat/objc.h
 *
 * Copyright (C) 2026 @oxeylie
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *-----------------------------------------------------------------------**/

#ifndef COMPAT_OBJC_H
#define COMPAT_OBJC_H
#ifdef __APPLE__
// Requires -lobjc

#include <macros.h>

#include <objc/message.h>
#include <objc/objc.h>
#include <objc/runtime.h>

#define FOR_EACH_TYPEOF(...) \
  __VA_OPT__(EXPAND(FOR_EACH_TYPEOF_HELPER(__VA_ARGS__)))

#define FOR_EACH_TYPEOF_HELPER(first, ...) \
  typeof(first)                            \
  __VA_OPT__(, FOR_EACH_TYPEOF_AGAIN PARENS(__VA_ARGS__))

#define FOR_EACH_TYPEOF_AGAIN() FOR_EACH_TYPEOF_HELPER

#define objc_CLASS(class) (Class) objc_getClass(STR(class))
#define objc_SEL(sel) (SEL) sel_registerName(STR(sel))
#define objc_CALLMTD(ret_t, obj, sel, ...) ((ret_t (*)(                         \
    typeof(obj), SEL __VA_OPT__(, FOR_EACH_TYPEOF(__VA_ARGS__))))objc_msgSend)( \
    (obj), (sel)__VA_OPT__(, __VA_ARGS__))
#define NSString(c_str) objc_CALLMTD(id,                                                      \
                                     objc_CALLMTD(id, objc_CLASS(NSString), objc_SEL(alloc)), \
                                     objc_SEL(initWithUTF8String), c_str)

typedef long unsigned NSUInteger;

extern void* objc_autoreleasePoolPush();
extern void objc_autoreleasePoolPop(void* pool);

#endif
#endif // COMPAT_OBJC_H
