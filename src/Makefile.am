# Process this file with automake to produce Makefile.in

localedir = $(datadir)/locale
INCLUDES = -I$(top_srcdir) -I$(includedir) -I../intl -DLOCALEDIR=\"$(localedir)\" -DPIXMAPS_DIR="\"$(datadir)/pixmaps/grisbi\"" -DLOGO_PATH="\"$(datadir)/pixmaps/grisbi/grisbi-logo.png\"" -DANIM_PATH="\"$(datadir)/pixmaps/grisbi/euro.gif\"" -DHELP_PATH="\"$(datadir)/grisbi/help\"" -Wall @g2banking_includes@
bin_PROGRAMS = grisbi

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
	data_currency.c		\
	data_payment.c		\
	devises.c		\
	dialog.c		\
	echeancier_formulaire.c	\
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
	exercice.c		\
	export.c		\
	export_csv.c		\
	fenetre_principale.c	\
	fichiers_gestion.c	\
	gbanking.c		\
	gnucash.c               \
	go-charmap-sel.c	\
	go-optionmenu.c		\
	gsb_assistant.c		\
	gsb_data_account.c	\
	gsb_data_budget.c	\
	gsb_data_category.c	\
	gsb_data_form.c		\
	gsb_data_payee.c	\
	gsb_data_report.c	\
	gsb_data_report_amout_comparison.c	\
	gsb_data_report_text_comparison.c	\
	gsb_data_scheduled.c	\
	gsb_data_transaction.c	\
	gsb_file_config.c	\
	gsb_file_load.c		\
	gsb_file_others.c	\
	gsb_file_save.c		\
	gsb_file_util.c		\
	gsb_form.c		\
	gsb_form_config.c	\
	gsb_payment_method.c	\
	gsb_payment_method_config.c	\
	gsb_scheduler_list.c	\
	gsb_status.c		\
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
	ofx.c                   \
	operations_comptes.c	\
	operations_formulaire.c \
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
	utils_devises.c		\
	utils_editables.c	\
	utils_exercices.c	\
	utils_file_selection.c	\
	utils_files.c		\
	utils_montants.c	\
	utils_operations.c	\
	utils_rapprochements.c	\
	utils_str.c		\
	utils_types.c		\
	utils_xml.c		

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
	gsb_data_account.h	\
	gsb_data_budget.h	\
	gsb_data_category.h	\
	gsb_data_form.h		\
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
	gsb_payment_method.h	\
	gsb_payment_method_config.h	\
	gsb_scheduler_list.h	\
	gsb_transactions_list.h	\
	data_currency.h		\
	data_payment.h		\
	devises.h		\
	dialog.h		\
	echeancier_formulaire.h	\
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
	exercice.h		\
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
	operations_formulaire.h	\
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
	utils_devises.h		\
	utils_editables.h	\
	utils_exercices.h	\
	utils_file_selection.h	\
	utils_files.h		\
	utils_montants.h	\
	utils_operations.h	\
	utils_rapprochements.h	\
	utils_str.h		\
	utils_types.h		\
	utils_xml.h		\
	search_glist.h		\
	gbanking.h


EXTRA_DIST = grisbi.desktop grisbi.keys grisbi.mime $(grisbi_INCLUDES)

mimedir = $(datadir)/mime-info
mime_DATA = grisbi.keys grisbi.mime
