/*  Fichier qui gère la liste des no_accounts, la partie gauche de l'onglet opérations */
/*      operations_no_accounts.c */

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


/*START_INCLUDE*/
#include "operations_comptes.h"
#include "equilibrage.h"
#include "gsb_account.h"
#include "comptes_onglet.h"
#include "operations_liste.h"
#include "gtk_list_button.h"
#include "menu.h"
#include "barre_outils.h"
#include "traitement_variables.h"
#include "utils_str.h"
#include "comptes_traitements.h"
#include "operations_formulaire.h"
#include "utils_comptes.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void changement_no_compte_par_menu ( gpointer null,
				     gint no_account_plus_un );
static gboolean gsb_account_list_gui_change_order ( GtkWidget *button );
static void verifie_no_account_clos ( gint no_nouveau_no_account );
/*END_STATIC*/


/* adresse de la vbox contenant les icones de no_accounts */

GtkWidget *vbox_liste_comptes;

/* adr du label du dernier relevé */

GtkWidget *label_last_statement;

GtkWidget *label_name_current_account;

/*START_EXTERN*/
extern GtkWidget *formulaire;
extern GtkItemFactory *item_factory_menu_general;
extern gchar *last_date;
extern gint mise_a_jour_liste_comptes_accueil;
extern gint nb_colonnes;
extern GtkWidget *notebook_general;
extern GtkWidget *tree_view;
/*END_EXTERN*/




/* ********************************************************************************************************** */
/*** Création de la fenêtre de no_accounts ***/
/* **************************************************************************************************** */

GtkWidget *creation_liste_comptes (void)
{
    GtkWidget *onglet;
    GtkWidget *frame_label_name_current_account;
    GtkWidget *button;
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
			 GTK_SIGNAL_FUNC ( gsb_transactions_list_key_press ),
			 NULL );
    gtk_widget_show ( onglet );

    /*  Création du label Comptes en haut */


    /*   on place le label dans une frame */

    frame_label_name_current_account = gtk_frame_new ( NULL );
    gtk_frame_set_shadow_type ( GTK_FRAME ( frame_label_name_current_account ),
				GTK_SHADOW_ETCHED_OUT );
    gtk_box_pack_start ( GTK_BOX (onglet),
			 frame_label_name_current_account,
			 FALSE,
			 TRUE,
			 0);
    gtk_widget_show (frame_label_name_current_account);


    /*   on ne met rien dans le label, il sera rempli ensuite */

    label_name_current_account = gtk_label_new ( "" );
    gtk_misc_set_alignment ( GTK_MISC (label_name_current_account  ),
			     0.5,
			     0.5);
    gtk_container_add ( GTK_CONTAINER ( frame_label_name_current_account ),
			label_name_current_account );
    gtk_widget_show (label_name_current_account);


    /*  Création de la fenêtre des no_accounts */
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


    /*  création d'une vbox contenant la liste des no_accounts */

    vbox_liste_comptes = gtk_vbox_new ( FALSE,
					0);
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW (scrolled_window ),
					    vbox_liste_comptes);
    gtk_viewport_set_shadow_type ( GTK_VIEWPORT ( GTK_BIN ( scrolled_window )  -> child ),
				   GTK_SHADOW_NONE );
    gtk_widget_show (vbox_liste_comptes);


    /*  Création d'une icone et du nom par no_account, et placement dans la
	liste selon l'ordre désiré  */
    if ( gsb_account_get_accounts_amount () )
    {
	gsb_account_list_gui_create_list ();
    }

    /* ajoute le button et le label pour l'équilibrage de no_account */
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

    if ( gsb_account_get_accounts_amount ())
    {
	gint current_account;
	GDate *reconcile_date;

	current_account = gsb_account_get_current_account ();
	reconcile_date = gsb_account_get_current_reconcile_date (current_account);
    }

    /* mise en place du button équilibrage */

    button = gtk_button_new_with_label ( _("Reconcile") );
    gtk_button_set_relief ( GTK_BUTTON ( button ),
			    GTK_RELIEF_NONE);
    gtk_box_pack_start ( GTK_BOX ( vbox_frame_equilibrage ),
			 button,
			 FALSE,
			 TRUE,
			 0);
    gtk_signal_connect ( GTK_OBJECT (button),
			 "clicked",
			 GTK_SIGNAL_FUNC ( equilibrage ),
			 NULL );
    gtk_widget_show ( button );


    return ( onglet );

}
/* ********************************************************************************************************** */




