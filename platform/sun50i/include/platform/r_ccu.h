/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef PLATFORM_R_CCU_H
#define PLATFORM_R_CCU_H

#include <bitmap.h>
#include <stdint.h>

#define R_CCU_CLOCK_OSC24M    0
#define R_CCU_CLOCK_OSC32K    1
#define R_CCU_CLOCK_OSC16M    2
#define R_CCU_CLOCK_AHB0      3
#define R_CCU_CLOCK_APB0      4
#define R_CCU_CLOCK_R_PIO     5
#define R_CCU_CLOCK_R_CIR     6
#define R_CCU_CLOCK_R_TIMER   7
#define R_CCU_CLOCK_R_UART    8
#define R_CCU_CLOCK_R_I2C     9
#define R_CCU_CLOCK_R_TWD     10
#define R_CCU_CLOCK_R_CIR_MOD 11
#define R_CCU_CLOCK_COUNT     12

#define R_CCU_CLOCK_AHB0_REG  0x0000
#define R_CCU_CLOCK_APB0_REG  0x000c
#define R_CCU_CLOCK_R_CIR_REG 0x0054

#define R_CCU_GATE_BASE       (0x0028 / sizeof(uint32_t))

#define R_CCU_GATE_R_PIO      BITMAP_INDEX(R_CCU_GATE_BASE + 0, 0)
#define R_CCU_GATE_R_CIR      BITMAP_INDEX(R_CCU_GATE_BASE + 0, 1)
#define R_CCU_GATE_R_TIMER    BITMAP_INDEX(R_CCU_GATE_BASE + 0, 2)
#define R_CCU_GATE_R_UART     BITMAP_INDEX(R_CCU_GATE_BASE + 0, 4)
#define R_CCU_GATE_R_I2C      BITMAP_INDEX(R_CCU_GATE_BASE + 0, 6)
#define R_CCU_GATE_R_TWD      BITMAP_INDEX(R_CCU_GATE_BASE + 0, 7)

#define R_CCU_RESET_BASE      (0x00b0 / sizeof(uint32_t))

#define R_CCU_RESET_R_CIR     BITMAP_INDEX(R_CCU_RESET_BASE + 0, 1)
#define R_CCU_RESET_R_TIMER   BITMAP_INDEX(R_CCU_RESET_BASE + 0, 2)
#define R_CCU_RESET_R_UART    BITMAP_INDEX(R_CCU_RESET_BASE + 0, 4)
#define R_CCU_RESET_R_I2C     BITMAP_INDEX(R_CCU_RESET_BASE + 0, 6)

#endif /* PLATFORM_R_CCU_H */
