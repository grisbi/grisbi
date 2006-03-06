/* ************************************************************************** */
/* Ce fichier contient les définitions de tous les menus et barres d'outils   */
/*                                                                            */
/*                                  menu.c                                    */
/*                                                                            */
/*     Copyright (C)	2000-2006 Cédric Auger (cedric@grisbi.org)	      */
/*			2004-2006 Benjamin Drieu (bdrieu@april.org)	      */
/*			http://www.grisbi.org				      */
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
#include "menu.h"
#include "help.h"
#include "gsb_scheduler_list.h"
#include "gsb_transactions_list.h"
#include "gsb_form_transaction.h"
#include "barre_outils.h"
#include "comptes_traitements.h"
#include "erreur.h"
#include "fichiers_gestion.h"
#include "export.h"
#include "tip.h"
#include "gsb_data_account.h"
#include "gsb_file_debug.h"
#include "navigation.h"
#include "import.h"
#include "utils.h"
#include "parametres.h"
#include "menu.h"
#include "include.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean gsb_gui_toggle_grid_mode ();
static void gsb_gui_toggle_line_view_mode ( GtkRadioAction * action, GtkRadioAction *current, 
				     gpointer user_data );
static gboolean gsb_gui_toggle_show_reconciled ();
static gboolean help_bugreport ();
static gboolean help_manual ();
static gboolean help_quick_start ();
static gboolean help_translation ();
static gboolean help_website ();
static  void menu_add_widget (GtkUIManager * p_uiManager, GtkWidget * p_widget, 
			     GtkContainer * p_box) ;
static gboolean gsb_gui_toggle_show_closed_accounts ();
/*END_STATIC*/



/*START_EXTERN*/
extern gsize nb_derniers_fichiers_ouverts ;
extern gint nb_max_derniers_fichiers_ouverts ;
extern gchar **tab_noms_derniers_fichiers_ouverts ;
extern GtkWidget *window;
extern GtkTreeStore * navigation_model;
/*END_EXTERN*/


gboolean block_menu_cb = FALSE;
GtkUIManager * ui_manager;
gint recent_files_merge_id = -1, move_to_account_merge_id = -1;


static gchar * buffer = 
"<ui>"
"  <menubar name='MenuBar'>"
"    <menu action='FileMenu'>"
"      <menuitem action='New'/>"
"      <menuitem action='Open'/>"
"      <menuitem action='Save'/>"
"      <menuitem action='SaveAs'/>"
"      <separator/>"
"      <menu action='RecentFiles'>"
"      </menu>"
"      <separator/>"
"      <menuitem action='ImportFile'/>"
"      <menuitem action='ExportFile'/>"
"      <separator/>"
"      <menuitem action='DebugFile'/>"
"      <separator/>"
"      <menuitem action='Close'/>"
"      <menuitem action='Quit'/>"
"    </menu>"
"    <menu action='EditMenu'>"
"	<menuitem action='NewTransaction'/>"
"	<menuitem action='RemoveTransaction'/>"
"	<menuitem action='CloneTransaction'/>"
"	<menuitem action='EditTransaction'/>"
"	<separator/>"
"	<menuitem action='ConvertToScheduled'/>"
"	<menu action='MoveToAnotherAccount'>"
"       </menu>"
"	<separator/>"
"       <menuitem action='NewAccount'/>"
"       <menuitem action='RemoveAccount'/>"
"	<separator/>"
"	<menuitem action='Preferences'/>"
"    </menu>"
"    <menu action='ViewMenu'>"
"      <menuitem action='ShowTransactionForm'/>"
"      <menuitem action='ShowGrid'/>"
"      <menuitem action='ShowReconciled'/>"
"      <menuitem action='ShowClosed'/>"
"      <separator/>"
"      <menuitem action='ShowOneLine'/>"
"      <menuitem action='ShowTwoLines'/>"
"      <menuitem action='ShowThreeLines'/>"
"      <menuitem action='ShowFourLines'/>"
"    </menu>"
"    <menu action='HelpMenu'>"
"      <menuitem action='Manual'/>"
"      <menuitem action='QuickStart'/>"
"      <menuitem action='Translation'/>"
"      <menuitem action='About'/>"
"      <separator/>"
"      <menuitem action='GrisbiWebsite'/>"
"      <menuitem action='ReportBug'/>"
"      <separator/>"
"      <menuitem action='Tip'/>"
"    </menu>"
"  </menubar>"
"</ui>";


