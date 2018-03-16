/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef DRIVERS_IRQCHIP_SUN4I_INTC_H
#define DRIVERS_IRQCHIP_SUN4I_INTC_H

#include <irqchip.h>
#include <work.h>

#define SUN4I_INTC_IRQS 32

#define SUN4I_INTC_DRVDATA \
	(uintptr_t)&(struct handler[SUN4I_INTC_IRQS])

extern const struct irqchip_driver sun4i_intc_driver;

#endif /* DRIVERS_IRQCHIP_SUN4I_INTC_H */
