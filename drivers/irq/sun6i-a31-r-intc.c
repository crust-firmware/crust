/*
 * Copyright © 2017-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <irq.h>
#include <mmio.h>
#include <stdint.h>
#include <platform/devices.h>

#include "irq.h"

#define INTC_VECTOR_REG      0x0000
#define INTC_BASE_ADDR_REG   0x0004
#define INTC_PROTECT_REG     0x0008
#define INTC_NMI_CTRL_REG    0x000c
#define INTC_IRQ_PEND_REG(n) (0x0010 + 4 * (n))
#define INTC_IRQ_EN_REG(n)   (0x0040 + 4 * (n))
#define INTC_IRQ_MASK_REG(n) (0x0050 + 4 * (n))
#define INTC_IRQ_RESP_REG(n) (0x0060 + 4 * (n))
#define INTC_MUX_EN_REG(n)   (0x00c0 + 4 * (n))

#define NUM_IRQ_REGS         (CONFIG(PLATFORM_H6) ? 2 : 1)
#define NUM_MUX_REGS         4

uint32_t
irq_poll(void)
{
	uint32_t pending = irq_poll_eint();

	for (int i = 0; i < NUM_IRQ_REGS; ++i)
		pending |= mmio_read_32(DEV_R_INTC + INTC_IRQ_PEND_REG(i));

	return pending;
}
