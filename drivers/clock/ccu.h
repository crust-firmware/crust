/*
 * Copyright © 2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef CCU_PRIVATE_H
#define CCU_PRIVATE_H

#include <bitfield.h>
#include <clock.h>
#include <stdint.h>
#include <clock/ccu.h>

#include "clock.h"

struct ccu_clock {
	/** Handles to parent clocks (one for each possible mux value). */
	const struct clock_handle *parents;
	/** Hook for calculating the clock rate from the parent rate. */
	uint32_t                   (*get_rate)(const struct ccu *self,
	                                       uint32_t rate, uint8_t id);
	/** Offset into the CCU of the clock gate bit, zero for none. */
	uint16_t                   gate;
	/** Offset into the CCU of the module reset bit, zero for none. */
	uint16_t                   reset;
	/** Offset into the CCU of the mux/factor register. */
	uint16_t                   reg;
	/** Offset and width of the parent mux control in the register. */
	bitfield_t                 mux;
};

uint32_t ccu_calc_rate_mp(uint32_t val, uint32_t rate,
                          uint32_t m_off, uint32_t m_width,
                          uint32_t p_off, uint32_t p_width);
uint32_t ccu_calc_rate_p(uint32_t val, uint32_t rate,
                         uint32_t p_off, uint32_t p_width);

uint32_t ccu_get_rate_parent(const struct ccu *self, uint32_t rate,
                             uint8_t id);

extern const struct clock_driver ccu_driver;

#endif /* CCU_PRIVATE_H */
