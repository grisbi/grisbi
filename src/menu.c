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
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "menu.h"
#include "fenetre_principale.h"
#include "grisbi_app.h"
#include "grisbi_prefs.h"
#include "gsb_account.h"
#include "gsb_file.h"
#include "gsb_form.h"
#include "gsb_navigation.h"
#include "gsb_navigation_view.h"
#include "gsb_scheduler_list.h"
#include "gsb_transactions_list.h"
#include "traitement_variables.h"
#include "utils.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/


/*START_EXTERN*/
/*END_EXTERN*/

/* variables globales */
static gboolean block_menu_cb = FALSE;

/**
 * Blank the "Recent files submenu".
 */
void efface_derniers_fichiers_ouverts ( void )
{
    GrisbiWindow *window;
    GtkUIManager *ui_manager;
    guint recent_files_merge_id;

    window = grisbi_app_get_active_window ( NULL );
    ui_manager = grisbi_window_get_ui_manager ( window );
    recent_files_merge_id = grisbi_window_get_sub_menu_merge_id  ( window, "recent_file" );

    gtk_ui_manager_remove_ui ( ui_manager, recent_files_merge_id );
}


/**
 * Add menu items to the "Recent files" submenu.
 */
gboolean affiche_derniers_fichiers_ouverts ( void )
{
    GrisbiWindow *window;
    GtkActionGroup *action_group;
    GrisbiAppConf *conf;
    GtkUIManager *ui_manager;
    guint recent_files_merge_id;
    gint i;

    window = grisbi_app_get_active_window ( NULL );
    ui_manager = grisbi_window_get_ui_manager ( window );

    conf = grisbi_app_get_conf ( );

    efface_derniers_fichiers_ouverts ();

    if ( !conf->nb_derniers_fichiers_ouverts || !conf->nb_max_derniers_fichiers_ouverts )
    {
        return FALSE;
    }

    if ( conf->nb_derniers_fichiers_ouverts > conf->nb_max_derniers_fichiers_ouverts )
    {
        conf->nb_derniers_fichiers_ouverts = conf->nb_max_derniers_fichiers_ouverts;
    }

    ui_manager = grisbi_window_get_ui_manager ( grisbi_app_get_active_window ( NULL ) );

    action_group = gtk_action_group_new ( "Group2" );

    for ( i = 0 ; i < conf->nb_derniers_fichiers_ouverts ; i++ )
    {
        gchar *tmp_name;
        GtkAction *action;

        tmp_name = g_strdup_printf ( "LastFile%d", i );

        action = gtk_action_new ( tmp_name,
                        conf->tab_noms_derniers_fichiers_ouverts[i],
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

    for ( i=0 ; i < conf->nb_derniers_fichiers_ouverts ; i++ )
    {
        gchar *tmp_name;
        gchar *tmp_label;

        tmp_name = g_strdup_printf ( "LastFile%d", i );
        tmp_label = g_strdup_printf ( "_%d LastFile%d", i, i );

        gtk_ui_manager_add_ui ( ui_manager,
                    recent_files_merge_id,
                    "/menubar/FileMenu/RecentFiles/FileRecentsPlaceholder",
                    tmp_label,
                    tmp_name,
                    GTK_UI_MANAGER_MENUITEM,
                    FALSE );

        g_free ( tmp_name );
        g_free ( tmp_label );
    }

    grisbi_window_set_sub_menu_merge_id ( window, recent_files_merge_id, "recent_file" );
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

    string = g_build_filename ( HELP_PATH, lang, "manual.html", NULL );

    if (g_file_test ( string,
		      G_FILE_TEST_EXISTS ))
    {
	lance_navigateur_web (string);
	g_free (string);
    }
    else
    {
	g_free (string);
	string = g_build_filename ( HELP_PATH, lang, "grisbi-manuel.html", NULL );
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

    gchar* tmpstr = g_build_filename ( HELP_PATH, lang, "quickstart.html", NULL );
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

    gchar* tmpstr = g_build_filename ( HELP_PATH, lang, "translation.html", NULL );
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
 * Set sensitiveness of a menu item according to a string
 * representation of its position in the menu.
 * menu.
 *
 * \param item_name		Path of the menu item.
 * \param state			Whether widget should be 'sensitive' or not.
 *
 * \return TRUE on success.
 */
gboolean gsb_gui_sensitive_menu_item ( gchar *item_name, gboolean state )
{
    GtkWidget *widget;
    GtkUIManager *ui_manager;

    ui_manager = grisbi_window_get_ui_manager ( grisbi_app_get_active_window ( NULL ) );

    widget = gtk_ui_manager_get_widget ( ui_manager, item_name );
    
    if ( widget && GTK_IS_WIDGET(widget) )
    {
        gtk_widget_set_sensitive ( widget, state );
        return TRUE;
    }
    return FALSE;
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
    if ( block_menu_cb )
        return FALSE;

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
    GrisbiWindowRun *run;

    if ( block_menu_cb )
        return FALSE;
    run = grisbi_window_get_struct_run ( NULL );

    current_account = gsb_gui_navigation_get_current_account ( );
    if ( current_account == -1 || run->equilibrage == 1 )
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
    GsbNavigationView *tree_view;
    GrisbiAppConf *conf;

    conf = grisbi_app_get_conf ( );
    conf->show_closed_accounts = !conf->show_closed_accounts;

    tree_view = GSB_NAVIGATION_VIEW ( grisbi_window_get_navigation_tree_view () );
    gsb_navigation_view_create_account_list ( tree_view );
    gsb_gui_navigation_update_home_page ( );

    gsb_file_set_modified ( TRUE );

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
    GtkUIManager *ui_manager;

    devel_debug_int (account_number);

    ui_manager = grisbi_window_get_ui_manager ( grisbi_app_get_active_window ( NULL ) );

    block_menu_cb = TRUE;

    /* update the showing of reconciled transactions */
    tmpstr = "/menubar/ViewMenu/ShowReconciled";
    gtk_toggle_action_set_active ( GTK_TOGGLE_ACTION (
                        gtk_ui_manager_get_action ( ui_manager, tmpstr) ),
                        gsb_data_account_get_r ( account_number ) );

    tmpstr = "/menubar/ViewMenu/ShowTransactionForm";
    gtk_toggle_action_set_active ( GTK_TOGGLE_ACTION (
                        gtk_ui_manager_get_action ( ui_manager, tmpstr) ),
                        gsb_form_is_visible ( ) );

    /* update the showing of archived transactions */
    tmpstr = "/menubar/ViewMenu/ShowArchived";
    gtk_toggle_action_set_active ( GTK_TOGGLE_ACTION (
                        gtk_ui_manager_get_action ( ui_manager, tmpstr) ),
                        gsb_data_account_get_l ( account_number ) );

    /* update the number of line showed */
    switch ( gsb_data_account_get_nb_rows (account_number))
    {
        default:
        case 1 :
            item_name = "/menubar/ViewMenu/ShowOneLine";
            break;
        case 2 :
            item_name = "/menubar/ViewMenu/ShowTwoLines";
            break;
        case 3 :
            item_name = "/menubar/ViewMenu/ShowThreeLines";
            break;
        case 4 :
            item_name = "/menubar/ViewMenu/ShowFourLines";
            break;
    }

    gtk_toggle_action_set_active ( GTK_TOGGLE_ACTION (
                        gtk_ui_manager_get_action ( ui_manager, item_name ) ),
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
    GrisbiWindow *window;
    GSList *list_tmp;
    GtkActionGroup *action_group;
    GtkUIManager *ui_manager;
    guint move_to_account_merge_id;

    window = grisbi_app_get_active_window ( NULL );
    ui_manager = grisbi_window_get_ui_manager ( window );
    move_to_account_merge_id = grisbi_window_get_sub_menu_merge_id ( window, "move_to_account" );

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

            g_signal_connect ( action,
                        "activate",
                        G_CALLBACK ( move_selected_operation_to_account_nb ),
                        GINT_TO_POINTER ( i ) );

            gtk_ui_manager_add_ui ( ui_manager, move_to_account_merge_id,
                        "/menubar/EditMenu/MoveToAnotherAccount/",
                        tmp_name, tmp_name,
                        GTK_UI_MANAGER_MENUITEM, FALSE );
            g_free ( tmp_name );
        }

        list_tmp = list_tmp -> next;
    }

    gtk_ui_manager_insert_action_group ( ui_manager, action_group, 2 );
    grisbi_window_set_sub_menu_merge_id ( window, move_to_account_merge_id, "move_to_account" );
    gtk_ui_manager_ensure_update ( ui_manager );

    /* return */
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

    gsb_gui_sensitive_menu_item ( "/menubar/EditMenu/RemoveTransaction", sensitive );
    gsb_gui_sensitive_menu_item ( "/menubar/EditMenu/TemplateTransaction", sensitive );
    gsb_gui_sensitive_menu_item ( "/menubar/EditMenu/CloneTransaction", sensitive );
    gsb_gui_sensitive_menu_item ( "/menubar/EditMenu/EditTransaction", sensitive );
    gsb_gui_sensitive_menu_item ( "/menubar/EditMenu/ConvertToScheduled", sensitive );
    gsb_gui_sensitive_menu_item ( "/menubar/EditMenu/MoveToAnotherAccount", sensitive );

    return FALSE;
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


/**
 *
 *
 *
 */
gboolean gsb_menu_set_block_menu_cb ( gboolean etat )
{
    
    block_menu_cb = etat;

    return FALSE;
}


/**
 *
 *
 * \param
 *
 * \return
 **/
void gsb_menu_sensitive ( gboolean sensitif )
{
    GrisbiWindow *window;
    GtkUIManager *ui_manager;
    GtkActionGroup *actions;

    devel_debug_int (sensitif);

    window = grisbi_app_get_active_window ( NULL );
    ui_manager = grisbi_app_get_active_ui_manager ( );


    actions = grisbi_window_get_action_group ( window, "DivisionSensitiveActions" );
    gtk_action_group_set_sensitive ( actions, sensitif );
    actions = grisbi_window_get_action_group ( window, "FileDebugToggleAction" );
    gtk_action_group_set_sensitive ( actions, sensitif );

}


/**
 *
 *
 * \param
 *
 * \return
 **/
void gsb_menu_full_screen_mode ( void )
{
    GrisbiWindow *window;
    GrisbiAppConf *conf;

    window = grisbi_app_get_active_window ( NULL );
    conf = grisbi_app_get_conf ( );
    if ( conf->full_screen )
        gtk_window_unfullscreen ( GTK_WINDOW ( window ) );
    else
        gtk_window_fullscreen ( GTK_WINDOW ( window ) );
}

/**
 * show the preferences
 *
 * \param
 *
 * \return
 **/
void gsb_menu_preferences ( GtkAction *action,
                        GrisbiWindow *window )
{
    GThread *thread;

    thread = g_thread_create ( (GThreadFunc) grisb_prefs_show_dialog, window, TRUE, NULL );

    g_thread_join ( thread );
}


/**
 *
 *
 * \param
 *
 * \return
 **/
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
