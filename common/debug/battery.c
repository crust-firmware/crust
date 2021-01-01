/*
 * Copyright © 2020-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <division.h>
#include <regmap.h>
#include <stdint.h>
#include <timeout.h>
#include <util.h>
#include <mfd/axp20x.h>

#define MEASUREMENT_INTERVAL (30 * USEC_PER_SEC) /* 30s */

static uint32_t timeout;

void
debug_print_battery(void)
{
	const struct regmap *map = &axp20x.map;
	uint32_t current, voltage;
	uint8_t  hi, lo, val;

	if (!timeout_expired(timeout))
		return;
	if (regmap_user_probe(map))
		return;

	/* Battery present? */
	if (regmap_read(map, 0x01, &val) || !(val & BIT(5)))
		goto err_put_mfd;
	/* Battery discharging? */
	if (regmap_read(map, 0x00, &val) || (val & BIT(2)))
		goto err_put_mfd;

	if (regmap_read(map, 0x78, &hi))
		goto err_put_mfd;
	if (regmap_read(map, 0x79, &lo))
		goto err_put_mfd;
	voltage = udiv_round(((hi << 4) | (lo & 0xf)) * 1100, 1000);

	if (regmap_read(map, 0x7c, &hi))
		goto err_put_mfd;
	if (regmap_read(map, 0x7d, &lo))
		goto err_put_mfd;
	current = (hi << 4) | (lo & 0xf);

	info("Using %u mW (%u mA @ %u mV)",
	     udiv_round(current * voltage, 1000), current, voltage);

err_put_mfd:
	regmap_user_release(map);
	timeout = timeout_set(MEASUREMENT_INTERVAL);
}
