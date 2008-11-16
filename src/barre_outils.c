/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cédric Auger (cedric@grisbi.org)	      */
/*			2004-2008 Benjamin Drieu (bdrieu@april.org)	      */
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

/**
 * \file barre_outils.c
 * set up the toolbar
 */


#include "include.h"

/*START_INCLUDE*/
#include "barre_outils.h"
#include "./gsb_automem.h"
#include "./gsb_data_account.h"
#include "./gsb_data_import_rule.h"
#include "./navigation.h"
#include "./menu.h"
#include "./import.h"
#include "./gsb_reconcile.h"
#include "./gsb_scheduler_list.h"
#include "./gsb_transactions_list.h"
#include "./traitement_variables.h"
#include "./print_transactions_list.h"
#include "./utils_buttons.h"
#include "./structures.h"
#include "./include.h"
/*END_INCLUDE*/

/*START_STATIC*/
static GtkWidget *creation_barre_outils ( void );
static gboolean popup_scheduled_view_mode_menu ( GtkWidget * button );
static gboolean popup_transaction_rules_menu ( GtkWidget * button,
					gpointer null );
static  gboolean popup_transaction_view_mode_menu ( GtkWidget * button,
						   gpointer null );
/*END_STATIC*/


/** Used to display/hide comments in scheduler list */
static GtkWidget *scheduler_display_hide_comments = NULL;

/* display/hide marked transactions */
static GtkWidget * bouton_affiche_ope_r = NULL;

/** here are the 3 buttons on the scheduler toolbar
 * which can been unsensitive or sensitive */
GtkWidget *scheduler_button_execute = NULL;
GtkWidget *scheduler_button_delete = NULL;
GtkWidget *scheduler_button_edit = NULL;


/** the import rules button is showed or hidden if account have or no some rules
 * so need to set in global variables */
GtkWidget *menu_import_rules;


/*START_EXTERN*/
extern GtkWidget *barre_outils;
extern gboolean block_menu_cb ;
extern GtkUIManager * ui_manager;
/*END_EXTERN*/



/*******************************************************************************************/
GtkWidget *creation_barre_outils ( void )
{
    GtkWidget *hbox, *menu, *button;

    /* Hbox */
    hbox = gtk_hbox_new ( FALSE, 0 );

    /* Add various icons */
    button = gsb_automem_imagefile_button_new ( etat.display_toolbar,
					       _("New transaction"),
					       "new-transaction.png",
					       G_CALLBACK ( new_transaction ),
					       GINT_TO_POINTER(-1) );
    gtk_widget_set_tooltip_text ( GTK_WIDGET (button),
				  SPACIFY(_("Blank the form to create a new transaction")));
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 0 );

    button = gsb_automem_stock_button_new ( etat.display_toolbar,
					   GTK_STOCK_DELETE, 
					   _("Delete"),
					   G_CALLBACK ( remove_transaction ),
					   NULL );
    gtk_widget_set_tooltip_text ( GTK_WIDGET (button),
				  SPACIFY(_("Delete selected transaction")));
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 0 );

    button = gsb_automem_stock_button_new ( etat.display_toolbar,
					   GTK_STOCK_PROPERTIES, 
					   _("Edit"),
					   G_CALLBACK ( gsb_transactions_list_edit_current_transaction ),
					   NULL );
    gtk_widget_set_tooltip_text ( GTK_WIDGET (button),
				  SPACIFY(_("Edit current transaction")));
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 0 );

    button = gsb_automem_imagefile_button_new ( etat.display_toolbar,
					       _("Reconcile"),
					       "reconciliation.png",
					       G_CALLBACK (gsb_reconcile_run_reconciliation),
					       GINT_TO_POINTER(-1) );
    gtk_widget_set_tooltip_text ( GTK_WIDGET (button),
				  SPACIFY(_("Start account reconciliation")));
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 0 );

#if GTK_CHECK_VERSION(2,10,0)
    /* This stuff needs GTK+ 2.10 to work. */
    button = gsb_automem_stock_button_new ( etat.display_toolbar,
					    GTK_STOCK_PRINT,
					    _("Print"),
					    G_CALLBACK (print_transactions_list),
					    NULL );
    gtk_widget_set_tooltip_text ( GTK_WIDGET (button),
				  SPACIFY(_("Print the transactions list")));
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 0 );
#endif /* GTK_CHECK_VERSION(2,10,0) */

    menu = gsb_automem_stock_button_menu_new ( etat.display_toolbar,
					      GTK_STOCK_SELECT_COLOR, _("View"),
					      G_CALLBACK(popup_transaction_view_mode_menu),
					      NULL );
    gtk_widget_set_tooltip_text ( GTK_WIDGET (menu),
				  SPACIFY(_("Change display mode of the list")));
    gtk_box_pack_start ( GTK_BOX(hbox), menu, FALSE, FALSE, 0 );

    /* set the button to show/hide R transactions */
