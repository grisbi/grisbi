/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2016 Pierre Biava (grisbi@pierre.biava.name)                    */
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
#include <stdlib.h>
#include <glib/gprintf.h>
#include <glib/gi18n.h>
#include <errno.h>

#ifdef HAVE_GOFFICE
#include <goffice/goffice.h>
#endif /* HAVE_GOFFICE */

/*START_INCLUDE*/
#include "grisbi_app.h"
#include "grisbi_settings.h"
#include "gsb_color.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "gsb_locale.h"
#include "help.h"
#include "import.h"
#include "menu.h"
#include "traitement_variables.h"
#include "erreur.h"
/*END_INCLUDE*/

 /*START_EXTERN Variables externes PROVISOIRE*/
extern gchar *nom_fichier_comptes;
extern gchar *titre_fichier;
/*END_EXTERN*/

typedef struct _GrisbiAppPrivate GrisbiAppPrivate;

struct _GrisbiAppPrivate
{
	/* menus builder */
	GtkBuilder 			*menus_builder;

    GrisbiSettings      *settings;              /* test utilisation GSettings */

	gint                first_use;

    /* command line parsing */
    gboolean 			new_window;
    gint 				debug_level;
    gchar 				*geometry;
    GSList 				*file_list;

	/* Menuapp et menubar */
	GMenuModel 			*appmenu;
	GMenuModel 			*menubar;

    GMenu               *item_recent_files;

    GAction             *prefs_action;
};

G_DEFINE_TYPE_WITH_PRIVATE(GrisbiApp, grisbi_app, GTK_TYPE_APPLICATION);

/* STRUCTURE CONF */
/**
 * libération de la mémoire de la variable conf
 *
 * \param
 *
 * \return
 **/
 void grisbi_app_free_struct_conf ( void )
{
    devel_debug (NULL);

	if ( conf.font_string )
    {
		g_free ( conf.font_string );
		conf.font_string = NULL;
    }
	if ( conf.browser_command )
	{
        g_free ( conf.browser_command );
		conf.browser_command = NULL;
	}

	gsb_file_free_last_path ();
	gsb_file_free_backup_path ();

}

/* MENU APP*/
/**
 *
 *
 * \param GSimpleAction     action
 * \param GVariant          state
 * \param gpointer          user_data
 *
 * \return
 **/
static void grisbi_app_change_fullscreen_state ( GSimpleAction *action,
                        GVariant *state,
                        gpointer user_data )
{
    GtkWindow *win;

    win = GTK_WINDOW ( grisbi_app_get_active_window ( user_data ) );

    if ( g_variant_get_boolean ( state ) )
    {
        conf.full_screen = TRUE;
        gtk_window_fullscreen ( win );
    }
    else
    {
        conf.full_screen = FALSE;
        gtk_window_unfullscreen ( win );
    }

    g_simple_action_set_state ( action, state );
}

/**
 *
 *
 * \param GSimpleAction     action
 * \param GVariant          state
 * \param gpointer          user_data
 *
 * \return
 **/
static void grisbi_app_change_radio_state ( GSimpleAction *action,
                        GVariant *state,
                        gpointer user_data )
{
    printf ("grisbi_app_change_radio_state = %s\n", g_variant_get_string (state, NULL) );
    g_simple_action_set_state ( action, state );
}

static GActionEntry app_entries[] =
{
	{ "new-window", grisbi_cmd_new_window, NULL, NULL, NULL },
	{ "about", grisbi_cmd_about, NULL, NULL, NULL },
	{ "prefs", grisbi_cmd_prefs, NULL, NULL, NULL },
	{ "quit", grisbi_cmd_quit, NULL, NULL, NULL },
};

