# Process this file with automake to produce Makefile.in

AM_CPPFLAGS = -I$(top_srcdir) \
			  -I$(top_srcdir)/src \
			  -I$(top_srcdir)/src/etats \
			  -I$(top_srcdir)/src/prefs \
			  $(GLIB_CFLAGS) $(GTK_CFLAGS) $(LIBWIDGETS_CFLAGS)

noinst_LTLIBRARIES = libwidgets.la

libwidgets_la_SOURCES = \
	widget_bank_details.c \
	widget_bank_details.h \
	widget_cmp_amount.c \
	widget_cmp_amount.h \
	widget_cmp_text.c \
	widget_cmp_text.h \
	widget_css_rules.c \
	widget_css_rules.h \
	widget_currency_details.c \
	widget_currency_details.h \
	widget_currency_link.c \
	widget_currency_link.h \
	widget_currency_popup.c \
	widget_currency_popup.h \
	widget_import_asso.c \
	widget_import_asso.h \
	widget_import_files.c \
	widget_import_files.h \
	widget_list_accounts.c \
	widget_list_accounts.h \
	widget_list_metatree.c \
	widget_list_metatree.h \
	widget_loan.c \
	widget_loan.h \
	widget_transfer.c \
	widget_transfer.h

libwidgets_la_LIBADD= $(LIBWIDGETS_LIBS)

CLEANFILES = \
	*~

MAINTAINERCLEANFILES = \
	Makefile.in
	
EXTRA_DIST = \
	widget_bank_details.ui \
	widget_cmp_amount.ui \
	widget_cmp_text.ui \
	widget_css_rules.ui \
	widget_currency_details.ui \
	widget_currency_link.ui \
	widget_currency_popup.ui \
	widget_import_files.ui \
	widget_import_asso.ui \
	widget_list_accounts.ui \
	widget_list_metatree.ui \
	widget_loan.ui \
	widget_transfer.ui




