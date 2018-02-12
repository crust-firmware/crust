/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <macros.S>
#include <platform/memory.h>

#define STACK_INIT (STACK_BASE + STACK_SIZE)

func start
	l.ori	r3, r0, lo(__bss_start)	# Clear .bss
	l.ori	r4, r0, lo(__bss_end)
1:	l.sw    0(r3), r0
	l.sfltu r3, r4
	l.bf	1b
	l.addi	r3, r3, 4
	l.jal	main			# Call C entry point
	l.ori	r1, r0, lo(STACK_INIT)	# Initialize stack pointer
2:	l.j	2b			# Infinite loop
	l.nop
endfunc start
