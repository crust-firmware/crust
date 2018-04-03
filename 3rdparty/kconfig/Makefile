#
# Copyright © 2017-2018 The Crust Firmware Authors.
# Adapted from Linux's scripts/kconfig/Makefile.
# SPDX-License-Identifier: GPL-2.0
#

silent := $(if $(filter-out 0,$(V)),,-s)

# These targets map 1:1 to the command line options of 'conf'.
simple-targets := \
	alldefconfig allmodconfig allnoconfig allyesconfig defconfig \
	listnewconfig oldconfig olddefconfig randconfig

$(simple-targets): $(hostobj)/conf
	$(Q) $< $(silent) --$@ $(SRC)/Kconfig

config: $(hostobj)/conf
	$(Q) $< $(silent) --oldaskconfig $(SRC)/Kconfig

%_defconfig: $(hostobj)/conf
	$(Q) $< $(silent) --defconfig=$(SRC)/board/$@ $(SRC)/Kconfig

savedefconfig: $(hostobj)/conf
	$(Q) $< $(silent) --$@=defconfig $(SRC)/Kconfig

nconfig: $(hostobj)/nconf
	$(Q) $< $(silent) $(SRC)/Kconfig

hostprogs-y += conf nconf
conf-objs := zconf.tab.o
nconf-objs := nconf.gui.o zconf.tab.o

$(hostobj)/zconf.tab.o: $(hostobj)/zconf.lex.c

$(hostobj)/nconf: HOSTLIBS += $(shell \
	pkg-config --libs menuw panelw ncursesw 2>/dev/null \
	|| pkg-config --libs menu panel ncurses 2>/dev/null \
	|| echo "-lmenu -lpanel -lncurses")

$(call add-headers,$(hostobj)/%.o,$(subdir))
$(hostobj)/%.o: HOSTCPPFLAGS += -DKBUILD_NO_NLS
$(hostobj)/%.o: HOSTCFLAGS += -std=gnu11 \
	-Wno-format-nonliteral -Wno-pedantic -Wno-shadow -Wno-sign-compare \
	-Wno-unused-parameter -Wno-vla

.PHONY: $(simple-targets) config %_defconfig savedefconfig nconfig
