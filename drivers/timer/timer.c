/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <compiler.h>
#include <dm.h>
#include <error.h>
#include <stddef.h>
#include <drivers/timer.h>
#include <drivers/wallclock.h>

/* One second at a reference clock rate of 24MHz. */
#define TICK_INTERVAL 24000000

static uint64_t last_tick;
static struct device *timer;

int
timer_cancel_periodic(work_function fn __unused, void *param __unused)
{
	return ENOTSUP;
}

int
timer_device_register(struct device *dev)
{
	if (timer != NULL)
		return EEXIST;

	timer = dev;

	return SUCCESS;
}

int
timer_get_timeout(uint32_t *timeout)
{
	if (timer == NULL)
		return ENODEV;

	return TIMER_OPS(timer)->get_timeout(timer, timeout);
}

int
timer_refresh(void)
{
	uint64_t current_time = wallclock_read();

	if (unlikely(timer == NULL))
		return ENODEV;

	if (unlikely(last_tick == 0))
		last_tick = current_time;
	while (last_tick <= current_time)
		last_tick += TICK_INTERVAL;

	return TIMER_OPS(timer)->set_timeout(timer, last_tick - current_time);
}

int
timer_run_delayed(work_function fn __unused, void *param __unused,
                  uint32_t delay __unused)
{
	return ENOTSUP;
}

int
timer_run_periodic(work_function fn __unused, void *param __unused)
{
	return ENOTSUP;
}

void
timer_tick(void)
{
	timer_refresh();
}
