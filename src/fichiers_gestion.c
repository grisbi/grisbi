/* ************************************************************************** */
/* Ce fichier comprend toutes les opérations concernant le traitement	      */
/* des fichiers								      */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2004 Benjamin Drieu (bdrieu@april.org)	      */
/* 			http://www.grisbi.org				      */
/*                                                                            */
/*  This program is free software; you can redistribute it and/or modify      */
/*  it under the terms of the GNU General Public License as published by      */
/*  the Free Software Foundation; either version 2 of the License, or         */
/*  (at your option) any later version.                                       */
/*                                                                            */
/*  This program is distributed in the hope that it will be useful,           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*  GNU General Public License for more details.                              */
/*                                                                            */
/*  You should have received a copy of the GNU General Public License         */
/*  along with this program; if not, write to the Free Software               */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                            */
/* ************************************************************************** */


#include "include.h"



/*START_INCLUDE*/
#include "fichiers_gestion.h"
#include "menu.h"
#include "devises.h"
#include "patienter.h"
#include "utils_montants.h"
#include "fenetre_principale.h"
#include "fichiers_io.h"
#include "categories_onglet.h"
#include "imputation_budgetaire.h"
#include "tiers_onglet.h"
#include "comptes_traitements.h"
#include "dialog.h"
#include "erreur.h"
#include "utils_file_selection.h"
#include "gsb_account.h"
#include "operations_comptes.h"
#include "echeancier_liste.h"
#include "traitement_variables.h"
#include "main.h"
#include "accueil.h"
#include "utils_files.h"
#include "affichage_liste.h"
#include "fichier_configuration.h"
#include "utils.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void ajoute_new_file_liste_ouverture ( gchar *path_fichier );
static gchar *demande_nom_enregistrement ( void );
static gboolean enregistrement_backup ( void );
static void fichier_selectionne ( GtkWidget *selection_fichier);
/*END_STATIC*/




gchar *nom_fichier_backup;



/*START_EXTERN*/
extern gboolean block_menu_cb ;
extern gint compression_backup;
extern gint compression_fichier;
extern gchar *dernier_chemin_de_travail;
extern gint id_temps;
extern GtkItemFactory *item_factory_menu_general;
extern GSList *liste_struct_echeances;
extern GSList *liste_struct_etats;
extern GtkWidget *main_hpaned, *main_vbox, *main_statusbar;
extern gint max;
extern gint mise_a_jour_fin_comptes_passifs;
extern gint mise_a_jour_liste_comptes_accueil;
extern gint mise_a_jour_soldes_minimaux;
extern gint nb_derniers_fichiers_ouverts;
extern gint nb_max_derniers_fichiers_ouverts;
extern gchar *nom_fichier_comptes;
extern GtkWidget *notebook_general;
extern gint rapport_largeur_colonnes[TRANSACTION_LIST_COL_NB];
extern GSList *scheduled_transactions_taken;
extern GSList *scheduled_transactions_to_take;
extern gchar **tab_noms_derniers_fichiers_ouverts;
extern gchar *titre_fichier;
extern GtkWidget *window;
extern GtkWidget *window_vbox_principale;
/*END_EXTERN*/



/** called by menu, close the last file and open a new one
 * \param none
 * \return FALSE
 * */
gboolean new_file ( void )
{
    kind_account type_de_compte;

    /*   si la fermeture du fichier en cours se passe mal, on se barre */

    if ( !fermer_fichier () )
	return FALSE;

    init_variables ();

    type_de_compte = demande_type_nouveau_compte ();

    if ( type_de_compte == -1 )
	return FALSE;

    /*     création de la 1ère devise */

    if ( ! ajout_devise ( NULL ) )
	return FALSE;

    gsb_account_set_current_account ( gsb_account_new( type_de_compte ));

    /* si la création s'est mal passée, on se barre */

    if ( gsb_account_get_current_account () == -1 )
	return FALSE;

    init_variables_new_file ();
    init_gui_new_file ();

    modification_fichier ( TRUE );
    return FALSE;
}
/* ************************************************************************************************************ */


