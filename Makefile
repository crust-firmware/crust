#
# Copyright © 2017-2018 The Crust Firmware Authors.
# SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
#

SRC		 = .
OBJ		 = build
TGT		 = $(OBJ)/scp

CROSS_COMPILE	?= or1k-linux-musl-
AR		 = $(CROSS_COMPILE)ar
CC		 = $(CROSS_COMPILE)gcc
CPP		 = $(CROSS_COMPILE)cpp
OBJCOPY		 = $(CROSS_COMPILE)objcopy

HOSTAR		 = ar
HOSTCC		 = cc

HAVE_GCC9	:= $(findstring version 9,$(shell $(CC) -v 2>&1;:))

COMMON_CFLAGS	 = -Os -pipe -std=c11 \
		   -fdata-sections \
		   -ffunction-sections \
		   -fno-builtin \
		   -fno-common \
		   -fvar-tracking-assignments \
		   -ggdb \
		   -Wall -Wextra -Wformat=2 -Wpedantic -Wshadow \
		   -Werror=implicit-function-declaration \
		   -Werror=implicit-int \
		   -Werror=pointer-arith \
		   -Werror=pointer-sign \
		   -Werror=strict-prototypes \
		   -Werror=vla \
		   -Wno-missing-field-initializers

CFLAGS		 = $(COMMON_CFLAGS) \
		   -ffreestanding \
		   -flto \
		   -fno-asynchronous-unwind-tables \
		   -fno-pie \
		   -fomit-frame-pointer \
		   -funsigned-char \
		   -mhard-mul -msoft-div \
		   $(if $(HAVE_GCC9),-msext -msfimm -mshftimm) \
		   -static \
		   -Wa,--fatal-warnings
CPPFLAGS	 = -DDEBUG=$(if $(filter-out 0,$(DEBUG)),1,0) \
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
objects		 = $(patsubst $(SRC)/%,$(OBJ)$2/%$3,$(basename $1))

