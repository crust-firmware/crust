#
# Copyright © 2017-2018 The Crust Firmware Authors.
# SPDX-License-Identifier: GPL-2.0-only
#
# Based in part on scripts/kconfig/Makefile from Linux v4.19.
#

ccflags-y	+= -std=gnu11 \
		   -Wno-implicit-fallthrough \
		   -Wno-pedantic \
		   -Wno-shadow \
		   -Wno-sign-compare \
		   -Wno-unused-parameter \
		   -Wno-vla
cppflags-y	+= -I$(src)

HOSTCFLAGS_nconf := $(shell \
	pkg-config --cflags menuw panelw ncursesw 2>/dev/null \
	|| pkg-config --cflags menu panel ncurses 2>/dev/null \
	|| echo "-D_GNU_SOURCE -I/usr/include/ncurses")
HOSTLDLIBS_nconf := $(shell \
	pkg-config --libs menuw panelw ncursesw 2>/dev/null \
	|| pkg-config --libs menu panel ncurses 2>/dev/null \
	|| echo "-lmenu -lpanel -lncurses")

hostprogs-y	+= conf nconf
conf-objs	:= conf.o zconf.tab.o
nconf-objs	:= nconf.o nconf.gui.o zconf.tab.o

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

$(obj)/zconf.tab.o: $(obj)/zconf.lex.c

.PHONY: $(simple-targets) config defconfig %_defconfig nconfig savedefconfig