/* init the variables when begin a new
 * */
void init_variables_new_file ( void )
{
    /*   la taille des colonnes est automatique au départ, on y met les rapports de base */

    etat.largeur_auto_colonnes = 1;
    rapport_largeur_colonnes[0] = 11;
    rapport_largeur_colonnes[1] = 13;
    rapport_largeur_colonnes[2] = 30;
    rapport_largeur_colonnes[3] = 3;
    rapport_largeur_colonnes[4] = 11;
    rapport_largeur_colonnes[5] = 11;
    rapport_largeur_colonnes[6] = 11;

    /* création des listes d'origine */

    creation_liste_categories ();
}
/* ************************************************************************************************************ */



/**
 * Initialize user interface part when a new accounts file is created.
 */
void init_gui_new_file ( void )
{
    /* dégrise les menus nécessaire */
    
    menus_sensitifs ( TRUE );

    creation_liste_categ_combofix ();

    /*     récupère l'organisation des colonnes  */
    recuperation_noms_colonnes_et_tips ();

    /* Create main widget. */
    gtk_box_pack_start ( GTK_BOX ( window_vbox_principale), create_main_widget(),
			 TRUE, TRUE, 0 );

    gsb_account_list_gui_change_current_account ( GINT_TO_POINTER ( gsb_account_get_current_account () ) );

    /* Display accounts in menus */
    gsb_account_list_gui_create_list ();

    /* Affiche le nom du fichier de comptes dans le titre de la fenetre */
    affiche_titre_fenetre();

    gtk_notebook_set_page ( GTK_NOTEBOOK( notebook_general ), GSB_HOME_PAGE );

    gtk_widget_show ( notebook_general );
}



void ouvrir_fichier ( void )
{
    GtkWidget *selection_fichier;

    selection_fichier = file_selection_new ( _("Open an accounts file"),FILE_SELECTION_MUST_EXIST);
    gtk_window_set_position ( GTK_WINDOW ( selection_fichier ),
			      GTK_WIN_POS_MOUSE);

    file_selection_set_filename ( GTK_FILE_SELECTION ( selection_fichier ),
				      dernier_chemin_de_travail );

    gtk_file_selection_complete ( GTK_FILE_SELECTION(selection_fichier), ".gsb" );

    gtk_signal_connect_object ( GTK_OBJECT ( GTK_FILE_SELECTION ( selection_fichier ) -> cancel_button ),
				"clicked",
				GTK_SIGNAL_FUNC ( gtk_widget_destroy ),
				GTK_OBJECT ( selection_fichier ));
    gtk_signal_connect_object ( GTK_OBJECT ( GTK_FILE_SELECTION ( selection_fichier ) -> ok_button ),
				"clicked",
				GTK_SIGNAL_FUNC ( fichier_selectionne ),
				GTK_OBJECT (selection_fichier) );

    gtk_widget_show ( selection_fichier );


}
/* ************************************************************************************************************ */

/* ************************************************************************************************************ */
void ouverture_fichier_par_menu ( gpointer null,
				  gint no_fichier )
{
    /*   si la fermeture du fichier courant se passe mal, on se barre */

    if ( !fermer_fichier() )
	return;

    nom_fichier_comptes = tab_noms_derniers_fichiers_ouverts[no_fichier];

    ouverture_confirmee ();
}
/* ************************************************************************************************************ */



/* ************************************************************************************************************ */
void fichier_selectionne ( GtkWidget *selection_fichier)
{

    /*   on cache la fenetre de sélection et ferme le fichier en cours */
    /*     si lors de la fermeture, on a voulu enregistrer mais ça n'a pas marché => return */

    gtk_widget_hide ( selection_fichier );

    /*   si la fermeture du fichier se passe mal, on se barre */

    if ( !fermer_fichier() )
    {
	gtk_widget_hide ( selection_fichier );
	return;
    }


    /* on prend le nouveau nom du fichier */

    nom_fichier_comptes = file_selection_get_filename ( GTK_FILE_SELECTION ( selection_fichier)) ;

    gtk_widget_hide ( selection_fichier );

    /* on met le répertoire courant dans la variable correspondante */

    dernier_chemin_de_travail = file_selection_get_last_directory(GTK_FILE_SELECTION ( selection_fichier),TRUE);

    ouverture_confirmee ();
}
/* ************************************************************************************************************ */



