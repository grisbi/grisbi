/* ************************************************************************** */
/* ce fichier de la gestion de l'import de fichiers (qif, ofx, csv, gnucash)  */
/*                                                                            */
/*                                  accueil.c                                 */
/*                                                                            */
/*     Copyright (C)	2000-2004 Cédric Auger (cedric@grisbi.org)	      */
/*			     2004 Benjamin Drieu (bdrieu@april.org)	      */
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
#include "import.h"
#include "devises.h"
#include "patienter.h"
#include "utils.h"
#include "utils_montants.h"
#include "utils_categories.h"
#include "operations_liste.h"
#include "comptes_gestion.h"
#include "utils_devises.h"
#include "dialog.h"
#include "utils_file_selection.h"
#include "utils_files.h"
#include "gsb_account.h"
#include "operations_comptes.h"
#include "utils_dates.h"
#include "gsb_transaction_data.h"
#include "fichiers_gestion.h"
#include "traitement_variables.h"
#include "utils_str.h"
#include "accueil.h"
#include "categories_onglet.h"
#include "tiers_onglet.h"
#include "utils_operations.h"
#include "import_csv.h"
#include "gnucash.h"
#include "html.h"
#include "ofx.h"
#include "qif.h"
#include "utils_comptes.h"
#include "imputation_budgetaire.h"
#include "utils_tiers.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void ajout_devise_dans_liste_import ( void );
static enum import_type autodetect_file_type ( FILE * fichier, gchar * pointeur_char );
static gboolean changement_valeur_echelle_recherche_date_import ( GtkWidget *spin_button );
static gboolean click_dialog_ope_orphelines ( GtkWidget *dialog,
				       gint result,
				       GtkWidget *liste_ope_celibataires );
static gboolean click_sur_liste_opes_orphelines ( GtkCellRendererToggle *renderer, 
					   gchar *ligne,
					   GtkTreeModel *store );
static void confirmation_enregistrement_ope_import ( struct struct_compte_importation *imported_account );
static GtkWidget * create_file_format_import_menu ();
static void cree_liens_virements_ope_import ( void );
static void cree_ligne_recapitulatif ( struct struct_compte_importation *compte,
				gint position );
static gboolean fichier_choisi_importation ( GtkWidget *fenetre );
static gboolean filetype_changed ( GtkOptionMenu * option_menu, gpointer user_data );
static void gsb_import_add_imported_transactions ( struct struct_compte_importation *imported_account,
					    gint account_number );
static gint gsb_import_create_imported_account ( struct struct_compte_importation *imported_account );
static gint gsb_import_create_transaction ( struct struct_ope_importation *imported_transaction,
				     gint account_number );
static void pointe_opes_importees ( struct struct_compte_importation *imported_account );
static void selection_fichiers_import ( void );
static void traitement_operations_importees ( void );
/*END_STATIC*/


/** used to keep the number of the mother transaction while importing breakdown transactions */

static gint mother_transaction_number;
gint valeur_echelle_recherche_date_import;
GSList *liste_comptes_importes;
GtkWidget *dialog_recapitulatif;
GtkWidget *table_recapitulatif;
gint virements_a_chercher;

enum import_type {
  TYPE_UNKNOWN = 0,
  TYPE_QIF,
  TYPE_OFX,
  TYPE_GNUCASH,
  TYPE_CSV,
  TYPE_HTML,
};

enum import_type file_type;

/*START_EXTERN*/
extern gchar *dernier_chemin_de_travail;
extern GtkWidget *formulaire;
extern gchar *last_date;
extern GSList *liste_struct_devises;
extern gint mise_a_jour_combofix_categ_necessaire;
extern gint mise_a_jour_combofix_tiers_necessaire;
extern gint mise_a_jour_liste_comptes_accueil;
extern gint mise_a_jour_soldes_minimaux;
extern GtkTreeStore *model;
extern GtkWidget *notebook_listes_operations;
extern GtkTreeSelection * selection;
extern GtkWidget *tree_view;
extern GtkWidget *window;
/*END_EXTERN*/



/* *******************************************************************************/
/* fonction importer_fichier */
/* appelée par le menu importer */
/* *******************************************************************************/

void importer_fichier ( void )
{
    liste_comptes_importes = NULL;
    dialog_recapitulatif = NULL;
    virements_a_chercher = 0;

    selection_fichiers_import ();
}
/* *******************************************************************************/



/* *******************************************************************************/
/* Affiche la fenêtre de sélection de fichier pour l'import */
/* *******************************************************************************/

void selection_fichiers_import ( void )
{
    GtkWidget *fenetre;

    file_type = TYPE_UNKNOWN;

    fenetre = file_selection_new ( _("Select files to import"),FILE_SELECTION_MUST_EXIST );
    gtk_window_set_transient_for ( GTK_WINDOW ( fenetre ),
				   GTK_WINDOW ( window ));
    gtk_window_set_modal ( GTK_WINDOW ( fenetre ),
			   TRUE );
    gtk_file_selection_set_select_multiple ( GTK_FILE_SELECTION ( fenetre ), TRUE );
    file_selection_set_filename ( GTK_FILE_SELECTION ( fenetre ),
				      dernier_chemin_de_travail );

    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( fenetre ) -> vbox ), 
			 create_file_format_import_menu(), FALSE, FALSE, 0 );

    gtk_signal_connect_object (GTK_OBJECT ( GTK_FILE_SELECTION( fenetre ) -> ok_button ),
			       "clicked",
			       GTK_SIGNAL_FUNC ( fichier_choisi_importation ),
			       GTK_OBJECT ( fenetre ));
    gtk_signal_connect_object (GTK_OBJECT ( GTK_FILE_SELECTION( fenetre ) -> cancel_button ),
			       "clicked",
			       GTK_SIGNAL_FUNC (gtk_widget_destroy),
			       GTK_OBJECT ( fenetre ));
    gtk_widget_show ( fenetre );


}
/* *******************************************************************************/



/* *******************************************************************************/
gboolean fichier_choisi_importation ( GtkWidget *fenetre )
{
    /* un ou plusieurs fichiers ont été sélectionnés dans la gtk_file_selection en argument */
    /* on va récupérer ces fichiers et les trier par qif/ofx/web */

    gchar **liste_selection;
    gint i;	
    gboolean result = TRUE;

    /* on sauve le répertoire courant  */

    dernier_chemin_de_travail = file_selection_get_last_directory( GTK_FILE_SELECTION ( fenetre ) , TRUE);

    /* on va récupérer tous les fichiers sélectionnés puis proposer d'en importer d'autres */


    liste_selection = file_selection_get_selections ( GTK_FILE_SELECTION ( fenetre ));
    i=0;
    gtk_widget_destroy ( fenetre );

    while ((result == TRUE)&&( liste_selection[i]))
    {
	FILE *fichier;
	gchar *pointeur_char = NULL;


	/* on ouvre maintenant le fichier pour tester sa structure */

	if ( !( fichier = utf8_fopen ( liste_selection[i], "r" )))
	{
	    /* on n'a pas réussi à ouvrir le fichier, on affiche
	       l'erreur et on retourne sur la sélection des
	       fichiers */
	    dialogue_error_hint ( latin2utf8 ( strerror(errno) ),
				  g_strdup_printf ( _("Error opening file '%s'"),
						    liste_selection[i] ) );
            return FALSE;
	}

	do
	    get_line_from_file ( fichier,
				 &pointeur_char );
	while ( strlen ( pointeur_char ) == 1 );

	if ( file_type == TYPE_UNKNOWN )
	  {
	    file_type = autodetect_file_type ( fichier, pointeur_char );
	  }

	switch ( file_type )
	  {
	  case TYPE_OFX:
	    result = recuperation_donnees_ofx ( liste_selection[i]);
	    break;

	  case TYPE_QIF:
	    result = recuperation_donnees_qif ( fichier );
	    break;

/* 	  case TYPE_HTML: */
	    /* Pour l'instant html non implémenté */
/* 	    result = recuperation_donnees_html ( fichier ); */
/* 	    break; */

	  case TYPE_GNUCASH:
	    result = recuperation_donnees_gnucash ( liste_selection[i] );
	    break;
		
		/* TODO : work in progress dude :p */
	  case TYPE_CSV: 
 	    result = recuperation_donnees_csv ( fichier ); 
 	    break;	 
		
	  case TYPE_UNKNOWN:
	  default:
	    result = FALSE;
	    break;
	  }

        /* clean up */
        if (pointeur_char) free ( pointeur_char );
        pointeur_char = NULL;
        
        /* In case of error, return to file selection dialog, else
	   import next selected file */
        if (!result) 
	  break;
	
        i++;

	fclose ( fichier );
    }

    affichage_recapitulatif_importation();

    /* Replace trees contents. */
    remplit_arbre_categ ();
    remplit_arbre_imputation ();
    remplit_arbre_tiers ();

    return ( result );
}
/* *******************************************************************************/



