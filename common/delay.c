/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <delay.h>
#include <stdint.h>
#include <drivers/wallclock.h>

void
delay_cycles(uint32_t cycles)
{
	uint64_t start = wallclock_read();

	/* If no wallclock driver is loaded, the read value won't change. */
	if (start == 0)
		return;

	while (wallclock_read() < start + cycles) {
		/* Wait for time to pass. */
	}
}
