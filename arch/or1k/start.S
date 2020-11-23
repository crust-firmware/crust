/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <macros.S>
#include <asm/spr.h>

func start
	# Save the exception vector address
	l.mfspr	r2, r0, SPR_SYS_PPC_ADDR
	l.sfltui r2, 0x4000		# Size of exception vector area
	l.bf	1f
	l.srli	r2, r2, 8		# Vector address → exception number
	l.addi	r2, r0, 0		# Set to zero if not an exception

	# Invalidate the instruction cache
1:	l.addi	r3, r0, 0
	l.addi	r4, r0, 4096		# Cache lines (256) * block size (16)
1:	l.mtspr	r0, r3, SPR_ICACHE_ICBIR_ADDR
	l.sfltu	r3, r4
	l.bf	1b
	l.addi	r3, r3, 16		# Cache block size

	# Flush the CPU pipeline
	l.psync

	# Enable the instruction cache
	l.mfspr	r3, r0, SPR_SYS_SR_ADDR
	l.ori	r3, r3, SPR_SYS_SR_ICE_MASK
	l.mtspr	r0, r3, SPR_SYS_SR_ADDR

	# One cache block of nops
	l.nop
	l.nop
	l.nop
	l.nop

	# Clear .bss
	l.movhi	r10, hi(start)		# High word is common to all symbols
	l.ori	r3, r10, lo(__bss_start)
	l.ori	r4, r10, lo(__bss_end)
1:	l.sw	0(r3), r0
	l.sfltu	r3, r4
	l.bf	1b
	l.addi	r3, r3, 4

	# Prepare function arguments
	l.or	r3, r2, r2

	# Jump to the C entry point
	l.j	system_state_machine
	l.ori	r1, r10, lo(__stack_end)
endfunc start
