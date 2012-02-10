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
#include <gdk/gdkkeysyms.h>

/*START_INCLUDE*/
#include "grisbi_window.h"
#include "grisbi_app.h"
#include "grisbi_ui.h"
#include "gsb_dirs.h"
#include "menu.h"
#include "navigation.h"
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
static GtkWidget *grisbi_window_new_headings_eb ( GrisbiWindow *window );
static GtkWidget *grisbi_window_new_statusbar ( GrisbiWindow *window );
/*END_STATIC*/


#define GRISBI_WINDOW_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GRISBI_TYPE_WINDOW, GrisbiWindowPrivate))

static GtkBuilder *grisbi_window_builder = NULL;

struct _GrisbiWindowPrivate
{
    /* Vbox générale */
    GtkWidget       *main_box;

    /* nom du fichier associé à la fenêtre */
    gchar           *filename;

    /* titre de la fenêtre */
    gchar           *title;

    /* Menus et barres d'outils */
    GtkWidget       *menu_bar;
    GtkUIManager    *ui_manager;
    GtkActionGroup  *always_sensitive_action_group;
    GtkActionGroup  *division_sensitive_action_group;
    GtkActionGroup  *file_save_action_group;
    GtkActionGroup  *file_recent_files_action_group;
    GtkActionGroup  *file_debug_toggle_action_group;
    GtkActionGroup  *edit_sensitive_action_group;
    GtkActionGroup  *edit_transactions_action_group;
    GtkActionGroup  *view_sensitive_action_group;

    /* statusbar */
    GtkWidget       *statusbar;
    guint           context_id;
    guint           message_id;

    /* headings_bar */
    GtkWidget       *headings_eb;

    /* vbox_geneal */
    GtkWidget       *vbox_general;

    /* hpaned */
    GtkWidget       *hpaned_general;
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
    g_free ( window->priv->filename );

    G_OBJECT_CLASS ( grisbi_window_parent_class )->finalize ( object );
}


static gboolean grisbi_window_key_press_event ( GtkWidget *widget,
                        GdkEventKey *event,
                        gpointer data )
{
    GrisbiAppConf *conf;

    switch ( event -> keyval )
    {
        case GDK_KEY_F11 :
            conf = grisbi_app_get_conf ( );
            if ( conf->full_screen )
                gtk_window_unfullscreen ( GTK_WINDOW ( widget ) );
            else
                gtk_window_fullscreen ( GTK_WINDOW ( widget ) );
        break;
    }

    return FALSE;
}


static gboolean grisbi_window_state_event ( GtkWidget *widget,
                        GdkEventWindowState *event )
{
    GrisbiWindow *window = GRISBI_WINDOW ( widget );
    GrisbiAppConf *conf;
    gboolean show;

    if ( event->changed_mask & GDK_WINDOW_STATE_MAXIMIZED )
    {
        show = !( event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED );

        gtk_statusbar_set_has_resize_grip ( GTK_STATUSBAR ( window->priv->statusbar ), show );
        conf = grisbi_app_get_conf ( );
        conf->maximize_screen = !show;
    }
    else if ( event->changed_mask & GDK_WINDOW_STATE_FULLSCREEN )
    {
        show = !( event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN );

        gtk_statusbar_set_has_resize_grip ( GTK_STATUSBAR ( window->priv->statusbar ), show );
        conf = grisbi_app_get_conf ( );
        conf->full_screen = !show;
    }

    return FALSE;
}


static void grisbi_window_class_init ( GrisbiWindowClass *klass )
{
    GObjectClass *object_class = G_OBJECT_CLASS ( klass );
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS ( klass );

    object_class->finalize = grisbi_window_finalize;
    widget_class->window_state_event = grisbi_window_state_event;

    g_type_class_add_private ( object_class, sizeof ( GrisbiWindowPrivate ) );
}


static void grisbi_window_init ( GrisbiWindow *window )
{
    GtkWidget *main_box;
    GtkWidget *statusbar;
    GtkWidget *headings_eb;

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

    /* create the headings eb */
    headings_eb = grisbi_window_new_headings_eb ( window );
    gtk_box_pack_start ( GTK_BOX ( main_box ), headings_eb, FALSE, FALSE, 0 );

    /* create the statusbar */
    statusbar = grisbi_window_new_statusbar ( window );
    gtk_box_pack_end ( GTK_BOX ( main_box ), statusbar, FALSE, FALSE, 0 );

    /* initialisation de vbox_general */
    window->priv->vbox_general = GTK_WIDGET ( gtk_builder_get_object ( grisbi_window_builder, "vbox_general" ) );

    /* initialisation du hpaned */
    window->priv->hpaned_general = GTK_WIDGET (
                        gtk_builder_get_object ( grisbi_window_builder, "hpaned_general" ) );

    /* set the signals */
    g_signal_connect ( G_OBJECT ( window ),
                        "realize",
                        G_CALLBACK ( grisbi_window_realized ),
                        NULL );

    g_signal_connect ( G_OBJECT ( window ),
                        "key-press-event",
                        G_CALLBACK ( grisbi_window_key_press_event ),
                        NULL );
}


/* MENUS */
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

    /* ShowFullScreenAction sensitive */
    gtk_action_group_add_toggle_actions ( actions,
                        show_full_screen_entrie,
                        G_N_ELEMENTS ( show_full_screen_entrie ),
                        NULL );

    gtk_ui_manager_insert_action_group ( ui_manager, actions, 0 );
    g_object_unref ( actions );
    window->priv->always_sensitive_action_group = actions;

    /* Actions pour la gestion des fichiers sensitives */
    actions = gtk_action_group_new ( "DivisionSensitiveActions" );
    gtk_action_group_set_translation_domain ( actions, NULL );
    gtk_action_group_add_actions (actions,
                        division_sensitive_entries,
                        G_N_ELEMENTS ( division_sensitive_entries ),
                        window );

    gtk_ui_manager_insert_action_group ( ui_manager, actions, 0 );
    g_object_unref ( actions );
    window->priv->division_sensitive_action_group = actions;
    gtk_action_group_set_sensitive ( actions, FALSE );

    /* Actions pour la gestion des fichiers sensitives */
    actions = gtk_action_group_new ( "FileSaveAction" );
    gtk_action_group_set_translation_domain ( actions, NULL );
    gtk_action_group_add_actions (actions,
                        file_save_entries,
                        G_N_ELEMENTS ( file_save_entries ),
                        window );

    gtk_ui_manager_insert_action_group ( ui_manager, actions, 0 );
    g_object_unref ( actions );
    window->priv->file_save_action_group = actions;
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
    else if ( strcmp ( action_group_name, "DivisionSensitiveActions" ) == 0 )
        return window->priv->division_sensitive_action_group;
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


/* TITLE OF MAIN WINDOW */
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


/* GTK_BUILDER */
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
 * retourne le nom du fichier associé à la fenêtre
 *
 * \param GrisbiWindow
 *
 * \return filename
 **/
const gchar *grisbi_window_get_filename ( GrisbiWindow *window )
{
    return window->priv->filename;
}

/**
 * définit le nom du fichier associé à la fenêtre
 *
 * \param GrisbiWindow
 * \param filename
 *
 * \return TRUE
 **/
gboolean grisbi_window_set_filename ( GrisbiWindow *window,
                        const gchar *filename )
{
    devel_debug ( filename );

    g_free ( window->priv->filename );

    window->priv->filename = g_strdup ( filename );

    return TRUE;
}

/**
 * return main_box of window
 *
 * \param GrisbiWindow
 *
 * \return main_box
 **/
GtkWidget *grisbi_window_get_main_box ( GrisbiWindow *window )
{
    return window->priv->main_box;
}


/* STATUS_BAR */
/**
 * Create and return a new GtkStatusBar to hold various status
 * information.
 *
 * \param
 *
 * \return  A newly allocated GtkStatusBar.
 */
static GtkWidget *grisbi_window_new_statusbar ( GrisbiWindow *window )
{
    GtkWidget *statusbar;

    statusbar = GTK_WIDGET ( gtk_builder_get_object ( grisbi_window_builder, "main_statusbar" ) );
    window->priv->statusbar = statusbar;
    window->priv->context_id = gtk_statusbar_get_context_id ( GTK_STATUSBAR ( statusbar ), "Grisbi" );
    window->priv->message_id = -1;

    return statusbar;
}


/**
 * supprime le message de la liste
 *
 * \param GrisbiWindow
 *
 * \return
 **/
void grisbi_window_statusbar_remove ( GrisbiWindow *window )
{
    if ( window->priv->message_id >= 0 )
    {
        gtk_statusbar_remove ( GTK_STATUSBAR ( window->priv->statusbar ),
                        window->priv->context_id,
                        window->priv->message_id );
        window->priv->message_id = -1;
    }

}


/**
 * met un message dans la barre d'état de la fenêtre
 *
 * \param GrisbiWindow
 * \param msg
 *
 * \return
 **/
void grisbi_window_statusbar_push ( GrisbiWindow *window,
                        const gchar *msg )
{
    window->priv->message_id = gtk_statusbar_push ( GTK_STATUSBAR ( window->priv->statusbar ),
                        window->priv->context_id, msg );
}


/* HEADINGS_EB */
/**
 * Trigger a callback functions only if button event that triggered it
 * was a simple click.
 *
 * \param button
 * \param event
 * \param callback function
 *
 * \return  TRUE.
 */
static gboolean grisbi_window_headings_simpleclick_event_run ( GtkWidget *button,
                        GdkEvent *button_event,
                        GCallback callback )
{
    if ( button_event -> type == GDK_BUTTON_PRESS )
    {
        callback ( );
    }

    return TRUE;
}


/**
 * Create and return a new headings_bar
 * information.
 *
 * \param
 *
 * \return  A newly allocated headings_eb.
 */
static GtkWidget *grisbi_window_new_headings_eb ( GrisbiWindow *window )
{
    GtkWidget *headings_eb;
    GtkWidget *arrow_eb;
    GtkStyle *style;

    headings_eb = GTK_WIDGET ( gtk_builder_get_object ( grisbi_window_builder, "headings_eb" ) );
    window->priv->headings_eb = headings_eb;
    style = gtk_widget_get_style ( headings_eb );

    arrow_eb = GTK_WIDGET ( gtk_builder_get_object ( grisbi_window_builder, "arrow_eb_left" ) );
    gtk_widget_modify_bg ( arrow_eb, 0, &(style -> bg[GTK_STATE_ACTIVE]) );
    g_signal_connect ( G_OBJECT ( arrow_eb ),
                        "button-press-event",
                        G_CALLBACK ( grisbi_window_headings_simpleclick_event_run ),
                        gsb_gui_navigation_select_prev );


    arrow_eb = GTK_WIDGET ( gtk_builder_get_object ( grisbi_window_builder, "arrow_eb_right" ) );
    gtk_widget_modify_bg ( arrow_eb, 0, &(style -> bg[GTK_STATE_ACTIVE]) );
    g_signal_connect ( G_OBJECT ( arrow_eb ), "button-press-event",
                        G_CALLBACK ( grisbi_window_headings_simpleclick_event_run ),
                        gsb_gui_navigation_select_next );

    gtk_widget_modify_bg ( headings_eb, 0, &(style -> bg[GTK_STATE_ACTIVE]) );


    return headings_eb;
}


/**
 * retourne headings_eb
 *
 * \param
 *
 * \return
 **/
GtkWidget *grisbi_window_get_headings_eb ( GrisbiWindow *window )
{
    return window->priv->headings_eb;
}


/**
 * Update one of the heading bar label with a new text.
 *
 * \param label Label to update.
 * \param text  String to display in headings bar.
 * \param text  escape string to display in headings bar.
 *
 */
void grisbi_window_headings_update_label_markup ( gchar *label_name,
                        const gchar *text,
                        gboolean escape_text )
{
    GtkWidget *label;
    gchar* tmpstr;

    if ( escape_text )
        tmpstr = g_markup_printf_escaped ("<b>%s</b>", text );
    else
        tmpstr = g_strconcat ( "<b>", text, "</b>", NULL );

    label = GTK_WIDGET ( gtk_builder_get_object ( grisbi_window_builder, label_name ) );

    if ( GTK_IS_LABEL ( label ) )
        gtk_label_set_markup ( GTK_LABEL ( label ), tmpstr );

    g_free ( tmpstr );
}


/* HPANED */
/**
 * met à jour la taille du panneau de gauche
 *
 * \param hpaned
 * \param allocation
 * \param null
 *
 * \return
 **/
static gboolean grisbi_window_hpaned_size_allocate ( GtkWidget *hpaned,
                        GtkAllocation *allocation,
                        gpointer null )
{
    GrisbiAppConf *conf;

    conf = grisbi_app_get_conf ( );
    conf->panel_width = gtk_paned_get_position ( GTK_PANED ( hpaned ) );
    
    return FALSE;

}

/**
 *
 *
 * \param
 *
 * \return
 **/
GtkWidget *grisbi_window_new_hpaned ( GrisbiWindow *window )
{
    GrisbiAppConf *conf;

    conf = grisbi_app_get_conf ( );

    g_signal_connect ( G_OBJECT ( window->priv->hpaned_general ),
                        "size_allocate",
                        G_CALLBACK ( grisbi_window_hpaned_size_allocate ),
                        NULL );
    gtk_container_set_border_width ( GTK_CONTAINER ( window->priv->hpaned_general ), 6 );

    if ( conf->panel_width == -1 )
    {
        gint width, height;

        gtk_window_get_size ( GTK_WINDOW ( grisbi_app_get_active_window ( NULL ) ), &width, &height );
        gtk_paned_set_position ( GTK_PANED ( window->priv->hpaned_general ), (gint) width / 4 );
    }
    else
    {
        if ( conf->panel_width )
            gtk_paned_set_position ( GTK_PANED ( window->priv->hpaned_general ), conf->panel_width );
        else
            gtk_paned_set_position ( GTK_PANED ( window->priv->hpaned_general ), 1 );
    }

    return window->priv->hpaned_general;
}


/**
 * retourne vbox_general
 *
 * \param GrisbiWindow
 *
 * \return vbox_general
 **/
GtkWidget *grisbi_window_get_vbox_general ( GrisbiWindow *window )
{
    return window->priv->vbox_general;
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
