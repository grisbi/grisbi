/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2023 Pierre Biava (grisbi@pierre.biava.name)                    */
/*          https://www.grisbi.org/                                              */
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

#include "config.h"
#include "include.h"
#include <stdlib.h>
#include <glib/gprintf.h>
#include <glib/gi18n.h>
#include <errno.h>

/*START_INCLUDE*/
#include "grisbi_app.h"
#include "dialog.h"
#include "gsb_assistant_first.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "grisbi_conf.h"
#include "gsb_file_save.h"
#include "gsb_locale.h"
#include "gsb_rgba.h"
#include "gsb_select_icon.h"
#include "help.h"
#include "import.h"
#include "menu.h"
#include "navigation.h"
#include "structures.h"
#include "tip.h"
#include "traitement_variables.h"
#include "utils_files.h"
#include "utils_prefs.h"
#include "utils_str.h"
#include "erreur.h"
#include "utils.h"

#ifdef GTKOSXAPPLICATION
#include <gtkosxapplication.h>
#endif
/*END_INCLUDE*/

/*START_STATIC*/
static GtkCssProvider *	css_provider = NULL;	/* css provider */
static gchar *			css_data = NULL;		/* fichier css sous forme de string */
static gboolean			has_started = FALSE;	/* TRUE when grisbi_app_activate() finishes */

static GrisbiWin *grisbi_app_create_window (GrisbiApp *app,
											GdkScreen *screen);
/*END_STATIC*/

/*START_EXTERN*/
gboolean				darkmode = FALSE;		/* use to set darkmode from command_line */
/*END_EXTERN*/

typedef struct	_GrisbiApp			GrisbiApp;
typedef struct	_GrisbiAppPrivate	GrisbiAppPrivate;

struct _GrisbiApp
{
	GtkApplication parent;
};

struct _GrisbiAppPrivate
{
	/* command line parsing */
	gboolean			new_window;
	gint				debug_level;
	GSList *			file_list;

	/* Menuapp et menubar */
	gchar **			recent_array;
	gboolean			has_app_menu;
	GMenuModel *		appmenu;
	GMenuModel *		menubar;
	GMenu *				item_recent_files;
	GMenu *				item_edit;
	GAction *			prefs_action;

	/* config structure */
	GrisbiAppConf		*a_conf;
};

G_DEFINE_TYPE_WITH_PRIVATE (GrisbiApp, grisbi_app, GTK_TYPE_APPLICATION)

struct AcceleratorStruct
{
	const gchar *action_and_target;
	const gchar *accelerators[2];
	const gchar *translate_string;
};

static struct AcceleratorStruct accels[] = {
		{ "app.new-window", { "<Alt>n", NULL }, N_("New_window")},
		{ "app.prefs", { "<Primary><Shift>p", NULL }, N_("Preferences")},
		{ "app.quit", { "<Primary>q", NULL }, N_("Quit")},
		{ "win.new-acc-file", { "<Primary>n", NULL }, N_("New account file")},
		{ "win.open-file", { "<Primary>o", NULL }, N_("Open")},
		{ "win.save", { "<Primary>s", NULL }, N_("Save")},
		{ "win.import-file", { "<Primary>i", NULL }, N_("Import file")},
		{ "win.export-accounts", { "<Primary>e", NULL }, N_("Export as QIF/CSV file")},
		{ "win.file-close", { "<Primary>w", NULL }, N_("Close")},
		{ "win.new-ope", { "<Primary>t", NULL }, N_("New transaction")},
		{ "win.new-acc", { "<Primary><Shift>n", NULL }, N_("New account")},
		{ "win.show-reconciled", { "<Alt>r", NULL }, N_("Show reconciled")},
		{ "win.show-archived", { "<Alt>l", NULL }, N_("Show lines archives")},
		{ "win.manual", { "F1", NULL }, N_("User's Manual")},
		{ "win.fullscreen", {"F11", NULL}, N_("Full screen")}
	};
