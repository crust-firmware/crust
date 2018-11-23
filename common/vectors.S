/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <macros.S>
#include <arch/exception.h>
#include <arch/spr.h>

/* Offset in bytes of the nth register stored on the stack. */
#define REG(n) (4 * (n))

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
	l.addi	r1, r1, -256		# 128b registers, 128b old redzone
	l.sw	REG(3)(r1), r3		# Argument 1 clobbers r3; save it first
	l.j	vector_stub_common
	l.addi	r3, r0, \num		# Send the exception number as argument
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
	l.sw	REG(2)(r1), r2
	l.sw	REG(4)(r1), r4
	l.sw	REG(5)(r1), r5
	l.sw	REG(6)(r1), r6
	l.sw	REG(7)(r1), r7
	l.sw	REG(8)(r1), r8
	l.sw	REG(9)(r1), r9
	l.sw	REG(10)(r1), r10
	l.sw	REG(11)(r1), r11
	l.sw	REG(12)(r1), r12
	l.sw	REG(13)(r1), r13
	l.sw	REG(14)(r1), r14
	l.sw	REG(15)(r1), r15
	l.sw	REG(16)(r1), r16
	l.sw	REG(17)(r1), r17
	l.sw	REG(18)(r1), r18
	l.sw	REG(19)(r1), r19
	l.sw	REG(20)(r1), r20
	l.sw	REG(21)(r1), r21
	l.sw	REG(22)(r1), r22
	l.sw	REG(23)(r1), r23
	l.sw	REG(24)(r1), r24
	l.sw	REG(25)(r1), r25
	l.sw	REG(26)(r1), r26
	l.sw	REG(27)(r1), r27
	l.sw	REG(28)(r1), r28
	l.sw	REG(29)(r1), r29
	l.sw	REG(30)(r1), r30
	l.sw	REG(31)(r1), r31
	l.mfspr	r4, r0, SPR_SYS_EPCR_INDEX(0) # Get the exception PC
	l.sw	REG(0)(r1), r4		# Save it in exception_regs
	l.addi	r4, r1, 256		# Get the original stack pointer
	l.sw	REG(1)(r1), r4		# Save it in exception_regs
	l.jal	handle_exception
	l.addi	r4, r1, 0		# Send &exception_regs as argument 2
	l.lwz	r4, REG(0)(r1)		# Load new PC from exception_regs
	l.mtspr	r0, r4, SPR_SYS_EPCR_INDEX(0) # Save it in the SPR
	l.lwz	r2, REG(2)(r1)
	l.lwz	r3, REG(3)(r1)
	l.lwz	r4, REG(4)(r1)
	l.lwz	r5, REG(5)(r1)
	l.lwz	r6, REG(6)(r1)
	l.lwz	r7, REG(7)(r1)
	l.lwz	r8, REG(8)(r1)
	l.lwz	r9, REG(9)(r1)
	l.lwz	r10, REG(10)(r1)
	l.lwz	r11, REG(11)(r1)
	l.lwz	r12, REG(12)(r1)
	l.lwz	r13, REG(13)(r1)
	l.lwz	r14, REG(14)(r1)
	l.lwz	r15, REG(15)(r1)
	l.lwz	r16, REG(16)(r1)
	l.lwz	r17, REG(17)(r1)
	l.lwz	r18, REG(18)(r1)
	l.lwz	r19, REG(19)(r1)
	l.lwz	r20, REG(20)(r1)
	l.lwz	r21, REG(21)(r1)
	l.lwz	r22, REG(22)(r1)
	l.lwz	r23, REG(23)(r1)
	l.lwz	r24, REG(24)(r1)
	l.lwz	r25, REG(25)(r1)
	l.lwz	r26, REG(26)(r1)
	l.lwz	r27, REG(27)(r1)
	l.lwz	r28, REG(28)(r1)
	l.lwz	r29, REG(29)(r1)
	l.lwz	r30, REG(30)(r1)
	l.lwz	r31, REG(31)(r1)
	l.addi	r1, r1, 256
	l.rfe				# No delay slot
endfunc vector_stub_common
