/*
 * Copyright © 2017 Samuel Holland <samuel@sholland.org>
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <macros.S>
#include <arch/exception.h>

	/* This macro creates an exception vector that jumps to a function. */
	.macro vector_jump num label
	.section .vectors, "ax", @progbits
	.org EXCEPTION_VECTOR_ADDRESS(\num)
	.global vector_\num
	.type vector_\num, %function
	.func vector_\num
	.cfi_sections .debug_frame
	.cfi_startproc
vector_\num:
	l.j \label
	l.nop
	.cfi_endproc
	.endfunc
	.size vector_\num, . - vector_\num
	.endm

vector_jump RESET_EXCEPTION start
