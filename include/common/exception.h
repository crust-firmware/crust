/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef COMMON_EXCEPTION_H
#define COMMON_EXCEPTION_H

#include <compiler.h>
#include <stdint.h>
#include <arch/exception.h>

void handle_exception(uint32_t number);

#endif /* COMMON_EXCEPTION_H */