/* ************************************************************************************************************ */
/* Fonction ouverture_confirmee */
/* ouvre le fichier dont le nom est dans nom_fichier_comptes */
/* ne se préocupe pas d'un ancien fichier ou d'une initialisation de variables */
/* ************************************************************************************************************ */

void ouverture_confirmee ( void )
{
    gint i;
    GtkWidget * widget;
    gchar * item_name = NULL;
    GSList *list_tmp;

    if ( DEBUG )
	printf ( "ouverture_confirmee\n" );

    mise_en_route_attente ( _("Load an accounts file") );

    /*  si charge opérations renvoie FALSE, c'est qu'il y a eu un pb et un message est déjà affiché */

    if ( !charge_operations ( nom_fichier_comptes ) )
    {
	/* 	  le chargement du fichier a planté, si l'option sauvegarde à l'ouverture est activée, on */
	/* propose de charger l'ancien fichier */

	annulation_attente ();

	if ( etat.sauvegarde_demarrage )
	{
	    gchar *nom;
	    gint result;
	    gchar **parametres;

	    /* on crée le nom de la sauvegarde */

	    nom = g_strdup ( nom_fichier_comptes );
	    i=0;

	    parametres = g_strsplit ( nom, C_DIRECTORY_SEPARATOR, 0);
	    while ( parametres[i] )
		i++;

	    nom = g_strconcat ( my_get_gsb_file_default_dir(),
				C_DIRECTORY_SEPARATOR,
				parametres [i-1],
				".bak",
				NULL );
	    g_strfreev ( parametres );

	    result = utf8_open ( nom, O_RDONLY);
	    if (result == -1)
		return;
	    else
		close (result);

	    mise_en_route_attente ( _("Loading backup") );

	    if ( charge_operations ( nom ) )
	    {
		/* on a réussi a charger la sauvegarde */
		dialogue_error_hint ( _("Grisbi was unable to load file.  However, Grisbi loaded a backup file instead but all changes made since this backup were possibly lost."),
				      g_strdup_printf ( _("Error loading file '%s'"), nom_fichier_comptes) );
	    }
	    else
	    {
		/* le chargement de la sauvegarde a échoué */

		annulation_attente ();
		dialogue_error_hint ( _("Grisbi was unable to load file.  Additionally, Grisbi was unable to load a backup file instead."),
				      g_strdup_printf ( _("Error loading file '%s'"), nom_fichier_comptes) );
		return;
	    }
	}
	else
	{
	    menus_sensitifs ( FALSE );
	    return;
	}
    }
    else
    {
	/* 	    l'ouverture du fichier s'est bien passée */
	/* 	si on veut faire une sauvegarde auto à chaque ouverture, c'est ici */

	if ( etat.sauvegarde_demarrage )
	{
	    gchar *nom;
	    gchar **parametres;

	    update_attente ( _("Autosave") );

	    nom = g_strdup ( nom_fichier_comptes );

	    i=0;

	    /* 	      on récupère uniquement le nom du fichier, pas le chemin */

	    parametres = g_strsplit ( nom,
				      C_DIRECTORY_SEPARATOR,
				      0);

	    while ( parametres[i] )
		i++;

	    nom = g_strconcat ( my_get_gsb_file_default_dir(),
				C_DIRECTORY_SEPARATOR,
#ifndef _WIN32
                                ".",
#endif
				parametres [i-1],
				".bak",
				NULL );

	    g_strfreev ( parametres );

	    /* on force l'enregistrement */

	    enregistre_fichier ( nom );
	}
    }

    update_attente ( _("Checking schedulers"));

    /*     on vérifie si des échéances sont à récupérer */

    gsb_scheduler_check_scheduled_transactions_time_limit ();

    /* affiche le nom du fichier de comptes dans le titre de la fenetre */

    affiche_titre_fenetre();

    /* on save le nom du fichier dans les derniers ouverts */

    if (nom_fichier_comptes)
	ajoute_new_file_liste_ouverture ( nom_fichier_comptes );

    /*     récupère l'organisation des colonnes  */

    recuperation_noms_colonnes_et_tips ();

    /*  on calcule les soldes courants */
    /*     important de le faire avant l'affichage de l'accueil */
    /* 	va afficher le message qu'une fois tous les comptes remplis */
    /* 	(donc après l'idle ) */
    
    update_attente ( _("Checking amounts"));

    list_tmp = gsb_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;

	i = gsb_account_get_no_account ( list_tmp -> data );

	gsb_account_set_current_balance ( i, 
					  calcule_solde_compte ( i ));
	gsb_account_set_marked_balance ( i, 
					 calcule_solde_pointe_compte ( i ));

	/* 	on met à jour les affichage solde mini et autres */

	gsb_account_set_mini_balance_authorized_message ( i,
							  gsb_account_get_current_balance (i) < gsb_account_get_mini_balance_authorized (i));
	gsb_account_set_mini_balance_wanted_message ( i,
						      gsb_account_get_current_balance (i) < gsb_account_get_mini_balance_wanted (i) );

	list_tmp = list_tmp -> next;
    }

    /*     on va afficher la page d'accueil */
    /*     l'appel se fera lors de la création de la fenêtre principale */

    mise_a_jour_liste_comptes_accueil = 1;
    mise_a_jour_soldes_minimaux = 1;
    mise_a_jour_fin_comptes_passifs = 1;

    /*     création de la liste des tiers pour le combofix */
    
    creation_liste_tiers_combofix ();

    /* creation de la liste des categ pour le combofix */

    creation_liste_categ_combofix ();

    /* creation de la liste des imputations pour le combofix */

    creation_liste_imputation_combofix ();

    /* on crée le notebook principal */

    update_attente ( _("Making main window"));

    create_main_widget();
    
    /*     on dégrise les menus */

    menus_sensitifs ( TRUE );

    /* On met à jour l'affichage des opérations rapprochées */
    block_menu_cb = TRUE;
    widget = gtk_item_factory_get_item ( item_factory_menu_general,
					 menu_name(_("View"), _("Show reconciled transactions"), NULL) );

    /* FIXME : à vérifier pour les 2 prochains gsb_account, mis gsb_account_get_current_account () en attendant */

    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), gsb_account_get_r (gsb_account_get_current_account ()) );

    /* On met à jour le contrôle dans le menu du nombre de lignes affichées */
    switch ( gsb_account_get_nb_rows ( gsb_account_get_current_account () ) )
      {
      case 1 :
	item_name = menu_name ( _("View"), _("Show one line per transaction"), NULL );
	break;
      case 2 :
	item_name = menu_name ( _("View"), _("Show two lines per transaction"), NULL );
	break;
      case 3 :
	item_name = menu_name ( _("View"), _("Show three lines per transaction"), NULL );
	break;
      case 4 :
	item_name = menu_name ( _("View"), _("Show four lines per transaction"), NULL );
	break;
      }

    widget = gtk_item_factory_get_item ( item_factory_menu_general, item_name );
    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), TRUE );
    block_menu_cb = FALSE;

    /* Fill menus with list of accounts. */
    gsb_account_list_gui_create_list ();

    /*     on ajoute la fentre principale à la window */
    gtk_box_pack_start ( GTK_BOX ( window_vbox_principale), main_vbox, TRUE, TRUE, 0 );
    gtk_widget_show ( main_vbox );

    mise_a_jour_accueil ();
    annulation_attente ();
}
/* ************************************************************************************************************ */






