/*
 * Copyright © 2019-2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef CSS_PRIVATE_H
#define CSS_PRIVATE_H

#include <stdbool.h>
#include <stdint.h>
#include <platform/css.h>

struct power_state {
	uint8_t core[MAX_CLUSTERS][MAX_CORES_PER_CLUSTER];
	uint8_t cluster[MAX_CLUSTERS];
	uint8_t css;
};

extern struct power_state power_state;

/**
 * Get the number of cores present in a cluster.
 *
 * The number returned cannot be greater than 8.
 *
 * @param cluster The index of the cluster.
 */
uint32_t css_get_core_count(uint32_t cluster) ATTRIBUTE(const);

/**
 * Get the pending IRQ status for each core in the CSS.
 *
 * Cores with pending IRQs will be woken up.
 *
 * @return Each set bit means some IRQ is pending for that core.
 */
uint32_t css_get_irq_status(void);

/**
 * Suspend the compute subsystem (CSS).
 *
 * This function assumes that the previous CSS power state was "on".
 *
 * @param new_state The new coordinated power state for the CSS.
 */
void css_suspend_css(uint32_t new_state);

/**
 * Prepare the compute subsystem (CSS) to resume execution.
 *
 * @param old_state The previous coordinated power state for the CSS.
 */
void css_resume_css(uint32_t old_state);

/**
 * Suspend a cluster.
 *
 * This function assumes that the previous cluster power state was "on".
 *
 * @param cluster   The index of the cluster.
 * @param new_state The new coordinated power state for this cluster.
 */
void css_suspend_cluster(uint32_t cluster, uint32_t new_state);

/**
 * Prepare a cluster to resume execution.
 *
 * @param cluster   The index of the cluster.
 * @param old_state The previous coordinated power state for this cluster.
 */
void css_resume_cluster(uint32_t cluster, uint32_t old_state);

/**
 * Suspend a core.
 *
 * This function assumes that the previous core power state was "on".
 *
 * @param cluster   The index of the cluster.
 * @param core      The index of the core within the cluster.
 * @param new_state The new coordinated power state for this core.
 */
void css_suspend_core(uint32_t cluster, uint32_t core, uint32_t new_state);

/**
 * Begin or resume execution on a core.
 *
 * @param cluster   The index of the cluster.
 * @param core      The index of the core within the cluster.
 * @param old_state The previous coordinated power state for this core.
 */
void css_resume_core(uint32_t cluster, uint32_t core, uint32_t old_state);

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
