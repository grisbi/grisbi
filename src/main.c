/* *******************************************************************************/
/*                                 GRISBI                                        */
/* Programme de gestion financière personnelle                                   */
/*           	  license : GPL                                                  */
/*                                                                               */
/*     Copyright (C)    2000-2003 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2004 Benjamin Drieu (bdrieu@april.org)              */
/*                      http://www.grisbi.org                                    */
/*      Version : 0.5.1                                                          */
/*                                                                               */
/* *******************************************************************************/

/* *******************************************************************************/
/*                                                                               */
/*     This program is free software; you can redistribute it and/or modify      */
/*     it under the terms of the GNU General Public License as published by      */
/*     the Free Software Foundation; either version 2 of the License, or         */
/*     (at your option) any later version.                                       */
/*                                                                               */
/*     This program is distributed in the hope that it will be useful,           */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*     GNU General Public License for more details.                              */
/*                                                                               */
/*     You should have received a copy of the GNU General Public License         */
/*     along with this program; if not, write to the Free Software               */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                               */
/* *******************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_G2BANKING
#include "gbanking.h"
#endif


/* Fichier de base contenant la procédure main */


#include "include.h"
#include "fichier_configuration_constants.h"


/*START_INCLUDE*/
#include "main.h"
#include "menu.h"
#include "operations_liste.h"
#include "fichier_configuration.h"
#include "utils.h"
#include "tip.h"
#include "erreur.h"
#include "data_account.h"
#include "traitement_variables.h"
#include "fichiers_gestion.h"
#include "parse_cmdline.h"
#include "etats_config.h"
#include "parametres.h"
#include "etats_calculs.h"
#include "etats_onglet.h"
#include "utils_files.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/




/* vbox ajoutée dans la fenetre de base, contient le menu et la fenetre d'utilisation */

GtkWidget *window;
GtkWidget *window_vbox_principale;
GtkWidget *menu_general;
GtkItemFactory *item_factory_menu_general;
gint id_fonction_idle;

#ifdef HAVE_G2BANKING
AB_BANKING *gbanking=0;
#endif


/*START_EXTERN*/
extern GtkWidget *bouton_dupliquer_etat;
extern GtkWidget *bouton_effacer_etat;
extern GtkWidget *bouton_exporter_etat;
extern GtkWidget *bouton_personnaliser_etat;
extern gint compte_courant;
extern struct struct_etat *etat_courant;
extern gint hauteur_window;
extern GtkWidget *label_etat_courant;
extern gint largeur_window;
extern GSList *list_struct_accounts;
extern GSList *liste_struct_etats;
extern gchar *nom_fichier_comptes;
extern GtkWidget *notebook_aff_donnees;
extern GtkWidget *notebook_config_etat;
extern GtkWidget *notebook_general;
extern GtkWidget *notebook_selection;
extern GtkTreeSelection * selection;
extern GtkStyle *style_label_nom_compte;
extern GtkWidget *tree_view;
/*END_EXTERN*/





/***********************************************************************************************
 ** Début de la procédure principale **
 ***********************************************************************************************/

/**                                                                              
 * Main function
 *
 * @param argc number of arguments
 * @param argv arguments
 *
 * @return Nothing
 */