/* *******************************************************************************/
gboolean affichage_recapitulatif_importation ( void )
{
    /* on affiche un tableau récapitulatif des comptes importés */
    /* propose l'action à faire pour chaque compte et propose */
    /* d'en importer d'autres */

    gint retour;
    GtkWidget *label;
    GSList *list_tmp;
    GtkWidget *scrolled_window;
    GtkWidget *bouton;
    GtkWidget *hbox;

    if ( !liste_comptes_importes )
      {
	dialogue_warning_hint ( _("Grisbi is unable to find an account in imported file.  Be sure this file is valid or try with another one.\nIf you think imported file is in valid format, please contact Grisbi development team."), 
				_("No account was imported") );
	return (FALSE);
      }

    /* We have to do that as soon as possible since this would reset currencies */
    if ( !gsb_account_get_accounts_amount () )
      {
	init_variables ();
      }

    /* First, iter to see if we need to create currencies */
    list_tmp = liste_comptes_importes;
    while ( list_tmp )
      {
	struct struct_compte_importation * compte;
	compte = list_tmp -> data;

	if ( compte -> devise )
	  {
	    struct struct_devise *devise;
		    
	    /* First, we search currency from ISO4217 code for
	       existing currencies */
	    devise = devise_par_code_iso ( compte -> devise );

	    /* Then, by nickname for existing currencies */
	    if ( ! devise )
	      devise = devise_par_nom ( compte -> devise );

	    /* Last ressort, we browse ISO4217 currency list and create
	       currency if found */
	    if ( ! devise )
	      devise = find_currency_from_iso4217_list ( compte -> devise );

	  }

	list_tmp = list_tmp -> next;
      }

    if ( dialog_recapitulatif )
      {
	/*  la boite a déjà été créé, on ajoute les nouveaux comptes à la suite */

	/* on vérifie déjà s'il y a plus d'éléments dans la liste que de lignes sur le tableau */

	if ( g_slist_length ( liste_comptes_importes ) > ( GTK_TABLE ( table_recapitulatif ) -> nrows - 1 ))
	{
	    /* on démarre au nouveaux comptes */

	    list_tmp = g_slist_nth ( liste_comptes_importes,
				      GTK_TABLE ( table_recapitulatif ) -> nrows - 1 );

	    while ( list_tmp )
	      {
		cree_ligne_recapitulatif ( list_tmp -> data,
					   g_slist_position ( liste_comptes_importes,
							      list_tmp ) + 1);
		    list_tmp = list_tmp -> next;
	      }
	}
	
	gtk_widget_show ( dialog_recapitulatif );

    }
    else
    {
	/* la boite n'a pas encore été créé, on le fait */

	dialog_recapitulatif = gtk_dialog_new_with_buttons ( _("Actions on imported accounts:" ),
							     GTK_WINDOW ( window ),
							     GTK_DIALOG_MODAL,
							     _("Add more accounts"), 1,
							     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
							     GTK_STOCK_OK, GTK_RESPONSE_OK,
							     NULL );
	gtk_window_set_policy ( GTK_WINDOW ( dialog_recapitulatif ),
				TRUE,
				TRUE,
				FALSE );

	if ( gsb_account_get_accounts_amount () )
	    gtk_widget_set_usize ( dialog_recapitulatif,
				   900,
				   400 );
	else
	    gtk_widget_set_usize ( dialog_recapitulatif,
				   650,
				   400 );


	label = gtk_label_new ( _("List of the selection of accounts :"));
	gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog_recapitulatif ) -> vbox ),
			     label,
			     FALSE,
			     FALSE,
			     0 );
	gtk_widget_show ( label );

	scrolled_window = gtk_scrolled_window_new ( FALSE,
						    FALSE );
	gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					 GTK_POLICY_AUTOMATIC,
					 GTK_POLICY_AUTOMATIC );
	gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog_recapitulatif ) -> vbox ),
			     scrolled_window,
			     TRUE,
			     TRUE,
			     0 );
	gtk_widget_show ( scrolled_window );


	table_recapitulatif = gtk_table_new ( g_slist_length ( liste_comptes_importes ),
					      7,
					      FALSE );
	gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( scrolled_window ),
						table_recapitulatif );
	gtk_container_set_border_width ( GTK_CONTAINER ( table_recapitulatif ), 10 );
	gtk_table_set_col_spacings ( GTK_TABLE(table_recapitulatif), 6 );
	gtk_table_set_row_spacings ( GTK_TABLE(table_recapitulatif), 6 );
	gtk_widget_show ( table_recapitulatif );

	/* on met les titres des colonnes */

	label = gtk_label_new ( _( "Date" ));
	gtk_table_attach ( GTK_TABLE ( table_recapitulatif ),
			   label,
			   0, 1,
			   0 ,1,
			   GTK_SHRINK,
			   GTK_SHRINK,
			   0, 0 );
	gtk_widget_show ( label );

	label = gtk_label_new ( _( "Name" ));
	gtk_table_attach ( GTK_TABLE ( table_recapitulatif ),
			   label,
			   1, 2,
			   0 ,1,
			   GTK_SHRINK,
			   GTK_SHRINK,
			   0, 0 );
	gtk_widget_show ( label );

	label = gtk_label_new ( _( "Currency" ));
	gtk_table_attach ( GTK_TABLE ( table_recapitulatif ),
			   label,
			   2, 3,
			   0 ,1,
			   GTK_SHRINK,
			   GTK_SHRINK,
			   0, 0 );
	gtk_widget_show ( label );

	label = gtk_label_new ( _( "Action" ));
	gtk_table_attach ( GTK_TABLE ( table_recapitulatif ),
			   label,
			   3, 4,
			   0 ,1,
			   GTK_SHRINK,
			   GTK_SHRINK,
			   0, 0 );
	gtk_widget_show ( label );

	if ( gsb_account_get_accounts_amount () )
	{
	    label = gtk_label_new ( _( "Account" ));
	    gtk_table_attach ( GTK_TABLE ( table_recapitulatif ),
			       label,
			       4, 5,
			       0 ,1,
			       GTK_SHRINK,
			       GTK_SHRINK,
			       0, 0 );
	    gtk_widget_show ( label );
	}

	label = gtk_label_new ( _( "Type of account" ));
	gtk_table_attach ( GTK_TABLE ( table_recapitulatif ),
			   label,
			   5, 6,
			   0 ,1,
			   GTK_SHRINK,
			   GTK_SHRINK,
			   0, 0 );
	gtk_widget_show ( label );

	label = gtk_label_new ( _( "Origine" ));
	gtk_table_attach ( GTK_TABLE ( table_recapitulatif ),
			   label,
			   6, 7,
			   0 ,1,
			   GTK_SHRINK,
			   GTK_SHRINK,
			   0, 0 );
	gtk_widget_show ( label );



	/* si aucun compte n'est ouvert, on crée les devises de base */

	if ( !gsb_account_get_accounts_amount () )
	{
	    menus_sensitifs ( FALSE );
	    ajout_devise (NULL);
	}

	list_tmp = liste_comptes_importes;

	while ( list_tmp )
	{
	    cree_ligne_recapitulatif ( list_tmp -> data,
				       g_slist_position ( liste_comptes_importes,
							  list_tmp ) + 1);
	    list_tmp = list_tmp -> next;
	}

	/* on rajoute ensuite le bouton pour ajouter des devises */

	hbox = gtk_hbox_new ( FALSE,
			      0 );
	gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog_recapitulatif ) -> vbox ),
			     hbox,
			     FALSE,
			     FALSE,
			     0 );
	gtk_widget_show ( hbox );

	bouton = gtk_button_new_with_label ( _("Add a currency") );
	gtk_button_set_relief ( GTK_BUTTON ( bouton ),
				GTK_RELIEF_NONE );
	gtk_signal_connect ( GTK_OBJECT ( bouton ),
			     "clicked",
			     GTK_SIGNAL_FUNC ( ajout_devise_dans_liste_import ),
			     NULL );
	gtk_box_pack_start ( GTK_BOX ( hbox ),
			     bouton,
			     TRUE,
			     FALSE,
			     0 );
	gtk_widget_show ( bouton );
    }

    retour = gtk_dialog_run ( GTK_DIALOG ( dialog_recapitulatif ));

    switch ( retour )
    {
	case GTK_RESPONSE_OK:

	    /*  on a appuyé sur ok, il ne reste plus qu'à traiter les infos */

	    traitement_operations_importees ();
	    gtk_widget_destroy ( dialog_recapitulatif );
	    break;

	case 1:
	    /*  on a appuyé sur ajouter, on réaffiche la boite de sélection de fichier */

	    gtk_widget_hide ( dialog_recapitulatif );
	    selection_fichiers_import ();

	    break;

	default:
	    /* on annule */
	    gtk_widget_destroy ( dialog_recapitulatif );

    }
    return ( FALSE );
}
/* *******************************************************************************/

/* *******************************************************************************/
void ajout_devise_dans_liste_import ( void )
{
    /* permet d'ajouter une devise au moment de l'importation d'opérations */

    GSList *list_tmp;


    ajout_devise (NULL);


    /*     on met maintenant à jour les options menu des devise dans la liste des comptes importés */

    list_tmp = liste_comptes_importes;

    while ( list_tmp )
    {
	struct struct_compte_importation *compte;

	compte = list_tmp -> data;
	gtk_option_menu_set_menu ( GTK_OPTION_MENU ( compte -> bouton_devise ),
				   creation_option_menu_devises ( 0,
								  liste_struct_devises ));
	gtk_widget_show_all ( compte -> bouton_devise );
	list_tmp = list_tmp -> next;
    }
}
/* *******************************************************************************/


