/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef COMMON_INTERRUPTS_H
#define COMMON_INTERRUPTS_H

#include <compiler.h>
#include <spr.h>
#include <stdint.h>

/**
 * Disable interrupts and provide a compiler barrier. The return value must be
 * saved and passed to restore_interrupts() at the end of the critical section.
 */
static inline uint32_t __must_check
disable_interrupts(void)
{
	uint32_t mask = SPR_SYS_SR_IEE_MASK | SPR_SYS_SR_TEE_MASK;
	uint32_t reg  = mfspr(SPR_SYS_SR_ADDR);

	mtspr(SPR_SYS_SR_ADDR, reg & ~mask);

	return reg;
}

/**
 * Restore interrupts and provide a compiler barrier.
 *
 * @param reg The register contents saved from a call to disable_interrupts().
 */
static inline void
restore_interrupts(uint32_t reg)
{
	mtspr(SPR_SYS_SR_ADDR, reg);
}

#endif /* COMMON_INTERRUPTS_H */
