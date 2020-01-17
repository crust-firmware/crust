/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <irq.h>
#include <mmio.h>
#include <stdbool.h>
#include <stdint.h>
#include <util.h>
#include <platform/devices.h>

#define INTC_VECTOR_REG    0x0000
#define INTC_BASE_ADDR_REG 0x0004
#define INTC_PROTECT_REG   0x0008
#define INTC_NMI_CTRL_REG  0x000c
#define INTC_IRQ_PEND_REG  0x0010
#define INTC_EN_REG        0x0040
#define INTC_MASK_REG      0x0050
#define INTC_RESP_REG      0x0060

bool
irq_is_enabled(uint32_t irq)
{
	return mmio_get_32(DEV_R_INTC + INTC_EN_REG, BIT(irq));
}

bool
irq_is_pending(uint32_t irq)
{
	return mmio_get_32(DEV_R_INTC + INTC_IRQ_PEND_REG, BIT(irq));
}

uint32_t
irq_poll(void)
{
	return mmio_read_32(DEV_R_INTC + INTC_EN_REG) &
	       mmio_read_32(DEV_R_INTC + INTC_IRQ_PEND_REG);
}
