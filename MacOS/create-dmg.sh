#! /bin/bash

# Create a read-only disk image of the contents of a folder

set -e;

function pure_version() {
  echo '1.0.0.2'
}

function version() {
  echo "create-dmg $(pure_version)"
}

function usage() {
  version
  echo "Creates a fancy DMG file."
  echo "Usage:  $(basename $0) options... image.dmg source_folder"
  echo "All contents of source_folder will be copied into the disk image."
  echo "Options:"
  echo "  --volname name"
  echo "      set volume name (displayed in the Finder sidebar and window title)"
  echo "  --volicon icon.icns"
  echo "      set volume icon"
  echo "  --background pic.png"
  echo "      set folder background image (provide png, gif, jpg)"
  echo "  --window-pos x y"
  echo "      set position the folder window"
  echo "  --window-size width height"
  echo "      set size of the folder window"
  echo "  --text-size text_size"
  echo "      set window text size (10-16)"
  echo "  --icon-size icon_size"
  echo "      set window icons size (up to 128)"
  echo "  --icon file_name x y"
  echo "      set position of the file's icon"
  echo "  --hide-extension file_name"
  echo "      hide the extension of file"
  echo "  --custom-icon file_name custom_icon_or_sample_file x y"
  echo "      set position and custom icon"
  echo "  --app-drop-link x y"
  echo "      make a drop link to Applications, at location x,y"
  echo "  --eula eula_file"
  echo "      attach a license file to the dmg"
  echo "  --version         show tool version number"
  echo "  -h, --help        display this help"
  exit 0
}

WINX=10
WINY=60
WINW=500
WINH=350
ICON_SIZE=128
TEXT_SIZE=16

while test "${1:0:1}" = "-"; do
  case $1 in
    --volname)
      VOLUME_NAME="$2"
      shift; shift;;
    --volicon)
      VOLUME_ICON_FILE="$2"
      shift; shift;;
    --background)
      BACKGROUND_FILE="$2"
      BACKGROUND_FILE_NAME="$(basename $BACKGROUND_FILE)"
      BACKGROUND_CLAUSE="set background picture of opts to file \".background:$BACKGROUND_FILE_NAME\""
      REPOSITION_HIDDEN_FILES_CLAUSE="set position of every item to {theBottomRightX + 100, 100}"
      shift; shift;;
    --icon-size)
      ICON_SIZE="$2"
      shift; shift;;
    --text-size)
      TEXT_SIZE="$2"
      shift; shift;;
    --window-pos)
      WINX=$2; WINY=$3
      shift; shift; shift;;
    --window-size)
      WINW=$2; WINH=$3
      shift; shift; shift;;
    --icon)
      POSITION_CLAUSE="${POSITION_CLAUSE}set position of item \"$2\" to {$3, $4}
"
      shift; shift; shift; shift;;
    --hide-extension)
      HIDING_CLAUSE="${HIDING_CLAUSE}set the extension hidden of item \"$2\" to true
"
      shift; shift;;
    --custom-icon)
      shift; shift; shift; shift; shift;;
    -h | --help)
      usage;;
    --version)
      version; exit 0;;
    --pure-version)
      pure_version; exit 0;;
    --app-drop-link)
      APPLICATION_LINK=$2
      APPLICATION_CLAUSE="set position of item \"Applications\" to {$2, $3}
"
      shift; shift; shift;;
    --eula)
      EULA_RSRC=$2
      shift; shift;;
    -*)
      echo "Unknown option $1. Run with --help for help."
      exit 1;;
  esac
done

test -z "$2" && {
  echo "Not enough arguments. Invoke with --help for help."
  exit 1
}

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
DMG_PATH="$1"
DMG_DIRNAME="$(dirname "$DMG_PATH")"
DMG_DIR="$(cd "$DMG_DIRNAME" > /dev/null; pwd)"
DMG_NAME="$(basename "$DMG_PATH")"
DMG_TEMP_NAME="$DMG_DIR/rw.${DMG_NAME}"
SRC_FOLDER="$(cd "$2" > /dev/null; pwd)"
test -z "$VOLUME_NAME" && VOLUME_NAME="$(basename "$DMG_PATH" .dmg)"

AUX_PATH="$SCRIPT_DIR/support"

test -d "$AUX_PATH" || {
  echo "Cannot find support directory: $AUX_PATH"
  exit 1
}

if [ -f "$SRC_FOLDER/.DS_Store" ]; then
    echo "Deleting any .DS_Store in source folder"
    rm "$SRC_FOLDER/.DS_Store"
fi

