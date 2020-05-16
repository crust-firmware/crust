/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef SPR_H
#define SPR_H

#include <stdint.h>
#include <asm/spr.h>

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
