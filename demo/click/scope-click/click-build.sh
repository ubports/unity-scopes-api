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
# Authored by: Michi Henning <michi.henning@canonical.com>

# Simple script to run the code base through astyle, followed by clang-format (which
# undoes some damage that's done by astyle, without wiping out astyle edits we want
# to happen).
#
# If either program makes a mess of some file such that it won't compile anymore
# or otherwise gets adorned with unacceptable edits, add the file to the list
# of files to filter out (grep -v below).


export LC_ALL=C

BZR_SOURCE=${1:-lp:scope-click}

CLICKARCH=amd64
rm -rf $CLICKARCH-build
mkdir $CLICKARCH-build
cd $CLICKARCH-build
cmake .. \
    -DCMAKE_INSTALL_PREFIX:PATH=../package \
    -DCLICK_MODE=on \
    -DBZR_REVNO=$(cd ..; bzr revno) \
    -DBZR_SOURCE="$BZR_SOURCE"
make install
cd ..
click build package
