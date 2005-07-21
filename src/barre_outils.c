/* ************************************************************************** */
/* Fichier qui s'occupe de la page d'accueil ( de démarrage lors de           */
/* l'ouverture d'un fichier de comptes                                        */
/*                                                                            */
/*                                  barre_outis.c                             */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2004-2005 Benjamin Drieu (bdrieu@april.org)	      */
/*			1995-1997 Peter Mattis, Spencer Kimball and	      */
/*			          Jsh MacDonald				      */
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
#include "barre_outils.h"
#include "echeancier_liste.h"
#include "operations_liste.h"
#include "equilibrage.h"
#include "gsb_account.h"
#include "utils_str.h"
#include "menu.h"
#include "traitement_variables.h"
#include "utils_buttons.h"
#include "include.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean popup_scheduled_view_mode_menu ( GtkWidget * button );
static gboolean popup_transaction_view_mode_menu ( GtkWidget * button );
/*END_STATIC*/



/** Used to display/hide comments in scheduler list */
GtkWidget *scheduler_display_hide_comments;

GtkWidget *bouton_affiche_cache_formulaire_echeancier;
GtkWidget *bouton_affiche_commentaire_echeancier;
GtkWidget *fleche_bas_echeancier;
GtkWidget *fleche_haut_echeancier;
GtkWidget *bouton_ope_lignes[4];
GtkWidget *bouton_affiche_r;
GtkWidget *bouton_enleve_r;
GtkWidget *bouton_grille;
GtkWidget *bouton_grille_echeancier;

/* widgets du bouton pour afficher/cacher le formulaire */

GtkWidget *bouton_affiche_cache_formulaire;
GtkWidget *fleche_haut;
GtkWidget *fleche_bas;




/*START_EXTERN*/
extern gboolean block_menu_cb ;
extern GtkWidget *bouton_saisir_echeancier;
extern GtkWidget *formulaire;
extern GtkItemFactory *item_factory_menu_general;
extern GtkTooltips *tooltips_general_grisbi;
extern GtkWidget *tree_view_liste_echeances;
/*END_EXTERN*/



/*******************************************************************************************/
GtkWidget *creation_barre_outils ( void )
{
    GtkWidget *handlebox, *hbox, *menu;

    if ( !tooltips_general_grisbi )
	tooltips_general_grisbi = gtk_tooltips_new ();

    /* HandleBox */
    handlebox = gtk_handle_box_new ();

    /* Hbox */
    hbox = gtk_hbox_new ( FALSE, 0 );
    gtk_container_add ( GTK_CONTAINER(handlebox), hbox );

    /* Add various icons */
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 new_button_with_label_and_image ( etat.display_toolbar,
							   _("Transaction"),
							   "new-transaction.png",
							   G_CALLBACK ( new_transaction ),
							   GINT_TO_POINTER(-1) ),
			 FALSE, FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), 
			 new_stock_button_with_label ( etat.display_toolbar,
						       GTK_STOCK_DELETE, 
						       _("Delete"),
						       G_CALLBACK ( remove_transaction ),
						       NULL ), 
			 FALSE, FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), 
			 new_stock_button_with_label ( etat.display_toolbar,
						       GTK_STOCK_PROPERTIES, 
						       _("Edit"),
						       G_CALLBACK ( gsb_transactions_list_edit_current_transaction ),
						       NULL ), 
			 FALSE, FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 new_button_with_label_and_image ( etat.display_toolbar,
							   _("Reconcile"),
							   "reconciliation.png",
							   G_CALLBACK ( equilibrage ),
							   GINT_TO_POINTER(-1) ),
			 FALSE, FALSE, 0 );

    menu = new_stock_button_with_label_menu ( etat.display_toolbar,
					      GTK_STOCK_SELECT_COLOR, _("View"),
					      G_CALLBACK(popup_transaction_view_mode_menu),
					      NULL );
    gtk_box_pack_start ( GTK_BOX(hbox), menu, FALSE, FALSE, 0 );

    gtk_widget_show_all ( handlebox );

    return ( handlebox );
}



/**
 *
 *
 */
gboolean popup_transaction_view_mode_menu ( GtkWidget * button )
{
    GtkWidget *menu, *menu_item;

    menu = gtk_menu_new ();

    menu_item = gtk_menu_item_new_with_label ( _("Simple view") );
    gtk_menu_append ( GTK_MENU ( menu ), menu_item );
    g_signal_connect_swapped ( G_OBJECT(menu_item), "activate", 
			       G_CALLBACK (change_aspect_liste), GINT_TO_POINTER (1) );

    menu_item = gtk_menu_item_new_with_label ( _("Two lines view") );
    gtk_menu_append ( GTK_MENU ( menu ), menu_item );
    g_signal_connect_swapped ( G_OBJECT(menu_item), "activate", 
			       G_CALLBACK (change_aspect_liste), GINT_TO_POINTER (2) );

    menu_item = gtk_menu_item_new_with_label ( _("Three lines view") );
    gtk_menu_append ( GTK_MENU ( menu ), menu_item );
    g_signal_connect_swapped ( G_OBJECT(menu_item), "activate", 
			       G_CALLBACK (change_aspect_liste), GINT_TO_POINTER (3) );

    menu_item = gtk_menu_item_new_with_label ( _("Complete view") );
    gtk_menu_append ( GTK_MENU ( menu ), menu_item );
    g_signal_connect_swapped ( G_OBJECT(menu_item), "activate", 
			       G_CALLBACK (change_aspect_liste), GINT_TO_POINTER (4) );

    gtk_option_menu_set_history ( GTK_OPTION_MENU(menu), 
				  gsb_account_get_nb_rows ( gsb_account_get_current_account () ) );

    gtk_widget_show_all ( menu );
    gtk_menu_popup ( GTK_MENU(menu), NULL, button, set_popup_position, button, 1, 
		     gtk_get_current_event_time());

    return FALSE;
}




/****************************************************************************************************/
gboolean change_aspect_liste ( gint demande )
{
    GtkWidget * widget;
    GSList *list_tmp;

    block_menu_cb = TRUE;

    switch ( demande )
    {
	case 0:
	    /* 	    changement de l'affichage de la grille */

	  etat.affichage_grille = ! etat.affichage_grille;

	    if ( etat.affichage_grille )
	    {
		/* 		on affiche les grilles */

		g_signal_connect_after ( G_OBJECT ( tree_view_liste_echeances ),
					 "expose-event",
					 G_CALLBACK ( affichage_traits_liste_echeances ),
					 NULL );

		list_tmp = gsb_account_get_list_accounts ();

		while ( list_tmp )
		{
		    gint i;

		    i = gsb_account_get_no_account ( list_tmp -> data );

		    g_signal_connect_after ( G_OBJECT ( gsb_transactions_list_get_tree_view()),
					     "expose-event",
					     G_CALLBACK ( affichage_traits_liste_operation ),
					     NULL );

		    list_tmp = list_tmp -> next;
		}
	    }
	    else
	    {
		GSList *list_tmp;

		g_signal_handlers_disconnect_by_func ( G_OBJECT ( tree_view_liste_echeances ),
						       G_CALLBACK ( affichage_traits_liste_echeances ),
						       NULL );

		list_tmp = gsb_account_get_list_accounts ();

		while ( list_tmp )
		{
		    gint i;

		    i = gsb_account_get_no_account ( list_tmp -> data );

		    g_signal_handlers_disconnect_by_func ( G_OBJECT ( gsb_transactions_list_get_tree_view()  ),
							   G_CALLBACK ( affichage_traits_liste_operation ),
							   NULL );

		    list_tmp = list_tmp -> next;
		}
	    }
	    gtk_widget_queue_draw ( gsb_transactions_list_get_tree_view());
	    gtk_widget_queue_draw ( tree_view_liste_echeances );

	    block_menu_cb = TRUE;
	    widget = gtk_item_factory_get_item ( item_factory_menu_general,
						 gsb_string_escape_underscores(menu_name(_("_View"), _("Show grid"), NULL) ) );
	    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), 
					    etat.affichage_grille );
	    block_menu_cb = FALSE;

	    break;

	/* 	1, 2, 3 et 4 sont les nb de lignes qu'on demande à afficher */

	case 1 :
	    widget = gtk_item_factory_get_item ( item_factory_menu_general,
						 gsb_string_escape_underscores( menu_name(_("_View"), _("Show one line per transaction"), NULL) ));
	    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), TRUE );
	    mise_a_jour_affichage_lignes ( demande );
	    modification_fichier ( TRUE );
	    break;
	case 2 :
	    widget = gtk_item_factory_get_item ( item_factory_menu_general,
						 gsb_string_escape_underscores( menu_name(_("_View"), _("Show two lines per transaction"), NULL) ) );
	    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), TRUE );
	    mise_a_jour_affichage_lignes ( demande );
	    modification_fichier ( TRUE );
	    break;
	case 3 :
	    widget = gtk_item_factory_get_item ( item_factory_menu_general,
						 gsb_string_escape_underscores( menu_name(_("_View"), _("Show three lines per transaction"), NULL) ) );
	    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), TRUE );
	    mise_a_jour_affichage_lignes ( demande );
	    modification_fichier ( TRUE );
	    break;
	case 4 :
	    widget = gtk_item_factory_get_item ( item_factory_menu_general,
						 gsb_string_escape_underscores(menu_name(_("_View"), _("Show four lines per transaction"), NULL) ) );
	    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), TRUE );
	    mise_a_jour_affichage_lignes ( demande );
	    modification_fichier ( TRUE );
	    break;

	case 5 :

	    /* ope avec r */

	    mise_a_jour_affichage_r ( 1 );
	    modification_fichier ( TRUE );

	    block_menu_cb = TRUE;
	    widget = gtk_item_factory_get_item ( item_factory_menu_general,
						 gsb_string_escape_underscores( menu_name(_("_View"), _("Show reconciled transactions"), NULL) ) );
	    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), TRUE );
	    block_menu_cb = FALSE;

	    break;

	case 6 :

	    /* ope sans r */

	    mise_a_jour_affichage_r ( 0 );
	    modification_fichier ( TRUE );

	    block_menu_cb = TRUE;
	    widget = gtk_item_factory_get_item ( item_factory_menu_general,
						 gsb_string_escape_underscores( menu_name(_("_View"), _("Show reconciled transactions"), NULL) ) );
	    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), FALSE );
	    block_menu_cb = FALSE;

	    break;
    }

    block_menu_cb = FALSE;

    return ( TRUE );
}



