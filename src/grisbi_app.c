/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2017 Pierre Biava (grisbi@pierre.biava.name)                    */
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
#include "config.h"
#endif

#include "include.h"
#include <stdlib.h>
#include <glib/gprintf.h>
#include <glib/gi18n.h>
#include <errno.h>

/*START_INCLUDE*/
#include "grisbi_app.h"
#include "grisbi_settings.h"
#include "gsb_assistant_first.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "gsb_locale.h"
#include "gsb_rgba.h"
#include "gsb_select_icon.h"
#include "help.h"
#include "import.h"
#include "menu.h"
#include "structures.h"
#include "tip.h"
#include "traitement_variables.h"
#include "utils_files.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static GrisbiWin *grisbi_app_create_window (GrisbiApp *app,
                                             GdkScreen *screen);
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct  _GrisbiAppPrivate	GrisbiAppPrivate;
struct _GrisbiAppPrivate
{
    /* command line parsing */
    gboolean 			new_window;
    gint 				debug_level;
    GSList *			file_list;

	/* Menuapp et menubar */
	gchar **			recent_array;
    gboolean            has_app_menu;
	GMenuModel *		appmenu;
	GMenuModel *		menubar;
    GMenu *				item_recent_files;
    GMenu *				item_edit;
    GAction *			prefs_action;
};

G_DEFINE_TYPE_WITH_PRIVATE (GrisbiApp, grisbi_app, GTK_TYPE_APPLICATION);

/* global variable, see structures.h */
struct GrisbiAppConf    conf;                   /* conf structure Provisoire */
GtkCssProvider *        css_provider = NULL;    /* css provider */

struct accelerator {
	const gchar *action_and_target;
	const gchar *accelerators[2];
};

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/* STRUCTURE CONF */
/**
 * initialisation de la variable conf
 *
 * \param
 *
 * \return
 **/
static void grisbi_app_struct_conf_init (void)
{
    devel_debug (NULL);

    conf.font_string = NULL;
    conf.browser_command = NULL;
	conf.import_directory = NULL;
	conf.language_chosen = NULL;
    conf.last_open_file = NULL;
}

/**
 * libération de la mémoire de la variable conf
 *
 * \param
 *
 * \return
 **/
static void grisbi_app_struct_conf_free (void)
{
    devel_debug (NULL);

	if (conf.font_string)
    {
		g_free (conf.font_string);
		conf.font_string = NULL;
    }
	if (conf.browser_command)
	{
        g_free (conf.browser_command);
		conf.browser_command = NULL;
	}

    if (conf.import_directory)
    {
        g_free (conf.import_directory);
        conf.import_directory = NULL;
    }

    if (conf.language_chosen)
    {
        g_free (conf.language_chosen);
        conf.language_chosen = NULL;
    }

	if (conf.last_open_file)
    {
        g_free (conf.last_open_file);
        conf.last_open_file = NULL;
    }

	gsb_file_free_last_path ();
	gsb_file_free_backup_path ();

}

/* ACCELERATORS*/
/**
 * Charge les raccourcis claviers (non modifiables)
 *
 * \param GrisbiApp     *app
 *
 * \return
 **/
