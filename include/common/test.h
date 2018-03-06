/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef COMMON_TEST_H
#define COMMON_TEST_H

#if TEST

void run_tests(void);

#else

static inline void
run_tests(void)
{
}

#endif

#endif /* COMMON_TEST_H */
