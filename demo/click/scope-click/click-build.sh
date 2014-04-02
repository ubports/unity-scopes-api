#!/bin/sh

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
