/*
 * Copyright © 2017-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef GPIO_PRIVATE_H
#define GPIO_PRIVATE_H

#include <device.h>
#include <gpio.h>
#include <stdbool.h>

struct gpio_driver_ops {
	int (*get_value)(const struct gpio_handle *gpio, bool *value);
	int (*init_pin)(const struct gpio_handle *gpio);
	int (*set_value)(const struct gpio_handle *gpio, bool value);
};

struct gpio_driver {
	struct driver          drv;
	struct gpio_driver_ops ops;
};

#endif /* GPIO_PRIVATE_H */
