/* Ce fichier s'occupe de la configuration du formulaire dans les paramÃštres */
/* affichage.c */

/*     Copyright (C)	2000-2003 CÃ©dric Auger (cedric@grisbi.org) */
/*			2003-2004 Benjamin Drieu (bdrieu@april.org) */
/* 			http://www.grisbi.org */

/*     This program is free software; you can redistribute it and/or modify */
/*     it under the terms of the GNU General Public License as published by */
/*     the Free Software Foundation; either version 2 of the License, or */
/*     (at your option) any later version. */

/*     This program is distributed in the hope that it will be useful, */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/*     GNU General Public License for more details. */

/*     You should have received a copy of the GNU General Public License */
/*     along with this program; if not, write to the Free Software */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */


#include "include.h"


/*START_INCLUDE*/
#include "affichage_formulaire.h"
#include "utils.h"
#include "comptes_traitements.h"
#include "dialog.h"
#include "gsb_account.h"
#include "utils_str.h"
#include "operations_comptes.h"
#include "traitement_variables.h"
#include "utils_buttons.h"
#include "utils_comptes.h"
#include "operations_formulaire.h"
#include "include.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean ajoute_colonne_organisation_formulaire ( void );
static gboolean ajoute_ligne_organisation_formulaire ( void );
static gboolean allocation_liste_organisation_formulaire ( GtkWidget *tree_view,
						    GtkAllocation *allocation );
static gboolean button_press_classement_formulaire ( GtkWidget *tree_view,
					      GdkEventButton *ev );
static gboolean button_release_classement_formulaire ( GtkWidget *tree_view,
						GdkEventButton *ev );
static gboolean change_taille_colonne_organisation_formulaire ( GtkWidget *tree_view,
							 GdkEventMotion *motion );
static void mise_a_jour_organisation_formulaire ( gint no_compte );
static gboolean modification_compte_choix_formulaire ( GtkWidget *menu_item );
static gboolean modification_formulaire_distinct_par_compte ( void );
static gchar *recherche_nom_element_formulaire ( gint no_element );
static void remplissage_liste_organisation_formulaire ( GtkListStore *store,
						 struct organisation_formulaire *structure_formulaire );
static gboolean retire_colonne_organisation_formulaire ( void );
static gboolean retire_ligne_organisation_formulaire ( void );
static gboolean toggled_signal_configuration_formulaire ( GtkWidget *toggle_button );
static gboolean verification_retrait_possible ( struct organisation_formulaire *structure_formulaire,
					 gint retrait_ligne );
/*END_STATIC*/



/* utilisÃ© pour Ã©viter que Ã§a s'emballe lors du rÃ©glage de la largeur des colonnes du formulaire */

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

/* utilisÃ© pour la configuration du formulaire */

gint col_depart_drag;
gint ligne_depart_drag;

/* les adr des toggles button du contenu du formulaire */

GtkWidget *boutons_organisation_formulaire[TRANSACTION_FORM_WIDGET_NB-3];

/* l'option menu des comptes pour le choix d'un formulaire par compte */
/* ou le mÃªme pour tous les comptes */

GtkWidget *option_menu_comptes_choix_formulaire;

/*     rempli lors de l'affichage des paramÃštres */

GtkWidget *tree_view_organisation_formulaire;
GtkTreeViewColumn *tree_view_column_organisation_formulaire[6];

GtkWidget *formulaire;



/*START_EXTERN*/
extern     gchar * buffer ;
extern gint col_depart_drag;
extern gint ligne_depart_drag;
extern gint nb_colonnes;
extern GtkWidget *preview;
extern GtkTooltips *tooltips_general_grisbi;
extern GtkWidget *tree_view;
extern GtkWidget *treeview;
extern GtkWidget *window;
/*END_EXTERN*/



/* *************************************************************************** */
/* cette fonction crÃ©e une liste de 6 colonnes sur 4 lignes utilisÃ©e pour */
/* organiser le formulaire d'opÃ©rations */
/* *************************************************************************** */
GtkWidget *creation_liste_organisation_formulaire ( void )
{
    GtkWidget *fenetre;
    GtkListStore *store;
    gint i, j;
    GtkWidget *hbox, *hbox2;
    GtkWidget *bouton;
    GtkWidget *label;
    GtkWidget *paddingbox;
    GtkWidget *table;
    gint no_element_en_cours;
    GtkWidget *scrolled_window;



    /*     le fenetre sera une vbox avec la liste en haut et les boutons +- col et lignes */
    /* 	en dessous, et ce qu'on veut afficher en dessous */

    fenetre = gtk_vbox_new ( FALSE,
			     5 );
    gtk_widget_show ( fenetre );

    
    /*     mise en place de la boite qui contiendra la liste du formulaire */

    paddingbox = new_paddingbox_with_title ( fenetre,
					     FALSE,
					     _("Form structure preview"));

    /*     on propose de sÃ©parer l'affichage par compte */
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
    /* 	il est utilisÃ© pour Ã©viter l'agrandissement de la fenetre de conf */
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

    /*     crÃ©ation du store */

    store = gtk_list_store_new ( 6,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING );

    /*     on crÃ©e maintenant le tree_view */

    tree_view_organisation_formulaire = gtk_tree_view_new_with_model ( GTK_TREE_MODEL ( store ));
    gtk_container_add ( GTK_CONTAINER ( scrolled_window),
			tree_view_organisation_formulaire );
    gtk_widget_show ( tree_view_organisation_formulaire );

    /*     on crÃ©e les colonnes */

    for ( i=0 ; i< 6 ; i++ )
    {
	tree_view_column_organisation_formulaire[i] = gtk_tree_view_column_new_with_attributes ( nom_colonne[i],
												 gtk_cell_renderer_text_new (),
												 "text", i,
												 NULL );
	gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view_organisation_formulaire ),
				      GTK_TREE_VIEW_COLUMN ( tree_view_column_organisation_formulaire[i]));
	gtk_tree_view_column_set_sizing ( GTK_TREE_VIEW_COLUMN (tree_view_column_organisation_formulaire[i]),
					  GTK_TREE_VIEW_COLUMN_FIXED );
	gtk_tree_view_column_set_resizable ( GTK_TREE_VIEW_COLUMN (tree_view_column_organisation_formulaire[i]),
					     TRUE );
    }


    /*     on met en place le size allocate pour rÃ©gler la largeur des colonnes en fonction de la */
    /* 	taille de la liste */

    g_signal_connect ( G_OBJECT ( tree_view_organisation_formulaire ),
		       "size-allocate",
		       G_CALLBACK ( allocation_liste_organisation_formulaire ),
		       NULL );


    /*     pour le changement de taille des colonnes, pas trouvÃ© mieux... */

    g_signal_connect ( G_OBJECT (tree_view_organisation_formulaire),
		       "motion-notify-event",
		       G_CALLBACK ( change_taille_colonne_organisation_formulaire ),
		       NULL );

    g_signal_connect ( G_OBJECT (tree_view_organisation_formulaire),
		       "button-press-event",
		       G_CALLBACK ( button_press_classement_formulaire ),
		       NULL );
    g_signal_connect ( G_OBJECT (tree_view_organisation_formulaire),
		       "button-release-event",
		       G_CALLBACK ( button_release_classement_formulaire ),
		       NULL );


    /*     mise en place des boutons d'agrandissement ou de rÃ©trÃ©cissement */

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

    /*     crÃ©ation du tooltips s'il ne l'est pas dÃ©jÃ  */

    if ( !tooltips_general_grisbi )
	tooltips_general_grisbi = gtk_tooltips_new ();

    /* on crÃ©e maintenant une table de 3x6 boutons */
    table = gtk_table_new ( 3, 6, FALSE );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), table,
			 FALSE, FALSE, 0 );

    /*     on commence aprÃšs la date, le dÃ©bit et le crÃ©dit qui sont obligatoires */

    no_element_en_cours = 4;

    for ( i=0 ; i<3 ; i++ )
	for ( j=0 ; j<6 ; j++ )
	{
	    gchar *string;
	    gchar *string_modifie;

	    string = recherche_nom_element_formulaire (no_element_en_cours);

	    if ( string )
	    {
		/*on va couper cette chaine si elle est plus grande que 10 caractÃšres */
		
		string_modifie = limit_string ( string,
						10 );

		boutons_organisation_formulaire[j + i*6] = gtk_toggle_button_new_with_label ( string_modifie );
		g_object_set_data ( G_OBJECT ( boutons_organisation_formulaire[j + i*6] ),
				    "no_element",
				    GINT_TO_POINTER ( no_element_en_cours));

		g_signal_connect ( G_OBJECT ( boutons_organisation_formulaire[j + i*6] ),
				   "toggled",
				   G_CALLBACK (toggled_signal_configuration_formulaire),
				   NULL );
		gtk_table_attach_defaults ( GTK_TABLE ( table ),
					    boutons_organisation_formulaire[j + i*6],
					    j, j+1,
					    i, i+1 );
		
		/* 		mise en place du tooltips */

		gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
				       boutons_organisation_formulaire[j + i*6],
				       string,
				       string );
	    }

	    no_element_en_cours++;
	}

    allocation_precedente_organisation_formulaire = 0;

    /*     tout est crÃ©Ã©, on rempli maintenant avec le compte 0 */

    mise_a_jour_organisation_formulaire ( 0 );

    return (fenetre); 
}
/* *************************************************************************** */


/* *************************************************************************** */
/* cette fonction remplit la liste de l'organisation du formulaire et rend */
/* actif les boutons nÃ©cessaires pour le compte donnÃ© en argument */
/* *************************************************************************** */
void mise_a_jour_organisation_formulaire ( gint no_compte )
{
    gint i;
    gint j;
    gint no_element_en_cours;
    
    /*     on commence par remplir le store */

    remplissage_liste_organisation_formulaire ( GTK_LIST_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_organisation_formulaire ))),
						gsb_account_get_form_organization (no_compte) );

    /*     on rend visible les colonnes nÃ©cessaires */

    for ( i=0 ; i<6 ; i++ )
	if ( i<gsb_account_get_form_organization (no_compte) -> nb_colonnes )
	    gtk_tree_view_column_set_visible ( GTK_TREE_VIEW_COLUMN ( tree_view_column_organisation_formulaire[i] ),
					       TRUE );
	else
    	    gtk_tree_view_column_set_visible ( GTK_TREE_VIEW_COLUMN ( tree_view_column_organisation_formulaire[i] ),
					       FALSE );

    /*     on rend in/actif les boutons nÃ©cessaires */

    /*     on commence aprÃšs la date, le dÃ©bit et le crÃ©dit qui sont obligatoires */

    no_element_en_cours = 4;

    for ( i=0 ; i<3 ; i++ )
	for ( j=0 ; j<6 ; j++ )
	{
	    if ( j + i*6 < TRANSACTION_FORM_WIDGET_NB - 3 &&
		 boutons_organisation_formulaire[j + i*6] )
	    {
		g_signal_handlers_block_by_func ( G_OBJECT ( boutons_organisation_formulaire[j + i*6] ),
						  G_CALLBACK ( toggled_signal_configuration_formulaire ),
						  NULL );
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( boutons_organisation_formulaire[j + i*6] ),
					       verifie_element_formulaire_existe ( no_element_en_cours ));
		g_signal_handlers_unblock_by_func ( G_OBJECT ( boutons_organisation_formulaire[j + i*6] ),
						    G_CALLBACK ( toggled_signal_configuration_formulaire ),
						    NULL );
	    }
	    no_element_en_cours++;
	}
}
/* *************************************************************************** */



/* *************************************************************************** */
/* cette fonction est appelÃ©e lorsqu'on change le bouton du choix du formulaire */
/* diffÃ©rent par compte. (dÃ©)grise l'option menu des comptes et le remet sur le premier */
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
/* cette fonction est appelÃ©e si on change le compte affichÃ© par l'option menu */
/* *************************************************************************** */
gboolean modification_compte_choix_formulaire ( GtkWidget *menu_item )
{
    gint no_compte;

    no_compte = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( menu_item ),
							"no_compte" ));
    mise_a_jour_organisation_formulaire ( no_compte );


    return FALSE;
}
/* *************************************************************************** */





/* *************************************************************************** */
/* cette fonction cherche dans l'organisation du formulaire la position de l'Ã©lÃ©ment */
/* donnÃ© en argument. */
/* si elle le trouve elle remplit ligne et colonne et return TRUE */
/* ligne et colonne peuvent Ãªtre NULL */
/* return FALSE si pas trouvÃ© */
/* *************************************************************************** */
gboolean recherche_place_element_formulaire ( struct organisation_formulaire *structure_formulaire,
					      gint no_element,
					      gint *ligne,
					      gint *colonne )
{
    gint i, j;

    for ( i=0 ; i < structure_formulaire -> nb_lignes ; i++ )
	for ( j=0 ; j < structure_formulaire -> nb_colonnes ; j++ )
	    if ( structure_formulaire -> tab_remplissage_formulaire[i][j] == no_element )
	    {
		if ( ligne )
		    *ligne = i;
		if ( colonne )
		    *colonne = j;
		return TRUE;
	    }
    return FALSE;
}
/* *************************************************************************** */




/* *************************************************************************** */
/* cette fonction utilise recherche_place_element_formulaire mais cherche juste */
/* Ã  vÃ©rifier que le champ donnÃ© en argument existe */
/* et elle ne cherche que sur le formulaire du compte courant */
/* \param no_element Ã©lÃ©ment recherchÃ© */
/* *************************************************************************** */
gboolean verifie_element_formulaire_existe ( gint no_element )
{
    struct organisation_formulaire *structure_formulaire;

    structure_formulaire = renvoie_organisation_formulaire ();

    return ( recherche_place_element_formulaire ( structure_formulaire,
						  no_element,
						  NULL,
						  NULL ));
}
/* *************************************************************************** */



/* *************************************************************************** */
/* cette fonction est appelÃ©e lorsqu'on clique sur l'un des boutons de l'organisation */
/* du formulaire, elle ajoute ou retire l'Ã©lÃ©ment de la liste */
/* *************************************************************************** */
gboolean toggled_signal_configuration_formulaire ( GtkWidget *toggle_button )
{
    gint no_element;
    gint no_second_element;
    gint i, j;
    gint no_compte;

    /*     on commence par rechercher le no de l'Ã©lÃ©ment */

    no_element = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( toggle_button ),
						       "no_element" ));
    /*     on met ici le 2nd elt si nÃ©cessaire */

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

    no_compte = recupere_no_compte ( option_menu_comptes_choix_formulaire );

    /*     on met Ã  jour le tableau */

    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( toggle_button )))
    {
	/* 	on l'a enclenchÃ©, on rajoute l'Ã©lÃ©ment */

	gint place_trouvee = 0;
	gint ligne_premier_elt = -1;
	gint colonne_premier_elt = -1;

	for ( i=0 ; i < gsb_account_get_form_organization (no_compte) -> nb_lignes ; i++)
	    for ( j=0 ; j < gsb_account_get_form_organization (no_compte) -> nb_colonnes ; j++ )
		if ( !gsb_account_get_form_organization (no_compte) -> tab_remplissage_formulaire[i][j] )
		{
		    /* 		    s'il n'y a qu'un elt, on le met et on termine, sinon on continue Ã  chercher */
		    /* 			pour le 2Ãšme */

		    if ( no_second_element == -1 )
		    {
			/* 			il n'y a qu'un elt */
			
			gsb_account_get_form_organization (no_compte) -> tab_remplissage_formulaire[i][j] = no_element;
			place_trouvee = 1;
			i = gsb_account_get_form_organization (no_compte) -> nb_lignes;
			j = gsb_account_get_form_organization (no_compte) -> nb_colonnes;
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
			    /* 			    on vient de trouver la place pour le 2Ãšme */

			    gsb_account_get_form_organization (no_compte) -> tab_remplissage_formulaire[ligne_premier_elt][colonne_premier_elt] = no_element;
			    gsb_account_get_form_organization (no_compte) -> tab_remplissage_formulaire[i][j] = no_second_element;
			    place_trouvee = 1;
			    i = gsb_account_get_form_organization (no_compte) -> nb_lignes;
			    j = gsb_account_get_form_organization (no_compte) -> nb_colonnes;
			}
		    }
		}

	if ( place_trouvee )
	{
	    /* 	    on a trouvÃ© une place pour le/les Ã©lements */
	    /* 		s'il y a plusieurs Ã©lÃ©ments en jeu, il faut le rendre actif aussi */

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
	    /* 	    le tableau est trop petit pour rajouter un Ã©lÃ©ment */
	
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
	/* 	on a dÃ©senclenchÃ© le bouton */

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

	for ( i=0 ; i < gsb_account_get_form_organization (no_compte) -> nb_lignes ; i++ )
	    for ( j=0 ; j < gsb_account_get_form_organization (no_compte) -> nb_colonnes ; j++ )
		if ( gsb_account_get_form_organization (no_compte) -> tab_remplissage_formulaire[i][j] == no_element )
		{
		    gsb_account_get_form_organization (no_compte) -> tab_remplissage_formulaire[i][j] = 0;
		    if ( no_second_element == -1 )
		    {
			i = gsb_account_get_form_organization (no_compte) -> nb_lignes;
			j = gsb_account_get_form_organization (no_compte) -> nb_colonnes;
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
    /*     on rÃ©affiche la liste */

    remplissage_liste_organisation_formulaire ( GTK_LIST_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_organisation_formulaire ))),
						gsb_account_get_form_organization (no_compte) );

    modification_fichier (TRUE);
    return FALSE;

}
/* *************************************************************************** */


/* *************************************************************************** */
/* cette fonction est appelÃ©e pour remplir/reremplir la liste d'organisation */
/* du formulaire */
/* *************************************************************************** */
void remplissage_liste_organisation_formulaire ( GtkListStore *store,
						 struct organisation_formulaire *structure_formulaire )
{
    gint i, j;
    GtkTreeIter iter;

    gtk_list_store_clear ( store );

    for ( i=0 ; i < structure_formulaire -> nb_lignes ; i++ )
    {
	gtk_list_store_append ( GTK_LIST_STORE ( store ),
				&iter );
	for ( j = 0 ; j < 6 ; j++ )
	{
	    gtk_list_store_set ( GTK_LIST_STORE ( store ),
				 &iter,
				 j, recherche_nom_element_formulaire (structure_formulaire -> tab_remplissage_formulaire[i][j]),
				 -1 );
	}
    }

    /*     on met Ã  jour si nÃ©cessaire le formulaire de la liste d'opÃ© */

    if ( !etat.formulaire_distinct_par_compte
	 ||
	 recupere_no_compte ( option_menu_comptes_choix_formulaire ) == gsb_account_get_current_account () )
	remplissage_formulaire ( gsb_account_get_current_account () );
}
/* *************************************************************************** */




/* *************************************************************************** */
/* cette fonction prend un numÃ©ro d'Ã©lÃ©ment du formulaire et renvoie son texte */
/* retourne NULL si pas trouvÃ© */
/* *************************************************************************** */
gchar *recherche_nom_element_formulaire ( gint no_element )
{
    switch ( no_element )
    {
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
/* fonction appelÃ©e Ã  l'affichage de la liste d'organisation du formulaire */
/* pour adapter la taille des colonnes Ã  celle demandÃ©e */
/* *************************************************************************** */
gboolean allocation_liste_organisation_formulaire ( GtkWidget *tree_view,
						    GtkAllocation *allocation )
{
    gint i;
    gint no_compte;

    if ( ! assert_account_loaded() )
      return FALSE;

    if ( allocation_precedente_organisation_formulaire == allocation -> width )
	return FALSE;

    allocation_precedente_organisation_formulaire= allocation -> width;

    no_compte = recupere_no_compte ( option_menu_comptes_choix_formulaire );

    for ( i=0 ; i < gsb_account_get_form_organization (no_compte) -> nb_colonnes - 1 ; i++ )
	gtk_tree_view_column_set_fixed_width ( gtk_tree_view_get_column ( GTK_TREE_VIEW ( tree_view ),
									  i ),
					       gsb_account_get_form_organization (no_compte) -> taille_colonne_pourcent[i] * allocation -> width / 100 );
    return FALSE;
}
/* *************************************************************************** */



/* *************************************************************************** */
/* cette fonction est appelÃ©e lors d'un motion sur la liste */
/* utilisÃ©e pour rÃ©cupÃ©rer les changements de taille des colonnes */
/* pas trouvÃ© de signal spÃ©cial pour le changement de taille des colonnes */
/* *************************************************************************** */
gboolean change_taille_colonne_organisation_formulaire ( GtkWidget *tree_view,
							 GdkEventMotion *motion )
{
    gint i;
    gint total_taille_colonnes;
    gint no_compte;

    /*     si le bouton gauche n'est pas enfoncÃ©, on vire */

    if ( motion -> state != GDK_BUTTON1_MASK )
	return FALSE;
    
    no_compte = recupere_no_compte ( option_menu_comptes_choix_formulaire );

    /*     on rÃ©cupÃšre la taille des colonnes et vÃ©rifie avec la derniÃšre s'il y a eu modification */
    /* 	ou non  */

    total_taille_colonnes = 0;

    for ( i=0 ; i < gsb_account_get_form_organization (no_compte) -> nb_colonnes - 1 ; i++ )
    {
	gint colonne;

	colonne = gtk_tree_view_column_get_width ( gtk_tree_view_get_column ( GTK_TREE_VIEW ( tree_view ),
									      i ));
	gsb_account_get_form_organization (no_compte) -> taille_colonne_pourcent[i] = colonne * 100 / tree_view -> allocation.width;
	total_taille_colonnes = total_taille_colonnes + gsb_account_get_form_organization (no_compte) -> taille_colonne_pourcent[i];
    }

    if ( gsb_account_get_form_organization (no_compte) -> taille_colonne_pourcent[gsb_account_get_form_organization (no_compte) -> nb_colonnes - 1] != ( 95 - total_taille_colonnes ))
    {
	gsb_account_get_form_organization (no_compte) -> taille_colonne_pourcent[gsb_account_get_form_organization (no_compte) -> nb_colonnes - 1] = 95 - total_taille_colonnes;
	modification_fichier ( TRUE );
    }
    
    /*     on met Ã  jour le formulaire des opÃ©s si nÃ©cessaire */

    if ( !etat.formulaire_distinct_par_compte
	 ||
	 recupere_no_compte ( option_menu_comptes_choix_formulaire ) == gsb_account_get_current_account () )
	mise_a_jour_taille_formulaire ( formulaire -> allocation.width );

    return FALSE;
}
/* *************************************************************************** */



/* *************************************************************************** */
/* fonction appelÃ©e lorsqu'on clicke sur + ligne */
/* *************************************************************************** */
gboolean ajoute_ligne_organisation_formulaire ( void )
{
    GtkTreeIter iter;
    GtkListStore *store;
    gint no_compte;

    no_compte = recupere_no_compte ( option_menu_comptes_choix_formulaire );

    if ( gsb_account_get_form_organization (no_compte) -> nb_lignes == 4 )
	return FALSE;

    gsb_account_get_form_organization (no_compte) -> nb_lignes++;

    /*     on crÃ©e une ligne blanche dans le treeview */

    store = GTK_LIST_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_organisation_formulaire )));

    gtk_list_store_append ( store,
			    &iter );

    modification_fichier (TRUE);

    return FALSE;
}
/* *************************************************************************** */




/* *************************************************************************** */
/* fonction appelÃ©e lorsqu'on clicke sur - ligne */
/* *************************************************************************** */
gboolean retire_ligne_organisation_formulaire ( void )
{
    GtkTreeIter iter;
    gint i;
    gint no_compte;

    no_compte = recupere_no_compte ( option_menu_comptes_choix_formulaire );

    if ( gsb_account_get_form_organization (no_compte) -> nb_lignes == 1 )
	return FALSE;


    /*     on vÃ©rifie que c'est possible */

    if ( !verification_retrait_possible ( gsb_account_get_form_organization (no_compte),
					  1 ))
	return FALSE;

    gsb_account_get_form_organization (no_compte) -> nb_lignes--;

    /*     on peut donc retirer la derniÃšre ligne, on replace les Ã©lÃ©ments s'y trouvant */
    /* 	dans ce qui restera */
    
    for ( i=0 ; i<gsb_account_get_form_organization (no_compte) -> nb_colonnes ; i++ )
    {
	if ( gsb_account_get_form_organization (no_compte) -> tab_remplissage_formulaire[gsb_account_get_form_organization (no_compte) -> nb_lignes][i] )
	{
	    /* 	    on a trouvÃ© qque chose, on recherche la premiÃšre place de libre */

	    gint j, k;

	    for ( j=0 ; j< gsb_account_get_form_organization (no_compte) -> nb_lignes ; j++ )
		for ( k=0 ; k<gsb_account_get_form_organization (no_compte) -> nb_colonnes ; k++ )
		    if ( gsb_account_get_form_organization (no_compte) -> tab_remplissage_formulaire[j][k] == 0 )
		    {
			gsb_account_get_form_organization (no_compte) -> tab_remplissage_formulaire[j][k] = gsb_account_get_form_organization (no_compte) -> tab_remplissage_formulaire[gsb_account_get_form_organization (no_compte) -> nb_lignes][i];
			 gsb_account_get_form_organization (no_compte) -> tab_remplissage_formulaire[gsb_account_get_form_organization (no_compte) -> nb_lignes][i] = 0;
			j = gsb_account_get_form_organization (no_compte) -> nb_lignes;
			k = gsb_account_get_form_organization (no_compte) -> nb_colonnes;
		    }
	}
    }

    /*     on vire la ligne */

    gtk_tree_model_get_iter_from_string ( GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_organisation_formulaire ))),
					  &iter,
					  utils_str_itoa ( gsb_account_get_form_organization (no_compte) -> nb_lignes ));
    gtk_list_store_remove ( GTK_LIST_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_organisation_formulaire ))),
			    &iter );

    /*     on rÃ©affiche la liste */
	    
    remplissage_liste_organisation_formulaire ( GTK_LIST_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_organisation_formulaire ))),
						gsb_account_get_form_organization (no_compte) );
    
    modification_fichier (TRUE);
    return FALSE;
}
/* *************************************************************************** */




/* *************************************************************************** */
/* fonction appelÃ©e lorsqu'on clicke sur + colonne */
/* *************************************************************************** */
gboolean ajoute_colonne_organisation_formulaire ( void )
{
    gint i;
    gint total_taille_colonnes;
    gint no_compte;

    no_compte = recupere_no_compte ( option_menu_comptes_choix_formulaire );

    if ( gsb_account_get_form_organization (no_compte) -> nb_colonnes == 6 )
	return FALSE;

    gtk_tree_view_column_set_visible ( GTK_TREE_VIEW_COLUMN ( tree_view_column_organisation_formulaire[gsb_account_get_form_organization (no_compte) -> nb_colonnes] ),
				       TRUE );

    /*     on modifie le pourcentage des colonnes pour la voir un peu */

    total_taille_colonnes = 0;

    for ( i=0 ; i < gsb_account_get_form_organization (no_compte) -> nb_colonnes ; i++ )
    {
	if ( gsb_account_get_form_organization (no_compte) -> taille_colonne_pourcent[i] > 5 )
	    gsb_account_get_form_organization (no_compte) -> taille_colonne_pourcent[i] = gsb_account_get_form_organization (no_compte) -> taille_colonne_pourcent[i] - 2;
	total_taille_colonnes = total_taille_colonnes + gsb_account_get_form_organization (no_compte) -> taille_colonne_pourcent[i];
    }    

    gsb_account_get_form_organization (no_compte) -> taille_colonne_pourcent[gsb_account_get_form_organization (no_compte) -> nb_colonnes] = 95 - total_taille_colonnes;

    gsb_account_get_form_organization (no_compte) -> nb_colonnes++;

    /*     on affiche le rÃ©sultat */

    allocation_precedente_organisation_formulaire = 0;
    allocation_liste_organisation_formulaire ( tree_view_organisation_formulaire,
					       &( tree_view_organisation_formulaire-> allocation));

    modification_fichier (TRUE);
    return FALSE;
}
/* *************************************************************************** */




