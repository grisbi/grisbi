
/* ************************************************************************** */
/*                                                                            */
/*     copyright (c)	2000-2006 C�dric Auger (cedric@grisbi.org)	      */
/*			2004-2006 Benjamin Drieu (bdrieu@april.org) 	      */
/*			http://www.grisbi.org   			      */
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

/**
 * \file gsb_form_config.c
 * all that you need for the config of the form is here !!!
 */


#include "include.h"

/*START_INCLUDE*/
#include "gsb_form_config.h"
#include "utils.h"
#include "comptes_traitements.h"
#include "dialog.h"
#include "gsb_data_account.h"
#include "gsb_data_form.h"
#include "utils_str.h"
#include "operations_comptes.h"
#include "traitement_variables.h"
#include "utils_buttons.h"
#include "utils_comptes.h"
#include "operations_formulaire.h"
#include "erreur.h"
#include "include.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean ajoute_colonne_organisation_formulaire ( void );
static gboolean ajoute_ligne_organisation_formulaire ( void );
static gboolean allocation_liste_organisation_formulaire ( GtkWidget *tree_view,
						    GtkAllocation *allocation );
static gboolean change_taille_colonne_organisation_formulaire ( GtkWidget *tree_view,
							 GdkEventMotion *motion );
static GtkWidget *creation_liste_organisation_formulaire ( void );
static gboolean gsb_form_config_check_for_removing ( gint account_number,
					      gint removing_row );
static gboolean gsb_form_config_drag_begin ( GtkWidget *tree_view,
				      GdkDragContext *drag_context,
				      gpointer null );
static gboolean gsb_form_config_drag_end ( GtkWidget *tree_view,
				      GdkDragContext *drag_context,
				      gpointer null );
static gboolean gsb_form_config_fill_store ( gint account_number );
static void mise_a_jour_organisation_formulaire ( gint account_number );
static gboolean modification_compte_choix_formulaire ( GtkWidget *menu_item );
static gboolean modification_formulaire_distinct_par_compte ( void );
static gchar *recherche_nom_element_formulaire ( gint no_element );
static gboolean retire_colonne_organisation_formulaire ( void );
static gboolean retire_ligne_organisation_formulaire ( void );
static gboolean toggled_signal_configuration_formulaire ( GtkWidget *toggle_button );
/*END_STATIC*/

/*START_EXTERN*/
extern     gchar * buffer ;
extern GtkWidget *preview;
extern GtkTooltips *tooltips_general_grisbi;
extern GtkWidget *tree_view;
extern GtkWidget *window;
/*END_EXTERN*/


/* utilisé pour éviter que ça s'emballe lors du réglage de la largeur des colonnes du formulaire */

gint allocation_precedente_organisation_formulaire;

/* titre des colonnes de la liste de configuration du formulaire */

gchar *nom_colonne[6] = {
    N_("Col 1"),
    N_("Col 2"),
    N_("Col 3"),
    N_("Col 4"),
    N_("Col 5"),
    N_("Col 6")
};

/* utilisé pour la configuration du formulaire */

gint start_drag_column;
gint start_drag_row;

/* les adr des toggles button du contenu du formulaire */

GtkWidget *boutons_organisation_formulaire[TRANSACTION_FORM_WIDGET_NB-3];

/* l'option menu des comptes pour le choix d'un formulaire par compte */
/* ou le même pour tous les comptes */

GtkWidget *option_menu_comptes_choix_formulaire;

/*     rempli lors de l'affichage des paramètres */

GtkWidget *tree_view_organisation_formulaire;
GtkTreeViewColumn *tree_view_column_organisation_formulaire[6];

GtkWidget *formulaire;



/**
 * create and return the page to be set in the configuration page
 *
 * \param
 *
 * \return the complete page
 * */
GtkWidget *onglet_display_transaction_form ( void )
{
    GtkWidget *vbox_pref;
    GtkWidget *liste_organisation;


    vbox_pref = new_vbox_with_title_and_icon ( _("Transaction form"), "form.png" );

    /*     organisation du formulaire */

    liste_organisation = creation_liste_organisation_formulaire ();
    gtk_box_pack_start ( GTK_BOX ( vbox_pref ),
			 liste_organisation,
			 FALSE, FALSE, 0 );
    gtk_widget_show ( liste_organisation );

    

    if ( !gsb_data_account_get_accounts_amount () )
    {
	gtk_widget_set_sensitive ( vbox_pref, FALSE );
    }

    return vbox_pref;
}


