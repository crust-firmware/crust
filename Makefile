#
# Copyright © 2017-2018 The Crust Firmware Authors.
# SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
#

CROSS_COMPILE	?= or1k-linux-musl-
AR		 = $(CROSS_COMPILE)ar
CC		 = $(CROSS_COMPILE)gcc
CPP		 = $(CROSS_COMPILE)cpp
OBJCOPY		 = $(CROSS_COMPILE)objcopy

HAVE_GCC9	:= $(findstring version 9,$(shell $(CC) -v 2>&1;:))

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
		   -fno-pie \
		   -fomit-frame-pointer \
		   -funsigned-char \
		   -g$(if $(filter-out 0,$(DEBUG)),gdb,0) \
		   -mhard-mul -msoft-div \
		   $(if $(HAVE_GCC9),-msext -msfimm -mshftimm) \
		   -static \
		   -Wa,--fatal-warnings \
		   $(WARNINGS)
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

HOSTAR		 = ar
HOSTCC		 = cc
HOSTCFLAGS	 = -fno-builtin \
		   -O2 -pipe -std=c11 \
		   $(WARNINGS)
HOSTCPPFLAGS	 = -D_XOPEN_SOURCE=700
HOSTLDFLAGS	 =
HOSTLIBS	 =

srcdir		 = .
objdir		 = build
platdir		 = platform/$(CONFIG_PLATFORM)

generated	 = $(objdir)/include/config.h

