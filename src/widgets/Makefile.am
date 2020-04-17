# Process this file with automake to produce Makefile.in

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src $(GLIB_CFLAGS) $(GTK_CFLAGS) $(LIBWIDGETS_CFLAGS)

noinst_LTLIBRARIES = libwidgets.la

libwidgets_la_SOURCES = \
	widget_bank_details.c \
	widget_bank_details.h

libwidgets_la_LIBADD= $(LIBWIDGETS_LIBS)

CLEANFILES = \
	*~

MAINTAINERCLEANFILES = \
	Makefile.in
