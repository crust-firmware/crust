#
# Copyright © 2017-2018 The Crust Firmware Authors.
# SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
#

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
		   $(addprefix -I,$(incdirs)) \
		   -nostdinc \
		   -Werror=missing-include-dirs
LDFLAGS		 = -nostdlib \
		   -Wl,-O1 \
		   -Wl,--build-id=none \
		   -Wl,--fatal-warnings \
		   -Wl,--gc-sections \
		   -Wl,--no-undefined

HOSTCC		 = cc
HOSTCFLAGS	 = -O2 -pipe -std=c11 \
		   $(WARNINGS)
HOSTCPPFLAGS	 = $(addprefix -I,$(toolincdirs))
HOSTLDFLAGS	 =
HOSTLIBS	 =

pathjoin	 = $(foreach X,$2,$(addsuffix /$X,$1))

incdirs		 = $(objdir)/include $(platdir)/include $(srcdir)/include
incpatterns	 = *.h */*.h */*/*.h *.S
includes	 = $(wildcard $(call pathjoin,$(incdirs),$(incpatterns))) \
		   $(objdir)/include/config.h

platdir		 = $(srcdir)/platform/$(CONFIG_PLATFORM)

srcdir		 = .
srcdirs		 = $(addprefix $(srcdir)/,common drivers/* lib) $(platdir)
srcpatterns	 = *.c *.S
sources		 = $(wildcard $(call pathjoin,$(srcdirs),$(srcpatterns)))

objdir		 = build
objdirs		 = $(sort $(dir $(objects)))
objects		 = $(patsubst $(srcdir)/%,$(objdir)/%.o,$(basename $(sources)))
outputs		 = $(addprefix $(objdir)/,scp.bin scp.elf scp.map)

fmtincdirs	 = $(addprefix $(srcdir)/,include platform/*/include)
fmtincludes	 = $(wildcard $(call pathjoin,$(fmtincdirs),*.h */*.h */*/*.h))
fmtsrcdirs	 = $(addprefix $(srcdir)/,common drivers/* lib platform/* tools)
fmtsources	 = $(wildcard $(call pathjoin,$(fmtsrcdirs),*.c))

toolincdirs	 = $(objdir)/include $(platdir)/include
toolincludes	 = $(wildcard $(call pathjoin,$(toolincdirs),$(incpatterns))) \
		   $(objdir)/include/config.h
toolsources	 = $(wildcard $(srcdir)/tools/*.c)
tools		 = $(patsubst $(srcdir)/%.c,$(objdir)/%,$(toolsources))

ifeq ($(MAKECMDGOALS),)
include $(objdir)/config.mk
else
ifneq ($(filter-out %clean %config %format,$(MAKECMDGOALS)),)
include $(objdir)/config.mk
endif
endif

M := @$(if $(filter-out 0,$(V)),:,printf '  %-7s %s\n')
Q :=  $(if $(filter-out 0,$(V)),,@)

all: $(outputs) $(tools)

check:

check-format: $(fmtincludes) $(fmtsources)
	$(Q) uncrustify -c $(srcdir)/.uncrustify -l C -q --check $^

clean:
	$(Q) rm -fr $(objdir)

%_defconfig:
	$(Q) cp -f $(srcdir)/board/$* .config

distclean:
	$(Q) rm -fr $(objdir) .config

firmware: $(outputs)

format: $(fmtincludes) $(fmtsources)
	$(Q) uncrustify -c $(srcdir)/.uncrustify -l C -q --no-backup $^

tools: $(tools)

$(objdir) $(objdirs) $(objdir)/include $(objdir)/tools:
	$(Q) mkdir -p $@

$(objdir)/%.bin: $(objdir)/%.elf | $(objdir)
	$(M) OBJCOPY $@
	$(Q) $(OBJCOPY) -O binary -S --reverse-bytes 4 $< $@

$(objdir)/%.elf $(objdir)/%.map: $(objdir)/%.ld $(objects) | $(objdir)
	$(M) CCLD $@
	$(Q) $(CC) $(CFLAGS) $(LDFLAGS) \
		-Wl,-Map,$(objdir)/$*.map -o $(objdir)/$*.elf -T $^

$(objdir)/%.ld: $(srcdir)/scripts/%.ld.S $(incdirs) $(includes) | $(objdir)
	$(M) CPP $@
	$(Q) $(CPP) $(CPPFLAGS) -o $@ -P $<

$(objdir)/%.o: $(srcdir)/%.c $(incdirs) $(includes) | $(objdirs)
	$(M) CC $@
	$(Q) $(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(objdir)/%.o: $(srcdir)/%.S $(incdirs) $(includes) | $(objdirs)
	$(M) CC $@
	$(Q) $(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(objdir)/config.mk: .config | $(objdir)
	$(Q) sed 's/#.*$$//;s/="\(.*\)"$$/=\1/' $< > $@

$(objdir)/include/config.h: .config | $(objdir)/include
	$(Q) sed -n 's/#.*$$//;s/^\([^=]\+\)=\(.*\)$$/#define \1 \2/p' $< > $@

$(objdir)/%: $(srcdir)/%.c $(toolincdirs) $(toolincludes) | $(objdir)/tools
	$(M) HOSTCC $@
	$(Q) $(HOSTCC) $(HOSTCPPFLAGS) $(HOSTCFLAGS) $(HOSTLDFLAGS) \
		-o $@ $< $(HOSTLIBS)

.PHONY: all check check-format clean distclean firmware format tools
.SECONDARY:
.SUFFIXES:
