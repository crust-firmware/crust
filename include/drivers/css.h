/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef COMMON_CSS_H
#define COMMON_CSS_H

#include <stdint.h>

/**
 * Get the number of clusters in the compute subsystem.
 *
 * The number returned cannot be greater than 8.
 */
uint32_t css_get_cluster_count(void) ATTRIBUTE(const);

/**
 * Get the state of a cluster and the cores it contains.
 *
 * The state of the cluster is returned in cluster_state.
 *
 * A bitmap representing the state of each core in the cluster is returned in
 * online_cores. A zero bit indicates that a core is completely off (it has no
 * execution context). Any other state is represented by a set bit.
 *
 * @param cluster       The index of the cluster.
 * @param cluster_state Where to store the cluster state.
 * @param online_cores  Where to store the bitmap of online cores.
 * @return              An SCPI success or error status.
 */
int css_get_power_state(uint32_t cluster, uint32_t *cluster_state,
                        uint32_t *online_cores);

/**
 * Initialize the CSS driver, assuming the CSS is already running. Since the
 * firmware starts after the CSS, the driver may need to synchronize its state
 * with the actual state of the hardware.
 */
void css_init(void);

/**
 * Set the state of a CPU core and its ancestor power domains. There are no
 * restrictions on the requested power states; the best available power state
 * will be computed for each power domain.
 *
 * @param cluster       The index of the cluster.
 * @param core          The index of the core within the cluster.
 * @param core_state    The requested power state for the core.
 * @param cluster_state The requested power state for the core's cluster.
 * @param css_state     The requested power state for the CSS.
 * @return              An SCPI success or error status.
 */
int css_set_power_state(uint32_t cluster, uint32_t core, uint32_t core_state,
                        uint32_t cluster_state, uint32_t css_state);

#endif /* COMMON_CSS_H */
