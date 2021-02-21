/*
 * Copyright © 2020-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <stdint.h>

#include "css.h"

/*
 * Generic implementation using the platform-provided constants.
 */
uint32_t WEAK
css_get_cluster_count(void)
{
	return MAX_CLUSTERS;
}

uint32_t WEAK
css_get_core_count(uint32_t cluster UNUSED)
{
	/* Assume each cluster contains the same number of cores. */
	return MAX_CORES_PER_CLUSTER;
}

/*
 * Generic implementation used when no platform customization is needed.
 */
void WEAK
css_set_css_state(uint32_t state UNUSED)
{
}

void WEAK
css_set_cluster_state(uint32_t cluster UNUSED, uint32_t state UNUSED)
{
}

void WEAK
css_set_core_state(uint32_t cluster UNUSED, uint32_t core UNUSED,
                   uint32_t state UNUSED)
{
}

void WEAK
css_init(void)
{
}
