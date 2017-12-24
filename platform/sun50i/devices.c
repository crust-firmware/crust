/*
 * Copyright © 2017 Samuel Holland <samuel@sholland.org>
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <dm.h>
#include <drivers/clock/sunxi-ccu.h>
#include <drivers/irqchip/sun4i-intc.h>
#include <drivers/msgbox/sunxi-msgbox.h>
#include <platform/ccu.h>
#include <platform/devices.h>
#include <platform/irq.h>

static struct device ccu    __device;
static struct device msgbox __device;
static struct device r_intc __device;

static struct device ccu = {
	.name = "ccu",
	.drv  = &sunxi_ccu_driver,
	.regs = DEV_CCU,
};

SUNXI_MSGBOX_ALLOC_DRVDATA(DEV_MSGBOX);
static struct device msgbox = {
	.name     = "msgbox",
	.clock    = CCU_GATE(CCU_GATE_MSGBOX) | CCU_RESET(CCU_RESET_MSGBOX),
	.clockdev = &ccu,
	.drv      = &sunxi_msgbox_driver,
	.drvdata  = SUNXI_MSGBOX_DRVDATA(DEV_MSGBOX),
	.irq      = IRQ_MSGBOX,
	.irqdev   = &r_intc,
	.regs     = DEV_MSGBOX,
};

SUN4I_INTC_ALLOC_DRVDATA(DEV_R_INTC);
static struct device r_intc = {
	.name    = "r_intc",
	.drv     = &sun4i_intc_driver,
	.drvdata = SUN4I_INTC_DRVDATA(DEV_R_INTC),
	.regs    = DEV_R_INTC,
};
