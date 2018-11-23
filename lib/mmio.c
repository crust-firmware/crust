/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <mmio.h>
#include <stdint.h>

void
mmio_poll32(uintptr_t addr, uint32_t mask)
{
	while ((mmio_read32(addr) & mask) != mask) {
		/* Wait for the bits to go high. */
	}
}

void
mmio_pollzero32(uintptr_t addr, uint32_t mask)
{
	while ((mmio_read32(addr) & mask) != 0) {
		/* Wait for the bits to go low. */
	}
}
