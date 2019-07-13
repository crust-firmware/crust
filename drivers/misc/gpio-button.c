/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <compiler.h>
#include <dm.h>
#include <intrusive.h>
#include <system_power.h>
#include <gpio/sunxi-gpio.h>
#include <irq/sunxi-gpio.h>
#include <misc/gpio-button.h>
#include <platform/devices.h>

static bool
gpio_button_irq(const struct irq_handle *irq __unused)
{
	system_wakeup();

	return true;
}

static int
gpio_button_probe(struct device *dev)
{
	int err;

	if ((err = dm_setup_pins(dev, 1)))
		return err;

	return irq_get(&container_of(dev, struct gpio_button, dev)->irq);
}

static const struct driver gpio_button_driver = {
	.class = DM_CLASS_NONE,
	.probe = gpio_button_probe,
};

struct gpio_button power_button = {
	.dev = {
		.name = "power-button",
		.drv  = &gpio_button_driver,
		.pins = GPIO_PINS(1) {
			{ &r_pio,
			  SUNXI_GPIO_PIN(0, CONFIG_GPIO_BUTTON_PIN), 6 },
		},
	},
	.irq = {
		.dev     = &r_pio_irqchip.dev,
		.irq     = SUNXI_GPIO_IRQ(0, CONFIG_GPIO_BUTTON_PIN),
		.mode    = SUNXI_GPIO_IRQ_MODE_FALLING_EDGE,
		.handler = gpio_button_irq,
	},
};
