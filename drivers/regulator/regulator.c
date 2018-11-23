/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <dm.h>
#include <error.h>
#include <regulator.h>
#include <stdbool.h>
#include <stdint.h>

int
regulator_disable(struct device *dev, uint8_t id)
{
	const struct regulator_driver_ops *ops = REGULATOR_OPS(dev);
	struct regulator_info *info = ops->get_info(dev, id);

	if (info->flags & REGL_CRITICAL)
		return EPERM;

	return ops->set_state(dev, id, false);
}

int
regulator_get_value(struct device *dev, uint8_t id, uint16_t *value)
{
	const struct regulator_driver_ops *ops = REGULATOR_OPS(dev);
	struct regulator_info *info = ops->get_info(dev, id);
	const struct regulator_range *range;
	int err;
	uint32_t raw;

	if ((err = ops->read_raw(dev, id, &raw)))
		return err;

	/* If the second range is defined, decide which range to use. */
	if (info->ranges[1].step != 0 && raw >= info->ranges[1].start_raw)
		range = &info->ranges[1];
	else
		range = &info->ranges[0];

	/* Calculate the cooked value from the raw value. */
	*value = range->start_value + (raw - range->start_raw) * range->step;

	return SUCCESS;
}

int
regulator_set_defaults(struct device *dev, uint16_t *values)
{
	int err;

	for (uint8_t id = 0; id < dev->subdev_count; ++id) {
		if (values[id] > 0) {
			if ((err = regulator_set_value(dev, id, values[id])))
				return err;
			if ((err = regulator_enable(dev, id)))
				return err;
		} else {
			if ((err = regulator_disable(dev, id)))
				return err;
		}
	}

	return SUCCESS;
}

int
regulator_set_value(struct device *dev, uint8_t id, uint16_t value)
{
	const struct regulator_driver_ops *ops = REGULATOR_OPS(dev);
	struct regulator_info *info = ops->get_info(dev, id);
	const struct regulator_range *range;
	uint32_t raw;

	if (value < info->min_value || value > info->max_value)
		return ERANGE;

	/* If the second range is defined, decide which range to use. */
	if (info->ranges[1].step != 0 && value >= info->ranges[1].start_value)
		range = &info->ranges[1];
	else
		range = &info->ranges[0];

	/* Calculate the raw value, being careful to round correctly. */
	raw  = (value - range->start_value + range->step / 2) / range->step;
	raw += range->start_raw;

	return ops->write_raw(dev, id, raw);
}
