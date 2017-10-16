/*
 * Copyright © 2017 Samuel Holland <samuel@sholland.org>
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef UTIL_H
#define UTIL_H

#ifdef __ASSEMBLER__
#define U(n)   (n)
#else
#define U(n)   (n ## U)
#endif

#define BIT(n) (U(1) << (n))

#endif /* UTIL_H */
