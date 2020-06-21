#
# Copyright © 2017-2020 The Crust Firmware Authors.
# SPDX-License-Identifier: GPL-2.0-only
#
# Based in part on scripts/kconfig/Makefile from Linux v4.19.
#

ccflags-y	+= -D_GNU_SOURCE \
		   -std=gnu11 \
		   -Wno-implicit-fallthrough \
		   -Wno-pedantic \
		   -Wno-shadow \
		   -Wno-sign-compare \
		   -Wno-unused-parameter \
		   -Wno-vla
cppflags-y	+= -I$(src)

BUILDCFLAGS_nconf := $(shell \
	pkg-config --cflags menuw panelw ncursesw 2>/dev/null \
	|| pkg-config --cflags menu panel ncurses 2>/dev/null \
	|| echo "-D_GNU_SOURCE -I/usr/include/ncurses")
BUILDLDLIBS_nconf := $(shell \
	pkg-config --libs menuw panelw ncursesw 2>/dev/null \
	|| pkg-config --libs menu panel ncurses 2>/dev/null \
	|| echo "-lmenu -lpanel -lncurses")

buildprogs-y	+= conf nconf
common-objs	:= confdata.o expr.o lexer.lex.o parser.tab.o \
		   preprocess.o symbol.o
conf-objs	:= $(common-objs) conf.o
nconf-objs	:= $(common-objs) nconf.o nconf.gui.o

silent := $(if $(filter-out 0,$(V)),,-s)

# These targets map 1:1 to the command line options of 'conf'.
simple-targets := \
	alldefconfig allnoconfig allyesconfig listnewconfig \
	oldconfig olddefconfig randconfig syncconfig

$(simple-targets): $(obj)/conf
	$(Q) $< $(silent) --$@ $(SRC)/Kconfig

config: $(obj)/conf
	$(Q) $< $(silent) --oldaskconfig $(SRC)/Kconfig

defconfig: $(obj)/conf
	$(Q) $< $(silent) --defconfig=$(SRC)/configs/$@ $(SRC)/Kconfig

%_defconfig: $(obj)/conf
	$(Q) $< $(silent) --defconfig=$(SRC)/configs/$@ $(SRC)/Kconfig

nconfig: $(obj)/nconf
	$(Q) $< $(silent) $(SRC)/Kconfig

savedefconfig: $(obj)/conf
	$(Q) $< $(silent) --$@=defconfig $(SRC)/Kconfig

$(obj)/lexer.lex.c: | $(obj)/.
$(obj)/lexer.lex.o: $(obj)/parser.tab.c
$(obj)/parser.tab.c: | $(obj)/.

.PHONY: $(simple-targets) config defconfig %_defconfig nconfig savedefconfig
