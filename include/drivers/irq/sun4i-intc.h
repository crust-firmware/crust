/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_IRQ_SUN4I_INTC_H
#define DRIVERS_IRQ_SUN4I_INTC_H

#include <dm.h>

struct sun4i_intc {
	struct device dev;
	uintptr_t     regs;
};

extern struct sun4i_intc r_intc;

#endif /* DRIVERS_IRQ_SUN4I_INTC_H */
