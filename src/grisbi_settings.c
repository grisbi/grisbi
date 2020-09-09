/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2020 Pierre Biava (grisbi@pierre.biava.name)                    */
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"
#include <errno.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "grisbi_settings.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "structures.h"
#include "utils_buttons.h"
#include "utils_str.h"
#include "erreur.h"

/*END_INCLUDE*/

/* START_EXTERN Variables externes PROVISOIRE */
/*END_EXTERN*/

struct _GrisbiSettingsPrivate
{
    GSettings *settings_root;
    GSettings *settings_backup;
    GSettings *settings_display;
    GSettings *settings_file;
    GSettings *settings_form;
    GSettings *settings_general;
    GSettings *settings_geometry;
    GSettings *settings_messages_delete;
    GSettings *settings_messages_tips;
    GSettings *settings_messages_warnings;
    GSettings *settings_panel;
    GSettings *settings_prefs;
    GSettings *settings_scheduled;
};

G_DEFINE_TYPE_WITH_PRIVATE (GrisbiSettings, grisbi_settings, G_TYPE_OBJECT)

/* singleton object - all consumers of GrisbiSettings get the same object (refcounted) */
static GrisbiSettings *singleton = NULL;


/*******************************************************************************
 * Private Methods
 ******************************************************************************/
/**
 * set root settings
 *
 * \param GSettings     settings
 *
 * \return
 **/
static void grisbi_settings_init_settings_root (GSettings *settings,
												GrisbiAppConf *a_conf)
{
    /* first use */
    a_conf->first_use = g_settings_get_boolean (settings, "first-use");
}

/**
 * set backup settings
 *
 * \param GSettings     settings
 *
 * \return
 **/
static void grisbi_settings_init_settings_backup (GSettings *settings,
												  GrisbiAppConf *a_conf)
{
    gchar *tmp_path;

    /* on fixe le backup path s'il n'est pas initialisé */
    tmp_path = g_settings_get_string (settings, "backup-path");
    if (tmp_path == NULL || strlen (tmp_path) == 0)
    {
        tmp_path = g_strdup (gsb_dirs_get_user_data_dir ());
        g_settings_set_string (settings, "backup-path", tmp_path);
    }
    gsb_file_set_backup_path (tmp_path);
    g_free (tmp_path);

    a_conf->compress_backup = g_settings_get_boolean (settings, "compress-backup");
    a_conf->make_backup_every_minutes = g_settings_get_boolean (settings, "make-backup-every-minutes");
    a_conf->make_backup_nb_minutes = g_settings_get_int (settings, "make-backup-nb-minutes");
    a_conf->make_bakup_single_file = g_settings_get_boolean (settings, "make-backup-single-file");
    a_conf->remove_backup_files = g_settings_get_boolean (settings, "remove-backup-files");
    a_conf->remove_backup_months = g_settings_get_int (settings, "remove-backup-months");
    a_conf->sauvegarde_fermeture = g_settings_get_boolean (settings, "sauvegarde-fermeture");
    a_conf->sauvegarde_demarrage = g_settings_get_boolean (settings, "sauvegarde-ouverture");
}

/**
 * set display settings
 *
 * \param GSettings     settings
 *
 * \return
 **/
static void grisbi_settings_init_settings_display (GSettings *settings,
												   GrisbiAppConf *a_conf)
{
    a_conf->display_window_title = g_settings_get_int (settings, "display-window-title");
    a_conf->display_toolbar = g_settings_get_int (settings, "display-toolbar");
    a_conf->formulaire_toujours_affiche = g_settings_get_boolean (settings, "formulaire-toujours-affiche");
    a_conf->group_partial_balance_under_accounts = g_settings_get_boolean (settings, "group-partial-balance-under-accounts");
    a_conf->show_closed_accounts = g_settings_get_boolean (settings, "show-closed-accounts");
    a_conf->show_headings_bar = g_settings_get_boolean (settings, "show-headings-bar");
}

/**
 * set file settings
 *
 * \param GSettings     settings
 *
 * \return
 **/
