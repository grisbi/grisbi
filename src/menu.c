/* ************************************************************************** */
/* Ce fichier contient les définitions de tous les menus et barres d'outils   */
/*                                                                            */
/*                                  menu.c                                    */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2004-2009 Benjamin Drieu (bdrieu@april.org)                       */
/*          http://www.grisbi.org                                             */
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"


/*START_INCLUDE*/
#include "menu.h"
#include "barre_outils.h"
#include "custom_list.h"
#include "export.h"
#include "fenetre_principale.h"
#include "file_obfuscate_qif.h"
#include "file_obfuscate.h"
#include "gsb_account.h"
#include "gsb_assistant_account.h"
#include "gsb_assistant_archive.h"
#include "gsb_assistant_archive_export.h"
#include "gsb_data_account.h"
#include "gsb_debug.h"
#include "gsb_file.h"
#include "gsb_form.h"
#include "gsb_scheduler_list.h"
#include "gsb_transactions_list.h"
#include "help.h"
#include "import.h"
#include "main.h"
#include "navigation.h"
#include "parametres.h"
#include "structures.h"
#include "traitement_variables.h"
#include "tip.h"
#include "utils.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean gsb_gui_toggle_show_closed_accounts ( void );
static gboolean gsb_gui_toggle_show_form ( void );

static gboolean help_bugreport ( void );
static gboolean help_manual ( void );
static gboolean help_quick_start ( void );
static gboolean help_translation ( void );
static gboolean help_website ( void );
static void menu_add_widget (GtkUIManager *p_uiManager,
                        GtkWidget *p_widget,
                        GtkContainer *p_box) ;
static gboolean gsb_menu_reinit_largeur_col_menu ( void );
/*END_STATIC*/



/*START_EXTERN*/
extern GtkTreeModel * navigation_model;
extern gsize nb_derniers_fichiers_ouverts;
extern gint nb_max_derniers_fichiers_ouverts;
extern gchar **tab_noms_derniers_fichiers_ouverts;
extern GtkWidget *window;
/*END_EXTERN*/


gboolean block_menu_cb = FALSE;
GtkUIManager *ui_manager;
static gint merge_id = -1;
static gint recent_files_merge_id = -1;
static gint move_to_account_merge_id = -1;

