/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_IRQCHIP_SUNXI_GPIO_H
#define DRIVERS_IRQCHIP_SUNXI_GPIO_H

#include <irqchip.h>

#define SUNXI_GPIO_IRQ(port, index) (32 * (port) + (index))

enum {
	SUNXI_GPIO_IRQ_MODE_RISING_EDGE  = 0,
	SUNXI_GPIO_IRQ_MODE_FALLING_EDGE = 1,
	SUNXI_GPIO_IRQ_MODE_HIGH_LEVEL   = 2,
	SUNXI_GPIO_IRQ_MODE_LOW_LEVEL    = 3,
	SUNXI_GPIO_IRQ_MODE_EITHER_EDGE  = 4,
};

extern const struct irqchip_driver sunxi_gpio_irqchip_driver;

#endif /* DRIVERS_IRQCHIP_SUNXI_GPIO_H */
