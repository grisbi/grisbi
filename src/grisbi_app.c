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
#include "grisbi_app.h"
#include "gsb_color.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "gsb_file_config.h"
#include "gsb_select_icon.h"
#include "menu.h"
#include "traitement_variables.h"
#include "erreur.h"
/*END_INCLUDE*/


/*START_STATIC*/
/*END_STATIC*/

/* mutex for GrisbiAppConf */
static GMutex *grisbi_app_conf_mutex = NULL;

#define GRISBI_APP_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GRISBI_TYPE_APP, GrisbiAppPrivate))

struct _GrisbiAppPrivate
{
    GList               *windows;
    GrisbiWindow        *active_window;

    GrisbiAppConf       *conf;
    gint                first_use;
};


G_DEFINE_TYPE(GrisbiApp, grisbi_app, G_TYPE_OBJECT)


/**
 * finalise GrisbiApp
 *
 * \param object
 *
 * \return
 */
static void grisbi_app_finalize ( GObject *object )
{
    GrisbiApp *app = GRISBI_APP ( object );
    GrisbiAppConf *conf;
    devel_debug (NULL);

    /* libération mémoire de la liste des fenêtres */
    g_list_free ( app->priv->windows );

    /* libération mémoire de la structure conf */
    conf = app->priv->conf;
    g_free ( conf->account_files_path );
    g_free ( conf->backup_path );
    g_free ( conf->font_string );
    g_free ( conf->browser_command );
    g_strfreev ( conf->tab_noms_derniers_fichiers_ouverts );
    g_free ( app->priv->conf );

    /* libération mémoire de grisbi_app_conf_mutex */
    g_mutex_trylock ( grisbi_app_conf_mutex );
    g_mutex_unlock ( grisbi_app_conf_mutex );
    g_mutex_free ( grisbi_app_conf_mutex );

    /* libération de l'objet app */
    G_OBJECT_CLASS ( grisbi_app_parent_class )->finalize ( object );
}


/**
 * Initialise GrisbiAppClass
 *
 * \param app
 *
 * \return
 */
static void grisbi_app_class_init ( GrisbiAppClass *klass )
{
    GObjectClass *object_class = G_OBJECT_CLASS ( klass );

    object_class->finalize = grisbi_app_finalize;

    g_type_class_add_private ( object_class, sizeof( GrisbiAppPrivate ) );
}


/**
 * GWeakNotify callback function
 *
 * \param
 *
 * \return
 */
static void grisbi_app_weak_notify ( gpointer data,
                        GObject *where_the_app_was )
{
    devel_debug (NULL);

    gtk_main_quit ();
}


/**
 * set active window.
 *
 * \param app
 * \param window
 *
 * \return
 */
static void grisbi_app_set_active_window (GrisbiApp *app,
                   GrisbiWindow *window)
{
    app->priv->active_window = window;
}


/**
 * updates active_window when a new toplevel receives focus.
 *
 * \param window
 * \param event
 * \param app
 *
 * \return TRUE
 * */
static gboolean grisbi_app_window_focus_in_event ( GrisbiWindow *window,
                        GdkEventFocus *event,
                        GrisbiApp *app )
{
    if ( !GRISBI_IS_WINDOW ( window ) )
        return FALSE;

    grisbi_app_set_active_window ( app, window );

    return FALSE;
}


/**
 * exit the gtk main loop when the main window is destroyed.
 *
 * \param window
 * \param event
 * \param app
 *
 * \return TRUE
 * */
static gboolean grisbi_app_window_delete_event ( GrisbiWindow *window,
                        GdkEvent *event,
                        GrisbiApp *app )
{
    devel_debug (NULL);

    gtk_widget_destroy ( GTK_WIDGET  (window ) );

    return TRUE;
}


/**
 * charge les raccourcis claviers.
 *
 * \param
 *
 * \return
 */
static void grisbi_app_load_accels ( void )
{
    gchar *accel_filename = NULL;
    gchar *msg = NULL;

    accel_filename = g_build_filename ( gsb_dirs_get_user_config_dir (), "grisbi-accels", NULL );
    if ( accel_filename )
    {
        gtk_accel_map_load ( accel_filename );
        g_free ( accel_filename );
    }
    else
        msg = g_strdup_printf ( "Error loading keyboard shortcuts\n" );

    if ( msg )
    {
        notice_debug ( msg );
        g_free ( msg );
    }
}


