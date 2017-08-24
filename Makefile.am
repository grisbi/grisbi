## Process this file with automake to produce Makefile.in

SUBDIRS = src po pixmaps help share
man_MANS = grisbi.1

# Key variable for a very own packaging; basically, I use:
# ./configure --prefix=~/my-inst && make && make install
#
# As usual, configure will install in $(prefix), but variable below will also be used
# by install-exec-hook (automatically launched at the end of make install) to add
# binary libraries needed by Grisbi x86_64:
#
# (see install-exec-hook target)
auto_dependencies=`ldd $(prefix)/bin/grisbi.exe | grep mingw64 | cut -f2 -d'>' | cut -f2 -d' '`

EXTRA_DIST = \
    grisbi.spec \
    win32 \
    win32-msvc\
    win32-msys2\
    $(man_MANS)

ACLOCAL_AMFLAGS = -I m4

# !!! Still used?
bundle:
	@if [ "x$(UNDER_JHBUILD)" != "xtrue" ]; then echo "*** Please run this command under JHbuild env."; exit 1; fi
	@if [ "x$(PREFIX)" != "x$(prefix)" ]; then echo "*** Please use ./configure --prefix=$(PREFIX) to configure Grisbi."; exit 1; fi
	@if [ -f $(srcdir)/MacOS/Grisbi.dmg ]; then echo "*** DMG file already exist."; exit 1; fi
	env GRIBSI_BUNDLE_PATH=$(srcdir) gtk-mac-bundler $(srcdir)/MacOS/Grisbi.bundle
	$(srcdir)/MacOS/create-dmg.sh --volname Grisbi --volicon $(srcdir)/MacOS/Grisbi.icns --window-size 640 400 --background $(srcdir)/MacOS/background.png --icon-size 96 --app-drop-link 500 250 --icon "Grisbi.app" 150 250  $(srcdir)/MacOS/Grisbi.dmg $(srcdir)/MacOS/dist


# Here we add all necessary stuff in $(prefix), including DLL libraries,
# icons, schemas or various data. Point is to get a full dependencies
# for Grisbi in a single directory tree.
#
# My view on final packaging implies this tree to be used by NSIS to
# produce end-user's package for 64bit Windows.
#
# Notice that many data files are copied that might be useless for
# Grisbi at this time...

	@if _WIN32; \
	then \
install-exec-hook: \
		echo "Adding dependencies: "; \
		for i in $(auto_dependencies); do \
			$(install_sh_PROGRAM) $$i $(prefix)/bin; \
			echo "  * $$i"; \
		done; \
		echo "Strip $(prefix)/bin/grisbi.exe"; \
		strip $(prefix)/bin/grisbi.exe; \
		if test ! -d $(prefix)/bin/share; then mkdir -p $(prefix)/bin/share; fi; \
		echo "Copying glib-2.0 schemas"; \
		cp -r /usr/share/glib-2.0/schemas $(prefix)/bin/share; \
		gdk_pixbuf_ver=`pkg-config.exe gdk-pixbuf-2.0 --variable=gdk_pixbuf_binary_version`; \
		echo "Copying gdk-pixbuf cache file ($$gdk_pixbuf_ver)"; \
		mkdir -p $(prefix)/lib/gdk-pixbuf-2.0/$$gdk_pixbuf_ver; \
		cp `pkg-config.exe gdk-pixbuf-2.0 --variable=gdk_pixbuf_cache_file` $(prefix)/lib/gdk-pixbuf-2.0/$$gdk_pixbuf_ver; \
		if test ! -d $(prefix)/share/icons/; then mkdir -p $(prefix)/share/icons; fi; \
		echo "Copying hicolor icons"; \
		cp -rf /mingw64/share/icons/hicolor $(prefix)/share/icons/; \
		echo "Copying Adwaita icons"; \
		cp -rf /mingw64/share/icons/Adwaita $(prefix)/share/icons/
	fi
