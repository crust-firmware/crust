/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <macros.S>
#include <util.h>

#define UART_THR			0x0000
#define UART_LSR			0x0014
#define UART_LSR_THRE			BIT(5)

data console_base
	.word	0

func console_init
	l.ori	r4, r0, lo(console_base)
	l.jr	r9
	l.sw	0(r4), r3
endfunc console_init

func console_putc
	l.ori	r4, r0, lo(console_base)
	l.andi	r3, r3, 0xff
	l.lwz	r4, 0(r4)
	l.sfeqi	r3, '\n'
	l.bnf	1f
	l.cmov	r6, r3, r0
	l.ori	r3, r0, '\r'
1:	l.lwz	r5, UART_LSR(r4)
2:	l.andi	r5, r5, UART_LSR_THRE
	l.sfeqi	r5, 0
	l.bf	2b
	l.lwz	r5, UART_LSR(r4)
	l.sw	UART_THR(r4), r3
	l.ori	r3, r6, 0
	l.sfnei	r3, 0
	l.bf	1b
	l.addi	r6, r0, 0
	l.jr	r9
	l.nop
endfunc console_putc