/* ************************************************************************************************************ */
/* Fonction appelé lorsqu'on veut enregistrer le fichier ( fin de prog, fermeture fichier ... ) */
/* enregistre automatiquement ou demande */
/* si origine = -1 : provient de la fonction fermeture_grisbi */
/* si origine = -2 : provient de enregistrer sous */
/* retour : TRUE si tout va bien, cad on ne veut pas enregistrer ou c'est enregistré */
/* ************************************************************************************************************ */

gboolean enregistrement_fichier ( gint origine )
{
    gint etat_force, result;
    gchar *nouveau_nom_enregistrement;

    if ( DEBUG )
	printf ( "enregistrement_fichier from %d\n", origine );

    etat_force = 0;

    if ( ( ! etat.modification_fichier && origine != -2 ) ||
	 ! gsb_account_get_accounts_amount () )
    {
	if ( DEBUG )
	    printf ( "nothing done in enregistrement_fichier\n" );
	return ( TRUE );
    }

    /* si le fichier de comptes n'a pas de nom ou si on enregistre sous un nouveau nom */
    /*     c'est ici */

    if ( !nom_fichier_comptes || origine == -2 )
	nouveau_nom_enregistrement = demande_nom_enregistrement ();
    else
	nouveau_nom_enregistrement = nom_fichier_comptes;

    if ( !nouveau_nom_enregistrement )
	return FALSE;

    /*     on vérifie que le fichier n'est pas locké */

    if ( etat.fichier_deja_ouvert
	 &&
	 !etat.force_enregistrement
	 &&
	 origine != -2 )
    {
	dialogue_conditional_hint ( g_strdup_printf( _("Can not save file \"%s\""), nom_fichier_comptes),
				    g_strdup_printf( _("Grisbi was unable to save this file because it is locked.  Please save it with another name or activate the \"%s\" option in setup.  Alternatively, choose the \"%s\" option below."),
						     _("Force saving of locked files"),
						     _("Do not show this message again")), 
				    &(etat.force_enregistrement ) );
	return ( FALSE );
    }

    /*   on a maintenant un nom de fichier */
    /*     et on sait qu'on peut sauvegarder */

    mise_en_route_attente ( _("Save file") );

    result = enregistre_fichier ( nouveau_nom_enregistrement );

    if ( result )
    {
	/* 	l'enregistrement s'est bien passé, */
	/* 	on délock le fichier (l'ancien ou le courant) */
	    
	modification_etat_ouverture_fichier ( FALSE );

	nom_fichier_comptes = nouveau_nom_enregistrement;

	/* 	... et locke le nouveau */

	modification_etat_ouverture_fichier ( TRUE );

	/* 	dans tout les cas, le fichier n'était plus ouvert à l'ouverture */

	etat.fichier_deja_ouvert = 0;
	modification_fichier ( FALSE );
	affiche_titre_fenetre ();
	ajoute_new_file_liste_ouverture ( nom_fichier_comptes );
    }

    /*     on enregistre la backup si nécessaire */

    enregistrement_backup();

    annulation_attente();

    return ( result );
}
/* ************************************************************************************************************ */


