# Process this file with automake to produce Makefile.in

AM_CPPFLAGS = -I$(top_srcdir) \
	      -I$(top_srcdir)/src/etats \
	      -I$(top_srcdir)/src/prefs \
	      -I$(top_srcdir)/src/widgets \
	      $(GLIB_CFLAGS) \
	      $(GTK_CFLAGS) \
	      $(LIBGSF_CFLAGS) \
	      $(GTK_MAC_CFLAGS) \
	      -DLOCALEDIR="\"$(localedir)\""	\
	      -DPIXMAPS_DIR="\"$(datadir)/pixmaps/@PACKAGE@\""	\
	      -DDATA_PATH="\"$(datadir)/@PACKAGE@\""	\
	      -DHELP_PATH="\"$(docdir)\""	\
	      -DUI_DIR="\"$(datadir)/@PACKAGE@/ui\""

bin_PROGRAMS = grisbi
noinst_LTLIBRARIES = libgrisbi.la

grisbi_LDADD = \
	libgrisbi.la \
	$(GRISBI_LIBS) \
	$(GTK_MAC_LIBS) \
	$(ZLIB_LIBS) \
	$(GTK_LIBS) \
	$(GLIB_LIBS)

libgrisbi_la_LIBADD =

libgrisbi_la_LIBADD += etats/libetatspages.la
libgrisbi_la_LIBADD += prefs/libprefspages.la
libgrisbi_la_LIBADD += widgets/libwidgets.la

if HAVE_XML2
libgrisbi_la_LIBADD += plugins/gnucash/gnucash.la
endif
if HAVE_OFX
libgrisbi_la_LIBADD += plugins/ofx/ofx.la
endif
if HAVE_SSL
libgrisbi_la_LIBADD += plugins/openssl/openssl.la
endif
if HAVE_GOFFICE
libgrisbi_la_LIBADD += $(GOFFICE_LIBS)
AM_CPPFLAGS += $(GOFFICE_CFLAGS)
endif
libgrisbi_la_LIBADD += $(GRISBI_LIBS) $(GTK_MAC_LIBS)

