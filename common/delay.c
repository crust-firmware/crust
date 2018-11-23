/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <delay.h>
#include <limits.h>
#include <stdint.h>
#include <wallclock.h>
#include <platform/time.h>

/**
 * Spin (do nothing) for at least the given number of reference clock cycles.
 *
 * @param The number of cycles to delay for.
 */
static void
delay_cycles(uint32_t cycles)
{
	uint64_t start = wallclock_read();

	while (wallclock_read() < start + cycles) {
		/* Wait for time to pass. */
	}
}

void
udelay(uint32_t useconds)
{
	assert(useconds < UINT32_MAX / REFCLK_MHZ);

	delay_cycles(REFCLK_MHZ * useconds);
}
