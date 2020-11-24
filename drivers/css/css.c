/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <css.h>
#include <debug.h>
#include <scpi_protocol.h>
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
uint32_t WEAK
css_get_css_state(void)
{
	/* Assume the CSS is always on. */
	return SCPI_CSS_ON;
}

/**
 * Generic implementation used when no platform support is available. Assume
 * the minimum possible number of clusters is present.
 */
uint32_t WEAK
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
uint32_t WEAK
css_get_cluster_state(uint32_t cluster UNUSED)
{
	/* Assume present clusters/cores are always on. */
	if (cluster >= css_get_cluster_count())
		return SCPI_CSS_OFF;

	return SCPI_CSS_ON;
}

/**
 * Generic implementation used when no platform support is available. Assume
 * the minimum possible number of cores is present in each cluster.
 */
uint32_t WEAK
css_get_core_count(uint32_t cluster UNUSED)
{
	/* Assume the CSS contains a single cluster with a single core. */
	return 1;
}

/**
 * Generic implementation used when no platform support is available. Because
 * the generic setters prevent changing any core state, the set of running
 * cores is always equal to the set of cores initialized by the boot ROM.
 */
uint32_t WEAK
css_get_core_state(uint32_t cluster, uint32_t core)
{
	/* Assume present clusters/cores are always on. */
	if (cluster >= css_get_cluster_count())
		return SCPI_CSS_OFF;
	if (core >= css_get_core_count(cluster))
		return SCPI_CSS_OFF;

	return SCPI_CSS_ON;
}

/*
 * There should usually be no reason to override this weak definition. It
 * correctly implements the algorithm specified in the CSS API using the
 * lower-level core state API. However, this definition is declared weak for
 * consistency with the other functions and flexibility for future platforms.
 */
uint32_t WEAK
css_get_online_cores(uint32_t cluster)
{
	uint32_t cores;
	uint32_t mask = 0;

	assert(cluster < css_get_cluster_count());

	cores = css_get_core_count(cluster);
	for (uint32_t core = 0; core < cores; ++core) {
		if (css_get_core_state(cluster, core) != SCPI_CSS_OFF)
			mask |= BIT(core);
	}

	return mask;
}

/**
 * Generic implementation used when no platform support is available.
 * Since the generic code has no state, no initialization is needed.
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
css_set_css_state(uint32_t state)
{
	/* Reject any attempts to change CSS, cluster, or core power states. */
	if (state != css_get_css_state())
		return SCPI_E_SUPPORT;

	return SCPI_OK;
}

/**
 * Generic implementation used when no platform support is available. Because
 * the generic code does not know how to control the hardware, prohibit changes
 * to the cluster state by default without a platform-specific implementation.
 */
int WEAK
css_set_cluster_state(uint32_t cluster, uint32_t state)
{
	/* Reject any attempts to change CSS, cluster, or core power states. */
	if (state != css_get_cluster_state(cluster))
		return SCPI_E_SUPPORT;

	return SCPI_OK;
}

/**
 * Generic implementation used when no platform support is available. Because
 * the generic code does not know how to control the hardware, prohibit changes
 * to the core state by default without a platform-specific implementation.
 */
int WEAK
css_set_core_state(uint32_t cluster, uint32_t core, uint32_t state)
{
	/* Reject any attempts to change CSS, cluster, or core power states. */
	if (state != css_get_core_state(cluster, core))
		return SCPI_E_SUPPORT;

	return SCPI_OK;
}

int
css_set_power_state(uint32_t cluster, uint32_t core, uint32_t core_state,
                    uint32_t cluster_state, uint32_t css_state)
{
	int err;

	if (css_state == SCPI_CSS_ON &&
	    (err = css_set_css_state(css_state)))
		return err;
	if (cluster_state == SCPI_CSS_ON &&
	    (err = css_set_cluster_state(cluster, cluster_state)))
		return err;
	if ((err = css_set_core_state(cluster, core, core_state)))
		return err;
	if (cluster_state != SCPI_CSS_ON &&
	    (err = css_set_cluster_state(cluster, cluster_state)))
		return err;
	if (css_state != SCPI_CSS_ON &&
	    (err = css_set_css_state(css_state)))
		return err;

	return SCPI_OK;
}
