/*
 * Copyright © 2017-2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <counter.h>
#include <debug.h>
#include <timeout.h>
#include <platform/time.h>

bool
timeout_expired(uint32_t timeout)
{
	uint32_t now = cycle_counter_read();

	/* If the timeout wrapped, wait until the counter also wraps. */
	return (now ^ timeout) >> 31 == 0 && now >= timeout;
}

uint32_t
timeout_set(uint32_t useconds)
{
	uint32_t cycles = CPUCLK_MHz * useconds;
	uint32_t now    = cycle_counter_read();

	/* Ensure the MSB is zero for the wraparound check above. */
	assert(cycles >> 31 == 0);

	return now + cycles;
}
