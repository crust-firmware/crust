/*
 * Copyright © 2017-2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef LIB_INTRUSIVE_H
#define LIB_INTRUSIVE_H

#include <stddef.h>
#include <stdint.h>

#define container_of(ptr, type, member) \
	((type *)((char *)(ptr) - offsetof(type, member)))

#endif /* LIB_INTRUSIVE_H */
