/*
 * Copyright © 2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <counter.h>
#include <debug.h>
#include <regmap.h>
#include <stddef.h>
#include <stdint.h>
#include <system.h>
#include <util.h>
#include <mfd/axp20x.h>
#include <platform/time.h>

#define DELAY (30 * REFCLK_HZ)

static uint64_t last_tick;

void
debug_print_battery(void)
{
	const struct regmap *map = &axp20x.map;
	uint64_t now = counter_read();
	int32_t  current, voltage;
	uint8_t  hi, lo, val;

	if (system_is_running())
		return;

	if (now - last_tick > DELAY) {
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
		voltage = (((hi << 4) | (lo & 0xf)) * 1100 + 500) / 1000;

		if (regmap_read(map, 0x7c, &hi))
			goto err_put_mfd;
		if (regmap_read(map, 0x7d, &lo))
			goto err_put_mfd;
		current = (hi << 4) | (lo & 0xf);

		info("Using %d mW (%d mA @ %d mV)",
		     (current * voltage + 500) / 1000, current, voltage);

err_put_mfd:
		regmap_user_release(map);
		last_tick = now;
	}
}
