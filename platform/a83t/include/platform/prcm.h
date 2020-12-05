/*
 * Copyright © 2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef PLATFORM_PRCM_H
#define PLATFORM_PRCM_H

#include <util.h>
#include <platform/devices.h>

#define CPUS_CLK_REG                      (DEV_R_PRCM + 0x0000)
#define CPUS_CLK_REG_CLK_SRC(x)           ((x) << 16)
#define CPUS_CLK_REG_CLK_SRC_MASK         (0x3 << 16)
#define CPUS_CLK_REG_PRE_DIV(x)           ((x) << 8)
#define CPUS_CLK_REG_PRE_DIV_MASK         (0x1f << 8)
#define CPUS_CLK_REG_DIV_P(x)             ((x) << 4)
#define CPUS_CLK_REG_DIV_P_MASK           (0x3 << 4)

#define APB0_CLK_REG                      (DEV_R_PRCM + 0x000c)
#define APB0_CLK_REG_DIV_M(x)             ((x) << 0)
#define APB0_CLK_REG_DIV_M_MASK           (0x3 << 0)

/* See r_ccu driver for bit definitions */
#define APB0_GATE_REG                     (DEV_R_PRCM + 0x0028)

/* Documented in A23/A31s manual */
#define PLL_CTRL_REG0                     (DEV_R_PRCM + 0x0040)
#define PLL_CTRL_REG0_TEST_CLK_SEL        BIT(24)
#define PLL_CTRL_REG0_OSC24M_CLK_SEL(x)   ((x) << 20)
#define PLL_CTRL_REG0_OSC24M_CLK_SEL_MASK (0x3 << 20)
#define PLL_CTRL_REG0_PLL_INPUT_SEL(x)    ((x) << 12)
#define PLL_CTRL_REG0_PLL_INPUT_SEL_MASK  (0x3 << 12)
#define PLL_CTRL_REG0_USB24M_CLK_SEL(x)   ((x) << 4)
#define PLL_CTRL_REG0_USB24M_CLK_SEL_MASK (0x3 << 4)
#define PLL_CTRL_REG0_OSC24M_GAIN_ENHANCE BIT(1)
#define PLL_CTRL_REG0_PLL_BIAS_EN         BIT(0)

/* Documented in A23/A31s manual; bits 3 and 15 are not verified on A83T */
#define PLL_CTRL_REG1                     (DEV_R_PRCM + 0x0044)
#define PLL_CTRL_REG1_KEY                 (0xa7 << 24)
#define PLL_CTRL_REG1_KEY_FIELD           (0xff << 24)
#define PLL_CTRL_REG1_PLL_LDO_OUT(x)      ((x) << 16)
#define PLL_CTRL_REG1_PLL_LDO_OUT_MASK    (0x7 << 16)
#define PLL_CTRL_REG1_PLL_IN_PWR_SEL      BIT(15)
#define PLL_CTRL_REG1_CLKTEST_EN          BIT(3)
#define PLL_CTRL_REG1_CRYSTAL_EN          BIT(2)
#define PLL_CTRL_REG1_LDO_EN              (0x3 << 0)

/* See r_ccu driver for bit definitions */
#define R_CIR_CLK_REG                     (DEV_R_PRCM + 0x0054)
#define APB0_RESET_REG                    (DEV_R_PRCM + 0x00b0)

#define VDD_SYS_PWROFF_GATING_REG         (DEV_R_PRCM + 0x0110)
#define VCC_GPIO_GATING                   BIT(12)
#define VCC_PLL_GATING                    BIT(8)
#define VCC_PLL_LOW_VOLTAGE_GATING        BIT(4)
#define VDD_CPUS_GATING                   BIT(3)
#define DRAM_PAD_HOLD                     (0x3 << 0)

#define GPU_PWROFF_GATING_REG             (DEV_R_PRCM + 0x0118)
#define GPU_PWROFF_GATING                 BIT(0)

#define VDD_SYS_RESET_REG                 (DEV_R_PRCM + 0x0120)
#define VDD_SYS_RESET                     BIT(0)

#define R_PIO_HOLD_REG                    (DEV_R_PRCM + 0x01f0)
#define R_PIO_HOLD_REG_WRITE_PULSE        BIT(31)
#define R_PIO_HOLD_REG_PIO_REG_ADDR(x)    ((x) << 16)
#define R_PIO_HOLD_REG_PIO_REG_ADDR_MASK  (0x3 << 16)
#define R_PIO_HOLD_REG_DATA_WRITE(x)      ((x) << 8)
#define R_PIO_HOLD_REG_DATA_WRITE_MASK    (0xff << 8)
#define R_PIO_HOLD_REG_DATA_READ_MASK     (0xff << 0)

#define OSC24M_CTRL_REG                   (DEV_R_PRCM + 0x01f4)
#define OSC24M_CTRL_REG_OSC24M_SRC_SELECT BIT(1)
#define OSC24M_CTRL_REG_OSC16M_ENABLE     BIT(0)

#endif /* PLATFORM_PRCM_H */
