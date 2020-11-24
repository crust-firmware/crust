/*
 * Copyright © 2019-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef CSS_PRIVATE_H
#define CSS_PRIVATE_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Get the state of the compute subsystem (CSS).
 */
uint32_t css_get_css_state(void);

/**
 * Get the state of a cluster.
 *
 * @param cluster The index of the cluster.
 */
uint32_t css_get_cluster_state(uint32_t cluster);

/**
 * Get the number of cores present in a cluster.
 *
 * The number returned cannot be greater than 8.
 *
 * @param cluster The index of the cluster.
 */
uint32_t css_get_core_count(uint32_t cluster) ATTRIBUTE(const);

/**
 * Get the state of a CPU core.
 *
 * @param cluster The index of the cluster.
 * @param core    The index of the core within the cluster.
 */
uint32_t css_get_core_state(uint32_t cluster, uint32_t core);

/**
 * Set the state of the compute subsystem (CSS). This state must not be
 * numbered higher than the lowest cluster state in the CSS.
 *
 * @param state The coordinated requested state for the CSS.
 */
int css_set_css_state(uint32_t state);

/**
 * Set the state of a cluster. This state must not be numbered lower than the
 * CSS state, nor higher than the lowest core state for this cluster.
 *
 * @param cluster The index of the cluster.
 * @param state   The coordinated requested state for the cluster.
 */
int css_set_cluster_state(uint32_t cluster, uint32_t state);

/**
 * Set the state of a CPU core. This state must not be numbered lower than the
 * core's cluster state.
 *
 * @param cluster The index of the cluster.
 * @param core    The index of the core within the cluster.
 * @param state   The coordinated requested state for the CPU core.
 */
int css_set_core_state(uint32_t cluster, uint32_t core, uint32_t state);

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
