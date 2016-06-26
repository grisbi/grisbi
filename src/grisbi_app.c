/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2015 Pierre Biava (grisbi@pierre.biava.name)                    */
/*          http://www.grisbi.org                                                */
/*                                                                               */
/*     This program is free software; you can redistribute it and/or modify      */
/*     it under the terms of the GNU General Public License as published by      */
/*     the Free Software Foundation; either version 2 of the License, or         */
/*     (at your option) any later version.                                       */
/*                                                                               */
/*     This program is distributed in the hope that it will be useful,           */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*     GNU General Public License for more details.                              */
/*                                                                               */
/*     You should have received a copy of the GNU General Public License         */
/*     along with this program; if not, write to the Free Software               */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                               */
/* *******************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"
#include <gtk/gtk.h>
#include <stdlib.h>
#include <glib/gi18n.h>
#include <errno.h>

#ifdef HAVE_GOFFICE
#include <goffice/goffice.h>
#endif /* HAVE_GOFFICE */

/*START_INCLUDE*/
#include "grisbi_app.h"
#include "gsb_dirs.h"
#include "gsb_file_config.h"
#include "gsb_locale.h"
#include "help.h"
#include "menu.h"
#include "erreur.h"
/*END_INCLUDE*/

 /*START_EXTERN Variables externes PROVISOIRE*/
extern gchar *nom_fichier_comptes;
extern gchar *titre_fichier;
/*END_EXTERN*/

struct _GrisbiApp
{
  GtkApplication parent;
};

struct _GrisbiAppClass
{
  GtkApplicationClass parent_class;
};

typedef struct _GrisbiAppPrivate GrisbiAppPrivate;

struct _GrisbiAppPrivate
{
/*    GrisbiAppConf       *conf;
*/
	gint                first_use;

    /* command line parsing */
    gboolean 			new_window;
    gint 				debug_level;
    gchar 				*geometry;
    GSList 				*file_list;

	/* Menuapp et menubar */
	GMenuModel 			*appmenu;
	GtkRecentManager 	*recent_manager;
};

G_DEFINE_TYPE_WITH_PRIVATE(GrisbiApp, grisbi_app, GTK_TYPE_APPLICATION);

/* MENU APP*/
static GActionEntry app_entries[] =
{
	{ "new_window", grisbi_cmd_new_window, NULL, NULL, NULL },
	{ "about", grisbi_cmd_about, NULL, NULL, NULL },
	{ "prefs", grisbi_cmd_prefs, NULL, NULL, NULL },
	{ "quit", grisbi_cmd_quit, NULL, NULL, NULL },
	{ "manual", grisbi_cmd_manual, NULL, NULL, NULL },
	{ "quick_start", grisbi_cmd_quick_start, NULL, NULL, NULL },
	{ "web_site", grisbi_cmd_web_site, NULL, NULL, NULL },
/*	{ "report_bug", grisbi_cmd_report_bug, NULL, NULL, NULL },
*/	{ "day_tip", grisbi_cmd_day_tip, NULL, NULL, NULL }
};

/**
 * crée et initialise le menu de grisbi.
 *
 * \param GApplication *app
 *
 * \return
 * */
