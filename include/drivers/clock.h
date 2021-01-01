/*
 * Copyright © 2017-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_CLOCK_H
#define DRIVERS_CLOCK_H

#include <device.h>
#include <stdint.h>

enum {
	CLOCK_STATE_DISABLED,
	CLOCK_STATE_GATED,
	CLOCK_STATE_ENABLED,
};

struct clock_handle {
	const struct device *dev; /**< The clock controller device. */
	uint8_t              id;  /**< The device-specific clock identifier. */
};

/**
 * Disable a clock.
 *
 * If the clock does not have a gate, this may have no effect on the hardware.
 * The clock's parent, if any, will be left as it is.
 *
 * @param clock A reference to a clock.
 */
void clock_disable(const struct clock_handle *clock);

/**
 * Enable a clock.
 *
 * If the clock does not have a gate, this may have no effect on the hardware.
 * The clock's parent, if any, will also be enabled.
 *
 * @param clock A reference to a clock.
 */
void clock_enable(const struct clock_handle *clock);

/**
 * Get a reference to a clock and its controller device, and enable the clock.
 *
 * If the clock does not have a gate, this may have no effect on the hardware.
 * The clock's parent, if any, will also be enabled.
 *
 * This function may fail with:
 *   EIO    There was a problem communicating with the hardware.
 *
 * @param clock A handle specifying the clock.
 * @return      A reference to the clock that was acquired.
 */
int clock_get(const struct clock_handle *clock);

/**
 * Get the current rate of a clock, as calculated from the hardware.
 *
 * This function returns the frequency the clock runs at when ungated,
 * regardless of if the clock is currently gated.
 *
 * @param clock A reference to a clock.
 * @return      The clock frequency in Hz on success; zero on failure.
 */
uint32_t clock_get_rate(const struct clock_handle *clock);

/**
 * Get the current state of a clock, as determined from the hardware.
 *
 * This function may fail with:
 *   EIO    There was a problem communicating with the hardware.
 *
 * @param clock A reference to a clock.
 * @return      One of the enumerated clock states.
 */
uint32_t clock_get_state(const struct clock_handle *clock);

/**
 * Release a reference to a clock and its controller device.
 *
 * If this is the last reference to a clock, that clock (and its ancestors
 * recursively, if they have no other consumers) will be disabled.
 *
 * @param clock A reference to a clock.
 */
void clock_put(const struct clock_handle *clock);

#endif /* DRIVERS_CLOCK_H */
