/*
 * Copyright © 2017-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <irq.h>
#include <mmio.h>
#include <stdint.h>
#include <platform/devices.h>

#include "irq.h"

#define EINT_CTL_REG(n)    (0x20 * (n) + 0x0210)
#define EINT_STATUS_REG(n) (0x20 * (n) + 0x0214)

uint32_t
irq_poll_eint(void)
{
	uint32_t pending = 0;

#if CONFIG(IRQ_POLL_EINT)
	uint32_t first = CONFIG_IRQ_POLL_EINT_FIRST_BANK;
	uint32_t last  = CONFIG_IRQ_POLL_EINT_LAST_BANK;

	for (uint32_t bank = first; bank <= last; ++bank) {
		pending |= mmio_read_32(DEV_PIO + EINT_CTL_REG(bank)) &
		           mmio_read_32(DEV_PIO + EINT_STATUS_REG(bank));
	}
#endif

	return pending;
}

uint32_t WEAK
irq_poll(void)
{
	return irq_poll_eint();
}