static gchar *ui_manager_buffer = 
"<ui>"
"  <menubar>"
"    <menu name='FileMenu' action='FileMenuAction' >"
"      <menuitem name='New' action='NewAction'/>"
"      <menuitem name='Open' action='OpenAction'/>"
"      <menu name='RecentFiles' action='RecentFilesAction'>"
"      </menu>"
"      <separator/>"
"      <menuitem name='Save' action='SaveAction'/>"
"      <menuitem name='SaveAs' action='SaveAsAction'/>"
"      <separator/>"
"      <menuitem name='ImportFile' action='ImportFileAction'/>"
"      <menuitem name='ExportFile' action='ExportFileAction'/>"
"      <separator/>"
"      <menuitem name='CreateArchive' action='CreateArchiveAction'/>"
"      <menuitem name='ExportArchive' action='ExportArchiveAction'/>"
"      <separator/>"
"      <menuitem name='DebugFile' action='DebugFileAction'/>"
"      <menuitem name='Obfuscate' action='ObfuscateAction'/>"
"      <menuitem name='ObfuscateQif' action='ObfuscateQifAction'/>"
"      <menuitem name='DebugMode' action='DebugModeAction'/>"
"      <separator/>"
"      <menuitem name='Close' action='CloseAction'/>"
"      <menuitem name='Quit' action='QuitAction'/>"
"    </menu>"
"    <menu name='EditMenu' action='EditMenuAction' >"
"      <menuitem name='NewTransaction' action='NewTransactionAction'/>"
"      <menuitem name='RemoveTransaction' action='RemoveTransactionAction'/>"
"      <menuitem name='CloneTransaction' action='CloneTransactionAction'/>"
"      <menuitem name='EditTransaction' action='EditTransactionAction'/>"
"      <separator/>"
"      <menuitem name='ConvertToScheduled' action='ConvertToScheduledAction'/>"
"      <menu name='MoveToAnotherAccount' action='MoveToAnotherAccountAction'>"
"      </menu>"
"   <separator/>"
"       <menuitem name='NewAccount' action='NewAccountAction'/>"
"       <menuitem name='RemoveAccount' action='RemoveAccountAction'/>"
#ifndef GTKOSXAPPLICATION
"      <separator/>"
#endif
"      <menuitem name='Preferences' action='PrefsAction'/>"
"    </menu>"
"    <menu name='ViewMenu' action='ViewMenuAction'>"
"      <menuitem name='ShowTransactionForm' action='ShowTransactionFormAction'/>"
"      <menuitem name='ShowReconciled' action='ShowReconciledAction'/>"
"      <menuitem name='ShowArchived' action='ShowArchivedAction'/>"
"      <menuitem name='ShowClosed' action='ShowClosedAction'/>"
"      <separator/>"
"      <menuitem name='ShowOneLine' action='ShowOneLineAction'/>"
"      <menuitem name='ShowTwoLines' action='ShowTwoLinesAction'/>"
"      <menuitem name='ShowThreeLines' action='ShowThreeLinesAction'/>"
"      <menuitem name='ShowFourLines' action='ShowFourLinesAction'/>"
"      <separator/>"
"      <menuitem name='InitwidthCol' action='InitwidthColAction'/>"
"    </menu>"
"    <menu name='Help' action='HelpMenuAction' >"
"      <menuitem name='Manual' action='ManualAction'/>"
"      <menuitem name='QuickStart' action='QuickStartAction'/>"
"      <menuitem name='Translation' action='TranslationAction'/>"
"      <menuitem name='About' action='AboutAction'/>"
"      <separator/>"
"      <menuitem name='GrisbiWebsite' action='GrisbiWebsiteAction'/>"
"      <menuitem name='ReportBug' action='ReportBugAction'/>"
"      <separator/>"
"      <menuitem name='Tip' action='TipAction'/>"
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
    GtkWidget *menubar;
    GtkActionGroup *actions;

    /* remind of GtkActionEntry : name, stock_id, label, accelerator, tooltip, callback */
    GtkActionEntry entries[] =
    {
        /* File menu */
        {"FileMenuAction", NULL, _("_File"), NULL, NULL, NULL},
	#ifdef GTKOSXAPPLICATION
        { "NewAction", GTK_STOCK_NEW, _("_New account file..."), "<Meta>N", NULL,
         G_CALLBACK ( gsb_file_new ) },
        {"OpenAction",  GTK_STOCK_OPEN, _("_Open..."), "<Meta>O", NULL,
         G_CALLBACK ( gsb_file_open_menu ) },
        {"RecentFilesAction", NULL, _("_Recently opened files"), NULL, NULL, NULL },
        {"SaveAction", GTK_STOCK_SAVE, _("_Save"), "<Meta>S", NULL,
         G_CALLBACK ( gsb_file_save ) },
	#else
        { "NewAction", GTK_STOCK_NEW, _("_New account file..."), NULL, NULL,
         G_CALLBACK ( gsb_file_new ) },
        {"OpenAction",  GTK_STOCK_OPEN, _("_Open..."), NULL, NULL,
         G_CALLBACK ( gsb_file_open_menu ) },
        {"RecentFilesAction", NULL, _("_Recently opened files"), NULL, NULL, NULL },
        {"SaveAction", GTK_STOCK_SAVE, _("_Save"), NULL, NULL,
         G_CALLBACK ( gsb_file_save ) },
	#endif
        {"SaveAsAction", GTK_STOCK_SAVE_AS,	_("_Save as..."), NULL, NULL,
         G_CALLBACK ( gsb_file_save_as ) },
        {"ImportFileAction", GTK_STOCK_CONVERT,	_("_Import file..."), NULL, NULL,
         G_CALLBACK ( importer_fichier ) },
        { "ExportFileAction", GTK_STOCK_CONVERT, _("_Export accounts as QIF/CSV file..."), NULL, NULL,
         G_CALLBACK ( export_accounts ) },
        {"CreateArchiveAction", GTK_STOCK_CLEAR, _("Archive transactions..."), NULL, NULL,
         G_CALLBACK ( gsb_assistant_archive_run_by_menu ) },
        {"ExportArchiveAction",	GTK_STOCK_HARDDISK,	_("_Export an archive as GSB/QIF/CSV file..."), NULL, NULL,
         G_CALLBACK ( gsb_assistant_archive_export_run ) },
        {"DebugFileAction", GTK_STOCK_FIND, _("_Debug account file..."), "", NULL,
         G_CALLBACK ( gsb_debug ) },
        {"ObfuscateAction", GTK_STOCK_FIND, _("_Obfuscate account file..."), "", NULL,
         G_CALLBACK ( file_obfuscate_run ) },
        {"ObfuscateQifAction", GTK_STOCK_FIND, _("_Obfuscate QIF file..."), "", NULL,
         G_CALLBACK ( file_obfuscate_qif_run ) },
	#ifdef GTKOSXAPPLICATION
        {"CloseAction", GTK_STOCK_CLOSE, _("_Close"), "<Meta>W", NULL,
         G_CALLBACK ( gsb_file_close ) },
	#else
        {"CloseAction", GTK_STOCK_CLOSE, _("_Close"), NULL, NULL,
         G_CALLBACK ( gsb_file_close ) },
	#endif
        {"QuitAction", GTK_STOCK_QUIT, _("_Quit"), NULL, NULL,
         G_CALLBACK ( gsb_grisbi_close ) },

        /* Editmenu */
        {"EditMenuAction", NULL, _("_Edit"), NULL, NULL, NULL },
        {"NewTransactionAction", GTK_STOCK_NEW, _("_New transaction"), "", NULL,
         G_CALLBACK ( new_transaction ) },
        {"RemoveTransactionAction", GTK_STOCK_DELETE, _("_Remove transaction"), "", NULL,
         G_CALLBACK ( remove_transaction ) },
        {"CloneTransactionAction", GTK_STOCK_COPY, _("_Clone transaction"), "", NULL,
         G_CALLBACK ( clone_selected_transaction ) },
        {"EditTransactionAction", GTK_STOCK_PROPERTIES, _("_Edit transaction"), "", NULL,
         G_CALLBACK ( gsb_transactions_list_edit_current_transaction ) },
        {"ConvertToScheduledAction", GTK_STOCK_CONVERT, _("Convert to _scheduled transaction"), NULL, NULL,
         G_CALLBACK ( schedule_selected_transaction ) },
        {"MoveToAnotherAccountAction", NULL, _("_Move transaction to another account"), NULL, NULL, NULL },
        {"NewAccountAction", GTK_STOCK_NEW, _("_New account"), "", NULL,
         G_CALLBACK ( gsb_assistant_account_run ) },
        {"RemoveAccountAction", GTK_STOCK_DELETE, _("_Remove current account"), "", NULL,
         G_CALLBACK ( gsb_account_delete ) },
        {"PrefsAction", GTK_STOCK_PREFERENCES, _("_Preferences"), NULL, NULL,
         G_CALLBACK ( preferences ) },

        /* View menu */
        {"ViewMenuAction", NULL, _("_View"), NULL, NULL, NULL },
        {"InitwidthColAction", NULL, _("Reset the column width"), NULL, NULL,
         G_CALLBACK ( gsb_menu_reinit_largeur_col_menu ) },

        /* Help menu */
        {"HelpMenuAction", NULL, _("_Help"), NULL, NULL, NULL },
	#ifdef GTKOSXAPPLICATION
        {"ManualAction", GTK_STOCK_HELP, _("_Manual"), "<Meta>H", NULL,
         G_CALLBACK ( help_manual ) },
	#else
        {"ManualAction", GTK_STOCK_HELP, _("_Manual"), NULL, NULL,
         G_CALLBACK ( help_manual ) },
	#endif
        {"QuickStartAction", NULL, _("_Quick start"), NULL, NULL,
         G_CALLBACK ( help_quick_start ) },
        {"TranslationAction", NULL, _("_Translation"), NULL, NULL,
         G_CALLBACK ( help_translation ) },
        {"AboutAction", GTK_STOCK_ABOUT, _("_About Grisbi..."), NULL, NULL,
         G_CALLBACK ( a_propos ) },
        {"GrisbiWebsiteAction", NULL, _("_Grisbi website"), NULL, NULL,
         G_CALLBACK ( help_website ) },
        {"ReportBugAction", NULL, _("_Report a bug"), NULL, NULL,
         G_CALLBACK ( help_bugreport ) },
        {"TipAction", GTK_STOCK_DIALOG_INFO, _("_Tip of the day"), NULL, NULL,
         G_CALLBACK ( force_display_tip ) },
    };

    GtkRadioActionEntry radio_entries[] =
    {
        /* Name, StockID, Label, Accelerator, Tooltip, Value */
        {"ShowOneLineAction", NULL, _("Show _one line per transaction"), NULL, NULL,
         ONE_LINE_PER_TRANSACTION },
        {"ShowTwoLinesAction", NULL, _("Show _two lines per transaction"), NULL, NULL,
         TWO_LINES_PER_TRANSACTION },
        {"ShowThreeLinesAction", NULL, _("Show _three lines per transaction"), NULL, NULL,
         THREE_LINES_PER_TRANSACTION },
        {"ShowFourLinesAction", NULL, _("Show _four lines per transaction"), NULL, NULL,
         FOUR_LINES_PER_TRANSACTION },
    };

    GtkToggleActionEntry toggle_entries[] =
    {
        {"DebugModeAction", NULL, _("Debug mode"), NULL, NULL,
         G_CALLBACK ( gsb_debug_start_log ), etat.debug_mode },
        {"ShowTransactionFormAction", NULL, _("Show transaction _form"), NULL, NULL,
         G_CALLBACK ( gsb_gui_toggle_show_form ), etat.formulaire_toujours_affiche },
#ifdef GTKOSXAPPLICATION
        {"ShowReconciledAction", NULL, _("Show _reconciled"), "<Meta>R", NULL,
         G_CALLBACK ( gsb_gui_toggle_show_reconciled ), 0 },
        {"ShowArchivedAction", NULL, _("Show _lines archives"), "<Meta>L", NULL,
         G_CALLBACK ( gsb_gui_toggle_show_archived ), 0 },
#else
        {"ShowReconciledAction", NULL, _("Show _reconciled"), "<Alt>R", NULL,
         G_CALLBACK ( gsb_gui_toggle_show_reconciled ), 0 },
        {"ShowArchivedAction", NULL, _("Show _lines archives"), "<Alt>L", NULL,
         G_CALLBACK ( gsb_gui_toggle_show_archived ), 0 },
#endif
        {"ShowClosedAction", NULL, _("Show _closed accounts"), NULL, NULL,
         G_CALLBACK ( gsb_gui_toggle_show_closed_accounts ), etat.show_closed_accounts } 
    };

    ui_manager = gtk_ui_manager_new ();

    actions = gtk_action_group_new ( "Actions" );

    gtk_action_group_add_actions (actions,
                        entries,
                        G_N_ELEMENTS ( entries ),
                        (gpointer) window );

    gtk_action_group_add_radio_actions ( actions,
                        radio_entries, 
                        G_N_ELEMENTS ( radio_entries ), 
                        -1,
                        G_CALLBACK ( gsb_gui_toggle_line_view_mode ), 
                        NULL );

    gtk_action_group_add_toggle_actions ( actions,
                        toggle_entries, 
                        G_N_ELEMENTS ( toggle_entries ), 
                        NULL );

    gtk_ui_manager_insert_action_group ( ui_manager, actions, 0 );
    merge_id = gtk_ui_manager_add_ui_from_string ( ui_manager,
                        ui_manager_buffer,
                        strlen ( ui_manager_buffer ),
                        NULL );

