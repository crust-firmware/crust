/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <css.h>
#include <debug.h>
#include <scpi_protocol.h>
#include <stdint.h>
#include <util.h>

#include "css.h"

/*
 * These generic functions provide a safe default implementation of the CSS
 * API, so platforms may implement power domain control functions
 * incrementally. Platforms should implement the getters before the setters.
 */

/**
 * Generic implementation using the platform-provided constants.
 */
uint32_t WEAK
css_get_cluster_count(void)
{
	return MAX_CLUSTERS;
}

/**
 * Generic implementation using the platform-provided constants.
 * Assume the same number of cores is present in each cluster.
 */
uint32_t WEAK
css_get_core_count(uint32_t cluster UNUSED)
{
	return MAX_CORES_PER_CLUSTER;
}

int
css_get_power_state(uint32_t cluster, uint32_t *cluster_state,
                    uint32_t *online_cores)
{
	uint32_t mask = 0;

	if (cluster >= css_get_cluster_count())
		return SCPI_E_PARAM;

	*cluster_state = power_state.cluster[cluster];

	for (uint32_t core = 0; core < MAX_CORES_PER_CLUSTER; ++core) {
		if (power_state.core[cluster][core] != SCPI_CSS_OFF)
			mask |= BIT(core);
	}
	*online_cores = mask;

	return SCPI_OK;
}

/**
 * Generic implementation used when no platform support is available.
 */
void WEAK
css_init(void)
{
}

/**
 * Generic implementation used when no platform support is available. Because
 * the generic code does not know how to control the hardware, prohibit changes
 * to the CSS state by default without a platform-specific implementation.
 */
int WEAK
css_set_css_state(uint32_t state UNUSED)
{
	/* Reject any attempts to change CSS, cluster, or core power states. */
	return SCPI_E_SUPPORT;
}

/**
 * Generic implementation used when no platform support is available. Because
 * the generic code does not know how to control the hardware, prohibit changes
 * to the cluster state by default without a platform-specific implementation.
 */
int WEAK
css_set_cluster_state(uint32_t cluster UNUSED, uint32_t state UNUSED)
{
	/* Reject any attempts to change CSS, cluster, or core power states. */
	return SCPI_E_SUPPORT;
}

/**
 * Generic implementation used when no platform support is available. Because
 * the generic code does not know how to control the hardware, prohibit changes
 * to the core state by default without a platform-specific implementation.
 */
int WEAK
css_set_core_state(uint32_t cluster UNUSED, uint32_t core UNUSED,
                   uint32_t state UNUSED)
{
	/* Reject any attempts to change CSS, cluster, or core power states. */
	return SCPI_E_SUPPORT;
}

int
css_set_power_state(uint32_t cluster, uint32_t core, uint32_t core_state,
                    uint32_t cluster_state, uint32_t css_state)
{
	uint32_t core_old, cluster_old, css_old;
	uint8_t *core_ps, *cluster_ps, *css_ps;
	int err;

	if (cluster >= css_get_cluster_count())
		return SCPI_E_PARAM;
	if (core >= css_get_core_count(cluster))
		return SCPI_E_PARAM;

	core_ps    = &power_state.core[cluster][core];
	cluster_ps = &power_state.cluster[cluster];
	css_ps     = &power_state.css;

	core_old    = *core_ps;
	*core_ps    = core_state;
	cluster_old = *cluster_ps;
	*cluster_ps = cluster_state;
	css_old     = *css_ps;
	*css_ps     = css_state;

	if (css_state < css_old &&
	    (err = css_set_css_state(css_state)))
		return err;
	if (cluster_state < cluster_old &&
	    (err = css_set_cluster_state(cluster, cluster_state)))
		return err;
	if (core_state != core_old &&
	    (err = css_set_core_state(cluster, core, core_state)))
		return err;
	if (cluster_state > cluster_old &&
	    (err = css_set_cluster_state(cluster, cluster_state)))
		return err;
	if (css_state > css_old &&
	    (err = css_set_css_state(css_state)))
		return err;

	return SCPI_OK;
}