/* ************************************************************************************************************ */
gboolean enregistrer_fichier_sous ( void )
{
    return (  enregistrement_fichier ( -2 ) );
}
/* ************************************************************************************************************ */



/* ************************************************************************************************************ */
/* cette fonction est appelée pour proposer d'enregistrer si le fichier est modifié */
/* elle n'enregistre pas, elle retourne juste le choix de l'utilisateur */
/* retourne : */
/* GTK_RESPONSE_OK : veut enregistrer */
/* GTK_RESPONSE_NO : veut pas enregistrer */
/* autre gint : annuler */
/* ************************************************************************************************************ */
gint question_fermer_sans_enregistrer ( void )
{
    gchar * hint, * message = "";
    gint result;
    GtkWidget *dialog;

    /*     si le fichier n'est pas modifié on renvoie qu'on ne veut pas enregistrer */

    if ( !etat.modification_fichier )
	return GTK_RESPONSE_NO;
    
    if ( etat.sauvegarde_auto && 
	 ( !etat.fichier_deja_ouvert || etat.force_enregistrement ) &&
	 nom_fichier_comptes )
      return GTK_RESPONSE_OK;

    /*     si le fichier était déjà locké et que force enregistrement n'est pas mis, */
    /*     on prévient ici */

    dialog = gtk_message_dialog_new ( GTK_WINDOW (window), 
				      GTK_DIALOG_DESTROY_WITH_PARENT,
				      GTK_MESSAGE_WARNING, 
				      GTK_BUTTONS_NONE,
				      " " );
    if ( etat.fichier_deja_ouvert
	 &&
	 !etat.force_enregistrement )
    {
	hint = _("Save locked files?");
	message = g_strdup_printf ( _("The document '%s' is locked but modified. If you want to save it, you must cancel and save it with another name or activate the \"%s\" option in setup."),
				    (nom_fichier_comptes ? g_path_get_basename(nom_fichier_comptes) : _("unnamed")),
				    _("Force saving of locked files"));
	gtk_dialog_add_buttons ( GTK_DIALOG(dialog),
				 _("Close without saving"), GTK_RESPONSE_NO,
				 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				 NULL );
	gtk_dialog_set_default_response ( GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL ); 
    }
    else
    {
	hint = g_strdup_printf (_("Save changes to document '%s' before closing?"),
				(nom_fichier_comptes ? g_path_get_basename(nom_fichier_comptes) : _("unnamed")));
	gtk_dialog_add_buttons ( GTK_DIALOG(dialog),
				 _("Close without saving"), GTK_RESPONSE_NO,
				 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				 GTK_STOCK_SAVE, GTK_RESPONSE_OK,
				 NULL );
	gtk_dialog_set_default_response ( GTK_DIALOG(dialog), GTK_RESPONSE_OK ); 
    }

    message = g_strconcat ( message, 
			    _("If you close without saving, all of your changes will be discarded."),
			    NULL );
    
    gtk_label_set_markup ( GTK_LABEL ( GTK_MESSAGE_DIALOG(dialog)->label ), 
			   make_hint ( hint, message ) );

    gtk_window_set_modal ( GTK_WINDOW ( dialog ), TRUE );

    result = gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy ( dialog );

    return result;
}
/* ************************************************************************************************************ */



