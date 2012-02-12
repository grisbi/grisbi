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
#include "gsb_plugins.h"
#include "import.h"
#include "traitement_variables.h"
#include "erreur.h"
/*END_INCLUDE*/


/*START_STATIC*/
static void grisbi_app_class_init ( GrisbiAppClass *klass );
static void grisbi_app_finalize ( GObject *object );
static void grisbi_app_init ( GrisbiApp *app );
static void grisbi_app_load_accels ( void );
static void grisbi_app_load_config_var ( GrisbiApp *app );
static void grisbi_app_load_import_formats ( GrisbiApp *app );
static void grisbi_app_save_accels ( void );
static void grisbi_app_set_active_window (GrisbiApp *app,
                   GrisbiWindow *window);
static void grisbi_app_weak_notify ( gpointer data,
                        GObject *where_the_app_was );
static gboolean grisbi_app_window_delete_event ( GrisbiWindow *window,
                        GdkEvent *event,
                        GrisbiApp *app );
static void grisbi_app_window_destroy ( GrisbiWindow *window,
                        GrisbiApp *app );
static gboolean grisbi_app_window_focus_in_event ( GrisbiWindow *window,
                        GdkEventFocus *event,
                        GrisbiApp *app );
static void grisbi_app_window_set_size_and_position ( GrisbiWindow *window );
/*END_STATIC*/


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

    /* libération mémoire de la configuration de grisbi */
    conf = app->priv->conf;
    g_free ( conf->font_string );
    g_free ( conf->browser_command );
    g_strfreev ( conf->tab_noms_derniers_fichiers_ouverts );
    g_free ( app->priv->conf );

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
 * Initialise GrisbiApp
 *
 * \param app
 *
 * \return
 */
static void grisbi_app_init ( GrisbiApp *app )
{
    gchar *string;

    app->priv = GRISBI_APP_GET_PRIVATE ( app );

    /* charge les raccourcis claviers */
    grisbi_app_load_accels ( );

    /* create the icon of grisbi (set in the panel of gnome or other) */
    string = g_build_filename ( gsb_dirs_get_pixmaps_dir ( ), "grisbi-logo.png", NULL );
    if ( g_file_test ( string, G_FILE_TEST_EXISTS ) )
        gtk_window_set_default_icon_from_file ( string, NULL );
    g_free (string);

    /* initialisation des paramètres de l'application */
    grisbi_app_load_config_var ( app );

    /* importation des formats de fichiers */
    grisbi_app_load_import_formats ( app );

    /* return */
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
 * Returns the GrisbiApp object.
 *
 * \param
 *
 * \return the GrisbiApp pointer
 */
GrisbiApp *grisbi_app_get_default ( void )
{
    static GrisbiApp *app = NULL;

    if ( app != NULL )
        return app;

    app = GRISBI_APP ( g_object_new ( GRISBI_TYPE_APP, NULL ) );

    g_object_add_weak_pointer ( G_OBJECT ( app ), (gpointer) &app );
    g_object_weak_ref ( G_OBJECT ( app ), grisbi_app_weak_notify, NULL );

    return app;
}


GrisbiWindow *grisbi_app_get_active_window ( GrisbiApp *app )
{
    GrisbiApp *tmp_app;

    if ( app == NULL )

        tmp_app = grisbi_app_get_default ( );
    else
        tmp_app = app;

    if ( !GRISBI_IS_APP ( tmp_app ) )
        return NULL;

    if ( !GTK_WIDGET_REALIZED ( GTK_WIDGET ( tmp_app->priv->active_window ) ) )
        gtk_widget_realize ( GTK_WIDGET ( tmp_app->priv->active_window ) );

    return tmp_app->priv->active_window;
}


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
    devel_debug (NULL);

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
        grisbi_app_save_accels ( );
        g_object_unref ( app );
    }
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
    gchar *msg;

    accel_filename = g_build_filename ( gsb_dirs_get_user_config_dir ( ), "grisbi-accels", NULL );
    if ( accel_filename )
    {
        gtk_accel_map_load ( accel_filename );
        msg = g_strdup_printf ( "Loading keybindings from %s\n", accel_filename );
        g_free ( accel_filename );
    }
    else
        msg = g_strdup_printf ( "Error loading keyboard shortcuts\n" );

        notice_debug ( msg );
        g_free ( msg );
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
    gchar *msg;

    accel_filename = g_build_filename ( gsb_dirs_get_user_config_dir ( ), "grisbi-accels", NULL );
    if ( accel_filename )
    {
        msg = g_strdup_printf ( "Saving keybindings in %s\n", accel_filename);
        gtk_accel_map_save ( accel_filename );
        g_free ( accel_filename );
    }
    else
        msg = g_strdup_printf ( "Error saving keyboard shortcuts\n" );

        notice_debug ( msg );
        g_free ( msg );
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

    /* initialisation des couleurs par défaut */
    gsb_color_initialise_couleurs_par_defaut ( );

    /* chargement des paramètres communs */
    app->priv->first_use = !gsb_file_config_load_config ( app->priv->conf );

    /* à reprendre autrement */
    init_variables ();

}

 
/**
 * initialises les formats de fichiers pour l'importation.
 *
 * \param app
 *
 * \return TRUE if first use
 */
static void grisbi_app_load_import_formats ( GrisbiApp *app )
{
    /* importation des formats de fichiers */
#ifdef HAVE_PLUGINS
    gsb_plugins_scan_dir ( gsb_dirs_get_plugins_dir ( ) );
#endif
    register_import_formats ();
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

    app = grisbi_app_get_default ( );
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

    app = grisbi_app_get_default ( );
    return app->priv->conf;
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

    app = grisbi_app_get_default ( );
    conf = grisbi_app_get_conf ( );

    /* on récupère la fenetre active */
    window = grisbi_app_get_active_window ( app );

    /* sauvegarde la position de la fenetre principale */
    if ( conf->full_screen == 0 && conf->maximize_screen == 0 )
        gtk_window_get_position ( GTK_WINDOW ( window ), &conf->root_x, &conf->root_y );

    /* sauvegarde de la taille de la fenêtre si nécessaire */
    if ( conf->full_screen == 0 && conf->maximize_screen == 0 )
        gtk_window_get_size ( GTK_WINDOW ( window ), &conf->main_width, &conf->main_height );

    gsb_file_config_save_config ( conf );

    if ( gsb_file_close ( ) )
        gtk_widget_destroy ( GTK_WIDGET ( window ) );

    /* clean finish of the debug file */
    if ( etat.debug_mode )
        gsb_debug_finish_log ( );

    return FALSE;
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

    devel_debug (NULL);

    app = grisbi_app_get_default ( );
    conf = grisbi_app_get_conf ( );

    /* on récupère la fenetre active */
    main_window = grisbi_app_get_active_window ( app );





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

    app = grisbi_app_get_default ( );
    window = grisbi_app_get_active_window ( app );

    return grisbi_window_get_filename ( window );
}


/**
 * définit le nom du fichier associé à la fenêtre
 *
 * \param filename
 *
 * \return TRUE
 **/
gboolean grisbi_app_set_active_filename ( const gchar *filename )
{
    GrisbiApp *app;
    GrisbiWindow *window;

    app = grisbi_app_get_default ( );
    window = grisbi_app_get_active_window ( app );

    grisbi_window_set_filename ( window, filename );

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

    app = grisbi_app_get_default ( );
    window = grisbi_app_get_active_window ( app );

    ui_manager = grisbi_window_get_ui_manager ( window );

    return ui_manager;
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
