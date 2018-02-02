/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef PLATFORM_R_CCU_H
#define PLATFORM_R_CCU_H

#include <bitmap.h>
#include <stdint.h>

#define R_CCU_CLOCK_R_PIO   0
#define R_CCU_CLOCK_R_CIR   1
#define R_CCU_CLOCK_R_TIMER 2
#define R_CCU_CLOCK_R_UART  3
#define R_CCU_CLOCK_R_I2C   4
#define R_CCU_CLOCK_R_TWD   5

#define R_CCU_GATE_BASE     (0x0028 / sizeof(uint32_t))

#define R_CCU_GATE_R_PIO    BITMAP_INDEX(R_CCU_GATE_BASE + 0, 0)
#define R_CCU_GATE_R_CIR    BITMAP_INDEX(R_CCU_GATE_BASE + 0, 1)
#define R_CCU_GATE_R_TIMER  BITMAP_INDEX(R_CCU_GATE_BASE + 0, 2)
#define R_CCU_GATE_R_UART   BITMAP_INDEX(R_CCU_GATE_BASE + 0, 4)
#define R_CCU_GATE_R_I2C    BITMAP_INDEX(R_CCU_GATE_BASE + 0, 6)
#define R_CCU_GATE_R_TWD    BITMAP_INDEX(R_CCU_GATE_BASE + 0, 7)

#define R_CCU_RESET_BASE    (0x00b0 / sizeof(uint32_t))

#define R_CCU_RESET_R_CIR   BITMAP_INDEX(R_CCU_RESET_BASE + 0, 1)
#define R_CCU_RESET_R_TIMER BITMAP_INDEX(R_CCU_RESET_BASE + 0, 2)
#define R_CCU_RESET_R_UART  BITMAP_INDEX(R_CCU_RESET_BASE + 0, 4)
#define R_CCU_RESET_R_I2C   BITMAP_INDEX(R_CCU_RESET_BASE + 0, 6)

#endif /* PLATFORM_R_CCU_H */
