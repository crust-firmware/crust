/*
 * Copyright © 2020-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <regulator_list.h>
#include <stddef.h>
#include <regulator/axp803.h>
#include <regulator/axp805.h>
#include <regulator/gpio.h>
#include <regulator/sy8106a.h>

const struct regulator_handle cpu_supply = {
#if CONFIG(REGULATOR_AXP803)
	.dev = &axp803_regulator.dev,
	.id  = AXP803_REGL_DCDC2,
#elif CONFIG(REGULATOR_AXP805)
	.dev = &axp805_regulator.dev,
	.id  = AXP805_REGL_DCDCA,
#elif CONFIG(REGULATOR_SY8106A)
	.dev = &sy8106a.dev,
#elif CONFIG(REGULATOR_GPIO_CPU)
	.dev = &gpio_cpu_regulator.dev,
#else
	.dev = NULL,
#endif
};

const struct regulator_handle dram_supply = {
#if CONFIG(REGULATOR_AXP803)
	.dev = &axp803_regulator.dev,
	.id  = AXP803_REGL_DCDC5,
#elif CONFIG(REGULATOR_AXP805)
	.dev = &axp805_regulator.dev,
	.id  = AXP805_REGL_DCDCE,
#elif CONFIG(REGULATOR_GPIO_DRAM)
	.dev = &gpio_dram_regulator.dev,
#else
	.dev = NULL,
#endif
};

const struct regulator_handle vcc_pll_supply = {
#if CONFIG(REGULATOR_AXP803)
	.dev = &axp803_regulator.dev,
	.id  = AXP803_REGL_ALDO3,
#elif CONFIG(REGULATOR_AXP805)
	.dev = &axp805_regulator.dev,
	.id  = AXP805_REGL_BLDO1,
#elif CONFIG(REGULATOR_GPIO_VCC_PLL)
	.dev = &gpio_vcc_pll_regulator.dev,
#else
	.dev = NULL,
#endif
};

const struct regulator_handle vdd_sys_supply = {
#if CONFIG(REGULATOR_AXP803)
	.dev = &axp803_regulator.dev,
	.id  = AXP803_REGL_DCDC6,
#elif CONFIG(REGULATOR_AXP805)
	.dev = &axp805_regulator.dev,
	.id  = AXP805_REGL_DCDCD,
#elif CONFIG(REGULATOR_GPIO_VDD_SYS)
	.dev = &gpio_vdd_sys_regulator.dev,
#else
	.dev = NULL,
#endif
};
