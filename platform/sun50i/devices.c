/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <dm.h>
#include <i2c.h>
#include <util.h>
#include <clock/sunxi-ccu.h>
#include <gpio/sunxi-gpio.h>
#include <i2c/sun6i-a31-i2c.h>
#include <irqchip/sun4i-intc.h>
#include <msgbox/sunxi-msgbox.h>
#include <regulator/axp803.h>
#include <regulator/sy8106a.h>
#include <sensor/sun8i-thermal.h>
#include <timer/sun8i-r_timer.h>
#include <watchdog/sunxi-twd.h>
#include <platform/ccu.h>
#include <platform/devices.h>
#include <platform/irq.h>
#include <platform/r_ccu.h>

#if CONFIG_REGULATOR_AXP803
static struct device axp803_regulator __device;
#endif
static struct device ccu      __device;
static struct device msgbox   __device;
static struct device pio      __device;
static struct device r_ccu    __device;
static struct device r_i2c    __device;
static struct device r_intc   __device;
static struct device r_pio    __device;
static struct device r_timer0 __device;
static struct device r_twd    __device;
#if CONFIG_REGULATOR_SY8106A
static struct device sy8106a __device;
#endif
static struct device ths __device;

#if CONFIG_REGULATOR_AXP803
static struct device axp803_regulator = {
	.name    = "axp803-regulator",
	.drv     = &axp803_regulator_driver.drv,
	.drvdata = AXP803_DRVDATA {
		[AXP803_REGL_DCDC1] = 3300,
		[AXP803_REGL_DCDC2] = 1100,
		[AXP803_REGL_DCDC3] = 1100,
		/* DCDC4 is not connected. */
		[AXP803_REGL_DCDC5] = 1500,
		[AXP803_REGL_DCDC6] = 1100,
		/* DC1SW is not connected. */
		[AXP803_REGL_ALDO1] = 2800,
		[AXP803_REGL_ALDO2] = 3300,
		[AXP803_REGL_ALDO3] = 3000,
		[AXP803_REGL_DLDO1] = 3300,
		[AXP803_REGL_DLDO2] = 3600,
		[AXP803_REGL_DLDO3] = 2800,
		[AXP803_REGL_DLDO4] = 3300,
		[AXP803_REGL_ELDO1] = 1800,
		/* ELDO2 is not connected. */
		[AXP803_REGL_ELDO3] = 1800,
		/* FLDO1 is connected but not used. */
		[AXP803_REGL_FLDO2] = 1100,
		/* GPIO0 is not connected. */
		/* GPIO1 is not connected. */
	},
	.bus  = &r_i2c,
	.addr = 0x34,
};
#endif

static struct device ccu = {
	.name    = "ccu",
	.regs    = DEV_CCU,
	.drv     = &sunxi_ccu_driver.drv,
	.drvdata = SUNXI_CCU_DRVDATA {
		[CCU_CLOCK_PLL_PERIPH0] = FIXED_CLOCK("pll_periph0",
		                                      600000000, 0),
		[CCU_CLOCK_MSGBOX] = {
			.info = {
				.name  = "msgbox",
				.flags = CLK_FIXED,
			},
			.gate  = CCU_GATE_MSGBOX,
			.reset = CCU_RESET_MSGBOX,
		},
		[CCU_CLOCK_PIO] = {
			.info = {
				.name  = "pio",
				.flags = CLK_FIXED,
			},
			.gate  = CCU_GATE_PIO,
			.reset = CCU_RESET_PIO,
		},
		[CCU_CLOCK_THS] = {
			.info = {
				.name  = "ths",
				.flags = CLK_FIXED,
			},
			.gate  = CCU_GATE_THS,
			.reset = CCU_RESET_THS,
		},
		[CCU_CLOCK_THS_MOD] = {
			.info = {
				.name     = "ths_mod",
				.max_rate = 6000000,
			},
			.parents = CLOCK_PARENTS(4) {
				{ .dev = &r_ccu, .id = R_CCU_CLOCK_OSC24M },
			},
			.gate = BITMAP_INDEX(CCU_CLOCK_THS_REG / 4, 31),
			.reg  = CCU_CLOCK_THS_REG,
			.mux  = BITFIELD(24, 2),
			.p    = BITFIELD(0, 2),
		},
	},
	.subdev_count = CCU_CLOCK_COUNT,
};

