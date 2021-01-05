/*
 * Copyright © 2017-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef LIB_DIVISION_H
#define LIB_DIVISION_H

#include <stdint.h>

#define UDIV_ROUND(dividend, divisor) \
	(((dividend) + (divisor) / 2) / (divisor))

/**
 * Perform correctly-rounded unsigned division.
 */
static inline uint32_t
udiv_round(uint32_t dividend, uint32_t divisor)
{
	return (dividend + divisor / 2) / divisor;
}

/**
 * Perform unsigned division.
 *
 * This function replaces the dividend with the quotient and returns the
 * remainder.
 */
uint32_t udivmod(uint32_t *dividend, uint32_t divisor);

#endif /* LIB_DIVISION_H */
