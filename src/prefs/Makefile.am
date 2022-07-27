# Process this file with automake to produce Makefile.in

AM_CPPFLAGS = -I$(top_srcdir) \
			  -I$(top_srcdir)/src \
			  -I$(top_srcdir)/src/widgets \
			  $(GLIB_CFLAGS) $(GTK_CFLAGS) $(LIBPREFSPAGES_CFLAGS)

noinst_LTLIBRARIES = libprefspages.la

libprefspages_la_SOURCES = \
	prefs_page_accueil.c	\
	prefs_page_archives.c	\
	prefs_page_bank.c	\
	prefs_page_bet_account.c	\
	prefs_page_bet_general.c	\
	prefs_page_currency.c	\
	prefs_page_currency_link.c	\
	prefs_page_display_adr.c	\
	prefs_page_display_fonts.c	\
	prefs_page_display_form.c	\
	prefs_page_display_gui.c \
	prefs_page_display_ope.c \
	prefs_page_divers.c	\
	prefs_page_files.c	\
	prefs_page_form_completion.c \
	prefs_page_form_options.c \
	prefs_page_fyear.c \
	prefs_page_import.c	\
	prefs_page_metatree.c \
	prefs_page_msg_delete.c \
	prefs_page_msg_warning.c \
	prefs_page_options_ope.c \
	prefs_page_payment_method.c \
	prefs_page_reconcile.c \
	prefs_page_reconcile_sort.c \
	prefs_page_accueil.h	\
	prefs_page_archives.h	\
	prefs_page_bank.h	\
	prefs_page_bet_account.h	\
	prefs_page_bet_general.h	\
	prefs_page_currency.h	\
	prefs_page_currency_link.h	\
	prefs_page_display_adr.h	\
	prefs_page_display_fonts.h	\
	prefs_page_display_form.h	\
	prefs_page_display_gui.h \
	prefs_page_display_ope.h \
	prefs_page_divers.h \
	prefs_page_files.h \
	prefs_page_form_completion.h \
	prefs_page_form_options.h \
	prefs_page_fyear.h \
	prefs_page_import.h \
	prefs_page_metatree.h \
	prefs_page_msg_delete.h \
	prefs_page_msg_warning.h \
	prefs_page_options_ope.h \
	prefs_page_payment_method.h \
	prefs_page_reconcile.h \
	prefs_page_reconcile_sort.h

libprefspages_la_LIBADD= $(LIBPREFSPAGES_LIBS)

CLEANFILES = \
	*~

MAINTAINERCLEANFILES = \
	Makefile.in

EXTRA_DIST = \
	prefs_page_accueil.ui \
	prefs_page_archives.ui \
	prefs_page_bet_account.ui

