/*
 * Copyright © 2017-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <irq.h>
#include <mmio.h>
#include <stdint.h>
#include <util.h>
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

/* Gating AVCC will prevent receiving any of these interrupts. */
static const uint32_t mux_needs_avcc[NUM_MUX_REGS] = {
#if CONFIG(PLATFORM_A64) || CONFIG(PLATFORM_H3)
	[0] = BIT(62 - 32), /* LRADC */
#else
	0 /* No applicable interrupts */
#endif
};

/* Gating VDD_SYS will prevent receiving any of these interrupts. */
static const uint32_t mux_needs_vdd_sys[NUM_MUX_REGS] = {
#if CONFIG(PLATFORM_A64) && CONFIG(SOC_A64)
	[0] = BIT(43 - 32) | /* Port B */
	      BIT(49 - 32) | /* Port G */
	      BIT(53 - 32),  /* Port H */
#elif CONFIG(PLATFORM_A64) && CONFIG(SOC_H5)
	[0] = BIT(43 - 32) | /* Port A */
	      BIT(49 - 32) | /* Port F */
	      BIT(55 - 32),  /* Port G */
#elif CONFIG(PLATFORM_A83T)
	[0] = BIT(47 - 32) | /* Port B */
	      BIT(49 - 32),  /* Port G */
#elif CONFIG(PLATFORM_H3)
	[0] = BIT(43 - 32) | /* Port A */
	      BIT(49 - 32),  /* Port G */
#elif CONFIG(PLATFORM_H6)
	[1] = BIT(83 - 64) | /* Port B */
	      BIT(85 - 64) | /* Port F */
	      BIT(86 - 64) | /* Port G */
	      BIT(91 - 64),  /* Port H */
#else
	0 /* No applicable interrupts */
#endif
};

uint32_t
irq_needs_avcc(void)
{
	uint32_t enabled = 0;

	/* Only read registers with relevant bits. */
	for (int i = 0; i < NUM_MUX_REGS; ++i) {
		if (!mux_needs_avcc[i])
			continue;

		enabled |= mmio_read_32(DEV_R_INTC + INTC_MUX_EN_REG(i)) &
		           mux_needs_avcc[i];
	}

	return enabled;
}

uint32_t
irq_needs_vdd_sys(void)
{
	uint32_t enabled = CONFIG(IRQ_POLL_EINT);

	/* Only read registers with relevant bits. */
	for (int i = 0; i < NUM_MUX_REGS; ++i) {
		if (!mux_needs_vdd_sys[i])
			continue;

		enabled |= mmio_read_32(DEV_R_INTC + INTC_MUX_EN_REG(i)) &
		           mux_needs_vdd_sys[i];
	}

	return enabled;
}

uint32_t
irq_poll(void)
{
	uint32_t pending = irq_poll_eint();

	for (int i = 0; i < NUM_IRQ_REGS; ++i)
		pending |= mmio_read_32(DEV_R_INTC + INTC_IRQ_PEND_REG(i));

	return pending;
}
