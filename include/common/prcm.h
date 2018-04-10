/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef COMMON_PRCM_H
#define COMMON_PRCM_H

#include <stdint.h>

/**
 * Intitiate the PRCM shutdown procedure.
 */
int prcm_shutdown(void);

#endif /* COMMON_PRCM_H */