static struct AcceleratorStruct accels_classic[] = {
		{ "win.new-window", { "<Alt>n", NULL }, N_("New_window")},
		{ "win.prefs", { "<Primary><Shift>p", NULL }, N_("Preferences")},
		{ "win.quit", { "<Primary>q", NULL }, N_("Quit")},
		{ "win.new-acc-file", { "<Primary>n", NULL }, N_("New account file")},
		{ "win.open-file", { "<Primary>o", NULL }, N_("Open")},
		{ "win.save", { "<Primary>s", NULL }, N_("Save")},
		{ "win.import-file", { "<Primary>i", NULL }, N_("Import file")},
		{ "win.export-accounts", { "<Primary>e", NULL }, N_("Export as QIF/CSV file")},
		{ "win.file-close", { "<Primary>w", NULL }, N_("Close")},
		{ "win.new-ope", { "<Primary>t", NULL }, N_("New transaction")},
		{ "win.new-acc", { "<Primary><Shift>n", NULL }, N_("New account")},
		{ "win.show-reconciled", { "<Alt>r", NULL }, N_("Show reconciled")},
		{ "win.show-archived", { "<Alt>l", NULL }, N_("Show lines archives")},
		{ "win.manual", { "F1", NULL }, N_("User's Manual")},
		{ "win.fullscreen", {"F11", NULL}, N_("Full screen")}
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
static void grisbi_app_struct_conf_init (GrisbiApp *app)
{
	GrisbiAppPrivate *priv;

	devel_debug (NULL);
	priv = grisbi_app_get_instance_private (GRISBI_APP (app));

	priv->a_conf = g_malloc0 (sizeof (GrisbiAppConf));

	(priv->a_conf)->browser_command = NULL;
	(priv->a_conf)->current_theme = g_strdup ("null");
	(priv->a_conf)->font_string = NULL;
	(priv->a_conf)->import_directory = NULL;
	(priv->a_conf)->language_chosen = NULL;
	(priv->a_conf)->last_open_file = NULL;
}

/**
 * libération de la mémoire de la variable conf
 *
 * \param
 *
 * \return
 **/
static void grisbi_app_struct_conf_free (GrisbiApp *app)
{
	GrisbiAppPrivate *priv;

	devel_debug (NULL);
	priv = grisbi_app_get_instance_private (GRISBI_APP (app));

	g_free ((priv->a_conf)->current_theme);
	if ((priv->a_conf)->font_string)
	{
		g_free ((priv->a_conf)->font_string);
		(priv->a_conf)->font_string = NULL;
	}
	if ((priv->a_conf)->browser_command)
	{
		g_free ((priv->a_conf)->browser_command);
		(priv->a_conf)->browser_command = NULL;
	}

	if ((priv->a_conf)->import_directory)
	{
		g_free ((priv->a_conf)->import_directory);
		(priv->a_conf)->import_directory = NULL;
	}

	if ((priv->a_conf)->language_chosen)
	{
		g_free ((priv->a_conf)->language_chosen);
		(priv->a_conf)->language_chosen = NULL;
	}

	if ((priv->a_conf)->last_open_file)
	{
		g_free ((priv->a_conf)->last_open_file);
		(priv->a_conf)->last_open_file = NULL;
	}

	g_free (priv->a_conf);

	gsb_file_free_last_path ();
	gsb_file_free_backup_path ();
}

/* ACCELERATORS*/
/**
 * Charge les raccourcis claviers (non modifiables)
 *
 * \param GrisbiApp	 *app
 *
 * \return
 **/
static void grisbi_app_setup_accelerators (GApplication *application,
										   gboolean has_app_menu)
{
	guint i = 0;

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
 * \param
 * \param
 *
 * \return
 **/
static void grisbi_app_save_win_geometry_data (GrisbiApp *app,
											   GtkWindow *win)
{
	GrisbiAppPrivate *priv;

	priv = grisbi_app_get_instance_private (GRISBI_APP (app));

	/* sauvegarde la position de la fenetre principale */
	gtk_window_get_position (GTK_WINDOW (win), &(priv->a_conf)->x_position, &(priv->a_conf)->y_position);

	/* sauvegarde de la taille de la fenêtre si nécessaire */
	gtk_window_get_size (GTK_WINDOW (win), &(priv->a_conf)->main_width, &(priv->a_conf)->main_height);
}

/**
 *
 *
 * \param GSimpleAction		action
 * \param GVariant			state
 * \param gpointer			user_data
 *
 * \return
 **/
static void grisbi_app_change_fullscreen_state (GSimpleAction *action,
												GVariant *state,
												gpointer user_data)
{
	GtkWindow *win;
	GrisbiApp *app;
	GrisbiAppPrivate *priv;

	app = GRISBI_APP (user_data);
	priv = grisbi_app_get_instance_private (GRISBI_APP (app));

	win = GTK_WINDOW (grisbi_app_get_active_window (app));
	if (g_variant_get_boolean (state))
		gtk_window_fullscreen (win);
	else
		gtk_window_unfullscreen (win);

	g_simple_action_set_state (action, state);
}

/**
 *
 *
 * \param GSimpleAction	 	action
 * \param GVariant			state
 * \param gpointer			user_data
 *
 * \return
 **/
static void grisbi_app_change_radio_state (GSimpleAction *action,
										   GVariant *state,
										   gpointer user_data)
{
	g_simple_action_set_state (action, state);
}

/**
 *
 *
 * \param GSimpleAction		action
 * \param GVariant			parameter
 * \param gpointer			user_data
 *
 * \return
 **/
static void grisbi_app_new_window (GSimpleAction *action,
								   GVariant *parameter,
								   gpointer user_data)
{
	GrisbiApp *app;

	app = GRISBI_APP (user_data);
	grisbi_app_create_window (GRISBI_APP (app), NULL);
}

/**
 *
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
static void grisbi_app_quit (GSimpleAction *action,
							 GVariant *parameter,
							 gpointer user_data)
{
	GList *l;
	gboolean first_win = TRUE;
	GrisbiApp *app;
	GrisbiWinRun *w_run;

	app = GRISBI_APP (user_data);
	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();

	/* do not exit while saving. See bug #1969 */
	if (w_run->menu_save)
		return;

	/* Do not exit while the preferences dialog is open.
	 * Otherwise we get a crash when the dialog is closed. */
	if (w_run->menu_prefs)
		return;

	/* Remove all windows registered in the application */
	while ((l = gtk_application_get_windows (GTK_APPLICATION (app))))
	{
		if (first_win)
		{
			grisbi_app_save_win_geometry_data (app, GTK_WINDOW (l->data));
			first_win = FALSE;
		}
		if (gsb_file_quit ())
		{
			grisbi_win_close_window (GTK_WINDOW (l->data));
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
	{ "day-tip", grisbi_cmd_day_tip, NULL, NULL, NULL },
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
	{ "search-acc", grisbi_cmd_search_acc, NULL, NULL, NULL },
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

	if (!priv->recent_array || (priv->recent_array && g_strv_length (priv->recent_array) == 0))
	{
		return;
	}

	for (i = 0 ; i < (priv->a_conf)->nb_derniers_fichiers_ouverts ; i++)
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

			for (j = i; j < (priv->a_conf)->nb_derniers_fichiers_ouverts -1; j++)
			{
				priv->recent_array[j] = priv->recent_array[j+1];
			}
		}
	}
	if (index < (priv->a_conf)->nb_derniers_fichiers_ouverts)
	{
		priv->recent_array = g_realloc (priv->recent_array, (index + 1) * sizeof (gchar*));
		priv->recent_array[index] = NULL;
		(priv->a_conf)->nb_derniers_fichiers_ouverts = index;
	}
}


#ifdef GTKOSXAPPLICATION
static gboolean grisbi_app_osx_openfile_callback(GtkosxApplication *osxapp,
		gchar const *path, GrisbiApp *app)
{
	GrisbiWinRun *w_run;

	devel_debug (NULL);
	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();

	/* continue only if can close the current file */
	if (w_run->file_is_loading && !gsb_file_close ())
		return TRUE;

	if (path)
	{
		if (has_started)
		{
			if (gsb_file_open_file (path))
			{
				if (!w_run->file_is_loading)
				{
					gsb_gui_navigation_select_line (NULL, NULL);
					w_run->file_is_loading = TRUE;
				}
				utils_files_append_name_to_recent_array (path);
			}
		}
		else
		{
			/* simulate an argument on the command line
			 as if we start Grisbi with the filename as argument */
			GrisbiAppPrivate *priv;

			priv = grisbi_app_get_instance_private (GRISBI_APP (app));
			priv->file_list = g_slist_prepend (priv->file_list, g_strdup (path));
		}
	}

	return TRUE;
}
#endif

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

