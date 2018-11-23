/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <devices.h>
#include <exception.h>
#include <irqchip.h>
#include <stdint.h>
#include <irqchip/sun4i-intc.h>

void
handle_exception(uint32_t number, struct exception_regs *regs)
{
	switch (number) {
	case TICK_TIMER_EXCEPTION:
		break;
	case EXTERNAL_INTERRUPT:
		sun4i_intc_irq(&r_intc);
		break;
	default:
		panic("Unhandled exception %d at %p! (lr=%p)",
		      number, (void *)regs->pc, (void *)regs->r9);
	}
}
