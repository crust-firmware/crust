/*
 * Copyright © 2017-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <delay.h>
#include <mmio.h>
#include <stdbool.h>
#include <stdint.h>
#include <util.h>

#include "css.h"

static const uint8_t power_switch_on_sequence[] = {
	0xfe, 0xf8, 0xe0, 0xc0, 0x80, 0x00,
};

void
css_set_power_switch(uintptr_t addr, bool enable)
{
	if (enable) {
		/* Avoid killing the power if the switch is already enabled. */
		if (mmio_read_32(addr) == 0x00)
			return;

		/* Allwinner's blob uses 10, 20, and 30μs delays, depending on
		 * the iteration. However, the same code works fine in ATF with
		 * no delays. The 10μs delay is here just to be extra safe. */
		const uint8_t *sequence = power_switch_on_sequence;
		do {
			mmio_write_32(addr, *sequence);
			udelay(10);
		} while (*sequence++ != 0x00);
	} else {
		mmio_write_32(addr, 0xff);
	}
}
