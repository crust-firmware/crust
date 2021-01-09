/*
 * Copyright © 2020-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef COMMON_REGULATOR_LIST_H
#define COMMON_REGULATOR_LIST_H

#include <regulator.h>

/**
 * The regulator supplying VDD-CPUX.
 */
extern const struct regulator_handle cpu_supply;

/**
 * The regulator supplying VCC-DRAM.
 */
extern const struct regulator_handle dram_supply;

/**
 * The regulator supplying VDD-SYS.
 */
extern const struct regulator_handle vdd_sys_supply;

#endif /* COMMON_REGULATOR_LIST_H */
