/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_SENSOR_H
#define DRIVERS_SENSOR_H

#include <dm.h>
#include <intrusive.h>
#include <stdbool.h>
#include <stdint.h>
#include <util.h>

#define SENSOR_OPS(dev) \
	(&container_of((dev)->drv, struct sensor_driver, drv)->ops)

enum {
	SENSOR_CLASS_TEMPERATURE,
	SENSOR_CLASS_VOLTAGE,
	SENSOR_CLASS_CURRENT,
	SENSOR_CLASS_POWER,
	SENSOR_CLASS_ENERGY,
};

enum {
	SENSOR_PERIODIC  = BIT(0),
	SENSOR_BOUNDS    = BIT(1),
	SENSOR_SCPI_MASK = SENSOR_PERIODIC | SENSOR_BOUNDS,
};

struct sensor_info {
	const char *const name;       /**< Name exported to SCPI. */
	const int32_t     offset;     /**< Additive factor in millicelsius.*/
	const int32_t     multiplier; /**< Multiplicative factor. */
	const uint8_t     class;      /**< Sensor class (measured unit). */
	const uint8_t     flags;      /**< Sensor trigger flags. */
};

struct sensor_driver_ops {
	struct sensor_info *(*get_info)(struct device *dev, uint8_t id);
	int                 (*read_raw)(struct device *dev, uint8_t id,
	                                uint32_t *raw);
};

struct sensor_driver {
	const struct driver            drv;
	const struct sensor_driver_ops ops;
};

/**
 * Get generic information about a sensor.
 *
 * This function has no defined errors.
 *
 * @param dev   The device containing this sensor.
 * @param id    The device-specific identifier for this sensor.
 * @return      A pointer to the information structure.
 */
static inline struct sensor_info *
sensor_get_info(struct device *dev, uint8_t id)
{
	return SENSOR_OPS(dev)->get_info(dev, id);
}

/**
 * Get the current reading from a sensor.
 *
 * This function may fail with:
 *   EIO    There was a problem communicating with the hardware.
 *
 * @param dev   The device containing this sensor.
 * @param id    The device-specific identifier for this sensor.
 * @param value The location to store the value read from the sensor.
 * @return      Zero on success; a defined error code on failure.
 */
int sensor_get_value(struct device *dev, uint8_t id, uint32_t *value);

#endif /* DRIVERS_SENSOR_H */