#ifndef GTKOSXAPPLICATION
    gtk_window_add_accel_group ( GTK_WINDOW ( window ),
                        gtk_ui_manager_get_accel_group ( ui_manager ) );
#endif /* GTKOSXAPPLICATION */

    menubar = gtk_ui_manager_get_widget ( ui_manager, "/menubar" );
    gtk_box_pack_start ( GTK_BOX ( vbox ),  menubar, FALSE, TRUE, 0 );

    gsb_gui_sensitive_menu_item ( "EditMenu", "NewTransaction", NULL, FALSE );
    gsb_menu_transaction_operations_set_sensitive ( FALSE );

    return menubar;
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

    for ( i = 0 ; i < nb_derniers_fichiers_ouverts ; i++ )
    {
        gchar *tmp_name;
        GtkAction *action;

        tmp_name = g_strdup_printf ( "LastFile%d", i );

        action = gtk_action_new ( tmp_name, 
                        tab_noms_derniers_fichiers_ouverts[i], 
                        "",
                        "" );
        g_free ( tmp_name );
        g_signal_connect ( action,
                        "activate",
                        G_CALLBACK ( gsb_file_open_direct_menu ), 
                        GINT_TO_POINTER ( i ) );
        gtk_action_group_add_action ( action_group, action );
    }

    gtk_ui_manager_insert_action_group ( ui_manager, action_group, 1 );

    recent_files_merge_id = gtk_ui_manager_new_merge_id ( ui_manager );

    for ( i=0 ; i<nb_derniers_fichiers_ouverts ; i++ )
    {
        gchar *tmp_name;
        gchar *tmp_label;

        tmp_name = g_strdup_printf ( "LastFile%d", i );
        tmp_label = g_strdup_printf ( "_%d LastFile%d", i, i );

        gtk_ui_manager_add_ui ( ui_manager,
                    recent_files_merge_id, 
                    "/menubar/FileMenu/RecentFiles/",
                    tmp_label,
                    tmp_name,
                    GTK_UI_MANAGER_MENUITEM,
                    FALSE );

        g_free ( tmp_name );
        g_free ( tmp_label );
    }

    /* add a separator */
    gtk_ui_manager_add_ui ( ui_manager,
                    merge_id, 
                    "/menubar/FileMenu/Open/",
                    NULL,
                    NULL,
                    GTK_UI_MANAGER_SEPARATOR,
                    FALSE );

    gtk_ui_manager_ensure_update ( ui_manager );

