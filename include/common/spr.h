/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef COMMON_SPR_H
#define COMMON_SPR_H

#include <compiler.h>
#include <stdint.h>
#include <arch/spr.h>

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

#endif /* COMMON_SPR_H */
