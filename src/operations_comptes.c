/*  Fichier qui gère la liste des comptes, la partie gauche de l'onglet opérations */
/*      operations_comptes.c */

/*     Copyright (C) 2000-2003  Cédric Auger */
/* 			cedric@grisbi.org */
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
#include "structures.h"
#include "variables-extern.c"
#include "operations_comptes.h"




#include "barre_outils.h"
#include "devises.h"
#include "equilibrage.h"
#include "menu.h"
#include "operations_formulaire.h"
#include "operations_liste.h"
#include "search_glist.h"
#include "type_operations.h"
#include "utils.h"
#include "ventilation.h"
#include "gtk_list_button.h"
#include "main.h"



extern GtkItemFactory *item_factory_menu_general;
extern gint id_fonction_idle;



/* ********************************************************************************************************** */
/*** Création de la fenêtre de comptes ***/
/* **************************************************************************************************** */

GtkWidget *creation_liste_comptes (void)
{
    GtkWidget *onglet;
    GtkWidget *frame_label_compte_courant;
    GtkWidget *bouton;
    GtkWidget *frame_equilibrage;
    GtkWidget *vbox_frame_equilibrage;
    GtkWidget *scrolled_window;

    /*  Création d'une fenêtre générale*/

    onglet = gtk_vbox_new ( FALSE,
			    10);
    gtk_container_set_border_width ( GTK_CONTAINER ( onglet ),
				     10 );
    gtk_signal_connect ( GTK_OBJECT ( onglet ),
			 "key_press_event",
			 GTK_SIGNAL_FUNC ( traitement_clavier_liste ),
			 NULL );
    gtk_widget_show ( onglet );

    /*  Création du label Comptes en haut */


    /*   on place le label dans une frame */

    frame_label_compte_courant = gtk_frame_new ( NULL );
    gtk_frame_set_shadow_type ( GTK_FRAME ( frame_label_compte_courant ),
				GTK_SHADOW_ETCHED_OUT );
    gtk_box_pack_start ( GTK_BOX (onglet),
			 frame_label_compte_courant,
			 FALSE,
			 TRUE,
			 0);
    gtk_widget_show (frame_label_compte_courant);


    /*   on ne met rien dans le label, il sera rempli ensuite */

    label_compte_courant = gtk_label_new ( "" );
    gtk_misc_set_alignment ( GTK_MISC (label_compte_courant  ),
			     0.5,
			     0.5);
    gtk_container_add ( GTK_CONTAINER ( frame_label_compte_courant ),
			label_compte_courant );
    gtk_widget_show (label_compte_courant);


    /*  Création de la fenêtre des comptes */
    /*  qui peut contenir des barres de défilement si */
    /*  nécessaire */

    scrolled_window = gtk_scrolled_window_new ( NULL,
						NULL);
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_NEVER,
				     GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start ( GTK_BOX ( onglet ),
			 scrolled_window,
			 TRUE,
			 TRUE,
			 0);
    gtk_widget_show ( scrolled_window );


    /*  création d'une vbox contenant la liste des comptes */

    vbox_liste_comptes = gtk_vbox_new ( FALSE,
					10);
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW (scrolled_window ),
					    vbox_liste_comptes);
    gtk_viewport_set_shadow_type ( GTK_VIEWPORT ( GTK_BIN ( scrolled_window )  -> child ),
				   GTK_SHADOW_NONE );
    gtk_widget_show (vbox_liste_comptes);


    /*  Création d'une icone et du nom par compte, et placement dans la
	liste selon l'ordre désiré  */
    if ( nb_comptes )
    {
	reaffiche_liste_comptes ();
    }

    /* ajoute le bouton et le label pour l'équilibrage de compte */
    /* les 2 seront intégrés dans une frame */

    frame_equilibrage = gtk_frame_new ( NULL );
    gtk_frame_set_shadow_type ( GTK_FRAME ( frame_equilibrage ),
				GTK_SHADOW_ETCHED_IN );
    gtk_box_pack_start ( GTK_BOX ( onglet ),
			 frame_equilibrage,
			 FALSE,
			 TRUE,
			 0);
    gtk_widget_show ( frame_equilibrage );


    vbox_frame_equilibrage = gtk_vbox_new ( FALSE,
					    5 );
    gtk_container_add ( GTK_CONTAINER  ( frame_equilibrage ),
			vbox_frame_equilibrage );
    gtk_widget_show ( vbox_frame_equilibrage );


    /* mise en place du label */

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

    if ( nb_comptes )
	label_releve = gtk_label_new ( g_strconcat ( COLON(_("Last statement")),
						     DATE_DERNIER_RELEVE,
						     NULL ) );
    else
	label_releve = gtk_label_new ( COLON(_("Last statement")) );

    gtk_box_pack_start ( GTK_BOX ( vbox_frame_equilibrage ),
			 label_releve,
			 FALSE,
			 TRUE,
			 0);
    gtk_widget_show ( label_releve );


    /* mise en place du bouton équilibrage */

    bouton = gtk_button_new_with_label ( _("Reconcile") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE);
    gtk_box_pack_start ( GTK_BOX ( vbox_frame_equilibrage ),
			 bouton,
			 FALSE,
			 TRUE,
			 0);
    gtk_signal_connect ( GTK_OBJECT (bouton),
			 "clicked",
			 GTK_SIGNAL_FUNC ( equilibrage ),
			 NULL );
    gtk_widget_show ( bouton );


    return ( onglet );

}
/* ********************************************************************************************************** */




