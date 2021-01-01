#!/bin/sh -eu
#
# Copyright © 2020-2021 The Crust Firmware Authors.
# SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
#

major=0
minor=3
patch=10000

srcdir=$1
output=$2

if test "$srcdir" = "$(git -C "$srcdir" rev-parse --show-toplevel 2>/dev/null)"; then
  set -- $(git -C "$srcdir" describe --dirty --long --match 'v[0-9]*' | tr '.-' '  ')

  if test "$#" -ge 4; then
    major=${1#v}
    minor=$2
    patch=$3
  fi

  if test "$#" -ge 5 && test "$5" = "dirty"; then
    patch=$((patch+10000))
  fi
fi

trap 'rm -f "$output.tmp"' EXIT

cat > "$output.tmp" << EOF
#define VERSION_MAJOR $major
#define VERSION_MINOR $minor
#define VERSION_PATCH $patch
EOF

if ! test -f "$output" ||
   ! test "$(cat "$output.tmp")" = "$(cat "$output")"; then
  mv -f "$output.tmp" "$output"
fi
