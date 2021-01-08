#!/bin/sh

# sortie si erreur
set -e

echo "configuration de grisbi"
version=$(git rev-parse --short HEAD)
echo "Using git version: $version"

FILE=configure.ac
sed -i.old -E "s/GRISBI_GIT_HASH/$version/" $FILE

OPTIONS=""
OPTIONS+=" --with-openssl"
OPTIONS+=" --with-ofx"
OPTIONS+=" --with-libxml2"
OPTIONS+=" --with-goffice"

# plateform specific options
case $(uname) in
	Darwin)
		# OPTIONS+=" --without-goffice"
		;;

	Linux)
		# OPTIONS+=" --with-goffice"
		;;
esac

./autogen.sh
./configure $OPTIONS $@
make clean

echo "création du package de Grisbi"
make dist

# restore configure.ac
echo restauration du fichier configure.ac
mv $FILE.old $FILE

