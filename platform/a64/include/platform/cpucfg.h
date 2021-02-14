/*
 * Copyright © 2020-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef PLATFORM_CPUCFG_H
#define PLATFORM_CPUCFG_H

#include <util.h>
#include <platform/devices.h>

#define C0_CTRL_REG0                      (DEV_CPUCFG + 0x0000)
#define C0_CTRL_REG0_SYSBARDISABLE        BIT(31)
#define C0_CTRL_REG0_BROADCASTINNER       BIT(30)
#define C0_CTRL_REG0_BROADCASTOUTER       BIT(29)
#define C0_CTRL_REG0_BROADCASTCACHEMAINT  BIT(28)
#define C0_CTRL_REG0_AA64nAA32(n)         BIT(24 + (n))
#define C0_CTRL_REG0_CP15SDISABLE(n)      BIT(8 + (n))
#define C0_CTRL_REG0_L2RSTDISABLE         BIT(4)

#define C0_CTRL_REG1                      (DEV_CPUCFG + 0x0004)
#define C0_CTRL_REG1_ACINACTM             BIT(0)

#define CACHE_CFG_REG0                    (DEV_CPUCFG + 0x0008)
#define CACHE_CFG_REG0_L1SDT_DELAY        (0x7 << 28)
#define CACHE_CFG_REG0_L1TLB_DELAY        (0x7 << 24)
#define CACHE_CFG_REG0_BTAC_DELAY         (0x7 << 20)
#define CACHE_CFG_REG0_L1DY_DELAY         (0x7 << 16)
#define CACHE_CFG_REG0_L1DT_DELAY         (0x7 << 12)
#define CACHE_CFG_REG0_L1DD_DELAY         (0x7 << 8)
#define CACHE_CFG_REG0_L1IT_DELAY         (0x7 << 4)
#define CACHE_CFG_REG0_L1ID_DELAY         (0x7 << 0)

#define CACHE_CFG_REG1                    (DEV_CPUCFG + 0x000c)
#define CACHE_CFG_REG1_EMAW               (0x7 << 24)
#define CACHE_CFG_REG1_EMA                (0x7 << 16)
#define CACHE_CFG_REG1_L2V_DELAY          (0x7 << 12)
#define CACHE_CFG_REG1_L2T_DELAY          (0x7 << 4)

#define DBG_REG0                          (DEV_CPUCFG + 0x0020)
/* This bit is present; its meaning is guessed from the H6 manual */
#define DBG_REG0_DBGL1RSTDISABLE          BIT(16)
#define DBG_REG0_DBGPWRDUP(n)             BIT(0 + (n))
#define DBG_REG0_DBGPWRDUP_MASK           (0xf << 0)

/* Name taken from H6 manual. */
#define C0_CTRL_REG2                      (DEV_CPUCFG + 0x0028)
#define C0_CTRL_REG2_EVENTI               BIT(24)
#define C0_CTRL_REG2_EXM_CLR(n)           BIT(20 + (n))
#define C0_CTRL_REG2_CLREXMONREQ          BIT(16)
#define C0_CTRL_REG2_CRYPTODISABLE(n)     BIT(12 + (n))
#define C0_CTRL_REG2_L2FLUSHREQ           BIT(8)
#define C0_CTRL_REG2_GICCDISABLE          BIT(4)

#define C0_CPU_STATUS_REG                 (DEV_CPUCFG + 0x0030)
#define C0_CPU_STATUS_REG_SMPnAMP(n)      BIT(24 + (n))
#define C0_CPU_STATUS_REG_STANDBYWFI(n)   BIT(16 + (n))
#define C0_CPU_STATUS_REG_STANDBYWFI_MASK (0xf << 16)
#define C0_CPU_STATUS_REG_STANDBYWFE(n)   BIT(8 + (n))
#define C0_CPU_STATUS_REG_STANDBYWFE_MASK (0xf << 8)
#define C0_CPU_STATUS_REG_STANDBYWFIL2    BIT(0)

#define L2_STATUS_REG                     (DEV_CPUCFG + 0x003c)
#define L2_STATUS_REG_L2FLUSHDONE         BIT(10)
#define L2_STATUS_REG_EVENTO              BIT(9)
#define L2_STATUS_REG_CLREXMONACK         BIT(8)

#define C0_RST_CTRL_REG                   (DEV_CPUCFG + 0x0080)
#define C0_RST_CTRL_REG_nDDR_RST          BIT(28)
#define C0_RST_CTRL_REG_nSOC_DBG_RST      BIT(24)
#define C0_RST_CTRL_REG_nMBISTRESET       BIT(20)
#define C0_RST_CTRL_REG_nH_RST            BIT(12)
#define C0_RST_CTRL_REG_nL2RESET          BIT(8)
#define C0_RST_CTRL_REG_nCORERESET(n)     BIT(0 + (n))
#define C0_RST_CTRL_REG_MASK              (C0_RST_CTRL_REG_nDDR_RST | \
	                                   C0_RST_CTRL_REG_nSOC_DBG_RST | \
	                                   C0_RST_CTRL_REG_nMBISTRESET | \
	                                   C0_RST_CTRL_REG_nH_RST | \
	                                   C0_RST_CTRL_REG_nL2RESET)

#define RVBA_LO_REG(n)                    (DEV_CPUCFG + 0x00a0 + 0x08 * (n))
#define RVBA_HI_REG(n)                    (DEV_CPUCFG + 0x00a4 + 0x08 * (n))

/* Four of these ranges exist. */
#define CPUS_BYTE_SWAP_EN_REG             (DEV_R_CPUCFG + 0x000c)
#define CPUS_BYTE_SWAP_EN_REG_RANGEn(n)   BIT(0 + (n))

#define CPUS_BYTE_SWAP_LO_REG(n)          (DEV_R_CPUCFG + 0x0010 + 0x08 * (n))
#define CPUS_BYTE_SWAP_HI_REG(n)          (DEV_R_CPUCFG + 0x0014 + 0x08 * (n))

#define C0_PWRON_RESET_REG                (DEV_R_CPUCFG + 0x0030)
#define C0_PWRON_RESET_REG_nCPUPORESET(n) BIT(0 + (n))

#define CPU_SYS_RESET_REG                 (DEV_R_CPUCFG + 0x0140)
#define CPU_SYS_RESET                     BIT(0)

#endif /* PLATFORM_CPUCFG_H */