static void grisbi_settings_init_settings_file (GSettings *settings,
												GrisbiAppConf *a_conf)
{
	gchar **recent_array;
	gchar *import_directory;

    a_conf->compress_file = g_settings_get_boolean (settings, "compress-file");
    a_conf->dernier_fichier_auto = g_settings_get_boolean (settings, "dernier-fichier-auto");
    a_conf->force_enregistrement = g_settings_get_boolean (settings, "force-enregistrement");
    a_conf->force_import_directory = g_settings_get_boolean (settings, "force-import-directory");
    a_conf->nb_max_derniers_fichiers_ouverts = g_settings_get_int (settings, "nb-max-derniers-fichiers-ouverts");
	a_conf->import_remove_file = g_settings_get_boolean (settings, "import-remove-file");
    a_conf->sauvegarde_auto = g_settings_get_boolean (settings, "sauvegarde-auto");

	if (a_conf->force_import_directory)
	{
		import_directory = g_settings_get_string (settings, "import-directory");
		if (import_directory && strlen (import_directory) > 0)
		{
			a_conf->import_directory = my_strdup (import_directory);
		}
		else
		{
			a_conf->import_directory = my_strdup (g_get_user_special_dir (G_USER_DIRECTORY_DOWNLOAD));
		}
		g_free (import_directory);
	}
	recent_array = g_settings_get_strv (settings, "names-last-files");
    if (recent_array)
	{
        a_conf->nb_derniers_fichiers_ouverts = g_strv_length (recent_array);
        if (a_conf->nb_derniers_fichiers_ouverts > 0)
        {
			if (a_conf->nb_derniers_fichiers_ouverts > a_conf->nb_max_derniers_fichiers_ouverts)
			{
				a_conf->nb_derniers_fichiers_ouverts = a_conf->nb_max_derniers_fichiers_ouverts;
			}

            a_conf->last_open_file = my_strdup (recent_array[0]);
        }
    }

	grisbi_app_set_recent_files_array (recent_array);

    /* archive stuff */
    a_conf->archives_check_auto = g_settings_get_boolean (settings, "archives-check-auto");
    a_conf->max_non_archived_transactions_for_check = g_settings_get_int (settings, "max-transactions-before-warn-archival");
}

/**
 * set form settings
 *
 * \param GSettings     settings
 *
 * \return
 **/
static void grisbi_settings_init_settings_form (GSettings *settings,
												GrisbiAppConf *a_conf)
{
    a_conf->affichage_exercice_automatique = g_settings_get_boolean (settings, "affichage-exercice-automatique");
    a_conf->automatic_completion_payee = g_settings_get_boolean (settings, "automatic-completion-payee");
    a_conf->automatic_erase_credit_debit = g_settings_get_boolean (settings, "automatic-erase-credit-debit");
    a_conf->automatic_recover_splits = g_settings_get_boolean (settings, "automatic-recover-splits");
	a_conf->completion_minimum_key_length = g_settings_get_int (settings, "completion-minimum-key-length");
    a_conf->form_enter_key = g_settings_get_boolean (settings, "form-enter-key");
	a_conf->form_validate_split = g_settings_get_boolean (settings, "form-validate-split");
	a_conf->fyear_combobox_sort_order = g_settings_get_int (settings, "fyear-combobox-sort-order");
    a_conf->limit_completion_to_current_account = g_settings_get_boolean (settings, "limit-completion-current-account");
}

/**
 * set general settings
 *
 * \param GSettings     settings
 *
 * \return
 **/