/* *************************************************************************** */
/* cette fonction crée une liste de 6 colonnes sur 4 lignes utilisée pour */
/* organiser le formulaire d'opérations */
/* *************************************************************************** */
GtkWidget *creation_liste_organisation_formulaire ( void )
{
    GtkWidget *fenetre;
    GtkListStore *store;
    gint row, column;
    GtkWidget *hbox, *hbox2;
    GtkWidget *bouton;
    GtkWidget *label;
    GtkWidget *paddingbox;
    GtkWidget *table;
    gint no_element_en_cours;
    GtkWidget *scrolled_window;
    GtkTargetEntry target_entry[] =
    {
	{ "text", GTK_TARGET_SAME_WIDGET, 0 }
    };


    /*     le fenetre sera une vbox avec la liste en haut et les boutons +- col et lignes */
    /* 	en dessous, et ce qu'on veut afficher en dessous */

    fenetre = gtk_vbox_new ( FALSE,
			     5 );
    gtk_widget_show ( fenetre );

    
    /*     mise en place de la boite qui contiendra la liste du formulaire */

    paddingbox = new_paddingbox_with_title ( fenetre,
					     FALSE,
					     _("Form structure preview"));

    /*     on propose de séparer l'affichage par compte */
    /* 	soit chaque compte a son formulaire */
    /* 	soit chaque compte utilise le formulaire du compte 0 */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    /* mise en place du bouton de choix */

    bouton = new_checkbox_with_title ( _("Each account has his own form"),
				       &etat.formulaire_distinct_par_compte,
				       G_CALLBACK ( modification_formulaire_distinct_par_compte ));
    gtk_box_pack_start ( GTK_BOX (hbox ),
			 bouton,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    /*     mise en place de l'option menu des comptes */

    option_menu_comptes_choix_formulaire =  gtk_option_menu_new ();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( option_menu_comptes_choix_formulaire ),
			       creation_option_menu_comptes ( (GtkSignalFunc) modification_compte_choix_formulaire ,
							      TRUE,
							      FALSE ));
    gtk_box_pack_start ( GTK_BOX (hbox ),
			 option_menu_comptes_choix_formulaire,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( option_menu_comptes_choix_formulaire );

    gtk_widget_set_sensitive ( option_menu_comptes_choix_formulaire,
			       etat.formulaire_distinct_par_compte );

    /*     mise en place du scrolled window */
    /* 	il est utilisé pour éviter l'agrandissement de la fenetre de conf */
    /* 	si l'utilisateur change de trop la largeur des colonnes */
    
    scrolled_window = gtk_scrolled_window_new ( FALSE,
						FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_ALWAYS,
				     GTK_POLICY_NEVER );
    gtk_box_pack_start ( GTK_BOX ( paddingbox),
			 scrolled_window,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( scrolled_window );

    /*     création du store */

    store = gtk_list_store_new ( MAX_WIDTH,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING );

    /*     on crée maintenant le tree_view */

    tree_view_organisation_formulaire = gtk_tree_view_new_with_model ( GTK_TREE_MODEL ( store ));
    gtk_tree_selection_set_mode ( GTK_TREE_SELECTION ( gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view_organisation_formulaire))),
				  GTK_SELECTION_NONE );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window),
			tree_view_organisation_formulaire );
    gtk_widget_show ( tree_view_organisation_formulaire );

    /*     on crée les colonnes */

    for ( column=0 ; column< MAX_WIDTH ; column++ )
    {
	tree_view_column_organisation_formulaire[column] = gtk_tree_view_column_new_with_attributes ( nom_colonne[column],
												      gtk_cell_renderer_text_new (),
												      "text", column,
												      NULL );
	gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view_organisation_formulaire ),
				      GTK_TREE_VIEW_COLUMN ( tree_view_column_organisation_formulaire[column]));
	gtk_tree_view_column_set_sizing ( GTK_TREE_VIEW_COLUMN (tree_view_column_organisation_formulaire[column]),
					  GTK_TREE_VIEW_COLUMN_AUTOSIZE );
	gtk_tree_view_column_set_expand ( GTK_TREE_VIEW_COLUMN (tree_view_column_organisation_formulaire[column]),
					  TRUE );
	gtk_tree_view_column_set_resizable ( GTK_TREE_VIEW_COLUMN (tree_view_column_organisation_formulaire[column]),
					     TRUE );
    }


    /*     on met en place le size allocate pour régler la largeur des colonnes en fonction de la */
    /* 	taille de la liste */

    g_signal_connect ( G_OBJECT ( tree_view_organisation_formulaire ),
		       "size-allocate",
		       G_CALLBACK ( allocation_liste_organisation_formulaire ),
		       NULL );

    /*     pour le changement de taille des colonnes, pas trouvé mieux... */

    g_signal_connect ( G_OBJECT (tree_view_organisation_formulaire),
		       "motion-notify-event",
		       G_CALLBACK ( change_taille_colonne_organisation_formulaire ),
		       NULL );

    gtk_drag_source_set ( tree_view_organisation_formulaire,
			  GDK_BUTTON1_MASK,
			  target_entry, 1,
			  GDK_ACTION_MOVE );
    g_signal_connect ( G_OBJECT (tree_view_organisation_formulaire),
		       "drag-begin",
		       G_CALLBACK ( gsb_form_config_drag_begin ),
		       NULL );
    gtk_drag_dest_set ( tree_view_organisation_formulaire,
			GTK_DEST_DEFAULT_ALL,
			target_entry, 1,
			GDK_ACTION_MOVE );
    g_signal_connect ( G_OBJECT (tree_view_organisation_formulaire),
		       "drag-end",
		       G_CALLBACK ( gsb_form_config_drag_end ),
		       NULL );


    /*     mise en place des boutons d'agrandissement ou de rétrécissement */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
			 hbox,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( hbox );

    /*     mise en place du choix du nb de lignes */

    hbox2 = gtk_hbox_new ( FALSE,
			   5 );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 hbox2,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox2 );

    bouton = gtk_button_new_from_stock ( GTK_STOCK_REMOVE );
    g_signal_connect ( G_OBJECT ( bouton ),
		       "clicked",
		       G_CALLBACK ( retire_ligne_organisation_formulaire ),
		       NULL );
    gtk_button_set_relief ( GTK_BUTTON( bouton ),
			    GTK_RELIEF_NONE );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ),
			 bouton,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    label = gtk_label_new ( _("rows"));
    gtk_box_pack_start ( GTK_BOX ( hbox2 ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    bouton = gtk_button_new_from_stock ( GTK_STOCK_ADD );
    g_signal_connect ( G_OBJECT ( bouton ),
		       "clicked",
		       G_CALLBACK ( ajoute_ligne_organisation_formulaire ),
		       NULL );
    gtk_button_set_relief ( GTK_BUTTON( bouton ),
			    GTK_RELIEF_NONE );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ),
			 bouton,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    /*     mise en place des boutons pour l'ajout de colonnes */

    hbox2 = gtk_hbox_new ( FALSE,
			   5 );
    gtk_box_pack_end ( GTK_BOX ( hbox ),
		       hbox2,
		       FALSE,
		       FALSE,
		       0 );
    gtk_widget_show ( hbox2 );

    bouton = gtk_button_new_from_stock ( GTK_STOCK_REMOVE );
    g_signal_connect ( G_OBJECT ( bouton ),
		       "clicked",
		       G_CALLBACK ( retire_colonne_organisation_formulaire ),
		       NULL );
    gtk_button_set_relief ( GTK_BUTTON( bouton ),
			    GTK_RELIEF_NONE );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ),
			 bouton,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    label = gtk_label_new ( _("columns"));
    gtk_box_pack_start ( GTK_BOX ( hbox2 ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    bouton = gtk_button_new_from_stock ( GTK_STOCK_ADD );
    g_signal_connect ( G_OBJECT ( bouton ),
		       "clicked",
		       G_CALLBACK ( ajoute_colonne_organisation_formulaire ),
		       NULL );
    gtk_button_set_relief ( GTK_BUTTON( bouton ),
			    GTK_RELIEF_NONE );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ),
			 bouton,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );
    

    /*     mise en place de la boite qui contiendra les composants du formulaire */

    paddingbox = new_paddingbox_with_title ( fenetre,
					     FALSE,
					     _("Form structure content"));

    /*     création du tooltips s'il ne l'est pas déjà */

    if ( !tooltips_general_grisbi )
	tooltips_general_grisbi = gtk_tooltips_new ();

    /* on crée maintenant une table de 3x6 boutons */
    table = gtk_table_new ( 3, 6, FALSE );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), table,
			 FALSE, FALSE, 0 );

    /*     on commence après la date, le débit et le crédit qui sont obligatoires */

    no_element_en_cours = 4;

    for ( row=0 ; row<3 ; row++ )
	for ( column=0 ; column<6 ; column++ )
	{
	    gchar *string;
	    gchar *string_modifie;

	    string = recherche_nom_element_formulaire (no_element_en_cours);

	    if ( string )
	    {
		/*on va couper cette chaine si elle est plus grande que 10 caractères */
		
		string_modifie = limit_string ( string,
						10 );

		boutons_organisation_formulaire[column + row*6] = gtk_toggle_button_new_with_label ( string_modifie );
		g_object_set_data ( G_OBJECT ( boutons_organisation_formulaire[column + row*6] ),
				    "no_element",
				    GINT_TO_POINTER ( no_element_en_cours));

		g_signal_connect ( G_OBJECT ( boutons_organisation_formulaire[column + row*6] ),
				   "toggled",
				   G_CALLBACK (toggled_signal_configuration_formulaire),
				   NULL );
		gtk_table_attach_defaults ( GTK_TABLE ( table ),
					    boutons_organisation_formulaire[column + row*6],
					    column, column+1,
					    row, row+1 );
		
		/* 		mise en place du tooltips */

		gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
				       boutons_organisation_formulaire[column + row*6],
				       string,
				       string );
	    }

	    no_element_en_cours++;
	}

    allocation_precedente_organisation_formulaire = 0;

    /*     tout est créé, on rempli maintenant avec le compte 0 */

    mise_a_jour_organisation_formulaire ( 0 );

    return (fenetre); 
}
/* *************************************************************************** */


