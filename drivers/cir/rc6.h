/*
 * Copyright © 2020-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef RC6_PRIVATE_H
#define RC6_PRIVATE_H

#include <stdint.h>

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

struct rc6_ctx {
	const int8_t *durations;
	uint32_t      buffer;
	uint8_t       bits;
	uint8_t       state;
	uint8_t       pulse;
	int8_t        width;
};

/**
 * Decode an RC6 pulse sequence.
 *
 * The pulse flag and width must be valid each time this function is called.
 * Each call will decode a single pulse, so decoding a complete scancode
 * requires many calls. All but the last in the sequence will return zero.
 *
 * If an error occurs, decoding restarts, but the error is not reported.
 *
 * @return A successfully decoded scancode, or zero.
 */
uint32_t rc6_decode(struct rc6_ctx *ctx);

#endif /* RC6_PRIVATE_H */