/* *******************************************************************************/
void cree_ligne_recapitulatif ( struct struct_compte_importation *compte,
				gint position )
{
    /* crée la ligne du compte en argument dans le récapitulatif à la position donnée */

    GtkWidget *label;
    GtkWidget *menu;
    GtkWidget *menu_item;
    gint no_compte_trouve;


    /* mise en place de la date si elle existe */

    if ( compte -> date_fin )
    {
	label = gtk_label_new ( g_strdup_printf ( "%02d/%02d/%d",
						  g_date_day ( compte -> date_fin ),
						  g_date_month ( compte -> date_fin ),
						  g_date_year ( compte -> date_fin )));
	gtk_table_attach ( GTK_TABLE ( table_recapitulatif ),
			   label,
			   0, 1,
			   position, position+1,
			   GTK_SHRINK,
			   GTK_SHRINK,
			   0, 0 );
	gtk_widget_show ( label );
    }
    else
    {
	label = gtk_label_new ( _("none"));
	gtk_table_attach ( GTK_TABLE ( table_recapitulatif ),
			   label,
			   0, 1,
			   position, position+1,
			   GTK_SHRINK,
			   GTK_SHRINK,
			   0, 0 );
	gtk_widget_show ( label );
    }


    /* mise en place du nom du compte s'il existe */

    if ( compte -> nom_de_compte )
    {
	label = gtk_label_new ( compte -> nom_de_compte );
	gtk_table_attach ( GTK_TABLE ( table_recapitulatif ),
			   label,
			   1, 2,
			   position, position+1,
			   GTK_SHRINK,
			   GTK_SHRINK,
			   0, 0 );
	gtk_widget_show ( label );
    }


    /* on crée le bouton de choix de devise */

    compte -> bouton_devise = gtk_option_menu_new ();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( compte -> bouton_devise ),
			       creation_option_menu_devises ( 0,
							      liste_struct_devises ));
    gtk_table_attach ( GTK_TABLE ( table_recapitulatif ),
		       compte -> bouton_devise,
		       2, 3,
		       position, position+1,
		       GTK_SHRINK,
		       GTK_SHRINK,
		       0, 0 );
    gtk_widget_show ( compte -> bouton_devise );


    if ( compte -> devise )
    {
	struct struct_devise *devise;

	/* First, we search currency from ISO4217 code for existing currencies */
	devise = devise_par_code_iso ( compte -> devise );
	/* Then, by nickname for existing currencies */
	if ( ! devise )
	  devise = devise_par_nom ( compte -> devise );

	if ( devise )
	    gtk_option_menu_set_history ( GTK_OPTION_MENU ( compte -> bouton_devise ),
					  g_slist_index ( liste_struct_devises,
							  devise ));
	else
	{
	    /* 	    la devise avait un nom mais n'a pas été retrouvée; 2 possibilités : */
	    /* 		- soit elle n'est pas crÃ©é (l'utilisateur
			  la créera une fois la fenetre affichée) */ 
	    /* 		- soit elle est créé mais pas avec le bon code */
	    dialogue_warning_hint ( g_strdup_printf ( _( "Currency of imported account '%s' is %s.  Either this currency doesn't exist so you have to create it in next window, or this currency already exists but the ISO code is wrong.\nTo avoid this message, please set its ISO code in configuration."),
						      compte -> nom_de_compte,
						      compte -> devise ),
				    g_strdup_printf ( _("Can't associate ISO 4217 code for currency '%s'."),  compte -> devise ));

	}
    }

    /* on crée les boutons de comptes et de type de compte tout de suite */
    /*   pour les (dé)sensitiver lors de changement de l'action */

    compte -> bouton_compte = gtk_option_menu_new ();
    compte -> bouton_type_compte = gtk_option_menu_new ();


    /* on crée le bouton de l'action demandée */
    /* si aucun fichier n'est ouvert, on ne propose que créer un compte */

    compte -> bouton_action = gtk_option_menu_new ();

    menu = gtk_menu_new ();

    menu_item = gtk_menu_item_new_with_label ( _("Create a new account"));
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_action",
			  GINT_TO_POINTER ( 0 ) );
    if ( gsb_account_get_accounts_amount () )
	gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				    "activate",
				    GTK_SIGNAL_FUNC ( desensitive_widget ),
				    GTK_OBJECT ( compte -> bouton_compte ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( sensitive_widget ),
				GTK_OBJECT ( compte -> bouton_type_compte ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_widget_show ( menu_item );

    if ( gsb_account_get_accounts_amount () )
    {
	menu_item = gtk_menu_item_new_with_label ( _("Add the transactions"));
	gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			      "no_action",
			      GINT_TO_POINTER ( 1 ));
	gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				    "activate",
				    GTK_SIGNAL_FUNC ( sensitive_widget ),
				    GTK_OBJECT ( compte -> bouton_compte ));
	gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				    "activate",
				    GTK_SIGNAL_FUNC ( desensitive_widget ),
				    GTK_OBJECT ( compte -> bouton_type_compte ));
	gtk_menu_append ( GTK_MENU ( menu ),
			  menu_item );
	gtk_widget_show ( menu_item );

	menu_item = gtk_menu_item_new_with_label ( _("Mark transactions"));
	gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			      "no_action",
			      GINT_TO_POINTER ( 2 ) );
	gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				    "activate",
				    GTK_SIGNAL_FUNC ( sensitive_widget ),
				    GTK_OBJECT ( compte -> bouton_compte ));
	gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				    "activate",
				    GTK_SIGNAL_FUNC ( desensitive_widget ),
				    GTK_OBJECT ( compte -> bouton_type_compte ));
	gtk_menu_append ( GTK_MENU ( menu ),
			  menu_item );
	gtk_widget_show ( menu_item );
    }

    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( compte -> bouton_action ),
			       menu );
    gtk_widget_show ( menu );
    gtk_table_attach ( GTK_TABLE ( table_recapitulatif ),
		       compte -> bouton_action,
		       3, 4,
		       position, position+1,
		       GTK_SHRINK,
		       GTK_SHRINK,
		       0, 0 );
    gtk_widget_show ( compte -> bouton_action );


    /* on crée le bouton du compte sélectionné */
    /* si aucun fichier n'est ouvert, on ne crée pas ce bouton */

    no_compte_trouve = -1;

    if ( gsb_account_get_accounts_amount () )
    {
	GSList *list_tmp;

	menu = gtk_menu_new ();

	list_tmp = gsb_account_get_list_accounts ();

	while ( list_tmp )
	{
	    gint i;

	    i = gsb_account_get_no_account ( list_tmp -> data );

	    if ( !gsb_account_get_closed_account (i))
	    {
		menu_item = gtk_menu_item_new_with_label ( gsb_account_get_name (i));
		gtk_object_set_data ( GTK_OBJECT ( menu_item ),
				      "account_number",
				      GINT_TO_POINTER (i));

		/* on recherche quel compte était noté dans le fichier  */
		/* s'il y a une id, on la prend en priorité sur le nom */

		if ( compte -> id_compte
		     &&
		     gsb_account_get_id (i)
		     &&
		     !g_strcasecmp ( compte -> id_compte,
				     gsb_account_get_id (i)))
		    no_compte_trouve = i;

		/* on ne passe par cette étape que si le compte n'a pas déjà été trouvé avec l'id */

		if ( no_compte_trouve == -1
		     &&
		     compte -> nom_de_compte
		     &&
		     !g_strcasecmp ( compte -> nom_de_compte,
				     gsb_account_get_name (i)))
		    no_compte_trouve = i;


		gtk_menu_append ( GTK_MENU ( menu ),
				  menu_item );
		gtk_widget_show ( menu_item );
	    }
	    list_tmp = list_tmp -> next;
	}

	gtk_option_menu_set_menu ( GTK_OPTION_MENU ( compte -> bouton_compte ),
				   menu );
	gtk_widget_show ( menu );

	gtk_table_attach ( GTK_TABLE ( table_recapitulatif ),
			   compte -> bouton_compte,
			   4, 5,
			   position, position+1,
			   GTK_SHRINK,
			   GTK_SHRINK,
			   0, 0 );
	gtk_widget_show ( compte -> bouton_compte );

    }

    /* on crée le bouton du type de compte  */


    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( compte -> bouton_type_compte ),
			       creation_menu_type_compte() );
    gtk_table_attach ( GTK_TABLE ( table_recapitulatif ),
		       compte -> bouton_type_compte,
		       5, 6,
		       position, position+1,
		       GTK_SHRINK,
		       GTK_SHRINK,
		       0, 0 );
    gtk_widget_show ( compte -> bouton_type_compte );

    switch ( compte -> type_de_compte )
    {
	case 3:
	    /* actif ou passif */

	    gtk_option_menu_set_history ( GTK_OPTION_MENU ( compte -> bouton_type_compte ),
					  2 );
	    break;

	case 7:
	    /* cash */

	    gtk_option_menu_set_history ( GTK_OPTION_MENU ( compte -> bouton_type_compte ),
					  1 );
	    break;
    }

    /* mise en place de l'origine */

    switch ( compte -> origine )
    {
	case QIF_IMPORT :
	    label = gtk_label_new ( _( "QIF file"));
	    break;

	case OFX_IMPORT:
	    label = gtk_label_new ( _( "OFX file"));
	    break;

/* 	case HTML_IMPORT: */
/* 	    label = gtk_label_new ( _( "HTML file")); */
/* 	    break; */

	case GNUCASH_IMPORT:
	    label = gtk_label_new ( _( "Gnucash file"));
	    break;

	default: 
	    label = gtk_label_new ( _("Unknown"));
    }

    gtk_table_attach ( GTK_TABLE ( table_recapitulatif ),
		       label,
		       6, 7,
		       position, position+1,
		       GTK_SHRINK,
		       GTK_SHRINK,
		       0, 0 );
    gtk_widget_show ( label );

    /* 	si on a trouvé un compte qui correspond, on l'affiche, et on passe le 1er option menu à ajouter les opérations */

    if ( no_compte_trouve != -1 )
    {
	gtk_option_menu_set_history ( GTK_OPTION_MENU ( compte -> bouton_compte ),
				      no_compte_trouve );
	gtk_option_menu_set_history ( GTK_OPTION_MENU ( compte -> bouton_action ),
				      1 );
	gtk_widget_set_sensitive ( compte -> bouton_type_compte,
				   FALSE );
    }
    else
	gtk_widget_set_sensitive ( compte -> bouton_compte,
				   FALSE );

}
/* *******************************************************************************/



/* *******************************************************************************/
void traitement_operations_importees ( void )
{
    /* cette fonction va faire le tour de liste_comptes_importes */
    /* et faire l'action demandée pour chaque compte importé */

    GSList *list_tmp;
    gint new_file;

    /* fait le nécessaire si aucun compte n'est ouvert */

    if ( gsb_account_get_accounts_amount () )
	new_file = 0;
    else
    {
/* 	init_variables (); */
	init_variables_new_file ();
	new_file = 1;
    }


    list_tmp = liste_comptes_importes;

    while ( list_tmp )
    {
	struct struct_compte_importation *compte;
	gint account_number;

	compte = list_tmp -> data;

	switch ( GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( compte -> bouton_action ) -> menu_item ),
							 "no_action" )))
	{
	    case 0:
		/* create */

		account_number = gsb_import_create_imported_account ( compte );

		if ( account_number != -1 )
		    gsb_import_add_imported_transactions ( compte,
							   account_number );
		else
		    dialogue_error ( "An error occured while creating the new account,\nthe import is stopped." );
		break;

	    case 1:
		/* add */

		gsb_import_add_imported_transactions ( compte,
						       recupere_no_compte ( compte -> bouton_compte ));

		break;

	    case 2:
		/* pointer */

		pointe_opes_importees ( compte );

		break;
	}
	list_tmp = list_tmp -> next;
    }

    /*     à ce niveau, il y a forcemment des comptes de créés donc si rien */
    /* 	c'est que pb, on se barre */

    if (!gsb_account_get_accounts_amount ())
	return;

    /* les différentes liste d'opérations ont été créés, on va faire le tour des opés */
    /* pour retrouver celles qui ont relation_no_compte à -2 */
    /* c'est que c'est un virement, il reste à retrouver le compte et l'opération correspondants */

    /* virements_a_chercher est à 1 si on doit chercher des relations entre opés importées */

    if ( virements_a_chercher )
	cree_liens_virements_ope_import ();


    /* création des listes d'opé */

    mise_en_route_attente ( _("Please wait") );

    if ( new_file )
    {
	init_gui_new_file ();
    }
    else
    {
	/* on fait le tour des comptes ajoutés pour leur créer une liste d'opé */
	/* 	et mettre à jour ceux qui le doivent */

	GSList *list_tmp;

	list_tmp = gsb_account_get_list_accounts ();

	while ( list_tmp )
	{
	    gint i;

	    i = gsb_account_get_no_account ( list_tmp -> data );

	    if ( !gsb_account_get_tree_view (i) )
	    {
		/*     on crée le tree_view du compte */

		creation_colonnes_tree_view_par_compte (i);

		gtk_box_pack_start ( GTK_BOX ( notebook_listes_operations ),
				     creation_tree_view_operations_par_compte (i),
				     TRUE,
				     TRUE,
				     0 );
		/* on met à jour l'option menu du formulaire des échéances */

		update_options_menus_comptes ();


		/* 	on réaffiche la liste des comptes */

		gsb_account_list_gui_create_list();
	    }
	    
	    if ( gsb_account_get_update_list(i) )
	    {
		gtk_list_store_clear ( gsb_account_get_store (i) );
		gsb_account_set_last_transaction ( i,
						   NULL );
		gsb_account_set_finished_background_color ( i,
							    0 );
		gsb_account_set_finished_balance_showed ( i,
							  0 );
		gsb_account_set_finished_selection_transaction ( i,
								 0);
		gsb_account_set_update_list ( i,
					      0 );
	    }

	    list_tmp = list_tmp -> next;
	}

	/* 	mise à jour de l'accueil */

	mise_a_jour_liste_comptes_accueil = 1;
	mise_a_jour_soldes_minimaux = 1;
	mise_a_jour_accueil ();


    }

    /* on recrée les combofix des tiers et des catégories */

    if ( mise_a_jour_combofix_tiers_necessaire )
	mise_a_jour_combofix_tiers ();
    if ( mise_a_jour_combofix_categ_necessaire )
	mise_a_jour_combofix_categ();

    /* 	on remplit ce qui est nécessaire */

    demarrage_idle ();

    annulation_attente();

    modification_fichier ( TRUE );
}
/* *******************************************************************************/

/* *******************************************************************************/
/* cette fontion recherche des opés qui sont des virements non encore reliés après un import
   dans ce cas ces opé sont marquées à -2 en relation_no_compte et info_banque_guichet contient
   le nom du compte de virement. la fonction crée donc les liens entre virements */
/* *******************************************************************************/

void cree_liens_virements_ope_import ( void )
{
    /* on fait le tour de toutes les opés des comptes */
    /* si une opé à un relation_no_compte à -2 , */
    /* on recherche le compte associé dont le nom est dans info_banque_guichet */
    /*   et une opé ayant une relation_no_compte à -2, le nom du compte dans info_banque_guichet */
    /* le même montant, le même jour avec le même tiers */

    GSList *list_accounts;

    list_accounts = gsb_account_get_list_accounts ();

    while ( list_accounts )
    {
	gint i;
	gchar *nom_compte_courant;
	GSList *list_tmp;
	gint currency;

	i = gsb_account_get_no_account ( list_accounts -> data );

	nom_compte_courant = gsb_account_get_name (i);
	list_tmp = gsb_account_get_transactions_list (i);
	currency = gsb_account_get_currency (i);

	while ( list_tmp )
	{
	    gint transaction_number;

	    transaction_number = gsb_transaction_data_get_transaction_number (list_tmp -> data);

	    /* on fait la sélection sur relation_no_compte */

	    if ( gsb_transaction_data_get_account_number_transfer (transaction_number)== -2 )
	    {
		/* recherche du compte associé */

		gint compte_trouve;
		GSList *list_tmp2;

		compte_trouve = -1;

		list_tmp2 = gsb_account_get_list_accounts ();

		while ( list_tmp2 )
		{
		    gint j;

		    j = gsb_account_get_no_account ( list_tmp2 -> data );

		    if ( !g_strcasecmp ( g_strconcat ( "[",
						       gsb_account_get_name (j),
						       "]",
						       NULL ),
					 g_strstrip ( gsb_transaction_data_get_bank_references ( transaction_number))))
			compte_trouve = j;

		    list_tmp2 = list_tmp2 -> next;
		}


		/* 		  si on n'a pas trouvé de relation avec l'autre compte, on vire les liaisons */
		/* et ça devient une opé normale sans catégorie */

		if ( compte_trouve == -1 )
		{
		    gsb_transaction_data_set_account_number_transfer ( transaction_number,
								       0);
		    gsb_transaction_data_set_transaction_number_transfer ( transaction_number,
									   0);
		    gsb_transaction_data_set_bank_references ( transaction_number,
							       NULL);
		}
		else
		{
		    /*  on a trouvé le compte opposé ; on cherche
			maintenant l'opération */
		    GSList *pointeur_tmp;
		    gboolean same_currency = FALSE;

		    if ( currency == gsb_account_get_currency (compte_trouve) )
		      same_currency = TRUE;

		    pointeur_tmp = gsb_account_get_transactions_list (compte_trouve);

		    while ( pointeur_tmp )
		    {
			gint contra_transaction_number;

			contra_transaction_number = gsb_transaction_data_get_transaction_number (pointeur_tmp -> data);

			if ( gsb_transaction_data_get_account_number_transfer ( contra_transaction_number )== -2
			     &&
			     gsb_transaction_data_get_bank_references ( contra_transaction_number )
			     &&
			     (!g_strcasecmp ( g_strconcat ("[", nom_compte_courant, "]", NULL),
 					      g_strstrip ( gsb_transaction_data_get_bank_references ( contra_transaction_number )))
			      || g_strcasecmp ( nom_compte_courant,
						g_strstrip ( gsb_transaction_data_get_bank_references ( contra_transaction_number )))) 
			     &&
			     ( !same_currency || fabs ( gsb_transaction_data_get_amount ( transaction_number)) == fabs ( gsb_transaction_data_get_amount ( contra_transaction_number )))
			     &&
			     ( gsb_transaction_data_get_party_number ( transaction_number)== gsb_transaction_data_get_party_number ( contra_transaction_number ))
			     &&
			     !g_date_compare ( gsb_transaction_data_get_date (transaction_number), gsb_transaction_data_get_date (contra_transaction_number )))
			{
			    /* la 2ème opération correspond en tout point à la 1ère, on met les relations */

			    gsb_transaction_data_set_transaction_number_transfer ( transaction_number,
										   contra_transaction_number );
			    gsb_transaction_data_set_account_number_transfer ( transaction_number,
									       gsb_transaction_data_get_account_number (contra_transaction_number ));

			    gsb_transaction_data_set_transaction_number_transfer ( contra_transaction_number ,
										   transaction_number);
			    gsb_transaction_data_set_account_number_transfer ( contra_transaction_number ,
									       gsb_transaction_data_get_account_number (transaction_number));

			    gsb_transaction_data_set_bank_references ( transaction_number,
								       NULL);
			    gsb_transaction_data_set_bank_references ( contra_transaction_number,
								       NULL);
			}
			pointeur_tmp = pointeur_tmp -> next;
		    }

		    /*   on a fait le tour de l'autre compte, si aucune contre opération n'a été trouvée, on vire les */
		    /* relations et ça devient une opé normale */

		    if ( gsb_transaction_data_get_account_number_transfer ( transaction_number)== -2 )
		    {
			gsb_transaction_data_set_account_number_transfer ( transaction_number,
									   0);
			gsb_transaction_data_set_transaction_number_transfer ( transaction_number,
									       0);
			gsb_transaction_data_set_bank_references ( transaction_number,
								   NULL);
		    }
		}
	    }
	    list_tmp = list_tmp -> next;
	}
	list_accounts = list_accounts -> next;
    }
}
/* *******************************************************************************/



/** create a new account with the datas in the imported account given in args
 * \param imported_account the account we want to create
 * \return the number of the new account
 * */
gint gsb_import_create_imported_account ( struct struct_compte_importation *imported_account )
{
    /* crée un nouveau compte contenant les données de la structure importée */
    /* ajoute ce compte aux anciens */

    gint account_number;

    /*     on crée et initialise le nouveau compte  */
    /*     le type par défaut est 0 (compte bancaire) */

    account_number = gsb_account_new( GSB_TYPE_BANK );

    /*     si ça c'est mal passé, on se barre */

    if ( account_number == -1 )
	return -1;

    /*     met l'id du compte s'il existe (import ofx) */

    if ( imported_account -> id_compte )
    {
	gchar **tab_str;

	gsb_account_set_id (account_number,
			    g_strdup ( imported_account -> id_compte ));

	/* 	en théorie cet id est "no_banque no_guichet no_comptecle" */
	/* on va essayer d'importer ces données ici */
	/* si on rencontre un null, on s'arrête */

	tab_str = g_strsplit ( gsb_account_get_id (account_number),
			       " ",
			       3 );
	if ( tab_str[1] )
	{
	    gsb_account_set_bank_branch_code ( account_number,
					       g_strdup ( tab_str[1] ) );

	    if ( tab_str[2] )
	    {
		gchar *temp;

		gsb_account_set_bank_account_key ( account_number,
						   g_strdup ( tab_str[2] + strlen ( tab_str[2] ) - 1 ) );

		temp = g_strdup ( tab_str[2] );

		temp[strlen (temp) - 1 ] = 0;
		gsb_account_set_bank_account_number ( account_number,
						      temp );
	    }
	}
	g_strfreev ( tab_str );
    }

    /* met le nom du compte */

    if ( imported_account -> nom_de_compte )
	gsb_account_set_name ( account_number,
			       g_strstrip ( imported_account -> nom_de_compte ) );
    else
	gsb_account_set_name ( account_number,
			       g_strdup ( _("Imported account")) );

    /* choix de la devise du compte */

    gsb_account_set_currency ( account_number,
			       GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( imported_account -> bouton_devise ) -> menu_item ),
								       "no_devise" )));

    /* met le type de compte si différent de 0 */

    switch ( imported_account -> type_de_compte )
    {
	case 3:
	    gsb_account_set_kind (account_number,
				  GSB_TYPE_LIABILITIES);
	    break;

	case 7:
	    gsb_account_set_kind (account_number,
				  GSB_TYPE_CASH);
	    break;
    }

    /* met le solde init */

    gsb_account_set_init_balance ( account_number,
				   imported_account -> solde);
    gsb_account_set_current_balance ( account_number, 
				      gsb_account_get_init_balance (account_number));
    gsb_account_set_marked_balance ( account_number, 
				     gsb_account_get_init_balance (account_number));

    return account_number;
}


/** import the transactions in an existent account
 * check the id of the account and if the transaction already exists
 * \param imported_account an imported structure account which contains the transactions
 * \param account_number the number of account where we want to put the new transations
 * \return
 * */
