#
# Copyright © 2017-2018 The Crust Firmware Authors.
# SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
#

SRC		 = .
OBJ		 = build

CROSS_COMPILE	?= or1k-linux-musl-
AR		 = $(CROSS_COMPILE)ar
CC		 = $(CROSS_COMPILE)gcc
CPP		 = $(CROSS_COMPILE)cpp
OBJCOPY		 = $(CROSS_COMPILE)objcopy

WARNINGS	 = -Wall -Wextra -Wformat=2 -Wpedantic -Wshadow \
		   -Werror=implicit-function-declaration \
		   -Werror=implicit-int \
		   -Werror=pointer-arith \
		   -Werror=pointer-sign \
		   -Werror=strict-prototypes \
		   -Werror=vla \
		   -Wno-missing-field-initializers

CFLAGS		 = -Os -pipe -std=c11 \
		   -fdata-sections \
		   -ffreestanding \
		   -ffunction-sections \
		   -flto \
		   -fno-asynchronous-unwind-tables \
		   -fno-common \
		   -fomit-frame-pointer \
		   -funsigned-char \
		   -g$(if $(CONFIG_DEBUG_INFO),gdb,0) \
		   -mdelay -mhard-mul -msoft-float \
		   -Wa,--fatal-warnings \
		   $(WARNINGS)
CPPFLAGS	 = -I$(OBJ)/include \
		   -include config.h \
		   -nostdinc \
		   -Werror=missing-include-dirs
LDFLAGS		 = -nostdlib \
		   -Wl,-O1 \
		   -Wl,--build-id=none \
		   -Wl,--fatal-warnings \
		   -Wl,--gc-sections \
		   -Wl,--no-undefined

HOSTAR		 = ar
HOSTCC		 = cc
HOSTCFLAGS	 = -fno-builtin \
		   -O2 -pipe -std=c11 \
		   $(WARNINGS)
HOSTCPPFLAGS	 = -D_XOPEN_SOURCE=700 \
		   -I$(OBJ)/include
HOSTLDFLAGS	 =
HOSTLIBS	 =

MAKEFLAGS	+= -r

###############################################################################

files		 = $(sort $(wildcard $(foreach x,$2,$(1:%=$(SRC)/%/$x))))
formatdirs	 = common drivers/* include/* lib platform/* test tools
formatfiles	 = $(call files,$(formatdirs),*.c *.h */*.h)

M := @$(if $(filter-out 0,$(V)),:,printf '  %-7s %s\n')
Q :=  $(if $(filter-out 0,$(V)),,@)

all: firmware tools

check:

check-format: $(formatfiles)
	$(Q) uncrustify -c $(SRC)/.uncrustify -l C -q --check $^

clean:
	$(Q) rm -fr $(OBJ)

distclean:
	$(Q) rm -fr $(OBJ) ..config* .config*

firmware: $(OBJ)/scp.bin $(OBJ)/scp.elf $(OBJ)/scp.map

format: $(formatfiles)
	$(Q) uncrustify -c $(SRC)/.uncrustify -l C -q --no-backup $^

test: check

tools:

.config:;

%/:
	$(Q) mkdir -p $@

$(OBJ)/include/config.h: $(OBJ)/include/config/auto.conf;

$(OBJ)/include/config/auto.conf: .config
	$(Q) $(MAKE) -f $(SRC)/Makefile silentoldconfig

$(OBJ)/include/config/auto.conf.cmd: $(OBJ)/include/config/auto.conf;

$(OBJ)/scp.bin: $(OBJ)/scp.elf
	$(M) OBJCOPY $@
	$(Q) $(OBJCOPY) -O binary -S --reverse-bytes 4 $< $@

$(OBJ)/scp.elf: $(OBJ)/scp.ld
	$(M) CCLD $@
	$(Q) $(CC) $(CFLAGS) $(LDFLAGS) -Wl,-Map,$(OBJ)/scp.map -o $@ -T $^

$(OBJ)/scp.ld: $(SRC)/scripts/scp.ld.S | $(OBJ)/
	$(M) CPP $@
	$(Q) $(CPP) $(CPPFLAGS) -MMD -MF $@.d -MT $@ -o $@ -P $<

$(OBJ)/scp.ld.d:;

-include $(OBJ)/scp.ld.d

$(OBJ)/scp.map: $(OBJ)/scp.elf;

$(OBJ)/%.a:
	$(M) AR $@
	$(Q) $(AR) rcs $@ $^

$(OBJ)/%.o: $(SRC)/%.c
	$(M) CC $@
	$(Q) $(CC) $(CPPFLAGS) $(CFLAGS) -MMD -c -o $@ $<

$(OBJ)/%.o: $(SRC)/%.S
	$(M) CC $@
	$(Q) $(CC) $(CPPFLAGS) $(CFLAGS) -MMD -c -o $@ $<

$(OBJ)/host/%: $(OBJ)/host/%.o
	$(M) HOSTLD $@
	$(Q) $(HOSTCC) $(HOSTCFLAGS) $(HOSTLDFLAGS) -o $@ $^ $(HOSTLIBS)

$(OBJ)/host/%.a:
	$(M) HOSTAR $@
	$(Q) $(HOSTAR) rcs $@ $^

$(OBJ)/host/%.o: $(OBJ)/host/%.c
	$(M) HOSTCC $@
	$(Q) $(HOSTCC) $(HOSTCPPFLAGS) $(HOSTCFLAGS) -MMD -c -o $@ $<

$(OBJ)/host/%.o: $(SRC)/%.c
	$(M) HOSTCC $@
	$(Q) $(HOSTCC) $(HOSTCPPFLAGS) $(HOSTCFLAGS) -MMD -c -o $@ $<

$(OBJ)/host/%.o: $(SRC)/3rdparty/%.c
	$(M) HOSTCC $@
	$(Q) $(HOSTCC) $(HOSTCPPFLAGS) $(HOSTCFLAGS) -MMD -c -o $@ $<

$(OBJ)/host/%.lex.c: $(SRC)/3rdparty/%.l
	$(M) LEX $@
	$(Q) $(LEX) -L -o $@ $<

$(OBJ)/host/%.tab.c $(OBJ)/host/%.tab.h: $(SRC)/3rdparty/%.y
	$(M) YACC $@
	$(Q) $(YACC) -d -l -t -o $@ $<

$(OBJ)/host/test/%.test: $(OBJ)/host/test/%
	$(M) TEST $@
	$(Q) $< > $@.tmp && mv -f $@.tmp $@ || { cat $@.tmp; rm -f $@.tmp; }

$(SRC)/Makefile:;

ifeq ($(MAKECMDGOALS),)
include $(OBJ)/include/config/auto.conf $(OBJ)/include/config/auto.conf.cmd
else
ifneq ($(filter-out %clean %config %format,$(MAKECMDGOALS)),)
include $(OBJ)/include/config/auto.conf $(OBJ)/include/config/auto.conf.cmd
endif
endif

include $(SRC)/scripts/Makefile.build

$(call add-headers,$(OBJ)/scp.elf,include/*)
$(call add-headers,$(OBJ)/host/test/%.o,3rdparty/unity include/lib)
$(call add-headers,$(OBJ)/host/tools/%.o,include/lib)
$(call add-subdirs,3rdparty common drivers lib platform test tools)

.PHONY: all check check-format clean distclean firmware format test tools
.SECONDARY:
.SUFFIXES:
