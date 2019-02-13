/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <compiler.h>
#include <dm.h>
#include <system_power.h>
#include <gpio/sunxi-gpio.h>
#include <irqchip/sunxi-gpio.h>
#include <misc/gpio-button.h>
#include <platform/devices.h>

static void
gpio_button_irq(void *param __unused)
{
	system_wakeup();
}

static int
gpio_button_probe(struct device *dev)
{
	int err;

	if ((err = dm_setup_pins(dev, 1)))
		return err;

	return dm_setup_irq(dev, gpio_button_irq);
}

static const struct driver gpio_button_driver = {
	.class = DM_CLASS_NONE,
	.probe = gpio_button_probe,
};

struct device power_button __device = {
	.name = "power-button",
	.drv  = &gpio_button_driver,
	.irq  = IRQ_HANDLE {
		.dev  = &r_pio_irqchip,
		.irq  = SUNXI_GPIO_IRQ(0, CONFIG_GPIO_BUTTON_PIN),
		.mode = SUNXI_GPIO_IRQ_MODE_FALLING_EDGE,
	},
	.pins = GPIO_PINS(1) {
		{ &r_pio, SUNXI_GPIO_PIN(0, CONFIG_GPIO_BUTTON_PIN), 6 },
	},
};
