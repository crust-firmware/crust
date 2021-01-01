#
# Copyright © 2020-2021 The Crust Firmware Authors.
# SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
#

CROSS_COMPILE	?= or1k-linux-musl-
CFLAGS		+= -ffixed-r2 \
		   -msfimm -mshftimm -msoft-div -msoft-mul

# The first object is used as the linker script.
obj-y += scp.ld.o

obj-y += counter.o
obj-y += exception.o
obj-y += math.o
obj-y += runtime.o
obj-y += start.o