/**
 *
 *
 *
 */
gboolean popup_scheduled_view_mode_menu ( GtkWidget * button )
{
    GtkWidget *menu, *item;
    gchar * names[] = { _("Unique view"), _("Week view"), _("Month view"), 
			_("Two months view"), _("Quarter view"), 
			_("Year view"), _("Custom view"), NULL, };
    int i;

    menu = gtk_menu_new ();
    
    for ( i = 0 ; names[i] ; i++ )
    {
	item = gtk_menu_item_new_with_label ( names[i] );
	gtk_signal_connect_object ( GTK_OBJECT ( item ), "activate",
				    GTK_SIGNAL_FUNC ( gsb_gui_change_scheduler_view ),
				    GINT_TO_POINTER(i) );
	gtk_menu_append ( GTK_MENU ( menu ), item );
    }

    gtk_widget_show_all ( menu );

    gtk_menu_popup ( GTK_MENU(menu), NULL, button, set_popup_position, button, 1, 
		     gtk_get_current_event_time());

    return FALSE;
}



/**
 * Create the toolbar that contains all elements needed to manipulate
 * the scheduler.
 *
 * \return A newly created hbox.
 */
GtkWidget *creation_barre_outils_echeancier ( void )
{
    GtkWidget *bouton, *hbox, *handlebox;

    /* HandleBox + inner hbox */
    handlebox = gtk_handle_box_new ();
    hbox = gtk_hbox_new ( FALSE, 0 );
    gtk_container_add ( GTK_CONTAINER(handlebox), hbox );

    /* Common actions */
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 new_button_with_label_and_image ( etat.display_toolbar,
							   _("_New transaction"),
							   "new-scheduled.png",
							   G_CALLBACK ( new_scheduled_transaction ),
							   GINT_TO_POINTER(-1) ),
			 FALSE, FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), 
			 new_stock_button_with_label ( etat.display_toolbar,
						       GTK_STOCK_DELETE, 
						       _("Delete"),
						       G_CALLBACK ( supprime_echeance ),
						       NULL ), 
			 FALSE, FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), 
			 new_stock_button_with_label ( etat.display_toolbar,
						       GTK_STOCK_PROPERTIES, 
						       _("Edit"),
						       G_CALLBACK ( edition_echeance ),
						       NULL ), 
			 FALSE, FALSE, 0 );

    /* Display/hide comments */
    scheduler_display_hide_comments = new_button_with_label_and_image ( etat.display_toolbar,
									_("Comments"),
									"comments.png",
									G_CALLBACK ( affiche_cache_commentaire_echeancier ),
									0 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), scheduler_display_hide_comments, 
			 FALSE, FALSE, 0 );

    /* Execute transaction */
    bouton_saisir_echeancier = new_stock_button_with_label ( etat.display_toolbar,
							     GTK_STOCK_EXECUTE, 
							     _("Execute"),
							     G_CALLBACK ( click_sur_saisir_echeance ),
							     NULL ); 
    gtk_box_pack_start ( GTK_BOX ( hbox ), bouton_saisir_echeancier, FALSE, FALSE, 0 );

    bouton = new_stock_button_with_label_menu ( etat.display_toolbar,
						GTK_STOCK_SELECT_COLOR, _("View"),
						G_CALLBACK(popup_scheduled_view_mode_menu),
						NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ), bouton, FALSE, FALSE, 0 );

    gtk_widget_show_all ( handlebox );

    return ( handlebox );
}
/*******************************************************************************************/

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
