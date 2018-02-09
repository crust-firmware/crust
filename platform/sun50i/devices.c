/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <dm.h>
#include <drivers/clock/sunxi-ccu.h>
#include <drivers/irqchip/sun4i-intc.h>
#include <drivers/msgbox/sunxi-msgbox.h>
#include <platform/ccu.h>
#include <platform/devices.h>
#include <platform/irq.h>
#include <platform/r_ccu.h>

static struct device ccu    __device;
static struct device msgbox __device;
static struct device r_ccu  __device;
static struct device r_intc __device;

static struct device ccu = {
	.name = "ccu",
	.regs = DEV_CCU,
	.drv  = &sunxi_ccu_driver,
};

static struct device msgbox = {
	.name     = "msgbox",
	.regs     = DEV_MSGBOX,
	.clock    = CCU_GATE(CCU_GATE_MSGBOX) | CCU_RESET(CCU_RESET_MSGBOX),
	.clockdev = &ccu,
	.drv      = &sunxi_msgbox_driver,
	.drvdata  = SUNXI_MSGBOX_DRVDATA { 0 },
	.irq      = IRQ_MSGBOX,
	.irqdev   = &r_intc,
};

static struct device r_ccu = {
	.name = "r_ccu",
	.regs = DEV_R_PRCM,
	.drv  = &sunxi_ccu_driver,
};

static struct device r_intc = {
	.name    = "r_intc",
	.regs    = DEV_R_INTC,
	.drv     = &sun4i_intc_driver,
	.drvdata = SUN4I_INTC_DRVDATA { { 0 } },
};
