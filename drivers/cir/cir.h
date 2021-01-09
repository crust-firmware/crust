/*
 * Copyright © 2020-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef CIR_PRIVATE_H
#define CIR_PRIVATE_H

#include <stdint.h>

/**
 * Context for CIR pulse decoding.
 */
struct cir_dec_ctx {
	uint32_t buffer; /*<< Buffer to store a partial scancode. */
	uint8_t  bits;   /*<< Number of bits decoded. */
	uint8_t  state;  /*<< Internal decoder state. */
	uint8_t  pulse;  /*<< Current pulse type (mark or space). */
	int8_t   width;  /*<< Current pulse width in clock cycles. */
};

/**
 * Decode a CIR pulse sequence.
 *
 * The pulse flag and width must be valid each time this function is called.
 * Each call will decode a single pulse, so decoding a complete scancode
 * requires many calls. All but the last in the sequence will return zero.
 *
 * If an error occurs, decoding restarts, but the error is not reported.
 *
 * @return A successfully decoded scancode, or zero.
 */
uint32_t cir_decode(struct cir_dec_ctx *ctx);

#endif /* CIR_PRIVATE_H */