format-filter	 = $(filter-out $(OBJ)/% %.S,$1)
formatheaders	 = $(call headers,include/* platform/*/include)
formatsources	 = $(call sources,common drivers/* lib platform/* test tools)
formatfiles	 = $(call format-filter,$(formatheaders) $(formatsources))

fwincbase	 = $(platdir)/include include/*
fwincdirs	 = $(call incdirs,$(fwincbase))
fwheaders	 = $(call headers,$(fwincbase))
fwsources	 = $(call sources,common drivers/* lib $(platdir))
fwobjects	 = $(call objects,$(fwsources),/scp,.o)
fwobjdirs	 = $(call objdirs,$(fwobjects))
fwfiles		 = $(addprefix $(TGT)/,scp.bin scp.elf scp.map)

libincbase	 = include/lib
libincdirs	 = $(call incdirs,$(libincbase))
libheaders	 = $(call headers,$(libincbase))
libsources	 = $(call files,lib,*.c)
libobjects	 = $(call objects,$(libsources),,.o)
libobjdirs	 = $(call objdirs,$(libobjects))
library		 = $(OBJ)/lib/libcrust.a

testincbase	 = 3rdparty/unity include/lib
testincdirs	 = $(call incdirs,$(testincbase))
testheaders	 = $(call headers,$(testincbase))
testsources	 = $(call sources,test)
testobjects	 = $(call objects,$(testsources),,.o)
testobjdirs	 = $(call objdirs,$(testobjects) $(unityobjects))
tests		 = $(basename $(testobjects))
testresults	 = $(addsuffix .test,$(tests))

toolincbase	 = $(platdir)/include include/lib
toolincdirs	 = $(call incdirs,$(toolincbase))
toolheaders	 = $(call headers,$(toolincbase))
toolsources	 = $(call sources,tools)
toolobjects	 = $(call objects,$(toolsources),,.o)
toolobjdirs	 = $(call objdirs,$(toolobjects))
tools		 = $(basename $(toolobjects))

unitysources	 = $(call sources,3rdparty/unity)
unityobjects	 = $(call objects,$(unitysources),,.o)

allobjdirs	 = $(OBJ) $(OBJ)/include $(TGT) \
		   $(fwobjdirs) $(libobjdirs) $(testobjdirs) $(toolobjdirs)

ifeq ($(MAKECMDGOALS),)
include $(OBJ)/config.mk
else
ifneq ($(filter-out %clean %clobber %config %format,$(MAKECMDGOALS)),)
include $(OBJ)/config.mk
endif
endif

M := @$(if $(filter-out 0,$(V)),:,printf '  %-7s %s\n')
Q :=  $(if $(filter-out 0,$(V)),,@)

all: $(fwfiles) $(tests)

check: $(testresults)

check-format: $(formatfiles)
	$(Q) uncrustify -c $(SRC)/.uncrustify -l C -q --check $^

clean:
	$(Q) rm -fr $(TGT)

clobber:
	$(Q) rm -fr $(OBJ)

%_defconfig:
	$(Q) cp -f $(SRC)/board/$* .config

distclean:
	$(Q) rm -fr $(OBJ) .config

format: $(formatfiles)
	$(Q) uncrustify -c $(SRC)/.uncrustify -l C -q --no-backup $^

scp: $(fwfiles)

tools: $(tools)

$(allobjdirs):
	$(Q) mkdir -p $@

$(OBJ)/config.mk: .config | $(OBJ)
	$(Q) sed 's/#.*$$//;s/="\(.*\)"$$/=\1/' $< > $@

$(OBJ)/include/config.h: .config | $(OBJ)/include
	$(Q) sed -n 's/#.*$$//;s/^\([^=]\+\)=\(.*\)$$/#define \1 \2/p' $< > $@

$(library): $(libobjects) | $(libobjdirs)
	$(M) HOSTAR $@
	$(Q) $(HOSTAR) rcs $@ $^

$(OBJ)/lib/%.o: $(SRC)/lib/%.c $(libheaders) | $(libobjdirs)
	$(M) HOSTCC $@
	$(Q) $(HOSTCC) $(HOSTCPPFLAGS) $(HOSTCFLAGS) $(libincdirs) -c -o $@ $<

$(OBJ)/test/%.test: $(SRC)/scripts/test.sh $(OBJ)/test/%
	$(M) TEST $@
	$(Q) $^ $@

$(OBJ)/test/%: $(OBJ)/test/%.o $(unityobjects) $(library)
	$(M) HOSTLD $@
	$(Q) $(HOSTCC) $(HOSTCFLAGS) $(HOSTLDFLAGS) -o $@ $^ $(HOSTLIBS)

$(OBJ)/3rdparty/%.o: $(SRC)/3rdparty/%.c $(testheaders) | $(testobjdirs)
	$(M) HOSTCC $@
	$(Q) $(HOSTCC) $(HOSTCPPFLAGS) $(HOSTCFLAGS) $(testincdirs) -c -o $@ $<

$(OBJ)/test/%.o: $(SRC)/test/%.c $(testheaders) | $(testobjdirs)
	$(M) HOSTCC $@
	$(Q) $(HOSTCC) $(HOSTCPPFLAGS) $(HOSTCFLAGS) $(testincdirs) -c -o $@ $<

$(OBJ)/tools/%: $(OBJ)/tools/%.o $(library)
	$(M) HOSTLD $@
	$(Q) $(HOSTCC) $(HOSTCFLAGS) $(HOSTLDFLAGS) -o $@ $^ $(HOSTLIBS)

$(OBJ)/tools/%.o: $(SRC)/tools/%.c $(toolheaders) | $(toolobjdirs)
	$(M) HOSTCC $@
	$(Q) $(HOSTCC) $(HOSTCPPFLAGS) $(HOSTCFLAGS) $(toolincdirs) -c -o $@ $<

$(TGT)/scp.bin: $(TGT)/scp.elf | $(TGT)
	$(M) OBJCOPY $@
	$(Q) $(OBJCOPY) -O binary -S --reverse-bytes 4 $< $@

$(TGT)/scp.elf: $(TGT)/scp.ld $(fwobjects) | $(TGT)
	$(M) CCLD $@
	$(Q) $(CC) $(CFLAGS) $(LDFLAGS) \
		-Wl,-Map,$(TGT)/$*.map -o $@ -T $^

$(TGT)/scp.ld: $(SRC)/scripts/scp.ld.S $(fwheaders) | $(TGT)
	$(M) CPP $@
	$(Q) $(CPP) $(CPPFLAGS) $(fwincdirs) -o $@ -P $<

$(TGT)/scp.map: $(TGT)/scp.elf;

$(TGT)/%.o: $(SRC)/%.c $(fwheaders) | $(fwobjdirs)
	$(M) CC $@
	$(Q) $(CC) $(CPPFLAGS) $(CFLAGS) $(fwincdirs) -c -o $@ $<

$(TGT)/%.o: $(SRC)/%.S $(fwheaders) | $(fwobjdirs)
	$(M) CC $@
	$(Q) $(CC) $(CPPFLAGS) $(CFLAGS) $(fwincdirs) -c -o $@ $<

.PHONY: all check check-format clean clobber distclean format scp tools
.SECONDARY:
.SUFFIXES:
