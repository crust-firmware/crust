/*
 * Copyright © 2017-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_REGULATOR_GPIO_H
#define DRIVERS_REGULATOR_GPIO_H

#include <gpio.h>
#include <regulator.h>

struct gpio_regulator {
	struct device      dev;
	struct gpio_handle pin;
};

#if CONFIG(REGULATOR_GPIO_CPU)
extern const struct gpio_regulator gpio_cpu_regulator;
#endif

#if CONFIG(REGULATOR_GPIO_DRAM)
extern const struct gpio_regulator gpio_dram_regulator;
#endif

#if CONFIG(REGULATOR_GPIO_VDD_SYS)
extern const struct gpio_regulator gpio_vdd_sys_regulator;
#endif

#endif /* DRIVERS_REGULATOR_GPIO_H */
