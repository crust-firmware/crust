/*
 * Copyright © 2005-2014 Rich Felker, et al.
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef STDLIB_LIMITS_H
#define STDLIB_LIMITS_H

#define CHAR_BIT   8
#define CHAR_MAX   0xff
#define CHAR_MIN   0

#define WORD_BIT   32
#define INT_MAX    0x7fffffff
#define INT_MIN    (-1 - 0x7fffffff)

#define LONG_BIT   32
#define LONG_MAX   0x7fffffffL
#define LONG_MIN   (-0x7fffffffL - 1)
#define LLONG_MAX  0x7fffffffffffffffLL
#define LLONG_MIN  (-0x7fffffffffffffffLL - 1)

#define SCHAR_MAX  0x7f
#define SCHAR_MIN  (-1 - 0x7f)

#define SHRT_MAX   0x7fff
#define SHRT_MIN   (-1 - 0x7fff)

#define UCHAR_MAX  0xff

#define UINT_MAX   0xffffffffU

#define ULONG_MAX  0xffffffffUL
#define ULLONG_MAX 0xffffffffffffffffULL

#define USHRT_MAX  0xffff

#endif /* STDLIB_LIMITS_H */
