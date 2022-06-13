/*
 * Copyright © 2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef PLATFORM_CSS_H
#define PLATFORM_CSS_H

#define MAX_CLUSTERS          1
#if CONFIG(SOC_A23)
#define MAX_CORES_PER_CLUSTER 2
#else
#define MAX_CORES_PER_CLUSTER 4
#endif

#endif /* PLATFORM_CSS_H */