void gsb_import_add_imported_transactions ( struct struct_compte_importation *imported_account,
					    gint account_number )
{
    GSList *list_tmp;
    GDate *last_date;
    gint demande_confirmation;

    /* si le compte importé a une id, on la vérifie ici */
    /*     si elle est absente, on met celle importée */
    /*     si elle est différente, on demande si on la remplace */

    if ( imported_account -> id_compte )
    {
	if ( gsb_account_get_id (account_number) )
	{
	    if ( g_strcasecmp ( gsb_account_get_id (account_number),
				imported_account -> id_compte ))
	    {
		/* 		l'id du compte choisi et l'id du compte importé sont différents */
		/* 		    on propose encore d'arrêter... */


		if ( question_yes_no_hint ( _("The id of the imported and chosen accounts are different"),
					    _("Perhaps you choose a wrong account ?  If you choose to continue, the id of the account will be changed.  Do you want to continue ?")))
		    gsb_account_set_id (account_number,
					g_strdup ( imported_account -> id_compte ));
		else
		    return;
	    }
	}
	else
	    gsb_account_set_id (account_number,
				g_strdup ( imported_account -> id_compte ));

    }


    /* on fait un premier tour de la liste des opés pour repérer celles qui sont déjà entrées */
    /*   si on n'importe que du ofx, c'est facile, chaque opé est repérée par une id */
    /*     donc si l'opé importée a une id, il suffit de rechercher l'id dans le compte, si elle */
    /*     n'y est pas l'opé est à enregistrer */
    /*     si on importe du qif ou du html, il n'y a pas d'id. donc soit on retrouve une opé semblable */
    /*     (cad même montant et même date, on ne fait pas joujou avec le tiers car l'utilisateur */
    /* a pu le changer), et on demande à l'utilisateur quoi faire, sinon on enregistre l'opé */


    /*   pour gagner en rapidité, on va récupérer la dernière date du compte, toutes les opés importées */
    /* qui ont une date supérieure sont automatiquement acceptées */


    list_tmp = gsb_account_get_transactions_list (account_number);
    last_date = NULL;

    while ( list_tmp )
    {
	gpointer operation;

	operation = list_tmp -> data;

	if ( !last_date
	     ||
	     g_date_compare ( gsb_transaction_data_get_date (gsb_transaction_data_get_transaction_number (operation)),
			      last_date ) > 0 )
	    last_date = gsb_transaction_data_get_date (gsb_transaction_data_get_transaction_number (operation));

	list_tmp = list_tmp -> next;
    }


    list_tmp = imported_account -> operations_importees;
    demande_confirmation = 0;

    while ( list_tmp )
    {
	struct struct_ope_importation *imported_transaction;
	imported_transaction = list_tmp -> data;

	/* on ne fait le tour de la liste des opés que si la date de l'opé importée est inférieure à la dernière date */
	/* de la liste */

	if ( g_date_compare ( last_date,
			      imported_transaction -> date ) >= 0 )
	{
	    /* 	    si l'opé d'import a une id, on recherche ça en priorité */

	    if ( imported_transaction -> id_operation
		 &&
		 operation_par_id ( imported_transaction -> id_operation,
				    account_number ))
		/* comme on est sûr que cette opé a déjà été enregistree, on met l'action à 2, cad on demande l'avis de personne pour */
		    /*     pas l'enregistrer */
		    imported_transaction -> action = 2;

	    /* 	    si l'opé d'import a un no de chq, on le recherche */

	    if ( imported_transaction -> action != 2
		 &&
		 imported_transaction -> cheque
		 &&
		 operation_par_cheque ( imported_transaction -> cheque,
					account_number ))
		/* 	comme on est sûr que cette opé a déjà été enregistree, on met l'action à 2, cad on demande l'avis de personne pour */
		/*  pas l'enregistrer */
		imported_transaction -> action = 2;

	    /* on fait donc le tour de la liste des opés pour retrouver une opé comparable */
	    /* si elle n'a pas déjà été retrouvée par id... */

	    if ( imported_transaction -> action != 2 )
	    {
		GDate *date_debut_comparaison;
		GDate *date_fin_comparaison;
		GSList *liste_ope;

		date_debut_comparaison = g_date_new_dmy ( g_date_get_day ( imported_transaction -> date ),
							  g_date_get_month ( imported_transaction -> date ),
							  g_date_get_year ( imported_transaction -> date ));
		g_date_subtract_days ( date_debut_comparaison,
				       valeur_echelle_recherche_date_import );

		date_fin_comparaison = g_date_new_dmy ( g_date_get_day ( imported_transaction -> date ),
							g_date_get_month ( imported_transaction -> date ),
							g_date_get_year ( imported_transaction -> date ));
		g_date_add_days ( date_fin_comparaison,
				  valeur_echelle_recherche_date_import );

		liste_ope = gsb_account_get_transactions_list (account_number);

		while ( liste_ope )
		{
		    gint transaction_number;

		    transaction_number = gsb_transaction_data_get_transaction_number (liste_ope -> data);

		    if ( fabs ( gsb_transaction_data_get_amount (transaction_number)- imported_transaction -> montant ) < 0.01
			 &&
			 ( g_date_compare ( gsb_transaction_data_get_date (transaction_number),
					    date_debut_comparaison ) >= 0 )
			 &&
			 ( g_date_compare ( gsb_transaction_data_get_date (transaction_number),
					    date_fin_comparaison ) <= 0 )

			 &&
			 !imported_transaction -> ope_de_ventilation )
		    {
			/* l'opé a la même date et le même montant, on la marque pour demander quoi faire à l'utilisateur */
			imported_transaction -> action = 1; 
			imported_transaction -> ope_correspondante = gsb_transaction_data_get_pointer_to_transaction (transaction_number);
			demande_confirmation = 1;
		    }
		    liste_ope = liste_ope -> next;
		}
	    }
	}
	list_tmp = list_tmp -> next;
    }

    /*   à ce niveau, toutes les opés douteuses ont été marquées, on appelle la fonction qui */
    /* se charge de demander à l'utilisateur que faire */

    if ( demande_confirmation )
	confirmation_enregistrement_ope_import ( imported_account );


    /* on fait le tour des opés de ce compte et enregistre les opés */

    /* la variable derniere_operation est utilisée pour garder le numéro de l'opé */
    /* précédente pour les ventilations */

    mother_transaction_number = 0;

    list_tmp = imported_account -> operations_importees;

    while ( list_tmp )
    {
	struct struct_ope_importation *imported_transaction;

	imported_transaction = list_tmp -> data;

	/* vérifie qu'on doit bien l'enregistrer */

	if ( !imported_transaction -> action )
	{
	    /* on récupère à ce niveau la devise choisie dans la liste */

	    imported_transaction -> devise = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( imported_account -> bouton_devise ) -> menu_item ),
										 "no_devise" ));
	    gsb_import_create_transaction ( imported_transaction,
					    account_number );
	} 
	list_tmp = list_tmp -> next;
    }


    gsb_account_set_update_list ( account_number,
				  1 );
/*     calcule_solde_compte ( account_number ); */
/*     calcule_solde_pointe_compte ( account_number ); */

}
/* *******************************************************************************/


/* *******************************************************************************/
void confirmation_enregistrement_ope_import ( struct struct_compte_importation *imported_account )
{
    /*   cette fonction fait le tour des opérations importées, et demande que faire pour celles */
    /* qui sont douteuses lors d'un ajout des opés à un compte existant */

    GSList *list_tmp;
    GtkWidget *dialog;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *scrolled_window;
    GtkWidget *label;
    gint action_derniere_ventilation;


    dialog = gtk_dialog_new_with_buttons ( _("Confirmation of importation of transactions"),
					   GTK_WINDOW ( window),
					   GTK_DIALOG_MODAL,
					   GTK_STOCK_OK,
					   GTK_RESPONSE_OK,
					   NULL );
    gtk_widget_set_usize ( dialog,
			   FALSE,
			   300 );

    label = gtk_label_new ( _("Some imported transactions seem to be already saved. Please select the transactions to import." ));
    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog )-> vbox ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    scrolled_window = gtk_scrolled_window_new ( FALSE,
						FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog )-> vbox ),
			 scrolled_window,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( scrolled_window );

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					    vbox );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox ),
				     10 );
    gtk_widget_show ( vbox );

    /*   on fait maintenant le tour des opés importées et affichent celles à problème */

    list_tmp = imported_account -> operations_importees;

    while ( list_tmp )
    {
	struct struct_ope_importation *ope_import;

	ope_import = list_tmp -> data;

	/* on n'affiche pas si c'est des opés de ventil, si la mère est cochée, les filles seront alors cochées */
	/* on ne teste pas ici car ça a été testé avant */

	if ( ope_import -> action == 1 
	     &&
	     !ope_import -> ope_de_ventilation )
	{
	    gpointer operation;
	    gchar *tiers;

	    operation = ope_import -> ope_correspondante;

	    hbox = gtk_hbox_new ( FALSE,
				  5 );
	    gtk_box_pack_start ( GTK_BOX ( vbox ),
				 hbox,
				 FALSE,
				 FALSE,
				 0 );
	    gtk_widget_show ( hbox );


	    ope_import -> bouton = gtk_check_button_new ();
	    gtk_box_pack_start ( GTK_BOX ( hbox ),
				 ope_import -> bouton,
				 FALSE,
				 FALSE,
				 0 );
	    gtk_widget_show ( ope_import -> bouton );

	    label = gtk_label_new ( g_strdup_printf ( _("Transactions to import : %02d/%02d/%04d ; %s ; %4.2f"),
						      g_date_day ( ope_import -> date ),
						      g_date_month ( ope_import -> date ),
						      g_date_year ( ope_import -> date ),
						      ope_import -> tiers,
						      ope_import -> montant ));
	    gtk_box_pack_start ( GTK_BOX ( hbox ),
				 label,
				 FALSE,
				 FALSE,
				 0 );
	    gtk_widget_show ( label );


	    hbox = gtk_hbox_new ( FALSE,
				  5 );
	    gtk_box_pack_start ( GTK_BOX ( vbox ),
				 hbox,
				 FALSE,
				 FALSE,
				 0 );
	    gtk_widget_show ( hbox );


	    label = gtk_label_new ( "       " );
	    gtk_box_pack_start ( GTK_BOX ( hbox ),
				 label,
				 FALSE,
				 FALSE,
				 0 );
	    gtk_widget_show ( label );

	    tiers = tiers_name_by_no ( gsb_transaction_data_get_party_number ( gsb_transaction_data_get_transaction_number (operation )), FALSE );

	    if ( gsb_transaction_data_get_notes ( gsb_transaction_data_get_transaction_number (operation)))
		label = gtk_label_new ( g_strdup_printf ( _("Transaction found : %02d/%02d/%04d ; %s ; %4.2f ; %s"),
							  g_date_day ( gsb_transaction_data_get_date (gsb_transaction_data_get_transaction_number (operation))),
							  g_date_month ( gsb_transaction_data_get_date (gsb_transaction_data_get_transaction_number (operation))),
							  g_date_year ( gsb_transaction_data_get_date (gsb_transaction_data_get_transaction_number (operation))),
							  tiers,
							  gsb_transaction_data_get_amount ( gsb_transaction_data_get_transaction_number (operation )),
							  gsb_transaction_data_get_notes ( gsb_transaction_data_get_transaction_number (operation ))));
	    else
		label = gtk_label_new ( g_strdup_printf ( _("Transaction found : %02d/%02d/%04d ; %s ; %4.2f"),
							  g_date_day ( gsb_transaction_data_get_date (gsb_transaction_data_get_transaction_number (operation))),
							  g_date_month ( gsb_transaction_data_get_date (gsb_transaction_data_get_transaction_number (operation))),
							  g_date_year ( gsb_transaction_data_get_date (gsb_transaction_data_get_transaction_number (operation))),
							  tiers,
							  gsb_transaction_data_get_amount ( gsb_transaction_data_get_transaction_number (operation ))));

	    gtk_box_pack_start ( GTK_BOX ( hbox ),
				 label,
				 FALSE,
				 FALSE,
				 0 );
	    gtk_widget_show ( label );
	}
	list_tmp = list_tmp -> next;
    }

    gtk_dialog_run ( GTK_DIALOG ( dialog ));

    /* on fait maintenant le tour des check buttons pour voir ce qu'on importe */

    list_tmp = imported_account -> operations_importees;
    action_derniere_ventilation = 1;

    while ( list_tmp )
    {
	struct struct_ope_importation *ope_import;

	ope_import = list_tmp -> data;

	/* si c'est une opé de ventil, elle n'était pas affichée, dans ce cas si l'action de la
	   dernière ventil était 0, on fait de même pour les filles */

	if ( ope_import -> ope_de_ventilation )
	{
	    if ( ope_import -> action )
		ope_import -> action = action_derniere_ventilation;
	}
	else
	    action_derniere_ventilation = 1;



	if ( ope_import -> bouton
	     &&
	     gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( ope_import -> bouton )))
	{
	    ope_import -> action = 0;

	    /* si c'était une ventil on met l'action de la dernière ventil à 0 */

	    if ( ope_import -> operation_ventilee )
		action_derniere_ventilation = 0;	
	}

	list_tmp = list_tmp -> next;
    }

    gtk_widget_destroy ( dialog );
}
/* *******************************************************************************/


