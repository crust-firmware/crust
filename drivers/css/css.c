/*
 * Copyright © 2017-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <css.h>
#include <scpi_protocol.h>
#include <stdint.h>
#include <system.h>
#include <util.h>

#include "css.h"

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

int
css_set_power_state(uint32_t cluster, uint32_t core, uint32_t core_state,
                    uint32_t cluster_state, uint32_t css_state)
{
	uint32_t core_old, cluster_old, css_old;
	uint8_t *core_ps, *cluster_ps, *css_ps;
	uint8_t *cluster_cores, *css_clusters;

	if (cluster >= css_get_cluster_count())
		return SCPI_E_PARAM;
	if (core >= css_get_core_count(cluster))
		return SCPI_E_PARAM;

	core_ps       = &power_state.core[cluster][core];
	cluster_cores = power_state.core[cluster];
	cluster_ps    = &power_state.cluster[cluster];
	css_clusters  = power_state.cluster;
	css_ps        = &power_state.css;

	core_old = *core_ps;
	*core_ps = core_state;

	/* A cluster must be on if any of its cores is on. */
	for (uint32_t i = 0; i < css_get_core_count(cluster); ++i)
		if (cluster_cores[i] < cluster_state)
			cluster_state = cluster_cores[i];

	cluster_old = *cluster_ps;
	*cluster_ps = cluster_state;

	/* The CSS must be on if any of its clusters is on. */
	for (uint32_t i = 0; i < css_get_cluster_count(); ++i)
		if (css_clusters[i] < css_state)
			css_state = css_clusters[i];

	css_old = *css_ps;
	*css_ps = css_state;

	if (css_state < css_old)
		css_set_css_state(css_state);
	if (cluster_state < cluster_old)
		css_set_cluster_state(cluster, cluster_state);
	if (core_state != core_old)
		css_set_core_state(cluster, core, core_state);
	if (cluster_state > cluster_old)
		css_set_cluster_state(cluster, cluster_state);
	if (css_state > css_old)
		css_set_css_state(css_state);

	/* Suspend the system when powering off the CSS. */
	if (css_state == SCPI_CSS_OFF)
		system_suspend();

	return SCPI_OK;
}
