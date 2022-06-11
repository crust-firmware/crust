/*
 * Copyright © 2020-2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <counter.h>
#include <debug.h>
#include <division.h>
#include <system.h>

#define ITERATIONS 10000

static uint32_t cycles;
static uint32_t iterations;
static uint8_t  last_state;

void
debug_print_latency(uint8_t current_state)
{
	if (current_state != last_state) {
		cycles     = counter_read();
		iterations = 0;
		last_state = current_state;
	} else if (iterations < ITERATIONS && ++iterations == ITERATIONS) {
		info("State %u: %u cycles/iteration", current_state,
		     udiv_round(counter_read() - cycles, ITERATIONS));
	}
}
