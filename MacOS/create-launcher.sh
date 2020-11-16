#! /bin/bash

# Script to add build the launcher and include Grisbi.app

# On macOS Catalina and later access to Documents, Desktop & Downloads
# folders is not allowed by default. On the first access a dialog is
# displayed by the system.
# Unfortunately the dialog is generated only when a Coca application
# is running. For the Gtk+3 application we need to use a Cocoa launcher
# that will "generate" the dialog and get the access rights.

set -e

cd $(dirname $0)

# sign Grisbi
codesign --deep -s -  dist/Grisbi.app || true

# undefine LD redifined by "jhbuild shell"
export -n LD

xcodebuild -project Grisbi\ Launcher/Grisbi\ Launcher.xcodeproj clean
xcodebuild -project Grisbi\ Launcher/Grisbi\ Launcher.xcodeproj

rm -rf dist.old
mv dist dist.old
mkdir dist
cp -a Grisbi\ Launcher/build/Release/Grisbi.app dist
