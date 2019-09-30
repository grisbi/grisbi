#! /bin/bash

# Script to add files not automatically detected

set -e

DEST=MacOS/dist/Grisbi.app/Contents/Resources

EXTRA_FILES="
lib/goffice/
share/libofx/dtd/
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
share/icons/Adwaita/scalable/devices/drive-harddisk-symbolic.svg
share/icons/Adwaita/scalable/mimetypes/inode-directory-symbolic.svg
share/icons/Adwaita/scalable/mimetypes/text-x-generic-symbolic.svg
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
share/icons/HighContrast/scalable/status/image-missing.svg
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
