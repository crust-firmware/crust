/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <dm.h>
#include <error.h>
#include <sensor.h>
#include <stdbool.h>
#include <stdint.h>

int
sensor_get_value(struct device *dev, uint8_t id, uint32_t *value)
{
	const struct sensor_driver_ops *ops = SENSOR_OPS(dev);
	struct sensor_info *info = ops->get_info(dev, id);
	int err;
	uint32_t raw;

	if ((err = ops->read_raw(dev, id, &raw)))
		return err;

	/* Calculate the cooked value in millicelsius from the raw value. */
	*value = info->offset + raw * info->multiplier;

	return SUCCESS;
}
