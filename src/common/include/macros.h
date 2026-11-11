/**-----------------------------------------------------------------------*
 * src/common/include/macros.h
 * 
 * Copyright (C) 2026 @oxeylie
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *-----------------------------------------------------------------------**/

#ifndef MACRO_H
#define MACRO_H

#define PARENS ()
#define EXPAND(...) EXPAND4(EXPAND4(EXPAND4(EXPAND4(__VA_ARGS__))))
#define EXPAND4(...) EXPAND3(EXPAND3(EXPAND3(EXPAND3(__VA_ARGS__))))
#define EXPAND3(...) EXPAND2(EXPAND2(EXPAND2(EXPAND2(__VA_ARGS__))))
#define EXPAND2(...) EXPAND1(EXPAND1(EXPAND1(EXPAND1(__VA_ARGS__))))
#define EXPAND1(...) __VA_ARGS__

#define STR_(x) #x
#define STR(x) STR_(x)

#ifndef APPENDIX
#define V_STR MAJOR_V "." MINOR_V
#else
#define V_STR MAJOR_V "." MINOR_V "-" APPENDIX
#endif

#define _printf_like(fmt, va) __attribute__((format(printf, fmt, va)))
#define _nonnull(...) __attribute__((nonnull __VA_OPT__((__VA_ARGS__))))

#endif // MACRO_H