/**
 * sauvegarde les raccourcis claviers.
 *
 * \param
 *
 * \return
 */
static void grisbi_app_save_accels ( void )
{
    gchar *accel_filename;
    gchar *msg = NULL;

    accel_filename = g_build_filename ( gsb_dirs_get_user_config_dir (), "grisbi-accels", NULL );
    if ( accel_filename )
    {
        gtk_accel_map_save ( accel_filename );
        g_free ( accel_filename );
    }
    else
        msg = g_strdup_printf ( "Error saving keyboard shortcuts\n" );

    if ( msg )
    {
        notice_debug ( msg );
        g_free ( msg );
    }
}


/**
 * exit the gtk main loop when the main window is destroyed.
 *
 * \param window
 * \param app
 *
 * \return TRUE
 * */
static void grisbi_app_window_destroy ( GrisbiWindow *window,
                        GrisbiApp *app )
{
    devel_debug (NULL);

    app->priv->windows = g_list_remove ( app->priv->windows, window );

    if ( window == app->priv->active_window )
    {
        if ( app->priv->windows != NULL )
            app->priv->active_window = app->priv->windows->data ;
        else
            app->priv->active_window = NULL;
    }

    if ( app->priv->windows == NULL )
    {
        grisbi_app_save_accels ();
        g_object_unref ( app );
    }
}


/**
 * initialises les paramètres de Grisbi.
 *
 * \param app
 *
 * \return TRUE if first use
 */
static void grisbi_app_load_config_var ( GrisbiApp *app )
{
    /* initialisation de la variable conf */
    app->priv->conf = g_malloc0 ( sizeof ( GrisbiAppConf ) );

    /* chargement des paramètres communs */
    app->priv->first_use = !gsb_file_config_load_config ( app->priv->conf );
}

 
/**
 * set size and position of the main window of grisbi.
 *
 * \param window
 *
 * \return
 *
 * */
static void grisbi_app_window_set_size_and_position ( GrisbiWindow *window )
{
    GrisbiApp *app;
    GrisbiAppConf *conf;

    app = grisbi_app_get_default ( FALSE );
    conf = app->priv->conf;

    /* set the size of the window */
    if ( conf->main_width && conf->main_height )
        gtk_window_set_default_size ( GTK_WINDOW ( window ), conf->main_width, conf->main_height );
    else
        gtk_window_set_default_size ( GTK_WINDOW ( window ), 900, 600 );

    /* display window at position */
    gtk_window_move ( GTK_WINDOW ( window ), conf->root_x, conf->root_y );

    /* set the full screen if necessary */
    if ( conf->full_screen )
        gtk_window_fullscreen ( GTK_WINDOW ( window ) );

    /* put up the screen if necessary */
    if ( conf->maximize_screen )
        gtk_window_maximize ( GTK_WINDOW ( window ) );
}


/**
 * Init conf mutex
 *
 * \param
 *
 * \return
 **/
static void grisbi_app_init_conf_mutex ( void )
{
    g_assert ( grisbi_app_conf_mutex == NULL );
    grisbi_app_conf_mutex = g_mutex_new ();
}


/* CREATE OBJECT */
/**
 * Initialise GrisbiApp
 *
 * \param app
 *
 * \return
 */
static void grisbi_app_init ( GrisbiApp *app )
{
    devel_debug (NULL);
    app->priv = GRISBI_APP_GET_PRIVATE ( app );

    /* charge les raccourcis claviers */
    grisbi_app_load_accels ();

    /* initialisation des paramètres de l'application */
    grisbi_app_init_conf_mutex ();
    g_mutex_lock ( grisbi_app_conf_mutex );
    grisbi_app_load_config_var ( app );
    g_mutex_unlock ( grisbi_app_conf_mutex );

    /* initialisation des couleurs par défaut */
    gsb_color_initialise_couleurs_par_defaut ();

    /* return */
}


/* PUBLIC FUNCTIONS */
/**
 * Création d'une fenêtre GrisbiWindow.
 *
 * \param app
 * \param screen
 *
 * \return une fenêtre pour Grisbi
 */