#ifdef GTKOSXAPPLICATION
    grisbi_osx_app_update_menus_cb ( );
#endif /* GTKOSXAPPLICATION */
    return FALSE;
}



/**
 * Start a browser processus with local copy of manual on command
 * line.
 *
 * \return FALSE
 */
gboolean help_manual ( void )
{
    gchar *lang = _("en");
    gchar *string;

    string = g_strconcat ( HELP_PATH, "/", lang, "/manual.html", NULL );

    if (g_file_test ( string,
		      G_FILE_TEST_EXISTS ))
    {
	lance_navigateur_web (string);
	g_free (string);
    }
    else
    {
	g_free (string);
	string = g_strconcat ( HELP_PATH, "/", lang, "/grisbi-manuel.html", NULL );
	lance_navigateur_web (string);
	g_free (string);
    }

    return FALSE;
}



/**
 * Start a browser processus with local copy of the quick start page
 * on command line.
 *
 * \return FALSE
 */
gboolean help_quick_start ( void )
{
    gchar *lang = _("en");

    gchar* tmpstr = g_strconcat ( HELP_PATH, "/", lang, "/quickstart.html", 
					 NULL );
    lance_navigateur_web ( tmpstr );
    g_free ( tmpstr );

    return FALSE;
}



/**
 * Start a browser processus with local copy of the translation page
 * on command line.
 *
 * \return FALSE
 */
