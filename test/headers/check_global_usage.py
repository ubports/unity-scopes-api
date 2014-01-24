#! /usr/bin/env python3

#
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
# Authored by: Jussi Pakkanen <jussi.pakkanen@canonical.com>

import os, sys

def check_headers(subdirs):
    errors = 0
    for d in subdirs:
        for root, dirs, files in os.walk(d):
            for f in files:
                if f.endswith('.h'):
                    fname = os.path.join(root, f)
                    if 'unity-scopes.h' in open(fname).read():
                        print("Global header included in", fname)
                        errors += 1
    return errors

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print(sys.argv[0], "header directories to check")
        sys.exit(1)
    sys.exit(check_headers(sys.argv[1:]))
