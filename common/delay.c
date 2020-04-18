/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <counter.h>
#include <debug.h>
#include <delay.h>
#include <limits.h>
#include <stdint.h>
#include <platform/time.h>

/**
 * Spin (do nothing) for at least the given number of reference clock cycles.
 *
 * @param cycles The number of cycles to delay for.
 */
static void
delay_cycles(uint32_t cycles)
{
	uint64_t start = counter_read();

	while (counter_read() < start + cycles) {
		/* Wait for time to pass. */
	}
}

void
udelay(uint32_t useconds)
{
	assert(useconds < UINT32_MAX / REFCLK_MHZ);

	delay_cycles(REFCLK_MHZ * useconds);
}
