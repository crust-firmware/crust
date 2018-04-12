/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
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

struct sensor_info {
	const char *const name;       /**< Name exported to SCPI. */
	const int32_t     offset;     /**< Additive factor in millicelsius.*/
	const int32_t     multiplier; /**< Multiplicative factor. */
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

/**
 * Polls all sensors belonging to a device and verifies that temperature is
 * within defined range. Enables DVFS or panics if the device temperature is
 * outside of the safe operating range.
 *
 * @param param A void pointer to the device containing the sensor to use for
 *              periodic readings.
 */
void sensor_poll_temp(void *param);

#endif /* DRIVERS_SENSOR_H */