/* *************************************************************************** */
/* fonction appelÃ©e lorsqu'on clicke sur - colonne */
/* *************************************************************************** */
gboolean retire_colonne_organisation_formulaire ( void )
{
    gint i;
    gint no_compte;

    no_compte = recupere_no_compte ( option_menu_comptes_choix_formulaire );

    if ( gsb_account_get_form_organization (no_compte) -> nb_colonnes == 1 )
	return FALSE;

    /*     on vÃ©rifie que c'est possible */

    if ( !verification_retrait_possible ( gsb_account_get_form_organization (no_compte),
					  0 ))
	return FALSE;

    gsb_account_get_form_organization (no_compte) -> nb_colonnes--;

    /*     on peut donc retirer la derniÃšre colonne, on replace les Ã©lÃ©ments s'y trouvant */
    /* 	dans ce qui restera */
    
    for ( i=0 ; i<gsb_account_get_form_organization (no_compte) -> nb_lignes ; i++ )
    {
	if ( gsb_account_get_form_organization (no_compte) -> tab_remplissage_formulaire[i][gsb_account_get_form_organization (no_compte) -> nb_colonnes] )
	{
	    /* 	    on a trouvÃ© qque chose, on recherche la premiÃšre place de libre */

	    gint j, k;

	    for ( j=0 ; j< gsb_account_get_form_organization (no_compte) -> nb_lignes ; j++ )
		for ( k=0 ; k<gsb_account_get_form_organization (no_compte) -> nb_colonnes ; k++ )
		    if ( gsb_account_get_form_organization (no_compte) -> tab_remplissage_formulaire[j][k] == 0 )
		    {
			gsb_account_get_form_organization (no_compte) -> tab_remplissage_formulaire[j][k] = gsb_account_get_form_organization (no_compte) -> tab_remplissage_formulaire[i][gsb_account_get_form_organization (no_compte) -> nb_colonnes];
			gsb_account_get_form_organization (no_compte) -> tab_remplissage_formulaire[i][gsb_account_get_form_organization (no_compte) -> nb_colonnes] = 0;
			j = gsb_account_get_form_organization (no_compte) -> nb_lignes;
			k = gsb_account_get_form_organization (no_compte) -> nb_colonnes;
		    }
	}
    }

    /*     on vire la colonne */

    	    gtk_tree_view_column_set_visible ( GTK_TREE_VIEW_COLUMN ( tree_view_column_organisation_formulaire[gsb_account_get_form_organization (no_compte) -> nb_colonnes] ),
					       FALSE );

    /*     on rÃ©affiche la liste */
	    
    remplissage_liste_organisation_formulaire ( GTK_LIST_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view_organisation_formulaire ))),
						gsb_account_get_form_organization (no_compte) );

    modification_fichier (TRUE);
    return FALSE;
}
/* *************************************************************************** */



/* *************************************************************************** */
/* cette fonction est appelÃ©e lorsqu'on retire 1 ligne ou une colonne de la liste */
/* d'organisation du formulaire, elle vÃ©rifie qu'il n'y a pas trop d'Ã©lÃ©ments dans la */
/* liste aprÃšs ce retrait */
/* retrait_ligne = 1 si c'est un retrait de ligne, 0 pour les colonnes */
/* renvoie TRUE si c'est possible */
/* *************************************************************************** */

gboolean verification_retrait_possible ( struct organisation_formulaire *structure_formulaire,
					 gint retrait_ligne )
{
    gint nb_elements;
    gint nb_lignes;
    gint nb_colonnes;
    gint i, j;

    nb_lignes = structure_formulaire -> nb_lignes;
    nb_colonnes = structure_formulaire -> nb_colonnes;

    if ( retrait_ligne )
	nb_lignes--;
    else
	nb_colonnes--;

    /*     le minimum est de 3 : date, dÃ©bit, crÃ©dit*/

    if ( nb_lignes * nb_colonnes < 3 )
	return FALSE;

    /*     on calcule le nb d'Ã©lÃ©ments dans la liste */

    nb_elements = 0;

    for ( i=0 ; i<4 ; i++ )
	for ( j=0 ; j<6 ; j++ )
	    if ( structure_formulaire -> tab_remplissage_formulaire[i][j] )
		nb_elements++;

    if ( nb_elements <= nb_lignes*nb_colonnes )
	return TRUE;
    else
	return FALSE;
}
/* *************************************************************************** */


					

