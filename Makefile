#
# Copyright © 2017-2018 The Crust Firmware Authors.
# SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
#

CROSS_COMPILE	?= or1k-linux-musl-
AR		 = $(CROSS_COMPILE)ar
CC		 = $(CROSS_COMPILE)gcc
CPP		 = $(CROSS_COMPILE)cpp
OBJCOPY		 = $(CROSS_COMPILE)objcopy

WARNINGS	 = -Wall -Wextra -Wformat=2 -Wshadow \
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

srcdir		 = .
objdir		 = build
platdir		 = platform/$(CONFIG_PLATFORM)

generated	 = $(objdir)/include/config.h

files		 = $(sort $(wildcard $(foreach x,$2,$(1:%=$(srcdir)/%/$x))))
incdirs		 = $(addprefix -I,$(sort $(objdir)/include $(wildcard $1)))
headers		 = $(call files,$1,*.h */*.h *.S) $(generated)
sources		 = $(call files,$1,*.c *.S)
objdirs		 = $(sort $(patsubst %/,%,$(dir $1)))
objects		 = $(patsubst $(srcdir)$2/%,$(objdir)$3/%$4,$(basename $1))

format-filter	 = $(filter-out $(objdir)/% %.S,$1)
formatheaders	 = $(call headers,include/* platform/*/include)
formatsources	 = $(call sources,common drivers/* lib platform/* test tools)
formatfiles	 = $(call format-filter,$(formatheaders) $(formatsources))

fwincbase	 = $(platdir)/include include/*
fwincdirs	 = $(call incdirs,$(fwincbase))
fwheaders	 = $(call headers,$(fwincbase))
fwsources	 = $(call sources,common drivers/* lib $(platdir))
fwobjects	 = $(call objects,$(fwsources),,,.o)
fwobjdirs	 = $(call objdirs,$(fwobjects))
fwfiles		 = $(addprefix $(objdir)/,scp.bin scp.elf scp.map)

libincbase	 = include/lib
libincdirs	 = $(call incdirs,$(libincbase))
libheaders	 = $(call headers,$(libincbase))
libsources	 = $(call files,lib,*.c)
libobjects	 = $(call objects,$(libsources),,/host,.o)
libobjdirs	 = $(call objdirs,$(libobjects))
library		 = $(objdir)/host/lib/libcrust.a

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

allobjdirs	 = $(objdir) $(objdir)/include $(fwobjdirs) $(libobjdirs) \
		   $(testobjdirs) $(toolobjdirs)

ifeq ($(MAKECMDGOALS),)
include $(objdir)/config.mk
else
ifneq ($(filter-out %clean %config %format,$(MAKECMDGOALS)),)
include $(objdir)/config.mk
endif
endif

M := @$(if $(filter-out 0,$(V)),:,printf '  %-7s %s\n')
Q :=  $(if $(filter-out 0,$(V)),,@)

all: $(fwfiles) $(tests) $(tools)

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

$(objdir)/%.bin: $(objdir)/%.elf | $(objdir)
	$(M) OBJCOPY $@
	$(Q) $(OBJCOPY) -O binary -S --reverse-bytes 4 $< $@

$(objdir)/%.elf $(objdir)/%.map: $(objdir)/%.ld $(fwobjects) | $(objdir)
	$(M) CCLD $@
	$(Q) $(CC) $(CFLAGS) $(LDFLAGS) \
		-Wl,-Map,$(objdir)/$*.map -o $(objdir)/$*.elf -T $^

$(objdir)/%.ld: $(srcdir)/scripts/%.ld.S $(fwheaders) | $(objdir)
	$(M) CPP $@
	$(Q) $(CPP) $(CPPFLAGS) $(fwincdirs) -o $@ -P $<

$(objdir)/%.o: $(srcdir)/%.c $(fwheaders) | $(fwobjdirs)
	$(M) CC $@
	$(Q) $(CC) $(CPPFLAGS) $(CFLAGS) $(fwincdirs) -c -o $@ $<

$(objdir)/%.o: $(srcdir)/%.S $(fwheaders) | $(fwobjdirs)
	$(M) CC $@
	$(Q) $(CC) $(CPPFLAGS) $(CFLAGS) $(fwincdirs) -c -o $@ $<

$(objdir)/config.mk: .config | $(objdir)
	$(Q) sed 's/#.*$$//;s/="\(.*\)"$$/=\1/' $< > $@

$(objdir)/include/config.h: .config | $(objdir)/include
	$(Q) sed -n 's/#.*$$//;s/^\([^=]\+\)=\(.*\)$$/#define \1 \2/p' $< > $@

$(library): $(libobjects) | $(libobjdirs)
	$(M) HOSTAR $@
	$(Q) $(HOSTAR) rcs $@ $^

$(objdir)/host/lib/%.o: $(srcdir)/lib/%.c $(libheaders) | $(libobjdirs)
	$(M) HOSTCC $@
	$(Q) $(HOSTCC) $(HOSTCPPFLAGS) $(HOSTCFLAGS) $(libincdirs) -c -o $@ $<

$(objdir)/host/test/%.test: $(objdir)/host/test/%
	$(M) TEST $@
	$(Q) $< > $@.tmp && mv -f $@.tmp $@ || { cat $@.tmp; rm -f $@.tmp; }

$(objdir)/host/test/%: $(objdir)/host/test/%.o $(unityobjects) $(library)
	$(M) HOSTLD $@
	$(Q) $(HOSTCC) $(HOSTCFLAGS) $(HOSTLDFLAGS) -o $@ $^ $(HOSTLIBS)

$(objdir)/host/test/%.o: $(srcdir)/3rdparty/%.c $(testheaders) | $(testobjdirs)
	$(M) HOSTCC $@
	$(Q) $(HOSTCC) $(HOSTCPPFLAGS) $(HOSTCFLAGS) $(testincdirs) -c -o $@ $<

$(objdir)/host/test/%.o: $(srcdir)/test/%.c $(testheaders) | $(testobjdirs)
	$(M) HOSTCC $@
	$(Q) $(HOSTCC) $(HOSTCPPFLAGS) $(HOSTCFLAGS) $(testincdirs) -c -o $@ $<

$(objdir)/host/tools/%: $(srcdir)/tools/%.c $(toolheaders) | $(toolobjdirs)
	$(M) HOSTCC $@
	$(Q) $(HOSTCC) $(HOSTCPPFLAGS) $(HOSTCFLAGS) $(HOSTLDFLAGS) \
		$(toolincdirs) -o $@ $< $(HOSTLIBS)

.PHONY: all check check-format clean distclean firmware format test tools
.SECONDARY:
.SUFFIXES:
