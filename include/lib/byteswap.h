/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef LIB_BYTESWAP_H
#define LIB_BYTESWAP_H

#include <stdint.h>

static inline uint16_t
bswap16(uint16_t n)
{
	return ((n << 8) & 0xff00U) | \
	       ((n >> 8) & 0xffU);
}

static inline uint32_t
bswap32(uint32_t n)
{
	return ((n << 24) & 0xff000000U) | \
	       ((n << 8) & 0xff0000U) | \
	       ((n >> 8) & 0xff00U) | \
	       ((n >> 24) & 0xffU);
}

#endif /* LIB_BYTESWAP_H */