/*     if ( bouton_affiche_ope_r && GTK_IS_WIDGET ( bouton_affiche_ope_r ) ) */
/* 	    gtk_box_pack_start ( GTK_BOX ( hbox ), bouton_affiche_ope_r, FALSE, FALSE, 0 ); */


    menu_import_rules = gsb_automem_stock_button_menu_new ( etat.display_toolbar,
							    GTK_STOCK_EXECUTE, _("Import rules"),
							    G_CALLBACK(popup_transaction_rules_menu),
							    NULL );
    gtk_widget_set_tooltip_text ( GTK_WIDGET (menu_import_rules),
				  SPACIFY(_("Quick file import by rules")));
    gtk_box_pack_start ( GTK_BOX(hbox), menu_import_rules, FALSE, FALSE, 0 );

    gtk_widget_show_all ( hbox );

    if ( gsb_data_import_rule_account_has_rule ( gsb_gui_navigation_get_current_account ( ) ) )
	    gtk_widget_show ( menu_import_rules );
    else
	    gtk_widget_hide ( menu_import_rules );

    return ( hbox );
}



/**
 *
 *
 *
 */
void gsb_gui_update_transaction_toolbar ( void )
{
    GList * list = NULL;

    list = gtk_container_get_children ( GTK_CONTAINER ( barre_outils ) );
    
    if ( list )
    {
	gtk_container_remove ( GTK_CONTAINER ( barre_outils ),
			       GTK_WIDGET ( list -> data ) );
	g_list_free ( list );
    }
    gtk_container_add ( GTK_CONTAINER ( barre_outils ), creation_barre_outils () );
}



/**
 *
 *
 */
static gboolean popup_transaction_view_mode_menu ( GtkWidget * button,
						   gpointer null )
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

    gtk_menu_append ( GTK_MENU ( menu ), gtk_separator_menu_item_new ( ) );

    menu_item = gtk_check_menu_item_new_with_label ( _("Display reconciled transactions") );
    gtk_check_menu_item_set_active ( menu_item,
				     gsb_data_account_get_r ( gsb_gui_navigation_get_current_account () ) );
    gtk_menu_append ( GTK_MENU ( menu ), menu_item );
    g_signal_connect_swapped ( G_OBJECT(menu_item), "activate", 
			       G_CALLBACK (gsb_gui_toggle_show_reconciled), GINT_TO_POINTER (4) );

    gtk_menu_set_active ( GTK_MENU(menu), 
			  gsb_data_account_get_nb_rows ( gsb_gui_navigation_get_current_account () ) );

    gtk_widget_show_all ( menu );
    gtk_menu_popup ( GTK_MENU(menu), NULL, button, set_popup_position, button, 1, 
		     gtk_get_current_event_time());

    return FALSE;
}

/**
 *
 *
 */
gboolean popup_transaction_rules_menu ( GtkWidget * button,
					gpointer null )
{
    GtkWidget *menu, *menu_item;
    GSList *tmp_list;
    gint current_account = gsb_gui_navigation_get_current_account ();
    gint i = 0;

    menu = gtk_menu_new ();

    tmp_list = gsb_data_import_rule_get_from_account (current_account);
    
    while (tmp_list)
    {
	gint rule;

	rule = gsb_data_import_rule_get_number (tmp_list -> data);
 
	if (i > 0)
	{ 
	    menu_item = gtk_separator_menu_item_new ( );
	    gtk_menu_append ( GTK_MENU ( menu ), menu_item );
	}
  
	menu_item = gtk_menu_item_new_with_label (gsb_data_import_rule_get_name (rule));
	gtk_menu_append ( GTK_MENU ( menu ), menu_item );
	g_signal_connect_swapped ( G_OBJECT(menu_item), "activate", 
				   G_CALLBACK (gsb_import_by_rule), GINT_TO_POINTER (rule) );
	menu_item = gtk_menu_item_new_with_label (_("Remove the rule"));
	g_signal_connect_swapped ( G_OBJECT(menu_item), "activate", 
				   G_CALLBACK (gsb_data_import_rule_remove), GINT_TO_POINTER (rule) );

	gtk_menu_append ( GTK_MENU ( menu ), menu_item );

	tmp_list = tmp_list -> next;
    }

    gtk_widget_show_all ( menu );
    gtk_menu_popup ( GTK_MENU(menu), NULL, button, set_popup_position, button, 1, 
		     gtk_get_current_event_time());

    return FALSE;
}




