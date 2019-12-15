/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <pmic.h>
#include <pmic/axp803.h>
#include <pmic/dummy.h>

const struct device *pmic;

void
pmic_detect(void)
{
	if (IS_ENABLED(CONFIG_PMIC_AXP803))
		pmic = &axp803_pmic.dev;
	else
		pmic = &dummy_pmic.dev;
}