static void menu_add_widget (GtkUIManager * p_uiManager, GtkWidget * p_widget, 
			     GtkContainer * p_box) 
{ 
    gtk_box_pack_start (GTK_BOX (p_box), p_widget, FALSE, FALSE, 0);
    gtk_widget_show (p_widget); 
    return; 
}



GtkWidget *init_menus ( GtkWidget *vbox )
{
    GtkWidget * barre_menu;
    GtkActionGroup * action_group;
    GtkActionEntry entries[] = {
	{ "FileMenu",		NULL,			_("_File"),
	  NULL,			NULL,			G_CALLBACK( NULL ) },

	{ "New",		GTK_STOCK_NEW,		_("_New account file..."),
	  NULL,			NULL,			G_CALLBACK( new_file ) },

	{ "Open",		GTK_STOCK_OPEN,		_("_Open..."),
	  NULL,			NULL,			G_CALLBACK( ouvrir_fichier ) },

	{ "RecentFiles",	NULL,			_("_Recently opened files"),
	  NULL,			NULL,			G_CALLBACK( NULL ) },

	{ "Save",		GTK_STOCK_SAVE,		_("_Save"),
	  NULL,			NULL,			G_CALLBACK( enregistrement_fichier ) },

	{ "SaveAs",		GTK_STOCK_SAVE_AS,	_("_Save as..."),
	  NULL,			NULL,			G_CALLBACK( gsb_save_file_as ) },	

	{ "ImportFile",		GTK_STOCK_CONVERT,	_("_Import file..."),
	  NULL,			NULL,			G_CALLBACK( importer_fichier ) },

	{ "ExportFile",		GTK_STOCK_CONVERT,	_("_Export accounts as QIF/CSV file..."),
	  NULL,			NULL,			G_CALLBACK( export_accounts ) },

	{ "DebugFile",		GTK_STOCK_FIND,		_("_Debug account file..."),
	  NULL,			NULL,			G_CALLBACK( gsb_file_debug ) },

	{ "Close",		GTK_STOCK_CLOSE,	_("_Close"),
	  NULL,			NULL,			G_CALLBACK( fermer_fichier ) },

	{ "Quit",		GTK_STOCK_QUIT,		_("_Quit"),
	  NULL,			NULL,			G_CALLBACK( fermeture_grisbi ) },

	{ "EditMenu",		NULL,			_("_Edit"),
	  NULL,			NULL,			G_CALLBACK( NULL ) },

	{ "NewTransaction",	GTK_STOCK_NEW,		_("_New transaction"),	
	  "",			NULL,			G_CALLBACK( new_transaction ) },

	{ "RemoveTransaction",	GTK_STOCK_DELETE,	_("_Remove transaction"),	
	  "",			NULL,			G_CALLBACK( remove_transaction ) },

	{ "CloneTransaction",	GTK_STOCK_COPY,		_("_Clone transaction"),	
	  "",			NULL,			G_CALLBACK( clone_selected_transaction ) },

	{ "EditTransaction",	GTK_STOCK_PROPERTIES,	_("_Edit transaction"),	
	  "",			NULL,			G_CALLBACK( gsb_transactions_list_edit_current_transaction ) },

	{ "ConvertToScheduled",	GTK_STOCK_CONVERT,	_("Convert to _scheduled transaction"),
	  NULL,			NULL,			G_CALLBACK( schedule_selected_transaction ) },

	{ "MoveToAnotherAccount",NULL,			_("_Move transaction to another account"),
	  NULL,			NULL,			G_CALLBACK( NULL ) },

	{ "Preferences",	GTK_STOCK_PREFERENCES,	_("_Preferences"),
	  NULL,			NULL,			G_CALLBACK( preferences ) },

	/* View menu */
	{ "ViewMenu",		NULL,			_("_View"),
	  NULL,			NULL,			G_CALLBACK( NULL ) },

	{ "NewAccount",		GTK_STOCK_NEW,		_("_New account"),
	  "",			NULL,			G_CALLBACK( new_account ) },

	{ "RemoveAccount",	GTK_STOCK_DELETE,	_("_Remove current account"),
	  "",			NULL,			G_CALLBACK( delete_account ) },

	/* Help Menu */
	{ "HelpMenu",		NULL,			_("_Help"),
	  NULL,			NULL,			G_CALLBACK( NULL ) },

	{ "Manual",		GTK_STOCK_HELP,		_("_Manual"),
	  NULL,			NULL,			G_CALLBACK( help_manual ) },

	{ "QuickStart",		NULL,			_("_Quick start"),
	  NULL,			NULL,			G_CALLBACK( help_quick_start ) },

	{ "Translation",	NULL,			_("_Translation"),
	  NULL,			NULL,			G_CALLBACK( help_translation ) },

	{ "About",		GTK_STOCK_ABOUT,	_("_About Grisbi..."),
	  NULL,			NULL,			G_CALLBACK( a_propos ) },

	{ "GrisbiWebsite",	NULL,			_("_Grisbi website"),
	  NULL,			NULL,			G_CALLBACK( help_website ) },

	{ "ReportBug",		NULL,			_("_Report a bug"),
	  NULL,			NULL,			G_CALLBACK( help_bugreport ) },

	{ "Tip",		GTK_STOCK_DIALOG_INFO,	_("_Tip of the day"),
	  NULL,			NULL,			G_CALLBACK( force_display_tip ) },

    };

    GtkRadioActionEntry radio_entries[] = {
	{ "ShowOneLine",	NULL,			_("Show _one line per transaction"),
	  NULL,			NULL,			ONE_LINE_PER_TRANSACTION },

	{ "ShowTwoLines",	NULL,			_("Show _two lines per transaction"),
	  NULL,			NULL,			TWO_LINES_PER_TRANSACTION },

	{ "ShowThreeLines",	NULL,			_("Show _three lines per transaction"),
	  NULL,			NULL,			THREE_LINES_PER_TRANSACTION },

	{ "ShowFourLines",	NULL,			_("Show _four lines per transaction"),
	  NULL,			NULL,			FOUR_LINES_PER_TRANSACTION },
    }; 
    GtkToggleActionEntry toggle_entries[] = {
	{ "ShowTransactionForm",NULL,			_("Show transaction _form"),
	  NULL,			NULL,			G_CALLBACK ( affiche_cache_le_formulaire ), 
	  etat.formulaire_toujours_affiche },
	{ "ShowGrid",		NULL,			_("Show _grid"),
	  NULL,			NULL,			G_CALLBACK ( gsb_gui_toggle_grid_mode ), 
	  etat.affichage_grille },
	{ "ShowReconciled",	NULL,			_("Show _reconciled"),
	  GINT_TO_POINTER (gsb_data_account_get_r ( gsb_gui_navigation_get_current_account ())), 
	  NULL,			G_CALLBACK ( gsb_gui_toggle_show_reconciled),
	  0 },
	{ "ShowClosed",		NULL,			_("Show _closed accounts"),
	  NULL,			NULL,			G_CALLBACK ( gsb_gui_toggle_show_closed_accounts ),
	  etat.show_closed_accounts } 
    };

    ui_manager = gtk_ui_manager_new ();

    g_signal_connect ( ui_manager, "add_widget", G_CALLBACK( menu_add_widget ), vbox );

    action_group = gtk_action_group_new ( "Group" );
    gtk_action_group_add_actions ( action_group, entries, G_N_ELEMENTS( entries ), NULL );
    gtk_action_group_add_radio_actions ( action_group, radio_entries, 
					 G_N_ELEMENTS( radio_entries ), 
					 -1, G_CALLBACK(gsb_gui_toggle_line_view_mode), 
					 NULL );
    gtk_action_group_add_toggle_actions ( action_group, toggle_entries, 
					  G_N_ELEMENTS( toggle_entries ), 
					  NULL );

    gtk_ui_manager_insert_action_group ( ui_manager, action_group, 0 );
    gtk_ui_manager_add_ui_from_string ( ui_manager, buffer, strlen(buffer), NULL );

    gtk_window_add_accel_group (GTK_WINDOW (window),
				gtk_ui_manager_get_accel_group (ui_manager));

    barre_menu = gtk_ui_manager_get_widget ( ui_manager, "/menubar" );
 
    return barre_menu;
}