/** return a button with an icon and the name of the account
 * \param no_account
 * \param group 1 for the transactions list, 2 for the account page
 * \param callback function to call when clicked
 * \return a GtkButton
 * */
GtkWidget *gsb_account_list_gui_create_account_button ( gint no_account,
							gint group,
							gpointer callback )
{
    GtkWidget *button;

    button = gtk_list_button_new ( gsb_account_get_name (no_account), group, TRUE, GINT_TO_POINTER (no_account));

    if ( group == 1 )
	gsb_account_set_account_button( no_account,
					button );

    g_signal_connect_swapped ( G_OBJECT (button),
			       "clicked",
			       G_CALLBACK ( callback ),
			       GINT_TO_POINTER ( no_account ) );
    g_signal_connect ( G_OBJECT ( button ),
		       "reordered",
		       G_CALLBACK ( gsb_account_list_gui_change_order ),
		       NULL );
    gtk_widget_show ( button );

    return ( button );
}
/* ********************************************************************************************************** */


/* ********************************************************************************************************** */
void changement_no_compte_par_menu ( gpointer null,
				     gint no_account_plus_un )
{
    gsb_account_list_gui_change_current_account ( GINT_TO_POINTER ( no_account_plus_un - 1) );
}
/* ********************************************************************************************************** */




/** change the list of transactions, according to the new account
 * \param no_account the number of the account we want to see
 * \return FALSE
 * */
gboolean gsb_account_list_gui_change_current_account ( gint *no_account )
{
    gint new_account;

    new_account = GPOINTER_TO_INT ( no_account );

    if ( DEBUG )
	printf ( "gsb_account_list_gui_change_current_account : %d\n", new_account );


    /*   go to the transactions list page */

    if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_general ) ) != 1 )
	gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ),
				1 );

    /* Removed because it prevents updating tab at first selection,
     * because saved account is ALWAYS the same as the current one. */
/*     if ( new_account == gsb_account_get_current_account () ) */
/* 	return FALSE; */

    /* 	on va sur un no_account, on vérifie que ce n'est pas un no_account clos */
    /* 	    si c'est le cas, on ferme l'icone */

    verifie_no_account_clos ( new_account);

    /*     on cache le tree_view */

    gtk_widget_hide ( gsb_account_get_scrolled_window (gsb_account_get_current_account ()) );

    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   menu_name(_("Edit"),
								     _("Move transaction to another account"),
								     my_strdelimit ( gsb_account_get_name (gsb_account_get_current_account ()),
										     "/",
										     "\\/" ))),
			       TRUE );

    /*     on se place sur les données du nouveau no_account */

    gsb_account_set_current_account ( new_account);

    /* change le nom du no_account courant */

    gtk_label_set_text ( GTK_LABEL ( label_name_current_account), gsb_account_get_name (new_account));

    /*     affiche le nouveau formulaire  */
    /*     il met aussi à jour la devise courante et les types */

    remplissage_formulaire ( new_account );


    /*     mise en place de la date du dernier relevé */

    if ( gsb_account_get_current_reconcile_date (new_account) )
	gtk_label_set_text ( GTK_LABEL ( label_last_statement ),
			     g_strdup_printf ( _("Last statement: %02d/%02d/%d"), 
					       g_date_day ( gsb_account_get_current_reconcile_date (new_account)),
					       g_date_month ( gsb_account_get_current_reconcile_date (new_account)),
					       g_date_year ( gsb_account_get_current_reconcile_date (new_account))));

    else
	gtk_label_set_text ( GTK_LABEL ( label_last_statement ),
			     _("Last statement: none") );


    /* affiche le solde final en bas */

    mise_a_jour_labels_soldes ();

    /* met les boutons R et nb lignes par opé comme il faut */

    mise_a_jour_boutons_caract_liste ( new_account );

    /*      on termine la liste d'opés si nécessaire */

    verification_list_store_termine ( new_account );

    /*     on affiche le tree_view */

    gtk_widget_show ( gsb_account_get_scrolled_window (new_account) );

    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   menu_name(_("Edit"),
								     _("Move transaction to another account"),
								     my_strdelimit ( gsb_account_get_name (new_account),
										     "/",
										     "\\/" ))),
			       FALSE );

    /* unset the last date written */

    last_date = NULL;

    return FALSE;
}
/* ********************************************************************************************************** */


