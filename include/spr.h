/*
 * Copyright © 2017 Samuel Holland <samuel@sholland.org>
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef SPR_H
#define SPR_H

#include <compiler.h>
#include <stdint.h>
#include <arch/spr.h>

#define SPR_VR       0x12000001
#define SPR_UPR      0x00000765
#define SPR_CPUCFGR  0x00000020
#define SPR_DMMUCFGR 0x00000000
#define SPR_IMMUCFGR 0x00000000
#define SPR_DCCFGR   0x00002600
#define SPR_ICCFGR   0x00002640

static inline uint32_t
mfspr(uint16_t addr)
{
	uint32_t value;

	asm volatile ("l.mfspr %0, r0, %1" : "=r" (value) : "K" (addr));
	return value;
}

static inline void
mtspr(uint16_t addr, uint32_t value)
{
	asm volatile ("l.mtspr r0, %1, %0" : : "K" (addr), "r" (value));
}

#endif /* SPR_H */
