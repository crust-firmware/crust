/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_CLOCK_H
#define DRIVERS_CLOCK_H

#include <device.h>
#include <stdbool.h>
#include <stdint.h>
#include <util.h>

enum {
	CLK_READABLE  = BIT(0), /**< Clock is readable via SCPI. */
	CLK_WRITABLE  = BIT(1), /**< Clock is writable via SCPI. */
	CLK_CRITICAL  = BIT(4), /**< Clock cannot be disabled. */
	CLK_FIXED     = BIT(5), /**< Clock rate cannot be changed. */
	CLK_SCPI_MASK = CLK_READABLE | CLK_WRITABLE,
};

struct clock_handle {
	const struct device *dev;  /**< The device containing this clock. */
	const uint8_t        id;   /**< The per-device clock identifier. */
	const uint8_t        vdiv; /**< Optional variable post-divider. */
};

struct clock_info {
	const char *const name;     /**< Clock name (exported via SCPI). */
	const uint32_t    min_rate; /**< Minimum allowed rate in Hz. */
	const uint32_t    max_rate; /**< Maximum allowed rate in Hz. */
	const uint8_t     flags;    /**< Flags from the clock class. */
};

/**
 * Disable a clock.
 *
 * If the clock does not have a gate, this may have no effect on the hardware.
 * If this clock is the last active user of its parent clock, that parent clock
 * will also be disabled.
 *
 * This function may fail with:
 *   EIO    There was a problem communicating with the hardware.
 *   EPERM  The clock is in use or is critical and cannot be disabled.
 *
 * @param clock A reference to a clock.
 * @return      Zero on success; an error code on failure.
 */
int clock_disable(const struct clock_handle *clock);

/**
 * Enable a clock.
 *
 * If the clock does not have a gate, this may have no effect on the hardware.
 * The clock's parent, if any, will also be enabled.
 *
 * This function may fail with:
 *   EIO    There was a problem communicating with the hardware.
 *
 * @param clock A reference to a clock.
 * @return      Zero on success; an error code on failure.
 */
int clock_enable(const struct clock_handle *clock);

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
 * This function may fail with:
 *   EIO    There was a problem communicating with the hardware.
 *
 * @param clock A reference to a clock.
 * @param rate  The location to store the calculated clock rate.
 * @return      Zero on success; an error code on failure.
 */
int clock_get_rate(const struct clock_handle *clock, uint32_t *rate);

/**
 * Get the current state of a clock, as determined from the hardware.
 *
 * This function may fail with:
 *   EIO    There was a problem communicating with the hardware.
 *
 * @param clock A reference to a clock.
 * @param state The location to store the calculated clock state.
 * @return      Zero on success; an error code on failure.
 */
int clock_get_state(const struct clock_handle *clock, bool *state);

#endif /* DRIVERS_CLOCK_H */