static const GActionEntry win_always_enabled_entries[] =
{
	{ "new-acc-file", grisbi_cmd_file_new, NULL, NULL, NULL },
	{ "open-file", grisbi_cmd_file_open_menu, NULL, NULL, NULL },
    { "direct-open-file", grisbi_cmd_file_open_direct_menu, "s", NULL, NULL },
	{ "import-file", grisbi_cmd_importer_fichier, NULL, NULL, NULL },
	{ "obf-qif-file", grisbi_cmd_obf_qif_file, NULL, NULL, NULL },
	{ "manual", grisbi_cmd_manual, NULL, NULL, NULL },
	{ "quick-start", grisbi_cmd_quick_start, NULL, NULL, NULL },
	{ "web-site", grisbi_cmd_web_site, NULL, NULL, NULL },
	{ "report-bug", grisbi_cmd_report_bug, NULL, NULL, NULL },
	{ "day_tip", grisbi_cmd_day_tip, NULL, NULL, NULL },
    { "fullscreen", NULL, NULL, "false", grisbi_app_change_fullscreen_state }
};

static const GActionEntry win_context_enabled_entries[] =
{
	{ "save", grisbi_cmd_file_save, NULL, NULL, NULL },
	{ "save-as", grisbi_cmd_file_save_as, NULL, NULL, NULL },
	{ "export-accounts", grisbi_cmd_export_accounts, NULL, NULL, NULL },
	{ "create-archive", grisbi_cmd_create_archive, NULL, NULL, NULL },
	{ "export-archive", grisbi_cmd_export_archive, NULL, NULL, NULL },
	{ "debug-acc-file", grisbi_cmd_debug_acc_file, NULL, NULL, NULL },
	{ "obf-acc-file", grisbi_cmd_obf_acc_file, NULL, NULL, NULL },
	{ "debug-mode", grisbi_cmd_debug_mode_toggle, NULL, "false", NULL },
	{ "file-close", grisbi_cmd_file_close, NULL, NULL, NULL },
	{ "edit-ope", grisbi_cmd_edit_ope, NULL, NULL, NULL },
	{ "new-ope", grisbi_cmd_new_ope, NULL, NULL, NULL },
	{ "remove-ope", grisbi_cmd_remove_ope, NULL, NULL, NULL },
	{ "template-ope", grisbi_cmd_template_ope, NULL, NULL, NULL },
	{ "clone-ope", grisbi_cmd_clone_ope, NULL, NULL, NULL },
	{ "convert-ope", grisbi_cmd_convert_ope, NULL, NULL, NULL },
    { "move-to-acc", grisbi_cmd_move_to_account_menu, NULL, NULL, NULL },
	{ "new-acc", grisbi_cmd_new_acc, NULL, NULL, NULL },
	{ "remove-acc", grisbi_cmd_remove_acc, NULL, NULL, NULL },
	{ "show-form", grisbi_cmd_show_form_toggle, NULL, "false", NULL },
	{ "show-reconciled", grisbi_cmd_show_reconciled_toggle, NULL, "false", NULL },
	{ "show-archived", grisbi_cmd_show_archived_toggle, NULL, "false", NULL },
	{ "show-closed-acc", grisbi_cmd_show_closed_acc_toggle, NULL, "false", NULL },
	{ "show-ope", grisbi_cmd_show_ope_radio, "s", "'1'", grisbi_app_change_radio_state },
	{ "reset-width-col", grisbi_cmd_reset_width_col, NULL, NULL, NULL }
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
    GrisbiAppPrivate *priv;
	GtkBuilder *builder;
	GError *error = NULL;

    printf ("grisbi_app_set_main_menu\n");
    priv = grisbi_app_get_instance_private ( GRISBI_APP ( app ) );

	/* chargement des actions */
	/* adding menus */
	priv->menus_builder = gtk_builder_new ();
	if ( !gtk_builder_add_from_resource ( priv->menus_builder,
						"/org/gtk/grisbi/ui/grisbi_menu.ui",
						&error ) )
	{
		g_warning ("loading menu builder file: %s", error->message);
		g_error_free (error);

		exit (1);
	}

    /* Menu Application */
    if ( conf.prefer_app_menu )
    {
        GAction *action;

        g_action_map_add_action_entries ( G_ACTION_MAP ( app ),
                        app_entries,
						G_N_ELEMENTS ( app_entries ),
                        app );

        priv->appmenu = G_MENU_MODEL ( gtk_builder_get_object ( priv->menus_builder, "appmenu" ) );
        gtk_application_set_app_menu ( GTK_APPLICATION ( app ), priv->appmenu );
        priv->menubar = G_MENU_MODEL ( gtk_builder_get_object ( priv->menus_builder, "menubar" ) );
        gtk_application_set_menubar ( GTK_APPLICATION ( app ), priv->menubar );

        priv->prefs_action = g_action_map_lookup_action (G_ACTION_MAP ( app ), "prefs" );
        priv->item_recent_files = grisbi_app_get_menu_by_id ( app, "recent-file" );    }
    else
    {
        /* Menu "traditionnel" */
        priv->menubar = G_MENU_MODEL ( gtk_builder_get_object ( priv->menus_builder, "classic" ) );
        gtk_application_set_menubar ( GTK_APPLICATION ( app ), priv->menubar );
        priv->item_recent_files = grisbi_app_get_menu_by_id ( app, "classic-recent-file" );
    }

    grisbi_app_set_recent_files_menu ( app, FALSE );
}

/* WINDOWS */
/**
 * Création d'une fenêtre GrisbiWin.
 *
 * \param app
 * \param screen
 *
 * \return une fenêtre pour Grisbi
 */
static GrisbiWin *grisbi_app_create_window ( GrisbiApp *app,
                        GdkScreen *screen )
{
    GrisbiWin *win;
    GrisbiAppPrivate *priv;
    gchar *string;

    printf ("grisbi_app_create_window\n");
	priv = grisbi_app_get_instance_private ( GRISBI_APP ( app ) );

	win = g_object_new ( GRISBI_WIN_TYPE, "application", app, NULL );
	gtk_application_add_window ( GTK_APPLICATION ( app ), GTK_WINDOW ( win ) );

    /* create the icon of grisbi (set in the panel of gnome or other) */
    string = g_build_filename ( gsb_dirs_get_pixmaps_dir (), "grisbi-logo.png", NULL );
    if ( g_file_test ( string, G_FILE_TEST_EXISTS ) )
        gtk_window_set_default_icon_from_file ( string, NULL );

    g_free (string);

	grisbi_win_set_size_and_position ( GTK_WINDOW ( win ) );

    /* set menubar */
	/* Win Menu : actions toujours actives */
	g_action_map_add_action_entries ( G_ACTION_MAP ( win ),
						win_always_enabled_entries,
						G_N_ELEMENTS ( win_always_enabled_entries ),
						app );

	/* Win Menu : actions actives selon le contexte */
	g_action_map_add_action_entries ( G_ACTION_MAP ( win ),
						win_context_enabled_entries,
						G_N_ELEMENTS ( win_context_enabled_entries ),
						app );

    /* Actions du menu Application à intégrer dans le menu classique */
    if ( !conf.prefer_app_menu )
    {
        g_action_map_add_action_entries ( G_ACTION_MAP ( win ),
                        app_entries,
						G_N_ELEMENTS ( app_entries ),
                        app );

        priv->prefs_action = g_action_map_lookup_action ( G_ACTION_MAP ( win ), "prefs" );
    }

    grisbi_win_init_menubar ( win, app );

	if ( screen != NULL )
        gtk_window_set_screen ( GTK_WINDOW ( win ), screen );

    return win;
}

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

    printf ("grisbi_app_cmdline\n");
	priv = grisbi_app_get_instance_private ( GRISBI_APP ( app ) );

	/* initialisation de debug_level à -1 */
	priv->debug_level = -1;

	/* initialisation des répertoires de grisbi */
	args = g_application_command_line_get_arguments ( cmdline, &argc );
	gsb_dirs_init ( args[0] );

	g_strfreev ( args );

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

