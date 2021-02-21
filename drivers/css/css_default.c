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
css_suspend_css(uint32_t new_state UNUSED)
{
}

void WEAK
css_resume_css(uint32_t old_state UNUSED)
{
}

void WEAK
css_suspend_cluster(uint32_t cluster UNUSED, uint32_t new_state UNUSED)
{
}

void WEAK
css_resume_cluster(uint32_t cluster UNUSED, uint32_t old_state UNUSED)
{
}

void WEAK
css_suspend_core(uint32_t cluster UNUSED, uint32_t core UNUSED,
                 uint32_t new_state UNUSED)
{
}

void WEAK
css_resume_core(uint32_t cluster UNUSED, uint32_t core UNUSED,
                uint32_t old_state UNUSED)
{
}

void WEAK
css_init(void)
{
}