GrisbiWindow *grisbi_app_create_window ( GrisbiApp *app,
                        GdkScreen *screen )
{
    GrisbiWindow *window;
    gchar *string;

    if ( app->priv->windows == NULL )
    {
        window = g_object_new ( GRISBI_TYPE_WINDOW, NULL );
        grisbi_app_set_active_window ( app, window );
    }
    else
    {
        window = g_object_new ( GRISBI_TYPE_WINDOW, NULL );
    }

    app->priv->windows = g_list_prepend ( app->priv->windows, window );

    grisbi_app_window_set_size_and_position ( window );

    /* create the icon of grisbi (set in the panel of gnome or other) */
    string = g_build_filename ( gsb_dirs_get_pixmaps_dir (), "grisbi.svg", NULL );
    if ( g_file_test ( string, G_FILE_TEST_EXISTS ) )
    {
        GdkPixbuf *pixbuf = NULL;

        pixbuf = gsb_select_icon_get_default_logo_pixbuf ();
        gtk_window_set_icon ( GTK_WINDOW ( window ), pixbuf );
    }
    g_free (string);

    g_signal_connect ( window,
                        "focus_in_event",
                        G_CALLBACK ( grisbi_app_window_focus_in_event ),
                        app );

    g_signal_connect ( window,
                        "delete_event",
                        G_CALLBACK ( grisbi_app_window_delete_event ),
                        app );

    g_signal_connect ( window,
                        "destroy",
                        G_CALLBACK ( grisbi_app_window_destroy ),
                        app );

    if ( screen != NULL )
        gtk_window_set_screen ( GTK_WINDOW ( window ), screen );

    return window;
}


/**
 * close the file
 * if no file loaded or no change, directly return TRUE
 *
 * \param
 *
 * \return FALSE if problem, TRUE if ok
 * */
gboolean grisbi_app_close_file ( void )
{
    GrisbiApp *app;
    GrisbiWindow *main_window;
    GrisbiAppConf *conf;
    GrisbiWindowEtat *etat;

    devel_debug (NULL);

    app = grisbi_app_get_default ( FALSE );
    conf = grisbi_app_get_conf ();
    etat = grisbi_window_get_struct_etat ();

    /* on récupère la fenetre active */
    main_window = grisbi_app_get_active_window ( app );

    /* On ferme le fichier de la fenêtre active */
    gsb_file_close ();

    /* on libère la mémoire utilisée par la fenêtre active */
    grisbi_window_free_priv_file ( main_window );

    /* initialisation de la structure etat */
    grisbi_window_init_struct_etat ( main_window );

    gtk_widget_hide ( grisbi_window_get_widget_by_name ( "vbox_general" ) );
    gtk_widget_hide ( grisbi_window_get_headings_eb ( main_window ) );
    gtk_widget_show ( grisbi_window_get_accueil_page ( main_window ) );

    /* grise les menus nécessaires */
    gsb_menu_sensitive ( FALSE );
    gsb_gui_sensitive_menu_item ( "/menubar/ViewMenu/ShowClosed", FALSE );
    gsb_gui_sensitive_menu_item ( "/menubar/ViewMenu/InitWidthCol", FALSE );

    return TRUE;
}


/**
 * retourne le nom du fichier de la fenêtre active
 *
 * \param
 *
 * \return
 **/
const gchar *grisbi_app_get_active_filename ( void )
{
    GrisbiApp *app;
    GrisbiWindow *window;

    app = grisbi_app_get_default ( TRUE );
    if ( app )
    {
        window = grisbi_app_get_active_window ( app );

        return grisbi_window_get_filename ( window );
    }
    else
        return NULL;
}


/**
 * définit le nom du fichier associé à la fenêtre active
 *
 * \param filename
 *
 * \return TRUE
 **/
gboolean grisbi_app_set_active_filename ( const gchar *filename )
{
    GrisbiApp *app;
    GrisbiWindow *window;

    app = grisbi_app_get_default ( FALSE );
    window = grisbi_app_get_active_window ( app );

    grisbi_window_set_filename ( window, filename );

    return TRUE;
}


/**
 * retourne le titre du fichier de la fenêtre active
 *
 * \param
 *
 * \return
 **/
const gchar *grisbi_app_get_active_file_title ( void )
{
    GrisbiApp *app;
    GrisbiWindow *window;

    app = grisbi_app_get_default ( FALSE );
    window = grisbi_app_get_active_window ( app );

    return grisbi_window_get_file_title ( window );
}


/**
 * définit le titre du fichier associé à la fenêtre active
 *
 * \param file_title
 *
 * \return TRUE
 **/
gboolean grisbi_app_set_active_file_title ( const gchar *file_title )
{
    GrisbiApp *app;
    GrisbiWindow *window;

    app = grisbi_app_get_default ( FALSE );
    window = grisbi_app_get_active_window ( app );

    grisbi_window_set_file_title ( window, file_title );

    return TRUE;
}


