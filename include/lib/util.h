/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef UTIL_H
#define UTIL_H

#ifdef __ASSEMBLER__
#define U(n)              (n)
#else
#define U(n)              (n ## U)
#endif

#define ARRAY_SIZE(a)     (sizeof(a) / sizeof((a)[0]))

#define BIT(n)            (U(1) << (n))
#define BITMASK(off, len) ((BIT(len) - 1) << (off))

/**
 * Count the number of set bits in an integer.
 */
#define popcount(x)       __builtin_popcount(x)

#endif /* UTIL_H */
