#!/bin/bash

# fail on first error
set -e

# fail on undefined variable
set -u

# debug
set -x

if [ -z "$JHBUILD_LIBDIR" ]
then
	echo "Use: jhbuild shell"
	exit
fi

BUILD_DIR="snapshot"
rm -rf "$BUILD_DIR"

meson setup "$BUILD_DIR" --prefix="$PREFIX" "$@"

meson compile -C "$BUILD_DIR"
meson install -C "$BUILD_DIR"

rm -f MacOS/Grisbi-*.dmg
rm -rf MacOS/dist

# extract version from config.h
GRISBI_VERSION=$(grep VERSION $BUILD_DIR/config.h | cut -f 3 -d ' ' | tr -d '"')

sed -e "s/VERSION/$GRISBI_VERSION/" MacOS/Info.plist.in > MacOS/Info.plist

GRIBSI_BUNDLE_PATH=. gtk-mac-bundler MacOS/Grisbi.bundle

./MacOS/manual_add.sh

./MacOS/create-dmg.sh \
	--volname Grisbi \
	--volicon MacOS/Grisbi.icns \
	--window-size 640 400 \
	--background MacOS/background.png \
	--icon-size 96 \
	--app-drop-link 500 250 \
	--icon "Grisbi.app" 150 250 \
   	MacOS/Grisbi-"$GRISBI_VERSION".dmg \
	MacOS/dist

rm -r "$BUILD_DIR"
