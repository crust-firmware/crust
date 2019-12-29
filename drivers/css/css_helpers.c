/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <delay.h>
#include <mmio.h>
#include <stdbool.h>
#include <stdint.h>
#include <util.h>

#include "css.h"

static const uint8_t power_switch_off_sequence[] = {
	0xff,
};

static const uint8_t power_switch_on_sequence[] = {
	0xfe, 0xf8, 0xe0, 0x80, 0x00,
};

void
css_set_power_switch(uintptr_t addr, bool enable)
{
	const uint8_t *values;
	uint32_t length;

	if (enable) {
		values = power_switch_on_sequence;
		length = ARRAY_SIZE(power_switch_on_sequence);
	} else {
		values = power_switch_off_sequence;
		length = ARRAY_SIZE(power_switch_off_sequence);
	}

	/* Avoid killing the power if the switch is already enabled. */
	if (mmio_read_32(addr) == values[length - 1])
		return;

	/* Allwinner's blob uses 10, 20, and 30μs delays, depending on
	 * the iteration. However, the same code works fine in ATF with
	 * no delays. The 10μs delay is here just to be extra safe. */
	for (uint32_t i = 0; i < length; ++i) {
		mmio_write_32(addr, values[i]);
		udelay(10);
	}
}
