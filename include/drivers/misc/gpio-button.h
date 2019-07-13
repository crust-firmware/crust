/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_MISC_GPIO_BUTTON_H
#define DRIVERS_MISC_GPIO_BUTTON_H

#include <dm.h>
#include <gpio.h>
#include <irq.h>

struct gpio_button {
	struct device      dev;
	struct irq_handle  irq;
	struct gpio_handle pin;
};

extern struct gpio_button power_button;

#endif /* DRIVERS_MISC_GPIO_BUTTON_H */