/* *************************************************************************** */
/* cette fonction remplit la liste de l'organisation du formulaire et rend */
/* actif les boutons nécessaires pour le compte donné en argument */
/* *************************************************************************** */
void mise_a_jour_organisation_formulaire ( gint account_number )
{
    gint row;
    gint column;
    gint no_element_en_cours;
    
    /*     on commence par remplir le store */

    gsb_form_config_fill_store (account_number);

    /*     on rend visible les colonnes nécessaires */

    for ( row=0 ; row<MAX_WIDTH ; row++ )
	if ( row<gsb_data_form_get_nb_columns (account_number))
	    gtk_tree_view_column_set_visible ( GTK_TREE_VIEW_COLUMN ( tree_view_column_organisation_formulaire[row] ),
					       TRUE );
	else
    	    gtk_tree_view_column_set_visible ( GTK_TREE_VIEW_COLUMN ( tree_view_column_organisation_formulaire[row] ),
					       FALSE );

    /*     on rend in/actif les boutons nécessaires */

    /*     on commence après la date, le débit et le crédit qui sont obligatoires */

    no_element_en_cours = 4;

    for ( row=0 ; row<3 ; row++ )
	for ( column=0 ; column<6 ; column++ )
	{
	    if ( column + row*6 < TRANSACTION_FORM_WIDGET_NB - 3 &&
		 boutons_organisation_formulaire[column + row*6] )
	    {
		g_signal_handlers_block_by_func ( G_OBJECT ( boutons_organisation_formulaire[column + row*6] ),
						  G_CALLBACK ( toggled_signal_configuration_formulaire ),
						  NULL );
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( boutons_organisation_formulaire[column + row*6] ),
					       gsb_data_form_look_for_value ( account_number,
									      no_element_en_cours,
									      NULL, NULL ));
		g_signal_handlers_unblock_by_func ( G_OBJECT ( boutons_organisation_formulaire[column + row*6] ),
						    G_CALLBACK ( toggled_signal_configuration_formulaire ),
						    NULL );
	    }
	    no_element_en_cours++;
	}
}
/* *************************************************************************** */



/* *************************************************************************** */
/* cette fonction est appelée lorsqu'on change le bouton du choix du formulaire */
/* différent par compte. (dé)grise l'option menu des comptes et le remet sur le premier */
/* compte si on le grise */
/* *************************************************************************** */
gboolean modification_formulaire_distinct_par_compte ( void )
{
    if ( etat.formulaire_distinct_par_compte )
	gtk_widget_set_sensitive ( option_menu_comptes_choix_formulaire,
				   TRUE );
    else
    {
	gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_comptes_choix_formulaire ),
				      0 );
	modification_compte_choix_formulaire ( GTK_OPTION_MENU ( option_menu_comptes_choix_formulaire ) -> menu_item );
	gtk_widget_set_sensitive ( option_menu_comptes_choix_formulaire,
				   FALSE );
    }

    return FALSE;
}
/* *************************************************************************** */

						       
/* *************************************************************************** */
/* cette fonction est appelée si on change le compte affiché par l'option menu */
/* *************************************************************************** */
gboolean modification_compte_choix_formulaire ( GtkWidget *menu_item )
{
    gint account_number;

    account_number = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( menu_item ),
							"account_number" ));
    mise_a_jour_organisation_formulaire ( account_number );


    return FALSE;
}
/* *************************************************************************** */




/* *************************************************************************** */
/* cette fonction est appelée lorsqu'on clique sur l'un des boutons de l'organisation */
/* du formulaire, elle ajoute ou retire l'élément de la liste */
/* *************************************************************************** */
gboolean toggled_signal_configuration_formulaire ( GtkWidget *toggle_button )
{
    gint no_element;
    gint no_second_element;
    gint i, j;
    gint account_number;

    /*     on commence par rechercher le no de l'élément */

    no_element = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( toggle_button ),
						       "no_element" ));
    /*     on met ici le 2nd elt si nécessaire */

    switch ( no_element )
    {
	case TRANSACTION_FORM_TYPE:
	    /* 	    c'est le mode de paiement, on met le chq */
	    no_second_element = TRANSACTION_FORM_CHEQUE;
	    break;

	case TRANSACTION_FORM_CHEQUE:
	    /* 	    c'est le chq, on met mode de paiement */
	    no_second_element = TRANSACTION_FORM_TYPE;
	    break;

	case TRANSACTION_FORM_DEVISE:
	    /* 	    c'est la devise, on met le bouton de change */
	    no_second_element = TRANSACTION_FORM_CHANGE;
	    break;

	case TRANSACTION_FORM_CHANGE:
	    /* 	    c'est le bouton de change, on met la devise */
	    no_second_element = TRANSACTION_FORM_DEVISE;
	    break;

	default:
	    no_second_element = -1;
    }

    account_number = recupere_no_compte ( option_menu_comptes_choix_formulaire );

    /*     on met à jour le tableau */

    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( toggle_button )))
    {
	/* 	on l'a enclenché, on rajoute l'élément */

	gint place_trouvee = 0;
	gint ligne_premier_elt = -1;
	gint colonne_premier_elt = -1;

	for ( i=0 ; i < gsb_data_form_get_nb_rows (account_number) ; i++)
	    for ( j=0 ; j < gsb_data_form_get_nb_columns (account_number) ; j++ )
		if ( !gsb_data_form_get_value (account_number,
					       j,
					       i ))
		{
		    /* 		    s'il n'y a qu'un elt, on le met et on termine, sinon on continue à chercher */
		    /* 			pour le 2ème */

		    if ( no_second_element == -1 )
		    {
			/* 			il n'y a qu'un elt */
			
			gsb_data_form_set_value ( account_number,
						  j,
						  i,
						  no_element );
			place_trouvee = 1;
			i = gsb_data_form_get_nb_rows (account_number);
			j = gsb_data_form_get_nb_columns (account_number);
		    }
		    else
		    {
			/* 			il y a 2 elts */

			if ( ligne_premier_elt == -1 )
			{
			    /* 			    on vient de trouver la place pour le 1er */
			    ligne_premier_elt = i;
			    colonne_premier_elt = j;
			}
			else
			{
			    /* 			    on vient de trouver la place pour le 2ème */

			    gsb_data_form_set_value ( account_number,
						      colonne_premier_elt,
						      ligne_premier_elt,
						      no_element );
			    gsb_data_form_set_value ( account_number,
						      j,
						      i,
						      no_second_element );
			    place_trouvee = 1;
			    i = gsb_data_form_get_nb_rows (account_number);
			    j = gsb_data_form_get_nb_columns (account_number);
			}
		    }
		}

	if ( place_trouvee )
	{
	    /* 	    on a trouvé une place pour le/les élements */
	    /* 		s'il y a plusieurs éléments en jeu, il faut le rendre actif aussi */

	    if ( no_second_element != -1 )
	    {
		g_signal_handlers_block_by_func ( G_OBJECT ( boutons_organisation_formulaire[no_second_element-4] ),
						  G_CALLBACK ( toggled_signal_configuration_formulaire ),
						  NULL );
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( boutons_organisation_formulaire[no_second_element-4] ),
					       TRUE );
		g_signal_handlers_unblock_by_func ( G_OBJECT ( boutons_organisation_formulaire[no_second_element-4] ),
						    G_CALLBACK ( toggled_signal_configuration_formulaire ),
						    NULL );
	    }
	}
	else
	{
	    /* 	    le tableau est trop petit pour rajouter un élément */
	
	    g_signal_handlers_block_by_func ( G_OBJECT ( toggle_button ),
					      G_CALLBACK ( toggled_signal_configuration_formulaire ),
					      NULL );
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( toggle_button ),
					   FALSE );
	    g_signal_handlers_unblock_by_func ( G_OBJECT ( toggle_button ),
						G_CALLBACK ( toggled_signal_configuration_formulaire ),
						NULL );

	    if ( no_second_element == -1 )
		dialogue_hint ( _("There is no place enough to put the element. You need to increase the number of rows or columns to add an element."),
				_("The table is full"));
	    else
		dialogue_hint ( _("There is no place enough to put the two elements (you have clicked on an element which contains two). You need to increase the number of rows or columns to add the elements."),
				_("The table is full"));

	    return TRUE;
	}
    }
    else
    {
	/* 	on a désenclenché le bouton */

	if ( no_second_element != -1 )
	{
	    g_signal_handlers_block_by_func ( G_OBJECT ( boutons_organisation_formulaire[no_second_element-4] ),
					      G_CALLBACK ( toggled_signal_configuration_formulaire ),
					      NULL );
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( boutons_organisation_formulaire[no_second_element-4] ),
					   FALSE );
	    g_signal_handlers_unblock_by_func ( G_OBJECT ( boutons_organisation_formulaire[no_second_element-4] ),
						G_CALLBACK ( toggled_signal_configuration_formulaire ),
						NULL );
	}

	for ( i=0 ; i < gsb_data_form_get_nb_rows (account_number) ; i++ )
	    for ( j=0 ; j < gsb_data_form_get_nb_columns (account_number) ; j++ )
		if ( gsb_data_form_get_value (account_number,
					      j,
					      i ) == no_element )
		{
		    gsb_data_form_set_value ( account_number,
					      j,
					      i,
					      0 );
		    if ( no_second_element == -1 )
		    {
			i = gsb_data_form_get_nb_rows (account_number);
			j = gsb_data_form_get_nb_columns (account_number);
		    }
		    else
		    {
			no_element = no_second_element;
			no_second_element = -1;
			i = 0;
			j = 0;
		    }
		}
    }
    /*     on réaffiche la liste */

    gsb_form_config_fill_store (account_number);

    modification_fichier (TRUE);
    return FALSE;

}
/* *************************************************************************** */


