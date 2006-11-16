# Process this file with automake to produce Makefile.in

localedir = $(datadir)/locale
INCLUDES = -I$(top_srcdir) -I$(includedir) -I../intl -DLOCALEDIR=\"$(localedir)\" -DPIXMAPS_DIR="\"$(datadir)/pixmaps/grisbi\"" -DPLUGINS_DIR="\"$(libdir)/grisbi\"" -DLOGO_PATH="\"$(datadir)/pixmaps/grisbi/grisbi-logo.png\"" -DDATA_PATH="\"$(datadir)/grisbi\"" -DHELP_PATH="\"$(datadir)/doc/grisbi\""  -Wall @g2banking_includes@
bin_PROGRAMS = grisbi
grisbi_LDFLAGS = -Wl,-E

config_dir = config

sources_config = 			\
	configuration.c

include_config= \
    configuration.h  \
    xmlnames.h      

sources = \
	accueil.c		\
	affichage.c		\
	affichage_liste.c	\
	banque.c		\
	barre_outils.c		\
	calendar.c		\
	categories_onglet.c	\
	classement_echeances.c	\
	classement_operations.c	\
	comptes_gestion.c	\
	comptes_onglet.c	\
	comptes_traitements.c	\
	csv_parse.c		\
	data_payment.c		\
	dialog.c		\
	echeancier_infos.c	\
	equilibrage.c		\
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
	fichiers_gestion.c	\
	go-charmap-sel.c	\
	go-optionmenu.c		\
	gsb_assistant.c		\
	gsb_autofunc.c		\
	gsb_automem.c		\
	gsb_combo_box.c		\
	gsb_currency.c		\
	gsb_currency_config.c	\
	gsb_currency_link_config.c	\
	gsb_data_account.c	\
	gsb_data_bank.c		\
	gsb_data_budget.c	\
	gsb_data_category.c	\
	gsb_data_currency.c	\
	gsb_data_currency_link.c	\
	gsb_data_form.c		\
	gsb_data_fyear.c	\
	gsb_data_mix.c		\
	gsb_data_payee.c	\
	gsb_data_report.c	\
	gsb_data_report_amout_comparison.c	\
	gsb_data_report_text_comparison.c	\
	gsb_data_scheduled.c	\
	gsb_data_transaction.c	\
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
	gsb_scheduler_list.c	\
	gsb_status.c		\
	gsb_transaction_model.c	\
	gsb_transactions_list.c	\
	gtk_combofix.c		\
	help.c			\
	html.c			\
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
	operations_comptes.c	\
	parametres.c		\
	parse_cmdline.c		\
	print_config.c		\
	qif.c			\
	search_glist.c		\
	tiers_onglet.c		\
	tip.c			\
	traitement_variables.c	\
	utils.c			\
	utils_buttons.c		\
	utils_comptes.c		\
	utils_dates.c		\
	utils_editables.c	\
	utils_file_selection.c	\
	utils_files.c		\
	utils_operations.c	\
	utils_rapprochements.c	\
	utils_str.c		\
	utils_types.c

grisbi_SOURCES = $(sources) $(sources_config)

grisbi_INCLUDES =		\
	$(include_config)       \
	accueil.h		\
	affichage.h		\
	affichage_liste.h	\
	banque.h		\
	barre_outils.h		\
	calendar.h		\
	categories_onglet.h	\
	classement_operations.h	\
	classement_echeances.h	\
	comptes_gestion.h	\
	comptes_onglet.h	\
	comptes_traitements.h	\
	constants.h		\
	import_csv.h		\
	gsb_assistant.h		\
	gsb_autofunc.h		\
	gsb_automem.h		\
	gsb_combo_box.h		\
	gsb_currency.h		\
	gsb_currency_config.h	\
	gsb_currency_link_config.h	\
	gsb_data_account.h	\
	gsb_data_bank.h		\
	gsb_data_budget.h	\
	gsb_data_category.h	\
	gsb_data_currency.h	\
	gsb_data_currency_link.h	\
	gsb_data_form.h		\
	gsb_data_fyear.c	\
	gsb_data_mix.h		\
	gsb_data_payee.h	\
	gsb_data_report.h	\
	gsb_data_report_amout_comparison.h	\
	gsb_data_report_text_comparison.h	\
	gsb_data_scheduled.h	\
	gsb_data_transaction.h	\
	gsb_file_config.h	\
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
	gsb_real.h		\
	gsb_scheduler_list.h	\
	gsb_transaction_model.h	\
	gsb_transactions_list.h	\
	data_payment.h		\
	dialog.h		\
	echeancier_infos.h	\
	equilibrage.h		\
	erreur.h		\
	etats_calculs.h		\
	etats_config.h		\
	etats_gtktable.h	\
	etats.h			\
	etats_latex.h		\
	etats_onglet.h		\
	etats_support.h		\
	fenetre_principale.h	\
	fichiers_gestion.h	\
	format.h		\
	gtk_combofix.h		\
	help.h			\
	imputation_budgetaire.h	\
	include.h		\
	main.h			\
	menu.h			\
	navigation.h		\
	operations_comptes.h	\
	parametres.h		\
	parse_cmdline.h		\
	print_config.h		\
	qif.h			\
	import.h		\
	html.h			\
	tip.h			\
	structures.h		\
	tiers_onglet.h		\
	traitement_variables.h	\
	utils.h			\
	utils_c.h		\
	utils_buttons.h		\
	utils_comptes.h		\
	utils_dates.h		\
	utils_editables.h	\
	utils_file_selection.h	\
	utils_files.h		\
	utils_operations.h	\
	utils_rapprochements.h	\
	utils_str.h		\
	utils_types.h		\
	search_glist.h


EXTRA_DIST = grisbi.desktop grisbi.keys grisbi.mime $(grisbi_INCLUDES)

mimedir = $(datadir)/mime-info
mime_DATA = grisbi.keys grisbi.mime

if HAVE_PLUGINS
SUBDIRS = plugins/g2banking plugins/gnucash plugins/ofx plugins/openssl
endif