/* ********************************************************************************************************** */
/* cette fonction est appelée lors d'un changement de no_account */
/* cherche si le nouveau no_account est clos, si c'est le cas, ferme l'icone du no_account courant */
/* ********************************************************************************************************** */
void verifie_no_account_clos ( gint no_nouveau_no_account )
{
    /*     si le no_account courant est déjà cloturé, on fait rien */

    if ( gsb_account_get_closed_account (gsb_account_get_current_account ()) )
	return;

    if ( gsb_account_get_closed_account (no_nouveau_no_account) )
    {
	gtk_list_button_close ( GTK_BUTTON ( gsb_account_get_account_button (gsb_account_get_current_account ()) ));
    }
}
/* ********************************************************************************************************** */




/** erase and create the clickable list of accounts, on the left of the transactions list
 * \param none
 * \return FALSE;
 * */
gboolean gsb_account_list_gui_create_list ( void )
{
    GSList *list_tmp;

    if ( DEBUG )
	printf ( "gsb_account_list_gui_create_list\n" );

    /*     erase the normal accounts */

    while ( GTK_BOX ( vbox_liste_comptes ) -> children )
	gtk_container_remove ( GTK_CONTAINER ( vbox_liste_comptes ),
			       (( GtkBoxChild *) ( GTK_BOX ( vbox_liste_comptes ) -> children -> data )) -> widget );

    /* erase the closed accounts and accounts in the menu to move a transaction */

    list_tmp = gsb_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;
	gchar *tmp;

	i = gsb_account_get_no_account ( list_tmp -> data );

	tmp = my_strdelimit ( gsb_account_get_name (i),
			      "/",
			      "\\/" );

	if ( gsb_account_get_closed_account ( i ))
	    gtk_item_factory_delete_item ( item_factory_menu_general,
					   menu_name(_("Accounts"), _("Closed accounts"), tmp ));
	else
	    gtk_item_factory_delete_item ( item_factory_menu_general,
					   menu_name(_("Edit"), _("Move transaction to another account"), tmp ));

	list_tmp = list_tmp -> next;
    }

    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   menu_name(_("Accounts"), _("Closed accounts"), NULL)),
			       FALSE );
    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   menu_name(_("Edit"), _("Move transaction to another account"), NULL)),
			       FALSE );


    /* create the list : a button and an icon for each account */

    list_tmp = gsb_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;
	GtkItemFactoryEntry *item_factory_entry;
	gchar *tmp;


	i = gsb_account_get_no_account ( list_tmp -> data );

	if ( gsb_account_get_closed_account (i))
	{
	    item_factory_entry = calloc ( 1,
					  sizeof ( GtkItemFactoryEntry ));

	    tmp = my_strdelimit ( gsb_account_get_name (i),
				  "/",
				  "\\/" );
	    tmp = my_strdelimit ( tmp,
				  "_",
				  "__" );

	    item_factory_entry -> path = menu_name(_("Accounts"),  _("Closed accounts"), tmp );
	    item_factory_entry -> callback = G_CALLBACK ( changement_no_compte_par_menu );

	    /* 	    on rajoute 1 car sinon pour le no_account 0 ça passerait pas... */

	    item_factory_entry -> callback_action = i + 1;

	    gtk_item_factory_create_item ( item_factory_menu_general,
					   item_factory_entry,
					   NULL,
					   1 );
	    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
								   menu_name(_("Accounts"), _("Closed accounts"), NULL)),
				       TRUE );
	}
	else
	{
	    GtkWidget *button;

	    button = gsb_account_list_gui_create_account_button (i,
								 1,
								 G_CALLBACK ( gsb_account_list_gui_change_current_account ));
	    gtk_box_pack_start (GTK_BOX (vbox_liste_comptes), button, FALSE, FALSE, 0);
	    gtk_widget_show (button);

	    /* 	we put all the accounts in the edition menu */

	    item_factory_entry = calloc ( 1, sizeof ( GtkItemFactoryEntry ));

	    tmp = my_strdelimit ( gsb_account_get_name (i), "/", "\\/" );

	    item_factory_entry -> path = menu_name(_("Edit"), 
						   _("Move transaction to another account"), 
						   my_strdelimit ( tmp, "_", "__" ) ); 

	    item_factory_entry -> callback = G_CALLBACK ( move_selected_operation_to_account_nb );

	    /* 	    on rajoute 1 car sinon pour le no_account 0 ça passerait pas... */

	    item_factory_entry -> callback_action = i + 1;

	    gtk_item_factory_create_item ( item_factory_menu_general,
					   item_factory_entry,
					   GINT_TO_POINTER (i),
					   1 );

	    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
								   menu_name(_("Edit"), _("Move transaction to another account"), NULL)),
				       TRUE );

	    /* 	    if it's the current account, we open the book and non-sensitive the menu entry */

	    if ( i == gsb_account_get_current_account () )
	    {
		/* open the book */

		gtk_list_button_clicked ( GTK_BUTTON ( button ));

		/* set the name of the account */

		gtk_label_set_text ( GTK_LABEL ( label_name_current_account),
				     gsb_account_get_name (i));

		/* un-sensitive the name of account in the menu */

		gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
								       menu_name(_("Edit"), _("Move transaction to another account"), tmp)),
					   FALSE );
	    }
	}
	list_tmp = list_tmp -> next;
    }
    return FALSE;
}
/* *********************************************************************************************************** */



