/*
 * Copyright © 2017-2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_REGULATOR_AXP805_H
#define DRIVERS_REGULATOR_AXP805_H

#include <regulator/axp20x.h>

enum {
	AXP805_REGL_DCDCA,
	AXP805_REGL_DCDCB,
	AXP805_REGL_DCDCC,
	AXP805_REGL_DCDCD,
	AXP805_REGL_DCDCE,
	AXP805_REGL_ALDO1,
	AXP805_REGL_ALDO2,
	AXP805_REGL_ALDO3,
	AXP805_REGL_BLDO1,
	AXP805_REGL_BLDO2,
	AXP805_REGL_BLDO3,
	AXP805_REGL_BLDO4,
	AXP805_REGL_CLDO1,
	AXP805_REGL_CLDO2,
	AXP805_REGL_CLDO3,
	AXP805_REGL_DCSW,
	AXP805_REGL_COUNT,
};

extern const struct axp20x_regulator axp805_regulator;

#endif /* DRIVERS_REGULATOR_AXP805_H */