		g_action_map_add_action_entries (G_ACTION_MAP (app), app_entries, G_N_ELEMENTS (app_entries), app);

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

#ifdef GTKOSXAPPLICATION
	GtkosxApplication *osxapp = gtkosx_application_get();
	g_signal_connect(G_OBJECT(osxapp), "NSApplicationOpenFile", G_CALLBACK(grisbi_app_osx_openfile_callback), app);
#endif
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
	gboolean result = FALSE;

	devel_debug (NULL);
	l = gtk_application_get_windows (GTK_APPLICATION (app));
	if (g_list_length (l) == 1)
	{
		last_win = TRUE;
	}

	grisbi_app_save_win_geometry_data (app, GTK_WINDOW (win));
	if (last_win)
		result = gsb_file_quit ();
	else
		result = gsb_file_close ();

	if (result)
	{
		grisbi_win_close_window (GTK_WINDOW (win));
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
	GdkWindow *window;
	GdkDisplay *display;
	GdkMonitor *monitor;
	GdkRectangle rectangle;
	GrisbiWin *win;
	GrisbiAppPrivate *priv;

	priv = grisbi_app_get_instance_private (GRISBI_APP (app));

	win = g_object_new (GRISBI_TYPE_WIN, "application", app, NULL);

	g_signal_connect (win,
					  "delete_event",
					  G_CALLBACK (grisbi_app_window_delete_event),
					  app);

	/* set signal for detecting new theme */
	g_signal_connect (win,
					  "style-updated",
					  G_CALLBACK (grisbi_app_window_style_updated),
					  GINT_TO_POINTER (FALSE));

	gtk_application_add_window (GTK_APPLICATION (app), GTK_WINDOW (win));

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
		g_action_map_add_action_entries (G_ACTION_MAP (win), app_entries, G_N_ELEMENTS (app_entries), app);

		priv->prefs_action = g_action_map_lookup_action (G_ACTION_MAP (win), "prefs");
	}

