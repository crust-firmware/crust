/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <compiler.h>
#include <dm.h>
#include <system_power.h>
#include <misc/gpio-button.h>

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

const struct driver gpio_button_driver = {
	.class = DM_CLASS_NONE,
	.probe = gpio_button_probe,
};