int main (int argc, char *argv[])
{
		initialize_debugging();
	
#ifndef _WIN32
    struct sigaction sig_sev;
#endif
    struct stat buffer_stat;
    cmdline_options  opt;

#ifdef HAVE_G2BANKING
    int rv;
#endif

    /* on ajoute la possibilité de mettre l'option --onglet dans la ligne de commande */
    /* Permet d'ouvrir le fichier demandé sur l'onglet désiré  */
    /* w=-1 : fenêtre de configuration */
    /*    x=numéro de l'onglet de configuration (0-9) */
    /* w=0 : onglet d'accueil (identique sans argument) */
    /* w=1 : onglet des opérations */
    /* w=2 : onglet de l'échéancier */
    /* w=3 : onglet des comptes */
    /* w=4 : onglet des tiers */
    /* w=5 : onglet des catégories */
    /* w=6 : onglet des imputations budgétaires */
    /* w=7 : onglet des états */
    /*   x=numéro de l'état à afficher */
    /*   y=absent pour rester sur l'état, numéro de l'onglet principal de personnalisation sinon */
    /*   z=numéro de l'onglet secondaire de la personnalisation */

    /*   Exemples : */
    /* grisbi --onglet=3 mes_comptes */
    /* place grisbi directement sur l'échéancier du fichier mes_comptes */
    /* grisbi --onglet=-1,3 mes_comptes */
    /* affiche la configuration de grisbi et la place sur Affichage */
    /* grisbi --onglet=7,2 mes_comptes */
    /* affiche le 3ème état */
    /* grisbi --onglet=7,2,2,2 mes_comptes */
    /* affiche l'onglet opération de l'onglet Affichage des données du 3ème état */

#ifdef _WIN32
    /* we store the path of the running file to use it for pixmaps, help and locales .... */
    win32_set_app_path(argv[0]);
    /* needed to be able to use the "common" installation of GTK libraries */
    win32_make_sure_the_gtk2_dlls_path_is_in_PATH(); 
#endif

	bindtextdomain ("grisbi-tips", LOCALEDIR);
	bind_textdomain_codeset ("grisbi-tips", "UTF-8");

	bindtextdomain (PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (PACKAGE, "UTF-8");
	textdomain (PACKAGE);

	/* FIXME : tous les arguments du gnome_init... */
	/* 	gnome_init_with_popt_table ("Grisbi", VERSION, argc, argv, options, 0, &ctx); */

	gtk_init(&argc, &argv);

	/* on commence par détourner le signal SIGSEGV */
#ifndef _WIN32
    /* sauf sous Windows*/
	memset ( &sig_sev, 0, sizeof ( struct sigaction ));
	sig_sev.sa_handler = traitement_sigsegv;
	sig_sev.sa_flags = 0;
	sigemptyset (&(sig_sev.sa_mask));

	if ( sigaction ( SIGSEGV, &sig_sev, NULL ))
	    printf (_("Error on sigaction: SIGSEGV won't be trapped\n"));
#endif

	/* parse des options de la ligne de commande */
	if (parse_options(argc, argv, &opt)) 
	{
		/* en cas d'erreur dans les options ou si on a passé --version ou --help */
		exit(0);
	}
	
#ifdef HAVE_G2BANKING
        gbanking=GBanking_new("grisbi", 0);
        GBanking_SetImportContextFn(gbanking, GrisbiBanking_ImportContext);
        rv=AB_Banking_Init(gbanking);
        if (rv) {
          printf (_("Could not initialize AqBanking, "
                    "online banking will not be available\n"));
          AB_Banking_free(gbanking);
          gbanking=0;
        }
#endif

	/*  Création de la fenêtre principale */

	window = gtk_window_new ( GTK_WINDOW_TOPLEVEL );

	gtk_signal_connect ( GTK_OBJECT (window),
			     "delete_event",
			     GTK_SIGNAL_FUNC ( fermeture_grisbi ),
			     NULL );

	gtk_window_set_policy ( GTK_WINDOW ( window ),
				TRUE,
				TRUE,
				FALSE );

	/* 	création de la pixmap du logiciel */

	if ( utf8_stat ( g_strconcat ( PIXMAPS_DIR, C_DIRECTORY_SEPARATOR, "grisbi.png", NULL ),&buffer_stat ) != -1 )
	    gtk_window_set_default_icon_from_file ( g_strconcat(PIXMAPS_DIR,
								C_DIRECTORY_SEPARATOR,
                                                                "grisbi.png",
								NULL),
						    NULL );

	window_vbox_principale = gtk_vbox_new ( FALSE,
						5 );
	gtk_container_add ( GTK_CONTAINER ( window ),
			    window_vbox_principale );
	gtk_widget_show ( window_vbox_principale );


	init_variables ();
	charge_configuration();

	/*   création des menus */
	menu_general = init_menus ( window_vbox_principale );
	gtk_box_pack_start ( GTK_BOX ( window_vbox_principale ),
			     menu_general,
			     FALSE,
			     FALSE,
			     0 );

	/* on grise les fonctions inutiles au départ */

	menus_sensitifs ( FALSE );

	style_label_nom_compte = NULL;


	/* on met dans le menu les derniers fichiers ouverts */

	affiche_derniers_fichiers_ouverts ();

	/*   si la taille avait déjà été sauvée, on remet l'ancienne taille à
	     la fenetre */
	if ( largeur_window && hauteur_window )
	    gtk_window_set_default_size ( GTK_WINDOW ( window ),
					  largeur_window,
					  hauteur_window );
	else
	    gtk_window_set_default_size ( GTK_WINDOW ( window ),
					  640, 480 );


	gtk_widget_show ( window );

	/*   initialisation des couleurs  */

	initialisation_couleurs_listes ();

	/* 	on met en place l'idle */

	demarrage_idle ();

	/* on vérifie les arguments de ligne de commande */

	if (opt.fichier != NULL) 
	{
		/* nom de fichier sur la ligne de commande; c'est celui-là qu'on ouvre */
		nom_fichier_comptes = opt.fichier;
		ouverture_confirmee();
	}
	else 
	{
		/* ouvre le dernier fichier si defini et si chargement auto. */
		if ( etat.dernier_fichier_auto
		     &&
		     nom_fichier_comptes
		     &&
		     strlen ( nom_fichier_comptes ) )
		    ouverture_confirmee();
	}
	
	/*   à ce niveau, le fichier doit être chargé, on met sur l'onglet demandé si nécessaire */

	if ( gsb_account_get_accounts_amount ()
	     &&
	     opt.demande_page )
	{
		switch ( opt.page_w )
		{
		    case -1:
			/* on demande l'onglet de configuration */

			/* on affiche l'onglet du 2ème argument s'il existe */

		    preferences ( NOT_A_PAGE );

			break;

		    case 0:
		    case 1:
		    case 2:
		    case 3:
		    case 4:
		    case 5:
		    case 6:

			gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ),
						opt.page_w);
			break;

		    case 7:
			/* on demande l'onglet des états  */

			gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ),
						opt.page_w);

			/* s'il y a un chiffre ensuite, on ouvre l'état correspondant à ce chiffre */

			if ( (opt.report_no != -1)
			     &&
			     liste_struct_etats )
			{
			    GSList *liste_tmp;

			    liste_tmp = g_slist_nth ( liste_struct_etats,
						      opt.report_no);

			    /* si on a sélectionné un état qui n'existait pas, on ouvre le 1er */

			    if ( !liste_tmp )
				liste_tmp = liste_struct_etats;

			    etat_courant = liste_tmp -> data;

			    remplissage_liste_etats ();

			    gtk_widget_set_sensitive ( bouton_personnaliser_etat,
						       TRUE );
			    gtk_widget_set_sensitive ( bouton_exporter_etat,
						       TRUE );
			    gtk_widget_set_sensitive ( bouton_dupliquer_etat,
						       TRUE );
			    gtk_widget_set_sensitive ( bouton_effacer_etat,
						       TRUE );

			    gtk_label_set_text ( GTK_LABEL ( label_etat_courant ),
						 etat_courant -> nom_etat );


			    rafraichissement_etat ( etat_courant );

			    /* s'il y a une suite dans la demande en ligne de commande, on ouvre la personnalisation */

			    if ( opt.customization_tab_no != -1 )
			    {
				personnalisation_etat ();

				/* le 1er chiffre correspond aux 1ers onglets */

				gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_config_etat ),
							opt.customization_tab_no);

				/* s'il y a encore un chiffre, c'est pour le sous onglet */

				if ( opt.subcustom_tab_no != -1 )
				{
				    switch ( opt.customization_tab_no )
				    {
					case 0:

					    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_selection ),
								    opt.subcustom_tab_no);
					    break;
					case 2:
					    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_aff_donnees ),
								    opt.subcustom_tab_no);
					    break;
				    }
				}
			    }
			}
			break;
		}
	}

	display_tip ( FALSE );

	gtk_main ();

	exit(0);
}
/************************************************************************************************/


