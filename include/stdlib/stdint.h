/*
 * Copyright © 2005-2014 Rich Felker, et al.
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef STDLIB_STDINT_H
#define STDLIB_STDINT_H

#define INT8_MAX    0x7f
#define INT8_MIN    (-1 - 0x7f)
#define INT16_MAX   0x7fff
#define INT16_MIN   (-1 - 0x7fff)
#define INT32_MAX   0x7fffffff
#define INT32_MIN   (-1 - 0x7fffffff)
#define INT64_MAX   0x7fffffffffffffff
#define INT64_MIN   (-1 - 0x7fffffffffffffff)

#define INTMAX_MAX  INT64_MAX
#define INTMAX_MIN  INT64_MIN
#define INTPTR_MAX  INT32_MAX
#define INTPTR_MIN  INT32_MIN

#define PTRDIFF_MAX INT32_MAX
#define PTRDIFF_MIN INT32_MIN

#define SIZE_MAX    UINT32_MAX

#define UINT8_MAX   0xff
#define UINT16_MAX  0xffff
#define UINT32_MAX  0xffffffffU
#define UINT64_MAX  0xffffffffffffffffU

#define UINTMAX_MAX UINT64_MAX
#define UINTPTR_MAX UINT32_MAX

typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef long long          int64_t;

typedef long long          intmax_t;
typedef int                intptr_t;

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned           uint32_t;
typedef unsigned long long uint64_t;

typedef unsigned long long uintmax_t;
typedef unsigned           uintptr_t;

#endif /* STDLIB_STDINT_H */