/** get an imported transaction structure in arg and create the corresponding transaction
 * \param imported_transaction the transaction to import
 * \param account_number the account where to put the new transaction
 * \return the number of the new transaction
 * */
gint gsb_import_create_transaction ( struct struct_ope_importation *imported_transaction,
				     gint account_number )
{
    gchar **tab_str;
    gint transaction_number;

    /* we create the new transaction */

    transaction_number = gsb_transaction_data_new_transaction ( account_number );

    /* récupération de l'id de l'opé s'il existe */

    if ( imported_transaction -> id_operation )
	gsb_transaction_data_set_transaction_id ( transaction_number,
						  g_strdup ( imported_transaction -> id_operation ));

    /* récupération de la date */

    gsb_transaction_data_set_date ( transaction_number,
				    gsb_date_copy ( imported_transaction -> date ));

    /* récupération de la date de valeur */

    gsb_transaction_data_set_value_date ( transaction_number,
					  gsb_date_copy ( imported_transaction -> date_de_valeur ));

    /* récupération du montant */

    gsb_transaction_data_set_amount ( transaction_number,
				      imported_transaction -> montant );

    /* 	  récupération de la devise */

    gsb_transaction_data_set_currency_number ( transaction_number,
					       imported_transaction -> devise );

    /* rÃ©cupération du tiers */

    if ( imported_transaction -> tiers 
	 &&
	 strlen ( g_strstrip ( imported_transaction -> tiers )))
	gsb_transaction_data_set_party_number ( transaction_number,
						tiers_par_nom ( imported_transaction -> tiers,
								1 ) -> no_tiers );

    /* vérification si c'est ventilé, sinon récupération des catégories */

    if ( imported_transaction -> operation_ventilee )
    {
	/* l'opération est ventilée */

	gsb_transaction_data_set_breakdown_of_transaction ( transaction_number,
							    1 );
    }
    else
    {
	/* vérification que ce n'est pas un virement */

	if ( imported_transaction -> categ
	     &&
	     strlen ( g_strstrip (imported_transaction -> categ)) )
	{
	    if ( imported_transaction -> categ[0] == '[' )
	    {
		/* 		      c'est un virement, or le compte n'a peut être pas encore été créé, */
		/* on va mettre le nom du compte dans info_banque_guichet qui n'est jamais utilisé */
		/* lors d'import, et relation_no_compte sera mis à -2 (-1 est déjà utilisé pour les comptes supprimés */

		gsb_transaction_data_set_bank_references ( transaction_number,
							   imported_transaction -> categ);
		gsb_transaction_data_set_account_number_transfer ( transaction_number,
								   -2);
		gsb_transaction_data_set_transaction_number_transfer ( transaction_number,
								       -1);
		virements_a_chercher = 1;
	    }
	    else
	    {
		struct struct_categ *categ;

		tab_str = g_strsplit ( imported_transaction -> categ,
				       ":",
				       2 );


		/* récupération ou création de la catégorie */

		categ = categ_par_nom ( tab_str[0],
					1,
					imported_transaction -> montant < 0,
					0 );

		if ( categ )
		{
		    struct struct_sous_categ *sous_categ ;

		    gsb_transaction_data_set_category_number ( transaction_number,
							       categ -> no_categ );

		    /* récupération ou création de la sous-catégorie */

		    sous_categ = sous_categ_par_nom ( categ,
						      g_strstrip ( tab_str[1]),
						      1 );
		    if ( sous_categ )
			gsb_transaction_data_set_sub_category_number ( transaction_number,
								       sous_categ -> no_sous_categ);
		}
		g_strfreev ( tab_str );
	    }
	}
    }

    /* récupération des notes */

    gsb_transaction_data_set_notes ( transaction_number,
				     imported_transaction -> notes );

    /* récupération du chèque et mise en forme du type d'opération */

    if ( imported_transaction -> cheque )
    {
	/* c'est un chèque, on va rechercher un type à incrémentation automatique et mettre l'opé sous ce type */
	/* si un tel type n'existe pas, on met quand même le no dans contenu_type et on met le type par défaut */

	struct struct_type_ope *type_choisi;
	GSList *list_tmp;

	if ( gsb_transaction_data_get_amount (transaction_number)< 0 )
	    gsb_transaction_data_set_method_of_payment_number ( transaction_number,
								gsb_account_get_default_debit (account_number));
	else
	    gsb_transaction_data_set_method_of_payment_number ( transaction_number,
								gsb_account_get_default_credit (account_number));

	gsb_transaction_data_set_method_of_payment_content ( transaction_number,
							     itoa ( imported_transaction -> cheque ) );

	list_tmp = gsb_account_get_method_payment_list (account_number);
	type_choisi = NULL;

	while ( list_tmp )
	{
	    struct struct_type_ope *type;

	    type = list_tmp -> data;

	    /* si l'opé on trouve un type à incrémentation automatique et que le signe du type est bon, on l'enregistre */
	    /*   et on arrête la recherche, sinon, on l'enregistre mais on continue la recherche dans l'espoir de trouver */
	    /* mieux */

	    if ( type -> numerotation_auto )
	    {
		if ( !type -> signe_type
		     ||
		     ( type -> signe_type == 1 && gsb_transaction_data_get_amount (transaction_number)< 0 )
		     ||
		     ( type -> signe_type == 2 && gsb_transaction_data_get_amount (transaction_number)> 0 ))
		{
		    gsb_transaction_data_set_method_of_payment_number ( transaction_number,
									type -> no_type );
		    type_choisi = type;
		    list_tmp = NULL;
		}
		else
		{
		    gsb_transaction_data_set_method_of_payment_number ( transaction_number,
									type -> no_type );
		    type_choisi = type;
		    list_tmp = list_tmp -> next;
		}
	    }
	    else
		list_tmp = list_tmp -> next;
	}

	/* type_choisi contient l'adr du type qui a été utilisé, on peut y mettre le dernier no de chèque */

	if ( type_choisi )
	    type_choisi -> no_en_cours = MAX ( imported_transaction -> cheque,
					       type_choisi -> no_en_cours );
    }
    else
    {
	/* comme ce n'est pas un chèque, on met sur le type par défaut */

	if ( gsb_transaction_data_get_amount (transaction_number)< 0 )
	    gsb_transaction_data_set_method_of_payment_number ( transaction_number,
								gsb_account_get_default_debit (account_number));
	else
	    gsb_transaction_data_set_method_of_payment_number ( transaction_number,
								gsb_account_get_default_credit (account_number));

    }

    /* récupération du pointé */

    gsb_transaction_data_set_marked_transaction ( transaction_number,
						  imported_transaction -> p_r );

    /* si c'est une ope de ventilation, lui ajoute le no de l'opération précédente */

    if ( imported_transaction -> ope_de_ventilation )
	gsb_transaction_data_set_mother_transaction_number ( transaction_number,
							     mother_transaction_number );
    else
	mother_transaction_number  = transaction_number;

    gsb_account_set_update_list ( account_number,
				  1 );

    return (transaction_number);
}
/* *******************************************************************************/



