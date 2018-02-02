/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef BITMAP_H
#define BITMAP_H

#include <limits.h>

#define BITMAP_INDEX(word, bit) (WORD_BIT * (word) + (bit))
#define BITMAP_WORD(index)      ((index) / WORD_BIT)
#define BITMAP_WORDOFF(index)   (BITMAP_WORD(index) * (WORD_BIT / CHAR_BIT))
#define BITMAP_BIT(index)       ((index) % WORD_BIT)

#endif /* BITMAP_H */
