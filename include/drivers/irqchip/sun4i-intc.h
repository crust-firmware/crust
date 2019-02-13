/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_IRQCHIP_SUN4I_INTC_H
#define DRIVERS_IRQCHIP_SUN4I_INTC_H

#include <irqchip.h>
#include <platform/irq.h>

extern struct device r_intc;

void sun4i_intc_irq(struct device *dev);

#endif /* DRIVERS_IRQCHIP_SUN4I_INTC_H */