/* ************************************************************************************************************ */
/* cette fonction est appelée lors de l'enregistrement, s'il n'y a pas de nom */
/* ou si on fait un enregistrement sous */
/* elle renvoie le nouveau nom */
/* ************************************************************************************************************ */
gchar *demande_nom_enregistrement ( void )
{
    gchar *nouveau_nom;
    GtkWidget *fenetre_nom;
    gint resultat;

    fenetre_nom = file_selection_new ( _("Name the accounts file"),FILE_SELECTION_IS_SAVE_DIALOG);
    gtk_window_set_modal ( GTK_WINDOW ( fenetre_nom ),
			   TRUE );
    file_selection_set_filename ( GTK_FILE_SELECTION ( fenetre_nom ),
                                  dernier_chemin_de_travail );
    file_selection_set_entry ( GTK_FILE_SELECTION ( fenetre_nom ),
			 ".gsb" );

    resultat = gtk_dialog_run ( GTK_DIALOG ( fenetre_nom ));

    switch ( resultat )
    {
	case GTK_RESPONSE_OK :
	    nouveau_nom = file_selection_get_filename ( GTK_FILE_SELECTION ( fenetre_nom ));

	    gtk_widget_destroy ( GTK_WIDGET ( fenetre_nom ));

	    /* Les vérifications d'usage ont  été faite par la boite de dialogue*/

	    break;

	default :
	    gtk_widget_destroy ( GTK_WIDGET ( fenetre_nom ));
	    return NULL;
    }

    return nouveau_nom;
}
/* ************************************************************************************************************ */