static void grisbi_settings_init_settings_general (GSettings *settings,
												   GrisbiAppConf *a_conf)
{
    gchar *tmp_str;

	if (a_conf->current_theme)
		g_free (a_conf->current_theme);
	a_conf->current_theme = g_settings_get_string (settings, "current-theme");

	a_conf->custom_fonte_listes = g_settings_get_boolean (settings, "custom-fonte-listes");
    if (a_conf->custom_fonte_listes)
    {
        a_conf->font_string = g_settings_get_string (settings, "font-string");
		if (strlen (a_conf->font_string) == 0)
		{
			a_conf->font_string = my_strdup (_("No font defined"));
			a_conf->custom_fonte_listes = FALSE;
		}
    }

    tmp_str = g_settings_get_string (settings, "browser-command");
    if (tmp_str == NULL || strlen (tmp_str) == 0)
    {
        g_settings_set_string (settings, "browser-command", ETAT_WWW_BROWSER);
        a_conf->browser_command = g_strdup (ETAT_WWW_BROWSER);
    }
    else
    {
        a_conf->browser_command = g_strdup (tmp_str);
        g_free (tmp_str);
    }

    a_conf->force_type_theme = g_settings_get_int (settings, "force-type-theme");

    tmp_str = g_settings_get_string (settings, "language-chosen");
    if (tmp_str == NULL)
    {
        a_conf->language_chosen = NULL;
    }
	else if (strlen (tmp_str) == 0)
    {
        a_conf->language_chosen = NULL;
        g_free (tmp_str);
    }
    else
    {
        a_conf->language_chosen = g_strdup (tmp_str);
        g_free (tmp_str);
    }

    tmp_str = g_settings_get_string (settings, "last-path");
    if (tmp_str == NULL || strlen (tmp_str) == 0)
    {
        tmp_str = g_strdup (g_get_user_special_dir (G_USER_DIRECTORY_DOCUMENTS));
        g_settings_set_string (settings, "last-path", tmp_str);
    }
	else
	{
		if (!g_file_test (tmp_str, G_FILE_TEST_EXISTS))
		{
			g_free (tmp_str);
			tmp_str = g_strdup (g_get_user_special_dir (G_USER_DIRECTORY_DOCUMENTS));
			g_settings_set_string (settings, "last-path", tmp_str);
		}
	}

	gsb_file_init_last_path (tmp_str);
    g_free (tmp_str);

    /* Home page */
    a_conf->pluriel_final = g_settings_get_boolean (settings, "pluriel-final");

    /* variables for the list of categories */
    a_conf->metatree_action_2button_press = g_settings_get_int (settings, "metatree-action-button-press");

    /* variables for the list of transactions */
    a_conf->show_transaction_gives_balance = g_settings_get_boolean (settings, "show-transaction-gives-balance");
    a_conf->show_transaction_selected_in_form = g_settings_get_boolean (settings, "show-transaction-selected-in-form");
    a_conf->transactions_list_primary_sorting = g_settings_get_int (settings, "transactions-list-primary-sorting");
    a_conf->transactions_list_secondary_sorting = g_settings_get_int (settings, "transactions-list-secondary-sorting");
}

/**
 * set geometry
 *
 * \param GSettings     settings
 *
 * \return
 **/
static void grisbi_settings_init_settings_geometry (GSettings *settings,
													GrisbiAppConf *a_conf)
{
    a_conf->low_resolution_screen = g_settings_get_boolean (settings, "low-resolution-screen");
    a_conf->x_position = g_settings_get_int (settings, "x-position");
    a_conf->y_position = g_settings_get_int (settings, "y-position");
    a_conf->main_height = g_settings_get_int (settings, "main-height");
    a_conf->main_width = g_settings_get_int (settings, "main-width");
    a_conf->full_screen = g_settings_get_boolean (settings, "fullscreen");
    a_conf->maximize_screen = g_settings_get_boolean (settings, "maximized");
}

/**
 * set messages delete settings
 *
 * \param GSettings     settings
 *
 * \return
 **/
static void grisbi_settings_init_settings_messages_delete (GSettings *settings,
														   GrisbiAppConf *a_conf)
{
    gint i;
	ConditionalMsg *warning;

	warning = (ConditionalMsg*) dialogue_get_tab_delete_msg ();
    for (i = 0; (warning+i)->name; i ++)
    {
        (warning+i)->hidden = !g_settings_get_boolean (G_SETTINGS (settings), (warning+i)->name);
        if ((warning+i)->hidden == 1)
            (warning+i)->default_answer = 1;
    }
}

/**
 * set messages tips settings
 *
 * \param GSettings     settings
 *
 * \return
 **/
static void grisbi_settings_init_settings_messages_tips (GSettings *settings,
														 GrisbiAppConf *a_conf)
{
    a_conf->last_tip = g_settings_get_int (settings, "last-tip");
    a_conf->show_tip = g_settings_get_boolean (settings, "show-tip");
}

