/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <device.h>
#include <error.h>
#include <regulator.h>
#include <stdbool.h>
#include <stdint.h>

#include "regulator.h"

/**
 * Get the ops for the regulator controller device.
 */
static inline const struct regulator_driver_ops *
regulator_ops_for(const struct device *dev)
{
	const struct regulator_driver *drv =
		container_of(dev->drv, const struct regulator_driver, drv);

	return &drv->ops;
}

int
regulator_disable(const struct device *dev, uint8_t id)
{
	const struct regulator_driver_ops *ops = regulator_ops_for(dev);
	struct regulator_info *info = ops->get_info(dev, id);

	if (info->flags & REGL_CRITICAL)
		return EPERM;

	return ops->set_state(dev, id, false);
}

int
regulator_enable(const struct device *dev, uint8_t id)
{
	return regulator_ops_for(dev)->set_state(dev, id, true);
}

struct regulator_info *
regulator_get_info(const struct device *dev, uint8_t id)
{
	return regulator_ops_for(dev)->get_info(dev, id);
}

int
regulator_get_state(const struct device *dev, uint8_t id)
{
	return regulator_ops_for(dev)->get_state(dev, id);
}

int
regulator_get_value(const struct device *dev, uint8_t id, uint16_t *value)
{
	const struct regulator_driver_ops *ops = regulator_ops_for(dev);
	struct regulator_info *info = ops->get_info(dev, id);
	const struct regulator_range *range;
	uint32_t raw;
	int err;

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
regulator_set_value(const struct device *dev, uint8_t id, uint16_t value)
{
	const struct regulator_driver_ops *ops = regulator_ops_for(dev);
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