/* *******************************************************************************/
void pointe_opes_importees ( struct struct_compte_importation *imported_account )
{
    GSList *list_tmp;
    GSList *liste_opes_import_celibataires;
    gint account_number;


    /* on se place sur le compte dans lequel on va pointer les opés */

    account_number = recupere_no_compte ( imported_account -> bouton_compte );

    /* si le compte importé a une id, on la vérifie ici */
    /*     si elle est absente, on met celle importée */
    /*     si elle est différente, on demande si on la remplace */

    if ( imported_account -> id_compte )
    {
	if ( gsb_account_get_id (account_number) )
	{
	    if ( g_strcasecmp ( gsb_account_get_id (account_number),
				imported_account -> id_compte ))
	    {
		/* 		l'id du compte choisi et l'id du compte importé sont différents */
		/* 		    on propose encore d'arrêter... */


		if ( question_yes_no_hint ( _("The id of the imported and chosen accounts are different"),
					    _("Perhaps you choose a wrong account ?  If you choose to continue, the id of the account will be changed.  Do you want to continue ?")))
		    gsb_account_set_id (account_number,
					g_strdup ( imported_account -> id_compte ));
		else
		    return;
	    }
	}
	else
	    gsb_account_set_id (account_number,
				g_strdup ( imported_account -> id_compte ));

    }

    /* on fait le tour des opés importées et recherche dans la liste d'opé s'il y a la correspondance */


    list_tmp = imported_account -> operations_importees;
    liste_opes_import_celibataires = NULL;

    while ( list_tmp )
    {
	GSList *liste_ope;
	GSList *ope_trouvees;
	struct struct_ope_importation *ope_import;
	gpointer operation;
	gint i;
	struct struct_ope_importation *autre_ope_import;

	ope_import = list_tmp -> data;
	ope_trouvees = NULL;
	operation = NULL;

	/* si l'opé d'import a une id, on recherche dans la liste d'opé pour trouver
	   une id comparable */

	if ( ope_import -> id_operation )
	{
	    gpointer ope;

	    ope = operation_par_id ( ope_import -> id_operation,
				     account_number );

	    if ( ope )
		ope_trouvees = g_slist_append ( ope_trouvees,
						ope );
	}

	/* si on n'a rien trouvé par id, */
	/* on fait le tour de la liste d'opés pour trouver des opés comparable */
	/* cad même date avec + ou - une échelle et même montant et pas une opé de ventil */


	if ( !ope_trouvees )
	{
	    GDate *date_debut_comparaison;
	    GDate *date_fin_comparaison;

	    date_debut_comparaison = g_date_new_dmy ( g_date_get_day ( ope_import -> date ),
						      g_date_get_month ( ope_import -> date ),
						      g_date_get_year ( ope_import -> date ));
	    g_date_subtract_days ( date_debut_comparaison,
				   valeur_echelle_recherche_date_import );

	    date_fin_comparaison = g_date_new_dmy ( g_date_get_day ( ope_import -> date ),
						    g_date_get_month ( ope_import -> date ),
						    g_date_get_year ( ope_import -> date ));
	    g_date_add_days ( date_fin_comparaison,
			      valeur_echelle_recherche_date_import );



	    liste_ope = gsb_account_get_transactions_list (account_number);

	    while ( liste_ope )
	    {
		operation = liste_ope -> data;

		if ( fabs ( gsb_transaction_data_get_amount ( gsb_transaction_data_get_transaction_number (operation ))- ope_import -> montant ) < 0.01
		     &&
		     ( g_date_compare ( gsb_transaction_data_get_date (gsb_transaction_data_get_transaction_number (operation)),
					date_debut_comparaison ) >= 0 )
		     &&
		     ( g_date_compare ( gsb_transaction_data_get_date (gsb_transaction_data_get_transaction_number (operation)),
					date_fin_comparaison ) <= 0 )

		     &&
		     !gsb_transaction_data_get_mother_transaction_number ( gsb_transaction_data_get_transaction_number (operation )))
		    /* on a retouvé une opé de même date et même montant, on l'ajoute à la liste des opés trouvées */
		    ope_trouvees = g_slist_append ( ope_trouvees,
						    operation );

		liste_ope = liste_ope -> next;
	    }
	}
	/*       à ce stade, ope_trouvees contient la ou les opés qui sont comparables à l'opé importée */
	/* soit il n'y en n'a qu'une, et on la pointe, soit il y en a plusieurs, et on recherche dans */
	/* 	les opés importées s'il y en a d'autre comparables, et on pointe les opés en fonction */
	/* du nb de celles importées */

	switch ( g_slist_length ( ope_trouvees ))
	{
	    case 0:
		/* aucune opé comparable n'a été retrouvée */
		/* on marque donc cette opé comme seule */
		/* sauf si c'est une opé de ventil  */

		if ( !ope_import -> ope_de_ventilation ) 
		{
		    /* on met le no de compte et la devise de l'opération si plus tard on l'enregistre */

		    ope_import -> no_compte = account_number;
		    ope_import -> devise = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( imported_account -> bouton_devise ) -> menu_item ),
										   "no_devise" ));
		    liste_opes_import_celibataires = g_slist_append ( liste_opes_import_celibataires,
								      ope_import );
		}

		break;

	    case 1:
		/*  il n'y a qu'une opé retrouvée, on la pointe */
		/* si elle est déjà pointée ou relevée, on ne fait rien */
		/* si l'opé d'import a une id et pas l'opé, on marque l'id dans l'opé */

		operation = ope_trouvees -> data;

		if ( !gsb_transaction_data_get_transaction_id ( gsb_transaction_data_get_transaction_number (operation))
		     &&
		     ope_import -> id_operation )
		    gsb_transaction_data_set_transaction_id ( gsb_transaction_data_get_transaction_number (operation),
							      ope_import -> id_operation );

		if ( !gsb_transaction_data_get_marked_transaction ( gsb_transaction_data_get_transaction_number (operation )))
		{
		    gsb_transaction_data_set_marked_transaction ( gsb_transaction_data_get_transaction_number (operation ),
								  2 );
		    gsb_account_set_update_list ( account_number,
						  1 );

		    /* si c'est une opé ventilée, on recherche les opé filles pour leur mettre le même pointage que la mère */

		    if ( gsb_transaction_data_get_breakdown_of_transaction ( gsb_transaction_data_get_transaction_number (operation )))
		    {

			liste_ope = gsb_account_get_transactions_list (account_number);

			while ( liste_ope )
			{
			    gpointer ope_fille;

			    ope_fille = liste_ope -> data;

			    if ( gsb_transaction_data_get_mother_transaction_number ( gsb_transaction_data_get_transaction_number (ope_fille ))== gsb_transaction_data_get_transaction_number (operation))
				gsb_transaction_data_set_marked_transaction ( gsb_transaction_data_get_transaction_number (ope_fille),
									      2 );

			    liste_ope = liste_ope -> next;
			}
		    }
		}
		break;

	    default: 	   
		/* il y a plusieurs opé trouvées correspondant à l'opé importée */

		/* on va voir s'il y a d'autres opées importées ayant la même date et le même montant
		   si on retrouve autant d'opé importées que d'opé trouvées, on peut marquer cette
		   opé sans s'en préoccuper */


		i=0;
		liste_ope = imported_account -> operations_importees;

		while ( liste_ope )
		{
		    GDate *date_debut_comparaison;
		    GDate *date_fin_comparaison;

		    autre_ope_import = liste_ope -> data;
		    date_debut_comparaison = g_date_new_dmy ( g_date_get_day ( autre_ope_import -> date ),
							      g_date_get_month ( autre_ope_import -> date ),
							      g_date_get_year ( autre_ope_import -> date ));
		    g_date_subtract_days ( date_debut_comparaison,
					   valeur_echelle_recherche_date_import );

		    date_fin_comparaison = g_date_new_dmy ( g_date_get_day ( autre_ope_import -> date ),
							    g_date_get_month ( autre_ope_import -> date ),
							    g_date_get_year ( autre_ope_import -> date ));
		    g_date_add_days ( date_fin_comparaison,
				      valeur_echelle_recherche_date_import );


		    if ( fabs ( autre_ope_import -> montant - ope_import -> montant ) < 0.01
			 &&
			 ( g_date_compare ( gsb_transaction_data_get_date (gsb_transaction_data_get_transaction_number (operation)),
					    date_debut_comparaison ) >= 0 )
			 &&
			 ( g_date_compare ( gsb_transaction_data_get_date (gsb_transaction_data_get_transaction_number (operation)),
					    date_fin_comparaison ) <= 0 )

			 &&
			 !autre_ope_import -> ope_de_ventilation )
			/* on a retouvé une opé d'import de même date et même montant, on incrémente le nb d'opé d'import semblables trouvees */
			i++;

		    liste_ope = liste_ope -> next;
		}

		if ( i ==  g_slist_length ( ope_trouvees ))
		{
		    /* on a trouvé autant d'opé d'import semblables que d'opés semblables dans la liste d'opé
		       donc on peut marquer les opés trouvées */
		    /* pour celles qui sont déjà pointées, on ne fait rien */
		    /* si l'opé importée à une id, on met cette id dans l'opération si elle n'en a pas */

		    GSList *list_tmp_2;

		    list_tmp_2 = ope_trouvees;

		    while ( list_tmp_2 )
		    {
			operation = list_tmp_2 -> data;

			if ( !gsb_transaction_data_get_transaction_id ( gsb_transaction_data_get_transaction_number (operation))
			     &&
			     ope_import -> id_operation )
			    gsb_transaction_data_set_transaction_id ( gsb_transaction_data_get_transaction_number (operation),
								      ope_import -> id_operation );

			if ( !gsb_transaction_data_get_marked_transaction ( gsb_transaction_data_get_transaction_number (operation )))
			{
			    gsb_transaction_data_set_marked_transaction ( gsb_transaction_data_get_transaction_number (operation ),
									  2 );
			    gsb_account_set_update_list ( account_number,
							  1 );

			    /* si c'est une opé ventilée, on recherche les opé filles pour leur mettre le même pointage que la mère */

			    if ( gsb_transaction_data_get_breakdown_of_transaction ( gsb_transaction_data_get_transaction_number (operation )))
			    {
				liste_ope = gsb_account_get_transactions_list (account_number);

				while ( liste_ope )
				{
				    gpointer ope_fille;

				    ope_fille = liste_ope -> data;

				    if ( gsb_transaction_data_get_mother_transaction_number ( gsb_transaction_data_get_transaction_number (ope_fille ))== gsb_transaction_data_get_transaction_number (operation))
					gsb_transaction_data_set_marked_transaction ( gsb_transaction_data_get_transaction_number (ope_fille),
										      2 );

				    liste_ope = liste_ope -> next;
				}
			    }
			}
			list_tmp_2 = list_tmp_2 -> next;
		    }
		}
		else
		{
		    /* on a trouvé un nombre différent d'opés d'import et d'opés semblables dans la liste d'opés
		       on marque donc cette opé d'import comme seule */

		    ope_import -> no_compte = account_number;
		    ope_import -> devise = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( imported_account -> bouton_devise ) -> menu_item ),
										   "no_devise" ));
		    liste_opes_import_celibataires = g_slist_append ( liste_opes_import_celibataires,
								      ope_import );

		}
	}
	list_tmp = list_tmp -> next;
    }

    /* a ce niveau, liste_opes_import_celibataires contient les opés d'import dont on n'a pas retrouvé
       l'opé correspondante
       on les affiche dans une liste en proposant de les ajouter à la liste */

    if ( liste_opes_import_celibataires )
    {
	GtkWidget *liste_ope_celibataires, *dialog, *label, *scrolled_window;
	GtkListStore *store;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;



	dialog = gtk_dialog_new_with_buttons ( _("Orphaned transactions"),
					       GTK_WINDOW ( window ),
					       GTK_DIALOG_DESTROY_WITH_PARENT,
					       GTK_STOCK_APPLY, 1,
					       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					       GTK_STOCK_OK, GTK_RESPONSE_OK,
					       NULL );

	label = gtk_label_new ( _("Mark transactions you want to add to the list and click the add button"));
	gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ),
			     label,
			     FALSE,
			     FALSE,
			     0 );
	gtk_widget_show ( label );

	store = gtk_list_store_new ( 4,
				     G_TYPE_BOOLEAN,
				     G_TYPE_STRING,
				     G_TYPE_STRING,
				     G_TYPE_DOUBLE );

	/* on remplit la liste */

	list_tmp = liste_opes_import_celibataires;

	while ( list_tmp ) 
	{
	    struct struct_ope_importation *ope_import;
	    GtkTreeIter iter;

	    ope_import = list_tmp -> data;

	    gtk_list_store_append ( store,
				    &iter );

	    gtk_list_store_set ( store,
				 &iter,
				 0, FALSE,
				 1, g_strdup_printf ( "%02d/%02d/%04d",
						      g_date_get_day ( ope_import -> date ),
						      g_date_get_month ( ope_import -> date ),
						      g_date_get_year ( ope_import -> date )),
				 2, ope_import -> tiers,
				 3, ope_import -> montant,
				 -1 );

	    list_tmp = list_tmp -> next;
	}

	/* on crée la liste des opés célibataires
	   et on y associe la gslist */

	liste_ope_celibataires = gtk_tree_view_new_with_model ( GTK_TREE_MODEL (store));
	g_object_set_data ( G_OBJECT ( liste_ope_celibataires ),
			    "liste_ope",
			    liste_opes_import_celibataires );
	scrolled_window = gtk_scrolled_window_new ( FALSE, FALSE );
	gtk_widget_set_usize ( scrolled_window, FALSE, 300 );
	gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ),
			     scrolled_window,
			     TRUE,
			     TRUE,
			     0 );
	gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					 GTK_POLICY_AUTOMATIC,
					 GTK_POLICY_AUTOMATIC );
	gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( scrolled_window ),
						liste_ope_celibataires );
	gtk_widget_show_all ( scrolled_window );

	/* on affiche les colonnes */

	renderer = gtk_cell_renderer_toggle_new ();
	g_signal_connect ( renderer,
			   "toggled",
			   G_CALLBACK (click_sur_liste_opes_orphelines ),
			   store );
	column = gtk_tree_view_column_new_with_attributes ( _("Mark"),
							    renderer,
							    "active", 0,

							    NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (liste_ope_celibataires), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ( _("Date"),
							    renderer,
							    "text", 1,
							    NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (liste_ope_celibataires), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ( _("Third party"),
							    renderer,
							    "text", 2,
							    NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (liste_ope_celibataires), column);


	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ( _("Amount"),
							    renderer,
							    "text", 3,
							    NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (liste_ope_celibataires), column);

	g_signal_connect ( G_OBJECT ( dialog ),
			   "response",
			   G_CALLBACK ( click_dialog_ope_orphelines ),
			   liste_ope_celibataires );

	gtk_widget_show ( dialog );
    }
}
/* *******************************************************************************/


