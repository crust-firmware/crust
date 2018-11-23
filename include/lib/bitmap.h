/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef LIB_BITMAP_H
#define LIB_BITMAP_H

#include <limits.h>
#include <mmio.h>
#include <stdbool.h>
#include <stdint.h>
#include <util.h>

/**
 * Calculate a bit index from a (system word size) word offset and a bit offset
 * within that word.
 *
 * @param word The offset of the word from the beginning of the bitmap.
 * @param bit  The offset (arithmetic shift) of the bit within the word.
 */
#define BITMAP_INDEX(word, bit) (WORD_BIT * (word) + (bit))

/**
 * Extract the word offset from a bitmap index.
 *
 * @param index An index into a bitmap, in bits.
 */
#define BITMAP_WORD(index)      ((index) / WORD_BIT)

/**
 * Extract the bit offset (arithmetic shift) from a bitmap index.
 *
 * @param index An index into a bitmap, in bits.
 */
#define BITMAP_BIT(index)       ((index) % WORD_BIT)

/**
 * Clear a bit in a bitmap.
 *
 * @param base  The address of the start of the bitmap.
 * @param index The index into the bitmap (in bits) of the bit to clear.
 */
static inline void
bitmap_clear(uintptr_t base, uint32_t index)
{
	uintptr_t word = (uintptr_t)((uint32_t *)base + BITMAP_WORD(index));

	mmio_clearbits32(word, BIT(BITMAP_BIT(index)));
}

/**
 * Get a bit in a bitmap.
 *
 * @param base  The address of the start of the bitmap.
 * @param index The index into the bitmap (in bits) of the bit to get.
 */
static inline bool
bitmap_get(uintptr_t base, uint32_t index)
{
	uintptr_t word = (uintptr_t)((uint32_t *)base + BITMAP_WORD(index));

	return mmio_read32(word) & BIT(BITMAP_BIT(index));
}

/**
 * Set a bit in a bitmap.
 *
 * @param base  The address of the start of the bitmap.
 * @param index The index into the bitmap (in bits) of the bit to set.
 */
static inline void
bitmap_set(uintptr_t base, uint32_t index)
{
	uintptr_t word = (uintptr_t)((uint32_t *)base + BITMAP_WORD(index));

	mmio_setbits32(word, BIT(BITMAP_BIT(index)));
}

#endif /* LIB_BITMAP_H */
