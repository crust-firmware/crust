/*
 * Copyright © 2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <regulator_list.h>
#include <stddef.h>
#include <regulator/axp803.h>
#include <regulator/axp805.h>
#include <regulator/gpio.h>
#include <regulator/sy8106a.h>

#if CONFIG(REGULATOR_AXP803)

const struct regulator_handle cpu_supply = {
	.dev = &axp803_regulator.dev,
	.id  = AXP803_REGL_DCDC2,
};

#elif CONFIG(REGULATOR_AXP805)

const struct regulator_handle cpu_supply = {
	.dev = &axp805_regulator.dev,
	.id  = AXP805_REGL_DCDCA,
};

#elif CONFIG(REGULATOR_SY8106A)

const struct regulator_handle cpu_supply = {
	.dev = &sy8106a.dev,
	.id  = SY8106A_REGL_VOUT,
};

#elif CONFIG(REGULATOR_GPIO_CPU)

const struct regulator_handle cpu_supply = {
	.dev = &gpio_cpu_regulator.dev,
};

#else

const struct regulator_handle cpu_supply = {
	.dev = NULL,
};

#endif

#if CONFIG(REGULATOR_AXP803)

const struct regulator_handle dram_supply = {
	.dev = &axp803_regulator.dev,
	.id  = AXP803_REGL_DCDC5,
};

#elif CONFIG(REGULATOR_AXP805)

const struct regulator_handle dram_supply = {
	.dev = &axp805_regulator.dev,
	.id  = AXP805_REGL_DCDCE,
};

#elif CONFIG(REGULATOR_GPIO_DRAM)

const struct regulator_handle dram_supply = {
	.dev = &gpio_dram_regulator.dev,
};

#else

const struct regulator_handle dram_supply = {
	.dev = NULL,
};

#endif
