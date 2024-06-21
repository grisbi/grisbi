#!/bin/sh
#
# Try to detect and copy all dependencies on MSYS2/Windows boxes to package
# an installer. Especially, it forces inclusion of libcroco & librsvg that
# are not automatically tracked by ldd (ntldd).
#
# Formerly, all copies were done through Makefile.am itself but I (Frédéric)
# chose to propose this independant hook file to clarify goals and method.

# This file is executed by Makefile (install-exec-hook rule) with
# $prefix as its only, mandatory argument.


prefix=""

msys2_arch=""
bits=""

export bits
export msys2_arch

# *****************************************************************************
# Functions
# *****************************************************************************

detect_arch() {
	# Detects architecture
	msys2_arch="w64-x86_64"
	bits="64"
	if test "$MSYSTEM"x == "MINGW32"x; then
		msys2_arch="w64-i686"
		bits="32"
	fi
	echo "Detected: $msys2_arch ($bits-bit)"
}

install_libs() {	
	auto_dependencies=$(ntldd -R $prefix/bin/grisbi.exe | grep "mingw64\|mingw32" | cut -f2 -d'>' | cut -f2 -d' ')
	for i in $auto_dependencies; do
		cp $i $prefix/bin || exit 1
		echo "  * $i"
	done
}

install_extra_libs() {
	echo "Copying extra (non-detected) libraries"
	libcroco=$(pacman -Ql mingw-$msys2_arch-libcroco | grep ".dll$" | cut -f2 -d' ')
	librsvg=$(pacman -Ql mingw-w64-ucrt-x86_64-librsvg | grep "rsvg\-.*\.dll$" | cut -f2 -d' ')

	for i in $libcroco $librsvg /tmp/inst/bin/*.dll; do
		cp $i $prefix/bin || exit 1
		echo "  * $i"
	done
}

install_data() {
	echo "Copying glib-2.0 schemas"; \
	if test ! -d $prefix/share/glib-2.0/schemas/; then mkdir -p $prefix/share/glib-2.0/schemas/; fi;
	cp /ucrt$bits/share/glib-2.0/schemas/gschema.dtd \
	/ucrt$bits/share/glib-2.0/schemas/org.gtk.Settings.FileChooser.gschema.xml \
	/ucrt$bits/share/glib-2.0/schemas/org.gtk.Settings.ColorChooser.gschema.xml \
	/ucrt$bits/share/glib-2.0/schemas/org.gtk.Settings.EmojiChooser.gschema.xml \
	$prefix/share/glib-2.0/schemas/; \
	glib-compile-schemas $prefix/share/glib-2.0/schemas/; \
	gdk_pixbuf_ver=$(pkg-config.exe gdk-pixbuf-2.0 --variable=gdk_pixbuf_binary_version); \
	echo "Copying gdk-pixbuf cache file ($gdk_pixbuf_ver)"; \
	mkdir -p $prefix/lib/gdk-pixbuf-2.0/$gdk_pixbuf_ver; \
	cp $(pkg-config.exe gdk-pixbuf-2.0 --variable=gdk_pixbuf_cache_file) $prefix/lib/gdk-pixbuf-2.0/$gdk_pixbuf_ver; \
	if test ! -d $prefix/share/icons/; then mkdir -p $prefix/share/icons; fi; \
	echo "Copying hicolor icons"; \
	cp -rf /ucrt$bits/share/icons/hicolor $prefix/share/icons/; \
	echo "Copying Adwaita icons"; \
	cp -rf /ucrt$bits/share/icons/Adwaita $prefix/share/icons/; \
	echo "Copying LC_MESSAGES for GTK"; \
    if test ! -d $prefix/share/locale/cs/LC_MESSAGES/; then mkdir -p $prefix/share/locale/cs/LC_MESSAGES/; fi; \
	cp /ucrt$bits/share/locale/cs/LC_MESSAGES/gtk30.mo \
	/ucrt$bits/share/locale/cs/LC_MESSAGES/gtk30-properties.mo \
	/ucrt$bits/share/locale/cs/LC_MESSAGES/glib20.mo \
	$prefix/share/locale/cs/LC_MESSAGES/; \
	if test ! -d $prefix/share/locale/da/LC_MESSAGES/; then mkdir -p $prefix/share/locale/da/LC_MESSAGES/; fi; \
	cp /ucrt$bits/share/locale/da/LC_MESSAGES/gtk30.mo \
	/ucrt$bits/share/locale/da/LC_MESSAGES/gtk30-properties.mo \
	/ucrt$bits/share/locale/da/LC_MESSAGES/glib20.mo \
	$prefix/share/locale/da/LC_MESSAGES/; \
	if test ! -d $prefix/share/locale/de/LC_MESSAGES/; then mkdir -p $prefix/share/locale/de/LC_MESSAGES/; fi; \
	cp /ucrt$bits/share/locale/de/LC_MESSAGES/gtk30.mo \
	/ucrt$bits/share/locale/de/LC_MESSAGES/gtk30-properties.mo \
	/ucrt$bits/share/locale/de/LC_MESSAGES/glib20.mo \
	$prefix/share/locale/de/LC_MESSAGES/; \
	if test ! -d $prefix/share/locale/el/LC_MESSAGES/; then mkdir -p $prefix/share/locale/el/LC_MESSAGES/; fi; \
	cp /ucrt$bits/share/locale/el/LC_MESSAGES/gtk30.mo \
	/ucrt$bits/share/locale/el/LC_MESSAGES/gtk30-properties.mo \
	/ucrt$bits/share/locale/el/LC_MESSAGES/glib20.mo \
	$prefix/share/locale/el/LC_MESSAGES/; \
	if test ! -d $prefix/share/locale/eo/LC_MESSAGES/; then mkdir -p $prefix/share/locale/eo/LC_MESSAGES/; fi; \
	cp /ucrt$bits/share/locale/eo/LC_MESSAGES/gtk30.mo \
	/ucrt$bits/share/locale/eo/LC_MESSAGES/gtk30-properties.mo \
	/ucrt$bits/share/locale/eo/LC_MESSAGES/glib20.mo \
	$prefix/share/locale/eo/LC_MESSAGES/; \
	if test ! -d $prefix/share/locale/es/LC_MESSAGES/; then mkdir -p $prefix/share/locale/es/LC_MESSAGES/; fi; \
	cp /ucrt$bits/share/locale/es/LC_MESSAGES/gtk30.mo \
	/ucrt$bits/share/locale/es/LC_MESSAGES/gtk30-properties.mo \
	/ucrt$bits/share/locale/es/LC_MESSAGES/glib20.mo \
	$prefix/share/locale/es/LC_MESSAGES/; \
	if test ! -d $prefix/share/locale/fa/LC_MESSAGES/; then mkdir -p $prefix/share/locale/fa/LC_MESSAGES/; fi; \
	cp /ucrt$bits/share/locale/fa/LC_MESSAGES/gtk30.mo \
	/ucrt$bits/share/locale/fa/LC_MESSAGES/gtk30-properties.mo \
	/ucrt$bits/share/locale/fa/LC_MESSAGES/glib20.mo \
	$prefix/share/locale/fa/LC_MESSAGES/; \
	if test ! -d $prefix/share/locale/fr/LC_MESSAGES/; then mkdir -p $prefix/share/locale/fr/LC_MESSAGES/; fi; \
	cp /ucrt$bits/share/locale/fr/LC_MESSAGES/gtk30.mo \
	/ucrt$bits/share/locale/fr/LC_MESSAGES/gtk30-properties.mo \
	/ucrt$bits/share/locale/fr/LC_MESSAGES/glib20.mo \
	$prefix/share/locale/fr/LC_MESSAGES/; \
	if test ! -d $prefix/share/locale/lv/LC_MESSAGES/; then mkdir -p $prefix/share/locale/lv/LC_MESSAGES/; fi; \
	cp /ucrt$bits/share/locale/lv/LC_MESSAGES/gtk30.mo \
	/ucrt$bits/share/locale/lv/LC_MESSAGES/gtk30-properties.mo \
	/ucrt$bits/share/locale/lv/LC_MESSAGES/glib20.mo \
	$prefix/share/locale/lv/LC_MESSAGES/; \
	if test ! -d $prefix/share/locale/he/LC_MESSAGES/; then mkdir -p $prefix/share/locale/he/LC_MESSAGES/; fi; \
	cp /ucrt$bits/share/locale/he/LC_MESSAGES/gtk30.mo \
	/ucrt$bits/share/locale/he/LC_MESSAGES/gtk30-properties.mo \
	/ucrt$bits/share/locale/he/LC_MESSAGES/glib20.mo \
	$prefix/share/locale/he/LC_MESSAGES/; \
	if test ! -d $prefix/share/locale/it/LC_MESSAGES/; then mkdir -p $prefix/share/locale/it/LC_MESSAGES/; fi; \
	cp /ucrt$bits/share/locale/it/LC_MESSAGES/gtk30.mo \
	/ucrt$bits/share/locale/it/LC_MESSAGES/gtk30-properties.mo \
	/ucrt$bits/share/locale/it/LC_MESSAGES/glib20.mo \
	$prefix/share/locale/it/LC_MESSAGES/; \
	if test ! -d $prefix/share/locale/lv/LC_MESSAGES/; then mkdir -p $prefix/share/locale/lv/LC_MESSAGES/; fi; \
	cp /ucrt$bits/share/locale/lv/LC_MESSAGES/gtk30.mo \
	/ucrt$bits/share/locale/lv/LC_MESSAGES/gtk30-properties.mo \
	/ucrt$bits/share/locale/lv/LC_MESSAGES/glib20.mo \
	$prefix/share/locale/lv/LC_MESSAGES/; \
	if test ! -d $prefix/share/locale/nl/LC_MESSAGES/; then mkdir -p $prefix/share/locale/nl/LC_MESSAGES/; fi; \
	cp /ucrt$bits/share/locale/nl/LC_MESSAGES/gtk30.mo \
	/ucrt$bits/share/locale/nl/LC_MESSAGES/gtk30-properties.mo \
	/ucrt$bits/share/locale/nl/LC_MESSAGES/glib20.mo \
	$prefix/share/locale/nl/LC_MESSAGES/; \
	if test ! -d $prefix/share/locale/pl/LC_MESSAGES/; then mkdir -p $prefix/share/locale/pl/LC_MESSAGES/; fi; \
	cp /ucrt$bits/share/locale/pl/LC_MESSAGES/gtk30.mo \
	/ucrt$bits/share/locale/pl/LC_MESSAGES/gtk30-properties.mo \
	/ucrt$bits/share/locale/pl/LC_MESSAGES/glib20.mo \
	$prefix/share/locale/pl/LC_MESSAGES/; \
	if test ! -d $prefix/share/locale/pt_BR/LC_MESSAGES/; then mkdir -p $prefix/share/locale/pt_BR/LC_MESSAGES/; fi; \
	cp /ucrt$bits/share/locale/pt_BR/LC_MESSAGES/gtk30.mo \
	/ucrt$bits/share/locale/pt_BR/LC_MESSAGES/gtk30-properties.mo \
	/ucrt$bits/share/locale/pt_BR/LC_MESSAGES/glib20.mo \
	$prefix/share/locale/pt_BR/LC_MESSAGES/; \
	if test ! -d $prefix/share/locale/ro/LC_MESSAGES/; then mkdir -p $prefix/share/locale/ro/LC_MESSAGES/; fi; \
	cp /ucrt$bits/share/locale/ro/LC_MESSAGES/gtk30.mo \
	/ucrt$bits/share/locale/ro/LC_MESSAGES/gtk30-properties.mo \
	/ucrt$bits/share/locale/ro/LC_MESSAGES/glib20.mo \
	$prefix/share/locale/ro/LC_MESSAGES/; \
	if test ! -d $prefix/share/locale/ru/LC_MESSAGES/; then mkdir -p $prefix/share/locale/ru/LC_MESSAGES/; fi; \
	cp /ucrt$bits/share/locale/ru/LC_MESSAGES/gtk30.mo \
	/ucrt$bits/share/locale/ru/LC_MESSAGES/gtk30-properties.mo \
	/ucrt$bits/share/locale/ru/LC_MESSAGES/glib20.mo \
	$prefix/share/locale/ru/LC_MESSAGES/; \
	if test ! -d $prefix/share/locale/sv/LC_MESSAGES/; then mkdir -p $prefix/share/locale/sv/LC_MESSAGES/; fi; \
	cp /ucrt$bits/share/locale/sv/LC_MESSAGES/gtk30.mo \
	/ucrt$bits/share/locale/sv/LC_MESSAGES/gtk30-properties.mo \
	/ucrt$bits/share/locale/sv/LC_MESSAGES/glib20.mo \
	$prefix/share/locale/sv/LC_MESSAGES/; \
	if test ! -d $prefix/share/locale/zh_CN/LC_MESSAGES/; then mkdir -p $prefix/share/locale/zh_CN/LC_MESSAGES/; fi; \
	cp /ucrt$bits/share/locale/zh_CN/LC_MESSAGES/gtk30.mo \
	/ucrt$bits/share/locale/zh_CN/LC_MESSAGES/gtk30-properties.mo \
	/ucrt$bits/share/locale/zh_CN/LC_MESSAGES/glib20.mo \
	$prefix/share/locale/zh_CN/LC_MESSAGES/; \
	echo "Copying gdk-pixbuf loaders ($gdk_pixbuf_ver)"; \
	cp -rf $(pkg-config.exe gdk-pixbuf-2.0 --variable=gdk_pixbuf_moduledir) $prefix/lib/gdk-pixbuf-2.0/$gdk_pixbuf_ver; \
	echo "Copying data files for libofx";  \
	mkdir -p $prefix/share/libofx/dtd; \
	cp -rf /tmp/inst/share/libofx/dtd/* $prefix/share/libofx/dtd; \
	echo "Copying data files & plugins for libgoffice";  \
	cp -rf /tmp/inst/lib/goffice $prefix/lib
	
	# patch Grisbi CSS for the dark mode
	DARK='@import url("resource:///org/gtk/libgtk/theme/Adwaita/gtk-contained-dark.css");'
	echo $DARK >> $prefix/share/grisbi/ui/grisbi-dark.css
}

# *****************************************************************************
# Main
# *****************************************************************************
if test "$1"x = ""x; then
	echo "Usage $0: <destination>"
	exit 1
fi

prefix=$1
detect_arch
install_libs
install_extra_libs

echo "Strip $prefix/bin/grisbi.exe"
strip $prefix/bin/grisbi.exe || exit 1

install_data
