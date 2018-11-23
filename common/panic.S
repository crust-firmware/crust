/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <macros.S>
#include <arch/spr.h>

#define EXCEPTION_MASK (SPR_SYS_SR_IEE_MASK | SPR_SYS_SR_TEE_MASK)

func panic
	l.jal	log			# Log the panic message (clobbers r9)
	l.nop
	l.addi	r4, r0, -1		# Mask interrupt and tick exceptions
	l.xori	r4, r4, EXCEPTION_MASK
	l.mfspr	r3, r0, SPR_SYS_SR_ADDR
	l.and	r3, r3, r4
	l.mtspr	r0, r3, SPR_SYS_SR_ADDR
1:	l.j	1b			# Infinite loop
	l.nop
endfunc panic
