/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <exception.h>
#include <irqchip.h>
#include <spr.h>
#include <stdint.h>
#include <irqchip/sun4i-intc.h>

void
handle_exception(uint32_t number)
{
	switch (number) {
	case TICK_TIMER_EXCEPTION:
		break;
	case EXTERNAL_INTERRUPT:
		sun4i_intc_irq(&r_intc);
		break;
	default:
		panic("Unhandled exception %d at %p!",
		      number, (void *)mfspr(SPR_SYS_EPCR_INDEX(0)));
	}
}