# Create the image
echo "Creating disk image..."
test -f "${DMG_TEMP_NAME}" && rm -f "${DMG_TEMP_NAME}"
ACTUAL_SIZE=$(du -sm "$SRC_FOLDER" | sed -e 's/	.*//g')
DISK_IMAGE_SIZE=$(( $ACTUAL_SIZE + 20 ))
hdiutil create -srcfolder "$SRC_FOLDER" -volname "${VOLUME_NAME}" -fs HFS+ -fsargs "-c c=64,a=16,e=16" -format UDRW -size ${DISK_IMAGE_SIZE}m "${DMG_TEMP_NAME}"

# mount it
echo "Mounting disk image..."
MOUNT_DIR="/Volumes/${VOLUME_NAME}"

# try unmount dmg if it was mounted previously (e.g. developer mounted dmg, installed app and forgot to unmount it)
echo "Unmounting disk image..."
DEV_NAME=$(hdiutil info | grep -E '^/dev/' | sed 1q | awk '{print $1}')
test -d "${MOUNT_DIR}" && hdiutil detach "${DEV_NAME}"

echo "Mount directory: $MOUNT_DIR"
DEV_NAME=$(hdiutil attach -readwrite -noverify -noautoopen "${DMG_TEMP_NAME}" | grep -E '^/dev/' | sed 1q | awk '{print $1}')
echo "Device name:     $DEV_NAME"

if ! test -z "$BACKGROUND_FILE"; then
  echo "Copying background file..."
  test -d "$MOUNT_DIR/.background" || mkdir "$MOUNT_DIR/.background"
  cp "$BACKGROUND_FILE" "$MOUNT_DIR/.background/$BACKGROUND_FILE_NAME"
fi

if ! test -z "$APPLICATION_LINK"; then
  echo "making link to Applications dir"
  echo $MOUNT_DIR
  ln -s /Applications "$MOUNT_DIR/Applications"
fi

if ! test -z "$VOLUME_ICON_FILE"; then
  echo "Copying volume icon file '$VOLUME_ICON_FILE'..."
  cp "$VOLUME_ICON_FILE" "$MOUNT_DIR/.VolumeIcon.icns"
  SetFile -c icnC "$MOUNT_DIR/.VolumeIcon.icns"
fi

# run applescript
APPLESCRIPT=$(mktemp -t createdmg)
cat "$AUX_PATH/template.applescript" | sed -e "s/WINX/$WINX/g" -e "s/WINY/$WINY/g" -e "s/WINW/$WINW/g" -e "s/WINH/$WINH/g" -e "s/BACKGROUND_CLAUSE/$BACKGROUND_CLAUSE/g" -e "s/REPOSITION_HIDDEN_FILES_CLAUSE/$REPOSITION_HIDDEN_FILES_CLAUSE/g" -e "s/ICON_SIZE/$ICON_SIZE/g" -e "s/TEXT_SIZE/$TEXT_SIZE/g" | perl -pe  "s/POSITION_CLAUSE/$POSITION_CLAUSE/g" | perl -pe "s/APPLICATION_CLAUSE/$APPLICATION_CLAUSE/g" | perl -pe "s/HIDING_CLAUSE/$HIDING_CLAUSE/" >"$APPLESCRIPT"

echo "Running Applescript: /usr/bin/osascript \"${APPLESCRIPT}\" \"${VOLUME_NAME}\""
"/usr/bin/osascript" "${APPLESCRIPT}" "${VOLUME_NAME}" || true
echo "Done running the applescript..."
sleep 4

rm "$APPLESCRIPT"

# make sure it's not world writeable
echo "Fixing permissions..."
chmod -Rf go-w "${MOUNT_DIR}" &> /dev/null || true
echo "Done fixing permissions."

# make the top window open itself on mount:
echo "Blessing started"
bless --folder "${MOUNT_DIR}"
echo "Blessing finished"

if ! test -z "$VOLUME_ICON_FILE"; then
   # tell the volume that it has a special file attribute
   SetFile -a C "$MOUNT_DIR"
fi

# unmount
echo "Unmounting disk image..."
hdiutil detach "${DEV_NAME}"

# compress image
echo "Compressing disk image..."
hdiutil convert "${DMG_TEMP_NAME}" -format UDZO -imagekey zlib-level=9 -o "${DMG_DIR}/${DMG_NAME}"
rm -f "${DMG_TEMP_NAME}"

# adding EULA resources
if [ ! -z "${EULA_RSRC}" ] && [ "${EULA_RSRC}" != "-null-" ]; then
        echo "adding EULA resources"
        "${AUX_PATH}/dmg-license.py" "${DMG_DIR}/${DMG_NAME}" "${EULA_RSRC}"
fi

echo "Disk image done"
exit 0
