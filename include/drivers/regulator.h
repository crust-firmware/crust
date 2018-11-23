/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_REGULATOR_H
#define DRIVERS_REGULATOR_H

#include <dm.h>
#include <intrusive.h>
#include <stdbool.h>
#include <stdint.h>
#include <util.h>

#define REGULATOR_OPS(dev) \
	(&container_of((dev)->drv, struct regulator_driver, drv)->ops)

enum {
	REGL_READABLE  = BIT(0), /**< Regulator is readable via SCPI. */
	REGL_WRITABLE  = BIT(1), /**< Regulator is writable via SCPI. */
	REGL_CRITICAL  = BIT(4), /**< Regulator cannot be disabled. */
	REGL_SCPI_MASK = REGL_READABLE | REGL_WRITABLE,
};

struct regulator_range {
	const uint16_t start_raw;   /**< Smallest raw value in the range. */
	const int16_t  start_value; /**< Cooked value at smallest raw value. */
	const uint16_t step;        /**< Distance between adjacent values. */
};

struct regulator_info {
	const char *const            name;      /**< Name exported to SCPI. */
	const uint16_t               min_value; /**< Minimum allowed value. */
	const uint16_t               max_value; /**< Maximum allowed value. */
	const struct regulator_range ranges[2]; /**< Range descriptions. */
	const uint8_t                flags;     /**< Generic class flags. */
};

struct regulator_driver_ops {
	struct regulator_info *(*get_info)(struct device *dev, uint8_t id);
	int                    (*get_state)(struct device *dev, uint8_t id);
	int                    (*read_raw)(struct device *dev, uint8_t id,
	                                   uint32_t *raw);
	int                    (*set_state)(struct device *dev, uint8_t id,
	                                    bool enable);
	int                    (*write_raw)(struct device *dev, uint8_t id,
	                                    uint32_t raw);
};

struct regulator_driver {
	const struct driver               drv;
	const struct regulator_driver_ops ops;
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
int regulator_disable(struct device *dev, uint8_t id);

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
static inline int
regulator_enable(struct device *dev, uint8_t id)
{
	return REGULATOR_OPS(dev)->set_state(dev, id, true);
}

/**
 * Get generic information about a regulator.
 *
 * This function has no defined errors.
 *
 * @param dev   The device containing this regulator.
 * @param id    The device-specific identifier for this regulator.
 * @return      A pointer to the information structure.
 */
static inline struct regulator_info *
regulator_get_info(struct device *dev, uint8_t id)
{
	return REGULATOR_OPS(dev)->get_info(dev, id);
}

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
static inline int
regulator_get_state(struct device *dev, uint8_t id)
{
	return REGULATOR_OPS(dev)->get_state(dev, id);
}

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
int regulator_get_value(struct device *dev, uint8_t id, uint16_t *value);

/**
 * Set all regulators controlled by a device to their default values.
 *
 * This function may fail with:
 *   EIO    There was a problem communicating with the hardware.
 *   ERANGE A requested value is below the minimum or above the maximum
 *          allowed value for the regulator.
 *
 * @param dev    A device containing one or more regulators.
 * @param values An array of values, one for each regulator in the device.
 */
int regulator_set_defaults(struct device *dev, uint16_t *values);

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
int regulator_set_value(struct device *dev, uint8_t id, uint16_t value);

#endif /* DRIVERS_REGULATOR_H */
