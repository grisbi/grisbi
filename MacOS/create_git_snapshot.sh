#!/bin/bash

set -e

if [ "x$JHBUILD_LIBDIR" = "x" ]
then
	echo "Use: jhbuild shell"
	exit
fi

share/create_git_distrib.sh --prefix=$PREFIX --enable-config-file

make
make install
rm -f MacOS/Grisbi-*.dmg
make bundle
