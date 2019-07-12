/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <kconfig.h>
#include <clock/sunxi-ccu.h>
#include <dvfs/cpux.h>
#include <misc/gpio-button.h>
#include <msgbox/sunxi-msgbox.h>
#include <pmic/axp803.h>
#include <pmic/dummy.h>
#include <watchdog/sunxi-twd.h>

/* format off -- IF_ENABLED_INIT includes a hidden comma */
struct device *const device_list[] = {

	/* Devices used internally by the firmware */
	&r_twd,
	&msgbox,

	/* PMICs */
	IF_ENABLED_INIT(CONFIG_PMIC_AXP803, &axp803_pmic.dev)
	IF_ENABLED_INIT(CONFIG_PMIC_DUMMY, &dummy_pmic)

	/* Wakeup sources */
	IF_ENABLED_INIT(CONFIG_GPIO_BUTTON, &power_button.dev)

	/* SCPI DVFS providers */
	IF_ENABLED_INIT(CONFIG_DVFS, &cpux.dev)

	/* SCPI clock providers */
	&ccu.dev,
	&r_ccu.dev,

	/* Sentinel */
	0,

};