static void grisbi_app_set_main_menu ( GApplication *app )
{
/*    GrisbiAppPrivate *priv;
	GtkBuilder *builder;
	GError *error = NULL;

    priv = grisbi_app_get_instance_private ( GRISBI_APP ( app ) );
*/
	g_action_map_add_action_entries ( G_ACTION_MAP ( app ),
                        app_entries,
						G_N_ELEMENTS ( app_entries ),
                        app );

	/* adding app menu EN ATTENTE */
/*	builder = gtk_builder_new ();
	if ( !gtk_builder_add_from_resource ( builder,
						"/org/gtk/grisbi/ui/grisbi_menu.ui",
						&error ) )
	{
		g_warning ("loading menu builder file: %s", error->message);
		g_error_free (error);

		exit (1);
	}
	else
	{

		priv->appmenu = G_MENU_MODEL ( gtk_builder_get_object ( builder, "appmenu" ) );
		gtk_application_set_app_menu ( GTK_APPLICATION ( app ), priv->appmenu );
	}

	g_object_unref ( builder );
*/
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

GtkRecentManager *grisbi_app_get_recent_manager ( void )
{
	GrisbiApp *app;
	GrisbiAppPrivate *priv;

	app = GRISBI_APP ( g_application_get_default () );
	priv = grisbi_app_get_instance_private ( GRISBI_APP ( app ) );

	return priv->recent_manager;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
/* Fonctions propres à l'initialisation de l'application */
static const GOptionEntry options[] =
{
	/* Version */
	{
		"version", 'V', 0, G_OPTION_ARG_NONE, NULL,
		N_("Show the application's version"), NULL
	},

	/* Open a new window */
/*	{
		"new-window", '\0', 0, G_OPTION_ARG_NONE, NULL,
		N_("Create a new top-level window in an existing instance of grisbi"),
		NULL
	},
*/
    /* debug level */
    {
        "debug", 'd', 0, G_OPTION_ARG_STRING, NULL,
        N_("Debug mode: level 0-5"),
		N_("DEBUG")
    },

	/* Window geometry */
	{
		"geometry", 'g', 0, G_OPTION_ARG_STRING, NULL,
		N_("Set the size and position of the window (WIDTHxHEIGHT+X+Y)"),
		N_("GEOMETRY")
	},

	/* New instance */
/*	{
		"standalone", 's', 0, G_OPTION_ARG_NONE, NULL,
		N_("Run grisbi in standalone mode"),
		NULL
	},
*/
	/* collects file arguments */
	{
		G_OPTION_REMAINING, '\0', 0, G_OPTION_ARG_FILENAME_ARRAY, NULL, NULL,
		N_("[FILE...]")
	},

	{NULL}
};

/**
 * affiche les information d'environnement
 *
 * \param
 *
 * \return
 * */
static gboolean grisbi_app_print_environment_var ( void )
{
    gchar *tmp_str;

    g_printf ("Variables d'environnement :\n\n" );

    tmp_str = gsb_locale_get_print_locale_var ();
    g_printf ("%s", tmp_str);

    g_free ( tmp_str );

    g_printf ( "gint64\n\tG_GINT64_MODIFIER = \"%s\"\n"
                        "\t%"G_GINT64_MODIFIER"d\n\n",
                        G_GINT64_MODIFIER,
                        G_MAXINT64 );

    tmp_str = gsb_dirs_get_print_dir_var ();
    g_printf ("%s", tmp_str);

    g_free ( tmp_str );

    return FALSE;
}

/**
 * gestionnaire des options passées au programme
 *
 * \param GApplication  *app
 * \param GApplicationCommandLine *command_line
 *
 * \return always TRUE
 **/
static gboolean grisbi_app_cmdline ( GApplication *app,
                        GApplicationCommandLine *cmdline )
{
	GrisbiAppPrivate *priv;
	GVariantDict *options;
	gchar **args;
	gchar *tmp_str = NULL;
	const gchar **remaining_args;
	gint argc;

	priv = grisbi_app_get_instance_private ( GRISBI_APP ( app ) );

	/* initialisation de debug_level à -1 */
	priv->debug_level = -1;

	/* initialisation des répertoires de grisbi */
	args = g_application_command_line_get_arguments (cmdline, &argc);
	gsb_dirs_init ( args[0] );

	g_strfreev (args);

	/* traitement des autres options */
	options = g_application_command_line_get_options_dict ( cmdline );

/*	g_variant_dict_lookup ( options, "new-window", "b", &priv->new_window );
*/	g_variant_dict_lookup ( options, "debug", "s", &tmp_str );
	g_variant_dict_lookup ( options, "d", "s", &tmp_str );
	g_variant_dict_lookup ( options, "geometry", "s", &priv->geometry );
	g_variant_dict_lookup ( options, "g", "s", &priv->geometry );

    /* Parse filenames */
	if ( g_variant_dict_lookup ( options, G_OPTION_REMAINING, "^a&ay", &remaining_args ) )
	{
		gint i;

		for (i = 0; remaining_args[i]; i++)
		{
			if ( g_file_test ( remaining_args[i], G_FILE_TEST_EXISTS ) )
			{
				priv->file_list = g_slist_prepend ( priv->file_list, g_strdup ( remaining_args[i] ) );
			}
		}

		if ( g_slist_length ( priv->file_list ) > 1 )
			priv->file_list = g_slist_reverse (priv->file_list);

		g_free ( remaining_args );
	}

	/* modification du niveau de débug si besoin */
	if ( tmp_str && strlen ( tmp_str ) > 0 )
	{
		gchar *endptr;
		gint64 number;

		errno = 0;
		number = g_ascii_strtoll ( tmp_str, &endptr, 10 );
		if ( endptr == NULL )
			priv->debug_level = number;
		else if ( errno == 0 && number == 0 )
			priv->debug_level = number;
	}

	if ( IS_DEVELOPMENT_VERSION == 1 )
    {
		if ( priv->debug_level >= 0 && priv->debug_level < 5 )
			debug_set_cmd_line_debug_level ( priv->debug_level );

		if ( priv->debug_level == -1 || priv->debug_level > 0 )
			grisbi_app_print_environment_var ();
    }
	else if ( priv->debug_level > 0 )
	{
		debug_initialize_debugging ( priv->debug_level );
        grisbi_app_print_environment_var ();

	}

	g_application_activate ( app );

	return FALSE;
}

static gint grisbi_app_handle_local_options (GApplication *app,
                        GVariantDict *options)
{
	if ( g_variant_dict_contains ( options, "version" ) )
	{
		g_print ( "%s - Version %s\n", g_get_application_name (), VERSION );
		g_print( "\n\n" );
		g_print( "%s", extra_support () );
		return 0;
	}

/*	if (g_variant_dict_contains (options, "standalone"))
	{
		GApplicationFlags old_flags;

		old_flags = g_application_get_flags (application);
		g_application_set_flags (application, old_flags | G_APPLICATION_NON_UNIQUE);
	}

	if (g_variant_dict_contains (options, "wait"))
	{
		GApplicationFlags old_flags;

		old_flags = g_application_get_flags (application);
		g_application_set_flags (application, old_flags | G_APPLICATION_IS_LAUNCHER);
	}
*/
    return -1;
}

/**
 * On detourne les signaux SIGINT, SIGTERM, SIGSEGV
 *
 * \param
 *
 * \return
 * */
static void grisbi_app_trappe_signaux ( void )
{
#ifndef G_OS_WIN32
    struct sigaction sig_sev;

    memset ( &sig_sev, 0, sizeof ( struct sigaction ) );
    sig_sev.sa_handler = debug_traitement_sigsegv;
    sig_sev.sa_flags = 0;
    sigemptyset ( &( sig_sev.sa_mask ) );

    if ( sigaction ( SIGINT, &sig_sev, NULL ) )
        g_print ( _("Error on sigaction: SIGINT won't be trapped\n") );

    if ( sigaction ( SIGTERM, &sig_sev, NULL ) )
        g_print ( _("Error on sigaction: SIGTERM won't be trapped\n") );

    if ( sigaction ( SIGSEGV, &sig_sev, NULL ) )
        g_print ( _("Error on sigaction: SIGSEGV won't be trapped\n") );
#endif /* G_OS_WIN32 */
}

/**
 * Load file if necessary
 *
 * \param GApplication *app
 *
 * \return
 **/
static void grisbi_app_load_file_if_necessary ( GrisbiApp *app )
{
    GrisbiAppPrivate *priv;

	printf ("grisbi_app_load_file_if_necessary\n");
    priv = grisbi_app_get_instance_private ( GRISBI_APP ( app ) );

    /* check the command line, if there is something to open */
    if ( priv->file_list )
    {
		gchar *tmp_str = NULL;
		GSList *tmp_list;

		tmp_list = 	priv->file_list;

		while ( tmp_list )
		{
			tmp_str = tmp_list -> data;

			if ( gsb_file_open_file ( tmp_str ) )
			{
				if ( nom_fichier_comptes )
					g_free ( nom_fichier_comptes );
				nom_fichier_comptes = tmp_str;
			}
			else
			{
				if ( nom_fichier_comptes )
					g_free ( nom_fichier_comptes );
				nom_fichier_comptes = NULL;
			}

			tmp_list = tmp_list -> next;
		}

	}
    else
    {
        /* open the last file if needed, nom_fichier_comptes was filled while loading the configuration */
        if ( conf.dernier_fichier_auto && nom_fichier_comptes )
        {
            if ( !gsb_file_open_file ( nom_fichier_comptes ) )
                g_free ( nom_fichier_comptes );
        }
    }
}

/**
 * grisbi_app_init
 *
 * \param GApplication *app
 *
 * \return
 **/
static void grisbi_app_init ( GrisbiApp *app )
{
    gchar *string;
    GrisbiAppPrivate *priv;

	priv = grisbi_app_get_instance_private ( GRISBI_APP ( app ) );

	/* initialize debugging */
    if ( IS_DEVELOPMENT_VERSION == 1 )
        debug_initialize_debugging ( 5 );

    g_set_application_name ("Grisbi");

    /* create the icon of grisbi (set in the panel of gnome or other) */
    string = g_build_filename ( gsb_dirs_get_pixmaps_dir (), "grisbi-logo.png", NULL );
    if ( g_file_test ( string, G_FILE_TEST_EXISTS ) )
        gtk_window_set_default_icon_from_file ( string, NULL );

    g_free (string);

	/* add options for app menu */
    g_application_add_main_option_entries ( G_APPLICATION ( app ), options );
}

/**
 * lancement de l'application
 *
 * \param GApplication *app
 *
 * \return
 **/
static void grisbi_app_startup ( GApplication *app )
{
    G_APPLICATION_CLASS (grisbi_app_parent_class)->startup ( app );

    bindtextdomain ( PACKAGE, gsb_dirs_get_locale_dir () );
    bind_textdomain_codeset ( PACKAGE, "UTF-8" );
    textdomain ( PACKAGE );

    /* Setup locale/gettext */
    setlocale ( LC_ALL, "" );
    gsb_locale_init ();
}

/**
 * grisbi_app_activate
 *
 * \param GApplication *app
 *
 * \return
 **/
static void grisbi_app_activate ( GApplication *app )
{
    GrisbiWin *win;
    GrisbiAppPrivate *priv;
	GtkBuilder *builder;
	GMenuModel *menubar;
	GError *error = NULL;

	devel_debug ( NULL );
	priv = grisbi_app_get_instance_private ( GRISBI_APP ( app ) );

#ifdef HAVE_GOFFICE
    /* initialisation libgoffice */
    libgoffice_init ();
    /* Initialize plugins manager */
    go_plugins_init ( NULL, NULL, NULL, NULL, TRUE, GO_TYPE_PLUGIN_LOADER_MODULE );
#endif /* HAVE_GOFFICE */

    /* on commence par détourner le signal SIGSEGV */
    grisbi_app_trappe_signaux ();

    /* initialisation of the variables */
    gsb_color_initialise_couleurs_par_defaut ();
    init_variables ();
    register_import_formats ();

	/* initiate recent_manager */
	priv->recent_manager = gtk_recent_manager_get_default ();

    /* firt use ? */
    priv->first_use = FALSE;
    if ( !gsb_file_config_load_config () )
        priv->first_use = TRUE;

	/* app menu */
	grisbi_app_set_main_menu ( app );

    /* charge les raccourcis claviers */
    grisbi_app_load_accels ( );

	/* création de la fenêtre pincipale */
    win = grisbi_app_create_window ( GRISBI_APP ( app ), NULL );

	if ( priv->geometry )
		gtk_window_parse_geometry ( GTK_WINDOW ( win ), priv->geometry );

	/* set menubar */
	grisbi_win_set_menubar ( win, app );

	/* affiche la fenêtre principale */
	gtk_window_present ( GTK_WINDOW ( win ) );

	/* ouvre un fichier si demandé */
	grisbi_app_load_file_if_necessary ( GRISBI_APP ( app ) );
}

/**
 * grisbi_app_open
 *
 * \param GApplication *app
 *
 * \return
 **/
static void grisbi_app_open ( GApplication *app,
                        GFile **files,
                        gint n_files,
                        const gchar *hint )
{
    GList *windows;
    GrisbiWin *win;
    int i;

    windows = gtk_application_get_windows ( GTK_APPLICATION ( app ) );
    if ( windows )
        win = GRISBI_WIN ( windows->data );
    else
        win = grisbi_app_create_window ( GRISBI_APP ( app ), NULL );

    for ( i = 0; i < n_files; i++ )
	{

/*        grisbi_win_open ( win, files[i] );
*/	}
    gtk_window_present ( GTK_WINDOW ( win ) );
}

/**
 * grisbi_app_shutdown
 *
 * \param GApplication *app
 *
 * \return
 **/
static void grisbi_app_shutdown ( GApplication *app )
{
	GtkWindow *win;

    printf ("grisbi_app_shutdown\n");
	devel_debug (NULL);

    G_APPLICATION_CLASS (grisbi_app_parent_class)->shutdown (app);

	/* on récupère la dernière fenêtre active */
	win = gtk_application_get_active_window ( GTK_APPLICATION ( app ) );

    /* sauvegarde les raccourcis claviers */
	grisbi_app_save_accels ( );

    /* sauvegarde la position de la fenetre principale */
    if ( win && conf.full_screen == 0 && conf.maximize_screen == 0 )
        gtk_window_get_position ( GTK_WINDOW ( win ), &conf.root_x, &conf.root_y );

    /* sauvegarde de la taille de la fenêtre si nécessaire */
    if ( win && conf.full_screen == 0 && conf.maximize_screen == 0 )
        gtk_window_get_size ( GTK_WINDOW ( win ), &conf.main_width, &conf.main_height );

    /* clean finish of the debug file */
    if ( etat.debug_mode )
        debug_finish_log ();

	/* on sauvegarde la configuration générale de grisbi */
	gsb_file_config_save_config ();

	/* on libère la mémoire utilisée par conf */
	 grisbi_app_free_struct_conf ();

	/* on libère la mémoire utilisée par etat */
	free_variables ();

	/* libération de mémoire utilisée par locale*/
    gsb_locale_shutdown ();

	/* libération de mémoire utilisée par gsb_dirs*/
    gsb_dirs_shutdown ();

#ifdef HAVE_GOFFICE
    /* liberation libgoffice */
    libgoffice_shutdown ();
#endif /* HAVE_GOFFICE */

#ifdef GSB_GMEMPROFILE
    g_mem_profile();
#endif
}

/**
 * grisbi_app_class_init
 *
 * \param GrisbiAppClass    *class
 *
 * \return
 **/
static void grisbi_app_class_init ( GrisbiAppClass *klass )
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GApplicationClass *app_class = G_APPLICATION_CLASS (klass);

    app_class->startup = grisbi_app_startup;
    app_class->activate = grisbi_app_activate;
    app_class->open = grisbi_app_open;
    app_class->shutdown = grisbi_app_shutdown;
    app_class->command_line = grisbi_app_cmdline;
    app_class->handle_local_options = grisbi_app_handle_local_options;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
GrisbiApp *grisbi_app_new (  char *app_name  )
{
    GrisbiApp *app;

    app = g_object_new ( GRISBI_APP_TYPE,
                        "application-id", "org.gtk.grisbi",
                        "flags",
                        G_APPLICATION_HANDLES_COMMAND_LINE,
                        NULL );

    return app;
}

/* WINDOWS */
/**
 * check on any change on the main window
 * for now, only to check if we set/unset the full-screen
 *
 * \param window
 * \param event
 * \param null
 *
 * \return FALSE
 * */
static gboolean grisbi_win_change_state_window ( GtkWidget *window,
                        GdkEventWindowState *event,
                        gpointer null )
{
    gboolean show;

    if ( event->changed_mask & GDK_WINDOW_STATE_MAXIMIZED )
    {
        show = !( event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED );

        gtk_window_set_has_resize_grip ( GTK_WINDOW ( window ), show );
        conf.maximize_screen = !show;
    }
    else if ( event->changed_mask & GDK_WINDOW_STATE_FULLSCREEN )
    {
        show = !( event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN );

/*        if ( show )
            gtk_widget_show  ( gsb_status_get_status_bar () );
        else
            gtk_widget_hide  ( gsb_status_get_status_bar () );
*/
        conf.full_screen = !show;
    }

    /* return value */
    return FALSE;
}


/**
 * This function is called when a window window is deleted.
 * It proposes to save the file if necessary.
 *
 * \param
 * \param
 *
 * \return
 * */
static gboolean grisbi_win_delete_event ( GtkWidget *win,
						gpointer data )
{
	GrisbiApp *app = GRISBI_APP ( g_application_get_default () );

	printf ("grisbi_win_delete_event\n'");

    if ( !gsb_file_close () )
        return TRUE;

	gtk_application_remove_window ( GTK_APPLICATION ( app ), GTK_WINDOW ( win ) );
    run.window = NULL;

    return TRUE;
}

/**
 * exit the app when the main window is destroyed.
 *
 * \param
 * \param
 *
 * \return
 * */
static void grisbi_win_destroy_event ( GtkWidget *win,
						gpointer data )
{
	printf ("grisbi_win_destroy_event\n'");

    run.window = NULL;
}

static gboolean grisbi_win_key_press_event ( GtkWidget *widget,
                        GdkEventKey *event,
                        gpointer data )
{
    switch ( event -> keyval )
    {
        case GDK_KEY_F11 :
            if ( conf.full_screen )
                gtk_window_unfullscreen ( GTK_WINDOW ( widget ) );
            else
                gtk_window_fullscreen ( GTK_WINDOW ( widget ) );
        return TRUE;
    }

    return FALSE;
}

/**
 * Création d'une fenêtre GrisbiWin.
 *
 * \param app
 * \param screen
 *
 * \return une fenêtre pour Grisbi
 */
GrisbiWin *grisbi_app_create_window ( GrisbiApp *app,
                        GdkScreen *screen )
{
    GrisbiWin *win;
    GrisbiAppPrivate *priv;

	priv = grisbi_app_get_instance_private ( GRISBI_APP ( app ) );

	win = g_object_new ( GRISBI_WIN_TYPE, "application", app, NULL );
	gtk_application_add_window ( GTK_APPLICATION ( app ), GTK_WINDOW ( win ) );

	grisbi_win_set_size_and_position ( GTK_WINDOW ( win ) );

    g_signal_connect ( win,
                        "window-state-event",
                        G_CALLBACK (grisbi_win_change_state_window),
                        NULL );

    g_signal_connect ( win,
                        "delete_event",
                        G_CALLBACK ( grisbi_win_delete_event ),
                        app );

    g_signal_connect ( win,
                        "destroy",
                        G_CALLBACK ( grisbi_win_destroy_event ),
                        app );

    g_signal_connect ( G_OBJECT ( run.window ),
                        "key-press-event",
                        G_CALLBACK ( grisbi_win_key_press_event ),
                        NULL );

	if ( screen != NULL )
        gtk_window_set_screen ( GTK_WINDOW ( win ), screen );

    return win;
}

/**
 * get active window.
 *
 * \param app
 *
 * \return active_window
 */
GrisbiWin *grisbi_app_get_active_window ( GrisbiApp *app )
{
    GrisbiApp *tmp_app;
	GrisbiWin *win;

    if ( app == NULL )
        tmp_app = GRISBI_APP ( g_application_get_default () );
    else
        tmp_app = app;

    if ( !tmp_app )
        return NULL;

	win = GRISBI_WIN ( gtk_application_get_active_window ( GTK_APPLICATION ( tmp_app ) ) );

    return win;
}

/**
 * cherche si le fichier est déjà utilisé
 *
 * \param gchar		filename
 *
 * \return TRUE is duplicate file FALSE otherwise
 **/
gboolean grisbi_app_is_duplicated_file ( const gchar *filename )
{
	GrisbiApp *app;
	GList *windows;
	GList *tmp_list;

	app = GRISBI_APP ( g_application_get_default () );

	windows = gtk_application_get_windows ( GTK_APPLICATION ( app ) );
	tmp_list = windows;

	while ( tmp_list )
	{
		GrisbiWin *win;
		const gchar *tmp_filename;
		gchar *key1;
		gchar *key2;
		gboolean result = FALSE;

		win = tmp_list->data;
		tmp_filename = grisbi_win_get_filename ( win );
		key1 = g_utf8_collate_key_for_filename ( filename, -1 );
		key2 = g_utf8_collate_key_for_filename ( tmp_filename, -1 );

		if ( strcmp ( key1, key2 ) == 0 )
		{
			g_free ( key1 );
			g_free ( key2 );
			return TRUE;
		}

		tmp_list = tmp_list->next;
	}
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
