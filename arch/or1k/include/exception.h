/*
 * Copyright © 2017-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <stdint.h>

/**
 * Report the exception that caused the firmware to restart, if applicable.
 *
 * @param exception Exception information provided by startup assembly code.
 */
void report_exception(uint32_t exception);

#endif /* EXCEPTION_H */
