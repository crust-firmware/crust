/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_CLOCK_H
#define DRIVERS_CLOCK_H

#include <dm.h>
#include <intrusive.h>
#include <stdbool.h>
#include <stdint.h>
#include <util.h>

#define CLOCK_OPS(dev) \
	(&container_of((dev)->drv, struct clock_driver, drv)->ops)

#define CLOCK_PARENT(d, i) \
	& (struct clock_handle) { \
		.dev = &(d), \
		.id  = (i), \
	}

#define CLOCK_PARENTS(n) (struct clock_handle[n])

#define FIXED_CLOCK(n, r, f) \
	{ \
		.info = { \
			.name     = (n), \
			.min_rate = (r), \
			.max_rate = (r), \
			.flags    = (f) | CLK_CRITICAL | CLK_FIXED, \
		}, \
	}

enum {
	CLK_READABLE  = BIT(0), /**< Clock is readable via SCPI. */
	CLK_WRITABLE  = BIT(1), /**< Clock is writable via SCPI. */
	CLK_CRITICAL  = BIT(4), /**< Clock cannot be disabled. */
	CLK_FIXED     = BIT(5), /**< Clock rate cannot be changed. */
	CLK_SCPI_MASK = CLK_READABLE | CLK_WRITABLE,
};

struct clock_handle {
	struct device *const dev;  /**< The device containing this clock. */
	const uint8_t        id;   /**< The per-device clock identifier. */
	const uint8_t        vdiv; /**< Optional variable post-divider. */
};

struct clock_info {
	const char *const name;     /**< Clock name (exported via SCPI). */
	const uint32_t    min_rate; /**< Minimum allowed rate in Hz. */
	const uint32_t    max_rate; /**< Maximum allowed rate in Hz. */
	const uint8_t     flags;    /**< Flags from the clock class. */
	uint8_t           refcount; /**< Number of references to this clock. */
};

struct clock_driver_ops {
	struct clock_info   *(*get_info)(struct device *dev, uint8_t id);
	struct clock_handle *(*get_parent)(struct device *dev, uint8_t id);
	int                  (*get_rate)(struct device *dev, uint8_t id,
	                                 uint32_t *rate);
	int                  (*get_state)(struct device *dev, uint8_t id);
	int                  (*set_rate)(struct device *dev, uint8_t id,
	                                 uint32_t rate);
	int                  (*set_state)(struct device *dev, uint8_t id,
	                                  bool enable);
};

struct clock_driver {
	const struct driver           drv;
	const struct clock_driver_ops ops;
};

/**
 * Disable a clock. If the clock does not have a gate, this may have no effect
 * on the hardware. If this clock is the last active user of its parent clock,
 * that parent clock will also be disabled.
 *
 * This function may fail with:
 *   EIO    There was a problem communicating with the hardware.
 *   EPERM  The clock is in use or is critical and cannot be disabled.
 *
 * @param dev The clock controller containing this clock.
 * @param id  The device-specific identifier for this clock.
 * @return    Zero on success; a defined error code on failure.
 */
int clock_disable(struct device *dev, uint8_t id);

/**
 * Enable a clock. If the clock does not have a gate, this may have no effect
 * on the hardware. The clock's rate will be clamped to its minimum and maximum
 * values, if defined. The clock's parent, if any, will also be enabled.
 *
 * This function may fail with:
 *   EIO    There was a problem communicating with the hardware.
 *   ERANGE No possible set of clock factors will produce a rate that is in
 *          range (between the minimum and maximum rate).
 *
 * @param dev The clock controller containing this clock.
 * @param id  The device-specific identifier for this clock.
 * @return    Zero on success; a defined error code on failure.
 */
int clock_enable(struct device *dev, uint8_t id);

/**
 * Get generic information about a clock.
 *
 * This function has no defined errors.
 *
 * @param dev The clock controller containing this clock.
 * @param id  The device-specific identifier for this clock.
 * @return    A pointer to the information structure.
 */
static inline struct clock_info *
clock_get_info(struct device *dev, uint8_t id)
{
	return CLOCK_OPS(dev)->get_info(dev, id);
}

/**
 * Get a handle to the parent of a clock.
 *
 * This function has no defined errors.
 *
 * @param dev The clock controller containing this clock.
 * @param id  The device-specific identifier for this clock.
 * @return    A pointer to the handle; NULL if the clock has no parent.
 */
static inline struct clock_handle *
clock_get_parent(struct device *dev, uint8_t id)
{
	return CLOCK_OPS(dev)->get_parent(dev, id);
}

/**
 * Get the current rate of a clock, as calculated from the hardware.
 *
 * This function may fail with:
 *   EIO    There was a problem communicating with the hardware.
 *
 * @param dev  The clock controller containing this clock.
 * @param id   The device-specific identifier for this clock.
 * @param rate The location to store the calculated clock rate.
 * @return     Zero on success; a defined error code on failure.
 */
static inline int
clock_get_rate(struct device *dev, uint8_t id, uint32_t *rate)
{
	return CLOCK_OPS(dev)->get_rate(dev, id, rate);
}

/**
 * Get the current state of a clock, as determined from the hardware.
 *
 * This function may fail with:
 *   EIO    There was a problem communicating with the hardware.
 *
 * @param dev  The clock controller containing this clock.
 * @param id   The device-specific identifier for this clock.
 * @return     On success, boolean true or false for if the clock is enabled; a
 *             defined error code on failure.
 */
int clock_get_state(struct device *dev, uint8_t id);

/**
 * Set the rate of a clock. This function takes into account the current rates
 * of all parent clocks, as well as the minimum and maximum rates for this
 * clock.
 *
 * This function may fail with:
 *   EIO    There was a problem communicating with the hardware.
 *   EPERM  The clock is in use or has a fixed rate that cannot be set.
 *   ERANGE No possible set of clock factors will produce a rate that is in
 *          range (between the minimum and maximum rate).
 *
 * @param dev  The clock controller containing this clock.
 * @param id   The device-specific identifier for this clock.
 * @param rate The requested clock rate.
 * @return     Zero on success; a defined error code on failure.
 */
int clock_set_rate(struct device *dev, uint8_t id, uint32_t rate);

#endif /* DRIVERS_CLOCK_H */
