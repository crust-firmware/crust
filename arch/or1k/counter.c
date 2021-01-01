/*
 * Copyright © 2017-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <counter.h>
#include <spr.h>

void
counter_init(void)
{
	mtspr(SPR_TICK_TTMR_ADDR,
	      SPR_TICK_TTMR_MODE_CONTINUE << SPR_TICK_TTMR_MODE_LSB);
}

uint32_t
counter_read(void)
{
	return mfspr(SPR_TICK_TTCR_ADDR);
}
