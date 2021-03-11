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

/* The register layout assumes a maximum of two clusters. */
#define FIQ_BIT BIT(2 * MAX_CORES_PER_CLUSTER)
#define IRQ_BIT BIT(0)

static uint8_t lead_cluster, lead_core;

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
	uint8_t *core_ps    = &power_state.core[cluster][core];
	uint8_t *cluster_ps = &power_state.cluster[cluster];
	uint8_t *css_ps     = &power_state.css;

	if (cluster >= css_get_cluster_count())
		return SCPI_E_PARAM;
	if (core >= css_get_core_count(cluster))
		return SCPI_E_PARAM;

	/*
	 * This implementation takes advantage of two restrictions on power
	 * state requests:
	 *
	 *   1. A request to suspend a core may only be sent from that core.
	 *      Therefore, at the time any such request is received, it is safe
	 *      to assume that the core and all of its ancestor power domains
	 *      are in the "on" state.
	 *
	 *   2. No power domain may be in a deeper power state than any of its
	 *      children. Therefore, any request to turn on a core must also
	 *      turn on all of its ancestor power domains, regardless of their
	 *      previous or requested states.
	 */
	if (core_state != SCPI_CSS_ON) {
		uint8_t *cluster_cores = power_state.core[cluster];
		uint8_t *css_clusters  = power_state.cluster;

		css_suspend_core(cluster, core, core_state);
		*core_ps = core_state;

		/* A cluster must be on if any of its cores is on. */
		for (uint32_t i = 0; i < css_get_core_count(cluster); ++i) {
			if (cluster_cores[i] < cluster_state)
				cluster_state = cluster_cores[i];
		}
		css_suspend_cluster(cluster, cluster_state);
		*cluster_ps = cluster_state;

		/* The CSS must be on if any of its clusters is on. */
		for (uint32_t i = 0; i < css_get_cluster_count(); ++i) {
			if (css_clusters[i] < css_state)
				css_state = css_clusters[i];
		}
		css_suspend_css(css_state);
		*css_ps = css_state;

		/* Suspend the system when powering off the CSS. */
		if (css_state == SCPI_CSS_OFF) {
			system_suspend();

			/* Remember the last active core. */
			lead_cluster = cluster;
			lead_core    = core;
		}
	} else {
		css_resume_css(*css_ps);
		*css_ps = SCPI_CSS_ON;

		css_resume_cluster(cluster, *cluster_ps);
		*cluster_ps = SCPI_CSS_ON;

		css_resume_core(cluster, core, *core_ps);
		*core_ps = SCPI_CSS_ON;
	}

	return SCPI_OK;
}

static void
css_wake_one_cpu(uint32_t cluster, uint32_t core)
{
	css_set_power_state(cluster, core,
	                    SCPI_CSS_ON, SCPI_CSS_ON, SCPI_CSS_ON);
}

void
css_resume(void)
{
	css_wake_one_cpu(lead_cluster, lead_core);
}

void
css_poll(void)
{
	uint32_t status = css_get_irq_status();

	for (uint32_t i = 0; i < css_get_cluster_count(); ++i) {
		/* Assume each cluster is allocated the same number of bits. */
		for (uint32_t j = 0; j < MAX_CORES_PER_CLUSTER; ++j) {
			if ((status & (FIQ_BIT | IRQ_BIT)) &&
			    (power_state.core[i][j] == SCPI_CSS_OFF))
				css_wake_one_cpu(i, j);
			/* Shift the next core status on top of the mask. */
			status >>= 1;
		}
	}
}
