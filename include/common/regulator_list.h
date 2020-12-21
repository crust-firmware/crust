/*
 * Copyright © 2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef COMMON_REGULATOR_LIST_H
#define COMMON_REGULATOR_LIST_H

#include <regulator.h>

/**
 * The regulator supplying VDD-CPUX.
 */
extern const struct regulator_handle cpu_supply;

#endif /* COMMON_REGULATOR_LIST_H */
