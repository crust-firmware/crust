/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <dm.h>
#include <util.h>
#include <clock/sunxi-ccu.h>
#include <irqchip/sun4i-intc.h>
#include <msgbox/sunxi-msgbox.h>
#include <pio/sunxi-pio.h>
#include <timer/sun8i-r_timer.h>
#include <watchdog/sunxi-twd.h>
#include <platform/ccu.h>
#include <platform/devices.h>
#include <platform/irq.h>
#include <platform/r_ccu.h>

static struct device ccu      __device;
static struct device msgbox   __device;
static struct device pio      __device;
static struct device r_ccu    __device;
static struct device r_intc   __device;
static struct device r_pio    __device;
static struct device r_timer0 __device;
static struct device r_twd    __device;

static struct device ccu = {
	.name    = "ccu",
	.regs    = DEV_CCU,
	.drv     = &sunxi_ccu_driver,
	.drvdata = SUNXI_CCU_DRVDATA {
		[CCU_CLOCK_MSGBOX] = {
			.gate    = CCU_GATE_MSGBOX,
			.reset   = CCU_RESET_MSGBOX,
			.parents = SUNXI_CCU_NO_PARENTS,
		},
		[CCU_CLOCK_PIO] = {
			.gate    = CCU_GATE_PIO,
			.reset   = CCU_RESET_PIO,
			.parents = SUNXI_CCU_NO_PARENTS,
		},
		[CCU_CLOCK_SENTINEL] = {
			.flags = SUNXI_CCU_FLAG_LAST,
		},
	},
};

static struct device msgbox = {
	.name     = "msgbox",
	.regs     = DEV_MSGBOX,
	.clock    = CCU_CLOCK_MSGBOX,
	.clockdev = &ccu,
	.drv      = &sunxi_msgbox_driver,
	.drvdata  = SUNXI_MSGBOX_DRVDATA { 0 },
	.irq      = IRQ_MSGBOX,
	.irqdev   = &r_intc,
};

static struct device pio = {
	.name     = "pio",
	.regs     = DEV_PIO,
	.clock    = CCU_CLOCK_PIO,
	.clockdev = &ccu,
	.drv      = &sunxi_pio_driver,
	.drvdata  = BITMASK(1, 7), /*< Physically implemented ports (1-7). */
};

static struct device r_ccu = {
	.name    = "r_ccu",
	.regs    = DEV_R_PRCM,
	.drv     = &sunxi_ccu_driver,
	.drvdata = SUNXI_CCU_DRVDATA {
		[R_CCU_CLOCK_OSC24M] = {
			.rate    = 24000000,
			.parents = SUNXI_CCU_NO_PARENTS,
		},
		[R_CCU_CLOCK_OSC32K] = {
			.rate    = 32768,
			.parents = SUNXI_CCU_NO_PARENTS,
		},
		[R_CCU_CLOCK_IOSC] = {
			.rate    = 16000000,
			.parents = SUNXI_CCU_NO_PARENTS,
		},
		[R_CCU_CLOCK_PLL_PERIPH0] = {
			.rate    = 600000000,
			.parents = SUNXI_CCU_NO_PARENTS,
		},
		[R_CCU_CLOCK_ARISC] = {
			.max_rate = 300000000,
			.reg      = R_CCU_CLOCK_ARISC_REG,
			.parents  = {
				R_CCU_CLOCK_OSC32K,
				R_CCU_CLOCK_OSC24M,
				R_CCU_CLOCK_PLL_PERIPH0,
				R_CCU_CLOCK_IOSC,
			},
			.mux = BITFIELD(16, 2),
			.pd  = {
				[2] = BITFIELD(8, 5),
			},
			.p = BITFIELD(4, 2),
		},
		[R_CCU_CLOCK_APB0] = {
			.max_rate = 100000000,
			.reg      = R_CCU_CLOCK_APB0_REG,
			.parents  = SUNXI_CCU_ONE_PARENT(R_CCU_CLOCK_ARISC),
			.p        = BITFIELD(0, 2),
		},
		[R_CCU_CLOCK_R_PIO] = {
			.gate    = R_CCU_GATE_R_PIO,
			.parents = SUNXI_CCU_NO_PARENTS,
		},
		[R_CCU_CLOCK_R_CIR] = {
			.gate    = R_CCU_GATE_R_CIR,
			.reset   = R_CCU_RESET_R_CIR,
			.reg     = R_CCU_CLOCK_R_CIR_REG,
			.parents = {
				R_CCU_CLOCK_OSC32K,
				R_CCU_CLOCK_OSC24M,
				SUNXI_CCU_NONE,
				SUNXI_CCU_NONE,
			},
			.mux   = BITFIELD(24, 2),
			.m     = BITFIELD(0, 4),
			.p     = BITFIELD(16, 2),
			.flags = SUNXI_CCU_FLAG_GATED,
		},
		[R_CCU_CLOCK_R_TIMER] = {
			.gate    = R_CCU_GATE_R_TIMER,
			.reset   = R_CCU_RESET_R_TIMER,
			.parents = SUNXI_CCU_NO_PARENTS,
		},
		[R_CCU_CLOCK_R_UART] = {
			.gate    = R_CCU_GATE_R_UART,
			.reset   = R_CCU_RESET_R_UART,
			.parents = SUNXI_CCU_NO_PARENTS,
		},
		[R_CCU_CLOCK_R_I2C] = {
			.gate    = R_CCU_GATE_R_I2C,
			.reset   = R_CCU_RESET_R_I2C,
			.parents = SUNXI_CCU_NO_PARENTS,
		},
		[R_CCU_CLOCK_R_TWD] = {
			.gate    = R_CCU_GATE_R_TWD,
			.parents = SUNXI_CCU_NO_PARENTS,
		},
		[R_CCU_CLOCK_SENTINEL] = {
			.flags = SUNXI_CCU_FLAG_LAST,
		},
	},
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
	.clock    = R_CCU_CLOCK_R_PIO,
	.clockdev = &r_ccu,
	.drv      = &sunxi_pio_driver,
	.drvdata  = BIT(0), /*< Physically implemented ports (0). */
};

static struct device r_timer0 = {
	.name     = "r_timer0",
	.regs     = DEV_R_TIMER,
	.clock    = R_CCU_CLOCK_R_TIMER,
	.clockdev = &r_ccu,
	.drv      = &sun8i_r_timer_driver,
	.drvdata  = 0, /*< Timer index within the device. */
	.irq      = IRQ_R_TIMER0,
	.irqdev   = &r_intc,
};

static struct device r_twd = {
	.name     = "r_twd",
	.regs     = DEV_R_TWD,
	.clock    = R_CCU_CLOCK_R_TWD,
	.clockdev = &r_ccu,
	.drv      = &sunxi_twd_driver,
	.irq      = IRQ_R_TWD,
	.irqdev   = &r_intc,
};