static struct device msgbox = {
	.name    = "msgbox",
	.regs    = DEV_MSGBOX,
	.drv     = &sunxi_msgbox_driver.drv,
	.drvdata = SUNXI_MSGBOX_DRVDATA { 0 },
	.clocks  = CLOCK_PARENT(ccu, CCU_CLOCK_MSGBOX),
	.irqdev  = &r_intc,
	.irq     = IRQ_MSGBOX,
};

static struct device pio = {
	.name    = "pio",
	.regs    = DEV_PIO,
	.drv     = &sunxi_gpio_driver.drv,
	.drvdata = BITMASK(1, 7), /**< Physically implemented ports (1-7). */
	.clocks  = CLOCK_PARENT(ccu, CCU_CLOCK_PIO),
};

static struct device r_ccu = {
	.name    = "r_ccu",
	.regs    = DEV_R_PRCM,
	.drv     = &sunxi_ccu_driver.drv,
	.drvdata = SUNXI_CCU_DRVDATA {
		[R_CCU_CLOCK_OSC24M] = FIXED_CLOCK("osc24m", 24000000, 0),
		[R_CCU_CLOCK_OSC32K] = FIXED_CLOCK("osc32k", 32768, 0),
		[R_CCU_CLOCK_OSC16M] = FIXED_CLOCK("osc16m", 16000000, 0),
		[R_CCU_CLOCK_AHB0]   = {
			.info = {
				.name     = "ahb0",
				.max_rate = 300000000,
				.flags    = CLK_CRITICAL,
			},
			.parents = CLOCK_PARENTS(4) {
				{ .dev = &r_ccu, .id = R_CCU_CLOCK_OSC32K },
				{ .dev = &r_ccu, .id = R_CCU_CLOCK_OSC24M },
				{
					.dev  = &ccu,
					.id   = CCU_CLOCK_PLL_PERIPH0,
					.vdiv = BITFIELD(8, 5),
				},
				{ .dev = &r_ccu, .id = R_CCU_CLOCK_OSC16M },
			},
			.reg = R_CCU_CLOCK_AHB0_REG,
			.mux = BITFIELD(16, 2),
			.p   = BITFIELD(4, 2),
		},
		[R_CCU_CLOCK_APB0] = {
			.info.name = "apb0",
			.parents   = CLOCK_PARENT(r_ccu, R_CCU_CLOCK_AHB0),
			.reg       = R_CCU_CLOCK_APB0_REG,
			.p         = BITFIELD(0, 2),
		},
		[R_CCU_CLOCK_R_PIO] = {
			.info.name = "r_pio",
			.parents   = CLOCK_PARENT(r_ccu, R_CCU_CLOCK_APB0),
			.gate      = R_CCU_GATE_R_PIO,
		},
		[R_CCU_CLOCK_R_CIR] = {
			.info = {
				.name     = "r_cir",
				.max_rate = 100000000,
			},
			.parents = CLOCK_PARENT(r_ccu, R_CCU_CLOCK_APB0),
			.gate    = R_CCU_GATE_R_CIR,
			.reset   = R_CCU_RESET_R_CIR,
		},
		[R_CCU_CLOCK_R_TIMER] = {
			.info.name = "r_timer",
			.parents   = CLOCK_PARENT(r_ccu, R_CCU_CLOCK_APB0),
			.gate      = R_CCU_GATE_R_TIMER,
			.reset     = R_CCU_RESET_R_TIMER,
		},
		[R_CCU_CLOCK_R_UART] = {
			.info.name = "r_uart",
			.parents   = CLOCK_PARENT(r_ccu, R_CCU_CLOCK_APB0),
			.gate      = R_CCU_GATE_R_UART,
			.reset     = R_CCU_RESET_R_UART,
		},
		[R_CCU_CLOCK_R_I2C] = {
			.info.name = "r_i2c",
			.parents   = CLOCK_PARENT(r_ccu, R_CCU_CLOCK_APB0),
			.gate      = R_CCU_GATE_R_I2C,
			.reset     = R_CCU_RESET_R_I2C,
		},
		[R_CCU_CLOCK_R_TWD] = {
			.info.name = "r_twd",
			.parents   = CLOCK_PARENT(r_ccu, R_CCU_CLOCK_APB0),
			.gate      = R_CCU_GATE_R_TWD,
		},
		[R_CCU_CLOCK_R_CIR_MOD] = {
			.info.name = "r_cir_mod",
			.parents   = CLOCK_PARENTS(4) {
				{ .dev = &r_ccu, .id = R_CCU_CLOCK_OSC32K },
				{ .dev = &r_ccu, .id = R_CCU_CLOCK_OSC24M },
			},
			.gate = BITMAP_INDEX(R_CCU_CLOCK_R_CIR_REG / 4, 31),
			.reg  = R_CCU_CLOCK_R_CIR_REG,
			.mux  = BITFIELD(24, 2),
			.m    = BITFIELD(0, 4),
			.p    = BITFIELD(16, 2),
		},
	},
	.subdev_count = R_CCU_CLOCK_COUNT,
};

