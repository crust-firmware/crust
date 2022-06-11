/*
 * Copyright © 2020-2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <scpi_protocol.h>

#include "css.h"

#define CONCAT(a, b) a ## b

#define DEC_1        0
#define DEC_2        1
#define DEC_3        2
#define DEC_4        3
#define DEC(v)       CONCAT(DEC_, v)

#define REP_0(...)
#define REP_1(...)   __VA_ARGS__
#define REP_2(...)   REP_1(__VA_ARGS__), __VA_ARGS__
#define REP_3(...)   REP_2(__VA_ARGS__), __VA_ARGS__
#define REP_4(...)   REP_3(__VA_ARGS__), __VA_ARGS__
#define REP(n, ...)  CONCAT(REP_, n)(__VA_ARGS__)

struct power_state power_state = {
	.core = {
		{
			SCPI_CSS_ON,
			REP(DEC(MAX_CORES_PER_CLUSTER), SCPI_CSS_OFF)
		},
		REP(DEC(MAX_CLUSTERS), {
			REP(MAX_CORES_PER_CLUSTER, SCPI_CSS_OFF)
		})
	},
	.cluster = {
		SCPI_CSS_ON,
		REP(DEC(MAX_CLUSTERS), SCPI_CSS_OFF)
	},
	.css = SCPI_CSS_ON,
};