/* ********************************************************************************************************** */
/** Fonction qui renvoie un widget contenant un bouton **/
/** de compte associé à son nom **/
/* ********************************************************************************************************** */

GtkWidget *comptes_appel ( gint no_de_compte )
{
    GtkWidget *bouton;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + no_de_compte;

    bouton = gtk_list_button_new ( NOM_DU_COMPTE, 2 );
    gtk_signal_connect_object ( GTK_OBJECT (bouton), "clicked",
				GTK_SIGNAL_FUNC ( changement_compte ),
				GINT_TO_POINTER ( no_de_compte ) );
    gtk_widget_show ( bouton );

    return ( bouton );
}
/* ********************************************************************************************************** */


/* ********************************************************************************************************** */
void changement_compte_par_menu ( gpointer null,
				  gint compte_plus_un )
{
    changement_compte ( GINT_TO_POINTER ( compte_plus_un - 1) );
}
/* ********************************************************************************************************** */




/* ********************************************************************************************************** */
/*  Routine appelée lors de changement de compte */
/* ********************************************************************************************************** */

gboolean changement_compte ( gint *compte)
{
    GtkWidget *menu;

    /* si on était dans une ventilation d'opération, alors on annule la ventilation */

/*     if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_comptes_equilibrage ) ) == 1 ) */
/* 	annuler_ventilation(); */

    if ( GPOINTER_TO_INT ( compte ) == compte_courant )
	return FALSE;

	/* ferme le formulaire */

	echap_formulaire ();

    /*     si compte = -1, c'est que c'est la 1ère fois qu'on va sur l'onglet */

    if ( GPOINTER_TO_INT ( compte ) != -1 )
    {
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

	VALUE_AJUSTEMENT_LISTE_OPERATIONS = gtk_tree_view_get_vadjustment ( GTK_TREE_VIEW ( TREE_VIEW_LISTE_OPERATIONS )) -> value;

	/*   si on n'est pas sur l'onglet comptes du notebook, on y passe */

	if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_general ) ) != 1 )
	    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ),
				    1 );

	/*     on retire la fleche du classement courant */

	gtk_tree_view_column_set_sort_indicator ( COLONNE_CLASSEMENT,
						  FALSE );

	/*     on cache le tree_view */

	gtk_widget_hide ( SCROLLED_WINDOW_LISTE_OPERATIONS );

    /*     on se place sur les données du nouveau compte */

	compte_courant = GPOINTER_TO_INT ( compte );
    }

    /*     si compte=-1, compte_courant était déjà réglé */
    /* 	sinon on vient juste de le régler */

    p_tab_nom_de_compte_courant = p_tab_nom_de_compte + compte_courant;
    p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

    /* change le nom du compte courant */
    gtk_label_set_text ( GTK_LABEL ( label_compte_courant), NOM_DU_COMPTE);

    /* change les types d'opé et met le défaut */

    if ( (menu = creation_menu_types ( 1, compte_courant, 0  )))
    {
	/* on joue avec les sensitive pour éviter que le 1er mot du menu ne reste grise */

	gtk_widget_set_sensitive ( widget_formulaire_operations[TRANSACTION_FORM_TYPE],
				   TRUE );
	gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_operations[TRANSACTION_FORM_TYPE] ),
				   menu );
	gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_operations[TRANSACTION_FORM_TYPE] ),
				      cherche_no_menu_type ( TYPE_DEFAUT_DEBIT ) );
	gtk_widget_set_sensitive ( widget_formulaire_operations[TRANSACTION_FORM_TYPE],
				   FALSE );
	gtk_widget_show ( widget_formulaire_operations[TRANSACTION_FORM_TYPE] );
    }
    else
    {
	gtk_widget_hide ( widget_formulaire_operations[TRANSACTION_FORM_TYPE] );
	gtk_widget_hide ( widget_formulaire_operations[TRANSACTION_FORM_CHEQUE] );
    }



    if ( DATE_DERNIER_RELEVE )
	gtk_label_set_text ( GTK_LABEL ( label_releve ),
			     g_strdup_printf ( _("Last statement: %02d/%02d/%d"), 
					       g_date_day ( DATE_DERNIER_RELEVE ),
					       g_date_month ( DATE_DERNIER_RELEVE ),
					       g_date_year ( DATE_DERNIER_RELEVE ) ));

    else
	gtk_label_set_text ( GTK_LABEL ( label_releve ),
			     _("Last statement: none") );


    /* affiche le solde final en bas */

    mise_a_jour_labels_soldes ( compte_courant );

    /* change le défaut de l'option menu des devises du formulaire */

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_operations[TRANSACTION_FORM_DEVISE] ),
				  g_slist_index ( liste_struct_devises,
						  devise_par_no ( DEVISE )));

    /* met les boutons R et nb lignes par opé comme il faut */

    mise_a_jour_boutons_caract_liste ( compte_courant );

    /*     on met la flèche sur le classement courant */

    gtk_tree_view_column_set_sort_indicator ( COLONNE_CLASSEMENT,
					      TRUE );
     if ( CLASSEMENT_CROISSANT )
	 gtk_tree_view_column_set_sort_order ( COLONNE_CLASSEMENT,
					       GTK_SORT_ASCENDING );
     else
	 gtk_tree_view_column_set_sort_order ( COLONNE_CLASSEMENT,
					       GTK_SORT_DESCENDING );
     CLASSEMENT_COURANT = recupere_classement_par_no ( NO_CLASSEMENT );


     /*      on termine la liste d'opés si nécessaire */

     verification_list_store_termine ( compte_courant );


    /*     on restore ou initialise la value du tree_view */
    /* 	si VALUE_AJUSTEMENT_LISTE_OPERATIONS = -1, c'est que c'est la première ouverture, on se met tout en bas */
    /* 	sinon on restore l'ancienne value */

    if ( VALUE_AJUSTEMENT_LISTE_OPERATIONS == -1 )
    {
	GtkAdjustment *ajustment;

	ajustment = gtk_tree_view_get_vadjustment ( GTK_TREE_VIEW ( TREE_VIEW_LISTE_OPERATIONS ));
	gtk_adjustment_set_value ( GTK_ADJUSTMENT ( ajustment ),
				   ajustment -> upper - ajustment -> page_size );
    }
    else
	gtk_adjustment_set_value ( GTK_ADJUSTMENT ( gtk_tree_view_get_vadjustment ( GTK_TREE_VIEW ( TREE_VIEW_LISTE_OPERATIONS ))),
				   VALUE_AJUSTEMENT_LISTE_OPERATIONS );

    /*     on affiche le tree_view */

    gtk_widget_show ( SCROLLED_WINDOW_LISTE_OPERATIONS );

    return FALSE;
}
/* ********************************************************************************************************** */