/**
 * Blank the "Recent files submenu".
 */
void efface_derniers_fichiers_ouverts ( void )
{
    gtk_ui_manager_remove_ui ( ui_manager, recent_files_merge_id );
}



/**
 * Add menu items to the "Recent files" submenu.
 */
gboolean affiche_derniers_fichiers_ouverts ( void )
{
    gint i;
    GtkActionGroup * action_group;

    efface_derniers_fichiers_ouverts ();

    if ( nb_derniers_fichiers_ouverts > nb_max_derniers_fichiers_ouverts )
    {
	nb_derniers_fichiers_ouverts = nb_max_derniers_fichiers_ouverts;
    }
	
    if ( ! nb_derniers_fichiers_ouverts || ! nb_max_derniers_fichiers_ouverts )
    {
	return FALSE;
    }

    action_group = gtk_action_group_new ( "Group2" );

    for ( i=0 ; i<nb_derniers_fichiers_ouverts ; i++ )
    {
	gchar * tmp_name = g_strdup_printf ( "LastFile%d", i );
	GtkAction * action = gtk_action_new ( tmp_name, 
					      tab_noms_derniers_fichiers_ouverts[i], 
					      "", "" );
	g_signal_connect ( action, "activate", G_CALLBACK(ouverture_fichier_par_menu), 
			   GINT_TO_POINTER(i) );
	gtk_action_group_add_action ( action_group, action );
    }

    gtk_ui_manager_insert_action_group ( ui_manager, action_group, 1 );

    recent_files_merge_id = gtk_ui_manager_new_merge_id ( ui_manager );

    for ( i=0 ; i<nb_derniers_fichiers_ouverts ; i++ )
    {
	gchar * tmp_name = g_strdup_printf ( "LastFile%d", i );
	gchar * tmp_label = g_strdup_printf ( "_%d LastFile%d", i, i );

	gtk_ui_manager_add_ui ( ui_manager, recent_files_merge_id, 
				"/MenuBar/FileMenu/RecentFiles/",
				tmp_label, tmp_name, GTK_UI_MANAGER_MENUITEM, FALSE );
    }
    gtk_ui_manager_ensure_update ( ui_manager );

    return FALSE;
}



