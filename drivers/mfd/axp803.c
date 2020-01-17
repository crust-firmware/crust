/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <device.h>
#include <error.h>
#include <regmap.h>
#include <stdbool.h>
#include <stdint.h>
#include <mfd/axp803.h>

#define IC_TYPE_REG   0x03
#define IC_TYPE_MASK  0xcf
#define IC_TYPE_VALUE 0x41

static uint8_t refcount;

int
axp803_get(const struct regmap *map)
{
	uint8_t reg;
	int err;

	if (!refcount) {
		if ((err = regmap_get(map)))
			return err;
		if ((err = regmap_read(map, IC_TYPE_REG, &reg)))
			goto err_put_regmap;
		if ((reg & IC_TYPE_MASK) != IC_TYPE_VALUE) {
			err = ENODEV;
			goto err_put_regmap;
		}
	}

	++refcount;

	return SUCCESS;

err_put_regmap:
	regmap_put(map);

	return err;
}

void
axp803_put(const struct regmap *map)
{
	if (--refcount)
		return;

	regmap_put(map);
}
