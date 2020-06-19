/*
 * Copyright © 2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <regulator_list.h>
#include <stddef.h>
#include <stdint.h>
#include <util.h>
#include <regulator/axp803.h>
#include <regulator/axp805.h>
#include <regulator/sy8106a.h>

#if CONFIG(REGULATOR_AXP803)

static const uint8_t inactive_ids[] = {
	AXP803_REGL_DCDC2,
	AXP803_REGL_DCDC3,
};

const struct regulator_list inactive_list = {
	.dev    = &axp803_regulator.dev,
	.ids    = inactive_ids,
	.nr_ids = ARRAY_SIZE(inactive_ids),
};

extern const struct regulator_list off_list ATTRIBUTE(alias("inactive_list"));

#elif CONFIG(REGULATOR_SY8106A)

static const uint8_t inactive_ids[] = {
	SY8106A_REGL_VOUT,
};

const struct regulator_list inactive_list = {
	.dev    = &sy8106a.dev,
	.ids    = inactive_ids,
	.nr_ids = ARRAY_SIZE(inactive_ids),
};

extern const struct regulator_list off_list ATTRIBUTE(alias("inactive_list"));

#else

const struct regulator_list inactive_list = {
	.dev = NULL,
};

extern const struct regulator_list off_list ATTRIBUTE(alias("inactive_list"));

#endif
