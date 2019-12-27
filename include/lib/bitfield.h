/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef LIB_BITFIELD_H
#define LIB_BITFIELD_H

#include <stdint.h>
#include <util.h>

/**
 * Create a bitmask for the bits included in a bitfield.
 *
 * @param field The description of the bitfield.
 */
#define BF_MASK(f)              ((BIT(BF_WIDTH(f)) - 1) << BF_OFFSET(f))

/**
 * Extract the offset portion of a bitfield description.
 *
 * @param field The description of the bitfield.
 */
#define BF_OFFSET(field)        ((field) & 0x1f)

/**
 * Return a boolean value for if a bitfield description represents a usable
 * (non-zero-width) bitfield.
 *
 * @param field The description of the bitfield.
 */
#define BF_PRESENT(field)       (BF_WIDTH(field) > 0)

/**
 * Extract the width portion of a bitfield description.
 *
 * @param field The description of the bitfield.
 */
#define BF_WIDTH(field)         ((field) >> 5)

/**
 * Create a bitfield description from an offset and width.
 *
 * @param offset The offset in bits of the LSB of the bitfield from the LSB of
 *               the word containing the bitfield.
 * @param width  The width of the bitfield in bits.
 */
#define BITFIELD(offset, width) (((width) << 5) | ((offset) & 0x1f))

/**
 * Create a bitfield description representing a non-present bitfield.
 */
#define BITFIELD_NONE           BITFIELD(0, 0)

typedef uint8_t bitfield_t;

/**
 * Get the value in a bitfield.
 *
 * @param word  A word containing a bitfield.
 * @param start The offset of the starting bit (LSB) of the bitfield.
 * @param width The width of the bitfield in bits.
 */
static inline uint32_t
bitfield_get(uint32_t word, uint32_t start, uint32_t width)
{
	return (word >> start) & (BIT(width) - 1);
}

/**
 * Set the value in a bitfield.
 *
 * @param word  A word containing a bitfield.
 * @param start The offset of the starting bit (LSB) of the bitfield.
 * @param width The width of the bitfield in bits.
 * @param value The value to place in the bitfield.
 * @return      The original word, with the value of the bitfield replaced.
 */
static inline uint32_t
bitfield_set(uint32_t word, uint32_t start, uint32_t width, uint32_t value)
{
	return word ^ ((value << start ^ word) & ((BIT(width) - 1) << start));
}

#endif /* LIB_BITFIELD_H */
