/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef LIB_MATH_H
#define LIB_MATH_H

/**
 * Count the number of set bits in an integer.
 */
#define popcount(x) __builtin_popcount(x)

/**
 * Perform unsigned division.
 *
 * This function replaces the dividend with the quotient and returns the
 * remainder.
 */
uint32_t udivmod(uint32_t *dividend, uint32_t divisor);

#endif /* LIB_MATH_H */
