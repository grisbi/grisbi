SUBDIRS = categories

desktopdir = $(datadir)/applications
desktop_in_files = grisbi.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = $(desktop_in_files) grisbi.xml

mimedir = $(datadir)/mime/packages
mime_DATA = grisbi.xml

DISTCLEANFILES = $(desktop_DATA)