static gint grisbi_app_handle_local_options ( GApplication *app,
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
    GrisbiAppPrivate *priv;

    printf ("grisbi_app_init\n");
	priv = grisbi_app_get_instance_private ( GRISBI_APP ( app ) );

	/* initialize debugging */
    if ( IS_DEVELOPMENT_VERSION == 1 )
        debug_initialize_debugging ( 5 );

    g_set_application_name ("Grisbi");

	/* add options for app */
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
    printf ("grisbi_app_startup\n");
    G_APPLICATION_CLASS (grisbi_app_parent_class)->startup ( app );

    bindtextdomain ( PACKAGE, gsb_dirs_get_locale_dir () );
    bind_textdomain_codeset ( PACKAGE, "UTF-8" );
    textdomain ( PACKAGE );

    /* Setup locale/gettext */
    setlocale ( LC_ALL, "" );
    gsb_locale_init ();

    #ifdef HAVE_GOFFICE
    /* initialisation libgoffice */
    libgoffice_init ();
    /* Initialize plugins manager */
    go_plugins_init ( NULL, NULL, NULL, NULL, TRUE, GO_TYPE_PLUGIN_LOADER_MODULE );
    #endif /* HAVE_GOFFICE */
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
    gint number = 0;
	GError *error = NULL;

	devel_debug ( NULL );
    printf ("grisbi_app_activate\n");

	priv = grisbi_app_get_instance_private ( GRISBI_APP ( app ) );

    /* on commence par détourner le signal SIGSEGV */
    grisbi_app_trappe_signaux ();

    /* initialisation of the variables */
    priv->settings = grisbi_settings_get ();

    gsb_color_initialise_couleurs_par_defaut ();
    init_variables ();
    register_import_formats ();

    /* firt use ? */
/*    priv->first_use = FALSE;
    if ( !gsb_file_config_load_config () )
        priv->first_use = TRUE;
*/
	/* app menu */
	grisbi_app_set_main_menu ( app );

    /* charge les raccourcis claviers */
    gtk_application_add_accelerator ( GTK_APPLICATION ( app ),
                        "F11", "win.fullscreen", NULL );

	/* création de la fenêtre pincipale */
    win = grisbi_app_create_window ( GRISBI_APP ( app ), NULL );

	if ( priv->geometry )
		gtk_window_parse_geometry ( GTK_WINDOW ( win ), priv->geometry );

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

    printf ("grisbi_app_open\n");
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
    GrisbiAppPrivate *priv;

    printf ("grisbi_app_shutdown\n");
	devel_debug (NULL);

    G_APPLICATION_CLASS (grisbi_app_parent_class)->shutdown (app);

	priv = grisbi_app_get_instance_private ( GRISBI_APP ( app ) );

    /* on récupère la dernière fenêtre active */
	win = gtk_application_get_active_window ( GTK_APPLICATION ( app ) );

    printf ("conf.main_width = %d conf.main_height = %d\n", conf.main_width, conf.main_height );

    if ( win && conf.full_screen == 0 && conf.maximize_screen == 0 )
    {
        /* sauvegarde la position de la fenetre principale */
        gtk_window_get_position ( GTK_WINDOW ( win ), &conf.x_position, &conf.y_position );

        /* sauvegarde de la taille de la fenêtre si nécessaire */
        gtk_window_get_size ( GTK_WINDOW ( win ), &conf.main_width, &conf.main_height );
    }

    /* clean finish of the debug file */
    if ( etat.debug_mode )
        debug_finish_log ();

	/* on libère la mémoire utilisée par etat */
	free_variables ();

	/* libération de mémoire utilisée par locale*/
    gsb_locale_shutdown ();

	/* libération de mémoire utilisée par gsb_dirs*/
    gsb_dirs_shutdown ();

    /* Sauvegarde de la configuration générale */
    grisbi_settings_save_app_config ( priv->settings );

	/* on libère la mémoire utilisée par conf */
    grisbi_app_free_struct_conf ();

    /* liberation de la mémoire utilisée par GrisbiSettings*/
    g_clear_object ( &priv->settings );

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

/*******************************************************************************
 * Public Methods
 ******************************************************************************/
/**
 *
 *
 * \param
 *
 * \return
 **/
GrisbiApp *grisbi_app_new ( char *app_name )
{
    GrisbiApp *app;

    printf ("grisbi_app_new\n");
    app = g_object_new ( GRISBI_APP_TYPE,
                        "application-id",
                        "org.gtk.grisbi",
                        "flags",
                        G_APPLICATION_HANDLES_COMMAND_LINE | G_APPLICATION_NON_UNIQUE, /* voir GApplicationFlags pour détails */
                        NULL );

    return app;
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
	GrisbiWin *win;

    if ( app == NULL )
        app = GRISBI_APP ( g_application_get_default () );

	win = GRISBI_WIN ( gtk_application_get_active_window ( GTK_APPLICATION ( app ) ) );

    return win;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
GrisbiSettings *grisbi_app_get_grisbi_settings ( void )
{
    GrisbiAppPrivate *priv;

    priv = grisbi_app_get_instance_private ( GRISBI_APP ( g_application_get_default () ) );

    return priv->settings;
}

/**
 * grisbi_app_get_menu_by_id
 *
 * \param GApplication 	app
 * \param const gchar  	id of GMenu
 *
 * \return GMenu
 * */
GMenu *grisbi_app_get_menu_by_id ( GApplication *app,
                        const gchar *id )
{
	GObject *object;
	GrisbiAppPrivate *priv;
	printf ("grisbi_app_get_menu_by_id : %s\n", id);

    if ( app == NULL )
        app = g_application_get_default ();

	priv = grisbi_app_get_instance_private ( GRISBI_APP ( app ) );
	if ( !priv->menus_builder )
		return NULL;

	object = gtk_builder_get_object ( priv->menus_builder, id );

	if ( !object || !G_IS_MENU ( object ) )
		return NULL;

	return G_MENU ( object );
}

/**
 *
 *
 * \param
 *
 * \return
 **/
GAction *grisbi_app_get_prefs_action ( void )
{
    GrisbiAppPrivate *priv;

    priv = grisbi_app_get_instance_private ( GRISBI_APP ( g_application_get_default () ) );

    return priv->prefs_action;
}

/**
 *
 *
 * \param gchar **  recent_array
 *
 * \return
 **/
void grisbi_app_init_recent_manager ( gchar **recent_array )
{
	GtkRecentManager *recent_manager;
	gchar *uri = NULL;
	gint i;
    gint nb_effectif = 0;
	gboolean result;
    GList *tmp_list;
	printf ("grisbi_app_init_recent_manager : %s\n", recent_array[0]);

	recent_manager = gtk_recent_manager_get_default ();

    if ( conf.nb_derniers_fichiers_ouverts > conf.nb_max_derniers_fichiers_ouverts )
    {
        conf.nb_derniers_fichiers_ouverts = conf.nb_max_derniers_fichiers_ouverts;
    }

    tmp_list = gtk_recent_manager_get_items ( recent_manager );
	for ( i=0 ; i < conf.nb_derniers_fichiers_ouverts ; i++ )
    {
		uri = g_filename_to_uri ( recent_array[i], NULL, NULL );
        if ( g_file_test ( recent_array[i], G_FILE_TEST_EXISTS ) )
        {
            if ( !gtk_recent_manager_has_item ( recent_manager, uri ) )
                result = gtk_recent_manager_add_item (  recent_manager, uri );
            if ( result )
            {
                nb_effectif++;
            }
        }
        g_free ( uri );
	}
    conf.nb_derniers_fichiers_ouverts = nb_effectif;
    g_list_free_full ( tmp_list, ( GDestroyNotify ) gtk_recent_info_unref );
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
gchar **grisbi_app_get_recent_files_array ( void )
{
	GtkRecentManager *recent_manager;
    GList *tmp_list;
    gchar **recent_array = NULL;
	gchar *uri = NULL;
    gint index = 0;

    printf ("grisbi_app_get_recent_files_array\n");
	/* initialisation du tableau des fichiers récents */
    recent_array = g_new ( gchar *, conf.nb_max_derniers_fichiers_ouverts + 1);

    recent_manager = gtk_recent_manager_get_default ();

    tmp_list = gtk_recent_manager_get_items ( recent_manager );
    while ( tmp_list )
    {
        GtkRecentInfo *info;

        info = tmp_list->data;
        uri = gtk_recent_info_get_uri_display ( info );
        if ( g_str_has_suffix ( uri, ".gsb" ) )
		{
            if ( !g_file_test ( uri, G_FILE_TEST_EXISTS ) )
            {
                tmp_list = tmp_list->next;
                continue;
            }
			if ( index < conf.nb_max_derniers_fichiers_ouverts )
			{
                printf ("uri = %s\n", uri );
				recent_array[index++] = uri;
			}
            else
                g_free ( uri );
		}
		tmp_list = tmp_list->next;
    }
    recent_array[index] = NULL;
    conf.nb_derniers_fichiers_ouverts = index;
    g_list_free_full ( tmp_list, ( GDestroyNotify ) gtk_recent_info_unref );
    printf ("conf.nb_derniers_fichiers_ouverts = %d\n", conf.nb_derniers_fichiers_ouverts);

    return recent_array;
}

/**
 * crée et initialise le sous menu des fichiers récents.
 *
 * \param GApplication  *app
 * \param gboolean      reset 0 = création 1 = update
 *
 * \return
 * */
void grisbi_app_set_recent_files_menu ( GApplication *app,
                        gboolean reset )
{
    GrisbiAppPrivate *priv;
    GList *tmp_list;
    gchar *detailled_action;
    gchar *uri;
    gint index = 0;
    GError *error = NULL;

    printf ("grisbi_app_set_recent_files_menu\n");

    if ( app == NULL );
        app = g_application_get_default ();

    priv = grisbi_app_get_instance_private ( GRISBI_APP ( app ) );

    if ( reset )
    {
        GMenuItem *menu_item;
        const gchar *filename;

        g_menu_remove_all ( priv->item_recent_files );
        filename = grisbi_win_get_filename ( NULL );
        detailled_action = g_strdup_printf ("win.direct-open-file::%d", index+1 );
        menu_item = g_menu_item_new ( filename, detailled_action );
        g_menu_append_item ( priv->item_recent_files, menu_item );
        index++;
        g_free ( detailled_action );
        g_object_unref ( menu_item );
    }
    tmp_list = gtk_recent_manager_get_items ( gtk_recent_manager_get_default () );
    while ( tmp_list )
    {
        GtkRecentInfo *info;

        info = tmp_list->data;
        uri = gtk_recent_info_get_uri_display ( info );
        if ( g_str_has_suffix ( uri, ".gsb" ) )
		{
            if ( !g_file_test ( uri, G_FILE_TEST_EXISTS ) )
            {
                tmp_list = tmp_list->next;
                continue;
            }
			if ( index < conf.nb_max_derniers_fichiers_ouverts )
			{
                GMenuItem *menu_item;

                detailled_action = g_strdup_printf ("win.direct-open-file::%d", index+1 );
                menu_item = g_menu_item_new ( uri, detailled_action );
                g_menu_append_item ( priv->item_recent_files, menu_item );
				index++;
                g_free ( detailled_action );
                g_object_unref ( menu_item );
			}
		}
        g_free ( uri );
        tmp_list = tmp_list->next;
    }
    g_list_free_full ( tmp_list, ( GDestroyNotify ) gtk_recent_info_unref );
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