gboolean help_translation ( void )
{
    gchar *lang = _("en");

    gchar* tmpstr = g_strconcat ( HELP_PATH, "/", lang, "/translation.html", 
					 NULL );
    lance_navigateur_web ( tmpstr );
    g_free ( tmpstr );

    return FALSE;
}



/**
 * Start a browser processus with Grisbi website displayed.
 *
 * \return FALSE
 */
gboolean help_website ( void )
{
    lance_navigateur_web ( "http://www.grisbi.org/" );

    return FALSE;
}



/**
 * Start a browser processus with Grisbi bug report page displayed.
 *
 * \return FALSE
 */
gboolean help_bugreport ( void )
{
    lance_navigateur_web ( "http://www.grisbi.org/bugtracking/" );

    return FALSE;
}



/**
 * Concatenate menu entry names to produce a valid menu path, like
 * /menubar/FileMenu/Save
 *
 * \param menu		Name of the root menu.
 * \param submenu	Name of the sub-menu.
 * \param subsubmenu	Name of the sub-sub-menu.
 *
 * \return A newly-created string representing the menu path.
 */
gchar *menu_name ( gchar *menu, gchar *submenu, gchar *subsubmenu )
{
  if ( subsubmenu )
    return g_strconcat ( "/menubar/", menu, "/", submenu, "/", subsubmenu, NULL );
  else if ( submenu )
    return g_strconcat ( "/menubar/", menu, "/", submenu, NULL );
  else
    return g_strconcat ( "/menubar/", menu, NULL );
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
gboolean gsb_gui_sensitive_menu_item_from_string ( gchar *item_name, gboolean state )
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
gboolean gsb_gui_sensitive_menu_item ( gchar *root_menu_name,
                        gchar *submenu_name,
                        gchar *subsubmenu_name,
                        gboolean state )
{
    gchar* tmpstr;
    gboolean result;

    tmpstr = menu_name ( root_menu_name, submenu_name, subsubmenu_name );
    result = gsb_gui_sensitive_menu_item_from_string ( tmpstr, state );

    g_free ( tmpstr );

    return result;
}


/** 
 * Callback called when an item of the "View/Show ... lines" menu is
 * triggered.
 */
void gsb_gui_toggle_line_view_mode ( GtkRadioAction *action,
                        GtkRadioAction *current,
                        gpointer user_data )
{
    /* FIXME benj: ugly but I cannot find a way to block this ... */
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
 * Show or hide the transactions form.
 *
 * \return FALSE
 */
gboolean gsb_gui_toggle_show_form ( void )
{
    devel_debug (NULL);

    /* FIXME benj: ugly but I cannot find a way to block this ... */
    if ( block_menu_cb ) return FALSE;

    gsb_form_switch_expander ( );

    return FALSE;
}



/**
 * Show or hide display of reconciled transactions.
 *
 * \return FALSE
 */
gboolean gsb_gui_toggle_show_reconciled ( void )
{
    gint current_account;

    if ( block_menu_cb )
	    return FALSE;

    current_account = gsb_gui_navigation_get_current_account ( );
    if ( current_account == -1 || etat.equilibrage == 1 )
        return FALSE;

    if ( gsb_data_account_get_r ( current_account ) )
	    change_aspect_liste ( 6 );
    else
	    change_aspect_liste ( 5 );

    return FALSE;
}


/**
 * Show or hide display of lines archives.
 *
 * \return FALSE
 */
gboolean gsb_gui_toggle_show_archived ( void )
{
    gint current_account;

    if ( block_menu_cb )
	    return FALSE;

    current_account = gsb_gui_navigation_get_current_account ( );
    if ( current_account == -1 )
        return FALSE;

    if ( gsb_data_account_get_l ( current_account ) )
	    change_aspect_liste ( 8 );
    else
	    change_aspect_liste ( 7 );

    return FALSE;
}


/**
 * Show or hide closed accounts.
 *
 * \return FALSE
 */
gboolean gsb_gui_toggle_show_closed_accounts ( void )
{
    etat.show_closed_accounts = ! etat.show_closed_accounts;

    create_account_list ( navigation_model );
    gsb_gui_navigation_update_home_page ( );

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );

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
    gchar *tmpstr;

    devel_debug_int (account_number);

    block_menu_cb = TRUE;

    /* update the showing of reconciled transactions */
    tmpstr = menu_name ( "ViewMenu", "ShowReconciled", NULL );
    gtk_toggle_action_set_active ( GTK_TOGGLE_ACTION (
                        gtk_ui_manager_get_action ( ui_manager, tmpstr) ), 
				        gsb_data_account_get_r ( account_number ) );
    g_free ( tmpstr );

    tmpstr = menu_name ( "ViewMenu", "ShowTransactionForm", NULL );
    gtk_toggle_action_set_active ( GTK_TOGGLE_ACTION (
                        gtk_ui_manager_get_action ( ui_manager, tmpstr) ), 
				        gsb_form_is_visible ( ) );
    g_free ( tmpstr );

    /* update the showing of archived transactions */
    tmpstr = menu_name ( "ViewMenu", "ShowArchived", NULL );
    gtk_toggle_action_set_active ( GTK_TOGGLE_ACTION (
                        gtk_ui_manager_get_action ( ui_manager, tmpstr) ), 
				        gsb_data_account_get_l ( account_number ) );
    g_free ( tmpstr );

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

    gtk_toggle_action_set_active ( GTK_TOGGLE_ACTION (
                        gtk_ui_manager_get_action ( ui_manager, item_name ) ),
				        TRUE );
    g_free ( item_name );

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
	    gchar * account_name = gsb_data_account_get_name(i);
	    GtkAction * action;

	    if ( ! account_name )
	    {
		account_name = _("Unnamed account");
	    }
	    action = gtk_action_new ( tmp_name, account_name, "", "" );

	    if ( gsb_gui_navigation_get_current_account () == i )
		gtk_action_set_sensitive ( action, FALSE );

	    gtk_action_group_add_action ( action_group, action );

	    g_signal_connect ( action, "activate", 
			       G_CALLBACK(move_selected_operation_to_account_nb), 
			       GINT_TO_POINTER(i) );
	    gtk_ui_manager_add_ui ( ui_manager, recent_files_merge_id, 
				    "/menubar/EditMenu/MoveToAnotherAccount/",
				    tmp_name, tmp_name,
				    GTK_UI_MANAGER_MENUITEM, FALSE );
	    g_free ( tmp_name );
	}

	list_tmp = list_tmp -> next;
    }

    gtk_ui_manager_insert_action_group ( ui_manager, action_group, 2 );
    gtk_ui_manager_ensure_update ( ui_manager );

    return FALSE;
}