	grisbi_win_init_menubar (win, app);

	/* affiche la fenêtre principale */
	gtk_window_present (GTK_WINDOW (win));

	if (screen != NULL)
		gtk_window_set_screen (GTK_WINDOW (win), screen);

	/* on teste s'il faut changer de résolution */
	if (!(priv->a_conf)->low_definition_screen)
	{
		window = gtk_widget_get_window (GTK_WIDGET (win));
		display = gdk_window_get_display (GDK_WINDOW (window));
		monitor = gdk_display_get_monitor_at_point (display, 0, 0);
		gdk_monitor_get_geometry (monitor, &rectangle);

		if (rectangle.width < LOW_DEF_WIDTH_HIGH || rectangle.height < LOW_DEF_HEIGHT_HIGH)
		{
			(priv->a_conf)->low_definition_screen = TRUE;
			(priv->a_conf)->main_height = WIN_MIN_HEIGHT;
			(priv->a_conf)->main_width = WIN_MIN_WIDTH;
			(priv->a_conf)->panel_width = PANEL_MIN_WIDTH;

			gtk_window_resize (GTK_WINDOW (grisbi_app_get_active_window (NULL)),
							   (priv->a_conf)->main_width,
							   (priv->a_conf)->main_height);
		}
	}
	return win;
}

