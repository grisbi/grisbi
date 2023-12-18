SUBDIRS = categories

desktopdir = $(datadir)/applications
desktop_in_files = grisbi.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
grisbi.desktop: grisbi.desktop.in
	$(AM_V_GEN)$(MSGFMT) --desktop --template $< -d $(top_srcdir)/po -o $@

appstreamdir = $(datadir)/metainfo/
appstream_in_files = grisbi.metainfo.xml.in
appstream_DATA = $(appstream_in_files:.xml.in=.xml)
grisbi.metainfo.xml: grisbi.metainfo.xml.in
	$(AM_V_GEN)$(MSGFMT) --xml --template $< -d $(top_srcdir)/po -o $@

EXTRA_DIST = $(desktop_in_files) grisbi.xml grisbi.nsi.in $(appstream_in_files)

mimedir = $(datadir)/mime/packages
mime_DATA = grisbi.xml

DISTCLEANFILES = $(desktop_DATA) $(appstream_DATA)

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

install-exec-hook:
if WIN32
	@$(top_srcdir)/share/msys2-extra-install-hook.sh $(prefix); \
	echo "Use makensis[w].exe from NSIS (http://nsis.sourceforget.net/) on $(bindir)/share/grisbi.nsi"
endif
