/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <debug.h>
#include <dm.h>
#include <error.h>
#include <sensor.h>
#include <stdbool.h>
#include <stdint.h>

#define SENSOR_TEMP_MAX      100000
#define SENSOR_TEMP_MIN_DVFS 90000

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

void
sensor_poll_temp(void *param)
{
	struct device *dev = param;
	uint32_t val       = UINT32_MAX;

	/* Read sensor values and verify values do not pass trip points. */
	for (size_t i = 0; i < dev->subdev_count; ++i) {
		if (sensor_get_value(dev, i, &val))
			error("Unable to read value of sensor with ID %u", i);

		if (val > SENSOR_TEMP_MAX)
			panic("Maximum, safe device temperature reached");
		else if (val > SENSOR_TEMP_MIN_DVFS)
			panic("Min DVFS not implemented.");
	}
}
