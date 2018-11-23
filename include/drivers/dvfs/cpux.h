/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_DVFS_CPUX_H
#define DRIVERS_DVFS_CPUX_H

#include <dvfs.h>

enum {
	CPUX_DVFS_DOMAIN,
	CPUX_DVFS_DOMAIN_COUNT,
};

extern const struct dvfs_driver cpux_driver;

#endif /* DRIVERS_DVFS_CPUX_H */
