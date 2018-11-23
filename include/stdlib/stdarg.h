/*
 * Copyright © 2005-2014 Rich Felker, et al.
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef STDLIB_STDARG_H
#define STDLIB_STDARG_H

#define va_arg(v, l)   __builtin_va_arg(v, l)
#define va_copy(d, s)  __builtin_va_copy(d, s)
#define va_end(v)      __builtin_va_end(v)
#define va_start(v, l) __builtin_va_start(v, l)

typedef __builtin_va_list va_list;

#endif /* STDLIB_STDARG_H */
