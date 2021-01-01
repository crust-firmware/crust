/*
 * Copyright © 2017-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef LIB_UTIL_H
#define LIB_UTIL_H

#ifdef __ASSEMBLER__
#define U(n)          (n)
#else
#define U(n)          (n ## U)
#endif

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define BIT(n)        (U(1) << (n))

#define GENMASK(h, l) ((U(0xffffffff) << (l)) & (U(0xffffffff) >> (31 - (h))))

#endif /* LIB_UTIL_H */
