/*
 * Copyright © 2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <division.h>
#include <spr.h>
#include <system.h>
#include <platform/time.h>

#define ITERATIONS 1000000

static uint32_t iterations;
static uint8_t  last_state;

void
debug_print_latency(void)
{
	uint8_t current_state = get_system_state();

	if (current_state != last_state) {
		iterations = 0;
		last_state = current_state;
		mtspr(SPR_TICK_TTMR_ADDR,
		      SPR_TICK_TTMR_MODE_CONTINUE << SPR_TICK_TTMR_MODE_LSB);
		mtspr(SPR_TICK_TTCR_ADDR, 0);
	} else if (iterations <= ITERATIONS && iterations++ == ITERATIONS) {
		info("Latency: average %d cycles/iteration in state %d",
		     udiv_round(mfspr(SPR_TICK_TTCR_ADDR), ITERATIONS),
		     current_state);
		mtspr(SPR_TICK_TTMR_ADDR, 0);
	}
}
