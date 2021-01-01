/*
 * Copyright © 2020-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <gpio/sunxi-gpio.h>
#include <regulator/gpio.h>

#include "regulator.h"

static inline const struct gpio_regulator *
to_gpio_regulator(const struct device *dev)
{
	return container_of(dev, const struct gpio_regulator, dev);
}

static int
gpio_regulator_get_state(const struct regulator_handle *handle, bool *enabled)
{
	const struct gpio_regulator *self = to_gpio_regulator(handle->dev);

	return gpio_get_value(&self->pin, enabled);
}

static int
gpio_regulator_set_state(const struct regulator_handle *handle, bool enabled)
{
	const struct gpio_regulator *self = to_gpio_regulator(handle->dev);

	return gpio_set_value(&self->pin, enabled);
}

static int
gpio_regulator_probe(const struct device *dev)
{
	const struct gpio_regulator *self = to_gpio_regulator(dev);

	return gpio_get(&self->pin);
}

static void
gpio_regulator_release(const struct device *dev)
{
	const struct gpio_regulator *self = to_gpio_regulator(dev);

	gpio_put(&self->pin);
}

static const struct regulator_driver gpio_regulator_driver = {
	.drv = {
		.probe   = gpio_regulator_probe,
		.release = gpio_regulator_release,
	},
	.ops = {
		.get_state = gpio_regulator_get_state,
		.set_state = gpio_regulator_set_state,
	},
};

#if CONFIG(REGULATOR_GPIO_CPU)
const struct gpio_regulator gpio_cpu_regulator = {
	.dev = {
		.name  = "gpio-cpu-regulator",
		.drv   = &gpio_regulator_driver.drv,
		.state = DEVICE_STATE_INIT,
	},
	.pin = {
		.dev   = &r_pio.dev,
		.id    = CONFIG_REGULATOR_GPIO_CPU_PIN,
		.drive = DRIVE_10mA,
		.mode  = MODE_OUTPUT,
		.pull  = PULL_NONE,
	},
};
#endif

#if CONFIG(REGULATOR_GPIO_DRAM)
const struct gpio_regulator gpio_dram_regulator = {
	.dev = {
		.name  = "gpio-dram-regulator",
		.drv   = &gpio_regulator_driver.drv,
		.state = DEVICE_STATE_INIT,
	},
	.pin = {
		.dev   = &r_pio.dev,
		.id    = CONFIG_REGULATOR_GPIO_DRAM_PIN,
		.drive = DRIVE_10mA,
		.mode  = MODE_OUTPUT,
		.pull  = PULL_NONE,
	},
};
#endif
