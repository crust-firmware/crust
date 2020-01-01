# Crust: Free management firmware for Allwinner SoCs

[![CI status](https://travis-ci.com/crust-firmware/crust.svg?branch=master)][t]

[t]: https://travis-ci.com/crust-firmware/crust

## Overview

Most Allwinner SoCs, including all recent ones, contain an [OpenRISC][or1k] CPU
in addition to their ARM cores. This CPU is in the RTC power domain and is
responsible for system suspend/resume/reset/shutdown, dynamic frequency scaling
for the ARM cores, and controlling the external power management IC (PMIC). See
the [AR100][ar100] article on the linux-sunxi wiki for more information.

This firmware attempts to completely replace the proprietary firmware provided
by Allwinner. It is designed to be compatible with mainline versions of Linux,
U-Boot, and ATF. While initially only developed on and for the A64 and H5 SoCs,
it aims to be portable to all relevant boards and SoCs. See below for details
about hardware support.

The current development version of this firmware requires the `crust` branches
of the (forked) [U-Boot][crust-u-boot] and [ATF][crust-atf] repositories. Those
patchsets are mostly glue to load and run Crust, but they also include some
configuration changes that make testing the firmware easier.

Users and contributors are encouraged to join `#crust` on freenode to discuss
the firmware and related power management topics.

[ar100]: https://linux-sunxi.org/AR100
[or1k]: http://openrisc.io/

## Architecture

This firmware is designed to be flexible yet extremely lightweight. It borrows
heavily from ideas in both Linux and ATF for its layout and driver model. The
code is divided into directories based on major function:

- `configs`: These files contain configuration for each board supported by this
  firmware. They determine which subdirectory of `platform` is used and which
  devices are enabled.
- `common`: Files in this directory contain the main logic of the firmware, as
  well as glue code for connecting drivers, handling exceptions, etc.
- `drivers`: This directory contains a subdirectory for each class of drivers.
  - `drivers/<class>`: These directories contain all of the drivers of a class,
    as well as generic dispatch code (in `drivers/<class>/<class>.c`).
- `include`: This directory contains headers for code in `common` and `lib`, as
  well as standalone definitions.
  - `include/common/arch`: These headers expose functionality of the CPU
    architecture that are not dependent on a specific hardware implementation.
  - `include/drivers`: These headers specify the interface for each class of
    drivers. Also included are headers for individual drivers; these provide
    declarations necessary to interact with devices from outside the drivers.
- `lib`: This directory contains standalone code that does not depend on other
  outside code (with few exceptions). This code should be easily reusable in
  other projects.
- `platform`: This directory contains a subdirectory for each supported
  platform, which refers to a family of SoCs with a similar programming
  interface.
  - `platform/<platform>/include`: This directory contains headers with
    platform-specific macro definitions, such as memory addresses of devices
    and register layouts that change between platforms.
- `scripts`: These files are used to assist in building and linking the
  firmware, but contain no code themselves.
- `tools`: Each file here is a self-contained program intended to be run on the
  host machine (in Linux userspace on the ARM cores) to gather information,
  manipulate hardware, load firmware, or for other reasons.

## Build prerequisites

Building this firmware requires a cross-compiler targeting the `or1k`
architecture, which is officially supported in GCC as of GCC 9. Development
snapshots are available from [the maintainer's GitHub account][stffrdhrn]. A
portable pre-built toolchain is available from [musl.cc][musl-cc-or1k].

If your cross toolchain has a different tuple (the toolchain's `libc` is not
relevant for compiling this firmware), or is not in your `PATH`, edit the top
of the `Makefile` to provide the appropriate full path or prefix.

[stffrdhrn]: https://github.com/stffrdhrn/gcc/releases
[musl-cc-or1k]: http://musl.cc/or1k-linux-musl-cross.tgz

## Supported devices

While this firmware should ideally support any SoC with the AR100 core present,
there are several limiting factors to doing so:
- MMIO device memory map differences between SoCs.
- Differences in SRAM area location and size between SOCs.
- Different PMICs used with different hardware generations.
- Variations in connection to external voltage regulators, buttons, LEDs, etc.
- Hardware bugs, if any.

This firmware is well-tested and known to properly suspend and resume the
following SoCs:
- A64
- H5

The firmware is also known to run and communicate over SCPI (but without
suspend/resume functionality) on the following SoCs:
- A83T
- H6

In order to support external devices, the firmware needs to know some
information about the layout of the board and the connections to port L GPIO
pins. This project uses Kconfig to handle the possible configurations. You can
run `make <board>_defconfig` for boards that are already supported by the
firmware, or `make config` or `make nconfig` to create a custom configuration.

## Running the firmware

To run crust, you'll need [a patched version of U-Boot][crust-u-boot] that
loads all of the firmware pieces to the right place in SRAM. And you'll need [a
version of ATF][crust-atf] that has an SCPI client that can take advantage of
this firmware's capabilities. You can use the build system in [the crust
meta-repository][crust-meta] to automatically generate an MMC or SPI flash
image containing the correct versions of all firmware components. Installation
works the same as for U-Boot without crust.

The ATF and U-Boot patches are being upstreamed as much as possible while crust
is in development. However, it will take a while before crust can be used
without custom patches, as it requires coordination between several independent
software projects.

[crust-atf]: https://github.com/crust-firmware/arm-trusted-firmware
[crust-meta]: https://github.com/crust-firmware/meta
[crust-u-boot]: https://github.com/crust-firmware/u-boot

## Contributing

The success of the crust firmware project is made possible by community
support. For more information regarding community contributions, please
reference the crust firmware [contribution guidelines][cg].

[cg]: CONTRIBUTING.md
