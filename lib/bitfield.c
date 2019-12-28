/*
 * Copyright © 2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <bitfield.h>
#include <stdint.h>
#include <util.h>

uint32_t
bitfield_get(uint32_t word, uint32_t start, uint32_t width)
{
	return (word >> start) & (BIT(width) - 1);
}

uint32_t
bitfield_set(uint32_t word, uint32_t start, uint32_t width, uint32_t value)
{
	return word ^ ((value << start ^ word) & ((BIT(width) - 1) << start));
}
