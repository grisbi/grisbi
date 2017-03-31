# Process this file with automake to produce Makefile.in

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src $(GRISBI_CFLAGS) $(LIBPREFSPAGES_CFLAGS)

noinst_LTLIBRARIES = libprefspages.la

libprefspages_la_SOURCES = \
	prefs_page_archives.c	\
	prefs_page_files.c	\
	prefs_page_archives.h	\
	prefs_page_files.h

libprefspages_la_LIBADD= $(LIBPREFSPAGES_LIBS)

CLEANFILES = \
	*~

MAINTAINERCLEANFILES = \
	Makefile.in
