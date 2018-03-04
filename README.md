# Crust: Free management firmware for Allwinner SoCs

[![CI status](https://travis-ci.org/crust-firmware/crust.svg?branch=master)][t]

[t]: https://travis-ci.org/crust-firmware/crust

## Overview

Most Allwinner SoCs, including all recent ones, contain an [OpenRISC][or1k] CPU
in addition to their ARM cores. This CPU is in the RTC power domain and is
responsible for system suspend/resume/reset/shutdown, dynamic frequency scaling
for the ARM cores, and controlling the external power management IC (PMIC).

Allwinner provides no source code for their "scp.bin" firmware, other than in
an [encrypted tarball][tar.aes], so [reverse-engineering][sunxi-blobs] their
released binaries was required to understand the hardware interfaces used by
this firmware.

This firmware attempts to completely replace the proprietary firmware provided
by Allwinner. It is designed to be compatible with mainline versions of Linux,
u-boot, and ATF. While initially only developed on and for the `sun50i` series
(A64/H64/H5/R18), ideally it could be backported for use on other SoC
platforms.

**This firmware is currently a work in progress! It is buggy and missing major
functionality!** Moreover, due to sharing SRAM A2 with ATF (explained below),
you likely cannot install it anyway.

[or1k]: http://openrisc.io/
[sunxi-blobs]: https://github.com/smaeul/sunxi-blobs
[tar.aes]: https://github.com/tinalinux/linux-3.10/tree/r18-v0.9/drivers/arisc

## Architecture

This firm is designed to be flexible yet extremely lightweight. It borrows
heavily from ideas in both Linux and ATF for its layout and driver model. The
code is divided into directories based on major function:

- `board`: These files contain configuration for each board supported by this
  firmware. They determine which subdirectory of `platform` is used (for
  SoC-internal devices) and which external devices are enabled.
- `common`: Files in this directory contain the main logic of the firmware, as
  well as glue code for connecting drivers, handling exceptions, etc.
- `drivers`: This directory contains a subdirectory for each class of drivers.
  - `drivers/<class>`: These directories contain all of the drivers of a class,
    as well as generic dispatch code (in `drivers/<class>/<class>.c`).
- `include`: This directory contains headers for code in `common` and `lib`, as
  well as standalone definitions.
  - `include/arch`: These headers expose functionality of the CPU architecture
    that are not dependent on a specific hardware implementation.
  - `include/drivers`: These headers specify the interface for each class of
    drivers. Also included are headers for drivers that share macros with
    platform-specific device definitions.
- `lib`: This directory contains standalone code that does not depend on other
  outside code (with few exceptions). This code should be easily reusable in
  other projects.
- `platform`: This directory contains a subdirectory for each supported
  platform, which refers to a family of SoCs with a similar programming
  interface.
  - `platform/<platform>`: This directory contains source files for device
    definitions and an early console implementation for each platform.
    - `platform/<platform>/include`: This directory contains headers with
      platform-specific macro definitions, such as memory addresses of devices
      and register layouts that change between platforms.
- `scripts`: These files are used to assist in building and linking the
  firmware, but contain no code themselves.
- `tools`: Each file here is a self-contained program intended to be run on the
  host machine (in Linux userspace on the ARM cores) to gather information,
  manipulate hardware, load firmware, or for other reasons.

The build system uses a linker script to coalesce driver and device definitions
together in the final binary, so they can be initialized and probed
iteratively.

## Build prerequisites

Building this firmware requires a cross-compiler targeting the `or1k`
architecture. Prebuilt toolchains can be downloaded from the OpenRISC
organization [on GitHub][or1k-toolchains], or you can manually build one with
[musl-cross-make][musl-cross-make]. If your cross toolchain has a different
tuple, or is not in your `PATH`, edit the top of the `Makefile` to provide
the appropriate full path or prefix.

[musl-cross-make]: https://github.com/smaeul/musl-cross-make
[or1k-toolchains]: https://github.com/openrisc/or1k-gcc/releases

## Supported devices

While this firmware should ideally support any SoC with the ARISC core present,
there are several limiting factors to doing so:
- Changing memory map for device access between SoCs.
- Differences in SRAM area location and size between SOCs (in some cases, SRAM
  may be too small to fit this firmware).
- Different PMICs used with different hardware generations.
- Variations in connection to external voltage regulators, buttons, LEDs, etc.
- Hardware bugs, if any.

Thus, this firmware is designed and implemented with the `sun50i` family (A64
and H5) first in mind, and other SoCs second.

In order to support external devices, the firmware needs to know some
information about the layout of the board and the connections to port L GPIO
pins. Therefore, the supported boards are enumerated in the `boards` directory,
and specifying a board with the `BOARD` `make` variable is required to build
the firmware. You can also specify `BOARD` in a `config.mk` file, that will be
included by the main `Makefile`.

## Running the firmware

The ARISC firmware *must* run from SRAM A2, because
- The OpenRISC CPU has its special exception vector area at the beginning of
  SRAM A2.
- SRAM A2 is the only memory on a bus directly connected to the ARISC core (in
  other words, using any other memory would prevent turning some busses off
  during suspend).
- SRAM A1 is not accessible from the OpenRISC core on some SoCs.

Unfortunately, on `sun50i` SoCs, ATF currently takes up the entirety of SRAM
A2. Fortunately, it can be made smaller. [This version of ATF][crust-atf] has
been patched to put its dynamically-allocated data in SRAM A1. This leaves half
of SRAM A2 for the ARISC firmware. You'll also need [a patched version of
U-Boot][crust-uboot] that loads ATF to the right place. You can use the build
system in [our meta-repository][crust-meta] to automatically generate an SPI
flash image containing the correct versions of all firmware components.

[crust-atf]: https://github.com/crust-firmware/arm-trusted-firmware
[crust-meta]: https://github.com/crust-firmware/meta
[crust-uboot]: https://github.com/crust-firmware/u-boot

## Contributing

The success of the Crust firmware project is made possible by community
support. For more information regarding community contributions, please
reference the Crust firmware [contribution guidelines][cg].

[cg]: CONTRIBUTING.md