/* *********************************************************************************************************** */
/*   on réaffiche la liste des comptes s'il y a eu un changement */
/* *********************************************************************************************************** */

void reaffiche_liste_comptes ( void )
{
    GSList *ordre_comptes_variable;
    GtkWidget *bouton;
    gint i;

    /* commence par effacer tous les comptes */

    while ( GTK_BOX ( vbox_liste_comptes ) -> children )
	gtk_container_remove ( GTK_CONTAINER ( vbox_liste_comptes ),
			       (( GtkBoxChild *) ( GTK_BOX ( vbox_liste_comptes ) -> children -> data )) -> widget );

    /* on efface les menus des comptes cloturés */

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

    for ( i=0 ; i<nb_comptes ; i++ )
    {
	gchar *tmp;

	tmp = my_strdelimit ( NOM_DU_COMPTE,
			      "/",
			      "\\/" );

	gtk_item_factory_delete_item ( item_factory_menu_general,
				       menu_name(_("Accounts"), _("Closed accounts"), tmp ));
	p_tab_nom_de_compte_variable++;
    }

    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   menu_name(_("Accounts"), _("Closed accounts"), NULL)),
			       FALSE );


    /*  Création d'une icone et du nom par compte, et placement dans la liste selon l'ordre désiré  */

    ordre_comptes_variable = ordre_comptes;
    do
    {
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + GPOINTER_TO_INT ( ordre_comptes_variable->data );

	if ( ! COMPTE_CLOTURE )
	{
	    bouton = comptes_appel( GPOINTER_TO_INT ( ordre_comptes_variable->data ));
	    gtk_box_pack_start (GTK_BOX (vbox_liste_comptes), bouton, FALSE, FALSE, 0);
	    gtk_widget_show (bouton);

	    /* 	    si c'est le compte courant, on ouvre le livre */

	    if ( p_tab_nom_de_compte_variable == p_tab_nom_de_compte + compte_courant )
		gtk_list_button_clicked ( GTK_BUTTON ( bouton ));
	}
	else
	{
	    GtkItemFactoryEntry *item_factory_entry;
	    gchar *tmp;


	    item_factory_entry = calloc ( 1,
					  sizeof ( GtkItemFactoryEntry ));

	    tmp = my_strdelimit ( NOM_DU_COMPTE,
				  "/",
				  "\\/" );
	    tmp = my_strdelimit ( tmp,
				  "_",
				  "__" );

	    item_factory_entry -> path = menu_name(_("Accounts"),  _("Closed accounts"), tmp );
	    item_factory_entry -> callback = G_CALLBACK ( changement_compte_par_menu );

	    /* 	    on rajoute 1 car sinon pour le compte 0 ça passerait pas... */

	    item_factory_entry -> callback_action = GPOINTER_TO_INT ( ordre_comptes_variable->data ) + 1;

	    gtk_item_factory_create_item ( item_factory_menu_general,
					   item_factory_entry,
					   NULL,
					   1 );
	    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
								   menu_name(_("Accounts"), _("Closed accounts"), NULL)),
				       TRUE );

	}

    }
    while ( (  ordre_comptes_variable = ordre_comptes_variable->next ) );

}
/* *********************************************************************************************************** */
