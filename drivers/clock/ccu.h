/*
 * Copyright © 2019-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef CCU_PRIVATE_H
#define CCU_PRIVATE_H

#include <clock.h>
#include <stdint.h>
#include <clock/ccu.h>

#include "clock.h"

struct ccu_clock {
	/** Hook for determining the parent clock. */
	const struct clock_handle *(*get_parent)(const struct ccu *self,
	                                         const struct ccu_clock *clk);
	/** Hook for calculating the clock rate from the parent rate. */
	uint32_t                   (*get_rate)(const struct ccu *self,
	                                       const struct ccu_clock *clk,
	                                       uint32_t rate);
	/** Byte offset of the clock configuration register. */
	uint16_t reg;
	/** Bit offset of the clock gate (valid if nonzero). */
	uint16_t gate;
	/** Bit offset of the module reset (valid if nonzero). */
	uint16_t reset;
};

uint32_t ccu_calc_rate_m(uint32_t val, uint32_t rate,
                         uint32_t m_off, uint32_t m_width);
uint32_t ccu_calc_rate_mp(uint32_t val, uint32_t rate,
                          uint32_t m_off, uint32_t m_width,
                          uint32_t p_off, uint32_t p_width);
uint32_t ccu_calc_rate_p(uint32_t val, uint32_t rate,
                         uint32_t p_off, uint32_t p_width);

const struct clock_handle *ccu_get_parent_none(const struct ccu *self,
                                               const struct ccu_clock *clk);

uint32_t ccu_get_rate_parent(const struct ccu *self,
                             const struct ccu_clock *clk, uint32_t rate);

extern const struct clock_driver ccu_driver;

#endif /* CCU_PRIVATE_H */
