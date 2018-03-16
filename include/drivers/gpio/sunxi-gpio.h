/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef DRIVERS_GPIO_SUNXI_GPIO_H
#define DRIVERS_GPIO_SUNXI_GPIO_H

#include <gpio.h>

#define SUNXI_GPIO_PIN(port, index) ((port) << 5 | ((index) & BITMASK(0, 5)))

extern const struct gpio_driver sunxi_gpio_driver;

#endif /* DRIVERS_GPIO_SUNXI_GPIO_H */