static void grisbi_app_setup_accelerators (GApplication *application,
										   gboolean has_app_menu)
{
	guint i = 0;
	struct accelerator accels[] = {
		{ "app.new-window", { "<Alt>n", NULL } },
		{ "app.prefs", { "<Primary><Shift>p", NULL } },
		{ "app.quit", { "<Primary>q", NULL } },
		{ "win.quit", { "<Primary>q", NULL } },
		{ "win.new-acc-file", { "<Primary>n", NULL } },
		{ "win.open-file", { "<Primary>o", NULL } },
		{ "win.save", { "<Primary>s", NULL } },
		{ "win.import-file", { "<Primary>i", NULL } },
		{ "win.file-close", { "<Primary>w", NULL } },
		{ "win.new-ope", { "<Primary>t", NULL } },
		{ "win.new-acc", { "<Primary><Shift>n", NULL } },
		{ "win.show-reconciled", { "<Alt>r", NULL } },
		{ "win.show-arch", { "<Alt>l", NULL } },
		{ "win.manual", { "F1", NULL } },
		{ "win.fullscreen", {"F11", NULL} }
	};
	struct accelerator accels_classic[] = {
		{ "win.new-window", { "<Alt>n", NULL } },
		{ "win.prefs", { "<Primary><Shift>p", NULL } },
		{ "win.quit", { "<Primary>q", NULL } },
		{ "win.new-acc-file", { "<Primary>n", NULL } },
		{ "win.open-file", { "<Primary>o", NULL } },
		{ "win.save", { "<Primary>s", NULL } },
		{ "win.import-file", { "<Primary>i", NULL } },
		{ "win.file-close", { "<Primary>w", NULL } },
		{ "win.new-ope", { "<Primary>t", NULL } },
		{ "win.new-acc", { "<Primary><Shift>n", NULL } },
		{ "win.show-reconciled", { "<Alt>r", NULL } },
		{ "win.show-arch", { "<Alt>l", NULL } },
		{ "win.manual", { "F1", NULL } },
		{ "win.fullscreen", {"F11", NULL} }
	};

	if (has_app_menu)
	{
		for (i = 0; i < G_N_ELEMENTS (accels); i++)
		{

			gtk_application_set_accels_for_action (GTK_APPLICATION (application),
											   accels[i].action_and_target,
											   accels[i].accelerators);
		}
	}
	else
	{
		for (i = 0; i < G_N_ELEMENTS (accels_classic); i++)
		{

			gtk_application_set_accels_for_action (GTK_APPLICATION (application),
												   accels_classic[i].action_and_target,
												   accels_classic[i].accelerators);
		}
	}
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
static void grisbi_app_change_fullscreen_state (GSimpleAction *action,
												GVariant *state,
												gpointer user_data)
{
    GtkWindow *win;

    win = GTK_WINDOW (grisbi_app_get_active_window (user_data));

    if (g_variant_get_boolean (state))
    {
        conf.full_screen = TRUE;
        gtk_window_fullscreen (win);
    }
    else
    {
        conf.full_screen = FALSE;
        gtk_window_unfullscreen (win);
    }

    g_simple_action_set_state (action, state);
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
static void grisbi_app_change_radio_state (GSimpleAction *action,
										   GVariant *state,
										   gpointer user_data)
{
    g_simple_action_set_state (action, state);
}

static void grisbi_app_new_window (GSimpleAction *action,
                                   GVariant      *parameter,
                                   gpointer       user_data)
{
    GrisbiApp *app;

    app = GRISBI_APP (user_data);
    grisbi_app_create_window (GRISBI_APP (app), NULL);
}

static void grisbi_app_quit (GSimpleAction *action,
                             GVariant      *parameter,
                             gpointer       user_data)
{
    GrisbiApp *app;
    GList *l;
    gboolean first_win = TRUE;

    app = GRISBI_APP (user_data);

    /* Remove all windows registered in the application */
    while ((l = gtk_application_get_windows (GTK_APPLICATION (app))))
    {
        if (first_win)
        {
            if (l->data && conf.full_screen == 0 && conf.maximize_screen == 0)
            {
                /* sauvegarde la position de la fenetre principale */
                gtk_window_get_position (GTK_WINDOW (l->data), &conf.x_position, &conf.y_position);

                /* sauvegarde de la taille de la fenêtre si nécessaire */
                gtk_window_get_size (GTK_WINDOW (l->data), &conf.main_width, &conf.main_height);
            }
            first_win = FALSE;
        }
        if (gsb_file_close ())
		{
			gtk_window_close (GTK_WINDOW (l->data));
			gtk_application_remove_window (GTK_APPLICATION (app), GTK_WINDOW (l->data));
		}
		else
			return;
    }

	g_application_quit (G_APPLICATION (app));
}

/* Disable: warning: missing field 'padding' initializer
 *
 * 'padding' is a private field in the GActionEntry stucture so it is
 * not a good idea to explicitly initialize it. */
/* https://gcc.gnu.org/onlinedocs/gcc/Diagnostic-Pragmas.html */
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#else
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#endif

static GActionEntry app_entries[] =
{
	{ "new-window", grisbi_app_new_window, NULL, NULL, NULL },
	{ "about", grisbi_cmd_about, NULL, NULL, NULL },
	{ "prefs", grisbi_cmd_prefs, NULL, NULL, NULL },
	{ "quit", grisbi_app_quit, NULL, NULL, NULL },
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

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#else
#pragma clang diagnostic pop
#endif

/**
 * ajoute les actions et les item du menu recent_file.
 *
 * \param GApplication  *app
 *
 * \return
 * */
static void grisbi_app_init_recent_files_menu (GrisbiApp *app)
{
    GrisbiAppPrivate *priv;
	GMenuItem *menu_item;
    gchar *detailled_action;
    gint index = 0;
	gint i = 0;

    devel_debug (NULL);

    if (app == NULL)
        app = GRISBI_APP (g_application_get_default ());

    priv = grisbi_app_get_instance_private (GRISBI_APP (app));

	for (i = 0 ; i < conf.nb_derniers_fichiers_ouverts ; i++)
	{
		if (g_file_test (priv->recent_array[i], G_FILE_TEST_EXISTS))
		{
			gchar *menu_name;

			detailled_action = g_strdup_printf ("win.direct-open-file::%d", index+1);
			menu_name = gsb_menu_normalise_label_name (priv->recent_array[i]);
			menu_item = g_menu_item_new (menu_name, detailled_action);
			g_menu_append_item (priv->item_recent_files, menu_item);
			index++;
			g_free (menu_name);
			g_free (detailled_action);
			g_object_unref (menu_item);
		}
		else
		{
			gint j;

			for (j = i; j < conf.nb_derniers_fichiers_ouverts -1; j++)
			{
				priv->recent_array[j] = priv->recent_array[j+1];
			}
		}
	}
	if (index < conf.nb_derniers_fichiers_ouverts)
	{
		priv->recent_array = g_realloc (priv->recent_array, (index + 1) * sizeof (gchar*));
		priv->recent_array[index] = NULL;
		conf.nb_derniers_fichiers_ouverts = index;
	}
}

/**
 * crée et initialise le menu de grisbi.
 *
 * \param GApplication *app
 *
 * \return
 * */
static void grisbi_app_set_main_menu (GrisbiApp *app,
                                      gboolean has_app_menu)
{
    GrisbiAppPrivate *priv;
	GtkBuilder *builder;
	GError *error = NULL;

    priv = grisbi_app_get_instance_private (GRISBI_APP (app));

	/* chargement des actions */
	/* adding menus */
	builder = gtk_builder_new ();
	if (!gtk_builder_add_from_resource (builder,
						"/org/gtk/grisbi/ui/grisbi_menu.ui",
						&error))
	{
		g_critical ("loading menu builder file: %s", error->message);
		g_error_free (error);

		exit (1);
	}

    /* Menu Application */
    priv->has_app_menu = has_app_menu;

    if (has_app_menu)
    {
        GAction *action;

        g_action_map_add_action_entries (G_ACTION_MAP (app),
                        app_entries,
						G_N_ELEMENTS (app_entries),
                        app);

        priv->appmenu = G_MENU_MODEL (gtk_builder_get_object (builder, "appmenu"));
        gtk_application_set_app_menu (GTK_APPLICATION (app), priv->appmenu);
        priv->menubar = G_MENU_MODEL (gtk_builder_get_object (builder, "menubar"));
        gtk_application_set_menubar (GTK_APPLICATION (app), priv->menubar);

        priv->prefs_action = g_action_map_lookup_action (G_ACTION_MAP (app), "prefs");
        priv->item_recent_files = G_MENU (gtk_builder_get_object (builder, "recent-file"));
        priv->item_edit = G_MENU (gtk_builder_get_object (builder, "edit"));
        action = g_action_map_lookup_action (G_ACTION_MAP (app), "new-window");
        g_simple_action_set_enabled (G_SIMPLE_ACTION (action), FALSE);

    }
    else
    {
        /* Menu "traditionnel" */
        priv->menubar = G_MENU_MODEL (gtk_builder_get_object (builder, "classic"));
        gtk_application_set_menubar (GTK_APPLICATION (app), priv->menubar);
        priv->item_recent_files = G_MENU (gtk_builder_get_object (builder, "classic-recent-file"));
        priv->item_edit = G_MENU (gtk_builder_get_object (builder, "classic-edit"));
    }

    grisbi_app_init_recent_files_menu (app);

    g_object_unref (builder);
}

/* WINDOWS */
/**
 *
 *
 * \param
 *
 * \return
 **/
static gboolean grisbi_app_window_delete_event (GrisbiWin *win,
                        GdkEvent *event,
                        GrisbiApp *app)
{
	GList *l;
	gboolean last_win = FALSE;

	devel_debug (NULL);
	l = gtk_application_get_windows (GTK_APPLICATION (app));
	if (g_list_length (l) == 1)
	{
		last_win = TRUE;
	}

    if (conf.full_screen == 0 && conf.maximize_screen == 0)
    {
        /* sauvegarde la position de la fenetre principale */
        gtk_window_get_position (GTK_WINDOW (win), &conf.x_position, &conf.y_position);

        /* sauvegarde de la taille de la fenêtre si nécessaire */
        gtk_window_get_size (GTK_WINDOW (win), &conf.main_width, &conf.main_height);
    }

    if (gsb_file_close ())
	{
		gtk_window_close (GTK_WINDOW (win));
		gtk_application_remove_window (GTK_APPLICATION (app), GTK_WINDOW (win));

		if (last_win)
			g_application_quit (G_APPLICATION (app));
		return FALSE;
	}
	else
		return TRUE;
}

/**
 * Création d'une fenêtre GrisbiWin.
 *
 * \param app
 * \param screen
 *
 * \return une fenêtre pour Grisbi
 */
static GrisbiWin *grisbi_app_create_window (GrisbiApp *app,
											GdkScreen *screen)
{
    GrisbiWin *win;
    GrisbiAppPrivate *priv;
    gchar *string;

    priv = grisbi_app_get_instance_private (GRISBI_APP (app));

	win = g_object_new (GRISBI_TYPE_WIN, "application", app, NULL);

    g_signal_connect (win,
                        "delete_event",
                        G_CALLBACK (grisbi_app_window_delete_event),
                        app);

	gtk_application_add_window (GTK_APPLICATION (app), GTK_WINDOW (win));

    /* create the icon of grisbi (set in the panel of gnome or other) */
    string = g_build_filename (gsb_dirs_get_pixmaps_dir (), "grisbi-logo.png", NULL);
    if (g_file_test (string, G_FILE_TEST_EXISTS))
        gtk_window_set_default_icon_from_file (string, NULL);

    g_free (string);

	/* Adding pixmaps_dir in the icon theme */
	gsb_select_icon_set_gtk_icon_theme_path ();

    grisbi_win_set_size_and_position (GTK_WINDOW (win));

    /* set menubar */
	/* Win Menu : actions toujours actives */
	g_action_map_add_action_entries (G_ACTION_MAP (win),
						win_always_enabled_entries,
						G_N_ELEMENTS (win_always_enabled_entries),
						app);

	/* Win Menu : actions actives selon le contexte */
	g_action_map_add_action_entries (G_ACTION_MAP (win),
						win_context_enabled_entries,
						G_N_ELEMENTS (win_context_enabled_entries),
						app);

    /* Actions du menu Application à intégrer dans le menu classique */
    if (!priv->has_app_menu)
    {
        g_action_map_add_action_entries (G_ACTION_MAP (win),
                        app_entries,
						G_N_ELEMENTS (app_entries),
                        app);

        priv->prefs_action = g_action_map_lookup_action (G_ACTION_MAP (win), "prefs");
    }

    grisbi_win_init_menubar (win, app);

	/* affiche la fenêtre principale */
	gtk_window_present (GTK_WINDOW (win));

	if (screen != NULL)
        gtk_window_set_screen (GTK_WINDOW (win), screen);

    return win;
}

static const GOptionEntry options[] =
{
	/* Version */
	{
		"version", 'V', 0, G_OPTION_ARG_NONE, NULL,
		N_("Show the application's version"), NULL
	},

	/* Open a new window */
	{
		"new-window", '\0', 0, G_OPTION_ARG_NONE, NULL,
		N_("Create a new top-level window in an existing instance of grisbi"),
		NULL
	},

    /* debug level */
    {
        "debug", 'd', 0, G_OPTION_ARG_STRING, NULL,
        N_("Debug mode: level 0-5"),
		N_("DEBUG")
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

	{ NULL, 0, 0, 0, NULL, NULL, NULL}
};

/**
 * affiche les information d'environnement
 *
 * \param
 *
 * \return
 * */
static gboolean grisbi_app_print_environment_var (void)
{
    gchar *tmp_str;

    g_printf ("Variables d'environnement :\n\n");

    tmp_str = gsb_locale_get_print_locale_var ();
    g_printf ("%s", tmp_str);

    g_free (tmp_str);

    g_printf ("gint64\n\tG_GINT64_MODIFIER = \"%s\"\n"
                        "\t%"G_GINT64_MODIFIER"d\n\n",
                        G_GINT64_MODIFIER,
                        G_MAXINT64);

    tmp_str = gsb_dirs_get_print_dir_var ();
    g_printf ("%s", tmp_str);

    g_free (tmp_str);

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
static gboolean grisbi_app_cmdline (GApplication *application,
									GApplicationCommandLine *cmdline)
{
	GrisbiAppPrivate *priv;
	GVariantDict *options;
	gchar *tmp_str = NULL;
	const gchar **remaining_args;

	priv = grisbi_app_get_instance_private (GRISBI_APP (application));

	/* initialisation de debug_level à -1 */
	priv->debug_level = -1;

	/* traitement des autres options */
	options = g_application_command_line_get_options_dict (cmdline);

	g_variant_dict_lookup (options, "new-window", "b", &priv->new_window);
	g_variant_dict_lookup (options, "debug", "s", &tmp_str);
	g_variant_dict_lookup (options, "d", "s", &tmp_str);

    /* Parse filenames */
	if (g_variant_dict_lookup (options, G_OPTION_REMAINING, "^a&ay", &remaining_args))
	{
		gint i;

		for (i = 0; remaining_args[i]; i++)
		{
			if (g_file_test (remaining_args[i], G_FILE_TEST_EXISTS))
			{
				priv->file_list = g_slist_prepend (priv->file_list, g_strdup (remaining_args[i]));
			}
		}

		if (g_slist_length (priv->file_list) > 1)
			priv->file_list = g_slist_reverse (priv->file_list);

		g_free (remaining_args);
	}

	/* modification du niveau de débug si besoin */
	if (tmp_str && strlen (tmp_str) > 0)
	{
		gchar *endptr;
		gint64 number;

		errno = 0;
		number = g_ascii_strtoll (tmp_str, &endptr, 10);
		if (endptr == NULL)
			priv->debug_level = (gint)number;
		else if (errno == 0 && number == 0)
			priv->debug_level = (gint)number;
	}

	if (IS_DEVELOPMENT_VERSION == 1)
    {
		if (priv->debug_level >= 0 && priv->debug_level < 5)
			debug_set_cmd_line_debug_level (priv->debug_level);
    }
	else if (priv->debug_level > 0)
	{
		debug_initialize_debugging (priv->debug_level);
        grisbi_app_print_environment_var ();
	}

	if (priv->new_window)
		grisbi_app_create_window (GRISBI_APP (application), NULL);

	g_application_activate (application);

	return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static gint grisbi_app_handle_local_options (GApplication *app,
											 GVariantDict *options)
{
    if (g_variant_dict_contains (options, "version"))
    {
        g_print ("%s - Version %s\n", g_get_application_name (), VERSION);
        g_print("\n\n");
        g_print("%s", extra_support ());
        return 0;
    }

    return -1;
}

/**
 * On detourne les signaux SIGINT, SIGTERM, SIGSEGV
 *
 * \param
 *
 * \return
 * */
static void grisbi_app_trappe_signaux (void)
{
#ifndef G_OS_WIN32
    struct sigaction sig_sev;

    memset (&sig_sev, 0, sizeof (struct sigaction));
    sig_sev.sa_handler = debug_traitement_sigsegv;
    sig_sev.sa_flags = 0;
    sigemptyset (&(sig_sev.sa_mask));

    if (sigaction (SIGINT, &sig_sev, NULL))
        g_print (_("Error on sigaction: SIGINT won't be trapped\n"));

    if (sigaction (SIGTERM, &sig_sev, NULL))
        g_print (_("Error on sigaction: SIGTERM won't be trapped\n"));

    if (sigaction (SIGSEGV, &sig_sev, NULL))
        g_print (_("Error on sigaction: SIGSEGV won't be trapped\n"));
#endif /* G_OS_WIN32 */
}

/**
 * Load file if necessary
 *
 * \param GApplication *app
 *
 * \return
 **/
static gboolean grisbi_app_load_file_if_necessary (GrisbiApp *app)
{
    GrisbiAppPrivate *priv;

    priv = grisbi_app_get_instance_private (GRISBI_APP (app));

    /* check the command line, if there is something to open */
    if (priv->file_list)
    {
		gchar *tmp_str = NULL;
		GSList *tmp_list;

		tmp_list = 	priv->file_list;

        /* on n'ouvre que le premier fichier de la liste */
        tmp_str = tmp_list -> data;

        if (gsb_file_open_file (tmp_str))
        {
			utils_files_append_name_to_recent_array (tmp_str);
            return TRUE;
        }
        else
            return FALSE;
	}
    else
    {
        /* open the last file if needed, filename was set while loading the configuration */
        if (conf.dernier_fichier_auto && conf.last_open_file)
        {
            if (!gsb_file_open_file (conf.last_open_file))
            {
                g_free (conf.last_open_file);
                conf.last_open_file = NULL;

                return FALSE;
            }
            else
                return TRUE;
        }
    }

    return FALSE;
}

/**
 * lancement de l'application
 *
 * \param GApplication *app
 *
 * \return
 **/
static void grisbi_app_startup (GApplication *application)
{
	GrisbiApp *app = GRISBI_APP (application);
    GFile *file = NULL;
    gchar *tmp_dir;
    GtkSettings* settings;
	gboolean has_app_menu = FALSE;

	/* Chain up parent's startup */
    G_APPLICATION_CLASS (grisbi_app_parent_class)->startup (application);

	settings = gtk_settings_get_default ();
    if (settings)
    {
        g_object_get (G_OBJECT (settings),
                      "gtk-shell-shows-app-menu", &has_app_menu,
                      NULL);
    }

    /* on commence par détourner le signal SIGSEGV */
    grisbi_app_trappe_signaux ();

    /* initialisation des variables de configuration globales */
    grisbi_settings_get ();

	/* set language and init locale parameters */
	gsb_locale_init_language (conf.language_chosen);
	gsb_locale_init_lconv_struct ();

	/* Print variables if necessary */
	if (IS_DEVELOPMENT_VERSION == 1)
    {
        grisbi_app_print_environment_var ();
    }

	/* MAJ de has_app_menu */
    if (conf.force_classic_menu)
        has_app_menu = FALSE;

    /* load the CSS properties */
    css_provider = gtk_css_provider_get_default ();
    tmp_dir = g_strconcat (gsb_dirs_get_ui_dir (), "/grisbi.css", NULL);
    file = g_file_new_for_path (tmp_dir);
    if (!gtk_css_provider_load_from_file (css_provider, file, NULL))
        warning_debug (tmp_dir);
    g_free (tmp_dir);
	g_object_unref (file);

    /* initialise les couleurs */
    gsb_rgba_initialise_couleurs_par_defaut ();

	/* initialise les variables d'état */
    init_variables ();

	/* enregistre les formats d'importation */
    gsb_import_register_import_formats ();

    /* charge les raccourcis claviers */
    grisbi_app_setup_accelerators (application, has_app_menu);

	/* app menu */
    grisbi_app_set_main_menu (app, has_app_menu);
}

/**
 * grisbi_app_activate
 *
 * \param GApplication *app
 *
 * \return
 **/
static void grisbi_app_activate (GApplication *application)
{

	GrisbiWin *win;
    gboolean load_file = FALSE;

	devel_debug (NULL);

	/* création de la fenêtre pincipale */
    win = grisbi_app_create_window (GRISBI_APP (application), NULL);

    /* set the CSS properties */
    if (css_provider)
        gtk_style_context_add_provider_for_screen (gdk_display_get_default_screen (
                                                    gdk_display_get_default ()),
                                                    GTK_STYLE_PROVIDER (css_provider),
                                                    GTK_STYLE_PROVIDER_PRIORITY_USER);

    /* lance un assistant si première utilisation */
    if (conf.first_use)
    {
        gsb_assistant_first_run ();
        conf.first_use = FALSE;
    }
    else
    {
        /* ouvre un fichier si demandé */
        load_file = grisbi_app_load_file_if_necessary (GRISBI_APP (application));
        if (load_file)
            display_tip (FALSE);
		else
		{
			grisbi_win_stack_box_show (win, "accueil_page");
			grisbi_win_status_bar_message (_("Start an action!"));
		}
    }
}

/**
 * grisbi_app_open
 *
 * \param GApplication *app
 *
 * \return
 **/
static void grisbi_app_open (GApplication *application,
							 GFile **files,
							 gint n_files,
							 const gchar *hint)
{
    GList *windows;
    GrisbiWin *win;
    int i;

    windows = gtk_application_get_windows (GTK_APPLICATION (application));
    if (windows)
        win = GRISBI_WIN (windows->data);
    else
        win = grisbi_app_create_window (GRISBI_APP (application), NULL);

    for (i = 0; i < n_files; i++)
	{

/*        grisbi_win_open (win, files[i]);
*/	}
    gtk_window_present (GTK_WINDOW (win));
}

/******************************************************************************/
/* Fonctions propres à l'initialisation de l'application                      */
/******************************************************************************/
/**
 * grisbi_app_dispose
 *
 * \param GApplication *app
 *
 * \return
 **/
static void grisbi_app_dispose (GObject *object)
{
    GrisbiAppPrivate *priv;

	devel_debug (NULL);

	priv = grisbi_app_get_instance_private (GRISBI_APP (object));

    /* liberation de la mémoire utilisée par les objets de priv*/
    g_clear_object (&priv->appmenu);
    g_clear_object (&priv->menubar);

	if (priv->recent_array && g_strv_length (priv->recent_array) > 0)
		g_strfreev (priv->recent_array);

    G_OBJECT_CLASS (grisbi_app_parent_class)->dispose (object);
}

/**
 * grisbi_app_shutdown
 *
 * \param GApplication *app
 *
 * \return
 **/
static void grisbi_app_shutdown (GApplication *application)
{
	devel_debug (NULL);

    /* clean finish of the debug file */
    if (etat.debug_mode)
        debug_finish_log ();

    /* on libère la mémoire utilisée par etat */
    free_variables ();

	/* libération de mémoire utilisée par locale*/
    gsb_locale_shutdown ();

	/* libération de mémoire utilisée par gsb_dirs*/
    gsb_dirs_shutdown ();

    /* Sauvegarde de la configuration générale */
    grisbi_settings_save_app_config ();

	/* on libère la mémoire utilisée par conf */
    grisbi_app_struct_conf_free ();

    G_APPLICATION_CLASS (grisbi_app_parent_class)->shutdown (application);
}

/**
 * grisbi_app_init
 *
 * \param GApplication *app
 *
 * \return
 **/
static void grisbi_app_init (GrisbiApp *app)
{
    /* initialize debugging */
    if (IS_DEVELOPMENT_VERSION == 1)
        debug_initialize_debugging (5);

    g_set_application_name ("Grisbi");

	/* add options for app */
    g_application_add_main_option_entries (G_APPLICATION (app), options);

    /* initialisation de la variable conf */
    grisbi_app_struct_conf_init ();
}

/**
 * grisbi_app_class_init
 *
 * \param GrisbiAppClass    *class
 *
 * \return
 **/
static void grisbi_app_class_init (GrisbiAppClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GApplicationClass *app_class = G_APPLICATION_CLASS (klass);

    app_class->startup = grisbi_app_startup;
    app_class->activate = grisbi_app_activate;
    app_class->open = grisbi_app_open;
    app_class->handle_local_options = grisbi_app_handle_local_options;
    app_class->command_line = grisbi_app_cmdline;

    app_class->shutdown = grisbi_app_shutdown;
    object_class->dispose = grisbi_app_dispose;
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 * get active window.
 *
 * \param app
 *
 * \return active_window
 */
GrisbiWin *grisbi_app_get_active_window (GrisbiApp *app)
{
	GrisbiWin *win;

    if (app == NULL)
        app = GRISBI_APP (g_application_get_default ());

    win = GRISBI_WIN (gtk_application_get_active_window (GTK_APPLICATION (app)));

    return win;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gboolean grisbi_app_get_has_app_menu (GrisbiApp *app)
{
	GrisbiAppPrivate *priv;

	priv = grisbi_app_get_instance_private (GRISBI_APP (app));

	return priv->has_app_menu;


}

/**
 * grisbi_app_get_menu_edit
 *
 * \param
 *
 * \return GMenu
 * */
GMenu *grisbi_app_get_menu_edit (void)
{
	GApplication *app;
	GrisbiAppPrivate *priv;

    app = g_application_get_default ();
	priv = grisbi_app_get_instance_private (GRISBI_APP (app));

	return priv->item_edit;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
GAction *grisbi_app_get_prefs_action (void)
{
    GrisbiAppPrivate *priv;

    priv = grisbi_app_get_instance_private (GRISBI_APP (g_application_get_default ()));

    return priv->prefs_action;
}

/**
 * cherche si le fichier est déjà utilisé
 *
 * \param gchar		filename
 *
 * \return TRUE is duplicate file FALSE otherwise
 **/
gboolean grisbi_app_is_duplicated_file (const gchar *filename)
{
	GrisbiApp *app;
	GList *windows;
	GList *tmp_list;

	app = GRISBI_APP (g_application_get_default ());

	windows = gtk_application_get_windows (GTK_APPLICATION (app));
	tmp_list = windows;

	while (tmp_list)
	{
		GrisbiWin *win;
		const gchar *tmp_filename;
		gchar *key1;
		gchar *key2;

		win = tmp_list->data;
		tmp_filename = grisbi_win_get_filename (win);
		key1 = g_utf8_collate_key_for_filename (filename, -1);
		key2 = g_utf8_collate_key_for_filename (tmp_filename, -1);

		if (strcmp (key1, key2) == 0)
		{
			g_free (key1);
			g_free (key2);
			return TRUE;
		}

		tmp_list = tmp_list->next;
	}
    return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gchar **grisbi_app_get_recent_files_array (void)
{
	GApplication *app;
	GrisbiAppPrivate *priv;

    app = g_application_get_default ();
	priv = grisbi_app_get_instance_private (GRISBI_APP (app));

	return priv->recent_array;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void grisbi_app_set_recent_files_array (gchar **recent_array)
{
	GApplication *app;
	GrisbiAppPrivate *priv;

	app = g_application_get_default ();
	priv = grisbi_app_get_instance_private (GRISBI_APP (app));

	priv->recent_array = recent_array;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void grisbi_app_update_recent_files_menu (void)
{
	GrisbiApp *app;
    GrisbiAppPrivate *priv;
	GMenuItem *menu_item;
    gchar *detailled_action;
    gint index = 0;
	gint i = 0;

    devel_debug (NULL);

    app = GRISBI_APP (g_application_get_default ());
    priv = grisbi_app_get_instance_private (GRISBI_APP (app));

	/* on vide le sous menu recent_files */
	g_menu_remove_all (priv->item_recent_files);

	for (i = 0 ; i < conf.nb_derniers_fichiers_ouverts ; i++)
	{
		gchar *menu_name;

		detailled_action = g_strdup_printf ("win.direct-open-file::%d", index+1);
		menu_name = gsb_menu_normalise_label_name (priv->recent_array[i]);
		menu_item = g_menu_item_new (menu_name, detailled_action);
		g_menu_append_item (priv->item_recent_files, menu_item);
		index++;
		g_free (menu_name);
		g_free (detailled_action);
		g_object_unref (menu_item);
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
