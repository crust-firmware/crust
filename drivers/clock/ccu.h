/*
 * Copyright © 2019-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef CCU_PRIVATE_H
#define CCU_PRIVATE_H

#include <clock.h>
#include <stdint.h>
#include <clock/ccu.h>

#include "clock.h"

#define DEFINE_FIXED_PARENT(_name, _dev, _id) \
	UNUSED const struct clock_handle * \
	_name(const struct ccu *self UNUSED, \
	      const struct ccu_clock *clk UNUSED) { \
		static const struct clock_handle _name ## _handle = { \
			.dev = &_dev.dev, \
			.id  = _id, \
		}; \
		return &_name ## _handle; \
	}

#define DEFINE_FIXED_RATE(_name, _rate) \
	uint32_t \
	_name(const struct ccu *self UNUSED, \
	      const struct ccu_clock *clk UNUSED, \
	      uint32_t rate UNUSED) { \
		return _rate; \
	}

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
	/** Offset of the lock bit inside the register (valid if nonzero). */
	uint8_t  lock;
	/** Offset of the update bit inside the register (valid if nonzero). */
	uint8_t  update;
	/** Bit offset of the clock gate (valid if nonzero). */
	uint16_t gate;
	/** Bit offset of the module reset (valid if nonzero). */
	uint16_t reset;
};

/*
 * ccu.c
 * =====
 */

extern const struct clock_driver ccu_driver;

/**
 * Default .get_parent implementation, returns NULL.
 */
const struct clock_handle *ccu_get_null_parent(const struct ccu *self,
                                               const struct ccu_clock *clk);

/**
 * Default .get_rate implementation, returns the parent's rate unmodified.
 */
uint32_t ccu_get_parent_rate(const struct ccu *self,
                             const struct ccu_clock *clk, uint32_t rate);

/*
 * ccu_helpers.c
 * =============
 */

uint32_t ccu_helper_get_rate_m(const struct ccu *self,
                               const struct ccu_clock *clk, uint32_t rate,
                               uint32_t m_shift, uint32_t m_width);
uint32_t ccu_helper_get_rate_mp(const struct ccu *self,
                                const struct ccu_clock *clk, uint32_t rate,
                                uint32_t m_shift, uint32_t m_width,
                                uint32_t p_shift, uint32_t p_width);
uint32_t ccu_helper_get_rate_p(const struct ccu *self,
                               const struct ccu_clock *clk, uint32_t rate,
                               uint32_t p_shift, uint32_t p_width);

/*
 * r_ccu_common.c
 * ==============
 */

uint32_t r_ccu_common_get_osc16m_rate(const struct ccu *self,
                                      const struct ccu_clock *clk,
                                      uint32_t rate);
void r_ccu_common_suspend(void);
void r_ccu_common_resume(void);
void r_ccu_common_init(void);

#endif /* CCU_PRIVATE_H */
