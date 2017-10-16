/*
 * Copyright © 2017 Samuel Holland <samuel@sholland.org>
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <macros.S>
#include <platform/memory.h>

#define STACK_INIT (STACK_BASE + STACK_SIZE)

func start
	l.ori	r3, r0, lo(__bss_start)
	l.ori	r4, r0, lo(__bss_end)
1:	l.sfgeu	r3, r4			# Clear .bss
	l.bf	2f
	l.addi	r3, r3, 4
	l.j	1b
	l.sw	-4(r3), r0
2:	l.jal	main			# Call C entry point
	l.ori	r1, r0, lo(STACK_INIT)	# Initialize stack pointer
3:	l.j	3b			# Infinite loop
	l.nop
endfunc start
