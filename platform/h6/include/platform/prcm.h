/*
 * Copyright © 2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef PLATFORM_PRCM_H
#define PLATFORM_PRCM_H

#include <util.h>
#include <platform/devices.h>

#define CPUS_CLK_REG                     (DEV_R_PRCM + 0x0000)
#define CPUS_CLK_REG_CLK_SRC(x)          ((x) << 24)
#define CPUS_CLK_REG_CLK_SRC_MASK        (0x3 << 24)
#define CPUS_CLK_REG_DIV_P(x)            ((x) << 8)
#define CPUS_CLK_REG_DIV_P_MASK          (0x3 << 8)
#define CPUS_CLK_REG_PRE_DIV(x)          ((x) << 0)
#define CPUS_CLK_REG_PRE_DIV_MASK        (0x1f << 0)

#define R_APB1_CLK_REG                   (DEV_R_PRCM + 0x000c)
#define R_APB1_CLK_REG_DIV_M(x)          ((x) << 0)
#define R_APB1_CLK_REG_DIV_M_MASK        (0x3 << 0)

#define R_APB2_CLK_REG                   (DEV_R_PRCM + 0x0010)
#define R_APB2_CLK_REG_CLK_SRC(x)        ((x) << 24)
#define R_APB2_CLK_REG_CLK_SRC_MASK      (0x3 << 24)
#define R_APB2_CLK_REG_DIV_P(x)          ((x) << 8)
#define R_APB2_CLK_REG_DIV_P_MASK        (0x3 << 8)
#define R_APB2_CLK_REG_PRE_DIV(x)        ((x) << 0)
#define R_APB2_CLK_REG_PRE_DIV_MASK      (0x1f << 0)

/* See r_ccu driver for bit definitions */
#define R_TIMER_GATE_REG                 (DEV_R_PRCM + 0x011c)
#define R_TWDOG_GATE_REG                 (DEV_R_PRCM + 0x012c)
#define R_PWM_GATE_REG                   (DEV_R_PRCM + 0x013c)
#define R_UART_GATE_REG                  (DEV_R_PRCM + 0x018c)
#define R_I2C_GATE_REG                   (DEV_R_PRCM + 0x019c)
#define R_RSB_GATE_REG                   (DEV_R_PRCM + 0x01bc)
#define R_CIR_RX_CLK_REG                 (DEV_R_PRCM + 0x01c0)
#define R_CIR_RX_GATE_REG                (DEV_R_PRCM + 0x01cc)
#define R_OWC_CLK_REG                    (DEV_R_PRCM + 0x01e0)
#define R_OWC_GATE_REG                   (DEV_R_PRCM + 0x01ec)
#define R_RTC_GATE_REG                   (DEV_R_PRCM + 0x020c)

/* Differs from earlier generations; only the bits below are valid */
#define PLL_CTRL_REG0                    (DEV_R_PRCM + 0x0240)
#define PLL_CTRL_REG0_UNK_BIT_24         BIT(24)
#define PLL_CTRL_REG0_UNK_BIT_02         BIT(2)
#define PLL_CTRL_REG0_UNK_BIT_01         BIT(1)
#define PLL_CTRL_REG0_UNK_BIT_00         BIT(0)

/* Documented in A23/A31s manual; bits 3 and 15 are not verified on H6 */
#define PLL_CTRL_REG1                    (DEV_R_PRCM + 0x0244)
#define PLL_CTRL_REG1_KEY                (0xa7 << 24)
#define PLL_CTRL_REG1_KEY_FIELD          (0xff << 24)
#define PLL_CTRL_REG1_PLL_LDO_OUT(x)     ((x) << 16)
#define PLL_CTRL_REG1_PLL_LDO_OUT_MASK   (0x7 << 16)
#define PLL_CTRL_REG1_PLL_IN_PWR_SEL     BIT(15)
#define PLL_CTRL_REG1_CLKTEST_EN         BIT(3)
#define PLL_CTRL_REG1_CRYSTAL_EN         BIT(2)
#define PLL_CTRL_REG1_LDO_EN             BIT(0)

#define VDD_SYS_PWROFF_GATING_REG        (DEV_R_PRCM + 0x0250)
#define VDD_CPUS_GATING                  BIT(3)
#define VCC_PLL_GATING                   BIT(2)
#define DRAM_PAD_HOLD                    (0x3 << 0)

#define GPU_PWROFF_GATING_REG            (DEV_R_PRCM + 0x0254)
#define GPU_PWROFF_GATING                BIT(0)

#define VDD_SYS_RESET_REG                (DEV_R_PRCM + 0x0260)
#define VDD_SYS_RESET                    BIT(0)

/* Bits are documented for A64 and assumed to match here */
#define PRCM_SEC_SWITCH_REG              (DEV_R_PRCM + 0x0290)
#define PRCM_SEC_SWITCH_REG_POWER_SEC    BIT(2)
#define PRCM_SEC_SWITCH_REG_PLL_SEC      BIT(1)
#define PRCM_SEC_SWITCH_REG_CPUS_CLK_SEC BIT(0)

#define RES_CAL_CTRL_REG                 (DEV_R_PRCM + 0x0310)
/* set in dram_power_up_process() */
#define RES_CAL_CTRL_REG_UNK_BIT_08      BIT(8)
/* "calibration circuits analog enable" */
#define RES_CAL_CTRL_REG_CAL_ANA_EN      BIT(1)
#define RES_CAL_CTRL_REG_CAL_EN          BIT(0)

#define UNK_REG_0318                     (DEV_R_PRCM + 0x0318)
/* cleared in dram_power_up_process() */
#define UNK_REG_0318_UNK_BIT_00_05       (0x3f << 0)

#endif /* PLATFORM_PRCM_H */
