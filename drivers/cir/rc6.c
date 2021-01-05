/*
 * Copyright © 2020-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <division.h>
#include <stdint.h>
#include <util.h>

#include "cir.h"

#define NUM_DATA_BITS    32
#define NUM_HEADER_BITS  4

/* RC6 time unit is 16 periods @ 36 kHz, ~444 us. */
#define RC6_CARRIER_FREQ 36000
#define RC6_UNIT_RATE    (RC6_CARRIER_FREQ / 16)

/* Convert specified number of time units to number of clock cycles. */
#define RC6_UNITS_TO_CLKS(num) \
	UDIV_ROUND((num) * CONFIG_CIR_CLK_RATE, RC6_UNIT_RATE)

enum {
	RC6_IDLE,
	RC6_LEADER_S,
	RC6_HEADER_P,
	RC6_HEADER_N,
	RC6_TRAILER_P,
	RC6_TRAILER_N,
	RC6_DATA_P,
	RC6_DATA_N,
	RC6_STATES
};

static const int16_t rc6_durations[RC6_STATES] = {
	[RC6_IDLE]      = RC6_UNITS_TO_CLKS(6),
	[RC6_LEADER_S]  = RC6_UNITS_TO_CLKS(2),
	[RC6_HEADER_P]  = RC6_UNITS_TO_CLKS(1),
	[RC6_HEADER_N]  = RC6_UNITS_TO_CLKS(1),
	[RC6_TRAILER_P] = RC6_UNITS_TO_CLKS(2),
	[RC6_TRAILER_N] = RC6_UNITS_TO_CLKS(2),
	[RC6_DATA_P]    = RC6_UNITS_TO_CLKS(1),
	[RC6_DATA_N]    = RC6_UNITS_TO_CLKS(1),
};

uint32_t
cir_decode(struct cir_dec_ctx *ctx)
{
	int32_t duration = rc6_durations[ctx->state];
	int32_t epsilon  = duration >> 1;

	/* Subtract the expected pulse with from the sample width. */
	ctx->width -= duration;

	/*
	 * If the duration of this pulse is larger than the remaining time in
	 * the current sample, an error has occurred. Either noise introduced
	 * a short pulse, or the decoder got out of sync. Restart the decoder.
	 */
	if (ctx->width < -epsilon) {
		if (ctx->state != RC6_IDLE)
			debug("RC6 fail %x", ctx->state);
		ctx->state = RC6_IDLE;
		return 0;
	}

	/* Discard the remainder of the sample if less than epsilon remains. */
	if (ctx->width <= epsilon)
		ctx->width = 0;

	switch (ctx->state) {
	case RC6_IDLE:
		if (!ctx->pulse)
			break;
		/* Found a leader mark. Initialize the context. */
		ctx->bits   = 0;
		ctx->buffer = 0;
		ctx->state  = RC6_LEADER_S;
		break;
	case RC6_LEADER_S:
		/* Expect a space. */
		ctx->state = ctx->pulse ? RC6_IDLE : RC6_HEADER_P;
		break;
	case RC6_HEADER_P:
	case RC6_DATA_P:
		ctx->bits++;
		ctx->buffer = ctx->buffer << 1 | ctx->pulse;
		ctx->state++;
		break;
	case RC6_HEADER_N:
		/* This pulse must negate the previous pulse. */
		if (ctx->pulse == (ctx->buffer & 1)) {
			ctx->state = RC6_IDLE;
		} else if (ctx->bits == NUM_HEADER_BITS) {
			/* Reinitialize the buffer for decoding data. */
			ctx->bits   = 0;
			ctx->buffer = 0;
			ctx->state  = RC6_TRAILER_P;
		} else {
			ctx->state = RC6_HEADER_P;
		}
		break;
	case RC6_TRAILER_P:
	case RC6_TRAILER_N:
		ctx->state++;
		break;
	case RC6_DATA_N:
		/* This pulse must negate the previous pulse. */
		if (ctx->pulse == (ctx->buffer & 1)) {
			ctx->state = RC6_IDLE;
		} else if (ctx->bits == NUM_DATA_BITS) {
			ctx->state = RC6_IDLE;
			/* Return the scan code minus the toggle bit. */
			debug("RC6 code %08x", ctx->buffer);
			return ctx->buffer & ~BIT(15);
		} else {
			ctx->state = RC6_DATA_P;
		}
		break;
	default:
		unreachable();
	}

	return 0;
}
