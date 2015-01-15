#!/bin/sh

# Copyright (C) 2014 Canonical Ltd
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Authored by: Pete Woods <pete.woods@canonical.com>

# Simple script to build a click packaged scope.


export LC_ALL=C

if [ "$#" -ne 1 ]; then
    echo "Usage: click-build.sh amd64|armhf|..."
    exit 1
fi

CLICK_ARCH="$1"
rm -rf "$CLICK_ARCH-build"
rm -rf "package"
mkdir "$CLICK_ARCH-build"
(
    cd "$CLICK_ARCH-build"
    cmake .. \
        -DCMAKE_INSTALL_PREFIX:PATH=../package \
        -DCLICK_MODE=on \
        -DCLICK_ARCH="$CLICK_ARCH"
    make install
)
click build package
rm -rf "$CLICK_ARCH-build"
rm -rf "package"
