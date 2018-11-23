/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <compiler.h>
#include <css.h>
#include <debug.h>
#include <error.h>
#include <stdint.h>
#include <util.h>

/*
 * These generic functions provide a safe default implementation of the CSS
 * API, so platforms may implement power domain control functions
 * incrementally. Platforms should implement the getters before the setters.
 */

/**
 * Generic implementation used when no platform support is available. Because
 * the generic setters prevent changing the CSS state, the CSS must always be
 * running.
 */
uint8_t __weak
css_get_css_state(void)
{
	/* Assume the CSS is always on. */
	return POWER_STATE_ON;
}

/**
 * Generic implementation used when no platform support is available. Assume
 * the minimum possible number of clusters is present.
 */
uint8_t __const __weak
css_get_cluster_count(void)
{
	/* Assume the CSS contains a single cluster with a single core. */
	return 1;
}

/**
 * Generic implementation used when no platform support is available. Because
 * the generic setters prevent changing any cluster state, the set of running
 * clusters is always equal to the set of clusters initialized by the boot ROM.
 */
uint8_t __weak
css_get_cluster_state(uint8_t cluster __unused)
{
	/* Assume present clusters/cores are always on. */
	if (cluster >= css_get_cluster_count())
		return POWER_STATE_OFF;

	return POWER_STATE_ON;
}

/**
 * Generic implementation used when no platform support is available. Assume
 * the minimum possible number of cores is present in each cluster.
 */
uint8_t __const __weak
css_get_core_count(uint8_t cluster __unused)
{
	/* Assume the CSS contains a single cluster with a single core. */
	return 1;
}

/**
 * Generic implementation used when no platform support is available. Because
 * the generic setters prevent changing any core state, the set of running
 * cores is always equal to the set of cores initialized by the boot ROM.
 */
uint8_t __weak
css_get_core_state(uint8_t cluster, uint8_t core)
{
	/* Assume present clusters/cores are always on. */
	if (cluster >= css_get_cluster_count())
		return POWER_STATE_OFF;
	if (core >= css_get_core_count(cluster))
		return POWER_STATE_OFF;

	return POWER_STATE_ON;
}

/*
 * There should usually be no reason to override this weak definition. It
 * correctly implements the algorithm specified in the CSS API using the
 * lower-level core state API. However, this definition is declared weak for
 * consistency with the other functions and flexibility for future platforms.
 */
uint8_t __weak
css_get_online_cores(uint8_t cluster)
{
	uint8_t cores;
	uint8_t mask = 0;

	assert(cluster < css_get_cluster_count());

	cores = css_get_core_count(cluster);
	for (uint8_t core = 0; core < cores; ++core) {
		if (css_get_core_state(cluster, core) != POWER_STATE_OFF)
			mask |= BIT(core);
	}

	return mask;
}

/**
 * Generic implementation used when no platform support is available. Because
 * the generic code does not know how to control the hardware, prohibit changes
 * to the CSS state by default without a platform-specific implementation.
 */
int __weak
css_set_css_state(uint8_t state)
{
	/* Reject any attempts to change CSS, cluster, or core power states. */
	return state == css_get_css_state() ? SUCCESS : ENOTSUP;
}

/**
 * Generic implementation used when no platform support is available. Because
 * the generic code does not know how to control the hardware, prohibit changes
 * to the cluster state by default without a platform-specific implementation.
 */
int __weak
css_set_cluster_state(uint8_t cluster, uint8_t state)
{
	/* Reject any attempts to change CSS, cluster, or core power states. */
	return state == css_get_cluster_state(cluster) ? SUCCESS : ENOTSUP;
}

/**
 * Generic implementation used when no platform support is available. Because
 * the generic code does not know how to control the hardware, prohibit changes
 * to the core state by default without a platform-specific implementation.
 */
int __weak
css_set_core_state(uint8_t cluster, uint8_t core, uint8_t state)
{
	/* Reject any attempts to change CSS, cluster, or core power states. */
	return state == css_get_core_state(cluster, core) ? SUCCESS : ENOTSUP;
}