#ifdef DEBUG
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

	g_printf ("gint64\n\tG_GINT64_MODIFIER = \"%s\"\n" "\t%"G_GINT64_MODIFIER"d\n\n", G_GINT64_MODIFIER, G_MAXINT64);

	tmp_str = gsb_dirs_get_print_dir_var ();
	g_printf ("%s", tmp_str);

	g_free (tmp_str);

	return FALSE;
}
#endif

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
	GrisbiWinRun *w_run;
	GrisbiAppPrivate *priv;

	/* let a chance to events (like grisbi_app_osx_openfile_callback) to be handled */
	update_gui();

	priv = grisbi_app_get_instance_private (GRISBI_APP (app));

	/* open the last file if needed, filename was set while loading the configuration */
	if ((priv->a_conf)->dernier_fichier_auto && (priv->a_conf)->last_open_file)
	{
		if (!gsb_file_open_file ((priv->a_conf)->last_open_file))
		{
			g_free ((priv->a_conf)->last_open_file);
			(priv->a_conf)->last_open_file = NULL;

			return FALSE;
		}
		else
		{
			w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
			w_run->file_is_loading = TRUE;

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
	GtkSettings* settings;
	gchar *css_filename;
	gchar *theme_name = NULL;
	gboolean has_app_menu = FALSE;
	GrisbiAppPrivate *priv;

	/* Chain up parent's startup */
	G_APPLICATION_CLASS (grisbi_app_parent_class)->startup (application);

	priv = grisbi_app_get_instance_private (GRISBI_APP (application));

	/* on commence par détourner le signal SIGSEGV */
	grisbi_app_trappe_signaux ();

	/* initialisation de la variable conf */
	grisbi_app_struct_conf_init (app);

	/* initialisation des variables de configuration globales */
	grisbi_conf_load_app_config ();

	/* force dark type */
	if (darkmode)
	{
		(priv->a_conf)->force_type_theme = 2;
	}

	settings = gtk_settings_get_default ();
	if (settings)
	{
		g_object_get (G_OBJECT (settings),
					  "gtk-shell-shows-app-menu", &has_app_menu,
					  "gtk-theme-name", &theme_name,
					  NULL);

		if (g_strcmp0 ((priv->a_conf)->current_theme, theme_name) == 0)
		{

			if ((priv->a_conf)->force_type_theme)
				(priv->a_conf)->use_type_theme = (priv->a_conf)->force_type_theme;
			else	/* mode automatique */
				(priv->a_conf)->use_type_theme = gsb_rgba_get_type_theme (theme_name);

			g_free (theme_name);
		}
		else
		{
			g_free ((priv->a_conf)->current_theme);
			(priv->a_conf)->current_theme = theme_name;
			(priv->a_conf)->use_type_theme = gsb_rgba_get_type_theme (theme_name);
			(priv->a_conf)->force_type_theme = 0;
		}
	}

	/* set language and init locale parameters */
	gsb_locale_init_language ((priv->a_conf)->language_chosen);
	gsb_locale_init_lconv_struct ();

	/* Print variables if necessary */
#ifdef DEBUG
	grisbi_app_print_environment_var ();
#endif

#ifdef OS_OSX
	has_app_menu = TRUE;
#else
	has_app_menu = FALSE;
#endif /* OS_OSX */

	/* load the CSS properties */
	css_provider = gtk_css_provider_new ();
	css_filename = gsb_css_get_filename ();
	file = g_file_new_for_path (css_filename);
	gtk_css_provider_load_from_file (css_provider, file, NULL);
	g_free (css_filename);
	g_object_unref (file);

	/* récupération des données du fichier grisbi.css*/
	css_data = gtk_css_provider_to_string (css_provider);

	/* set the CSS properties */
	gtk_style_context_add_provider_for_screen (gdk_screen_get_default (),
											   GTK_STYLE_PROVIDER (css_provider),
											   GTK_STYLE_PROVIDER_PRIORITY_USER);

	/* initialise les couleurs */
	gsb_rgba_initialise_couleurs (css_data);

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
	GrisbiAppPrivate *priv;

	devel_debug (NULL);
	priv = grisbi_app_get_instance_private (GRISBI_APP (application));

	/* création de la fenêtre pincipale */
	win = grisbi_app_create_window (GRISBI_APP (application), NULL);

	/* lance un assistant si première utilisation */
	if ((priv->a_conf)->first_use)
	{
		gsb_assistant_first_run ();
		(priv->a_conf)->first_use = FALSE;
	}
	else
	{
		/* ouvre un fichier si demandé */
		load_file = grisbi_app_load_file_if_necessary (GRISBI_APP (application));
		if (load_file)
		{
			display_tip (FALSE);

			/* Si sauvegarde automatique on la lance ici */
			if ((priv->a_conf)->make_backup_every_minutes
				&& (priv->a_conf)->make_backup_nb_minutes)
				gsb_file_automatic_backup_start (NULL, NULL);
		}
		else
		{
			grisbi_win_stack_box_show (win, "accueil_page");
			grisbi_win_status_bar_message (_("Start an action!"));
		}
	}

	has_started = TRUE;
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

	windows = gtk_application_get_windows (GTK_APPLICATION (application));
	if (windows)
		win = GRISBI_WIN (windows->data);
	else
		win = grisbi_app_create_window (GRISBI_APP (application), NULL);

	if (n_files == 1)
	{
		grisbi_win_open (win, files[0]);
	}
	else
	{
		gchar *msg;

		msg = g_strdup (_("Grisbi has detected several character strings "
						  "separated by one or more spaces.\n\n"
						  "\t- If it's a file name with spaces, surround it with \"....\"\n\n"
						  "\t- If there are multiple file names, keep one."));

		dialogue_warning_hint (msg,_("Inconsistency of the command line."));
		g_free (msg);

		has_started = TRUE;
	}

	gtk_window_present (GTK_WINDOW (win));
}

/******************************************************************************/
/* Fonctions propres à l'initialisation de l'application                      */
/******************************************************************************/
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
#ifdef DEBUG
	debug_initialize_debugging (5);
#endif

	g_set_prgname ("grisbi");
}

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
	if (priv->appmenu)
		g_clear_object (&priv->appmenu);
	else
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
	GrisbiAppPrivate *priv;

	priv = grisbi_app_get_instance_private (GRISBI_APP (application));
	devel_debug (NULL);

	/* on sauvegarde éventuellement le fichier CSS local */
	gsb_file_save_css_local_file (css_data);

	/* on libère la mémoire utilisée par css_data */
	g_free (css_data);

	/* libération de mémoire utilisée par locale*/
	gsb_locale_shutdown ();

	/* on remet la détection du theme en auto si darkmode = TRUE */
	if (darkmode)
		(priv->a_conf)->force_type_theme = 0;

	/* Sauvegarde de la configuration générale */
	grisbi_conf_save_app_config ();

	/* libération de mémoire utilisée par gsb_dirs*/
	gsb_dirs_shutdown ();

	/* on libère la mémoire utilisée par conf */
	grisbi_app_struct_conf_free (GRISBI_APP (application));

	/* clean finish of the debug file */
	if (debug_get_debug_mode ())
	{
		debug_finish_log ();
	}

	G_APPLICATION_CLASS (grisbi_app_parent_class)->shutdown (application);
}