files		 = $(sort $(wildcard $(foreach x,$2,$(1:%=$(srcdir)/%/$x))))
incdirs		 = $(addprefix -I,$(sort $(objdir)/include $(wildcard $1)))
headers		 = $(call files,$1,*.h */*.h *.S) $(generated)
sources		 = $(call files,$1,*.c *.S)
objdirs		 = $(sort $(patsubst %/,%,$(dir $1)))
objects		 = $(patsubst $(srcdir)/%,$(objdir)$2/%$3,$(basename $1))

format-filter	 = $(filter-out $(objdir)/% %.S,$1)
formatheaders	 = $(call headers,include/* platform/*/include)
formatsources	 = $(call sources,common drivers/* lib platform/* test tools)
formatfiles	 = $(call format-filter,$(formatheaders) $(formatsources))

fwincbase	 = $(platdir)/include include/*
fwincdirs	 = $(call incdirs,$(fwincbase))
fwheaders	 = $(call headers,$(fwincbase))
fwsources	 = $(call sources,common drivers/* lib $(platdir))
fwobjects	 = $(call objects,$(fwsources),/scp,.o)
fwobjdirs	 = $(call objdirs,$(fwobjects))
fwfiles		 = $(addprefix $(objdir)/scp/,scp.bin scp.elf scp.map)

libincbase	 = include/lib
libincdirs	 = $(call incdirs,$(libincbase))
libheaders	 = $(call headers,$(libincbase))
libsources	 = $(call files,lib,*.c)
libobjects	 = $(call objects,$(libsources),,.o)
libobjdirs	 = $(call objdirs,$(libobjects))
library		 = $(objdir)/lib/libcrust.a

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

allobjdirs	 = $(objdir) $(objdir)/include $(objdir)/scp \
		   $(fwobjdirs) $(libobjdirs) $(testobjdirs) $(toolobjdirs)

ifeq ($(MAKECMDGOALS),)
include $(objdir)/config.mk
else
ifneq ($(filter-out %clean %config %format,$(MAKECMDGOALS)),)
include $(objdir)/config.mk
endif
endif

M := @$(if $(filter-out 0,$(V)),:,printf '  %-7s %s\n')
Q :=  $(if $(filter-out 0,$(V)),,@)

all: $(fwfiles) $(tests)

check: $(testresults)

check-format: $(formatfiles)
	$(Q) uncrustify -c $(srcdir)/.uncrustify -l C -q --check $^

clean:
	$(Q) rm -fr $(objdir)

%_defconfig:
	$(Q) cp -f $(srcdir)/board/$* .config

distclean:
	$(Q) rm -fr $(objdir) .config

firmware: $(fwfiles)

format: $(formatfiles)
	$(Q) uncrustify -c $(srcdir)/.uncrustify -l C -q --no-backup $^

test: check

tools: $(tools)

$(allobjdirs):
	$(Q) mkdir -p $@

$(objdir)/scp/scp.bin: $(objdir)/scp/scp.elf | $(objdir)/scp
	$(M) OBJCOPY $@
	$(Q) $(OBJCOPY) -O binary -S --reverse-bytes 4 $< $@

$(objdir)/scp/scp.elf: $(objdir)/scp/scp.ld $(fwobjects) | $(objdir)/scp
	$(M) CCLD $@
	$(Q) $(CC) $(CFLAGS) $(LDFLAGS) \
		-Wl,-Map,$(objdir)/scp/$*.map -o $@ -T $^

$(objdir)/scp/scp.ld: $(srcdir)/scripts/scp.ld.S $(fwheaders) | $(objdir)/scp
	$(M) CPP $@
	$(Q) $(CPP) $(CPPFLAGS) $(fwincdirs) -o $@ -P $<

$(objdir)/scp/scp.map: $(objdir)/scp/scp.elf;

$(objdir)/scp/%.o: $(srcdir)/%.c $(fwheaders) | $(fwobjdirs)
	$(M) CC $@
	$(Q) $(CC) $(CPPFLAGS) $(CFLAGS) $(fwincdirs) -c -o $@ $<

$(objdir)/scp/%.o: $(srcdir)/%.S $(fwheaders) | $(fwobjdirs)
	$(M) CC $@
	$(Q) $(CC) $(CPPFLAGS) $(CFLAGS) $(fwincdirs) -c -o $@ $<

$(objdir)/config.mk: .config | $(objdir)
	$(Q) sed 's/#.*$$//;s/="\(.*\)"$$/=\1/' $< > $@

$(objdir)/include/config.h: .config | $(objdir)/include
	$(Q) sed -n 's/#.*$$//;s/^\([^=]\+\)=\(.*\)$$/#define \1 \2/p' $< > $@

$(library): $(libobjects) | $(libobjdirs)
	$(M) HOSTAR $@
	$(Q) $(HOSTAR) rcs $@ $^

$(objdir)/lib/%.o: $(srcdir)/lib/%.c $(libheaders) | $(libobjdirs)
	$(M) HOSTCC $@
	$(Q) $(HOSTCC) $(HOSTCPPFLAGS) $(HOSTCFLAGS) $(libincdirs) -c -o $@ $<

$(objdir)/test/%.test: $(objdir)/test/%
	$(M) TEST $@
	$(Q) $< > $@.tmp && mv -f $@.tmp $@ || { cat $@.tmp; rm -f $@.tmp; }

$(objdir)/test/%: $(objdir)/test/%.o $(unityobjects) $(library)
	$(M) HOSTLD $@
	$(Q) $(HOSTCC) $(HOSTCFLAGS) $(HOSTLDFLAGS) -o $@ $^ $(HOSTLIBS)

$(objdir)/3rdparty/%.o: $(srcdir)/3rdparty/%.c $(testheaders) | $(testobjdirs)
	$(M) HOSTCC $@
	$(Q) $(HOSTCC) $(HOSTCPPFLAGS) $(HOSTCFLAGS) $(testincdirs) -c -o $@ $<

$(objdir)/test/%.o: $(srcdir)/test/%.c $(testheaders) | $(testobjdirs)
	$(M) HOSTCC $@
	$(Q) $(HOSTCC) $(HOSTCPPFLAGS) $(HOSTCFLAGS) $(testincdirs) -c -o $@ $<

$(objdir)/tools/%: $(objdir)/tools/%.o $(library)
	$(M) HOSTLD $@
	$(Q) $(HOSTCC) $(HOSTCFLAGS) $(HOSTLDFLAGS) -o $@ $^ $(HOSTLIBS)

$(objdir)/tools/%.o: $(srcdir)/tools/%.c $(toolheaders) | $(toolobjdirs)
	$(M) HOSTCC $@
	$(Q) $(HOSTCC) $(HOSTCPPFLAGS) $(HOSTCFLAGS) $(toolincdirs) -c -o $@ $<

.PHONY: all check check-format clean distclean firmware format test tools
.SECONDARY:
.SUFFIXES:
