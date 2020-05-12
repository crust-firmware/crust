# Crust Firmware ABI

Crust, and any firmware that wishes to be compatible with it, interacts with
other third-party software and firmware components at several ABI boundaries.
Some of those boundaries were arbitrarily chosen, and are now hardcoded in
various places throughout the ecosystem. This document attempts to enumerate
those boundaries and provide the rationale for choosing them.

## Loading the firmware

### Location in SRAM

Crust is loaded into SRAM A2, because that is the SRAM region in the same power
domain as the AR100. It is also the SRAM region containing the AR100 exception
vector area.

On platforms that use ATF (A64/H5/H6), Crust is allocated the last 16KiB of
SRAM A2. 16KiB was chosen as the smallest size that an SCP firmware could
feasibly be made. It is simultaneously the largest amount of space that could
be shaved off of ATF without breaking debug builds (it was previously using all
of SRAM A2).

Crust was placed at the end of SRAM A2 to avoid changing ATF's load address.
This allowed SCP firmware support to be added to ATF over the course of several
months, without requiring any breaking changes.

On platforms that do not use ATF (32-bit platforms), Crust reserves the right
to use the entirety of SRAM A2, if needed.

Crust's load address is hardcoded:
- In Crust, as `FIRMWARE_BASE` in `platform/*/include/platform/memory.h`
- In ATF, as `SUNXI_SCP_BASE` in `plat/allwinner/common/include/platform_def.h`
- In U-Boot, as `SCP_ADDR` in `board/sunxi/mksunxi_fit_atf.sh`

### Firmware contents

Other than the first 32-bit word, no assumptions should be made about the
contents of the Crust firmware binary.

The first 32-bit word (the entry point) is stable, because it is the only way
to use a single entry point for all exception vectors. Other firmware can
assume that if the first four bytes at Crust's load address are `12 00 40 b4`,
then Crust has been loaded into SRAM. Conversely, if the bytes are any other
value, it is safe to assume that Crust has not been loaded.

This magic value is hardcoded:
- In Crust, as the first instruction in `common/start.S`
- In ATF, as `SCP_FIRMWARE_MAGIC` in `plat/allwinner/common/sunxi_pm.c`

### Loading process

Crust is not loaded adjacent to the AR100 exception vector area (which is 4KiB
before the beginning of "usable" SRAM A2). Therefore, the exception vectors
cannot be stored inside the firmware binary, and they must be programmed
separately.

Since the opcode for the `l.j` (unconditional jump) instruction is 0, this is
easy. The value to write is `(entry - vector) >> 2`.

This process is performed:
- In Crust, as the loop labeled "Writing exception vectors" in `tools/load.c`
- In ATF, inside `plat_setup_psci_ops()` in `plat/allwinner/common/sunxi_pm.c`

## Communicating via SCPI

SCPI uses two hardware interfaces: a mailbox and a shared memory area.

