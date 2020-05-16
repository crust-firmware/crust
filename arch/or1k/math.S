/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <macros.S>

func udivmod
	l.sw	-8(r1), r18
	l.ori	r18, r3, 0		# Save pointer to dividend
	l.sw	-4(r1), r9
	l.lwz	r3, 0(r3)		# Load dividend
	l.jal	__udivsi3
	l.addi	r1, r1, -8
	l.sw	0(r18), r11		# Replace dividend with quotient
	l.addi	r1, r1, 8
	l.lwz	r9, -4(r1)
	l.lwz	r18, -8(r1)
	l.jr	r9
	l.ori	r11, r12, 0		# Return remainder
endfunc udivmod
