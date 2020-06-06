/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <platform/memory.h>

#if CONFIG(COMPILE_TEST)
#undef  SRAM_A2_LIMIT
#define SRAM_A2_LIMIT 0xffffffff
#endif

OUTPUT_ARCH(or1k)
OUTPUT_FORMAT(elf32-or1k)

ENTRY (start)

MEMORY {
  SRAM_A2 (rwx): ORIGIN = SRAM_A2_BASE, LENGTH = SRAM_A2_SIZE
}

SECTIONS
{
  ASSERT(FIRMWARE_BASE >= SRAM_A2_BASE, "Firmware must be placed in SRAM A2")

  . = FIRMWARE_BASE;

  .text . : ALIGN(4) {
    KEEP(*(.text.start))
    *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.text*)))
    . = ALIGN(4);
  } >SRAM_A2

  .rodata . : ALIGN(4) {
    *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
    . = ALIGN(4);
  } >SRAM_A2

  .data . : ALIGN(4) {
    *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.data*)))
    . = ALIGN(4);
  } >SRAM_A2

  ASSERT(SIZEOF(.data) == 0, "Global data must be constant or set at runtime")

  .bss . : ALIGN(4) {
    __bss_start = .;
    *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.bss*)))
    . = ALIGN(4);
    __bss_end = .;
  } >SRAM_A2

  __stack_start = .;
  . += STACK_SIZE;
  __stack_end = .;

  ASSERT(. <= FIRMWARE_LIMIT, "Firmware overflows allocated memory area")
  ASSERT(. <= SCPI_MEM_BASE, "Firmware overflows into SCPI shared memory")
  ASSERT(. <= SRAM_A2_LIMIT, "Firmware overflows SRAM A2")

  __scpi_mem = SCPI_MEM_BASE;

  /DISCARD/ : {
    *(.comment*)
    *(.eh_frame_hdr*)
    *(.iplt*)
    *(.note*)
    *(.rela*)
  }
}