libgrisbi_la_SOURCES = \
	accueil.c		\
	bet_data.c \
	bet_data_finance.c \
	bet_finance_ui.c \
	bet_future.c \
	bet_hist.c \
	bet_tab.c	\
	categories_onglet.c	\
	classement_echeances.c	\
	csv_parse.c		\
	csv_template_rule.c	\
	custom_list.c		\
	dialog.c		\
	erreur.c		\
	etats_affiche.c		\
	etats_calculs.c		\
	etats_config.c		\
	etats_csv.c		\
	etats_gtktable.c	\
	etats_html.c		\
	etats_onglet.c		\
	etats_prefs.c	\
	etats_support.c		\
	export.c		\
	export_csv.c		\
	file_obfuscate.c	\
	file_obfuscate_qif.c	\
	grisbi_app.c		\
	grisbi_conf.c \
	grisbi_prefs.c \
	grisbi_win.c		\
	gsb_account.c		\
	gsb_assistant.c		\
	gsb_assistant_account.c		\
	gsb_assistant_archive.c		\
	gsb_assistant_archive_export.c		\
	gsb_assistant_file.c		\
	gsb_assistant_first.c		\
	gsb_assistant_reconcile_config.c		\
	gsb_autofunc.c		\
	gsb_automem.c		\
	gsb_bank.c		\
	gsb_category.c		\
	gsb_combo_box.c		\
	gsb_calendar.c		\
	gsb_calendar_entry.c	\
	gsb_currency.c		\
	gsb_currency_popup.c \
	gsb_data_account.c	\
	gsb_data_archive.c	\
	gsb_data_archive_store.c	\
	gsb_data_bank.c		\
	gsb_data_budget.c	\
	gsb_data_category.c	\
	gsb_data_currency.c	\
	gsb_data_currency_link.c	\
	gsb_data_form.c		\
	gsb_data_fyear.c	\
	gsb_data_import_rule.c	\
	gsb_data_mix.c		\
	gsb_data_partial_balance.c   \
	gsb_data_payee.c	\
	gsb_data_payment.c		\
	gsb_data_print_config.c		\
	gsb_data_reconcile.c	\
	gsb_data_report.c	\
	gsb_data_report_amout_comparison.c	\
	gsb_data_report_text_comparison.c	\
	gsb_data_scheduled.c	\
	gsb_data_transaction.c	\
	gsb_debug.c		\
	gsb_dirs.c		\
	gsb_file.c		\
	gsb_file_load.c		\
	gsb_file_others.c	\
	gsb_file_save.c		\
	gsb_file_util.c		\
	gsb_form.c		\
	gsb_form_scheduler.c	\
	gsb_form_transaction.c	\
	gsb_form_widget.c	\
	gsb_fyear.c		\
	gsb_locale.c	\
	gsb_payment_method.c	\
	gsb_real.c		\
	gsb_reconcile.c		\
	gsb_reconcile_list.c	\
	gsb_regex.c		\
	gsb_report.c		\
	gsb_rgba.c  \
	gsb_scheduler.c		\
	gsb_scheduler_list.c	\
	gsb_select_icon.c \
	gsb_transactions_list.c	\
	gsb_transactions_list_sort.c	\
	gtk_combofix.c		\
	help.c			\
	import.c		\
	import_csv.c		\
	imputation_budgetaire.c	\
	menu.c			\
	meta_budgetary.c	\
	meta_categories.c	\
	meta_payee.c		\
	metatree.c		\
	navigation.c		\
	print_dialog_config.c		\
	print_report.c		\
	print_transactions_list.c		\
	print_tree_view_list.c	\
	qif.c			\
	tiers_onglet.c		\
	tip.c			\
	traitement_variables.c	\
	transaction_list.c	\
	transaction_list_select.c\
	transaction_list_sort.c	\
	transaction_model.c	\
	utils.c			\
	utils_buttons.c		\
	utils_dates.c		\
	utils_editables.c	\
	utils_files.c		\
	utils_operations.c	\
	utils_prefs.c	\
	utils_real.c		\
	utils_str.c		\
	utils_widgets.c \
	accueil.h		\
	bet_data.h \
	bet_data_finance.h \
	bet_finance_ui.h \
	bet_future.h \
	bet_hist.h \
	bet_tab.h  \
	categories_onglet.h	\
	classement_echeances.h	\
	csv_parse.h             \
	csv_template_rule.h	\
	custom_list.h		\
	dialog.h		\
	erreur.h		\
	etats_affiche.h         \
	etats_calculs.h		\
	etats_config.h		\
	etats_csv.h             \
	etats_gtktable.h	\
	etats_html.h            \
	etats_onglet.h		\
	etats_prefs.h	\
	etats_support.h		\
	export.h                \
	export_csv.h            \
	file_obfuscate.h	\
	file_obfuscate_qif.h	\
	grisbi_app.h		\
	grisbi_conf.h \
	grisbi_prefs.h \
	grisbi_win.h		\
	gsb_account.h		\
	gsb_assistant.h		\
	gsb_assistant_account.h		\
	gsb_assistant_archive.h		\
	gsb_assistant_archive_export.h \
	gsb_assistant_file.h		\
	gsb_assistant_first.h		\
	gsb_assistant_reconcile_config.h \
	gsb_autofunc.h		\
	gsb_automem.h		\
	gsb_bank.h		\
	gsb_category.h		\
	gsb_combo_box.h		\
	gsb_calendar.h		\
	gsb_calendar_entry.h	\
	gsb_currency.h		\
	gsb_currency_popup.h \
	gsb_data_account.h	\
	gsb_data_archive.h	\
	gsb_data_archive_store.h	\
	gsb_data_bank.h		\
	gsb_data_budget.h	\
	gsb_data_category.h	\
	gsb_data_currency.h	\
	gsb_data_currency_link.h	\
	gsb_data_form.h		\
	gsb_data_fyear.h	\
	gsb_data_import_rule.h	\
	gsb_data_mix.h		\
	gsb_data_partial_balance.h   \
	gsb_data_payee.h	\
	gsb_data_payment.h		\
	gsb_data_print_config.h		\
	gsb_data_reconcile.h	\
	gsb_data_report.h	\
	gsb_data_report_amout_comparison.h	\
	gsb_data_report_text_comparison.h	\
	gsb_data_scheduled.h	\
	gsb_data_transaction.h	\
	gsb_debug.h		\
	gsb_dirs.h		\
	gsb_file.h		\
	gsb_file_load.h		\
	gsb_file_others.h	\
	gsb_file_save.h		\
	gsb_file_util.h		\
	gsb_form.h		\
	gsb_form_scheduler.h	\
	gsb_form_transaction.h	\
	gsb_form_widget.h	\
	gsb_fyear.h		\
	gsb_locale.h	\
	gsb_payment_method.h	\
	gsb_real.h		\
	gsb_reconcile.h		\
	gsb_reconcile_list.h	\
	gsb_regex.h		\
	gsb_report.h		\
	gsb_rgba.h  \
	gsb_select_icon.h 	\
	gsb_scheduler.h		\
	gsb_scheduler_list.h	\
	gsb_transactions_list.h	\
	gsb_transactions_list_sort.h	\
	gtk_combofix.h		\
	help.h			\
	import.h		\
	import_csv.h		\
	imputation_budgetaire.h	\
	include.h		\
	menu.h			\
	meta_budgetary.h        \
	meta_categories.h       \
	meta_payee.h            \
	metatree.h              \
	mouse.h                 \
	navigation.h		\
	print_dialog_config.h	\
	print_report.h		\
	print_transactions_list.h	\
	print_tree_view_list.h	\
	qif.h			\
	tip.h			\
	structures.h		\
	tiers_onglet.h		\
	traitement_variables.h	\
	transaction_list.h	\
	transaction_list_select.h	\
	transaction_list_sort.h	\
	transaction_model.h	\
	utils.h			\
	utils_buttons.h		\
	utils_dates.h		\
	utils_editables.h	\
	utils_files.h		\
	utils_operations.h	\
	utils_prefs.h	\
	utils_real.h		\
	utils_str.h		\
	utils_widgets.h \
	grisbi.gresource.xml

