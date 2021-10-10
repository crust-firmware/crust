/*
 * Copyright © 2020-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef PLATFORM_CPUCFG_H
#define PLATFORM_CPUCFG_H

#include <util.h>
#include <platform/devices.h>

#define CPUS_RESET_REG                   (DEV_R_CPUCFG + 0x0000)
#define CPUS_RESET                       BIT(0)

/* Four of these ranges exist. */
#define CPUS_BYTE_SWAP_EN_REG            (DEV_R_CPUCFG + 0x000c)
#define CPUS_BYTE_SWAP_EN_REG_RANGEn(n)  BIT(0 + (n))

#define CPUS_BYTE_SWAP_LO_REG(n)         (DEV_R_CPUCFG + 0x0010 + 0x08 * (n))
#define CPUS_BYTE_SWAP_HI_REG(n)         (DEV_R_CPUCFG + 0x0014 + 0x08 * (n))

#define CPUn_RST_CTRL_REG(n)             (DEV_R_CPUCFG + 0x0040 + 0x40 * (n))
#define CPUn_RST_CTRL_REG_nCORERESET     BIT(1)
#define CPUn_RST_CTRL_REG_nCPUPORESET    BIT(0)

#define CPUn_CTRL_REG(n)                 (DEV_R_CPUCFG + 0x0044 + 0x40 * (n))
#define CPUn_CTRL_REG_CP15SDISABLE       BIT(0)

#define CPUn_STATUS_REG(n)               (DEV_R_CPUCFG + 0x0048 + 0x40 * (n))
#define CPUn_STATUS_REG_STANDBYWFI       BIT(2)
#define CPUn_STATUS_REG_STANDBYWFE       BIT(1)
#define CPUn_STATUS_REG_SMPnAMP          BIT(0)

#define CPU_SYS_RESET_REG                (DEV_R_CPUCFG + 0x0140)
#define CPU_SYS_RESET                    BIT(0)

#define CPU_CLK_GATING_REG               (DEV_R_CPUCFG + 0x0144)
#define CPU_CLK_GATING_REG_L2_GATING     BIT(8)
#define CPU_CLK_GATING_REG_CPU_GATING(n) BIT(0 + (n))

#define IRQ_FIQ_STATUS_REG               (DEV_R_CPUCFG + 0x014c)
#define C0_IRQ_OUT(n)                    BIT(0 + (n))
#define C0_IRQ_OUT_MASK                  (0xf << 0)
#define C0_FIQ_OUT(n)                    BIT(8 + (n))
#define C0_FIQ_OUT_MASK                  (0xf << 8)

#define GEN_CTRL_REG                     (DEV_R_CPUCFG + 0x0184)
#define GEN_CTRL_REG_CFGSDISABLE         BIT(8)
#define GEN_CTRL_REG_ACINACTM            BIT(6)
#define GEN_CTRL_REG_nL2RESET            BIT(5)
#define GEN_CTRL_REG_L2RSTDISABLE        BIT(4)
#define GEN_CTRL_REG_L1RSTDISABLE(n)     BIT(0 + (n))
#define GEN_CTRL_REG_L1RSTDISABLE_MASK   (0xf << 0)

#define EVENT_IN_REG                     (DEV_R_CPUCFG + 0x0190)
#define EVENT_IN                         BIT(0)

#define DBG_CTRL_REG0                    (DEV_R_CPUCFG + 0x01e0)

#define DBG_CTRL_REG1                    (DEV_R_CPUCFG + 0x01e4)
#define DBG_CTRL_REG1_DBGPWRDUP(n)       BIT(0 + (n))
#define DBG_CTRL_REG1_DBGPWRDUP_MASK     (0xf << 0)

#endif /* PLATFORM_CPUCFG_H */
