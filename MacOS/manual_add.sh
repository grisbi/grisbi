#! /bin/bash

# Script to add files not automatically detected

set -e

DEST=MacOS/dist/Grisbi.app/Contents/Resources

EXTRA_FILES="
share/libofx/dtd/
share/icons/Adwaita/16x16/actions/bookmark-new-symbolic.symbolic.png
share/icons/Adwaita/16x16/actions/folder-new-symbolic.symbolic.png
share/icons/Adwaita/16x16/actions/go-down-symbolic.symbolic.png
share/icons/Adwaita/16x16/actions/go-up-symbolic.symbolic.png
share/icons/Adwaita/16x16/actions/list-add-symbolic.symbolic.png
share/icons/Adwaita/16x16/actions/list-remove-all-symbolic.symbolic.png
share/icons/Adwaita/16x16/actions/list-remove-symbolic.symbolic.png
share/icons/Adwaita/16x16/actions/media-eject-symbolic.symbolic.png
share/icons/Adwaita/16x16/devices/drive-harddisk-symbolic.symbolic.png
share/icons/Adwaita/16x16/mimetypes/image-x-generic.png
share/icons/Adwaita/16x16/mimetypes/inode-directory-symbolic.symbolic.png
share/icons/Adwaita/16x16/mimetypes/inode-directory.png
share/icons/Adwaita/16x16/mimetypes/text-x-generic.png
share/icons/Adwaita/16x16/mimetypes/video-x-generic.png
share/icons/Adwaita/16x16/places/folder-documents-symbolic.symbolic.png
share/icons/Adwaita/16x16/places/folder-documents.png
share/icons/Adwaita/16x16/places/folder-download-symbolic.symbolic.png
share/icons/Adwaita/16x16/places/folder-download.png
share/icons/Adwaita/16x16/places/folder-music-symbolic.symbolic.png
share/icons/Adwaita/16x16/places/folder-music.png
share/icons/Adwaita/16x16/places/folder-pictures-symbolic.symbolic.png
share/icons/Adwaita/16x16/places/folder-pictures.png
share/icons/Adwaita/16x16/places/folder-videos-symbolic.symbolic.png
share/icons/Adwaita/16x16/places/folder-videos.png
share/icons/Adwaita/16x16/places/user-desktop-symbolic.symbolic.png
share/icons/Adwaita/16x16/places/user-desktop.png
share/icons/Adwaita/16x16/places/user-home-symbolic.symbolic.png
share/icons/Adwaita/16x16/places/user-home.png
share/icons/Adwaita/16x16/places/user-trash-symbolic.symbolic.png
share/icons/Adwaita/16x16/places/user-trash.png
share/icons/Adwaita/16x16/ui/pan-down-symbolic.symbolic.png
share/icons/Adwaita/16x16/ui/pan-end-symbolic.symbolic.png
share/icons/Adwaita/16x16/ui/pan-start-symbolic.symbolic.png
share/icons/Adwaita/16x16/ui/window-close-symbolic.symbolic.png
share/icons/Adwaita/32x32/actions/go-down-symbolic.symbolic.png
share/icons/Adwaita/32x32/actions/go-up-symbolic.symbolic.png
share/icons/Adwaita/32x32/actions/list-add-symbolic.symbolic.png
share/icons/Adwaita/32x32/actions/list-remove-all-symbolic.symbolic.png
share/icons/Adwaita/32x32/actions/list-remove-symbolic.symbolic.png
share/icons/Adwaita/32x32/ui/window-close-symbolic.symbolic.png
share/icons/Adwaita/index.theme
share/icons/Adwaita/scalable/mimetypes/inode-directory.svg
share/icons/Adwaita/scalable/mimetypes/text-x-generic.svg
share/icons/Adwaita/scalable/places/folder-documents.svg
share/icons/Adwaita/scalable/places/folder-download.svg
share/icons/Adwaita/scalable/places/folder-music.svg
share/icons/Adwaita/scalable/places/folder-pictures.svg
share/icons/Adwaita/scalable/places/folder-publicshare.svg
share/icons/Adwaita/scalable/places/folder-remote.svg
share/icons/Adwaita/scalable/places/folder.svg
share/icons/Adwaita/scalable/places/folder-templates.svg
share/icons/Adwaita/scalable/places/folder-videos.svg
share/icons/Adwaita/scalable/places/network-server.svg
share/icons/Adwaita/scalable/places/network-workgroup.svg
share/icons/Adwaita/scalable/places/user-bookmarks.svg
share/icons/Adwaita/scalable/places/user-desktop.svg
share/icons/Adwaita/scalable/places/user-home.svg
share/icons/Adwaita/scalable/places/user-trash.svg
"

if [ -z "$PREFIX" ]
then
	echo "Run inside jhbuild shell"
	exit 1
fi

for file in $EXTRA_FILES
do
	echo "$file"
	mkdir -p "$DEST"/$(dirname "$file")
	cp -a "$PREFIX/$file" "$DEST/$file"
done

# the icons are NOT found in "ui/" directory. So make them available in
# "places/" instead.
( cd $DEST/share/icons/Adwaita/scalable/places ; ln -sf ../ui/* . )

#
( cd $DEST/share/pixmaps/grisbi ; ln -sf gtk-execute-24.png gtk-execute.png )

# patch Grisbi CSS for the dark mode
DARK='@import url("resource:///org/gtk/libgtk/theme/Adwaita/gtk-contained-dark.css");'
echo "$DARK" >> "$DEST"/share/grisbi/ui/grisbi-dark.css
