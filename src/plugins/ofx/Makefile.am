# OFX plugin

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/src

plugindir = $(libdir)/grisbi

if HAVE_OFX

plugin_LTLIBRARIES = ofx.la
ofx_la_SOURCES = ofx.c ofx.h
ofx_la_LDFLAGS = -module -avoid-version

# Libs must be cleared, or else libtool won't create a shared module.
# If your module needs to be linked against any particular libraries,
# add them here.
LIBS = -lofx

endif

CLEANFILES = \
	*~

MAINTAINERCLEANFILES = \
	Makefile.in