/**
 * set messages warnings settings
 *
 * \param GSettings     settings
 *
 * \return
 **/
static void grisbi_settings_init_settings_messages_warnings (GSettings *settings,
															 GrisbiAppConf *a_conf)
{
    gint i;
	ConditionalMsg *warning;

	warning = (ConditionalMsg*) dialogue_get_tab_warning_msg ();
	for (i = 0; (warning+i)->name; i ++)
    {
        (warning+i)->hidden = !g_settings_get_boolean (G_SETTINGS (settings), (warning+i)->name);
        if ((warning+i)->hidden == 1)
            (warning+i)->default_answer = 1;
    }
}

/**
 * set panel settings
 *
 * \param GSettings     settings
 *
 * \return
 **/
static void grisbi_settings_init_settings_panel (GSettings *settings,
												 GrisbiAppConf *a_conf)
{
    a_conf->active_scrolling_left_pane = g_settings_get_boolean (settings, "active-scrolling-left-pane");
    a_conf->panel_width = g_settings_get_int (settings, "panel-width");
}

/**
 * set prefs settings
 *
 * \param GSettings     settings
 *
 * \return
 **/
static void grisbi_settings_init_settings_prefs (GSettings *settings,
												 GrisbiAppConf *a_conf)
{
    a_conf->prefs_archives_sort_order = g_settings_get_boolean (settings, "prefs-archives-sort-order");
    a_conf->prefs_fyear_sort_order = g_settings_get_boolean (settings, "prefs-fyear-sort-order");
    a_conf->prefs_height = g_settings_get_int (settings, "prefs-height");
    a_conf->prefs_panel_width = g_settings_get_int (settings, "prefs-panel-width");
    a_conf->prefs_width = g_settings_get_int (settings, "prefs-width");
}

/**
 * set scheduled settings
 *
 * \param GSettings     settings
 *
 * \return
 **/
