/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <platform/memory.h>

#if CONFIG(COMPILE_TEST)
#undef  SRAM_A2_SIZE
#define SRAM_A2_SIZE 0xffffffff
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
  ASSERT(FIRMWARE_LIMIT <= SRAM_A2_LIMIT, "Firmware must fit inside SRAM A2")

  /*
   * If addresses within the firmware differ only in their lower 16 bits, a
   * single l.movhi instruction can be shared by multiple symbol references.
   */
  ASSERT(FIRMWARE_BASE >> 16 == (FIRMWARE_LIMIT - 1) >> 16,
         "Firmware must not cross a 64k address boundary")

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

  /*
   * With few exceptions, global data should be constant or initialized during
   * boot. Before changing this value, verify the firmware can recover from a
   * crash even after the new data is modified.
   */
  ASSERT(SIZEOF(.data) == 0x04, "Changes to .data persist after an exception!")

  .bss . : ALIGN(4) {
    __bss_start = .;
    *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.bss*)))
    . = ALIGN(4);
    __bss_end = .;
  } >SRAM_A2

  __stack_start = .;
  . += STACK_SIZE;
  __stack_end = .;

#if !CONFIG(COMPILE_TEST)
  ASSERT(. <= FIRMWARE_LIMIT, "Firmware overflows allocated memory area")
  ASSERT(. <= SCPI_MEM_BASE, "Firmware overflows into SCPI shared memory")
#endif

  __scpi_mem = SCPI_MEM_BASE;

  /DISCARD/ : {
    *(.comment*)
    *(.eh_frame_hdr*)
    *(.iplt*)
    *(.note*)
    *(.rela*)
  }
}
