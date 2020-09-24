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

# Build this project OUTSIDE of jhbuild or the link will fail with:
# ld: unknown option: -target

# xcodebuild -project Grisbi\ Launcher/Grisbi\ Launcher.xcodeproj clean
# xcodebuild -project Grisbi\ Launcher/Grisbi\ Launcher.xcodeproj

mv dist/Grisbi.app dist/Grisbi.app.real

cp -a Grisbi\ Launcher/build/Release/Grisbi.app dist/
mv dist/Grisbi.app.real dist/Grisbi.app/Contents/Resources/Grisbi.app
