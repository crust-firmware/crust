/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_DVFS_CPUX_H
#define DRIVERS_DVFS_CPUX_H

#include <dvfs.h>

enum {
	CPUX_DVFS_DOMAIN,
	CPUX_DVFS_DOMAIN_COUNT,
};

struct cpux {
	struct device dev;
	uint8_t      *state;
};

extern struct cpux cpux;

#endif /* DRIVERS_DVFS_CPUX_H */
