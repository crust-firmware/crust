/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <dm.h>
#include <util.h>
#include <drivers/clock/sunxi-ccu.h>
#include <drivers/irqchip/sun4i-intc.h>
#include <drivers/msgbox/sunxi-msgbox.h>
#include <drivers/pio/sunxi-pio.h>
#include <drivers/timer/sun8i-r_timer.h>
#include <drivers/wallclock/sun6i-cnt64.h>
#include <platform/ccu.h>
#include <platform/devices.h>
#include <platform/irq.h>
#include <platform/r_ccu.h>

static struct device ccu      __device;
static struct device msgbox   __device;
static struct device pio      __device;
static struct device r_ccu    __device;
static struct device r_cnt64  __device;
static struct device r_intc   __device;
static struct device r_pio    __device;
static struct device r_timer0 __device;

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

static struct device pio = {
	.name     = "pio",
	.regs     = DEV_PIO,
	.clock    = CCU_GATE(CCU_GATE_PIO) | CCU_RESET(CCU_RESET_PIO),
	.clockdev = &ccu,
	.drv      = &sunxi_pio_driver,
	.drvdata  = BITMASK(1, 7), /*< Physically implemented ports (1-7). */
};

static struct device r_ccu = {
	.name = "r_ccu",
	.regs = DEV_R_PRCM,
	.drv  = &sunxi_ccu_driver,
};

static struct device r_cnt64 = {
	.name = "r_cnt64",
	.regs = DEV_R_CPUCFG,
	.drv  = &sun6i_cnt64_driver,
};

static struct device r_intc = {
	.name    = "r_intc",
	.regs    = DEV_R_INTC,
	.drv     = &sun4i_intc_driver,
	.drvdata = SUN4I_INTC_DRVDATA { { 0 } },
};

static struct device r_pio = {
	.name     = "r_pio",
	.regs     = DEV_R_PIO,
	.clock    = CCU_GATE(R_CCU_GATE_R_PIO),
	.clockdev = &r_ccu,
	.drv      = &sunxi_pio_driver,
	.drvdata  = BIT(0), /*< Physically implemented ports (0). */
};

static struct device r_timer0 = {
	.name  = "r_timer0",
	.regs  = DEV_R_TIMER,
	.clock = CCU_GATE(R_CCU_GATE_R_TIMER) |
	         CCU_RESET(R_CCU_GATE_R_TIMER),
	.clockdev = &r_ccu,
	.drv      = &sun8i_r_timer_driver,
	.drvdata  = 0, /*< Timer index within the device. */
	.irq      = IRQ_R_TIMER0,
	.irqdev   = &r_intc,
};