/**
 * Set sensitiveness of all menu items that work on the selected transaction.

 * \param sensitive	Sensitiveness (FALSE for unsensitive, TRUE for
 *			sensitive).
 * 
 * \return		FALSE
 */
gboolean gsb_menu_transaction_operations_set_sensitive ( gboolean sensitive )
{
    devel_debug ( sensitive ? "item sensitive" : "item unsensitive" );

    gsb_gui_sensitive_menu_item ( "EditMenu", "RemoveTransaction", NULL, sensitive );
    gsb_gui_sensitive_menu_item ( "EditMenu", "CloneTransaction", NULL, sensitive );
    gsb_gui_sensitive_menu_item ( "EditMenu", "EditTransaction", NULL, sensitive );
    gsb_gui_sensitive_menu_item ( "EditMenu", "ConvertToScheduled", NULL, sensitive );
    gsb_gui_sensitive_menu_item ( "EditMenu", "MoveToAnotherAccount", NULL, sensitive );

    return FALSE;
}


/**
 *
 *
 *
 *
 **/
GtkUIManager *gsb_menu_get_ui_manager ( void )
{
    return ui_manager;
}


/**
 *
 *
 *
 *
 **/
gboolean gsb_menu_reinit_largeur_col_menu ( void )
{
    gint current_page;

    current_page = gsb_gui_navigation_get_current_page ( );

    if ( current_page == GSB_ACCOUNT_PAGE )
    {
        initialise_largeur_colonnes_tab_affichage_ope ( GSB_ACCOUNT_PAGE, NULL );

        gsb_transactions_list_set_largeur_col ( );
    }
    else if ( current_page == GSB_SCHEDULER_PAGE )
    {
        initialise_largeur_colonnes_tab_affichage_ope ( GSB_SCHEDULER_PAGE, NULL );

        gsb_scheduler_list_set_largeur_col ( );
    }

    return FALSE;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
