/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef BITMAP_H
#define BITMAP_H

#include <limits.h>
#include <mmio.h>
#include <stdbool.h>
#include <stdint.h>
#include <util.h>

#define BITMAP_INDEX(word, bit) (WORD_BIT * (word) + (bit))
#define BITMAP_WORD(index)      ((index) / WORD_BIT)
#define BITMAP_WORDOFF(index)   (BITMAP_WORD(index) * (WORD_BIT / CHAR_BIT))
#define BITMAP_BIT(index)       ((index) % WORD_BIT)

static inline void
bitmap_clear(uintptr_t base, uint32_t index)
{
	uintptr_t word = (uintptr_t)((uint32_t *)base + BITMAP_WORD(index));

	mmio_clearbits32(word, BIT(BITMAP_BIT(index)));
}

static inline bool
bitmap_get(uintptr_t base, uint32_t index)
{
	uintptr_t word = (uintptr_t)((uint32_t *)base + BITMAP_WORD(index));

	return !!(mmio_read32(word) & BIT(BITMAP_BIT(index)));
}

static inline void
bitmap_set(uintptr_t base, uint32_t index)
{
	uintptr_t word = (uintptr_t)((uint32_t *)base + BITMAP_WORD(index));

	mmio_setbits32(word, BIT(BITMAP_BIT(index)));
}

#endif /* BITMAP_H */
