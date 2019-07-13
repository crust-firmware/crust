/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_IRQ_SUNXI_GPIO_H
#define DRIVERS_IRQ_SUNXI_GPIO_H

#include <irq.h>

#define SUNXI_GPIO_IRQ(port, index) (32 * (port) + (index))

enum {
	SUNXI_GPIO_IRQ_MODE_RISING_EDGE  = 0,
	SUNXI_GPIO_IRQ_MODE_FALLING_EDGE = 1,
	SUNXI_GPIO_IRQ_MODE_HIGH_LEVEL   = 2,
	SUNXI_GPIO_IRQ_MODE_LOW_LEVEL    = 3,
	SUNXI_GPIO_IRQ_MODE_EITHER_EDGE  = 4,
};

struct sunxi_gpio_irqchip {
	struct device       dev;
	struct clock_handle clock;
	struct irq_handle   irq;
	struct irq_handle  *list;
};

extern struct sunxi_gpio_irqchip r_pio_irqchip;

#endif /* DRIVERS_IRQ_SUNXI_GPIO_H */
