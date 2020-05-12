# Crust: Libre SCP firmware for Allwinner sunxi SoCs

[![CI status](https://travis-ci.com/crust-firmware/crust.svg?branch=master)][t]

[t]: https://travis-ci.com/crust-firmware/crust

## What is it?

The crust is the lowest-level component of a delicious fruit pie. Similary,
Crust is the lowest-level firmware component that runs on `$FRUIT` (Banana,
Orange, Lichee) Pi single-board computers and other Allwinner-based devices,
such as the Pine64 Pinebook and PinePhone.

Crust improves battery life and thermal performance by implementing a deep
sleep state. During deep sleep, the CPU cores, the DRAM controller, and most
onboard peripherals are powered down, reducing power consumption by 80% or more
compared to an idle device. On boards without a PMIC, Crust is also responsible
for orderly power-off and power-on of the device.

For this to work, Crust runs outside the main CPU and DRAM, on a dedicated
always-on microprocessor called a System Control Processor (SCP). Crust is
designed to run on a specific SCP implementation, Allwinner's [AR100][].

See [Crust's ABI documentation][abi] for a detailed description of how Crust
interacts with other firmware components at runtime.

Interested users and contributors are encouraged to join `#crust` on freenode
to discuss the firmware and related power management topics.

[abi]: docs/abi.md
[AR100]: https://linux-sunxi.org/AR100

## Supported devices

Crust supports any board with a SoC listed in the table below. There is no
board-specific code needed for basic functionality. Boards that are tested and
known to work have a `defconfig` file in the repository. For everything else,
use the defconfig for a similar board, or run `make config` or `make nconfig`
to choose the appropriate options (there aren't many).

|  SoC  |   Support level   | SCPI | CPU cores | CPU subsystem | DRAM | PMIC |
|-------|-------------------|------|-----------|---------------|------|------|
| A64   | Production/stable | Yes  | Yes       | Yes           | Yes  | Yes  |
| A83T  | Known to compile  | Yes  | No        | No            | No   | No   |
| H5    | Production/stable | Yes  | Yes       | Yes           | Yes  | N/A  |
| H6    | Known to run      | Yes  | Yes       | No            | No   | No   |

Support for the H3 SoC is planned, but it has not yet been implemented. Patches
providing support for H3 or older chips are welcome, as are patches providing
defconfigs for tested boards.

## Prerequisites

Crust supports mainline Linux only. It completely replaces Allwinner's bespoke,
proprietary firmware with a libre solution that supports [standard
protocols][scpi] and is developed entirely in the open with community input.
Effort is underway to upstream all changes to third-party projects; however,
some patches are currently still needed.

- ARM Trusted Firmware: upstream support for Crust was merged in commit
  [`c335ad480d41`][atf-c335ad480d41], and is present in all releases starting
  with [v2.3][atf-v2.3]. Optional patches for improved support are available in
  the `crust` branch of [the crust-firmware fork][crust-atf].
- Linux: while Linux does not directly communicate with Crust, it requires some
  small patches to cleanly share the clock controller and PMIC bus controller
  hardware with Crust. They are available in the `crust-minimal` branch of [the
  crust-firmware fork][crust-linux]. Those patches, plus additional changes for
  reduced power consumption (helpful even if you are not using Crust), are
  available in the `crust` branch.
- U-Boot: some small patches are needed for U-Boot to load Crust to the right
  place in SRAM at boot. There are in the `crust` branch of [the crust-firmware
  fork][crust-u-boot]. While these patches are recommended, it is possible to
  avoid them by padding your ATF binary to 48KiB (64KiB for H6) and then
  concatenating Crust onto the end.

[atf-c335ad480d41]: https://github.com/ARM-Software/ARM-Trusted-Firmware/commits/c335ad480d41
[atf-v2.3]: https://github.com/ARM-software/arm-trusted-firmware/releases/tag/v2.3
[crust-atf]: https://github.com/crust-firmware/arm-trusted-firmware
[crust-linux]: https://github.com/crust-firmware/linux
[crust-u-boot]: https://github.com/crust-firmware/u-boot
[scpi]: http://infocenter.arm.com/help/topic/com.arm.doc.dui0922-/index.html

## Building the firmware

An easy way to get all the pieces in the right places, with the right patches,
is to use the `Makefile` in the [crust-firmware meta repository][crust-meta].
See the README file there for further instructions. Alternatively, you can
build each firmware component individually. See the [README.sunxi64][sunxi64]
file in the U-Boot source tree for more details. Installation of the combined
U-Boot+ATF+Crust binary works the same as for U-Boot without Crust.

Building Crust requires a cross-compiler targeting the `or1k` architecture
(OpenRISC 1000, *not* RISC-V), which is officially supported in upstream GCC
starting with GCC 9.1.0. Prebuilt toolchains are available from [musl.cc][],
[bootlin][], and possibly your Linux distribution's package archive.

If your cross toolchain has a different tuple (the toolchain's `libc` is not
relevant when compiling freestanding firmware programs), or if it is not in
your `PATH`, export `CROSS_COMPILE` or edit the top of the `Makefile` to
provide the appropriate prefix or full path.

Run `make` to build the firmware and related tools, or `make scp` to build just
the firmware, which will be placed at `build/scp/scp.bin`. Adding `V=1` to the
command line will run a verbose build, showing you the commands as they run.
Set `SRC`, `OBJ`, or `TGT` as necessary if you want to do an out-of-tree build.

[bootlin]: https://toolchains.bootlin.com/
[crust-meta]: https://github.com/crust-firmware/meta
[musl.cc]: http://musl.cc/or1k-linux-musl-cross.tgz
[sunxi64]: https://github.com/u-boot/u-boot/raw/master/board/sunxi/README.sunxi64

## Contributing

The success of the crust firmware project is made possible by community
support. For more information regarding community contributions, please
reference the crust firmware [contribution guidelines][cg].

[cg]: CONTRIBUTING.md
