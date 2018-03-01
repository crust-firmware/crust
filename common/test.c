/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <debug.h>
#include <test.h>

#if TEST

static void
test_crust_boots(void)
{
	test("Boot: PASS");
}

void
run_tests(void)
{
	test_crust_boots();
}

#endif