/**
 * grisbi_app_class_init
 *
 * \param GrisbiAppClass	*class
 *
 * \return
 **/
static void grisbi_app_class_init (GrisbiAppClass *class)
{
	G_APPLICATION_CLASS (class)->startup = grisbi_app_startup;
	G_APPLICATION_CLASS (class)->activate = grisbi_app_activate;
	G_APPLICATION_CLASS (class)->open = grisbi_app_open;

	G_APPLICATION_CLASS (class)->shutdown = grisbi_app_shutdown;
	G_OBJECT_CLASS (class)->dispose = grisbi_app_dispose;
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 * création de l"application grisbi
 *
 * \param
 *
 * \return		grisbi_app
 **/
GrisbiApp *grisbi_app_new (void)
{
	return g_object_new (GRISBI_TYPE_APP,
						 "application-id",   "org.gtk.grisbi",
						 "flags",			G_APPLICATION_HANDLES_OPEN|G_APPLICATION_NON_UNIQUE,
						 NULL);
}

void grisbi_app_window_style_updated (GtkWidget *win,
									  gpointer force)
{
	GtkSettings* settings;
	gint forced = 0;

	g_signal_handlers_block_by_func (G_OBJECT (win),
									 G_CALLBACK (grisbi_app_window_style_updated),
									 force);

	forced = GPOINTER_TO_INT (force);
	settings = gtk_settings_get_default ();
	if (settings)
	{
		GFile *file = NULL;
		gchar *tmp_theme_name;
		const gchar *css_filename;
		GrisbiAppConf *a_conf;

		a_conf = grisbi_app_get_a_conf ();

		g_object_get (G_OBJECT (settings),
					  "gtk-theme-name", &tmp_theme_name,
					  NULL);

		if (g_strcmp0 (tmp_theme_name, a_conf->current_theme) || forced)
		{
			g_free (a_conf->current_theme);
			a_conf->current_theme = tmp_theme_name;

			/* set force_type_theme en automatique sauf si commande forcée */
			if (!forced)
			{
				a_conf->use_type_theme = gsb_rgba_get_type_theme (tmp_theme_name);
				a_conf->force_type_theme = 0;
			}
			else
			{
				a_conf->use_type_theme = a_conf->force_type_theme;
			}

			/* on sauvegarde éventuellement les données locales */
			gsb_file_save_css_local_file (css_data);

			/* on charge les nouvelles données */
			css_filename = gsb_css_get_filename ();
			file = g_file_new_for_path (css_filename);
			gtk_css_provider_load_from_file (css_provider, file, NULL);

			/* on met à jour css_data et les couleurs de base */
			g_free (css_data);
			css_data = gtk_css_provider_to_string (css_provider);
			gsb_rgba_initialise_couleurs (css_data);

			/* MAJ Home page */
			if (grisbi_win_file_is_loading ())
				gsb_gui_navigation_update_home_page_from_theme ();

			/* on ferme les preferences si necessaire */
			utils_prefs_close_prefs_from_theme ();
		}
		else
		{
			g_free (tmp_theme_name);
		}
	}
	g_signal_handlers_unblock_by_func (G_OBJECT (win),
									   G_CALLBACK (grisbi_app_window_style_updated),
									   force);
}

/**
 * Affiche les raccourcis claviers dans un text view
 *
 * \param text_view
 *
 * \return
 **/
void grisbi_app_display_gui_dump_accels (GtkApplication *application,
										 GtkWidget *text_view)
{
	GtkTextBuffer *buffer;
	GtkTextIter iter;
	const gchar *tab = "\t";
	gchar *tmp_str;
	guint i;
	GrisbiAppPrivate *priv;

	priv = grisbi_app_get_instance_private (GRISBI_APP (application));
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
	gtk_text_buffer_set_text (buffer, "", 0);
	gtk_text_buffer_get_iter_at_offset (buffer, &iter, 0);

	if (priv->has_app_menu)
	{
		for (i = 0; i < G_N_ELEMENTS (accels); i++)
		{
			gchar *str;
			gchar *str_to_free;
			gchar *text;

			str_to_free = g_strjoinv (",", (gchar **) accels[i].accelerators);
#ifdef GTKOSXAPPLICATION
			str = gsb_string_remplace_string (str_to_free, "Primary", "Command");
#else
			str = gsb_string_remplace_string (str_to_free, "Primary", "Ctrl");
#endif
			g_free (str_to_free);

			text = g_strdup_printf ("%s%s-> %s.\n", gettext (accels[i].translate_string), tab, str);
			gtk_text_buffer_insert (buffer, &iter, text, -1);
			g_free (str);
			g_free (text);
		}
	}
	else
	{
		for (i = 0; i < G_N_ELEMENTS (accels_classic); i++)
		{
			gchar *str;
			gchar *str_to_free;
			gchar *text;

			str_to_free = g_strjoinv (",", (gchar **) accels[i].accelerators);
#ifdef GTKOSXAPPLICATION
			str = gsb_string_remplace_string (str_to_free, "Primary", "Command");
#else
			str = gsb_string_remplace_string (str_to_free, "Primary", "Ctrl");
#endif
			g_free (str_to_free);

			text = g_strdup_printf ("%s%s-> %s.\n", gettext (accels[i].translate_string), tab, str);
			gtk_text_buffer_insert (buffer, &iter, text, -1);
			g_free (str);
			g_free (text);
		}
	}

	gtk_text_buffer_insert (buffer, &iter, "\n", -1);
	gtk_text_buffer_insert_markup (buffer, &iter, _("<b>Actions in transaction list:</b>"), -1);
	gtk_text_buffer_insert (buffer, &iter, "\n", -1);
#ifdef GTKOSXAPPLICATION
	tmp_str = g_strdup_printf (_("(Un)Pointing a transaction%s-> <Command>p, <Command>F12\n"), tab);
	gtk_text_buffer_insert (buffer, &iter, tmp_str, -1);
	g_free (tmp_str);
	tmp_str = g_strdup_printf (_("(Un)Reconcile a transaction%s-> <Command>r\n"), tab);
	gtk_text_buffer_insert (buffer, &iter, tmp_str, -1);
	g_free (tmp_str);
#else
	tmp_str = g_strdup_printf (_("(Un)Pointing a transaction%s-> <Ctrl>p, <Ctrl>F12\n"), tab);
	gtk_text_buffer_insert (buffer, &iter, tmp_str, -1);
	g_free (tmp_str);
	tmp_str = g_strdup_printf (_("(Un)Reconcile a transaction%s-> <Ctrl>p, <Ctrl>F12\n"), tab);
	gtk_text_buffer_insert (buffer, &iter, tmp_str, -1);
	g_free (tmp_str);
#endif
}

gpointer grisbi_app_get_a_conf (void)
{
	GApplication *app;
	GrisbiAppPrivate *priv;

	app = g_application_get_default ();
	priv = grisbi_app_get_instance_private (GRISBI_APP (app));

	return priv->a_conf;
}

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
gboolean grisbi_app_get_low_definition_screen (void)
{
	GApplication *app;
	GrisbiAppPrivate *priv;

	app = g_application_get_default ();
	priv = grisbi_app_get_instance_private (GRISBI_APP (app));

	return (priv->a_conf)->low_definition_screen;
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

	if ((priv->a_conf)->nb_derniers_fichiers_ouverts == 0)
		return;

	if ((priv->a_conf)->nb_derniers_fichiers_ouverts > (priv->a_conf)->nb_max_derniers_fichiers_ouverts)
		(priv->a_conf)->nb_derniers_fichiers_ouverts = (priv->a_conf)->nb_max_derniers_fichiers_ouverts;

	for (i = 0 ; i < (priv->a_conf)->nb_derniers_fichiers_ouverts ; i++)
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
GtkCssProvider *grisbi_app_get_css_provider (void)
{
	return css_provider;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
const gchar *grisbi_app_get_css_data (void)
{
	return css_data;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void grisbi_app_set_css_data (const gchar *new_css_data)
{
	g_free (css_data);
	css_data = g_strdup (new_css_data);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void grisbi_app_set_has_started_true (void)
{
	has_started = TRUE;
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
