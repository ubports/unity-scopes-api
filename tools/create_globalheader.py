#!/usr/bin/env python3
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


from glob import glob
import sys, os

def build_header(inc_root, outfile):
    ofile = open(outfile, 'w')
    headers = glob(os.path.join(inc_root, 'unity/scopes', '*.h'))
    headers.sort()
    for f in headers:
        base = os.path.split(f)[1]
        line = '#include<unity/scopes/%s>\n' % base
        ofile.write(line)

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print(sys.argv[0], 'include_root outfile')
        sys.exit(1)
    incroot = sys.argv[1]
    outfile = sys.argv[2]
    build_header(incroot, outfile)
