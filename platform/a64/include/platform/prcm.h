/*
 * Copyright © 2020-2022 The Crust Firmware Authors.
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
#define RTC_GATE_REG                      (DEV_R_PRCM + 0x002c)

/* Documented in A23/A31s manual; all bits are present on A64 */
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

/* Documented in A23/A31s manual; bits 3 and 15 are not verified on A64 */
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
#define R_CIR_RX_CLK_REG                  (DEV_R_PRCM + 0x0054)
#define APB0_RESET_REG                    (DEV_R_PRCM + 0x00b0)

/* CPU0 does not have its own gating */
#define C0_PWROFF_GATING_REG              (DEV_R_PRCM + 0x0100)
#define C0_CPUn_PWROFF_GATING(n)          BIT(0 + (n))
#define C0_PWROFF_GATING                  BIT(0)

#define VDD_SYS_PWROFF_GATING_REG         (DEV_R_PRCM + 0x0110)
#define VDD_CPUS_GATING                   BIT(3)
#define AVCC_GATING                       BIT(2)
#define DRAM_PAD_HOLD                     (0x3 << 0)

#define GPU_PWROFF_GATING_REG             (DEV_R_PRCM + 0x0118)
#define GPU_PWROFF_GATING                 BIT(0)

#define VDD_SYS_RESET_REG                 (DEV_R_PRCM + 0x0120)
#define VDD_SYS_RESET                     BIT(0)

/* CPU0 does not have its own power switch */
#define C0_CPUn_PWR_SWITCH_REG(n)         (DEV_R_PRCM + 0x0140 + 0x04 * (n))

#define ADDA_PR_CFG_REG                   (DEV_R_PRCM + 0x01c0)
#define ADDA_PR_CFG_REG_RESET             BIT(28)
#define ADDA_PR_CFG_REG_RW                BIT(24)
#define ADDA_PR_CFG_REG_ADDR(x)           ((x) << 16)
#define ADDA_PR_CFG_REG_ADDR_MASK         (0x1f << 16)
#define ADDA_PR_CFG_REG_WDAT(x)           ((x) << 8)
#define ADDA_PR_CFG_REG_WDAT_MASK         (0xff << 8)
#define ADDA_PR_CFG_REG_RDAT(x)           ((x) << 0)
#define ADDA_PR_CFG_REG_RDAT_MASK         (0xff << 0)

#define PRCM_SEC_SWITCH_REG               (DEV_R_PRCM + 0x01d0)
#define PRCM_SEC_SWITCH_REG_POWER_SEC     BIT(2)
#define PRCM_SEC_SWITCH_REG_PLL_SEC       BIT(1)
#define PRCM_SEC_SWITCH_REG_CPUS_CLK_SEC  BIT(0)

#endif /* PLATFORM_PRCM_H */
