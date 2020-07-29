#!/bin/sh

# sortie si erreur
set -e

echo "configuration de grisbi"
version=$(git rev-parse --short HEAD)
echo "Using git version: $version"

FILE=configure.ac
sed -i.old -E "s/GRISBI_GIT_HASH/$version/" $FILE

./autogen.sh
./configure --with-openssl --with-ofx --with-goffice --with-libxml2 $@
make clean

echo "création du package de Grisbi"
make dist

# restore configure.ac
echo restauration du fichier configure.ac
mv $FILE.old $FILE