if !HAVE_GOFFICE
libgrisbi_la_SOURCES += \
	go-charmap-sel.c \
	go-optionmenu.c \
	go-charmap-sel.h \
	go-optionmenu.h
else
libgrisbi_la_SOURCES += \
	bet_graph.c \
	bet_graph.h
endif

if PLATFORM_OSX
libgrisbi_la_SOURCES += \
	grisbi_osx.m \
	grisbi_osx.h

grisbi_LDFLAGS = -framework Foundation
endif

grisbi_SOURCES = 		\
	main.c				\
	grisbi_resources.c



BUILT_SOURCES = grisbi_resources.c

.rc.res:
		windres $< -O coff -o $@

if WIN32
grisbi_LDADD += win-main.res
endif

grisbi_resources.c: grisbi.gresource.xml $(shell $(GLIB_COMPILE_RESOURCES) --generate-dependencies --sourcedir=$(srcdir) $(srcdir)/grisbi.gresource.xml)
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) --target=$@ --generate-source --c-name grisbi --sourcedir=$(srcdir) $<

if WIN32
grisbi_LDFLAGS = -mwindows
endif

SUBDIRS = etats prefs ui widgets plugins/gnucash plugins/ofx plugins/openssl images

if HAVE_CUNIT
SUBDIRS += tests
endif

EXTRA_DIST = grisbi.keys grisbi.mime \
	grisbi.gresource.xml

CLEANFILES = $(BUILT_SOURCES)


mimedir = $(datadir)/mime-info
mime_DATA = grisbi.keys grisbi.mime

ALLHEADERS := $(filter %.h,$(libgrisbi_la_SOURCES) $(grisbi_SOURCES))

checkheaders: $(ALLHEADERS:.h=.ho)

%.ho: %.h
	$(CC) -include config.h $(AM_CPPFLAGS) $(CPPFLAGS) $(CFLAGS) -c -o $@ -x c $<

%.o: %.m
	$(OBJC) -c -o $@ $<
