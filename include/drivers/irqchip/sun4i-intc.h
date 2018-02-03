/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef DRIVERS_IRQCHIP_SUN4I_INTC_H
#define DRIVERS_IRQCHIP_SUN4I_INTC_H

#include <drivers/irqchip.h>

#define SUN4I_INTC_IRQS 32

#define SUN4I_INTC_ALLOC_DRVDATA(id) \
	static struct irq_vector sun4i_intc_ ## id ## _vectors[SUN4I_INTC_IRQS]

#define SUN4I_INTC_DRVDATA(id) \
	((uintptr_t)&sun4i_intc_ ## id ## _vectors)

extern const struct driver sun4i_intc_driver;

#endif /* DRIVERS_IRQCHIP_SUN4I_INTC_H */