/**
 * fill the configuration store according to the organization for the account given
 *
 * \param account_number
 *
 * \return FALSE
 * */
gboolean gsb_form_config_fill_store ( gint account_number )
{
    gint row;
    GtkListStore *store;

    store = GTK_LIST_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_organisation_formulaire )));
						
    gtk_list_store_clear ( store );

    for ( row=0 ; row < gsb_data_form_get_nb_rows (account_number) - 1 ; row++ )
    {
	GtkTreeIter iter;
	gint column;

	gtk_list_store_append ( GTK_LIST_STORE ( store ),
				&iter );

	for ( column = 0 ; column < gsb_data_form_get_nb_columns (account_number) ; column++ )
	{
	    gtk_list_store_set ( GTK_LIST_STORE ( store ),
				 &iter,
				 column, recherche_nom_element_formulaire (gsb_data_form_get_value ( account_number,
												     column,
												     row )),
				 -1 );
	}
    }

    /* update the form if necessary */

    if ( !etat.formulaire_distinct_par_compte
	 ||
	 account_number == gsb_data_account_get_current_account () )
	remplissage_formulaire (account_number);

    return FALSE;
}




/* *************************************************************************** */
/* cette fonction prend un numéro d'élément du formulaire et renvoie son texte */
/* retourne NULL si pas trouvé */
/* *************************************************************************** */
gchar *recherche_nom_element_formulaire ( gint no_element )
{
    switch ( no_element )
    {
	case -1:
	    /* that value shouldn't be there, it shows that a gsb_data_form_... returns
	     * an error value */
	    warning_debug ( "recherche_nom_element_formulaire : a value in the form is -1 wich should not happen.\nA gsb_data_form_... function must have returned an error value..." );
	    return ("");
	    break;

	case TRANSACTION_FORM_DATE:
	    return (N_("Date"));
	    break;

	case TRANSACTION_FORM_DEBIT:
	    return (N_("Debit"));
	    break;

	case TRANSACTION_FORM_CREDIT:
	    return (N_("Credit"));
	    break;

	case TRANSACTION_FORM_VALUE_DATE:
	    return (N_("Value date"));
	    break;

	case TRANSACTION_FORM_EXERCICE:
	    return (N_("Financial year"));
	    break;

	case TRANSACTION_FORM_PARTY:
	    return (N_("Payee"));
	    break;

	case TRANSACTION_FORM_CATEGORY:
	    return (N_("Categories"));
	    break;

	case TRANSACTION_FORM_FREE:
	    return (N_("Free"));
	    break;

	case TRANSACTION_FORM_BUDGET:
	    return (N_("Budgetary line"));
	    break;

	case TRANSACTION_FORM_NOTES:
	    return (N_("Notes"));
	    break;

	case TRANSACTION_FORM_TYPE:
	    return (N_("Method of payment"));
	    break;

	case TRANSACTION_FORM_CHEQUE:
	    return (N_("Cheque/Transfer number"));
	    break;

	case TRANSACTION_FORM_DEVISE:
	    return (N_("Currency"));
	    break;

	case TRANSACTION_FORM_CHANGE:
	    return (N_("Change"));
	    break;

	case TRANSACTION_FORM_VOUCHER:
	    return (N_("Voucher"));
	    break;

	case TRANSACTION_FORM_BANK:
	    return (N_("Bank references"));
	    break;

	case TRANSACTION_FORM_CONTRA:
	    return (N_("Contra-transaction method of payment"));
	    break;

	case TRANSACTION_FORM_OP_NB:
	    return (N_("Transaction number"));
	    break;

	case TRANSACTION_FORM_MODE:
	    return (N_("Automatic/Manual"));
	    break;
    }

    return NULL;
}
/* *************************************************************************** */


