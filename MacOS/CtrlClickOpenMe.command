#!/bin/bash

#    CtrlClickOpenMe: script to remove the quarantine attribute
#    Copyright (C) 2025  Ludovic Rousseau
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

# fail on first error
set -o errexit

# fail on pipe error
set -o pipefail

#set -x

DIR=$(dirname "$0")
echo "$DIR"

if [ -z "$1" ]
then
	echo "respawn"
	DEST="$(mktemp)"
	cp "$0" "$DEST"
	chmod +x "$DEST"
	exec "$DEST" "$DIR"
fi

DIR="$1"
echo "normal run: $DIR"

# fail on undefined variable
set -o nounset

# Get DMG image file and mount point
declare -a VOLUMES
declare -a MOUNT_POINTS

HDINFO=$(mktemp)
hdiutil info -plist > "$HDINFO"

IFS=$'\n' read -r -d '' -a VOLUMES < <(xmllint --xpath '//key[text()="image-path"]//following::string[1]/text()' "$HDINFO") || true

IFS=$'\n' read -r -d '' -a MOUNT_POINTS < <(xmllint --xpath '//key[text()="mount-point"]//following::string[1]/text()' "$HDINFO") || true

DMG=""
for i in "${!VOLUMES[@]}"
do
	echo "$i"
	echo "${VOLUMES[$i]}"
	echo "${MOUNT_POINTS[$i]}"
	if [ "$DIR" = "${MOUNT_POINTS[$i]}" ]
	then
		echo "found"
		DMG="${VOLUMES[$i]}"
		MOUNT_POINT="${MOUNT_POINTS[$i]}"
	fi
done

rm "$HDINFO"

if [ -z "$DMG" ]
then
	echo "DMG not found"
	exit 1
fi

echo "DMG: $DMG"
echo "MOUNT_POINT: $MOUNT_POINT"

if ls -l@ "$DMG" | grep com.apple.quarantine &> /dev/null
then
	echo "Found quarantine flag"
else
	echo "Nothing to do"
	osascript -e 'display dialog "Nothing to do here.\nInstall Grisbi as usual." buttons { "OK" } default button "OK"'
	exit
fi

# unmout
hdiutil detach "$MOUNT_POINT"

# Remove quarantine attribute
ls -l@ "$DMG"
xattr -d com.apple.quarantine "$DMG" || true

# mount
hdiutil attach -autoopen "$DMG"

osascript -e 'display dialog "You can now install the Grisbi application" buttons { "OK" } default button "OK"'
