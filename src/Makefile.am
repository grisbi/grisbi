# Process this file with automake to produce Makefile.in

localedir = $(datadir)/locale
INCLUDES = -I$(top_srcdir) -I$(includedir) $(GNOME_INCLUDEDIR) $(EXTRA_GNOME_CFLAGS) -I../intl -DLOCALEDIR=\"$(localedir)\" -DCHEMIN_LOGO="\"$(datadir)/pixmaps/logo_grisbi.xpm\"" -DAPPLET_BIN_DIR="\"$(bindir)/grisbi_applet\""

bin_PROGRAMS = grisbi

grisbi_SOURCES = 		\
	accueil.c		\
	fenetre_principale.c	\
	operations_liste.c	\
	banque.c		\
	fichier_configuration.c	\
	operations_onglet.c	\
	barre_outils.c		\
	fichiers_gestion.c	\
	parametres.c		\
	categories_onglet.c	\
	fichiers_io.c		\
	patienter.c		\
	comptes_gestion.c	\
	comptes_onglet.c	\
	gtk_combofix.c		\
	qif.c			\
	comptes_traitements.c	\
	help.c			\
	tiers_onglet.c		\
	devises.c		\
	main.c			\
	traitement_variables.c	\
	echeancier_formulaire.c	\
	menu.c			\
	echeancier_liste.c	\
	operations_classement.c	\
	equilibrage.c		\
	operations_comptes.c	\
	ventilation.c		\
	erreur.c		\
	operations_formulaire.c \
	type_operations.c	\
	exercice.c		\
	imputation_budgetaire.c	\
	affichage.c		\
	etat_io.c		\
	etats_onglet.c		\
	etats_config.c		\
	etats_calculs.c		\
	etats_support.c		\
	etats_gtktable.c	\
	etats_gnomeprint.c	\
	impression.c		\
	affichage_liste.c



grisbi_LDADD =	$(GNOME_LIBDIR)	$(GNOMEUI_LIBS) $(EXTRA_GNOME_LIBS) -lgnomeprint


EXTRA_DIST = grisbi.desktop grisbi.png logo_grisbi.xpm grisbi.keys grisbi.mime

Applicationsdir = $(datadir)/gnome/apps/Applications
Applications_DATA = grisbi.desktop

appicondir = $(datadir)/pixmaps
appicon_DATA = grisbi.png logo_grisbi.xpm

mimedir = $(datadir)/mime-info
mime_DATA = grisbi.keys grisbi.mime