/* ************************************************************************************************************ */
gboolean fermer_fichier ( void )
{
    gint result;
    GSList *list_tmp;


    if ( DEBUG)
	printf ( "fermer_fichier\n" );


    if ( !gsb_account_get_accounts_amount () )
	return ( TRUE );


    /*     on propose d'enregistrer */

    result = question_fermer_sans_enregistrer();

    switch ( result )
    {
	case GTK_RESPONSE_OK:

	    /* 	    on a choisi d'enregistrer, si va pas, on s'en va */

	    if ( !enregistrement_fichier (-1) )
		return ( FALSE );

	case GTK_RESPONSE_NO :

	    /* 	    l'enregistrement s'est bien passé ou on a choisi de ne pas enregistrer */

	    /*     on enregistre la config */

	    sauve_configuration ();

	    /*     s'il y a de l'idle, on le retire */

	    termine_idle ();

	    /* si le fichier n'était pas déjà ouvert, met à 0 l'ouverture */

	    if ( !etat.fichier_deja_ouvert
		 &&
		 gsb_account_get_accounts_amount ()
		 &&
		 nom_fichier_comptes )
		modification_etat_ouverture_fichier ( FALSE );

	    /*       stoppe le timer */

	    if ( id_temps )
	    {
		gtk_timeout_remove ( id_temps );
		id_temps = 0;
	    }

	    /* libère les opérations de tous les comptes */

	    list_tmp = gsb_account_get_list_accounts ();

	    while ( list_tmp )
	    {
		gint i;

		i = gsb_account_get_no_account ( list_tmp -> data );

		if ( gsb_account_get_transactions_list (i) )
		    g_slist_free ( gsb_account_get_transactions_list (i) );

		list_tmp = list_tmp -> next;
	    }

	    g_slist_free ( gsb_account_get_list_accounts () );

	    /* libère les échéances */

	    g_slist_free ( liste_struct_echeances );
	    g_slist_free ( scheduled_transactions_to_take );
	    g_slist_free ( scheduled_transactions_taken );
	    g_slist_free ( liste_struct_etats );

	    gtk_signal_disconnect_by_func ( GTK_OBJECT ( notebook_general ),
					    GTK_SIGNAL_FUNC ( change_page_notebook),
					    NULL );

	    gtk_widget_destroy ( main_vbox );

	    init_variables ();

	    affiche_titre_fenetre();

	    menus_sensitifs ( FALSE );

	    return ( TRUE );
	    break;

	default :
	    return FALSE;
    }
}
/* ************************************************************************************************************ */




/* ************************************************************************************************************ */
/* Fonction appelée une fois qu'on a nommé le fichier de compte */
/* met juste le titre dans la fenetre principale */
/* ************************************************************************************************************ */
void affiche_titre_fenetre ( void )
{
    gchar **parametres = NULL;
    gchar *titre = NULL;
    gint i=0;

    if ( DEBUG )
	printf ( "affiche_titre_fenetre\n" );

    if ( titre_fichier && strlen(titre_fichier) )
      titre = titre_fichier;
    else if ( nom_fichier_comptes )
    {
	parametres = g_strsplit ( nom_fichier_comptes, C_DIRECTORY_SEPARATOR, 0);
	while ( parametres[i] )
	  i++;
	titre = g_strdup(parametres [i-1]);
	g_strfreev ( parametres );
    }
    else
    {
      titre = g_strconcat ( "<", _("unnamed"), ">", NULL );
    }

    titre = g_strconcat ( titre, " - ", _("Grisbi"), NULL );
    gtk_window_set_title ( GTK_WINDOW ( window ), titre );

}
/* ************************************************************************************************************ */



/* ************************************************************************************************************ */
/* Fonction enregistrement_backup */
/* appelée si necessaire au début de l'enregistrement */
/* ************************************************************************************************************ */

gboolean enregistrement_backup ( void )
{
    gboolean retour;

    if ( !nom_fichier_backup || !strlen(nom_fichier_backup) )
	return FALSE;

    update_attente ( _("Saving backup") );

    mise_en_route_attente ( _("Saving backup") );

    xmlSetCompressMode ( compression_backup );

    retour = enregistre_fichier( nom_fichier_backup );

    xmlSetCompressMode ( compression_fichier );

    annulation_attente();

    return ( retour );
}
/* ************************************************************************************************************ */