/**
 * Start a browser processus with local copy of manual on command
 * line.
 *
 * \return FALSE
 */
gboolean help_manual ()
{
    gchar *lang = _("_C");
    struct stat test_file;

    if ( utf8_stat ( g_strconcat ( HELP_PATH, "/", lang+1, "/manual.html", NULL ), 
		     &test_file ) != -1 )
    {
	lance_navigateur_web ( g_strconcat ( HELP_PATH, "/", lang+1, "/manual.html", 
					     NULL ) );
    }
    else
    {
	lance_navigateur_web ( g_strconcat ( HELP_PATH, "/", lang+1, "/grisbi-manuel.html", 
					     NULL ) );
    }

    return FALSE;
}



/**
 * Start a browser processus with local copy of the quick start page
 * on command line.
 *
 * \return FALSE
 */
gboolean help_quick_start ()
{
    gchar *lang = _("_C");

    lance_navigateur_web ( g_strconcat ( HELP_PATH, "/", lang+1, "/quickstart.html", 
					 NULL ));

    return FALSE;
}



/**
 * Start a browser processus with local copy of the translation page
 * on command line.
 *
 * \return FALSE
 */
gboolean help_translation ()
{
    gchar *lang = _("_C");

    lance_navigateur_web ( g_strconcat ( HELP_PATH, "/", lang+1, "/translation.html", 
					 NULL ));

    return FALSE;
}



/**
 * Start a browser processus with Grisbi website displayed.
 *
 * \return FALSE
 */
gboolean help_website ()
{
    lance_navigateur_web ( "http://www.grisbi.org/" );

    return FALSE;
}



/**
 * Start a browser processus with Grisbi bug report page displayed.
 *
 * \return FALSE
 */
gboolean help_bugreport ()
{
    lance_navigateur_web ( "http://www.grisbi.org/bugtracking/" );

    return FALSE;
}



/**
 * Concatenate menu entry names to produce a valid menu path, like
 * /Menubar/FileMenu/Save
 *
 * \param menu		Name of the root menu.
 * \param submenu	Name of the sub-menu.
 * \param subsubmenu	Name of the sub-sub-menu.
 *
 * \return A newly-created string representing the menu path.
 */
