/*
 * Copyright © 2017-2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <spr.h>
#include <stdint.h>

#define present(val) ((val) ? "present" : "not present")
#define yesno(val)   ((val) ? "yes" : "no")

void
debug_print_sprs(void)
{
	uint32_t val;

	val = mfspr(SPR_SYS_VR_ADDR);
	info("Version Register: 0x%08x", val);
	info("\tRevision: %u", SPR_SYS_VR_REV_GET(val));
	info("\tUpdated Version Registers: %s",
	     present(SPR_SYS_VR_UVRP_GET(val)));
	info("\tConfiguration Template: 0x%x", SPR_SYS_VR_CFG_GET(val));
	info("\tVersion: 0x%x", SPR_SYS_VR_VER_GET(val));

	val = mfspr(SPR_SYS_UPR_ADDR);
	info("Unit Present Register: 0x%08x", val);
	info("\tUPR: %s", present(SPR_SYS_UPR_UP_GET(val)));
	info("\tData Cache: %s", present(SPR_SYS_UPR_DCP_GET(val)));
	info("\tInstruction Cache: %s", present(SPR_SYS_UPR_ICP_GET(val)));
	info("\tData MMU: %s", present(SPR_SYS_UPR_DMP_GET(val)));
	info("\tInstruction MMU: %s", present(SPR_SYS_UPR_IMP_GET(val)));
	info("\tMAC: %s", present(SPR_SYS_UPR_MP_GET(val)));
	info("\tDebug Unit: %s", present(SPR_SYS_UPR_DUP_GET(val)));
	info("\tPerformance Counters Unit: %s",
	     present(SPR_SYS_UPR_PCUP_GET(val)));
	info("\tPower Management: %s", present(SPR_SYS_UPR_PICP_GET(val)));
	info("\tProgrammable Interrupt Controller: %s",
	     present(SPR_SYS_UPR_PMP_GET(val)));
	info("\tTick Timer: %s", present(SPR_SYS_UPR_TTP_GET(val)));
	info("\tCustom Units (mask): 0x%02x", SPR_SYS_UPR_CUP_GET(val));

	val = mfspr(SPR_SYS_CPUCFGR_ADDR);
	info("CPU Configuration Register: 0x%08x", val);
	info("\tNumber of Shadow GPR Files: %d",
	     SPR_SYS_CPUCFGR_NSGF_GET(val));
	info("\tCustom GPR File: %s", yesno(SPR_SYS_CPUCFGR_CGF_GET(val)));
	info("\tORBIS32 Supported: %s", yesno(SPR_SYS_CPUCFGR_OB32S_GET(val)));
	info("\tORBIS64 Supported: %s", yesno(SPR_SYS_CPUCFGR_OB64S_GET(val)));
	info("\tORFPX32 Supported: %s", yesno(SPR_SYS_CPUCFGR_OF32S_GET(val)));
	info("\tORFPX64 Supported: %s", yesno(SPR_SYS_CPUCFGR_OF64S_GET(val)));
	info("\tORVDX64 Supported: %s", yesno(SPR_SYS_CPUCFGR_OV64S_GET(val)));
	info("\tDelay Slot: %s", yesno(!SPR_SYS_CPUCFGR_ND_GET(val)));
	info("\tArchitecture Version Register: %s",
	     present(SPR_SYS_CPUCFGR_AVRP_GET(val)));
	info("\tException Vector Base Address Register: %s",
	     present(SPR_SYS_CPUCFGR_EVBARP_GET(val)));
	info("\tImplementation-Specific Registers (ISR0-7): %s",
	     present(SPR_SYS_CPUCFGR_ISRP_GET(val)));
	info("\tArithmetic Exception Control/Status Registers: %s",
	     present(SPR_SYS_CPUCFGR_AECSRP_GET(val)));

	val = mfspr(SPR_SYS_DCCFGR_ADDR);
	info("Data Cache Configuration Register: 0x%08x", val);
	info("\tNumber of Cache Ways: %d", BIT(SPR_SYS_DCCFGR_NCW_GET(val)));
	info("\tNumber of Cache Sets: %d", BIT(SPR_SYS_DCCFGR_NCS_GET(val)));
	info("\tCache Block Size: %d", BIT(4 + SPR_SYS_DCCFGR_CBS_GET(val)));
	info("\tCache Write Strategy: %s",
	     SPR_SYS_DCCFGR_CWS_GET(val) ? "WT" : "WB");
	info("\tCache Control Register Implemented: %s",
	     yesno(SPR_SYS_DCCFGR_CCRI_GET(val)));
	info("\tCache Block Invalidate Register Implemented: %s",
	     yesno(SPR_SYS_DCCFGR_CBIRI_GET(val)));
	info("\tCache Block Prefetch Register Implemented: %s",
	     yesno(SPR_SYS_DCCFGR_CBPRI_GET(val)));
	info("\tCache Block Lock Register Implemented: %s",
	     yesno(SPR_SYS_DCCFGR_CBLRI_GET(val)));
	info("\tCache Block Flush Register Implemented: %s",
	     yesno(SPR_SYS_DCCFGR_CBFRI_GET(val)));
	info("\tCache Block Write-back Register Implemented: %s",
	     yesno(SPR_SYS_DCCFGR_CBWBRI_GET(val)));

	val = mfspr(SPR_SYS_ICCFGR_ADDR);
	info("Instruction Cache Configuration Register: 0x%08x", val);
	info("\tNumber of Cache Ways: %d", BIT(SPR_SYS_ICCFGR_NCW_GET(val)));
	info("\tNumber of Cache Sets: %d", BIT(SPR_SYS_ICCFGR_NCS_GET(val)));
	info("\tCache Block Size: %d", BIT(4 + SPR_SYS_ICCFGR_CBS_GET(val)));
	info("\tCache Control Register Implemented: %s",
	     yesno(SPR_SYS_ICCFGR_CCRI_GET(val)));
	info("\tCache Block Invalidate Register Implemented: %s",
	     yesno(SPR_SYS_ICCFGR_CBIRI_GET(val)));
	info("\tCache Block Prefetch Register Implemented: %s",
	     yesno(SPR_SYS_ICCFGR_CBPRI_GET(val)));
	info("\tCache Block Lock Register Implemented: %s",
	     yesno(SPR_SYS_ICCFGR_CBLRI_GET(val)));
}
