/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <compiler.h>
#include <css.h>
#include <debug.h>
#include <delay.h>
#include <dm.h>
#include <error.h>
#include <pmic.h>
#include <scpi_protocol.h>
#include <stdbool.h>
#include <stddef.h>
#include <system_power.h>
#include <watchdog.h>
#include <watchdog/sunxi-twd.h>

static bool is_off;
static bool is_suspended;

bool __pure
system_is_off(void)
{
	return is_off;
}

bool __pure
system_is_suspended(void)
{
	return is_suspended;
}

noreturn void
system_reset(void)
{
	pmic_reset(pmic);

	watchdog_disable(&r_twd.dev);
	watchdog_enable(&r_twd.dev, 0);
	/* This is always at least one reference clock cycle. */
	udelay(1);

	panic("Failed to reset system");
}

void
system_shutdown(void)
{
	pmic_shutdown(pmic);

	/* We didn't actually shut down. Wait for a wakeup event and reset. */
	is_off = true;
}

void
system_suspend(void)
{
	/* If the system is already suspended, do nothing. */
	if (is_suspended)
		return;

	/* Mark the system as being suspended. */
	is_suspended = true;

	pmic_suspend(pmic);
}

void
system_wakeup(void)
{
	/* Tried to wake up from a fake "off" state. Reset the system. */
	if (is_off)
		system_reset();
	/* If the system is already awake, do nothing. */
	if (!is_suspended)
		return;

	/* Mark the system as no longer being suspended. */
	is_suspended = false;

	/* Tell the PMIC to turn everything back on. */
	pmic_wakeup(pmic);

	/* Resume execution on the CSS. */
	css_set_css_state(SCPI_CSS_ON);
	css_set_cluster_state(0, SCPI_CSS_ON);
	css_set_core_state(0, 0, SCPI_CSS_ON);
}
