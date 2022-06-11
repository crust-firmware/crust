/*
 * Copyright © 2021-2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <division.h>
#include <stdint.h>
#include <util.h>

#include "cir.h"

#define EQ_MARGIN(val, time, margin) \
	(((time) - (margin)) < (val) && (val) < ((time) + (margin)))

#define NUM_DATA_BITS 32

/* NEC time unit is ~562.5 us, or ~1777 units/s. */
#define NEC_UNIT_RATE 1777

/* Convert specified number of time units to number of clock cycles. */
#define NEC_UNITS_TO_CLKS(num) \
	UDIV_ROUND((num) * CONFIG_CIR_CLK_RATE, NEC_UNIT_RATE)

#define NEC_LEAD_M        NEC_UNITS_TO_CLKS(16)
#define NEC_LEAD_S        NEC_UNITS_TO_CLKS(8)
#define NEC_DATA_M        NEC_UNITS_TO_CLKS(1)
#define NEC_DATA_S_0      NEC_UNITS_TO_CLKS(1)
#define NEC_DATA_S_1      NEC_UNITS_TO_CLKS(3)

#define NEC_HALF_MARGIN   (NEC_UNITS_TO_CLKS(1) / 2)
#define NEC_SINGLE_MARGIN NEC_UNITS_TO_CLKS(1)
#define NEC_DOUBLE_MARGIN NEC_UNITS_TO_CLKS(2)

enum {
	NEC_IDLE,
	NEC_HEAD_S,
	NEC_PULSE,
	NEC_DATA,
	NEC_STATES
};

static const uint8_t nec_pulse_states[NEC_STATES] = {
	[NEC_IDLE]   = CIR_MARK,
	[NEC_HEAD_S] = CIR_SPACE,
	[NEC_PULSE]  = CIR_MARK,
	[NEC_DATA]   = CIR_SPACE,
};

uint32_t
cir_decode(struct cir_dec_ctx *ctx)
{
	uint32_t counter = ctx->counter;
	uint32_t ret     = 0;

	/* Consume samples until the pulse state changes. */
	if (nec_pulse_states[ctx->state] == ctx->pulse) {
		ctx->counter += ctx->width;
		ctx->width    = 0;
		return 0;
	}

	/* Then reinitialize the cumulative counter for the next state. */
	ctx->counter = 0;

	switch (ctx->state) {
	case NEC_IDLE:
		if (EQ_MARGIN(counter, NEC_LEAD_M, NEC_DOUBLE_MARGIN))
			ctx->state = NEC_HEAD_S;
		else
			ctx->width = 0;
		break;
	case NEC_HEAD_S:
		if (EQ_MARGIN(counter, NEC_LEAD_S, NEC_SINGLE_MARGIN)) {
			ctx->bits   = 0;
			ctx->buffer = 0;
			ctx->state  = NEC_PULSE;
		} else {
			ctx->state = NEC_IDLE;
		}
		break;
	case NEC_PULSE:
		ctx->state = NEC_IDLE;
		if (!EQ_MARGIN(counter, NEC_DATA_M, NEC_HALF_MARGIN))
			break;
		if (ctx->bits == NUM_DATA_BITS) {
			/* Would be nice to check if inverted values match. */
			if (CONFIG(CIR_PROTO_NECX)) {
				ret = ((ctx->buffer << 16) & GENMASK(23, 16)) |
				      (ctx->buffer & GENMASK(15, 8)) |
				      ((ctx->buffer >> 16) & GENMASK(7, 0));
			} else {
				ret = ((ctx->buffer << 8) & GENMASK(15, 8)) |
				      ((ctx->buffer >> 16) & GENMASK(7, 0));
			}
			debug("NEC code %06x", ret);
		} else {
			ctx->state = NEC_DATA;
		}
		break;
	case NEC_DATA:
		/* NEC is LSB first. */
		ctx->buffer >>= 1;
		ctx->bits++;
		ctx->state = NEC_PULSE;
		if (EQ_MARGIN(counter, NEC_DATA_S_1, NEC_HALF_MARGIN))
			ctx->buffer |= BIT(31);
		else if (!EQ_MARGIN(counter, NEC_DATA_S_0, NEC_HALF_MARGIN))
			ctx->state = NEC_IDLE;
		break;
	default:
		unreachable();
	}

	return ret;
}