/* *******************************************************************************/
gboolean click_dialog_ope_orphelines ( GtkWidget *dialog,
				       gint result,
				       GtkWidget *liste_ope_celibataires )
{
    GSList *liste_opes_import_celibataires;
    GSList *list_tmp;
    GtkTreeIter iter;
    GtkTreeModel *model;

    switch ( result )
    {
	case 1:
	case GTK_RESPONSE_OK:
	    /* on ajoute la ou les opés marquées à la liste d'opés en les pointant d'un T
	       puis on les retire de la liste des orphelines
	       s'il ne reste plus d'opés orphelines, on ferme la boite de dialogue */

	    liste_opes_import_celibataires = g_object_get_data ( G_OBJECT ( liste_ope_celibataires ),
								 "liste_ope" );
	    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( liste_ope_celibataires ));
	    gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( model ),
					    &iter );

	    list_tmp = liste_opes_import_celibataires;

	    /* normalement, pas besoin de mettre ça à 0 car normalement pas de ventilations à ce stade... */

	    mother_transaction_number = 0;

	    while ( list_tmp )
	    {
		gboolean enregistre;
		GSList *last_item;

		gtk_tree_model_get ( GTK_TREE_MODEL ( model ),
				     &iter,
				     0, &enregistre,
				     -1 );

		if ( enregistre )
		{
		    /* à ce niveau, l'opé a été cochée donc on l'enregistre en la marquant T	 */

		    struct struct_ope_importation *ope_import;
		    gint transaction_number;

		    ope_import = list_tmp -> data;

		    transaction_number = gsb_import_create_transaction ( ope_import,
									 ope_import -> no_compte );
		    gsb_transaction_data_set_marked_transaction ( transaction_number,
								  2 );

		    /* on a enregistré l'opé, on la retire maintenant de la liste et de la sliste */

		    last_item = list_tmp;
		    list_tmp = list_tmp -> next;
		    liste_opes_import_celibataires = g_slist_remove_link ( liste_opes_import_celibataires,
									   last_item );

		    /* on retire la ligne qu'on vient d'enregistrer, celà met l'iter directement sur la suite */

		    gtk_list_store_remove ( GTK_LIST_STORE ( model),
					    &iter );
		}
		else
		{
		    gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ),
					       &iter );
		    list_tmp = list_tmp -> next;
		}
	    }

	    /* on enregistre la nouvelle liste d'opé pour la retrouver plus tard */

	    g_object_set_data ( G_OBJECT ( liste_ope_celibataires ),
				"liste_ope",
				liste_opes_import_celibataires );

	    /* il est possible que les opés importées soient des virements, il faut faire les
	       relations ici */
	    if ( virements_a_chercher )
		cree_liens_virements_ope_import ();

	    /* on recrée les combofix des tiers et des catégories */

	    if ( mise_a_jour_combofix_tiers_necessaire )
		mise_a_jour_combofix_tiers ();
	    if ( mise_a_jour_combofix_categ_necessaire )
		mise_a_jour_combofix_categ();

	    /* mise à jour de l'accueil */

	    mise_a_jour_liste_comptes_accueil = 1;
	    mise_a_jour_soldes_minimaux = 1;

	    modification_fichier ( TRUE );

	    if ( result != GTK_RESPONSE_OK
		 &&
		 g_slist_length ( liste_opes_import_celibataires ))
		break;

	default:
	    gtk_widget_destroy ( dialog );
	    break;
    }

    return ( FALSE );
}
/* *******************************************************************************/


/* *******************************************************************************/
gboolean click_sur_liste_opes_orphelines ( GtkCellRendererToggle *renderer, 
					   gchar *ligne,
					   GtkTreeModel *store )
{
    GtkTreeIter iter;

    if ( gtk_tree_model_get_iter_from_string ( GTK_TREE_MODEL ( store ),
					       &iter,
					       ligne ))
    {
	gboolean valeur;

	gtk_tree_model_get ( GTK_TREE_MODEL ( store ),
			     &iter,
			     0, &valeur,
			     -1 );
	gtk_list_store_set ( GTK_LIST_STORE ( store ),
			     &iter,
			     0, 1 - valeur,
			     -1 );
    }
    return ( FALSE );
}		
/* *******************************************************************************/



/* *******************************************************************************/
/* page de configuration pour l'importation */
/* *******************************************************************************/
GtkWidget *onglet_importation (void)
{
    GtkWidget *vbox_pref;
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *bouton;

    vbox_pref = new_vbox_with_title_and_icon ( _("Import"),
					       "files.png" ); 
    hbox = gtk_hbox_new ( FALSE,
			  0 );
    gtk_box_pack_start ( GTK_BOX ( vbox_pref ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    label = gtk_label_new ( _("Search the transaction "));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    bouton = gtk_spin_button_new_with_range ( 0.0,
					      100.0,
					      1.0);
    gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( bouton ),
				valeur_echelle_recherche_date_import );
    g_signal_connect ( G_OBJECT ( bouton ),
		       "value-changed",
		       G_CALLBACK ( changement_valeur_echelle_recherche_date_import ),
		       NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    label = gtk_label_new ( _("days around the date in the imported transaction."));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    if ( ! assert_account_loaded() )
      gtk_widget_set_sensitive ( vbox_pref, FALSE );

    return ( vbox_pref );
}
/* *******************************************************************************/


/* *******************************************************************************/
gboolean changement_valeur_echelle_recherche_date_import ( GtkWidget *spin_button )
{
    valeur_echelle_recherche_date_import = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON ( spin_button ));
    modification_fichier ( TRUE );
    return ( FALSE );
}
/* *******************************************************************************/


GtkWidget * create_file_format_import_menu ()
{
  GtkWidget *hbox, *omenu, *menu, *menu_item;

  hbox = gtk_hbox_new ( FALSE, 0 );

  menu = gtk_menu_new ();

  menu_item = gtk_menu_item_new_with_label ( _("Autodetect") );
  g_object_set_data ( G_OBJECT ( menu_item ), "file", (gpointer) TYPE_UNKNOWN );
  gtk_menu_append ( GTK_MENU ( menu ), menu_item );

  menu_item = gtk_menu_item_new_with_label ( _("QIF file") );
  g_object_set_data ( G_OBJECT ( menu_item ), "file", (gpointer) TYPE_QIF );
  gtk_menu_append ( GTK_MENU ( menu ), menu_item );

  menu_item = gtk_menu_item_new_with_label ( _("OFX file") );
  g_object_set_data ( G_OBJECT ( menu_item ), "file", (gpointer) TYPE_OFX );
  gtk_menu_append ( GTK_MENU ( menu ), menu_item );

  menu_item = gtk_menu_item_new_with_label ( _("Gnucash file") );
  g_object_set_data ( G_OBJECT ( menu_item ), "file", (gpointer) TYPE_GNUCASH );
  gtk_menu_append ( GTK_MENU ( menu ), menu_item );

/* TODO: add it again */
/*   menu_item = gtk_menu_item_new_with_label ( _("CSV file") ); */
/*   g_object_set_data ( G_OBJECT ( menu_item ), "file", (gpointer) TYPE_CSV ); */
/*   gtk_menu_append ( GTK_MENU ( menu ), menu_item ); */

  omenu = gtk_option_menu_new ();
  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( omenu ), menu );
  g_signal_connect ( G_OBJECT(omenu), "changed", G_CALLBACK (filetype_changed), NULL );
  gtk_box_pack_end ( GTK_BOX(hbox), omenu, TRUE, TRUE, 6 );
  gtk_box_pack_end ( GTK_BOX(hbox), gtk_label_new (COLON(_("Format de fichier"))),
		     FALSE, FALSE, 0 );

  gtk_widget_show_all ( hbox );

  return hbox;
}



gboolean filetype_changed ( GtkOptionMenu * option_menu, gpointer user_data )
{
  file_type = gtk_option_menu_get_history ( option_menu );
  return FALSE;
}



enum import_type autodetect_file_type ( FILE * fichier, gchar * pointeur_char )
{
  enum import_type type;

/* TODO: add it again */
   if ( g_strrstr ( pointeur_char,  
 		   "Date;Type;" )) /* c'est un fichier csv */ 
     { 
       type = TYPE_CSV; 
     } 
   else  
   if ( g_strrstr ( pointeur_char,
			"ofx" )
	    ||
	    g_strrstr ( pointeur_char,
			"OFX" ))
    {
      type = TYPE_OFX;
    }
   else if ( !my_strncasecmp ( pointeur_char, "!Type", 5 ) ||
	     !my_strncasecmp ( pointeur_char, "!Account", 8 ) || 
	     !my_strncasecmp ( pointeur_char, "!Option", 7 ))
   {
       type = TYPE_QIF;
   }
  else
    {
      if ( !strncmp ( pointeur_char, "<?xml", 5 ))
	{
	  get_line_from_file ( fichier, &pointeur_char );
	  if ( !strncmp ( pointeur_char, "<gnc-v2", 7 ))
	    {
	      type = TYPE_GNUCASH;
	    }
	  else
	    {
	      type = TYPE_UNKNOWN;
	    }
	}
      else {
	  /* Do not implement HTML at the moment */
/* 	if ( pointeur_char[0] == '<' ) */
/* 	  { */
/* 	    type = TYPE_HTML; */
/* 	  } */
/* 	else */
/* 	  { */
	    type = TYPE_UNKNOWN;
/* 	  } */
      }
    }

  return type;
}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