static struct device r_i2c = {
	.name   = "r_i2c",
	.regs   = DEV_R_I2C,
	.drv    = &sun6i_a31_i2c_driver.drv,
	.clocks = CLOCK_PARENT(r_ccu, R_CCU_CLOCK_R_I2C),
	.pins   = GPIO_PINS(I2C_NUM_PINS) {
		{ &r_pio, 0, 3 },
		{ &r_pio, 1, 3 },
	},
	.irqdev = &r_intc,
	.irq    = IRQ_R_I2C,
};

static struct device r_intc = {
	.name    = "r_intc",
	.regs    = DEV_R_INTC,
	.drv     = &sun4i_intc_driver.drv,
	.drvdata = SUN4I_INTC_DRVDATA { { 0 } },
};

static struct device r_pio = {
	.name    = "r_pio",
	.regs    = DEV_R_PIO,
	.drv     = &sunxi_gpio_driver.drv,
	.drvdata = BIT(0), /**< Physically implemented ports (0). */
	.clocks  = CLOCK_PARENT(r_ccu, R_CCU_CLOCK_R_PIO),
};

static struct device r_timer0 = {
	.name    = "r_timer0",
	.regs    = DEV_R_TIMER,
	.drv     = &sun8i_r_timer_driver.drv,
	.drvdata = 0, /**< Timer index within the device. */
	.clocks  = CLOCK_PARENT(r_ccu, R_CCU_CLOCK_R_TIMER),
	.irqdev  = &r_intc,
	.irq     = IRQ_R_TIMER0,
};

static struct device r_twd = {
	.name   = "r_twd",
	.regs   = DEV_R_TWD,
	.drv    = &sunxi_twd_driver.drv,
	.clocks = CLOCK_PARENT(r_ccu, R_CCU_CLOCK_R_TWD),
	.irqdev = &r_intc,
	.irq    = IRQ_R_TWD,
};

#if CONFIG_REGULATOR_SY8106A
static struct device sy8106a = {
	.name    = "sy8106a",
	.drv     = &sy8106a_driver.drv,
	.drvdata = 1100, /**< Default CPU voltage. */
	.bus     = &r_i2c,
	.addr    = SY8106A_I2C_ADDRESS,
};
#endif

static struct device ths = {
	.name   = "ths",
	.regs   = DEV_THS,
	.drv    = &sun8i_thermal_driver.drv,
	.clocks = CLOCK_PARENTS(2) {
		{ .dev = &ccu, .id = CCU_CLOCK_THS },
		{ .dev = &ccu, .id = CCU_CLOCK_THS_MOD },
	},
};
