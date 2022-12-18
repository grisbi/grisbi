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
share/icons/Adwaita/16x16/devices/drive-harddisk.png
share/icons/Adwaita/16x16/legacy/edit-undo.png
share/icons/Adwaita/16x16/legacy/go-down.png
share/icons/Adwaita/16x16/legacy/go-up.png
share/icons/Adwaita/16x16/legacy/list-add.png
share/icons/Adwaita/16x16/legacy/list-remove.png
share/icons/Adwaita/16x16/legacy/window-close.png
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
share/icons/Adwaita/16x16/status/image-missing.png
share/icons/Adwaita/16x16/ui/pan-down-symbolic.symbolic.png
share/icons/Adwaita/16x16/ui/pan-end-symbolic.symbolic.png
share/icons/Adwaita/16x16/ui/pan-start-symbolic.symbolic.png
share/icons/Adwaita/16x16/ui/window-close-symbolic.symbolic.png
share/icons/Adwaita/22x22/legacy/go-down.png
share/icons/Adwaita/22x22/legacy/go-up.png
share/icons/Adwaita/22x22/legacy/list-add.png
share/icons/Adwaita/22x22/legacy/list-remove.png
share/icons/Adwaita/22x22/legacy/window-close.png
share/icons/Adwaita/24x24/actions/go-down-symbolic.symbolic.png
share/icons/Adwaita/24x24/actions/go-up-symbolic.symbolic.png
share/icons/Adwaita/24x24/actions/list-add-symbolic.symbolic.png
share/icons/Adwaita/24x24/actions/list-remove-all-symbolic.symbolic.png
share/icons/Adwaita/24x24/actions/list-remove-symbolic.symbolic.png
share/icons/Adwaita/24x24/legacy/go-down.png
share/icons/Adwaita/24x24/legacy/go-up.png
share/icons/Adwaita/24x24/legacy/list-add.png
share/icons/Adwaita/24x24/legacy/list-remove.png
share/icons/Adwaita/24x24/legacy/window-close.png
share/icons/Adwaita/24x24/status/image-missing.png
share/icons/Adwaita/24x24/ui/window-close-symbolic.symbolic.png
share/icons/Adwaita/32x32/actions/go-down-symbolic.symbolic.png
share/icons/Adwaita/32x32/actions/go-up-symbolic.symbolic.png
share/icons/Adwaita/32x32/actions/list-add-symbolic.symbolic.png
share/icons/Adwaita/32x32/actions/list-remove-all-symbolic.symbolic.png
share/icons/Adwaita/32x32/actions/list-remove-symbolic.symbolic.png
share/icons/Adwaita/32x32/legacy/go-down.png
share/icons/Adwaita/32x32/legacy/go-up.png
share/icons/Adwaita/32x32/legacy/list-add.png
share/icons/Adwaita/32x32/legacy/list-remove.png
share/icons/Adwaita/32x32/legacy/window-close.png
share/icons/Adwaita/32x32/ui/window-close-symbolic.symbolic.png
share/icons/Adwaita/48x48/actions/go-down-symbolic.symbolic.png
share/icons/Adwaita/48x48/actions/go-up-symbolic.symbolic.png
share/icons/Adwaita/48x48/actions/list-add-symbolic.symbolic.png
share/icons/Adwaita/48x48/actions/list-remove-all-symbolic.symbolic.png
share/icons/Adwaita/48x48/actions/list-remove-symbolic.symbolic.png
share/icons/Adwaita/48x48/legacy/go-down.png
share/icons/Adwaita/48x48/legacy/go-up.png
share/icons/Adwaita/48x48/legacy/list-add.png
share/icons/Adwaita/48x48/legacy/list-remove.png
share/icons/Adwaita/48x48/legacy/window-close.png
share/icons/Adwaita/48x48/ui/window-close-symbolic.symbolic.png
share/icons/Adwaita/64x64/actions/go-down-symbolic.symbolic.png
share/icons/Adwaita/64x64/actions/go-up-symbolic.symbolic.png
share/icons/Adwaita/64x64/actions/list-add-symbolic.symbolic.png
share/icons/Adwaita/64x64/actions/list-remove-all-symbolic.symbolic.png
share/icons/Adwaita/64x64/actions/list-remove-symbolic.symbolic.png
share/icons/Adwaita/64x64/ui/window-close-symbolic.symbolic.png
share/icons/Adwaita/96x96/actions/go-down-symbolic.symbolic.png
share/icons/Adwaita/96x96/actions/go-up-symbolic.symbolic.png
share/icons/Adwaita/96x96/actions/list-add-symbolic.symbolic.png
share/icons/Adwaita/96x96/actions/list-remove-all-symbolic.symbolic.png
share/icons/Adwaita/96x96/actions/list-remove-symbolic.symbolic.png
share/icons/Adwaita/96x96/ui/window-close-symbolic.symbolic.png
share/icons/Adwaita/index.theme
share/icons/Adwaita/scalable/actions/bookmark-new-symbolic.svg
share/icons/Adwaita/scalable/actions/document-open-recent-symbolic.svg
share/icons/Adwaita/scalable/actions/document-open-symbolic.svg
share/icons/Adwaita/scalable/actions/document-save-symbolic.svg
share/icons/Adwaita/scalable/actions/edit-find-symbolic.svg
share/icons/Adwaita/scalable/actions/folder-new-symbolic.svg
share/icons/Adwaita/scalable/actions/go-bottom-symbolic.svg
share/icons/Adwaita/scalable/actions/go-down-symbolic.svg
share/icons/Adwaita/scalable/actions/go-top-symbolic.svg
share/icons/Adwaita/scalable/actions/go-up-symbolic.svg
share/icons/Adwaita/scalable/actions/list-add-symbolic.svg
share/icons/Adwaita/scalable/actions/list-remove-all-symbolic.svg
share/icons/Adwaita/scalable/actions/list-remove-symbolic.svg
share/icons/Adwaita/scalable/actions/media-eject-symbolic.svg
share/icons/Adwaita/scalable/actions/object-select-symbolic.svg
share/icons/Adwaita/scalable/devices/drive-harddisk-symbolic.svg
share/icons/Adwaita/scalable/mimetypes/inode-directory-symbolic.svg
share/icons/Adwaita/scalable/mimetypes/text-x-generic-symbolic.svg
share/icons/Adwaita/scalable/places/folder-documents-symbolic.svg
share/icons/Adwaita/scalable/places/folder-download-symbolic.svg
share/icons/Adwaita/scalable/places/folder-music-symbolic.svg
share/icons/Adwaita/scalable/places/folder-pictures-symbolic.svg
share/icons/Adwaita/scalable/places/folder-publicshare-symbolic.svg
share/icons/Adwaita/scalable/places/folder-remote-symbolic.svg
share/icons/Adwaita/scalable/places/folder-saved-search-symbolic.svg
share/icons/Adwaita/scalable/places/folder-symbolic.svg
share/icons/Adwaita/scalable/places/folder-templates-symbolic.svg
share/icons/Adwaita/scalable/places/folder-videos-symbolic.svg
share/icons/Adwaita/scalable/places/network-server-symbolic.svg
share/icons/Adwaita/scalable/places/network-workgroup-symbolic.svg
share/icons/Adwaita/scalable/places/start-here-symbolic.svg
share/icons/Adwaita/scalable/places/user-bookmarks-symbolic.svg
share/icons/Adwaita/scalable/places/user-desktop-symbolic.svg
share/icons/Adwaita/scalable/places/user-home-symbolic.svg
share/icons/Adwaita/scalable/places/user-trash-symbolic.svg
share/icons/Adwaita/scalable/ui/pan-down-symbolic.svg
share/icons/Adwaita/scalable/ui/pan-end-symbolic-rtl.svg
share/icons/Adwaita/scalable/ui/pan-end-symbolic.svg
share/icons/Adwaita/scalable/ui/pan-start-symbolic-rtl.svg
share/icons/Adwaita/scalable/ui/pan-start-symbolic.svg
share/icons/Adwaita/scalable/ui/pan-up-symbolic.svg
share/icons/Adwaita/scalable/ui/window-close-symbolic.svg
"

if [ -z "$PREFIX" ]
then
	echo "Run inside jhbuild shell"
	exit 1
fi

for file in $EXTRA_FILES
do
	echo $file
	mkdir -p $DEST/$(dirname $file)
	cp -a $PREFIX/$file $DEST/$file
done

# the icons are NOT found in "ui/" directory. So make them available in
# "places/" instead.
( cd $DEST/share/icons/Adwaita/scalable/places ; ln -sf ../ui/* . )

#
( cd $DEST/share/pixmaps/grisbi ; ln -sf gtk-execute-24.png gtk-execute.png )

# patch Grisbi CSS for the dark mode
DARK='@import url("resource:///org/gtk/libgtk/theme/Adwaita/gtk-contained-dark.css");'
echo $DARK >> $DEST/share/grisbi/ui/grisbi-dark.css