/* ************************************************************************************************************ */
void ajoute_new_file_liste_ouverture ( gchar *path_fichier )
{
    gint i;
    gint position;
    gchar *dernier;

    if ( DEBUG )
	printf ( "ajoute_new_file_liste_ouverture : %s\n", path_fichier );

    if ( !nb_max_derniers_fichiers_ouverts ||
	 ! path_fichier)
	return;

    if ( nb_derniers_fichiers_ouverts < 0 )
	nb_derniers_fichiers_ouverts = 0;

    /* ALAIN-FIXME */
    /* si on n'a pas un chemin absolu, on n'enregistre pas ce fichier
       dans la liste. Du moins jusqu'à ce que quelqu'un trouve un moyen
       pour récupérer le chemein absolu */

    if ( !g_path_is_absolute ( nom_fichier_comptes ) )
    {
	/*     gchar *tmp_name, *tmp_name2;
	       tmp_name = realpath(nom_fichier_comptes, tmp_name2);

	       dialogue(tmp_name);
	       free(tmp_name);
	       dialogue(tmp_name2);
	       free(tmp_name2);*/

	return;
    }

    /* on commence par vérifier si ce fichier n'est pas dans les nb_derniers_fichiers_ouverts noms */

    position = 0;

    for ( i=0 ; i<nb_derniers_fichiers_ouverts ; i++ )
	if ( !strcmp ( path_fichier,
		       tab_noms_derniers_fichiers_ouverts[i] ))
	{
	    /* 	si ce fichier est déjà le dernier ouvert, on laisse tomber */

	    if ( !i )
		return;

	    position = i;
	}

    efface_derniers_fichiers_ouverts();

    if ( position )
    {
	/*       le fichier a été trouvé, on fait juste une rotation */

	for ( i=position ; i>0 ; i-- )
	    tab_noms_derniers_fichiers_ouverts[i] = tab_noms_derniers_fichiers_ouverts[i-1];
	if ( path_fichier )
	    tab_noms_derniers_fichiers_ouverts[0] = g_strdup ( path_fichier );
	else
	    tab_noms_derniers_fichiers_ouverts[0] = g_strdup ( "<no file>" );

	affiche_derniers_fichiers_ouverts();

	return;
    }

    /*   le fichier est nouveau, on décale tout d'un cran et on met le nouveau à 0 */

    /*   si on est déjà au max, c'est juste un décalage avec perte du dernier */
    /* on garde le ptit dernier dans le cas contraire */

    if ( nb_derniers_fichiers_ouverts )
	dernier = tab_noms_derniers_fichiers_ouverts[nb_derniers_fichiers_ouverts-1];
    else
	dernier = NULL;

    for ( i= nb_derniers_fichiers_ouverts - 1 ; i>0 ; i-- )
	tab_noms_derniers_fichiers_ouverts[i] = tab_noms_derniers_fichiers_ouverts[i-1];

    if ( nb_derniers_fichiers_ouverts < nb_max_derniers_fichiers_ouverts )
    {
/* xxx */
	tab_noms_derniers_fichiers_ouverts = realloc ( tab_noms_derniers_fichiers_ouverts,
						       ( ++nb_derniers_fichiers_ouverts ) * sizeof ( gpointer ));
	tab_noms_derniers_fichiers_ouverts[nb_derniers_fichiers_ouverts-1] = dernier;
    }

    tab_noms_derniers_fichiers_ouverts[0] = g_strdup ( path_fichier );


    affiche_derniers_fichiers_ouverts();
}
/* ************************************************************************************************************ */


/****************************************************************************/
void remove_file_from_last_opened_files_list ( gchar * nom_fichier )
{
    gint i, j;

    efface_derniers_fichiers_ouverts();

    for ( i = 0 ; i < nb_derniers_fichiers_ouverts ; i++ )
    {
	if ( ! strcmp (nom_fichier_comptes, tab_noms_derniers_fichiers_ouverts[i]) )
	{
	    for ( j = i; j < nb_derniers_fichiers_ouverts-1; j++ )
	    {
		tab_noms_derniers_fichiers_ouverts[j] = tab_noms_derniers_fichiers_ouverts[j+1];

	    }
	    break;
	}
    }
    nb_derniers_fichiers_ouverts--;
    affiche_derniers_fichiers_ouverts();
}
/****************************************************************************/



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
