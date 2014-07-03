#! /usr/bin/env python

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
# Authored by: Michi Henning <michi.henning@canonical.com>
#

import argparse
import os
import sys
import u1db

def run():
    parser = argparse.ArgumentParser(description = 'Add a document to a u1db database')
    parser.add_argument('db', nargs = 1, help = 'The path to the u1db database file')
    parser.add_argument('doc_id', nargs = 1, help = 'The document ID of the entry to be added')
    parser.add_argument('json', nargs = 1, help = 'The JSON string for that document')
    args = parser.parse_args()

    db = u1db.open(args.db[0], create=True)

    db.create_doc_from_json(args.json[0], args.doc_id[0])

if __name__ == '__main__':
   run()