gchar * menu_name ( gchar * menu, gchar * submenu, gchar * subsubmenu )
{
  if ( subsubmenu )
    return g_strconcat ( "/MenuBar/", menu, "/", submenu, "/", subsubmenu, NULL );
  else if ( submenu )
    return g_strconcat ( "/MenuBar/", menu, "/", submenu, NULL );
  else
    return g_strconcat ( "/MenuBar/", menu, NULL );
}



/**
 * Set sensitiveness of a menu item according to a string
 * representation of its position in the menu.
 * menu.
 *
 * \param root_menu_name	Name of the menu.
 * \param submenu_name		Name of the sub menu.
 * \param subsubmenu_name	Name of the sub sub menu.
 *
 * \return TRUE on success.
 */
gboolean gsb_gui_sensitive_menu_item_from_string ( gchar * item_name, gboolean state )
{
    GtkWidget * widget;

    widget = gtk_ui_manager_get_widget ( ui_manager, item_name );
    
    if ( widget && GTK_IS_WIDGET(widget) )
    {
	gtk_widget_set_sensitive ( widget, state );
	return TRUE;
    }
    return FALSE;
}



/**
 * Set sensitiveness of a menu item according to its position in the
 * menu.
 *
 * \param root_menu_name	Name of the menu.
 * \param submenu_name		Name of the sub menu.
 * \param subsubmenu_name	Name of the sub sub menu.
 *
 * \return TRUE on success.
 */
gboolean gsb_gui_sensitive_menu_item ( gchar * root_menu_name, gchar * submenu_name,
				       gchar * subsubmenu_name, gboolean state )
{
    return gsb_gui_sensitive_menu_item_from_string ( menu_name ( root_menu_name,
								 submenu_name,
								 subsubmenu_name ), 
						     state );
}


/** 
 * Callback called when an item of the "View/Show ... lines" menu is
 * triggered.
 */
void gsb_gui_toggle_line_view_mode ( GtkRadioAction * action, GtkRadioAction *current, 
				     gpointer user_data )
/* void view_menu_cb ( gpointer callback_data, guint callback_action, GtkWidget *widget ) */
{
    /* FIXME benj: ugly but I cannot find a way to block this ... I
       understand why gtkitemfactory is deprecated. */
    if ( block_menu_cb ) return;

    switch ( gtk_radio_action_get_current_value(current) )
    {
	case ONE_LINE_PER_TRANSACTION:
	    change_aspect_liste (1);
	    break;
	case TWO_LINES_PER_TRANSACTION:
	    change_aspect_liste (2);
	    break;
	case THREE_LINES_PER_TRANSACTION:
	    change_aspect_liste (3);
	    break;
	case FOUR_LINES_PER_TRANSACTION:
	    change_aspect_liste (4);
	    break;
    }
}



/**
 *
 *
 *
 */
