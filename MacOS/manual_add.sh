#! /bin/bash

# Script to add files not automatically detected

set -e

DEST=MacOS/dist/Grisbi.app/Contents/Resources

EXTRA_FILES="share/icons/Adwaita/scalable/actions/pan-down-symbolic.svg
share/icons/Adwaita/scalable/actions/pan-end-symbolic-rtl.svg
share/icons/Adwaita/scalable/actions/pan-end-symbolic.svg
share/icons/Adwaita/scalable/actions/pan-start-symbolic-rtl.svg
share/icons/Adwaita/scalable/actions/pan-start-symbolic.svg
share/icons/Adwaita/scalable/actions/pan-up-symbolic.svg"

if [ -z "$PREFIX" ]
then
	echo "Run inside jhbuild shell"
	exit 1
fi

for file in $EXTRA_FILES
do
	echo $file
	mkdir -p $DEST/$(dirname $file)
	cp $PREFIX/$file $DEST/$file
done
