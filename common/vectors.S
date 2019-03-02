/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <macros.S>
#include <arch/exception.h>
#include <arch/spr.h>

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

	/* This macro creates an exception vector with a common handler. */
	.macro vector_stub num
vector_jump \num vector_stub_\num

func vector_stub_\num
	l.j	vector_stub_common
	l.addi	r2, r0, \num		# Save the exception number
endfunc vector_stub_\num
	.endm

/*
 * To save space, only handle exceptions that can actually happen. For example,
 * this processor does not have an MMU, so it can never experience a page
 * fault. Conveniently, the zero instruction (0x00000000) on OpenRISC 1000 is
 * an unconditional jump back to itself, i.e. an infinite loop. So it's safe to
 * entirely omit vectors for impossible exceptions.
 */
vector_jump RESET_EXCEPTION start
vector_stub BUS_ERROR
vector_stub TICK_TIMER_EXCEPTION
vector_stub ALIGNMENT_EXCEPTION
vector_stub ILLEGAL_INSTRUCTION
vector_stub EXTERNAL_INTERRUPT
vector_stub RANGE_EXCEPTION
vector_stub TRAP_EXCEPTION

func vector_stub_common
	l.addi	r1, r1, -0x50		# 80 bytes for caller-saved registers
	l.sw	0x00(r1), r3
	l.sw	0x04(r1), r4
	l.sw	0x08(r1), r5
	l.sw	0x0c(r1), r6
	l.sw	0x10(r1), r7
	l.sw	0x14(r1), r8
	l.sw	0x18(r1), r9
	l.sw	0x1c(r1), r10
	l.sw	0x20(r1), r11
	l.sw	0x24(r1), r12
	l.sw	0x28(r1), r13
	l.sw	0x2c(r1), r15
	l.sw	0x30(r1), r17
	l.sw	0x34(r1), r19
	l.sw	0x38(r1), r21
	l.sw	0x3c(r1), r23
	l.sw	0x40(r1), r25
	l.sw	0x44(r1), r27
	l.sw	0x48(r1), r29
	l.sw	0x4c(r1), r31
	l.jal	handle_exception
	l.or	r3, r2, r2		# First arg is the exception number
	l.lwz	r3,  0x00(r1)
	l.lwz	r4,  0x04(r1)
	l.lwz	r5,  0x08(r1)
	l.lwz	r6,  0x0c(r1)
	l.lwz	r7,  0x10(r1)
	l.lwz	r8,  0x14(r1)
	l.lwz	r9,  0x18(r1)
	l.lwz	r10, 0x1c(r1)
	l.lwz	r11, 0x20(r1)
	l.lwz	r12, 0x24(r1)
	l.lwz	r13, 0x28(r1)
	l.lwz	r15, 0x2c(r1)
	l.lwz	r17, 0x30(r1)
	l.lwz	r19, 0x34(r1)
	l.lwz	r21, 0x38(r1)
	l.lwz	r23, 0x3c(r1)
	l.lwz	r25, 0x40(r1)
	l.lwz	r27, 0x44(r1)
	l.lwz	r29, 0x48(r1)
	l.lwz	r31, 0x4c(r1)
	l.addi	r1, r1, 0x50
	l.rfe				# No delay slot
endfunc vector_stub_common
