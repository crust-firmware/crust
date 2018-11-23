#
# Copyright © 2017-2018 The Crust Firmware Authors.
# SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
#

SRC		 = .
OBJ		 = build
TGT		 = $(OBJ)/scp

CROSS_COMPILE	?= or1k-linux-musl-
AR		 = $(CROSS_COMPILE)gcc-ar
CC		 = $(CROSS_COMPILE)gcc
CPP		 = $(CROSS_COMPILE)cpp
OBJCOPY		 = $(CROSS_COMPILE)objcopy

HOSTAR		 = ar
HOSTCC		 = cc

LEX		 = lex
YACC		 = yacc

HAVE_GCC9	:= $(findstring version 9,$(shell $(CC) -v 2>&1;:))

COMMON_CFLAGS	 = -Os -pipe -std=c11 \
		   -fdata-sections \
		   -ffunction-sections \
		   -fno-builtin \
		   -fno-common \
		   -fvar-tracking-assignments \
		   -g$(if $(CONFIG_DEBUG_INFO),gdb,0) \
		   -Wall -Wextra -Wformat=2 -Wpedantic -Wshadow \
		   -Werror=implicit-function-declaration \
		   -Werror=implicit-int \
		   -Werror=pointer-arith \
		   -Werror=pointer-sign \
		   -Werror=strict-prototypes \
		   -Werror=vla \
		   -Wno-missing-field-initializers
COMMON_CPPFLAGS	 = -I$(OBJ)/include \
		   -I$(SRC)/include/lib

AFLAGS		 = -Wa,--fatal-warnings
CFLAGS		 = $(COMMON_CFLAGS) \
		   -ffreestanding \
		   -flto \
		   -fno-asynchronous-unwind-tables \
		   -fno-pie \
		   -fomit-frame-pointer \
		   -funsigned-char \
		   -mhard-mul -msoft-div \
		   $(if $(HAVE_GCC9),-msext -msfimm -mshftimm) \
		   -static
CPPFLAGS	 = $(COMMON_CPPFLAGS) \
		   -I$(SRC)/include/common \
		   -I$(SRC)/include/drivers \
		   -I$(SRC)/include/stdlib \
		   -I$(SRC)/platform/$(CONFIG_PLATFORM)/include \
		   -include config.h \
		   -nostdinc \
		   -Werror=missing-include-dirs
LDFLAGS		 = -nostdlib \
		   -Wl,-O1 \
		   -Wl,--build-id=none \
		   -Wl,--fatal-warnings \
		   -Wl,--gc-sections \
		   -Wl,--no-dynamic-linker \
		   -Wl,--no-undefined

HOSTCFLAGS	 = $(COMMON_CFLAGS)
HOSTCPPFLAGS	 = $(COMMON_CPPFLAGS) \
		   -D_XOPEN_SOURCE=700
HOSTLDFLAGS	 =
HOSTLDLIBS	 =

###############################################################################

.DEFAULT_GOAL	:= all
GOALS		:= $(if $(MAKECMDGOALS),$(MAKECMDGOALS),$(.DEFAULT_GOAL))
MAKEFLAGS	+= -Rr

export KCONFIG_AUTOCONFIG := $(OBJ)/include/config/auto.conf
export KCONFIG_AUTOHEADER := $(OBJ)/include/config.h
export KCONFIG_TRISTATE   := $(OBJ)/include/config/tristate.conf

ifneq ($(filter-out %clean %clobber %config %format,$(GOALS)),)
  include $(OBJ)/include/config/auto.conf
  include $(OBJ)/include/config/auto.conf.cmd
endif

include $(SRC)/scripts/Makefile.format
include $(SRC)/scripts/Makefile.kbuild

$(call descend,3rdparty common drivers lib platform scripts test tools)

###############################################################################

M := @$(if $(filter-out 0,$(V)),:,exec printf '  %-7s %s\n')
Q :=  $(if $(filter-out 0,$(V)),,@)exec

all: scp $(test-all)

check: $(test-all:%=%.test)

clean:
	$(Q) rm -fr $(TGT)

clobber:
	$(Q) rm -fr $(OBJ)

distclean:
	$(Q) rm -fr $(OBJ) ..config* .config*

scp: $(TGT)/scp.bin $(TGT)/scp.elf $(TGT)/scp.map

tools: $(tools-all)

.config:;

%/.:
	$(Q) mkdir -p $*

%.d:;

$(OBJ)/%.test: $(SRC)/scripts/test.sh $(OBJ)/%
	$(M) TEST $@
	$(Q) $^ $@

$(OBJ)/%: $(OBJ)/%.o
	$(M) HOSTLD $@
	$(Q) $(HOSTCC) $(HOSTCFLAGS) $(HOSTLDFLAGS) -o $@ $^ $(HOSTLDLIBS)

$(test-all): $(OBJ)/lib.a $(OBJ)/3rdparty/unity/unity.o
$(tools-all): $(OBJ)/lib.a

$(OBJ)/lib.a:
	$(M) HOSTAR $@
	$(Q) $(HOSTAR) Drcs $@ $^

$(OBJ)/%.o: $(OBJ)/%.c
	$(M) HOSTCC $@
	$(Q) $(HOSTCC) $(HOSTCPPFLAGS) $(HOSTCFLAGS) -MMD -c -o $@ $<

$(OBJ)/%.o: $(SRC)/%.c
	$(M) HOSTCC $@
	$(Q) $(HOSTCC) $(HOSTCPPFLAGS) $(HOSTCFLAGS) -MMD -c -o $@ $<

$(OBJ)/%.lex.c: $(SRC)/%.l
	$(M) LEX $@
	$(Q) $(LEX) -o $@ $<

$(OBJ)/%.tab.c $(OBJ)/%.tab.h: $(SRC)/%.y
	$(M) YACC $@
	$(Q) $(YACC) -d -t -o $@ $<

$(OBJ)/include/config.h: $(OBJ)/include/config/auto.conf;

$(OBJ)/include/config/auto.conf: $(OBJ)/3rdparty/kconfig/conf .config
	$(M) GEN $@
	$(Q) $< --syncconfig $(SRC)/Kconfig

$(OBJ)/include/config/auto.conf.cmd: $(OBJ)/include/config/auto.conf;

$(TGT)/%.bin: $(TGT)/%.elf
	$(M) OBJCOPY $@
	$(Q) $(OBJCOPY) -O binary -S --reverse-bytes 4 $< $@

$(TGT)/%.elf $(TGT)/%.map: $(TGT)/scripts/%.ld $(obj-all) $(TGT)/lib.a
	$(M) LD $@
	$(Q) $(CC) $(CFLAGS) $(LDFLAGS) -Wl,-Map,$(TGT)/$*.map -o $@ -T $^

$(TGT)/%.ld: $(SRC)/%.ld.S
	$(M) CPP $@
	$(Q) $(CPP) $(CPPFLAGS) -MMD -MF $@.d -MT $@ -P -o $@ $<

$(TGT)/lib.a:
	$(M) AR $@
	$(Q) $(AR) Drcs $@ $^

$(TGT)/%.o: $(SRC)/%.c
	$(M) CC $@
	$(Q) $(CC) $(CPPFLAGS) $(CFLAGS) $(AFLAGS) -MMD -c -o $@ $<

$(TGT)/%.o: $(SRC)/%.S
	$(M) AS $@
	$(Q) $(CC) $(CPPFLAGS) $(AFLAGS) -MMD -c -o $@ $<

$(SRC)/Makefile:;

.PHONY: all check clean clobber distclean scp tools
.SECONDARY:
.SUFFIXES:
