/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_IRQ_SUN4I_INTC_H
#define DRIVERS_IRQ_SUN4I_INTC_H

#include <irq.h>
#include <platform/irq.h>

extern struct irq_device r_intc;

void sun4i_intc_irq(struct device *dev);

#endif /* DRIVERS_IRQ_SUN4I_INTC_H */
