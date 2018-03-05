/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <compiler.h>
#include <css.h>
#include <stdint.h>

#define CLUSTER_MAX                 1
#define CORE_MAX                    4

/* c = cluster, n = core. */
#define CPUCFG_CLUSTER_CTRL0_REG(c) (0x0000 + 0x10 * (c))
#define CPUCFG_CLUSTER_CTRL1_REG(c) (0x0004 + 0x10 * (c))
#define CPUCFG_CACHE_CFG0_REG       0x0008
#define CPUCFG_CACHE_CFG1_REG       0x000c
#define CPUCFG_DEBUG_REG            0x0020
#define CPUCFG_GENERAL_CTRL_REG     0x0028
#define CPUCFG_CPU_STATUS_REG(c)    (0x0030 + 0x04 * (c))
#define CPUCFG_L2_STATUS_REG        0x003c
#define CPUCFG_RESET_CTRL_REG(c)    (0x0080 + 0x04 * (c))
#define CPUCFG_RVBAR_LO_REG(c, n)   (0x00a0 + 0x20 * (c) + 0x08 * (n))
#define CPUCFG_RVBAR_HI_REG(c, n)   (0x00a4 + 0x20 * (c) + 0x08 * (n))

#define R_CPUCFG_ARISC_RESET_REG    0x0000
#define R_CPUCFG_PWRON_RESET_REG(c) (0x0030 + 0x04 * (c))
#define R_CPUCFG_SYS_RESET_REG      0x0140
#define R_CPUCFG_SS_FLAG_REG        0x01a0
#define R_CPUCFG_CPU_ENTRY_REG      0x01a4
#define R_CPUCFG_SS_ENTRY_REG       0x01a8
#define R_CPUCFG_HP_FLAG_REG        0x01ac

#define R_PRCM_PWROFF_GATING_REG(c) (0x0100 + 0x04 * (c))
#define R_PRCM_PWR_CLAMP_REG(c, n)  (0x0140 + 0x10 * (c) + 0x04 * (n))

uint8_t
css_get_cluster_count(void)
{
	return CLUSTER_MAX;
}

uint8_t
css_get_core_count(uint8_t cluster __unused)
{
	return CORE_MAX;
}