/************************************************************************************************/
gboolean utilisation_temps_idle ( gpointer null )
{
    GSList *list_tmp;

    if ( !gsb_account_get_accounts_amount () )
    {
	if ( DEBUG )
	    printf ( "termine_idle\n" );
	return FALSE;
    }

/*     dans l'ordre, on va créer et remplir la liste d'opé du compte courant, */
/*     le 1er à être ouvert, puis les autres comptes */

/*     réalisation de tous les list_store */
/* 	de cette manière, lors du remplissage, les opé seront ajoutées */
/* 	directement au tree view */

    list_tmp = list_struct_accounts;

    while ( list_tmp )
    {
	gint i;

	i = gsb_account_get_no_account ( list_tmp -> data );

	if ( !GTK_WIDGET_REALIZED ( gsb_account_get_tree_view (i) ))
	{
	    if( DEBUG )
		printf ( "realize tree_view compte %d\n", i );

	    gtk_widget_realize ( gsb_account_get_tree_view (i) );
	    return TRUE;
	}

	list_tmp = list_tmp -> next;
    }

 
/*     remplissage du list_store du compte courant */
/*     on remplit par parties de x opés, invisible ici */
/* 	une fois que tout le compte a été remplit, OPE_EN_COURS = -1 */


    if ( gsb_account_get_last_transaction (compte_courant) != GINT_TO_POINTER (-1))
    {
	if ( DEBUG
	     &&
	     !gsb_account_get_last_transaction (compte_courant) )
	    printf ( "remplissage compte courant no %d par idle\n", compte_courant );

	ajoute_operations_compte_dans_list_store ( compte_courant,
						   1 );
	return TRUE;
    }

/*     update visibles rows for current account */

    if ( !gsb_account_get_finished_visible_rows (compte_courant) )
    {
	if ( DEBUG )
	    printf ( "show or hide the rows for the current account no %d by idle\n", compte_courant );

	 set_visibles_rows_on_account ( compte_courant );

	return TRUE;
    }


/*     mise à jour de la couleur du fond du compte courant */

    if ( !gsb_account_get_finished_background_color (compte_courant) )
    {
	if ( DEBUG )
	    printf ( "mise en place couleur du fond de liste compte courant no %d par idle\n", compte_courant );

	gsb_transactions_list_set_background_color ( compte_courant );

	return TRUE;
    }

/*     mise à jour des soldes du compte courant */

    if ( !gsb_account_get_finished_balance_showed (compte_courant) )
    {
	if ( DEBUG )
	    printf ( "mise en place des soldes de liste compte courant no %d par idle\n", compte_courant );

	gsb_transactions_list_set_transactions_balances ( compte_courant );

	return TRUE;
    }
 

/*     mise en place de la sélection du compte courant */

    if ( !gsb_account_get_finished_selection_transaction (compte_courant) )
    {
	if ( DEBUG )
	    printf ( "mise en place de la selection du compte courant no %d par idle\n", compte_courant );

	gsb_transactions_list_set_current_transaction ( gsb_account_get_current_transaction (compte_courant),
							compte_courant );

	return TRUE;
    }
 	

/*     création du list_store des différents comptes */

    list_tmp = list_struct_accounts;

    while ( list_tmp )
    {
	gint i;

	i = gsb_account_get_no_account ( list_tmp -> data );

	if ( gsb_account_get_last_transaction (i) != GINT_TO_POINTER (-1))
	{
	    if ( DEBUG
		 &&
		 gsb_account_get_last_transaction (i) )
		printf ( "remplissage compte %d par idle\n", i );

	    ajoute_operations_compte_dans_list_store ( i,
						       1 );
	    return TRUE;
	}

	list_tmp = list_tmp -> next;
    }


 /*     update visibles rows for current account */

    list_tmp = list_struct_accounts;

    while ( list_tmp )
    {
	gint i;

	i = gsb_account_get_no_account ( list_tmp -> data );

	if ( !gsb_account_get_finished_visible_rows (i) )
	{
	    if ( DEBUG )
		printf ( "show or hide the rows for the account no %d by idle\n", i );

	    set_visibles_rows_on_account ( i );

	    return TRUE;
	}

	list_tmp = list_tmp -> next;
    }


/*     mise à jour de la couleur du fond des différents comptes */

    list_tmp = list_struct_accounts;

    while ( list_tmp )
    {
	gint i;

	i = gsb_account_get_no_account ( list_tmp -> data );

	if ( !gsb_account_get_finished_background_color (i) )
	{
	    if ( DEBUG )
		printf ( "mise en place couleur du fond de liste compte no %d par idle\n", i );

	    gsb_transactions_list_set_background_color ( i );

	    return TRUE;
	}

	list_tmp = list_tmp -> next;
    }

/*     mise à jour des soldes des différents comptes */

    list_tmp = list_struct_accounts;

    while ( list_tmp )
    {
	gint i;

	i = gsb_account_get_no_account ( list_tmp -> data );

	if ( !gsb_account_get_finished_balance_showed (i) )
	{
	    if ( DEBUG )
		printf ( "mise en place des soldes de liste compte no %d par idle\n", i );

	    gsb_transactions_list_set_transactions_balances ( i );

	    return TRUE;
	}

	list_tmp = list_tmp -> next;
    }


    /*     mise en place de la sélection du compte courant */

    list_tmp = list_struct_accounts;

    while ( list_tmp )
    {
	gint i;

	i = gsb_account_get_no_account ( list_tmp -> data );

	if ( !gsb_account_get_finished_selection_transaction (i) )
	{
	    if ( DEBUG )
		printf ( "mise en place de la selection du compte courant no %d par idle\n", i );

	    gsb_transactions_list_set_current_transaction ( gsb_account_get_current_transaction (i),
							    i );
	    return TRUE;
	}

	list_tmp = list_tmp -> next;
    }

	

    id_fonction_idle = 0; 
    if ( DEBUG )
	printf ( "termine_idle\n" );

    return FALSE;
}
/************************************************************************************************/


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
