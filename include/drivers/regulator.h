/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_REGULATOR_H
#define DRIVERS_REGULATOR_H

#include <device.h>
#include <intrusive.h>
#include <stdbool.h>
#include <stdint.h>
#include <util.h>

enum {
	REGL_READABLE  = BIT(0), /**< Regulator is readable via SCPI. */
	REGL_WRITABLE  = BIT(1), /**< Regulator is writable via SCPI. */
	REGL_CRITICAL  = BIT(4), /**< Regulator cannot be disabled. */
	REGL_SCPI_MASK = REGL_READABLE | REGL_WRITABLE,
};

struct regulator_handle {
	const struct device *dev; /**< The device containing this regulator. */
	uint8_t              id;  /**< The per-device regulator identifier. */
};

struct regulator_range {
	uint16_t start_raw;   /**< Smallest raw value in the range. */
	int16_t  start_value; /**< Cooked value at smallest raw value. */
	uint16_t step;        /**< Distance between adjacent values. */
};

struct regulator_info {
	const char            *name;      /**< Name exported to SCPI. */
	uint16_t               min_value; /**< Minimum allowed value. */
	uint16_t               max_value; /**< Maximum allowed value. */
	struct regulator_range ranges[2]; /**< Range descriptions. */
	uint8_t                flags;     /**< Generic class flags. */
};

/**
 * Disable the output of a regulator. If the regulator does not have output
 * on/off control, this function may have no effect on the hardware.
 *
 * This function may fail with:
 *   EIO    There was a problem communicating with the hardware.
 *   EPERM  The regulator is critical and cannot be disabled.
 *
 * @param dev   The device containing this regulator.
 * @param id    The device-specific identifier for this regulator.
 * @return      Zero on success; a defined error code on failure.
 */
int regulator_disable(const struct device *dev, uint8_t id);

/**
 * Enable the output of a regulator. If the regulator does not have output
 * on/off control, this function may have no effect on the hardware.
 *
 * This function may fail with:
 *   EIO    There was a problem communicating with the hardware.
 *
 * @param dev   The device containing this regulator.
 * @param id    The device-specific identifier for this regulator.
 * @return      Zero on success; a defined error code on failure.
 */
int regulator_enable(const struct device *dev, uint8_t id);

/**
 * Get generic information about a regulator.
 *
 * This function has no defined errors.
 *
 * @param dev   The device containing this regulator.
 * @param id    The device-specific identifier for this regulator.
 * @return      A pointer to the information structure.
 */
struct regulator_info *regulator_get_info(const struct device *dev,
                                          uint8_t id);

/**
 * Get the current state of a regulator, as determined from the hardware.
 *
 * This function may fail with:
 *   EIO    There was a problem communicating with the hardware.
 *
 * @param dev   The device containing this regulator.
 * @param id    The device-specific identifier for this regulator.
 * @return      On success, boolean true or false for if the regulator is
 *              enabled; a defined error code on failure.
 */
int regulator_get_state(const struct device *dev, uint8_t id);

/**
 * Get the current value of a regulator. If the regulator is disabled, this
 * function returns the value the regulator would have if it was enabled. If
 * the value is unknown, this function returns zero for the value
 *
 * This function may fail with:
 *   EIO    There was a problem communicating with the hardware.
 *
 * @param dev   The device containing this regulator.
 * @param id    The device-specific identifier for this regulator.
 * @param value The location to store the value read from the regulator.
 * @return      Zero on success; a defined error code on failure.
 */
int regulator_get_value(const struct device *dev, uint8_t id, uint16_t *value);

/**
 * Set the value of a regulator. If the regulator is currently disabled, this
 * will update the value the regulator would have if it was enabled.
 *
 * This function may fail with:
 *   EIO    There was a problem communicating with the hardware.
 *   ERANGE The requested value is below the minimum or above the maximum
 *          allowed value for this regulator.
 *
 * @param dev   The device containing this regulator.
 * @param id    The device-specific identifier for this regulator.
 * @param value The new value for this regulator.
 * @return      Zero on success; a defined error code on failure.
 */
int regulator_set_value(const struct device *dev, uint8_t id, uint16_t value);

#endif /* DRIVERS_REGULATOR_H */
