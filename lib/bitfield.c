/*
 * Copyright © 2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <bitfield.h>
#include <mmio.h>
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

uint32_t
mmio_get_bitfield_32(uintptr_t addr, uint32_t start, uint32_t width)
{
	return bitfield_get(mmio_read_32(addr), start, width);
}

void
mmio_set_bitfield_32(uintptr_t addr, uint32_t start, uint32_t width,
                     uint32_t value)
{
	uint32_t word = mmio_read_32(addr);

	mmio_write_32(addr, bitfield_set(word, start, width, value));
}
