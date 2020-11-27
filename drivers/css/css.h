/*
 * Copyright © 2019-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef CSS_PRIVATE_H
#define CSS_PRIVATE_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Enable or disable power to a core or cluster power domain.
 *
 * When enabling a power switch, the power domain will be turned on gradually
 * to minimize inrush current and voltage drops.
 *
 * The mapping of cores/clusters to register addresses is platform-dependent.
 *
 * @param addr    The address of the register controlling the power switch.
 * @param enable  Whether to enable or disable the power switch.
 */
void css_set_power_switch(uintptr_t addr, bool enable);

#endif /* CSS_PRIVATE_H */
