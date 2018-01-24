## Process this file with automake to produce Makefile.in

SUBDIRS = src po pixmaps help share
man_MANS = grisbi.1

EXTRA_DIST = \
    grisbi.spec \
    win32 \
    win32-msys2\
    $(man_MANS)

ACLOCAL_AMFLAGS = -I m4

bundle:
	@if [ "x$(UNDER_JHBUILD)" != "xtrue" ]; then echo "*** Please run this command under JHbuild env."; exit 1; fi
	@if [ "x$(PREFIX)" != "x$(prefix)" ]; then echo "*** Please use ./configure --prefix=$(PREFIX) to configure Grisbi."; exit 1; fi
	@if [ -f $(srcdir)/MacOS/Grisbi.dmg ]; then echo "*** DMG file already exist."; exit 1; fi
	env GRIBSI_BUNDLE_PATH=$(srcdir) gtk-mac-bundler $(srcdir)/MacOS/Grisbi.bundle
	$(srcdir)/MacOS/create-dmg.sh --volname Grisbi --volicon $(srcdir)/MacOS/Grisbi.icns --window-size 640 400 --background $(srcdir)/MacOS/background.png --icon-size 96 --app-drop-link 500 250 --icon "Grisbi.app" 150 250  $(srcdir)/MacOS/Grisbi.dmg $(srcdir)/MacOS/dist