/**
 * retourne le ui_manager pour la gestion des menus et barres d'outils
 *
 * \param
 *
 * \return
 **/
GtkUIManager *grisbi_app_get_active_ui_manager ( void )
{
    GrisbiApp *app;
    GrisbiWindow *window;
    GtkUIManager *ui_manager;

    app = grisbi_app_get_default ( FALSE );
    window = grisbi_app_get_active_window ( app );

    ui_manager = grisbi_window_get_ui_manager ( window );

    return ui_manager;
}


/**
 * get active window.
 *
 * \param app
 *
 * \return active_window
 */
GrisbiWindow *grisbi_app_get_active_window ( GrisbiApp *app )
{
    GrisbiApp *tmp_app;

    if ( app == NULL )
        tmp_app = grisbi_app_get_default ( TRUE );
    else
        tmp_app = app;

    if ( tmp_app == NULL || !GRISBI_IS_APP ( tmp_app ) )
        return NULL;

    if ( tmp_app->priv->active_window != 0
     &&
     !gtk_widget_get_realized ( ( GTK_WIDGET ( tmp_app->priv->active_window ) ) ) )
        gtk_widget_realize ( GTK_WIDGET ( tmp_app->priv->active_window ) );

    return tmp_app->priv->active_window;
}


/**
 * retourne la structure de configuration de Grisbi
 *
 * \param app
 *
 * \return conf
 *
 **/
GrisbiAppConf *grisbi_app_get_conf ( void )
{
    GrisbiApp *app;

    app = grisbi_app_get_default ( TRUE );
    if ( app )
        return app->priv->conf;
    else
        return NULL;
}


/**
 * Returns the GrisbiApp object.
 *
 * \param
 *
 * \return the GrisbiApp pointer
 */
GrisbiApp *grisbi_app_get_default ( gboolean return_null )
{
    static GrisbiApp *app = NULL;

    if ( app != NULL )
        return app;

    if ( return_null )
        return NULL;

    app = GRISBI_APP ( g_object_new ( GRISBI_TYPE_APP, NULL ) );

    g_object_add_weak_pointer ( G_OBJECT ( app ), (gpointer) &app );
    g_object_weak_ref ( G_OBJECT ( app ), grisbi_app_weak_notify, NULL );

    return app;
}


/**
 * get_first_use.
 *
 * \param app
 *
 * \return TRUE if first use
 */
gboolean grisbi_app_get_first_use ( GrisbiApp *app )
{
    return app->priv->first_use;

}

 
/**
 * close grisbi by destroying the main window
 * This function is called by the Quit menu option.
 *
 * \param
 *
 * \return FALSE
 * */
gboolean grisbi_app_quit ( void )
{
    GrisbiApp *app;
    GrisbiWindow *window;
    GrisbiAppConf *conf;

    devel_debug (NULL);

    app = grisbi_app_get_default ( FALSE );
    conf = app->priv->conf;

    /* on récupère la fenetre active */
    window = grisbi_app_get_active_window ( app );

    /* sauvegarde la position de la fenetre principale */
    if ( conf->full_screen == 0 && conf->maximize_screen == 0 )
        gtk_window_get_position ( GTK_WINDOW ( window ), &conf->root_x, &conf->root_y );

    /* sauvegarde de la taille de la fenêtre si nécessaire */
    if ( conf->full_screen == 0 && conf->maximize_screen == 0 )
        gtk_window_get_size ( GTK_WINDOW ( window ), &conf->main_width, &conf->main_height );

    g_mutex_lock ( grisbi_app_conf_mutex );
    gsb_file_config_save_config ( conf );
    g_mutex_unlock ( grisbi_app_conf_mutex );

    if ( gsb_file_close () )
        gtk_widget_destroy ( GTK_WIDGET ( window ) );

    /* clean finish of the debug file */
    if ( gsb_debug_get_debug_mode () )
        gsb_debug_finish_log ();

    return FALSE;
}


/**
 * lock conf_mutex
 *
 * \param
 *
 * \return
 **/
void grisbi_app_conf_mutex_lock ( void )
{
    g_mutex_lock ( grisbi_app_conf_mutex );
}


/**
 * unlock conf_mutex
 *
 * \param
 *
 * \return
 **/
void grisbi_app_conf_mutex_unlock ( void )
{
    g_mutex_unlock ( grisbi_app_conf_mutex );
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