static void grisbi_settings_init_settings_scheduled (GSettings *settings,
													 GrisbiAppConf *a_conf)
{
    a_conf->balances_with_scheduled = g_settings_get_boolean (settings, "balances-with-scheduled");
    a_conf->execute_scheduled_of_month = g_settings_get_boolean (settings, "execute-scheduled-of-month");
    a_conf->nb_days_before_scheduled = g_settings_get_int (settings, "nb-days-before-scheduled");
    a_conf->scheduler_set_fixed_day = g_settings_get_boolean (settings, "scheduler-set-fixed-day");
    a_conf->scheduler_fixed_day = g_settings_get_int (settings, "scheduler-fixed-day");
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void grisbi_settings_init (GrisbiSettings *self)
{
    GrisbiSettingsPrivate *priv;
	GrisbiAppConf *a_conf;

    devel_debug (NULL);

	a_conf = grisbi_app_get_a_conf ();

    priv = grisbi_settings_get_instance_private (self);

    priv->settings_root = g_settings_new ("org.gtk.grisbi");
    grisbi_settings_init_settings_root (priv->settings_root, a_conf);

    priv->settings_backup = g_settings_new ("org.gtk.grisbi.files.backup");
    grisbi_settings_init_settings_backup (priv->settings_backup, a_conf);

    priv->settings_display = g_settings_new ("org.gtk.grisbi.display");
    grisbi_settings_init_settings_display (priv->settings_display, a_conf);

    priv->settings_file = g_settings_new ("org.gtk.grisbi.files.file");
    grisbi_settings_init_settings_file (priv->settings_file, a_conf);

    priv->settings_form = g_settings_new ("org.gtk.grisbi.form");
    grisbi_settings_init_settings_form (priv->settings_form, a_conf);

    priv->settings_general = g_settings_new ("org.gtk.grisbi.general");
    grisbi_settings_init_settings_general (priv->settings_general, a_conf);

    priv->settings_geometry = g_settings_new ("org.gtk.grisbi.geometry");
    grisbi_settings_init_settings_geometry (priv->settings_geometry, a_conf);

    priv->settings_messages_delete = g_settings_new ("org.gtk.grisbi.messages.delete");
    grisbi_settings_init_settings_messages_delete (priv->settings_messages_delete, a_conf);

    priv->settings_messages_tips = g_settings_new ("org.gtk.grisbi.messages.tips");
    grisbi_settings_init_settings_messages_tips (priv->settings_messages_tips, a_conf);

    priv->settings_messages_warnings = g_settings_new ("org.gtk.grisbi.messages.warnings");
    grisbi_settings_init_settings_messages_warnings (priv->settings_messages_warnings, a_conf);

    priv->settings_panel = g_settings_new ("org.gtk.grisbi.panel");
    grisbi_settings_init_settings_panel (priv->settings_panel, a_conf);

    priv->settings_prefs = g_settings_new ("org.gtk.grisbi.prefs");
    grisbi_settings_init_settings_prefs (priv->settings_prefs, a_conf);

    priv->settings_scheduled = g_settings_new ("org.gtk.grisbi.scheduled");
    grisbi_settings_init_settings_scheduled (priv->settings_scheduled, a_conf);
}


 /**
 *
 *
 * \param
 *
 * \return
 **/
static void dispose (GObject *object)
{
    GrisbiSettings *self = GRISBI_SETTINGS (object);
    GrisbiSettingsPrivate *priv;

    priv = grisbi_settings_get_instance_private (self);

    g_clear_object (&priv->settings_root);
    g_clear_object (&priv->settings_backup);
    g_clear_object (&priv->settings_display);
    g_clear_object (&priv->settings_file);
    g_clear_object (&priv->settings_form);
    g_clear_object (&priv->settings_general);
    g_clear_object (&priv->settings_geometry);
    g_clear_object (&priv->settings_messages_delete);
    g_clear_object (&priv->settings_messages_tips);
    g_clear_object (&priv->settings_messages_warnings);
    g_clear_object (&priv->settings_panel);
    g_clear_object (&priv->settings_prefs);
    g_clear_object (&priv->settings_scheduled);

    G_OBJECT_CLASS (grisbi_settings_parent_class)->dispose (object);
}


 /**
 *
 *
 * \param
 *
 * \return
 **/
static void finalize (GObject *object)
{
    singleton = NULL;

    /* Chain up to the parent class */
    G_OBJECT_CLASS (grisbi_settings_parent_class)->finalize (object);
}


 /**
 *
 *
 * \param
 *
 * \return
 **/
static void grisbi_settings_class_init (GrisbiSettingsClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        object_class->dispose = dispose;
        object_class->finalize = finalize;
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
GrisbiSettings *grisbi_settings_load_app_config (void)
{
    if (!singleton)
    {
        singleton = GRISBI_SETTINGS (g_object_new (GRISBI_TYPE_SETTINGS, NULL));
    }
    else
    {
        g_object_ref (singleton);
    }
    g_assert (singleton);

    return singleton;
}

 /**
 * FONCTION PROVISOIRE AVANT TRAITEMENT DES PREFERENCES
 *
 * \param
 *
 * \return
 **/
void grisbi_settings_save_app_config (void)
{
    GrisbiSettingsPrivate *priv;
	gchar **recent_array;
	GrisbiAppConf *a_conf;

    devel_debug (NULL);

	a_conf = grisbi_app_get_a_conf ();
    priv = grisbi_settings_get_instance_private (grisbi_settings_load_app_config ());

    /* priv->settings_root */
    g_settings_set_boolean (G_SETTINGS (priv->settings_root),
                        "first-use",
                        a_conf->first_use);

    /* priv->settings_backup */
    g_settings_set_boolean (G_SETTINGS (priv->settings_backup),
                        "make-backup-single-file",
                        a_conf->make_bakup_single_file);
    g_settings_set_boolean (G_SETTINGS (priv->settings_backup),
                        "compress-backup",
                        a_conf->compress_backup);
    g_settings_set_boolean (G_SETTINGS (priv->settings_backup),
                        "remove-backup-files",
                        a_conf->remove_backup_files);
    g_settings_set_int (G_SETTINGS (priv->settings_backup),
                        "remove-backup-months",
                        a_conf->remove_backup_months);
    g_settings_set_boolean (G_SETTINGS (priv->settings_backup),
                        "sauvegarde-ouverture",
                        a_conf->sauvegarde_demarrage);
    g_settings_set_boolean (G_SETTINGS (priv->settings_backup),
                        "sauvegarde-fermeture",
                        a_conf->sauvegarde_fermeture);
    g_settings_set_boolean (G_SETTINGS (priv->settings_backup),
                        "make-backup-every-minutes",
                        a_conf->make_backup_every_minutes);
    g_settings_set_int (G_SETTINGS (priv->settings_backup),
                        "make-backup-nb-minutes",
                        a_conf->make_backup_nb_minutes);

    /* priv->settings_display */
	if (a_conf->display_window_title)
		g_settings_set_int (G_SETTINGS (priv->settings_display),
							"display-window-title",
							a_conf->display_window_title);
	else
		g_settings_reset (G_SETTINGS (priv->settings_display),
						  "display-window-title");

    g_settings_set_boolean ( G_SETTINGS (priv->settings_display),
                        "formulaire-toujours-affiche",
                        a_conf->formulaire_toujours_affiche);
    g_settings_set_boolean (G_SETTINGS (priv->settings_display),
                        "show-closed-accounts",
                        a_conf->show_closed_accounts);

    /* priv->settings_file */
    g_settings_set_boolean ( G_SETTINGS (priv->settings_file),
                        "dernier-fichier-auto",
                        a_conf->dernier_fichier_auto);
    g_settings_set_boolean (G_SETTINGS (priv->settings_file),
                        "sauvegarde-auto",
                        a_conf->sauvegarde_auto);
    g_settings_set_boolean (G_SETTINGS (priv->settings_file),
                        "force-enregistrement",
                        a_conf->force_enregistrement);
    g_settings_set_boolean (G_SETTINGS (priv->settings_file),
                        "force-import-directory",
                        a_conf->force_import_directory);
	if (a_conf->force_import_directory)
	{
		g_settings_set_string (G_SETTINGS (priv->settings_file),
										   "import-directory",
										   a_conf->import_directory);
	}
    g_settings_set_boolean (G_SETTINGS (priv->settings_file),
                        "import-remove-file",
                        a_conf->import_remove_file);

    g_settings_set_boolean (G_SETTINGS (priv->settings_file),
                        "compress-file",
                        a_conf->compress_file);

	recent_array = grisbi_app_get_recent_files_array ();
    if (g_strv_length (recent_array))
    {
        g_settings_set_strv (G_SETTINGS (priv->settings_file),
                        "names-last-files",
                        (const gchar * const *) recent_array);
    }

    g_settings_set_int (G_SETTINGS (priv->settings_file),
                        "nb-max-derniers-fichiers-ouverts",
                        a_conf->nb_max_derniers_fichiers_ouverts);

    g_settings_set_boolean (priv->settings_file,
                        "archives-check-auto",
                        a_conf->archives_check_auto);
    g_settings_set_int (priv->settings_file,
                        "max-transactions-before-warn-archival",
                        a_conf->max_non_archived_transactions_for_check);

    /* priv->settings_form */
    g_settings_set_boolean ( G_SETTINGS (priv->settings_form),
                        "affichage-exercice-automatique",
                        a_conf->affichage_exercice_automatique);
    g_settings_set_boolean ( G_SETTINGS (priv->settings_form),
                            "form-enter-key", a_conf->form_enter_key);
	if (a_conf->form_validate_split)
		g_settings_set_boolean (G_SETTINGS (priv->settings_form),
								"form-validate-split",
								a_conf->form_validate_split);
	else
		g_settings_reset (G_SETTINGS (priv->settings_form),
						  "form-validate-split");

    /* priv->settings_general */
	if (a_conf->browser_command)
		g_settings_set_string (G_SETTINGS (priv->settings_general),
							   "browser-command",
							   a_conf->browser_command);

	if (a_conf->metatree_action_2button_press)
		g_settings_set_int (G_SETTINGS (priv->settings_general),
							"metatree-action-button-press",
							a_conf->metatree_action_2button_press);
	else
		g_settings_reset (G_SETTINGS (priv->settings_general),
						  "metatree-action-button-press");

    g_settings_set_boolean (G_SETTINGS (priv->settings_general),
                            "show-transaction-gives-balance",
                            a_conf->show_transaction_gives_balance);

    /* priv->settings_geometry */
    g_settings_set_int (G_SETTINGS (priv->settings_geometry),
                        "x-position",
                        a_conf->x_position);
    g_settings_set_int (G_SETTINGS (priv->settings_geometry),
                        "y-position",
                        a_conf->y_position);
    g_settings_set_int (G_SETTINGS (priv->settings_geometry),
                        "main-height",
                        a_conf->main_height);
    g_settings_set_int (G_SETTINGS (priv->settings_geometry),
                        "main-width",
                        a_conf->main_width);
    g_settings_set_boolean (G_SETTINGS (priv->settings_geometry),
                        "fullscreen",
                        a_conf->full_screen);
    g_settings_set_boolean (G_SETTINGS (priv->settings_geometry),
                        "maximized",
                        a_conf->maximize_screen);

    /* priv->settings_messages_tips */
    g_settings_set_int (G_SETTINGS (priv->settings_messages_tips),
                         "last-tip",
                         a_conf->last_tip);
    g_settings_set_boolean (G_SETTINGS (priv->settings_messages_tips),
                         "show-tip",
                         a_conf->show_tip);

    /* priv->settings_panel */
    g_settings_set_int (G_SETTINGS (priv->settings_panel),
                        "panel-width",
                        a_conf->panel_width);

    /* priv->settings_scheduled */
    g_settings_set_boolean (priv->settings_scheduled,
                        "execute-scheduled-of-month",
                        a_conf->execute_scheduled_of_month);
    g_settings_set_int (priv->settings_scheduled,
                        "nb-days-before-scheduled",
                        a_conf->nb_days_before_scheduled);
    g_settings_set_boolean (priv->settings_scheduled,
							"scheduler-set-fixed-day",
							a_conf->scheduler_set_fixed_day);
    g_settings_set_int (priv->settings_scheduled,
						"scheduler-fixed-day",
						a_conf->scheduler_fixed_day);
}

GSettings *grisbi_settings_get_settings (gint schema)
{
    GSettings *settings = NULL;
    GrisbiSettingsPrivate *priv = NULL;

    priv = grisbi_settings_get_instance_private (grisbi_settings_load_app_config());

    switch (schema)
    {
        case SETTINGS_ROOT:
            settings = priv->settings_root;
            break;
        case SETTINGS_FILES_BACKUP:
            settings = priv->settings_backup;
            break;
        case SETTINGS_DISPLAY:
            settings = priv->settings_display;
            break;
        case SETTINGS_FILES_FILE:
            settings = priv->settings_file;
            break;
        case SETTINGS_FORM:
            settings = priv->settings_form;
            break;
        case SETTINGS_GENERAL:
            settings = priv->settings_general;
            break;
        case SETTINGS_GEOMETRY:
            settings = priv->settings_geometry;
            break;
        case SETTINGS_MESSAGES_DELETE:
            settings = priv->settings_messages_delete;
            break;
        case SETTINGS_MESSAGES_TIPS:
            settings = priv->settings_messages_tips;
            break;
        case SETTINGS_MESSAGES_WARNINGS:
            settings = priv->settings_messages_warnings;
            break;
        case SETTINGS_PANEL:
            settings = priv->settings_panel;
            break;
        case SETTINGS_PREFS:
            settings = priv->settings_prefs;
            break;
        case SETTINGS_SCHEDULED:
            settings = priv->settings_scheduled;
            break;
    }

    return settings;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void grisbi_settings_set_current_theme (const gchar *current_theme,
										gint force_type_theme)
{
    GSettings *settings = NULL;
    GrisbiSettingsPrivate *priv = NULL;

    priv = grisbi_settings_get_instance_private (grisbi_settings_load_app_config());

	settings = priv->settings_general;
	g_settings_set_string (settings, "current-theme", current_theme);
	if (force_type_theme)
		g_settings_set_int (settings, "force-type-theme", force_type_theme);
	else
		g_settings_reset (G_SETTINGS (settings), "force-type-theme");
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