gboolean gsb_gui_toggle_grid_mode ()
{
    GSList *list_tmp;

    etat.affichage_grille = ! etat.affichage_grille;

    if ( etat.affichage_grille )
    {
	/* 		on affiche les grilles */

	g_signal_connect_after ( G_OBJECT ( gsb_scheduler_list_get_tree_view () ),
				 "expose-event",
				 G_CALLBACK ( affichage_traits_liste_echeances ),
				 NULL );

	list_tmp = gsb_data_account_get_list_accounts ();

	while ( list_tmp )
	{
	    gint i;

	    i = gsb_data_account_get_no_account ( list_tmp -> data );

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

	g_signal_handlers_disconnect_by_func ( G_OBJECT ( gsb_scheduler_list_get_tree_view () ),
					       G_CALLBACK ( affichage_traits_liste_echeances ),
					       NULL );

	list_tmp = gsb_data_account_get_list_accounts ();

	while ( list_tmp )
	{
	    gint i;

	    i = gsb_data_account_get_no_account ( list_tmp -> data );

	    g_signal_handlers_disconnect_by_func ( G_OBJECT ( gsb_transactions_list_get_tree_view()  ),
						   G_CALLBACK ( affichage_traits_liste_operation ),
						   NULL );

	    list_tmp = list_tmp -> next;
	}
    }

    gtk_widget_queue_draw ( gsb_transactions_list_get_tree_view());
    gtk_widget_queue_draw ( gsb_scheduler_list_get_tree_view () );
    return FALSE;
}



/**
 * Show or hide display of reconciled transactions.
 *
 * \return FALSE
 */
gboolean gsb_gui_toggle_show_reconciled ()
{
    if ( block_menu_cb )
	return FALSE;

    if ( gsb_data_account_get_r ( gsb_gui_navigation_get_current_account () ) )
	change_aspect_liste(6);
    else
	change_aspect_liste(5);

    return FALSE;
}



/**
 * Show or hide closed accounts.
 *
 * \return FALSE
 */
gboolean gsb_gui_toggle_show_closed_accounts ()
{
    etat.show_closed_accounts = ! etat.show_closed_accounts;
    modification_fichier ( TRUE );

    create_account_list ( navigation_model );

    return FALSE;
}



/**
 * Update the view menu in the menu bar
 *
 * \param account_number	The account used to update the menu
 *
 * \return FALSE
 * */
gboolean gsb_menu_update_view_menu ( gint account_number )
{
    gchar * item_name = NULL;

    devel_debug ( g_strdup_printf ("gsb_menu_update_view_menu account : %d",
				   account_number ));

    block_menu_cb = TRUE;

    /* update the showing of reconciled transactions */

    gtk_toggle_action_set_active ( GTK_TOGGLE_ACTION (gtk_ui_manager_get_action ( ui_manager, 
										  menu_name ( "ViewMenu", "ShowReconciled", NULL ))), 
				   gsb_data_account_get_r (account_number) );

    /* update the number of line showed */

    switch ( gsb_data_account_get_nb_rows (account_number))
    {
	default:
	case 1 :
	    item_name = menu_name ( "ViewMenu", "ShowOneLine", NULL );
	    break;
	case 2 :
	    item_name = menu_name ( "ViewMenu", "ShowTwoLines", NULL );
	    break;
	case 3 :
	    item_name = menu_name ( "ViewMenu", "ShowThreeLines", NULL );
	    break;
	case 4 :
	    item_name = menu_name ( "ViewMenu", "ShowFourLines", NULL );
	    break;
    }

    gtk_toggle_action_set_active ( GTK_TOGGLE_ACTION (gtk_ui_manager_get_action ( ui_manager, item_name )),
				   TRUE );

    block_menu_cb = FALSE;

    return FALSE;
}


/** 
 * Update the clickable list of closed accounts and target
 * accounts to move a transaction, in menu.
 *
 * \param
 * \return FALSE
 * */
gboolean gsb_menu_update_accounts_in_menus ( void )
{
    GSList *list_tmp;
    GtkActionGroup * action_group;

    devel_debug ( "gsb_menu_update_accounts_in_menus" );

    if ( move_to_account_merge_id != -1 ) 
	gtk_ui_manager_remove_ui ( ui_manager, move_to_account_merge_id );

    move_to_account_merge_id = gtk_ui_manager_new_merge_id ( ui_manager );
    action_group = gtk_action_group_new ( "Group3" );

    /* create the closed accounts and accounts in the menu to move a transaction */
    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;

	i = gsb_data_account_get_no_account ( list_tmp -> data );

	if ( !gsb_data_account_get_closed_account ( i ))
	{
	    gchar * tmp_name = g_strdup_printf ( "MoveToAccount%d", i );
	    GtkAction * action = gtk_action_new ( tmp_name, 
						  gsb_data_account_get_name(i),
						  "", "" );

	    if ( gsb_gui_navigation_get_current_account () == i )
		gtk_action_set_sensitive ( action, FALSE );

	    gtk_action_group_add_action ( action_group, action );

	    g_signal_connect ( action, "activate", 
			       G_CALLBACK(move_selected_operation_to_account_nb), 
			       GINT_TO_POINTER(i) );
	    gtk_ui_manager_add_ui ( ui_manager, recent_files_merge_id, 
				    "/MenuBar/EditMenu/MoveToAnotherAccount/",
				    tmp_name, tmp_name,
				    GTK_UI_MANAGER_MENUITEM, FALSE );
	}

	list_tmp = list_tmp -> next;
    }

    gtk_ui_manager_insert_action_group ( ui_manager, action_group, 2 );
    gtk_ui_manager_ensure_update ( ui_manager );

    return FALSE;
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
