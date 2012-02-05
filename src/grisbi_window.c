/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2001-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*          2009-2012 Pierre Biava (grisbi@pierre.biava.name)                 */
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

#include <string.h>
#include <unistd.h>

#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "grisbi_window.h"
#include "grisbi_app.h"
#include "grisbi_ui.h"
#include "gsb_dirs.h"
#include "menu.h"
#include "structures.h"
#include "utils_gtkbuilder.h"
/*END_INCLUDE*/


/*START_STATIC*/
static GtkActionGroup *grisbi_window_add_recents_action_group ( GtkUIManager *ui_manager,
                        GrisbiAppConf *conf );
static void grisbi_window_add_recents_sub_menu ( GtkUIManager *ui_manager,
                        gint nb_derniers_fichiers_ouverts );
static void grisbi_window_init_menus ( GrisbiWindow *window );
static gboolean grisbi_window_initialise_builder ( void );
/*END_STATIC*/


#define GRISBI_WINDOW_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GRISBI_TYPE_WINDOW, GrisbiWindowPrivate))

static GtkBuilder *grisbi_window_builder = NULL;

struct _GrisbiWindowPrivate
{
    /* Vbox générale */
    GtkWidget       *main_box;

    /* titre de la fenêtre */
    gchar           *title;

    /* Menus et barres d'outils */
    GtkWidget       *menu_bar;
    GtkUIManager    *ui_manager;
    GtkActionGroup  *always_sensitive_action_group;
    GtkActionGroup  *file_sensitive_action_group;
    GtkActionGroup  *file_recent_files_action_group;
    GtkActionGroup  *file_debug_toggle_action_group;
    GtkActionGroup  *edit_sensitive_action_group;
    GtkActionGroup  *edit_transactions_action_group;
    GtkActionGroup  *view_sensitive_action_group;
};


G_DEFINE_TYPE(GrisbiWindow, grisbi_window, GTK_TYPE_WINDOW)


static void grisbi_window_realized ( GtkWidget *window,
                        gpointer  *data )
{

    /* return */
}


static void grisbi_window_finalize ( GObject *object )
{
    GrisbiWindow *window;
devel_debug (NULL);

    window = GRISBI_WINDOW ( object );

    g_free ( window->priv->title );

    G_OBJECT_CLASS ( grisbi_window_parent_class )->finalize ( object );
}


static void grisbi_window_class_init ( GrisbiWindowClass *klass )
{
    GObjectClass *object_class = G_OBJECT_CLASS ( klass );
/*     GtkWidgetClass *widget_class = GTK_WIDGET_CLASS ( klass );  */

    object_class->finalize = grisbi_window_finalize;

    g_type_class_add_private ( object_class, sizeof ( GrisbiWindowPrivate ) );
}


static void grisbi_window_init ( GrisbiWindow *window )
{
    GtkWidget *main_box;

    window->priv = GRISBI_WINDOW_GET_PRIVATE ( window );

    if ( !grisbi_window_initialise_builder ( ) )
        return;

    /* Création de la fenêtre de dialogue pour le graph */
    main_box = GTK_WIDGET ( gtk_builder_get_object ( grisbi_window_builder, "main_vbox" ) );

    window->priv->main_box = main_box;

    gtk_container_add ( GTK_CONTAINER ( window ), main_box );
    gtk_widget_show ( main_box );

    /* create the menus */
    grisbi_window_init_menus ( window );

    /* set the signals */
    g_signal_connect ( window,
                        "realize",
                        G_CALLBACK ( grisbi_window_realized ),
                        NULL );
}


static void grisbi_window_init_menus ( GrisbiWindow *window )
{
    GrisbiAppConf *conf;
    GtkUIManager *ui_manager;
    GtkActionGroup *actions;
    GError *error = NULL;
    gchar *ui_file;

    ui_manager = gtk_ui_manager_new ( );
    window->priv->ui_manager = ui_manager;

    conf = grisbi_app_get_conf ( );

    /* actions toujours accessibles (sensitives) */
    actions = gtk_action_group_new ( "AlwaysSensitiveActions" );
    gtk_action_group_set_translation_domain ( actions, NULL );
    gtk_action_group_add_actions (actions,
                        always_sensitive_entries,
                        G_N_ELEMENTS ( always_sensitive_entries ),
                        window );

    gtk_ui_manager_insert_action_group ( ui_manager, actions, 0 );
    g_object_unref ( actions );
    window->priv->always_sensitive_action_group = actions;

    /* Actions pour la gestion des fichiers sensitives */
    actions = gtk_action_group_new ( "FileSensitiveActions" );
    gtk_action_group_set_translation_domain ( actions, NULL );
    gtk_action_group_add_actions (actions,
                        file_sensitive_entries,
                        G_N_ELEMENTS ( file_sensitive_entries ),
                        window );

    gtk_ui_manager_insert_action_group ( ui_manager, actions, 0 );
    g_object_unref ( actions );
    window->priv->file_sensitive_action_group = actions;
    gtk_action_group_set_sensitive ( actions, FALSE );

    /* Actions pour la gestion des fichiers récents */
    actions = gtk_action_group_new ( "FileRecentFilesAction" );
    gtk_action_group_set_translation_domain ( actions, NULL );
    gtk_action_group_add_actions (actions,
                        file_recent_files_entrie,
                        G_N_ELEMENTS ( file_recent_files_entrie ),
                        window );

    gtk_ui_manager_insert_action_group ( ui_manager, actions, 0 );
    g_object_unref ( actions );
    gtk_action_group_set_sensitive ( actions, TRUE );

    /* DebugToggle_Action sensitive */
    actions = gtk_action_group_new ( "FileDebugToggleAction" );
    gtk_action_group_set_translation_domain ( actions, NULL );
    gtk_action_group_add_toggle_actions ( actions,
                        file_debug_toggle_entrie,
                        G_N_ELEMENTS ( file_debug_toggle_entrie ),
                        NULL );

    gtk_ui_manager_insert_action_group ( ui_manager, actions, 0 );
    g_object_unref ( actions );
    window->priv->file_debug_toggle_action_group = actions;
    gtk_action_group_set_sensitive ( actions, FALSE );

    /* actions du menu Edit sensitives */
    actions = gtk_action_group_new ( "EditSensitiveActions" );
    gtk_action_group_set_translation_domain ( actions, NULL );
    gtk_action_group_add_actions (actions,
                        edit_sensitive_entries,
                        G_N_ELEMENTS ( edit_sensitive_entries ),
                        window );

    gtk_ui_manager_insert_action_group ( ui_manager, actions, 0 );
    g_object_unref ( actions );
    window->priv->edit_sensitive_action_group = actions;
    gtk_action_group_set_sensitive ( actions, FALSE );

    /* actions propres aux transactions */
    actions = gtk_action_group_new ( "EditTransactionsActions" );
    gtk_action_group_set_translation_domain ( actions, NULL );
    gtk_action_group_add_actions (actions,
                        edit_transactions_entries,
                        G_N_ELEMENTS ( edit_transactions_entries ),
                        window );

    gtk_ui_manager_insert_action_group ( ui_manager, actions, 0 );
    g_object_unref ( actions );
    window->priv->edit_transactions_action_group = actions;
    gtk_action_group_set_sensitive ( actions, FALSE );

    /* actions du menu View sensitives */
    actions = gtk_action_group_new ( "ViewSensitiveActions" );
    gtk_action_group_set_translation_domain ( actions, NULL );
    gtk_action_group_add_actions (actions,
                        view_init_width_col_entrie,
                        G_N_ELEMENTS ( view_init_width_col_entrie ),
                        window );

    gtk_action_group_add_radio_actions ( actions,
                        view_radio_entries,
                        G_N_ELEMENTS ( view_radio_entries ),
                        -1,
                        G_CALLBACK ( gsb_gui_toggle_line_view_mode ),
                        NULL );

    gtk_action_group_add_toggle_actions ( actions,
                        view_toggle_entries,
                        G_N_ELEMENTS ( view_toggle_entries ),
                        NULL );

    gtk_ui_manager_insert_action_group ( ui_manager, actions, 0 );
    g_object_unref ( actions );
    window->priv->view_sensitive_action_group = actions;
    gtk_action_group_set_sensitive ( actions, FALSE );

    /* now load the UI definition */
    ui_file = g_build_filename ( gsb_dirs_get_ui_dir ( ), "grisbi_ui.xml", NULL );
    gtk_ui_manager_add_ui_from_file ( ui_manager, ui_file, &error );
    if ( error != NULL )
    {
        g_warning ( "Could not merge %s: %s", ui_file, error->message );
        g_error_free ( error );
    }
    g_free ( ui_file );

    /* on ajoute les derniers fichiers utilisés */
    if ( conf->nb_derniers_fichiers_ouverts && conf->nb_max_derniers_fichiers_ouverts )
    {
        actions = grisbi_window_add_recents_action_group ( ui_manager, conf );
        grisbi_window_add_recents_sub_menu ( ui_manager, conf->nb_derniers_fichiers_ouverts );
        window->priv->file_recent_files_action_group = actions;
    }

#ifndef GTKOSXAPPLICATION
    gtk_window_add_accel_group ( GTK_WINDOW ( window ),
                        gtk_ui_manager_get_accel_group ( ui_manager ) );
#endif /* !GTKOSXAPPLICATION */

    window->priv->menu_bar = gtk_ui_manager_get_widget ( ui_manager, "/menubar" );
    gtk_box_pack_start ( GTK_BOX ( window->priv->main_box ),  window->priv->menu_bar, FALSE, TRUE, 0 );

    /* return */
}


/**
 *
 *
 *
 *
 **/
GtkUIManager *grisbi_window_get_ui_manager ( GrisbiWindow *window )
{
    return window->priv->ui_manager;
}


GtkActionGroup *grisbi_window_get_action_group ( GrisbiWindow *window,
                        const gchar *action_group_name )
{
    if ( strcmp ( action_group_name, "AlwaysSensitiveActions" ) == 0 )
        return window->priv->edit_sensitive_action_group;
    else if ( strcmp ( action_group_name, "FileSensitiveActions" ) == 0 )
        return window->priv->file_sensitive_action_group;
    else if ( strcmp ( action_group_name, "FileRecentFilesAction" ) == 0 )
        return window->priv->file_recent_files_action_group;
    else if ( strcmp ( action_group_name, "FileDebugToggleAction" ) == 0 )
        return window->priv->file_debug_toggle_action_group;
    else if ( strcmp ( action_group_name, "EditSensitiveActions" ) == 0 )
        return window->priv->edit_sensitive_action_group;
    else if ( strcmp ( action_group_name, "EditTransactionsActions" ) == 0 )
        return window->priv->edit_transactions_action_group;
    else if ( strcmp ( action_group_name, "ViewSensitiveActions" ) == 0 )
        return window->priv->view_sensitive_action_group;
    else
        return NULL;
}


/**
 * set window title
 *
 *
 *
 * */
void grisbi_window_set_title ( GrisbiApp *app,
                        const gchar *title )
{
    GrisbiWindow *window;

    window = grisbi_app_get_active_window ( app );

    if ( window->priv->title )
        g_free ( window->priv->title );

    gtk_window_set_title ( GTK_WINDOW ( window ), title );
}


/**
 * Crée un builder et récupère les widgets du fichier grisbi.ui
 *
 *
 *
 * */
gboolean grisbi_window_initialise_builder ( void )
{
    /* Creation d'un nouveau GtkBuilder */
    grisbi_window_builder = gtk_builder_new ( );
    if ( grisbi_window_builder == NULL )
        return FALSE;

    /* récupère les widgets */
    if ( utils_gtkbuilder_merge_ui_data_in_builder ( grisbi_window_builder, "grisbi.ui" ) )
        return TRUE;
    else
        return FALSE;
}


/**
 * Add menu items to the action_group "FileRecentFilesGroupAction"
 *
 * \param
 *
 * \return
 **/
static GtkActionGroup *grisbi_window_add_recents_action_group ( GtkUIManager *ui_manager,
                        GrisbiAppConf *conf )
{
    GtkActionGroup *actions;
    gint i;

    devel_debug (NULL);

    actions = gtk_action_group_new ( "FileRecentFilesGroupAction" );
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
        gtk_action_group_add_action ( actions, action );
    }

    gtk_ui_manager_insert_action_group ( ui_manager, actions, 1 );
    g_object_unref ( actions );

    return actions;
}


/**
 * Add menu items to the action_group "FileRecentFilesGroupAction".
 *
 * \param
 *
 * \return
 **/
static void grisbi_window_add_recents_sub_menu ( GtkUIManager *ui_manager,
                        gint nb_derniers_fichiers_ouverts )
{
    gint recent_files_merge_id = -1;
    gint i;

    devel_debug (NULL);

    recent_files_merge_id = gtk_ui_manager_new_merge_id ( ui_manager );

    for ( i=0 ; i < nb_derniers_fichiers_ouverts ; i++ )
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
}


/**
 *
 *
 *
 *
 **/

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