/* *************************************************************************** */
/* fonction appelée à l'affichage de la liste d'organisation du formulaire */
/* pour adapter la taille des colonnes à celle demandée */
/* *************************************************************************** */
gboolean allocation_liste_organisation_formulaire ( GtkWidget *tree_view,
						    GtkAllocation *allocation )
{
    gint column;
    gint account_number;

    if ( ! assert_account_loaded() )
      return FALSE;

    if ( allocation_precedente_organisation_formulaire == allocation -> width )
	return FALSE;

    allocation_precedente_organisation_formulaire= allocation -> width;

    account_number = recupere_no_compte ( option_menu_comptes_choix_formulaire );

    for ( column=0 ; column < gsb_data_form_get_nb_columns (account_number) - 1 ; column++ )
	gtk_tree_view_column_set_fixed_width ( gtk_tree_view_get_column ( GTK_TREE_VIEW ( tree_view ),
									  column ),
					       gsb_data_form_get_width_column (account_number,
									       column ) * allocation -> width / 100 );
    return FALSE;
}
/* *************************************************************************** */



/* *************************************************************************** */
/* cette fonction est appelée lors d'un motion sur la liste */
/* utilisée pour récupérer les changements de taille des colonnes */
/* pas trouvé de signal spécial pour le changement de taille des colonnes */
/* *************************************************************************** */
gboolean change_taille_colonne_organisation_formulaire ( GtkWidget *tree_view,
							 GdkEventMotion *motion )
{
    gint column;
    gint total_taille_colonnes;
    gint account_number;

    /*     si le bouton gauche n'est pas enfoncé, on vire */

    if ( motion -> state != GDK_BUTTON1_MASK )
	return FALSE;
    
    account_number = recupere_no_compte ( option_menu_comptes_choix_formulaire );

    /*     on récupère la taille des colonnes et vérifie avec la dernière s'il y a eu modification */
    /* 	ou non  */

    total_taille_colonnes = 0;

    for ( column=0 ; column < gsb_data_form_get_nb_columns (account_number) - 1 ; column++ )
    {
	gint colonne;
	gfloat total;

	colonne = gtk_tree_view_column_get_width ( gtk_tree_view_get_column ( GTK_TREE_VIEW ( tree_view ),
									      column ));
	total = colonne * 100 / tree_view -> allocation.width;
	gsb_data_form_set_width_column ( account_number,
					 column,
					 total );
	total_taille_colonnes = total_taille_colonnes + total;
    }

    if ( gsb_data_form_get_width_column ( account_number,
					  gsb_data_form_get_nb_columns (account_number) - 1) != ( 95 - total_taille_colonnes ))
    {
	gsb_data_form_set_width_column ( account_number,
					 gsb_data_form_get_nb_columns (account_number) - 1,
					 95 - total_taille_colonnes );
	modification_fichier ( TRUE );
    }
    
    /*     on met à jour le formulaire des opés si nécessaire */

    if ( !etat.formulaire_distinct_par_compte
	 ||
	 recupere_no_compte ( option_menu_comptes_choix_formulaire ) == gsb_data_account_get_current_account () )
	mise_a_jour_taille_formulaire ( formulaire -> allocation.width );

    return FALSE;
}
/* *************************************************************************** */



/* *************************************************************************** */
/* fonction appelée lorsqu'on clicke sur + ligne */
/* *************************************************************************** */
gboolean ajoute_ligne_organisation_formulaire ( void )
{
    GtkTreeIter iter;
    GtkListStore *store;
    gint account_number;

    account_number = recupere_no_compte ( option_menu_comptes_choix_formulaire );

    if ( gsb_data_form_get_nb_rows (account_number) == MAX_HEIGHT )
	return FALSE;

    gsb_data_form_set_nb_rows ( account_number,
				gsb_data_form_get_nb_rows (account_number) + 1 );

    /*     on crée une ligne blanche dans le treeview */

    store = GTK_LIST_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_organisation_formulaire )));

    gtk_list_store_append ( store,
			    &iter );

    modification_fichier (TRUE);

    return FALSE;
}
/* *************************************************************************** */




