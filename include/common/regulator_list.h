/*
 * Copyright © 2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef COMMON_REGULATOR_LIST_H
#define COMMON_REGULATOR_LIST_H

#include <regulator.h>

/**
 * The list of regulators to disable while the system is inactive (suspended).
 */
extern const struct regulator_list inactive_list;

/**
 * The list of regulators to disable while system power is off.
 */
extern const struct regulator_list off_list;

#endif /* COMMON_REGULATOR_LIST_H */
