# Process this file with automake to produce Makefile.in

localedir = $(datadir)/locale
INCLUDES = -I$(top_srcdir) -I$(includedir) $(GNOME_INCLUDEDIR) -I../intl -DLOCALEDIR=\"$(localedir)\" -DPIXMAPS_DIR="\"$(datadir)/pixmaps/grisbi\"" -DCHEMIN_LOGO="\"$(datadir)/pixmaps/grisbi/grisbi-logo.png\""
bin_PROGRAMS = grisbi

grisbi_SOURCES = 		\
	accueil.c		\
	affichage.c		\
	affichage_liste.c	\
	banque.c		\
	barre_outils.c		\
	calendar.c		\
	categories_onglet.c	\
	comptes_gestion.c	\
	comptes_onglet.c	\
	comptes_traitements.c	\
	devises.c		\
	echeancier_formulaire.c	\
	echeancier_liste.c	\
	equilibrage.c		\
	erreur.c		\
	etat_io.c		\
	etats_affiche.c		\
	etats_calculs.c		\
	etats_config.c		\
	etats_gtktable.c	\
	etats_latex.c		\
	etats_onglet.c		\
	etats_support.c		\
	exercice.c		\
	fenetre_principale.c	\
	fichier_configuration.c	\
	fichiers_gestion.c	\
	fichiers_io.c		\
	gtk_combofix.c		\
	help.c			\
	imputation_budgetaire.c	\
	main.c			\
	menu.c			\
	operations_classement.c	\
	operations_comptes.c	\
	operations_formulaire.c \
	operations_liste.c	\
	operations_onglet.c	\
	parametres.c		\
	patienter.c		\
	qif.c			\
	tiers_onglet.c		\
	traitement_variables.c	\
	type_operations.c	\
	ventilation.c


#grisbi_LDADD =	$(GNOME_LIBDIR)	$(GNOMEUI_LIBS)


EXTRA_DIST = grisbi.desktop grisbi.keys grisbi.mime

Applicationsdir = $(datadir)/gnome/apps/Applications
Applications_DATA = grisbi.desktop

mimedir = $(datadir)/mime-info
mime_DATA = grisbi.keys grisbi.mime