/* *************************************************************************** */
/* fonction appelée lorsqu'on clicke sur - ligne */
/* *************************************************************************** */
gboolean retire_ligne_organisation_formulaire ( void )
{
    GtkTreeIter iter;
    gint column;
    gint account_number;

    account_number = recupere_no_compte ( option_menu_comptes_choix_formulaire );

    if ( gsb_data_form_get_nb_rows (account_number) == 1 )
	return FALSE;

    /* check if it's possible */

    if ( !gsb_form_config_check_for_removing ( account_number,
					       1 ))
	return FALSE;

    gsb_data_form_set_nb_rows ( account_number,
				gsb_data_form_get_nb_rows (account_number) - 1 );

    /*     on peut donc retirer la dernière ligne, on replace les éléments s'y trouvant */
    /* 	dans ce qui restera */
    
    for ( column=0 ; column< gsb_data_form_get_nb_columns (account_number) ; column++ )
    {
	if ( gsb_data_form_get_value ( account_number,
				       column,
				       gsb_data_form_get_nb_rows (account_number)))
	{
	    /* 	    on a trouvé qque chose, on recherche la première place de libre */

	    gint tmp_row, tmp_column;

	    for ( tmp_row=0 ; tmp_row < gsb_data_form_get_nb_rows (account_number) ; tmp_row++ )
		for ( tmp_column=0 ; tmp_column < gsb_data_form_get_nb_columns (account_number) ; tmp_column++ )
		    if ( !gsb_data_form_get_value ( account_number,
						    tmp_column,
						    tmp_row ))
		    {
			gsb_data_form_set_value ( account_number,
						  tmp_column,
						  tmp_row,
						  gsb_data_form_get_value ( account_number,
									    column,
									    gsb_data_form_get_nb_rows (account_number)));
			gsb_data_form_set_value ( account_number,
						  column,
						  gsb_data_form_get_nb_rows (account_number),
						  0 );
			tmp_row = gsb_data_form_get_nb_rows (account_number);
			tmp_column = gsb_data_form_get_nb_columns (account_number);
		    }
	}
    }

    /*     on vire la ligne */

    gtk_tree_model_get_iter_from_string ( GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_organisation_formulaire ))),
					  &iter,
					  utils_str_itoa (gsb_data_form_get_nb_rows (account_number)));
    gtk_list_store_remove ( GTK_LIST_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_organisation_formulaire ))),
			    &iter );

    /*     on réaffiche la liste */
	    
    gsb_form_config_fill_store (account_number);
    
    modification_fichier (TRUE);
    return FALSE;
}
/* *************************************************************************** */




/* *************************************************************************** */
/* fonction appelée lorsqu'on clicke sur + colonne */
/* *************************************************************************** */
gboolean ajoute_colonne_organisation_formulaire ( void )
{
    gint column;
    gint total_taille_colonnes;
    gint account_number;
    gint nb_columns;

    account_number = recupere_no_compte ( option_menu_comptes_choix_formulaire );
    nb_columns = gsb_data_form_get_nb_columns (account_number);

    if ( nb_columns == MAX_WIDTH )
	return FALSE;

    gtk_tree_view_column_set_visible ( GTK_TREE_VIEW_COLUMN ( tree_view_column_organisation_formulaire[nb_columns]),
				       TRUE );

    /*     on modifie le pourcentage des colonnes pour la voir un peu */

    total_taille_colonnes = 0;

    for ( column=0 ; column < nb_columns ; column++ )
    {
	gfloat total;

	total = gsb_data_form_get_width_column ( account_number,
						 column );
	if ( total > 5 )
	{
	    total = total - 2;
	    gsb_data_form_set_width_column ( account_number,
					     column,
					     total );
	}
	total_taille_colonnes = total_taille_colonnes + total;
    }    

    gsb_data_form_set_width_column ( account_number,
				     nb_columns,
				     95 - total_taille_colonnes );

    gsb_data_form_set_nb_columns ( account_number,
				   nb_columns + 1 );

    /*     on affiche le résultat */

    allocation_precedente_organisation_formulaire = 0;
    allocation_liste_organisation_formulaire ( tree_view_organisation_formulaire,
					       &( tree_view_organisation_formulaire-> allocation));

    modification_fichier (TRUE);
    return FALSE;
}
/* *************************************************************************** */




/* *************************************************************************** */
/* fonction appelée lorsqu'on clicke sur - colonne */
/* *************************************************************************** */
gboolean retire_colonne_organisation_formulaire ( void )
{
    gint row;
    gint account_number;
    gint nb_columns;
    
    account_number = recupere_no_compte ( option_menu_comptes_choix_formulaire );
    nb_columns = gsb_data_form_get_nb_columns (account_number);

    if ( nb_columns == 1 )
	return FALSE;

    /* check if it's possible */

    if ( !gsb_form_config_check_for_removing ( account_number,
					       0 ))
	return FALSE;

    nb_columns--;
    gsb_data_form_set_nb_columns ( account_number,
				   nb_columns );


    /*     on peut donc retirer la dernière colonne, on replace les éléments s'y trouvant */
    /* 	dans ce qui restera */
    
    for ( row=0 ; row< gsb_data_form_get_nb_rows (account_number) ; row++ )
    {
	if ( gsb_data_form_get_value (account_number,
				      nb_columns,
				      row ))
	{
	    /* found something, look for the first place to set it */

	    gint tmp_row, tmp_column;

	    for ( tmp_row=0 ; tmp_row< gsb_data_form_get_nb_rows (account_number) ; tmp_row++ )
		for ( tmp_column=0 ; tmp_column<nb_columns ; tmp_column++ )
		    if ( !gsb_data_form_get_value ( account_number,
						    tmp_column,
						    tmp_row ))
		    {
			gsb_data_form_set_value ( account_number,
						  tmp_column,
						  tmp_row,
						  gsb_data_form_get_value ( account_number,
									    nb_columns,
									    row ));
			gsb_data_form_set_value ( account_number,
						  nb_columns,
						  row,
						  0 );
			tmp_row = gsb_data_form_get_nb_rows (account_number);
			tmp_column = nb_columns;
		    }
	}
    }

    /*     on vire la colonne */

    	    gtk_tree_view_column_set_visible ( GTK_TREE_VIEW_COLUMN ( tree_view_column_organisation_formulaire[nb_columns] ),
					       FALSE );

    /*     on réaffiche la liste */
	    
    gsb_form_config_fill_store ( account_number);

    modification_fichier (TRUE);
    return FALSE;
}
/* *************************************************************************** */



