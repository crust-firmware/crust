/*
 * Copyright © 2017 Samuel Holland <samuel@sholland.org>
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <debug.h>
#include <exception.h>
#include <stdint.h>

void
handle_exception(uint32_t number, struct exception_regs *regs)
{
	switch (number) {
	case TICK_TIMER_EXCEPTION:
		break;
	case EXTERNAL_INTERRUPT:
		break;
	default:
		panic("unhandled exception %d at %p! (lr=%p)",
		      number, (void *)regs->pc, (void *)regs->r9);
	}
}