Crust provides two SCPI communication channels: one for ATF (secure") and one
for Linux ("non-secure"). Crust reserves the right to add a third communication
channel in the future for a TEE in Secure EL1. Therefore, it must be assumed to
use three pairs mailbox channels and three shared memory segments.

### Mailbox

SCP firmware is responsible for enabling the mailbox clock and reset. Other
mailbox drivers SHOULD enable them if they are not already enabled, but MUST
NOT disable them once they are enabled.

SCP firmware is responsible for configuring the mailbox channel directions.
Other mailbox drivers MUST NOT modify the channel direction registers.

Crust has reserved the first six mailbox channels for use with SCPI. While they
MAY also be used for other protocols, SCPI client drivers are unlikely to
handle this cleanly. Channel allocation is as follows:

| Channel | Direction | Use                        |
|---------|-----------|----------------------------|
|       0 | AP  → SCP | SCPI (Secure EL3)          |
|       1 | SCP → AP  | SCPI (Secure EL3)          |
|       2 | AP  → SCP | SCPI (Non-secure EL1/EL2)  |
|       3 | SCP → AP  | SCPI (Non-secure EL1/EL2)  |
|       4 | AP  → SCP | SCPI (Secure EL1) (future) |
|       5 | SCP → AP  | SCPI (Secure EL1) (future) |
|       6 | AP  → SCP | Unallocated                |
|       7 | SCP → AP  | Unallocated                |

This allocation is defined:
- In Crust, as `RX_CHAN`/`TX_CHAN` in `common/scpi.c`
- In ATF, as `RX_CHAN`/`TX_CHAN` in `drivers/allwinner/sunxi_msgbox.c`
- In Linux, in the `mboxes` property of the `scpi` node in the device tree

SCPI mailbox notifications use the standard virtual channel number 1. This is
defined:
- In Crust, as `SCPI_VIRTUAL_CHANNEL` in `include/lib/scpi_protocol.h`
- In ATF, as `SCPI_MHU_SLOT_ID` in `drivers/arm/css/scpi/css_scpi.c`

### Shared memory

Crust supports SCPI with a message size of 256 bytes. This is hardcoded:
- In Crust, as `SCPI_MESSAGE_SIZE` in `include/lib/scpi_protocol.h`
- In ATF, as `SCPI_SHARED_MEM_AP_TO_SCP` in `drivers/arm/css/scpi/css_scpi.c`

Thus, each shared memory segment is 512 bytes long. Shared memory segments grow
down from the top of SRAM A2, in the same order as mailbox channels grow up.

| Offset | Direction | Use                        |
|--------|-----------|----------------------------|
| -0x100 | AP  → SCP | SCPI (Secure EL3)          |
| -0x200 | SCP → AP  | SCPI (Secure EL3)          |
| -0x300 | AP  → SCP | SCPI (Non-secure EL1/EL2)  |
| -0x400 | SCP → AP  | SCPI (Non-secure EL1/EL2)  |
| -0x500 | AP  → SCP | SCPI (Secure EL1) (future) |
| -0x600 | SCP → AP  | SCPI (Secure EL1) (future) |

These offsets are defined:
- In Crust, as `SCPI_MEM_AREA` in `common/scpi.c`
- In ATF, as `PLAT_CSS_SCP_COM_SHARED_MEM_BASE` in
  `plat/allwinner/common/include/platform_def.h`
- In Linux, as the `reg` property of the `scp-shmem` node in the device tree

Note that because SRAM A2 is "secure-only", Linux may not be able to access it
when the secure mode fuse is blown. In that case, the non-secure shared memory
will have to be moved to DRAM or SRAM A1.

### Timeouts

Testing shows that Crust normally responds to SCPI commands within 1ms.
Acknowledgment of responses is polled in ATF and interrupt-driven in Linux. In
both cases, it should be at least as fast. Thus the current timeouts are very
conservative. The only timeout that is sometimes too short is the ATF polling
timeout, when adding many lines of serial debugging to the Crust boot process.

Timeouts are defined:
- In Crust, as `SCPI_TX_TIMEOUT` in `common/scpi.c` (10ms)
- In ATF, as `MHU_TIMEOUT_ITERS` in `drivers/allwinner/sunxi_msgbox.c` (1s)
- In Linux, as `MAX_RX_TIMEOUT` in `drivers/firmware/arm_scpi.c` (30ms)

### Concurrent use

Crust supports concurrent SCPI commands by all clients, but only one command at
a time per client. No further commands from a client will be processed until
the previous response is acknowledged.

Currently, neither ATF nor Linux attempts to send multiple concurrent commands
on a single channel.

### Supported commands

Crust attempts to follow the SCPI specification in advertising the list of
supported commands, and implementing them according to spec.

### Sending "SCP Ready"

Since the mailbox hardware in sunxi SoCs is a FIFO, not the doorbell that SCPI
was designed for, it is important to avoid sending messages that will be
ignored. To this end, Crust only sends the "SCP Ready" message when it believes
the system is first booting.

Crust assumes the system is booting iff CPU 0 is the only running CPU. This can
be wrong, for example if Crust crashed and restarted while Linux was booted and
all other CPUs were offlined to save power.

It may be prudent to introduce an explicit handshake with ATF to signal first
boot.

### Power states

Crust uses the same CPU/CSS power states as defined in the SCPI specification
appendix:

| State | Meaning   |
|-------|-----------|
|     0 | On        |
|     1 | Retention |
|     2 | RESERVED  |
|     3 | Off       |

These are also defined:
- In Crust, as `SCPI_CSS_*` in `include/lib/scpi_protocol.h`
- In ATF, as `scpi_power_state_t` in `include/drivers/arm/css/css_scpi.h`

System power states are defined by the SCPI specification.

## Hardware ownership

- Crust owns the shared part and its private half of `HWSPINLOCK` and `MSGBOX`.
  ATF and Linux MUST NOT modify any part but the ARM private half.
- Crust owns `R_CPUCFG`, `R_TIMER`, and `R_TWD`. Linux MUST NOT use them. ATF
  MUST NOT use them except for using `R_CPUCFG` to turn on the AR100.
- Crust owns all of `R_PRCM` except the clock/reset registers. ATF and Linux
  MUST NOT use any registers except those controlling clocks/resets.
- Crust MAY modify `R_RSB` and `R_TWI`, but only during boot or suspend. Linux
  and ATF MUST reset these devices after resume before attempting to use them.
- Crust MAY modify `RTC`, but only during boot or suspend, except for the
  general purpose registers, which may be modified at any time. Use of general
  purpose registers by ATF, Linux, or Crust MUST be documented.
- Crust MAY modify `CCU` and `R_PRCM`, but only during boot or suspend, and it
  MUST restore the original configuration before Linux resumes, except for:
  - The following bus clocks/resets, which Crust owns: `HWSPINLOCK`, `MSGBOX`,
    `DRAM`, `R_TWD`
  - And the following bus clocks/resets, which Crust MAY leave disabled after
    resume: `R_RSB`, `R_TWI`
- Crust MAY modify `PIO`, `R_CIR_RX`, `R_PIO`, `R_INTC`, and `R_UART`, but only
  during boot or suspend, and it MUST restore the original configuration before
  Linux resumes.
