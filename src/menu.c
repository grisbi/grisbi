/* ************************************************************************** */
/* Ce fichier contient les définitions de tous les menus et barres d'outils   */
/*                                                                            */
/*                                  menu.c                                    */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2004-2005 Benjamin Drieu (bdrieu@april.org)	      */
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
#include "echeancier_liste.h"
#include "operations_liste.h"
#include "operations_formulaire.h"
#include "etats_onglet.h"
#include "barre_outils.h"
#include "comptes_traitements.h"
#include "fichiers_gestion.h"
#include "qif.h"
#include "erreur.h"
#include "tip.h"
#include "gsb_data_account.h"
#include "utils_str.h"
#include "import.h"
#include "etats_calculs.h"
#include "utils.h"
#include "main.h"
#include "etats_config.h"
#include "parametres.h"
#include "gsb_file_config.h"
#include "structures.h"
#include "menu.h"
#include "include.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void affiche_aide_locale ( gpointer null,
			   gint origine );
static gboolean gsb_gui_toggle_grid_mode ();
static void gsb_gui_toggle_line_view_mode ( GtkRadioAction * action, GtkRadioAction *current, 
				     gpointer user_data );
static void lien_web ( GtkWidget *widget,
		gint origine );
static  void menu_add_widget (GtkUIManager * p_uiManager, GtkWidget * p_widget, 
			     GtkContainer * p_box) ;
/*END_STATIC*/



/*START_EXTERN*/
extern GtkItemFactory *item_factory_menu_general;
extern GtkTreeModelFilter * navigation_model_filtered;
extern gsize nb_derniers_fichiers_ouverts ;
extern gchar **tab_noms_derniers_fichiers_ouverts ;
extern GtkWidget *tree_view_liste_echeances;
extern GtkWidget *window;
/*END_EXTERN*/


gboolean block_menu_cb = FALSE;
GtkUIManager * ui_manager;
gint recent_files_merge_id;


    gchar * buffer = 
"<ui>"
"  <menubar name='MenuBar'>"
"    <menu action='FileMenu'>"
"      <menuitem action='New'/>"
"      <menuitem action='Open'/>"
"      <menuitem action='Save'/>"
"      <menuitem action='SaveAs'/>"
"      <separator/>"
"      <menu action=\"RecentFiles\">"
"      </menu>"
"      <separator/>"
"      <menuitem action='ImportFile'/>"
"      <menuitem action='ExportFile'/>"
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
"	<menuitem action='MoveToAnotherAccount'/>"
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
"    <menu action='AccountMenu'>"
"      <menuitem action='NewAccount'/>"
"      <menuitem action='RemoveAccount'/>"
"    </menu>"
"    <menu action='ReportsMenu'>"
"      <menuitem action='NewReport'/>"
"      <separator/>"
"      <menuitem action='CloneReport'/>"
"      <menuitem action='PrintReport'/>"
"      <menuitem action='ImportReport'/>"
"      <menuitem action='ExportReport'/>"
"      <menuitem action='ExportReportHTML'/>"
"      <separator/>"
"      <menuitem action='RemoveReport'/>"
"      <menuitem action='EditReport'/>"
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

/* Arg */
{ "SaveAs",		GTK_STOCK_SAVE_AS,	_("_Save as..."),
  NULL,			NULL,			G_CALLBACK( gsb_save_file_as ) },	

{ "ImportFile",		NULL,			_("_Import QIF/OFX/CSV/Gnucash file..."),
  NULL,			NULL,			G_CALLBACK( importer_fichier ) },

{ "ExportFile",		NULL,			_("_Export QIF file..."),
  NULL,			NULL,			G_CALLBACK( exporter_fichier_qif ) },

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

/* Argh */
{ "ConvertToScheduled",	GTK_STOCK_CONVERT,	_("Convert to _scheduled transaction"),
  NULL,			NULL,			G_CALLBACK( NULL ) },

{ "MoveToAnotherAccount",NULL,			_("_Move transaction to another account"),
  NULL,			NULL,			G_CALLBACK( NULL ) },

{ "Preferences",	GTK_STOCK_PREFERENCES,	_("_Preferences"),
  NULL,			NULL,			G_CALLBACK( preferences ) },

/* View menu */
{ "ViewMenu",		NULL,			_("_View"),
  NULL,			NULL,			G_CALLBACK( NULL ) },

/* Account menu */
{ "AccountMenu",	NULL,			_("_Accounts"),
  NULL,			NULL,			G_CALLBACK( NULL ) },

{ "NewAccount",		GTK_STOCK_NEW,		_("_New account"),
  "",			NULL,			G_CALLBACK( NULL ) },

