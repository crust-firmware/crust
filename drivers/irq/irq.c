/*
 * Copyright © 2017-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <irq.h>
#include <stdint.h>

uint32_t WEAK
irq_needs_avcc(void)
{
	return 0;
}

uint32_t WEAK
irq_needs_vdd_sys(void)
{
	return 0;
}

uint32_t WEAK
irq_poll(void)
{
	return 0;
}