/****************************************************************************************************/
gboolean change_aspect_liste ( gint demande )
{
    switch ( demande )
    {
	case 0:
	    /* not used */
	    break;

	/* 	1, 2, 3 et 4 sont les nb de lignes qu'on demande à afficher */

	case 1 :
	    gtk_toggle_action_set_active ( GTK_TOGGLE_ACTION (gtk_ui_manager_get_action ( ui_manager, 
											  menu_name ( "ViewMenu", "ShowOneLine", NULL ) ) ), 
					   TRUE );
	    gsb_transactions_list_set_visible_rows_number ( demande );
	    modification_fichier ( TRUE );
	    break;
	case 2 :
	    gtk_toggle_action_set_active ( GTK_TOGGLE_ACTION (gtk_ui_manager_get_action ( ui_manager, 
											  menu_name ( "ViewMenu", "ShowTwoLines", NULL ) ) ), 
					   TRUE );
	    gsb_transactions_list_set_visible_rows_number ( demande );
	    modification_fichier ( TRUE );
	    break;
	case 3 :
	    gtk_toggle_action_set_active ( GTK_TOGGLE_ACTION (gtk_ui_manager_get_action ( ui_manager, 
											  menu_name ( "ViewMenu", "ShowThreeLines", NULL ) ) ), 
					   TRUE );
	    gsb_transactions_list_set_visible_rows_number ( demande );
	    modification_fichier ( TRUE );
	    break;
	case 4 :
	    gtk_toggle_action_set_active ( GTK_TOGGLE_ACTION (gtk_ui_manager_get_action ( ui_manager, 
											  menu_name ( "ViewMenu", "ShowFourLines", NULL ) ) ), 
					   TRUE );
	    gsb_transactions_list_set_visible_rows_number ( demande );
	    modification_fichier ( TRUE );
	    break;

	case 5 :

	    /* ope avec r */

	    mise_a_jour_affichage_r ( 1 );
	    modification_fichier ( TRUE );

	    block_menu_cb = TRUE;
	    gtk_toggle_action_set_active ( GTK_TOGGLE_ACTION (gtk_ui_manager_get_action ( ui_manager, 
											  menu_name ( "ViewMenu", "ShowReconciled", NULL ) ) ), 
					   TRUE );
	    block_menu_cb = FALSE;

	    break;

	case 6 :

	    /* ope sans r */

	    mise_a_jour_affichage_r ( 0 );
	    modification_fichier ( TRUE );

	    block_menu_cb = TRUE;
	    gtk_toggle_action_set_active ( GTK_TOGGLE_ACTION (gtk_ui_manager_get_action ( ui_manager, 
											  menu_name ( "ViewMenu", "ShowReconciled", NULL ) ) ), 
					   FALSE );
	    block_menu_cb = FALSE;

	    break;
    }

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
				    GTK_SIGNAL_FUNC ( gsb_scheduler_list_change_scheduler_view ),
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
 * \param 
 *
 * \return A newly created hbox.
 */
GtkWidget *creation_barre_outils_echeancier ( void )
{
    GtkWidget *hbox, *handlebox, *button;

    /* HandleBox + inner hbox */
    handlebox = gtk_handle_box_new ();
    hbox = gtk_hbox_new ( FALSE, 0 );
    gtk_container_add ( GTK_CONTAINER(handlebox), hbox );

    /* Common actions */
    button = gsb_automem_imagefile_button_new ( etat.display_toolbar,
					       _("_New scheduled"),
					       "new-scheduled.png",
					       G_CALLBACK (gsb_scheduler_list_edit_transaction),
					       GINT_TO_POINTER(-1) );
    gtk_widget_set_tooltip_text ( GTK_WIDGET (button),
				  SPACIFY(_("Prepare form to create a new scheduled transaction")));
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 0 );

    scheduler_button_delete = gsb_automem_stock_button_new ( etat.display_toolbar,
							    GTK_STOCK_DELETE, 
							    _("Delete"),
							    G_CALLBACK ( gsb_scheduler_list_delete_scheduled_transaction_by_menu ),
							    NULL );
    g_signal_connect ( G_OBJECT (scheduler_button_delete ), "destroy",
		       G_CALLBACK ( gtk_widget_destroyed), &scheduler_button_delete );
    gtk_widget_set_sensitive ( scheduler_button_delete,
			       FALSE );
    gtk_widget_set_tooltip_text ( GTK_WIDGET (scheduler_button_delete),
				  SPACIFY(_("Delete selected scheduled transaction")));
    gtk_box_pack_start ( GTK_BOX ( hbox ), scheduler_button_delete, FALSE, FALSE, 0 );

    scheduler_button_edit = gsb_automem_stock_button_new ( etat.display_toolbar,
							  GTK_STOCK_PROPERTIES, 
							  _("Edit"),
							  G_CALLBACK ( gsb_scheduler_list_edit_transaction ),
							  0 );
    g_signal_connect ( G_OBJECT (scheduler_button_edit ), "destroy",
    		G_CALLBACK ( gtk_widget_destroyed), &scheduler_button_edit );
    gtk_widget_set_sensitive ( scheduler_button_edit,
			       FALSE );
    gtk_widget_set_tooltip_text ( GTK_WIDGET (scheduler_button_edit),
				SPACIFY(_("Edit selected transaction")));
    gtk_box_pack_start ( GTK_BOX ( hbox ), scheduler_button_edit, FALSE, FALSE, 0 );

    /* Display/hide comments */
    scheduler_display_hide_comments = gsb_automem_imagefile_button_new ( etat.display_toolbar,
									_("Comments"),
									"comments.png",
									G_CALLBACK ( gsb_scheduler_list_show_notes ),
									0 );
    g_signal_connect ( G_OBJECT (scheduler_display_hide_comments ), "destroy",
		       G_CALLBACK ( gtk_widget_destroyed), &scheduler_display_hide_comments );
    gtk_widget_set_tooltip_text ( GTK_WIDGET (scheduler_display_hide_comments),
				  SPACIFY(_("Display scheduled transactions comments")));
    gtk_box_pack_start ( GTK_BOX ( hbox ), scheduler_display_hide_comments, 
			 FALSE, FALSE, 0 );

    /* Execute transaction */
    scheduler_button_execute = gsb_automem_stock_button_new ( etat.display_toolbar,
							     GTK_STOCK_EXECUTE, 
							     _("Execute"),
							     G_CALLBACK ( gsb_scheduler_list_execute_transaction ),
							     NULL ); 
    g_signal_connect ( G_OBJECT (scheduler_button_execute ), "destroy",
    		G_CALLBACK ( gtk_widget_destroyed), &scheduler_button_execute );
    gtk_widget_set_sensitive ( scheduler_button_execute,
			       FALSE );
    gtk_widget_set_tooltip_text ( GTK_WIDGET (scheduler_button_execute),
				  SPACIFY(_("Execute current scheduled transaction")));
    gtk_box_pack_start ( GTK_BOX ( hbox ), scheduler_button_execute, FALSE, FALSE, 0 );

    button = gsb_automem_stock_button_menu_new ( etat.display_toolbar,
						GTK_STOCK_SELECT_COLOR, _("View"),
						G_CALLBACK(popup_scheduled_view_mode_menu),
						NULL );
    gtk_widget_set_tooltip_text ( GTK_WIDGET (button),
				  SPACIFY(_("Change display mode of scheduled transaction list")));
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 0 );

    gtk_widget_show_all ( handlebox );

    return ( handlebox );
}




/* ajouté pour la gestion des boutons afficher/masquer les opérations rapprochées */
void gsb_gui_update_bouton_affiche_ope_r ( gboolean show_r )
{
    if (show_r)
    {
	bouton_affiche_ope_r = gsb_automem_imagefile_button_new ( etat.display_toolbar,
								  _("Mask reconcile"),
								  "hide_r.png",
								  G_CALLBACK (gsb_gui_toggle_show_reconciled),
								  NULL );
	gtk_widget_set_tooltip_text ( GTK_WIDGET (bouton_affiche_ope_r),
				      SPACIFY(_("Mask reconciled transactions")));
    }
    else
    {
	bouton_affiche_ope_r = gsb_automem_imagefile_button_new ( etat.display_toolbar,
								  _("Display reconcile"),
								  "show_r.png",
								  G_CALLBACK (gsb_gui_toggle_show_reconciled),
								  NULL );
	gtk_widget_set_tooltip_text ( GTK_WIDGET (bouton_affiche_ope_r),
				      SPACIFY(_("Display reconciled transactions")));
    }
    gsb_gui_update_transaction_toolbar ( );

}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
