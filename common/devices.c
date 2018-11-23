/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <devices.h>
#include <irqchip/sun4i-intc.h>
#include <platform/devices.h>

struct device r_intc __device;

struct device r_intc = {
	.name = "r_intc",
	.regs = DEV_R_INTC,
	.drv  = &sun4i_intc_driver.drv,
};
