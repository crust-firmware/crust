/*
 * Copyright © 2005-2014 Rich Felker, et al.
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef STDLIB_STDDEF_H
#define STDLIB_STDDEF_H

#define NULL                   ((void *)0)
#define offsetof(type, member) __builtin_offsetof(type, member)

typedef int          ptrdiff_t;
typedef unsigned int size_t;

#endif /* STDLIB_STDDEF_H */
