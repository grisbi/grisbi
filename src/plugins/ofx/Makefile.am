# OFX plugin

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/src

plugindir = $(libdir)/@PACKAGE@

if HAVE_OFX

plugin_LTLIBRARIES = ofx.la
ofx_la_SOURCES = ofx.c ofx.h
ofx_la_LDFLAGS = -module -avoid-version -lofx

if HAVE_OFX_SET_STATUS_CB 
ofx_la_CFLAGS = -DOFX_0_7
endif

# Libs must be cleared, or else libtool won't create a shared module.
# If your module needs to be linked against any particular libraries,
# add them here.

ofx_la_LIBS = 

endif

CLEANFILES = \
	*~

MAINTAINERCLEANFILES = \
	Makefile.in