{ "RemoveAccount",	GTK_STOCK_DELETE,	_("_Remove current account"),
  "",			NULL,			G_CALLBACK( NULL ) },

/* Reports menu */
{ "ReportsMenu",	NULL,			_("_Reports"),
  NULL,			NULL,			G_CALLBACK( NULL ) },

{ "NewReport",		GTK_STOCK_NEW,		_("_New report"),
  "",			"",			G_CALLBACK( ajout_etat ) },

{ "CloneReport",	GTK_STOCK_COPY,		_("_Clone report"),
  "",			NULL,			G_CALLBACK( dupliquer_etat ) },

{ "PrintReport",	GTK_STOCK_PRINT,	_("_Print report..."),
  "",			"",			G_CALLBACK( impression_etat_courant ) },

{ "ImportReport",	GTK_STOCK_CONVERT,	_("_Import report..."),
  NULL,			NULL,			G_CALLBACK( importer_etat ) },

{ "ExportReport",	GTK_STOCK_CONVERT,	_("_Export report..."),
  NULL,			NULL,			G_CALLBACK( exporter_etat ) },

{ "ExportReportHTML",	NULL,			_("_Export report as HTML..."),
  NULL,			NULL,			G_CALLBACK( export_etat_courant_vers_html ) },

{ "RemoveReport",	GTK_STOCK_DELETE,	_("_Remove report"),
  NULL,			"",			G_CALLBACK( efface_etat ) },

{ "EditReport",		GTK_STOCK_PROPERTIES,	_("_Edit report"),
  NULL,			"",			G_CALLBACK( personnalisation_etat ) },

/* Help Menu */
{ "HelpMenu",		NULL,			_("_Help"),
  NULL,			NULL,			G_CALLBACK( NULL ) },

{ "Manual",		GTK_STOCK_HELP,		_("_Manual"),
  NULL,			NULL,			G_CALLBACK( NULL ) },

{ "QuickStart",		NULL,			_("_Quick start"),
  NULL,			NULL,			G_CALLBACK( NULL ) },

{ "Translation",	NULL,			_("_Translation"),
  NULL,			NULL,			G_CALLBACK( NULL ) },

{ "About",		GTK_STOCK_ABOUT,	_("_About Grisbi..."),
  NULL,			NULL,			G_CALLBACK( a_propos ) },

{ "GrisbiWebsite",	NULL,			_("_Grisbi website"),
  NULL,			NULL,			G_CALLBACK( NULL ) },

