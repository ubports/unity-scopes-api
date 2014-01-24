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

def build_header(outfile, incroots):
    ofile = open(outfile, 'w')
    headers = []
    for r in incroots:
        headers += glob(os.path.join(r, 'unity/scopes', '*.h'))
    headers = [os.path.split(f)[1] for f in headers]
    headers.sort()
    ofile.write("#ifndef UNITY_SCOPES_H\n")
    ofile.write("#define UNITY_SCOPES_H\n")
    for f in headers:
        line = '#include<unity/scopes/%s>\n' % f
        ofile.write(line)
    ofile.write("#endif\n")

if __name__ == '__main__':
    if len(sys.argv) <= 3:
        print(sys.argv[0], 'outfile include_roots')
        sys.exit(1)
    outfile = sys.argv[1]
    incroots = sys.argv[2:]
    build_header(outfile, incroots)
