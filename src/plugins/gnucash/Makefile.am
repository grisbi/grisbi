# Gnucash plugin

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/src

plugindir = $(libdir)/grisbi

if HAVE_XML2

plugin_LTLIBRARIES = gnucash.la
gnucash_la_SOURCES = gnucash.c gnucash.h
gnucash_la_LDFLAGS = -module -avoid-version $(XML2_LIBS)
gnucash_la_CFLAGS = $(XML2_CFLAGS)


# Libs must be cleared, or else libtool won't create a shared module.
# If your module needs to be linked against any particular libraries,
# add them here.
LIBS = 

endif

CLEANFILES = \
	*~

MAINTAINERCLEANFILES = \
	Makefile.in
