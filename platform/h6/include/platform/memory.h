/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef PLATFORM_MEMORY_H
#define PLATFORM_MEMORY_H

#define FIRMWARE_BASE  0x00014000
#define FIRMWARE_LIMIT SCPI_MEM_BASE
#define FIRMWARE_SIZE  (FIRMWARE_LIMIT - FIRMWARE_BASE)

#define SCPI_MEM_BASE  (SRAM_A2_LIMIT - 0x400)
#define SCPI_MEM_LIMIT SRAM_A2_LIMIT
#define SCPI_MEM_SIZE  (SCPI_MEM_LIMIT - SCPI_MEM_BASE)

#define SRAM_A2_BASE   0x00000000
#define SRAM_A2_LIMIT  0x00018000
#define SRAM_A2_SIZE   (SRAM_A2_LIMIT - SRAM_A2_BASE)

/* Difference between SRAM_A2_BASE in the AR100 and ARM address spaces. */
#define SRAM_A2_OFFSET 0x00100000

#define STACK_SIZE     0x00000400

#endif /* PLATFORM_MEMORY_H */
