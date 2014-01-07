#!/bin/sh

# Simple script to run the code base through astyle, followed by clang-format (which
# undoes some damage that's done by astyle, without wiping out astyle edits we want
# to happen).
#
# If either program makes a mess of some file such that it won't compile anymore
# or otherwise gets adorned with unacceptable edits, add the file to the list
# of files to filter out (grep -v below).

usage()
{
    echo usage: format-files project_dir astyle_cmd clang_format_cmd 2>&1
    exit 1
}

[ $# -ne 3 ] && usage

dir="$1"
astyle="$2"
format="$3"

files=`find "$dir" -name '*.h' -o -name '*.cpp' -o -name '*.c' \
    | grep -v UnityScopesApi_tp.h`

"$astyle" --options="$dir"/astyle-config $files
[ $? -ne 0 ] && exit $?

"$format" -i -style=file $files
exit $?