/* ************************************************************************************************************** */
gboolean button_press_classement_formulaire ( GtkWidget *tree_view,
					      GdkEventButton *ev )
{
    GdkCursor *cursor;
    GdkPixmap *source, *mask;
    GdkColor fg = { 0, 65535, 0, 0 }; /* Red. */
    GdkColor bg = { 0, 0, 0, 65535 }; /* Blue. */
    gint x, y;
    GtkTreePath *path;
    GtkTreeViewColumn *tree_colonne;

    static unsigned char cursor1_bits[] = {
	0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01,
	0x00, 0x00, 0x3f, 0xfc, 0x3f, 0xfc, 0x00, 0x00, 0x80, 0x01, 0x80, 0x01,
	0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01
    };
    static unsigned char cursor1mask_bits[] = {
	0x80, 0x01, 0x8e, 0x71, 0x86, 0x61, 0x8a, 0x51, 0x90, 0x09, 0xa0, 0x05,
	0x40, 0x02, 0x3f, 0xfc, 0x3f, 0xfc, 0x40, 0x02, 0xa0, 0x05, 0x90, 0x09,
	0x8a, 0x51, 0x86, 0x61, 0x8e, 0x71, 0x80, 0x01
    };

    /*   si la souris se trouve dans les titres, on se barre simplement */

    if ( ev -> window != gtk_tree_view_get_bin_window ( GTK_TREE_VIEW ( tree_view )) )
	return(FALSE);


    /* on crÃ©e le nouveau curseur */

    source = gdk_bitmap_create_from_data (NULL,
					  cursor1_bits,
					  16,
					  16);
    mask = gdk_bitmap_create_from_data (NULL,
					cursor1mask_bits,
					16,
					16);

    cursor = gdk_cursor_new_from_pixmap (source,
					 mask,
					 &fg,
					 &bg,
					 8,
					 8);
    gdk_pixmap_unref (source);
    gdk_pixmap_unref (mask);


    /* RÃ©cupÃ©ration des coordonnÃ©es de la souris */


    gdk_window_get_pointer ( gtk_tree_view_get_bin_window ( GTK_TREE_VIEW ( tree_view )),
			     &x,
			     &y,
			     FALSE );

    /*     on rÃ©cupÃšre le path aux coordonnÃ©es */
    /* 	si ce n'est pas une ligne de la liste, on se barre */

    gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW ( tree_view ),
				    x,
				    y,
				    &path,
				    &tree_colonne,
				    NULL,
				    NULL );

    /* rÃ©cupÃšre et sauve les coordonnÃ©es de la liste au dÃ©part */

    col_depart_drag = g_list_index ( gtk_tree_view_get_columns ( GTK_TREE_VIEW ( tree_view )),
				     tree_colonne );
    ligne_depart_drag = utils_str_atoi ( gtk_tree_path_to_string ( path ));


    /* on grab la souris */

    gdk_pointer_grab ( gtk_tree_view_get_bin_window ( GTK_TREE_VIEW ( tree_view )),
		       FALSE,
		       GDK_BUTTON_RELEASE_MASK,
		       gtk_tree_view_get_bin_window ( GTK_TREE_VIEW ( tree_view )),
		       cursor,
		       GDK_CURRENT_TIME );

    return ( TRUE );
}
/* ************************************************************************************************************** */




/* ************************************************************************************************************** */
gboolean button_release_classement_formulaire ( GtkWidget *tree_view,
						GdkEventButton *ev )
{
    gint ligne_arrivee_drag;
    gint col_arrivee_drag;
    gint buffer;
    gint x, y;
    GtkTreePath *path;
    GtkTreeViewColumn *tree_colonne;
    gint no_compte;

    /*   si la souris se trouve dans les titres, on se barre simplement */

    if ( ev -> window != gtk_tree_view_get_bin_window ( GTK_TREE_VIEW ( tree_view )) )
	return(FALSE);

    /* RÃ©cupÃ©ration des coordonnÃ©es de la souris */

    gdk_window_get_pointer ( gtk_tree_view_get_bin_window ( GTK_TREE_VIEW ( tree_view )),
			     &x,
			     &y,
			     FALSE );

    /*     on rÃ©cupÃšre le path aux coordonnÃ©es */
    /* 	si ce n'est pas une ligne de la liste, on se barre */

    gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW ( tree_view ),
				    x,
				    y,
				    &path,
				    &tree_colonne,
				    NULL,
				    NULL );

    /* rÃ©cupÃšre et sauve les coordonnÃ©es de la liste au dÃ©part */

    col_arrivee_drag = g_list_index ( gtk_tree_view_get_columns ( GTK_TREE_VIEW ( tree_view )),
				      tree_colonne );
    ligne_arrivee_drag = utils_str_atoi ( gtk_tree_path_to_string ( path ));


     /* on dÃ©grab la souris */

    gdk_pointer_ungrab ( GDK_CURRENT_TIME );

    /* si la cellule de dÃ©part est la mÃªme que celle de l'arrivÃ©e, on se barre */

    if ( ligne_depart_drag == ligne_arrivee_drag
	 &&
	 col_depart_drag == col_arrivee_drag )
	return ( TRUE );

    /*     on Ã©change dans le tableau */

    no_compte = recupere_no_compte ( option_menu_comptes_choix_formulaire );

    buffer = gsb_account_get_form_organization (no_compte) -> tab_remplissage_formulaire[ligne_depart_drag][col_depart_drag];
    gsb_account_get_form_organization (no_compte) -> tab_remplissage_formulaire[ligne_depart_drag][col_depart_drag] = gsb_account_get_form_organization (no_compte) -> tab_remplissage_formulaire[ligne_arrivee_drag][col_arrivee_drag];
    gsb_account_get_form_organization (no_compte) -> tab_remplissage_formulaire[ligne_arrivee_drag][col_arrivee_drag] = buffer;

    /*     on rÃ©affiche la liste */
	    
    remplissage_liste_organisation_formulaire ( GTK_LIST_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW (  tree_view ))),
						gsb_account_get_form_organization (no_compte) );


    modification_fichier (TRUE);
    return ( TRUE );
}
/* ************************************************************************************************************** */

	   



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
