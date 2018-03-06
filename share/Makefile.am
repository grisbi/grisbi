SUBDIRS = categories

desktopdir = $(datadir)/applications
desktop_in_files = grisbi.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = $(desktop_in_files) grisbi.xml grisbi.nsi.in

mimedir = $(datadir)/mime/packages
mime_DATA = grisbi.xml

DISTCLEANFILES = $(desktop_DATA)

# Key variable for a very own packaging; basically, I use:
# ./configure --prefix=~/my-inst && make && make install
#
# As usual, configure will install in $(prefix), but variable below will also be used
# by install-exec-hook (automatically launched at the end of make install) to add
# binary libraries needed by Grisbi x86_64:
#
# (see install-exec-hook target)

# Here we add all necessary stuff in $(prefix), including DLL libraries,
# icons, schemas or various data. Point is to get a full dependencies
# for Grisbi in a single directory tree.
#
# My view on final packaging implies this tree to be used by NSIS to
# produce end-user's package for 64bit Windows.
#
# Notice that many data files are copied that might be useless for
# Grisbi at this time...

auto_dependencies=`ntldd -R $(prefix)/bin/grisbi.exe | grep "mingw64\|mingw32" | cut -f2 -d'>' | cut -f2 -d' '`

install-exec-hook:
if WIN32
	@echo "Windows run"

	echo "Adding dependencies: ";
		for i in $(auto_dependencies); do \
			$(install_sh_PROGRAM) $$i $(prefix)/bin; \
			echo "  * $$i"; \
		done;
	echo "install other dependencies" ;
	$(install_sh_PROGRAM) C:/msys64/mingw64/bin/libcroco-0.6-3.dll $(prefix)/bin; \
	$(install_sh_PROGRAM) C:/msys64/mingw64/bin/librsvg-2-2.dll $(prefix)/bin;
	echo "Strip $(prefix)/bin/grisbi.exe";
	strip $(prefix)/bin/grisbi.exe;
	echo "Copying glib-2.0 schemas";
	if test ! -d $(prefix)/share/glib-2.0/schemas/; then mkdir -p $(prefix)/share/glib-2.0/schemas/; fi;
	bits=64; \
	export bits; \
	if test "$$MSYSTEM"x == "MINGW32"x; then bits=32; fi; \
	cp -v /mingw$$bits/share/glib-2.0/schemas/gschema.dtd \
	/mingw$$bits/share/glib-2.0/schemas/org.gtk.Settings.FileChooser.gschema.xml \
	/mingw$$bits/share/glib-2.0/schemas/org.gtk.Settings.ColorChooser.gschema.xml \
	$(prefix)/share/glib-2.0/schemas/; \
	glib-compile-schemas $(prefix)/share/glib-2.0/schemas/; \
	gdk_pixbuf_ver=`pkg-config.exe gdk-pixbuf-2.0 --variable=gdk_pixbuf_binary_version`; \
	echo "Copying gdk-pixbuf cache file ($$gdk_pixbuf_ver)"; \
	mkdir -p $(prefix)/lib/gdk-pixbuf-2.0/$$gdk_pixbuf_ver; \
	cp `pkg-config.exe gdk-pixbuf-2.0 --variable=gdk_pixbuf_cache_file` $(prefix)/lib/gdk-pixbuf-2.0/$$gdk_pixbuf_ver; \
	if test ! -d $(prefix)/share/icons/; then mkdir -p $(prefix)/share/icons; fi; \
	echo "Copying hicolor icons"; \
	cp -rf /mingw$$bits/share/icons/hicolor $(prefix)/share/icons/; \
	echo "Copying Adwaita icons"; \
	cp -rf /mingw$$bits/share/icons/Adwaita $(prefix)/share/icons/; \
	echo "Copying LC_MESSAGES for GTK"; \
    if test ! -d $(prefix)/share/locale/cs/LC_MESSAGES/; then mkdir -p $(prefix)/share/locale/cs/LC_MESSAGES/; fi; \
	cp -v /mingw$$bits/share/locale/cs/LC_MESSAGES/gtk30.mo \
	/mingw$$bits/share/locale/cs/LC_MESSAGES/gtk30-properties.mo \
	/mingw$$bits/share/locale/cs/LC_MESSAGES/glib20.mo \
	$(prefix)/share/locale/cs/LC_MESSAGES/; \
	if test ! -d $(prefix)/share/locale/da/LC_MESSAGES/; then mkdir -p $(prefix)/share/locale/da/LC_MESSAGES/; fi; \
	cp -v /mingw$$bits/share/locale/da/LC_MESSAGES/gtk30.mo \
	/mingw$$bits/share/locale/da/LC_MESSAGES/gtk30-properties.mo \
	/mingw$$bits/share/locale/da/LC_MESSAGES/glib20.mo \
	$(prefix)/share/locale/da/LC_MESSAGES/; \
	if test ! -d $(prefix)/share/locale/de/LC_MESSAGES/; then mkdir -p $(prefix)/share/locale/de/LC_MESSAGES/; fi; \
	cp -v /mingw$$bits/share/locale/de/LC_MESSAGES/gtk30.mo \
	/mingw$$bits/share/locale/de/LC_MESSAGES/gtk30-properties.mo \
	/mingw$$bits/share/locale/de/LC_MESSAGES/glib20.mo \
	$(prefix)/share/locale/de/LC_MESSAGES/; \
	if test ! -d $(prefix)/share/locale/el/LC_MESSAGES/; then mkdir -p $(prefix)/share/locale/el/LC_MESSAGES/; fi; \
	cp -v /mingw$$bits/share/locale/el/LC_MESSAGES/gtk30.mo \
	/mingw$$bits/share/locale/el/LC_MESSAGES/gtk30-properties.mo \
	/mingw$$bits/share/locale/el/LC_MESSAGES/glib20.mo \
	$(prefix)/share/locale/el/LC_MESSAGES/; \
	if test ! -d $(prefix)/share/locale/eo/LC_MESSAGES/; then mkdir -p $(prefix)/share/locale/eo/LC_MESSAGES/; fi; \
	cp -v /mingw$$bits/share/locale/eo/LC_MESSAGES/gtk30.mo \
	/mingw$$bits/share/locale/eo/LC_MESSAGES/gtk30-properties.mo \
	/mingw$$bits/share/locale/eo/LC_MESSAGES/glib20.mo \
	$(prefix)/share/locale/eo/LC_MESSAGES/; \
	if test ! -d $(prefix)/share/locale/es/LC_MESSAGES/; then mkdir -p $(prefix)/share/locale/es/LC_MESSAGES/; fi; \
	cp -v /mingw$$bits/share/locale/es/LC_MESSAGES/gtk30.mo \
	/mingw$$bits/share/locale/es/LC_MESSAGES/gtk30-properties.mo \
	/mingw$$bits/share/locale/es/LC_MESSAGES/glib20.mo \
	$(prefix)/share/locale/es/LC_MESSAGES/; \
	if test ! -d $(prefix)/share/locale/fa/LC_MESSAGES/; then mkdir -p $(prefix)/share/locale/fa/LC_MESSAGES/; fi; \
	cp -v /mingw$$bits/share/locale/fa/LC_MESSAGES/gtk30.mo \
	/mingw$$bits/share/locale/fa/LC_MESSAGES/gtk30-properties.mo \
	/mingw$$bits/share/locale/fa/LC_MESSAGES/glib20.mo \
	$(prefix)/share/locale/fa/LC_MESSAGES/; \
	if test ! -d $(prefix)/share/locale/fr/LC_MESSAGES/; then mkdir -p $(prefix)/share/locale/fr/LC_MESSAGES/; fi; \
	cp -v /mingw$$bits/share/locale/fr/LC_MESSAGES/gtk30.mo \
	/mingw$$bits/share/locale/fr/LC_MESSAGES/gtk30-properties.mo \
	/mingw$$bits/share/locale/fr/LC_MESSAGES/glib20.mo \
	$(prefix)/share/locale/fr/LC_MESSAGES/; \
	if test ! -d $(prefix)/share/locale/lv/LC_MESSAGES/; then mkdir -p $(prefix)/share/locale/lv/LC_MESSAGES/; fi; \
	cp -v /mingw$$bits/share/locale/lv/LC_MESSAGES/gtk30.mo \
	/mingw$$bits/share/locale/lv/LC_MESSAGES/gtk30-properties.mo \
	/mingw$$bits/share/locale/lv/LC_MESSAGES/glib20.mo \
	$(prefix)/share/locale/lv/LC_MESSAGES/; \
	if test ! -d $(prefix)/share/locale/he/LC_MESSAGES/; then mkdir -p $(prefix)/share/locale/he/LC_MESSAGES/; fi; \
	cp -v /mingw$$bits/share/locale/he/LC_MESSAGES/gtk30.mo \
	/mingw$$bits/share/locale/he/LC_MESSAGES/gtk30-properties.mo \
	/mingw$$bits/share/locale/he/LC_MESSAGES/glib20.mo \
	$(prefix)/share/locale/he/LC_MESSAGES/; \
	if test ! -d $(prefix)/share/locale/it/LC_MESSAGES/; then mkdir -p $(prefix)/share/locale/it/LC_MESSAGES/; fi; \
	cp -v /mingw$$bits/share/locale/it/LC_MESSAGES/gtk30.mo \
	/mingw$$bits/share/locale/it/LC_MESSAGES/gtk30-properties.mo \
	/mingw$$bits/share/locale/it/LC_MESSAGES/glib20.mo \
	$(prefix)/share/locale/it/LC_MESSAGES/; \
	if test ! -d $(prefix)/share/locale/lv/LC_MESSAGES/; then mkdir -p $(prefix)/share/locale/lv/LC_MESSAGES/; fi; \
	cp -v /mingw$$bits/share/locale/lv/LC_MESSAGES/gtk30.mo \
	/mingw$$bits/share/locale/lv/LC_MESSAGES/gtk30-properties.mo \
	/mingw$$bits/share/locale/lv/LC_MESSAGES/glib20.mo \
	$(prefix)/share/locale/lv/LC_MESSAGES/; \
	if test ! -d $(prefix)/share/locale/nl/LC_MESSAGES/; then mkdir -p $(prefix)/share/locale/nl/LC_MESSAGES/; fi; \
	cp -v /mingw$$bits/share/locale/nl/LC_MESSAGES/gtk30.mo \
	/mingw$$bits/share/locale/nl/LC_MESSAGES/gtk30-properties.mo \
	/mingw$$bits/share/locale/nl/LC_MESSAGES/glib20.mo \
	$(prefix)/share/locale/nl/LC_MESSAGES/; \
	if test ! -d $(prefix)/share/locale/pl/LC_MESSAGES/; then mkdir -p $(prefix)/share/locale/pl/LC_MESSAGES/; fi; \
	cp -v /mingw$$bits/share/locale/pl/LC_MESSAGES/gtk30.mo \
	/mingw$$bits/share/locale/pl/LC_MESSAGES/gtk30-properties.mo \
	/mingw$$bits/share/locale/pl/LC_MESSAGES/glib20.mo \
	$(prefix)/share/locale/pl/LC_MESSAGES/; \
	if test ! -d $(prefix)/share/locale/pt_BR/LC_MESSAGES/; then mkdir -p $(prefix)/share/locale/pt_BR/LC_MESSAGES/; fi; \
	cp -v /mingw$$bits/share/locale/pt_BR/LC_MESSAGES/gtk30.mo \
	/mingw$$bits/share/locale/pt_BR/LC_MESSAGES/gtk30-properties.mo \
	/mingw$$bits/share/locale/pt_BR/LC_MESSAGES/glib20.mo \
	$(prefix)/share/locale/pt_BR/LC_MESSAGES/; \
	if test ! -d $(prefix)/share/locale/ro/LC_MESSAGES/; then mkdir -p $(prefix)/share/locale/ro/LC_MESSAGES/; fi; \
	cp -v /mingw$$bits/share/locale/ro/LC_MESSAGES/gtk30.mo \
	/mingw$$bits/share/locale/ro/LC_MESSAGES/gtk30-properties.mo \
	/mingw$$bits/share/locale/ro/LC_MESSAGES/glib20.mo \
	$(prefix)/share/locale/ro/LC_MESSAGES/; \
	if test ! -d $(prefix)/share/locale/ru/LC_MESSAGES/; then mkdir -p $(prefix)/share/locale/ru/LC_MESSAGES/; fi; \
	cp -v /mingw$$bits/share/locale/ru/LC_MESSAGES/gtk30.mo \
	/mingw$$bits/share/locale/ru/LC_MESSAGES/gtk30-properties.mo \
	/mingw$$bits/share/locale/ru/LC_MESSAGES/glib20.mo \
	$(prefix)/share/locale/ru/LC_MESSAGES/; \
	if test ! -d $(prefix)/share/locale/sv/LC_MESSAGES/; then mkdir -p $(prefix)/share/locale/sv/LC_MESSAGES/; fi; \
	cp -v /mingw$$bits/share/locale/sv/LC_MESSAGES/gtk30.mo \
	/mingw$$bits/share/locale/sv/LC_MESSAGES/gtk30-properties.mo \
	/mingw$$bits/share/locale/sv/LC_MESSAGES/glib20.mo \
	$(prefix)/share/locale/sv/LC_MESSAGES/; \
	if test ! -d $(prefix)/share/locale/zh_CN/LC_MESSAGES/; then mkdir -p $(prefix)/share/locale/zh_CN/LC_MESSAGES/; fi; \
	cp -v /mingw$$bits/share/locale/zh_CN/LC_MESSAGES/gtk30.mo \
	/mingw$$bits/share/locale/zh_CN/LC_MESSAGES/gtk30-properties.mo \
	/mingw$$bits/share/locale/zh_CN/LC_MESSAGES/glib20.mo \
	$(prefix)/share/locale/zh_CN/LC_MESSAGES/; \
	echo "Copying gdk-pixbuf loaders ($$gdk_pixbuf_ver)"; \
	cp -rf `pkg-config.exe gdk-pixbuf-2.0 --variable=gdk_pixbuf_moduledir` $(prefix)/lib/gdk-pixbuf-2.0/$$gdk_pixbuf_ver; \
	echo "Use makensis[w].exe from NSIS (http://nsis.sourceforget.net/) on $(bindir)/share/grisbi.nsi";
endif
