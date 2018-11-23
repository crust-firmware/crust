#!/bin/sh
#
# Copyright © 2017-2018 The Crust Firmware Authors.
# SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
#

cpu_disable() {
  printf 0 > "$1"/online
  if [ "$(cat "$1"/hotplug/state)" -ne 0 ] ||
     [ "$(cat "$1"/hotplug/fail)" -ne -1 ]; then
    exit 1
  fi
}

cpu_enable() {
  printf 1 > "$1"/online
  if [ "$(cat "$1"/hotplug/state)" -eq 0 ] ||
     [ "$(cat "$1"/hotplug/fail)" -ne -1 ]; then
    exit 1
  fi
}

# Disable and enable every CPU core.
for cpu in /sys/bus/cpu/devices/cpu*; do
  cpu_disable "$cpu"
  cpu_enable "$cpu"
done

# Disable and enable CPU cores in pairs of two and three.
for cpu0 in /sys/bus/cpu/devices/cpu*; do
  for cpu1 in /sys/bus/cpu/devices/cpu*; do
    for cpu2 in /sys/bus/cpu/devices/cpu*; do
      # Skip cycling a single core.
      if [ "$cpu1" = "$cpu2" ]; then
        continue
      fi
      cpu_disable "$cpu0"
      cpu_disable "$cpu1"
      cpu_disable "$cpu2"
      cpu_enable "$cpu0"
      cpu_enable "$cpu1"
      cpu_enable "$cpu2"
    done
  done
done

exit 0
