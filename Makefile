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
		   -g$(if $(filter-out 0,$(DEBUG)),gdb,0) \
		   -mdelay -mhard-mul -msoft-float \
		   -Wa,--fatal-warnings \
		   $(WARNINGS)
CPPFLAGS	 = -DDEBUG=$(if $(filter-out 0,$(DEBUG)),1,0) \
		   -DTEST=$(if $(filter-out 0,$(TEST)),1,0) \
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
HOSTCPPFLAGS	 = -D_XOPEN_SOURCE=700
HOSTLDFLAGS	 =
HOSTLIBS	 =

platdir		 = platform/$(CONFIG_PLATFORM)

generated	 = $(OBJ)/include/config.h

files		 = $(sort $(wildcard $(foreach x,$2,$(1:%=$(SRC)/%/$x))))
incdirs		 = $(addprefix -I,$(sort $(OBJ)/include $(wildcard $1)))
headers		 = $(call files,$1,*.h */*.h *.S) $(generated)
sources		 = $(call files,$1,*.c *.S)
objdirs		 = $(sort $(patsubst %/,%,$(dir $1)))
objects		 = $(patsubst $(SRC)$2/%,$(OBJ)$3/%$4,$(basename $1))

format-filter	 = $(filter-out $(OBJ)/% %.S,$1)
formatheaders	 = $(call headers,include/* platform/*/include)
formatsources	 = $(call sources,common drivers/* lib platform/* test tools)
formatfiles	 = $(call format-filter,$(formatheaders) $(formatsources))

fwincbase	 = $(platdir)/include include/*
fwincdirs	 = $(call incdirs,$(fwincbase))
fwheaders	 = $(call headers,$(fwincbase))
fwsources	 = $(call sources,common drivers/* lib $(platdir))
fwobjects	 = $(call objects,$(fwsources),,,.o)
fwobjdirs	 = $(call objdirs,$(fwobjects))
fwfiles		 = $(addprefix $(OBJ)/,scp.bin scp.elf scp.map)

libincbase	 = include/lib
libincdirs	 = $(call incdirs,$(libincbase))
libheaders	 = $(call headers,$(libincbase))
libsources	 = $(call files,lib,*.c)
libobjects	 = $(call objects,$(libsources),,/host,.o)
libobjdirs	 = $(call objdirs,$(libobjects))
library		 = $(OBJ)/host/lib/libcrust.a

testincbase	 = 3rdparty/unity include/lib
testincdirs	 = $(call incdirs,$(testincbase))
testheaders	 = $(call headers,$(testincbase))
testsources	 = $(call sources,test)
testobjects	 = $(call objects,$(testsources),,/host,.o)
testobjdirs	 = $(call objdirs,$(testobjects) $(unityobjects))
tests		 = $(basename $(testobjects))
testresults	 = $(addsuffix .test,$(tests))

toolincbase	 = $(platdir)/include include/lib
toolincdirs	 = $(call incdirs,$(toolincbase))
toolheaders	 = $(call headers,$(toolincbase))
toolsources	 = $(call sources,tools)
toolobjects	 = $(call objects,$(toolsources),,/host,.o)
toolobjdirs	 = $(call objdirs,$(toolobjects))
tools		 = $(basename $(toolobjects))

unitysources	 = $(call sources,3rdparty/unity)
unityobjects	 = $(call objects,$(unitysources),/3rdparty,/host/test,.o)

allobjdirs	 = $(OBJ) $(OBJ)/include $(fwobjdirs) $(libobjdirs) \
		   $(testobjdirs) $(toolobjdirs)

ifeq ($(MAKECMDGOALS),)
include $(OBJ)/config.mk
else
ifneq ($(filter-out %clean %config %format,$(MAKECMDGOALS)),)
include $(OBJ)/config.mk
endif
endif

M := @$(if $(filter-out 0,$(V)),:,printf '  %-7s %s\n')
Q :=  $(if $(filter-out 0,$(V)),,@)

all: $(fwfiles) $(tests) $(tools)

check: $(testresults)

check-format: $(formatfiles)
	$(Q) uncrustify -c $(SRC)/.uncrustify -l C -q --check $^

clean:
	$(Q) rm -fr $(OBJ)

%_defconfig:
	$(Q) cp -f $(SRC)/board/$* .config

distclean:
	$(Q) rm -fr $(OBJ) .config

firmware: $(fwfiles)

format: $(formatfiles)
	$(Q) uncrustify -c $(SRC)/.uncrustify -l C -q --no-backup $^

test: check

tools: $(tools)

$(allobjdirs):
	$(Q) mkdir -p $@

$(OBJ)/%.bin: $(OBJ)/%.elf | $(OBJ)
	$(M) OBJCOPY $@
	$(Q) $(OBJCOPY) -O binary -S --reverse-bytes 4 $< $@

$(OBJ)/%.elf $(OBJ)/%.map: $(OBJ)/%.ld $(fwobjects) | $(OBJ)
	$(M) CCLD $@
	$(Q) $(CC) $(CFLAGS) $(LDFLAGS) \
		-Wl,-Map,$(OBJ)/$*.map -o $(OBJ)/$*.elf -T $^

$(OBJ)/%.ld: $(SRC)/scripts/%.ld.S $(fwheaders) | $(OBJ)
	$(M) CPP $@
	$(Q) $(CPP) $(CPPFLAGS) $(fwincdirs) -o $@ -P $<

$(OBJ)/%.o: $(SRC)/%.c $(fwheaders) | $(fwobjdirs)
	$(M) CC $@
	$(Q) $(CC) $(CPPFLAGS) $(CFLAGS) $(fwincdirs) -c -o $@ $<

$(OBJ)/%.o: $(SRC)/%.S $(fwheaders) | $(fwobjdirs)
	$(M) CC $@
	$(Q) $(CC) $(CPPFLAGS) $(CFLAGS) $(fwincdirs) -c -o $@ $<

$(OBJ)/config.mk: .config | $(OBJ)
	$(Q) sed 's/#.*$$//;s/="\(.*\)"$$/=\1/' $< > $@

$(OBJ)/include/config.h: .config | $(OBJ)/include
	$(Q) sed -n 's/#.*$$//;s/^\([^=]\+\)=\(.*\)$$/#define \1 \2/p' $< > $@

$(library): $(libobjects) | $(libobjdirs)
	$(M) HOSTAR $@
	$(Q) $(HOSTAR) rcs $@ $^

$(OBJ)/host/lib/%.o: $(SRC)/lib/%.c $(libheaders) | $(libobjdirs)
	$(M) HOSTCC $@
	$(Q) $(HOSTCC) $(HOSTCPPFLAGS) $(HOSTCFLAGS) $(libincdirs) -c -o $@ $<

$(OBJ)/host/test/%.test: $(OBJ)/host/test/%
	$(M) TEST $@
	$(Q) $< > $@.tmp && mv -f $@.tmp $@ || { cat $@.tmp; rm -f $@.tmp; }

$(OBJ)/host/test/%: $(OBJ)/host/test/%.o $(unityobjects) $(library)
	$(M) HOSTLD $@
	$(Q) $(HOSTCC) $(HOSTCFLAGS) $(HOSTLDFLAGS) -o $@ $^ $(HOSTLIBS)

$(OBJ)/host/test/%.o: $(SRC)/3rdparty/%.c $(testheaders) | $(testobjdirs)
	$(M) HOSTCC $@
	$(Q) $(HOSTCC) $(HOSTCPPFLAGS) $(HOSTCFLAGS) $(testincdirs) -c -o $@ $<

$(OBJ)/host/test/%.o: $(SRC)/test/%.c $(testheaders) | $(testobjdirs)
	$(M) HOSTCC $@
	$(Q) $(HOSTCC) $(HOSTCPPFLAGS) $(HOSTCFLAGS) $(testincdirs) -c -o $@ $<

$(OBJ)/host/tools/%: $(SRC)/tools/%.c $(toolheaders) | $(toolobjdirs)
	$(M) HOSTCC $@
	$(Q) $(HOSTCC) $(HOSTCPPFLAGS) $(HOSTCFLAGS) $(HOSTLDFLAGS) \
		$(toolincdirs) -o $@ $< $(HOSTLIBS)

.PHONY: all check check-format clean distclean firmware format test tools
.SECONDARY:
.SUFFIXES:
