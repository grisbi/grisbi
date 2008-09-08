# Process this file with automake to produce Makefile.in

localedir = $(datadir)/locale
INCLUDES = -I$(top_srcdir) -I$(includedir) -I../intl -DLOCALEDIR=\"$(localedir)\" -DPIXMAPS_DIR="\"$(datadir)/pixmaps/@PACKAGE@\"" -DPLUGINS_DIR="\"$(prefix)/lib/@PACKAGE@\"" -DLOGO_PATH="\"$(datadir)/pixmaps/@PACKAGE@/grisbi-logo.png\"" -DDATA_PATH="\"$(datadir)/@PACKAGE@\"" -DHELP_PATH="\"$(datadir)/doc/@PACKAGE@\""  -Wall @g2banking_includes@
bin_PROGRAMS = grisbi

# TODO: This manual setting of -Wl is a hack, I cannot get it from
# configure right now because of no doc.
grisbi_LDFLAGS = @lt_prog_compiler_wl@@export_dynamic_flag_spec@

config_dir = config

sources = \
	accueil.c		\
	affichage.c		\
	affichage_liste.c	\
	barre_outils.c		\
	categories_onglet.c	\
	classement_echeances.c	\
	comptes_onglet.c	\
	csv_parse.c		\
	custom_list.c		\
	dialog.c		\
	echeancier_infos.c	\
	erreur.c		\
	etats_affiche.c		\
	etats_calculs.c		\
	etats_config.c		\
	etats_csv.c		\
	etats_gtktable.c	\
	etats_html.c		\
	etats_latex.c		\
	etats_onglet.c		\
	etats_support.c		\
	export.c		\
	export_csv.c		\
	fenetre_principale.c	\
	go-charmap-sel.c	\
	go-optionmenu.c		\
	gsb_account.c		\
	gsb_account_property.c	\
	gsb_archive_config.c	\
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
	gsb_calendar_entry.c	\
	gsb_currency.c		\
	gsb_currency_config.c	\
	gsb_currency_link_config.c	\
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
	gsb_data_payee.c	\
	gsb_data_payment.c		\
	gsb_data_reconcile.c	\
	gsb_data_report.c	\
	gsb_data_report_amout_comparison.c	\
	gsb_data_report_text_comparison.c	\
	gsb_data_scheduled.c	\
	gsb_data_transaction.c	\
	gsb_file.c		\
	gsb_file_config.c	\
	gsb_file_debug.c	\
	gsb_file_load.c		\
	gsb_file_others.c	\
	gsb_file_save.c		\
	gsb_file_util.c		\
	gsb_form.c		\
	gsb_form_config.c	\
	gsb_form_scheduler.c	\
	gsb_form_transaction.c	\
	gsb_form_widget.c	\
	gsb_fyear.c		\
	gsb_fyear_config.c	\
	gsb_payment_method.c	\
	gsb_payment_method_config.c	\
	gsb_plugins.c		\
	gsb_real.c		\
	gsb_reconcile.c		\
	gsb_reconcile_config.c	\
	gsb_reconcile_list.c	\
	gsb_reconcile_sort_config.c	\
	gsb_report.c		\
	gsb_scheduler.c		\
	gsb_scheduler_list.c	\
	gsb_status.c		\
	gsb_transactions_list.c	\
	gsb_transactions_list_sort.c	\
	gtk_combofix.c		\
	help.c			\
	import.c		\
	import_csv.c		\
	imputation_budgetaire.c	\
	main.c			\
	menu.c			\
	meta_budgetary.c	\
	meta_categories.c	\
	meta_payee.c		\
	metatree.c		\
	navigation.c		\
	parametres.c		\
	parse_cmdline.c		\
	print_config.c		\
	print_dialog_config.c		\
	print_transactions_list.c		\
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
	utils_file_selection.c	\
	utils_files.c		\
	utils_font.c		\
	utils_operations.c	\
	utils_str.c		\
	balance_estimate_tab.c

grisbi_SOURCES = $(sources) $(sources_config)

grisbi_INCLUDES =		\
	$(include_config)       \
	accueil.h		\
	affichage.h		\
	affichage_liste.h	\
	barre_outils.h		\
        balance_estimate_tab.h  \
	categories_onglet.h	\
	classement_echeances.h	\
	comptes_onglet.h	\
        csv_parse.h             \
	custom_list.h		\
        etats_affiche.h         \
        etats_csv.h             \
        etats_html.h            \
        export.h                \
        export_csv.h            \
	import_csv.h		\
        go-charmap-sel.h        \
        go-optionmenu.h         \
	gsb_account.h		\
	gsb_account_property.h	\
	gsb_archive_config.h	\
	gsb_assistant.h		\
	gsb_assistant_account.h		\
	gsb_assistant_archive.h		\
	gsb_assistant_archive_export.h		\
	gsb_assistant_file.h		\
	gsb_assistant_first.h		\
	gsb_assistant_reconcile_config.h		\
	gsb_autofunc.h		\
	gsb_automem.h		\
	gsb_bank.h		\
	gsb_category.h		\
	gsb_combo_box.h		\
	gsb_calendar_entry.h	\
        gsb_crypt.h             \
	gsb_currency.h		\
	gsb_currency_config.h	\
	gsb_currency_link_config.h	\
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
	gsb_data_payee.h	\
	gsb_data_payment.h		\
	gsb_data_reconcile.h	\
	gsb_data_report.h	\
	gsb_data_report_amout_comparison.h	\
	gsb_data_report_text_comparison.h	\
	gsb_data_scheduled.h	\
	gsb_data_transaction.h	\
	gsb_file.h		\
	gsb_file_config.h	\
        gsb_file_debug.h        \
	gsb_file_load.h		\
	gsb_file_others.h	\
	gsb_file_save.h		\
	gsb_file_util.h		\
	gsb_form.h		\
	gsb_form_config.h	\
	gsb_form_scheduler.h	\
	gsb_form_transaction.h	\
	gsb_form_widget.h	\
	gsb_fyear.h		\
	gsb_fyear_config.h	\
	gsb_payment_method.h	\
	gsb_payment_method_config.h	\
        gsb_plugins.h           \
	gsb_real.h		\
	gsb_reconcile.h		\
	gsb_reconcile_config.h	\
	gsb_reconcile_list.h	\
	gsb_reconcile_sort_config.h	\
	gsb_report.h		\
        gsb_status.h            \
	gsb_scheduler.h		\
	gsb_scheduler_list.h	\
	gsb_transactions_list.h	\
	gsb_transactions_list_sort.h	\
	dialog.h		\
	echeancier_infos.h	\
	erreur.h		\
	etats_calculs.h		\
	etats_config.h		\
	etats_gtktable.h	\
	etats.h			\
	etats_latex.h		\
	etats_onglet.h		\
	etats_support.h		\
	fenetre_principale.h	\
	gtk_combofix.h		\
	help.h			\
	imputation_budgetaire.h	\
	include.h		\
	main.h			\
	menu.h			\
        meta_budgetary.h        \
        meta_categories.h       \
        meta_payee.h            \
        metatree.h              \
        mouse.h                 \
	navigation.h		\
	parametres.h		\
	parse_cmdline.h		\
	print_config.h		\
	print_dialog_config.h		\
	print_transactions_list.h		\
	qif.h			\
	import.h		\
	tip.h			\
	structures.h		\
	tiers_onglet.h		\
	traitement_variables.h	\
	transaction_list.h	\
	transaction_list_select.h\
	transaction_list_sort.h	\
	transaction_model.h	\
	utils.h			\
	utils_buttons.h		\
	utils_dates.h		\
	utils_editables.h	\
	utils_file_selection.h	\
	utils_files.h		\
	utils_font.h		\
	utils_operations.h	\
	utils_str.h


EXTRA_DIST = grisbi.desktop grisbi.keys grisbi.mime $(grisbi_INCLUDES)

mimedir = $(datadir)/mime-info
mime_DATA = grisbi.keys grisbi.mime

if HAVE_PLUGINS
SUBDIRS = plugins/g2banking plugins/gnucash plugins/ofx plugins/openssl
endif
