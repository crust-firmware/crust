/*
 * Copyright © 2019-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <bitfield.h>
#include <mmio.h>
#include <stdint.h>

#include "ccu.h"

uint32_t
ccu_helper_get_rate_m(const struct ccu *self,
                      const struct ccu_clock *clk, uint32_t rate,
                      uint32_t m_shift, uint32_t m_width)
{
	uint32_t val = mmio_read_32(self->regs + clk->reg);

	rate /= bitfield_get(val, m_shift, m_width) + 1;

	return rate;
}

uint32_t
ccu_helper_get_rate_mp(const struct ccu *self,
                       const struct ccu_clock *clk, uint32_t rate,
                       uint32_t m_shift, uint32_t m_width,
                       uint32_t p_shift, uint32_t p_width)
{
	uint32_t val = mmio_read_32(self->regs + clk->reg);

	rate  /= bitfield_get(val, m_shift, m_width) + 1;
	rate >>= bitfield_get(val, p_shift, p_width);

	return rate;
}

uint32_t
ccu_helper_get_rate_p(const struct ccu *self,
                      const struct ccu_clock *clk, uint32_t rate,
                      uint32_t p_shift, uint32_t p_width)
{
	uint32_t val = mmio_read_32(self->regs + clk->reg);

	rate >>= bitfield_get(val, p_shift, p_width);

	return rate;
}
