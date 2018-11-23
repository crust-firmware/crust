/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef LIB_MATH_H
#define LIB_MATH_H

/**
 * Count the number of set bits in an integer.
 */
#define popcount(x) __builtin_popcount(x)

#endif /* LIB_MATH_H */
