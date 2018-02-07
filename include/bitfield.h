/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef BITFIELD_H
#define BITFIELD_H

#include <limits.h>
#include <stdint.h>
#include <util.h>

/**
 * Create a bitmask for the bits included in a bitfield.
 *
 * @param field The description of the bitfield.
 */
#define BF_MASK(field)          BITMASK(BF_OFFSET(field), BF_WIDTH(field))

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
 * Get the value of a bitfield.
 *
 * @param word  The word containing this bitfield.
 * @param field The description of the bitfield.
 */
static inline uint8_t
bitfield_get(uint32_t word, bitfield_t field)
{
	return (word >> BF_OFFSET(field)) & BITMASK(0, BF_WIDTH(field));
}

/**
 * Set the value of a bitfield, returning the updated word.
 *
 * @param word  The word containing this bitfield.
 * @param field The description of the bitfield.
 */
static inline uint32_t
bitfield_set(uint32_t word, bitfield_t field, uint8_t value)
{
	if (!BF_PRESENT(field))
		return word;

	return (word & ~BF_MASK(field)) |
	       ((value & BITMASK(0, BF_WIDTH(field))) << BF_OFFSET(field));
}

#endif /* BITFIELD_H */
