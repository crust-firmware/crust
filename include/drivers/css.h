/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef COMMON_CSS_H
#define COMMON_CSS_H

#include <stdint.h>

/**
 * Get the state of the compute subsystem (CSS).
 */
uint8_t css_get_css_state(void);

/**
 * Get the number of clusters in the compute subsystem.
 *
 * The number returned cannot be greater than 8.
 */
uint8_t css_get_cluster_count(void) ATTRIBUTE(const);

/**
 * Get the state of a cluster.
 *
 * @param cluster The index of the cluster.
 */
uint8_t css_get_cluster_state(uint8_t cluster);

/**
 * Get the number of cores present in a cluster.
 *
 * The number returned cannot be greater than 8.
 *
 * @param cluster The index of the cluster.
 */
uint8_t css_get_core_count(uint8_t cluster) ATTRIBUTE(const);

/**
 * Get the state of a CPU core.
 *
 * @param cluster The index of the cluster.
 * @param core    The index of the core within the cluster.
 */
uint8_t css_get_core_state(uint8_t cluster, uint8_t core);

/**
 * Get a bitmask of the states of the cores in a cluster. A zero bit indicates
 * that a core is completely off (i.e. it has no execution context, and must be
 * manually woken up). Any other state is represented by a set bit.
 *
 * @param cluster The index of the cluster.
 */
uint8_t css_get_online_cores(uint8_t cluster);

/**
 * Initialize the CSS driver, assuming the CSS is already running. Since the
 * firmware starts after the CSS, the driver may need to synchronize its state
 * with the actual state of the hardware.
 */
void css_init(void);

/**
 * Set the state of the compute subsystem (CSS). This state must not be
 * numbered higher than the lowest cluster state in the CSS.
 *
 * @param state The coordinated requested state for the CSS.
 */
int css_set_css_state(uint8_t state);

/**
 * Set the state of a cluster. This state must not be numbered lower than the
 * CSS state, nor higher than the lowest core state for this cluster.
 *
 * @param cluster The index of the cluster.
 * @param state   The coordinated requested state for the cluster.
 */
int css_set_cluster_state(uint8_t cluster, uint8_t state);

/**
 * Set the state of a CPU core. This state must not be numbered lower than the
 * core's cluster state.
 *
 * @param cluster The index of the cluster.
 * @param core    The index of the core within the cluster.
 * @param state   The coordinated requested state for the CPU core.
 */
int css_set_core_state(uint8_t cluster, uint8_t core, uint8_t state);

#endif /* COMMON_CSS_H */
