#!/bin/bash

set -e

if [ "x$JHBUILD_LIBDIR" = "x" ]
then
	echo "Use: jhbuild shell"
	exit
fi

version=$(git rev-parse --short HEAD)
echo "Using git version: $version"

FILE=configure.ac
sed -i.old -E "s/grisbi_micro_version, ([0-9]+)/grisbi_micro_version, \1-$version/" $FILE

./autogen.sh
./configure --prefix=$PREFIX --with-openssl --with-ofx --with-goffice --with-libxml2
make clean
make dist
make
make install
rm -f MacOS/Grisbi-*.dmg
make bundle

# restore configure.ac
mv $FILE.old $FILE