{ "ReportBug",		NULL,			_("_Report a bug"),
  NULL,			NULL,			G_CALLBACK( NULL ) },

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
  NULL,			NULL,			G_CALLBACK ( NULL ),
  0 },
{ "ShowClosed",		NULL,			_("Show _closed accounts"),
  NULL,			NULL,			G_CALLBACK ( NULL ),
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
    gtk_widget_show_all ( barre_menu );
 
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
void affiche_derniers_fichiers_ouverts ( void )
{
    gint i;
    GtkActionGroup * action_group;

    efface_derniers_fichiers_ouverts ();

    if ( !nb_derniers_fichiers_ouverts )
    {
	return;
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
}



void lien_web ( GtkWidget *widget,
		gint origine )
{
    switch ( origine )
    {
	case 1:
	    lance_navigateur_web ( "http://www.grisbi.org");
	    break;	

	case 2:
	    lance_navigateur_web ( "http://www.grisbi.org/bugtracking");
	    break;

	case 3:
	    lance_navigateur_web ( "http://www.grisbi.org/manuel.html");
	    break;
    }
}
/* **************************************************************************************************** */


/* **************************************************************************************************** */
void affiche_aide_locale ( gpointer null,
			   gint origine )
{
    gchar *lang = _("_C");

    /* we use lang to translate it, else C will be translated as for checked transactions */

    switch ( origine )
    {
	case 1:
	    lance_navigateur_web ( g_strconcat ( HELP_PATH, "/", lang+1, "/grisbi-manuel.html", NULL ));
	    break;	

	case 2:
	    lance_navigateur_web ( g_strconcat ( HELP_PATH, "/", lang+1, "/quickstart.html", NULL ));
	    break;

	case 3:
	    lance_navigateur_web ( g_strconcat ( HELP_PATH, "/", lang+1, "/translation.html", NULL ));
	    break;
    }
}
/* **************************************************************************************************** */


gchar * menu_name ( gchar * menu, gchar * submenu, gchar * subsubmenu )
{
  if ( subsubmenu )
    return g_strconcat ( "/", menu, "/", submenu, "/", subsubmenu, NULL );
  else if ( submenu )
    return g_strconcat ( "/", menu, "/", submenu, NULL );
  else
    return g_strconcat ( "/", menu, NULL );
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

	g_signal_connect_after ( G_OBJECT ( tree_view_liste_echeances ),
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

	g_signal_handlers_disconnect_by_func ( G_OBJECT ( tree_view_liste_echeances ),
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
    gtk_widget_queue_draw ( tree_view_liste_echeances );
    return FALSE;
}

/*     widget_state = gtk_check_menu_item_get_active( GTK_CHECK_MENU_ITEM(widget) ); */
/*     current_account = gsb_data_account_get_current_account (); */

/*     switch ( callback_action ) */
/*     { */
/* 	case HIDE_SHOW_TRANSACTION_FORM: */
/* 	    affiche_cache_le_formulaire(); */
/* 	    break; */
/* 	case HIDE_SHOW_GRID: */
/* 	    change_aspect_liste (0); */
/* 	    break; */
/* 	case HIDE_SHOW_RECONCILED_TRANSACTIONS: */
/* 	    if ( gsb_data_account_get_r ( current_account ) ) */
/* 		change_aspect_liste(6); */
/* 	    else */
/* 		change_aspect_liste(5); */
/* 	    gsb_data_account_set_r ( current_account, widget_state ); */
/* 	    break; */
/* 	case HIDE_SHOW_CLOSED_ACCOUNTS: */
/* 	    etat.show_closed_accounts = widget_state; */
/* 	    gtk_tree_model_filter_refilter ( navigation_model_filtered ); */
/* 	    break; */
/* 	default: */
/* 	    break; */
/*     } */



/**
 * update the view menu in the menu bar
 * \account_number the account used to update the menu
 * \return FALSE
 * */
gboolean gsb_menu_update_view_menu ( gint account_number )
{
    GtkWidget *check_menu_item;
    gchar * item_name = NULL;

    if ( DEBUG )
	printf ( "gsb_menu_update_view_menu account : %d\n",
		 account_number );

    block_menu_cb = TRUE;

    /* update the showing of reconciled transactions */

    check_menu_item = gtk_item_factory_get_item ( item_factory_menu_general,
						  menu_name(_("View"), _("Show reconciled transactions"), NULL) );
    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(check_menu_item),
				    gsb_data_account_get_r (account_number));

    /* update the number of line showed */

    switch ( gsb_data_account_get_nb_rows (account_number))
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

    check_menu_item = gtk_item_factory_get_item ( item_factory_menu_general, item_name );
    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(check_menu_item),
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

    if ( DEBUG )
	printf ( "gsb_menu_update_accounts_in_menus\n" );

    /* erase the closed accounts and accounts in the menu to move a transaction */
    /* FIXME : je pense un bug ici, il va effacer que les comptes dont il connait le nom, peut 
     * être faudrait passer par les menus directement ?? à vérifier (cédric)*/

    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;
	gchar *tmp;

	i = gsb_data_account_get_no_account ( list_tmp -> data );

	tmp = my_strdelimit ( gsb_data_account_get_name (i),
			      "/",
			      "\\/" );

	if ( !gsb_data_account_get_closed_account ( i ))
	    gtk_item_factory_delete_item ( item_factory_menu_general,
					   menu_name(_("Edit"), _("Move transaction to another account"), tmp ));
	
	list_tmp = list_tmp -> next;
    }

    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   menu_name(_("Edit"), _("Move transaction to another account"), NULL)),
			       FALSE );


    /* create the closed accounts and accounts in the menu to move a transaction */

    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;
	GtkItemFactoryEntry *item_factory_entry;
	gchar *tmp;

	i = gsb_data_account_get_no_account ( list_tmp -> data );

	/* 	we put all the accounts in the edition menu */
	item_factory_entry = calloc ( 1, sizeof ( GtkItemFactoryEntry ));

	tmp = my_strdelimit ( gsb_data_account_get_name (i), "/", "\\/" );

	item_factory_entry -> path = menu_name(_("Edit"), 
					       _("Move transaction to another account"), 
					       my_strdelimit ( tmp, "_", "__" ) ); 

	item_factory_entry -> callback = G_CALLBACK ( move_selected_operation_to_account_nb );

	/* add 1, else for 0 it wouldn't work */
	item_factory_entry -> callback_action = i + 1;

	gtk_item_factory_create_item ( item_factory_menu_general,
				       item_factory_entry,
				       GINT_TO_POINTER (i),
				       1 );

	gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							       menu_name(_("Edit"), _("Move transaction to another account"), NULL)),
				   TRUE );

	if ( i == gsb_data_account_get_current_account () )
	{
	    /* un-sensitive the name of account in the menu */
	    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
								   menu_name(_("Edit"), _("Move transaction to another account"), tmp)),
				       FALSE );
	}

	list_tmp = list_tmp -> next;
    }
    return FALSE;
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
