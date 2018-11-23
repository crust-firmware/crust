/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_IRQCHIP_SUN4I_INTC_H
#define DRIVERS_IRQCHIP_SUN4I_INTC_H

#include <irqchip.h>

extern const struct irqchip_driver sun4i_intc_driver;

void sun4i_intc_irq(struct device *dev);

#endif /* DRIVERS_IRQCHIP_SUN4I_INTC_H */