/** called when the order of accounts changed
 * \param button
 * \return FALSE
 * */
gboolean gsb_account_list_gui_change_order ( GtkWidget *button )
{
    GSList *new_list_order;
    GList *list_buttons_accounts;

    list_buttons_accounts = GTK_BOX ( button-> parent ) -> children;
    new_list_order = NULL;

    while ( list_buttons_accounts )
    {
	GtkBoxChild *box_child;

	box_child = list_buttons_accounts -> data;

	new_list_order = g_slist_append ( new_list_order,
					  gtk_list_button_get_data ( GTK_LIST_BUTTON ( box_child -> widget )));
	list_buttons_accounts = list_buttons_accounts -> next;
    }

    gsb_account_reorder ( new_list_order );
    g_slist_free (new_list_order);

    /* we remake the other accounts list */

    if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_general )) == 1 ) 
	gsb_account_page_create_accounts_list ();
    else
	gsb_account_list_gui_create_list ();

    mise_a_jour_liste_comptes_accueil = 1;

    update_options_menus_comptes ();
    modification_fichier (TRUE);

    return ( FALSE );
}
/* *********************************************************************************************************** */




/******************************************************************************/
/* règle la taille des widgets dans le formulaire des opés en fonction */
/* des paramètres */
/******************************************************************************/
void mise_a_jour_taille_formulaire ( gint largeur_formulaire )
{

    gint i, j;
    struct organisation_formulaire *organisation_formulaire;

    if ( !largeur_formulaire )
	return;

    organisation_formulaire = renvoie_organisation_formulaire ();

    for ( i=0 ; i < organisation_formulaire -> nb_lignes ; i++ )
	for ( j=0 ; j < organisation_formulaire -> nb_colonnes ; j++ )
	{
	    GtkWidget *widget;

	    widget = widget_formulaire_par_element ( organisation_formulaire -> tab_remplissage_formulaire[i][j] );

	    if ( widget )
		gtk_widget_set_usize ( widget,
				       organisation_formulaire -> taille_colonne_pourcent[j] * largeur_formulaire / 100,
				       FALSE );
	}
}
/******************************************************************************/




/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