/**
 * check if we can remove a row or a column according the number
 * of values inside
 *
 * \param account_number
 * \param removing_row if TRUE it's a row we want to remove, else it's a column
 *
 * \return TRUE ok we can remove it, FALSE else
 * */
gboolean gsb_form_config_check_for_removing ( gint account_number,
					      gint removing_row )
{
    gint values;
    gint rows;
    gint columns;

    rows = gsb_data_form_get_nb_rows (account_number);
    columns = gsb_data_form_get_nb_columns (account_number);

    if ( !rows
	 ||
	 !columns )
	return FALSE;

    if ( removing_row )
	rows--;
    else
	columns--;

    /* the minimum of values is 3 : date, debit, credit*/

    if ( rows * columns < 3 )
	return FALSE;

    values = gsb_data_form_get_values_total (account_number);

     if ( values <= rows*columns )
	return TRUE;
    else
	return FALSE;
}


/**
 * called when we begin a drag,
 * find what cell was under the cursor and change it
 *
 * \param tree_view
 * \param drag_context
 * \param null
 *
 * \return FALSE
 * */
gboolean gsb_form_config_drag_begin ( GtkWidget *tree_view,
				      GdkDragContext *drag_context,
				      gpointer null )
{
    gint x, y;
    GtkTreePath *path;
    GtkTreeViewColumn *tree_column;
    GdkWindow *drawable;
    GdkRectangle rectangle;
    GdkPixbuf *pixbuf_cursor;

    /* get the cell */

    gdk_window_get_pointer ( gtk_tree_view_get_bin_window ( GTK_TREE_VIEW ( tree_view )),
			     &x,
			     &y,
			     FALSE );
    gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW ( tree_view ),
				    x,
				    y,
				    &path,
				    &tree_column,
				    NULL,
				    NULL );

    if ( !path
	 ||
	 !tree_column )
	return FALSE;

    start_drag_column = g_list_index ( gtk_tree_view_get_columns ( GTK_TREE_VIEW ( tree_view )),
				     tree_column );
    start_drag_row = utils_str_atoi ( gtk_tree_path_to_string ( path ));

    /* draw the new cursor */

    drawable = gtk_tree_view_get_bin_window (GTK_TREE_VIEW ( tree_view ));
    gtk_tree_view_get_cell_area ( GTK_TREE_VIEW ( tree_view ),
				  path,
				  tree_column,
				  &rectangle );
    pixbuf_cursor = gdk_pixbuf_get_from_drawable ( NULL,
						   GDK_DRAWABLE (drawable),
						   gdk_colormap_get_system (),
						   rectangle.x, rectangle.y,
						   0, 0,
						   rectangle.width, rectangle.height );

    gtk_drag_source_set_icon_pixbuf ( tree_view,
				      pixbuf_cursor );
    g_object_unref (pixbuf_cursor);

    return FALSE;
}
					

/**
 * called when we end a drag,
 * find what cell was under the cursor and do the split between the 2 cells
 *
 * \param tree_view
 * \param drag_context
 * \param null
 *
 * \return FALSE
 * */
gboolean gsb_form_config_drag_end ( GtkWidget *tree_view,
				      GdkDragContext *drag_context,
				      gpointer null )
{
    gint x, y;
    GtkTreePath *path;
    GtkTreeViewColumn *tree_column;
    gint end_drag_row;
    gint end_drag_column;
    gint buffer;
    gint account_number;

    /* get the cell position */

    gdk_window_get_pointer ( gtk_tree_view_get_bin_window ( GTK_TREE_VIEW ( tree_view )),
			     &x,
			     &y,
			     FALSE );
    gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW ( tree_view ),
				    x,
				    y,
				    &path,
				    &tree_column,
				    NULL,
				    NULL );

    if ( !path
	 ||
	 !tree_column )
	return FALSE;

    end_drag_column = g_list_index ( gtk_tree_view_get_columns ( GTK_TREE_VIEW ( tree_view )),
				     tree_column );
    end_drag_row = utils_str_atoi ( gtk_tree_path_to_string ( path ));

    /* if we are on the same cell, go away */

    if ( start_drag_row == end_drag_row
	 &&
	 start_drag_column == end_drag_column )
	return ( FALSE );

    /* swap the cells in the tab */

    account_number = recupere_no_compte ( option_menu_comptes_choix_formulaire );

    buffer = gsb_data_form_get_value ( account_number,
				       start_drag_column,
				       start_drag_row );
    gsb_data_form_set_value ( account_number,
			      start_drag_column,
			      start_drag_row,
			      gsb_data_form_get_value ( account_number,
							end_drag_column,
							end_drag_row ));
    gsb_data_form_set_value ( account_number,
			      end_drag_column,
			      end_drag_row,
			      buffer );

    /* fill the list */
	    
    gsb_form_config_fill_store (account_number);

    modification_fichier (FALSE);
    return (FALSE);
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */



