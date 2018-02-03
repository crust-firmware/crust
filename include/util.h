/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef UTIL_H
#define UTIL_H

#include <limits.h>

#ifdef __ASSEMBLER__
#define U(n)                    (n)
#else
#define U(n)                    (n ## U)
#endif

#define ARRAY_SIZE(a)           (sizeof(a) / sizeof((a)[0]))

#define BIT(n)                  (U(1) << (n))
#define BITMASK(off, len)       ((BIT(len) - 1) << (off))

#define BITMAP_INDEX(word, bit) (WORD_BIT * (word) + (bit))
#define BITMAP_WORD(index)      ((index) / WORD_BIT)
#define BITMAP_WORDOFF(index)   (BITMAP_WORD(index) * (WORD_BIT / CHAR_BIT))
#define BITMAP_BIT(index)       ((index) % WORD_BIT)

#endif /* UTIL_H */
