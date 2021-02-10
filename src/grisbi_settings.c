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

#ifndef USE_CONFIG_FILE

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
#include "utils_files.h"
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

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
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

	if (a_conf->current_theme)
		g_free (a_conf->current_theme);
	a_conf->current_theme = g_settings_get_string (settings, "current-theme");

	a_conf->custom_fonte_listes = g_settings_get_boolean (settings, "custom-fonte-listes");
    if (a_conf->custom_fonte_listes)
    {
        a_conf->font_string = g_settings_get_string (settings, "font-string");
		if (a_conf->font_string && strlen (a_conf->font_string) == 0)
		{
			g_settings_reset (G_SETTINGS (settings), "font-string");
			a_conf->font_string = g_settings_get_string (settings, "font-string");
			a_conf->custom_fonte_listes = FALSE;
		}
    }
	else
		a_conf->font_string = g_settings_get_string (settings, "font-string");

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
    a_conf->low_definition_screen = g_settings_get_boolean (settings, "low-definition-screen");
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
    a_conf->prefs_archives_sort_order = g_settings_get_int (settings, "prefs-archives-sort-order");
    a_conf->prefs_fyear_sort_order = g_settings_get_int (settings, "prefs-fyear-sort-order");
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
	a_conf->last_selected_scheduler = g_settings_get_boolean (settings, "last-selected-scheduler");
    a_conf->nb_days_before_scheduled = g_settings_get_int (settings, "nb-days-before-scheduled");
    a_conf->scheduler_set_fixed_day = g_settings_get_boolean (settings, "scheduler-set-fixed-day");
    a_conf->scheduler_fixed_day = g_settings_get_int (settings, "scheduler-fixed-day");
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void grisbi_settings_save_settings_root (GSettings *settings,
												GrisbiAppConf *a_conf)
{
	if (a_conf->first_use)
		g_settings_reset (G_SETTINGS (settings), "first-use");
	else
		g_settings_set_boolean (G_SETTINGS (settings), "first-use", FALSE);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void grisbi_settings_save_settings_backup (GSettings *settings,
												  GrisbiAppConf *a_conf)
{
	const gchar* backup_path = NULL;

	backup_path = gsb_file_get_backup_path ();
	if (backup_path && strlen (backup_path) > 0)
		g_settings_set_string (G_SETTINGS (settings), "backup-path", backup_path);

	if (a_conf->compress_backup)
		g_settings_set_boolean (G_SETTINGS (settings), "compress-backup",  TRUE);
	else
		g_settings_reset (G_SETTINGS (settings), "compress-backup");

	if (a_conf->make_backup_every_minutes)
	{
		g_settings_set_boolean (G_SETTINGS (settings), "make-backup-every-minutes", TRUE);
		if (a_conf->make_backup_nb_minutes == 5)
			g_settings_reset (G_SETTINGS (settings), "make-backup-nb-minutes");
		else
			g_settings_set_int (G_SETTINGS (settings),
								"make-backup-nb-minutes",
								a_conf->make_backup_nb_minutes);
	}
	else
		g_settings_reset (G_SETTINGS (settings), "make-backup-every-minutes");


	if (a_conf->make_bakup_single_file)
		g_settings_set_boolean (G_SETTINGS (settings), "make-backup-single-file", TRUE);
	else
		g_settings_reset (G_SETTINGS (settings), "make-backup-single-file");

	if (a_conf->remove_backup_files)
	{
		g_settings_set_boolean (G_SETTINGS (settings), "remove-backup-files", TRUE);
		if (a_conf->remove_backup_months != 12)
			g_settings_set_int (G_SETTINGS (settings),
								"remove-backup-months",
								a_conf->remove_backup_months);
		else
			g_settings_reset (G_SETTINGS (settings), "remove-backup-months");
	}
	else
	{
		g_settings_reset (G_SETTINGS (settings), "remove-backup-files");
		g_settings_reset (G_SETTINGS (settings), "remove-backup-months");
	}

	if (a_conf->sauvegarde_demarrage)
		g_settings_reset (G_SETTINGS (settings), "sauvegarde-ouverture");
	else
		g_settings_set_boolean (G_SETTINGS (settings), "sauvegarde-ouverture", FALSE);

	if (a_conf->sauvegarde_fermeture)
		g_settings_reset (G_SETTINGS (settings), "sauvegarde-fermeture");
	else
		g_settings_set_boolean (G_SETTINGS (settings), "sauvegarde-fermeture", FALSE);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void grisbi_settings_save_settings_display (GSettings *settings,
												   GrisbiAppConf *a_conf)
{
	if (a_conf->display_window_title)
		g_settings_set_int (G_SETTINGS (settings),
							"display-window-title",
							a_conf->display_window_title);
	else
		g_settings_reset (G_SETTINGS (settings), "display-window-title");

	if (a_conf->display_toolbar == 2)
		g_settings_reset (G_SETTINGS (settings), "display-toolbar");
	else
    	g_settings_set_int (G_SETTINGS (settings), "display-toolbar", a_conf->display_toolbar);

	if (a_conf->formulaire_toujours_affiche)
		g_settings_reset (G_SETTINGS (settings), "formulaire-toujours-affiche");
	else
		g_settings_set_boolean (G_SETTINGS (settings), "formulaire-toujours-affiche", FALSE);

	if (a_conf->group_partial_balance_under_accounts)
		g_settings_reset (G_SETTINGS (settings), "group-partial-balance-under-accounts");
	else
		g_settings_set_boolean (G_SETTINGS (settings), "group-partial-balance-under-accounts", FALSE);

	if (a_conf->show_closed_accounts)
		g_settings_set_boolean (G_SETTINGS (settings), "show-closed-accounts", TRUE);
	else
		g_settings_reset (G_SETTINGS (settings), "show-closed-accounts");

	if (a_conf->show_headings_bar)
		g_settings_reset (G_SETTINGS (settings), "show-headings-bar");
	else
		g_settings_set_boolean (G_SETTINGS (settings), "show-headings-bar", FALSE);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void grisbi_settings_save_settings_file (GSettings *settings,
												GrisbiAppConf *a_conf)
{
	gchar **recent_array;

	if (a_conf->archives_check_auto)
	{
		g_settings_reset (G_SETTINGS (settings), "archives-check-auto");
		if (a_conf->max_non_archived_transactions_for_check == 3000)
			g_settings_reset (G_SETTINGS (settings), "max-transactions-before-warn-archival");
		else
			g_settings_set_int (settings,
								"max-transactions-before-warn-archival",
								a_conf->max_non_archived_transactions_for_check);
	}
	else
	{
		g_settings_set_boolean (settings, "archives-check-auto", FALSE);
		g_settings_reset (G_SETTINGS (settings), "max-transactions-before-warn-archival");
	}

	if (a_conf->compress_file)
		g_settings_set_boolean (G_SETTINGS (settings), "compress-file", TRUE);
	else
		g_settings_reset (G_SETTINGS (settings), "compress-file");

	if (a_conf->dernier_fichier_auto)
		g_settings_set_boolean ( G_SETTINGS (settings), "dernier-fichier-auto", TRUE);
	else
		g_settings_reset (G_SETTINGS (settings), "dernier-fichier-auto");

	if (a_conf->force_enregistrement)
		g_settings_set_boolean (G_SETTINGS (settings), "force-enregistrement", TRUE);
	else
		g_settings_reset (G_SETTINGS (settings), "force-enregistrement");

	if (a_conf->force_import_directory)
	{
		g_settings_set_boolean (G_SETTINGS (settings), "force-import-directory", TRUE);
		g_settings_set_string (G_SETTINGS (settings),
							   "import-directory",
							   a_conf->import_directory);
	}
	else
	{
		g_settings_reset (G_SETTINGS (settings), "force-import-directory");
		g_settings_reset (G_SETTINGS (settings), "import-directory");
	}

	if (a_conf->import_remove_file)
		g_settings_set_boolean (G_SETTINGS (settings), "import-remove-file", TRUE);
	else
		g_settings_reset (G_SETTINGS (settings), "import-remove-file");

	recent_array = grisbi_app_get_recent_files_array ();
    if (g_strv_length (recent_array))
    {
	   g_settings_set_strv (G_SETTINGS (settings),
							"names-last-files",
							(const gchar * const *) recent_array);
    }

	if (a_conf->nb_max_derniers_fichiers_ouverts == 3)
		g_settings_reset (G_SETTINGS (settings), "nb-max-derniers-fichiers-ouverts");
	else
		g_settings_set_int (G_SETTINGS (settings),
							"nb-max-derniers-fichiers-ouverts",
							a_conf->nb_max_derniers_fichiers_ouverts);

	if (a_conf->sauvegarde_auto)
		g_settings_set_boolean (G_SETTINGS (settings), "sauvegarde-auto", TRUE);
	else
		g_settings_reset (G_SETTINGS (settings), "sauvegarde-auto");
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void grisbi_settings_save_settings_form (GSettings *settings,
												GrisbiAppConf *a_conf)
{
	if (a_conf->affichage_exercice_automatique)
		g_settings_set_boolean (G_SETTINGS (settings), "affichage-exercice-automatique", TRUE);
	else
		g_settings_reset (G_SETTINGS (settings), "affichage-exercice-automatique");

	if (a_conf->completion_minimum_key_length > 1)
		g_settings_set_int (G_SETTINGS (settings),
							"completion-minimum-key-length",
							a_conf->completion_minimum_key_length);
	else
		g_settings_reset (G_SETTINGS (settings), "completion-minimum-key-length");

	if (a_conf->automatic_completion_payee)
		g_settings_set_boolean (G_SETTINGS (settings), "automatic-completion-payee", TRUE);
	else
		g_settings_reset (G_SETTINGS (settings), "automatic-completion-payee");

	if (a_conf->automatic_erase_credit_debit)
		g_settings_set_boolean (G_SETTINGS (settings), "automatic-erase-credit-debit", TRUE);
	else
		g_settings_reset (G_SETTINGS (settings), "automatic-erase-credit-debit");

	if (a_conf->automatic_recover_splits)
		g_settings_set_boolean (G_SETTINGS (settings), "automatic-recover-splits", TRUE);
	else
		g_settings_reset (G_SETTINGS (settings), "automatic-recover-splits");

	if (a_conf->form_enter_key)
		g_settings_set_boolean (G_SETTINGS (settings), "form-enter-key", TRUE);
	else
		g_settings_reset (G_SETTINGS (settings), "form-enter-key");

	if (a_conf->form_validate_split)
		g_settings_set_boolean (G_SETTINGS (settings), "form-validate-split", TRUE);
	else
		g_settings_reset (G_SETTINGS (settings), "form-validate-split");

	if (a_conf->fyear_combobox_sort_order)
		g_settings_set_int (G_SETTINGS (settings), "fyear-combobox-sort-order", TRUE);
	else
		g_settings_reset (G_SETTINGS (settings), "fyear-combobox-sort-order");

	if (a_conf->limit_completion_to_current_account)
		g_settings_set_boolean (G_SETTINGS (settings), "limit-completion-current-account", TRUE);
	else
		g_settings_reset (G_SETTINGS (settings), "limit-completion-current-account");
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void grisbi_settings_save_settings_general (GSettings *settings,
												   GrisbiAppConf *a_conf)
{
	const gchar *tmp_str;

	if (a_conf->browser_command)
		g_settings_set_string (G_SETTINGS (settings), "browser-command", a_conf->browser_command);

	if (a_conf->current_theme)
		g_settings_set_string (settings, "current-theme", a_conf->current_theme);

	if (a_conf->custom_fonte_listes)
		g_settings_set_boolean (G_SETTINGS (settings), "custom-fonte-listes", TRUE);
	else
		g_settings_reset (G_SETTINGS (settings), "custom-fonte-listes");

	if (strcmp (a_conf->font_string, "Monospace 10") == 0)
		g_settings_reset (G_SETTINGS (settings), "font-string");
	else
		g_settings_set_string (G_SETTINGS (settings), "font-string", a_conf->font_string);

	if (a_conf->force_type_theme)
		g_settings_set_int (settings, "force-type-theme", a_conf->force_type_theme);
	else
		g_settings_reset (G_SETTINGS (settings), "force-type-theme");

	if (a_conf->language_chosen)
		g_settings_set_string (G_SETTINGS (settings), "language-chosen", a_conf->language_chosen);
	else
		g_settings_reset (G_SETTINGS (settings), "language-chosen");

	tmp_str = gsb_file_get_last_path ();
    if (!tmp_str || strlen (tmp_str) == 0)
		tmp_str = g_get_user_special_dir (G_USER_DIRECTORY_DOCUMENTS);
	g_settings_set_string (G_SETTINGS (settings), "last-path", tmp_str);

	if (a_conf->metatree_action_2button_press == 0)
		g_settings_reset (G_SETTINGS (settings), "metatree-action-button-press");
	else
		g_settings_set_int (G_SETTINGS (settings),
							"metatree-action-button-press",
							a_conf->metatree_action_2button_press);

	if (a_conf->pluriel_final)
		g_settings_set_boolean (G_SETTINGS (settings), "pluriel-final",  TRUE);
	else
		g_settings_reset (G_SETTINGS (settings), "pluriel-final");

	if (a_conf->show_transaction_gives_balance)
		g_settings_set_boolean (G_SETTINGS (settings), "show-transaction-gives-balance", TRUE);
	else
		g_settings_reset (G_SETTINGS (settings), "show-transaction-gives-balance");

	if (a_conf->show_transaction_selected_in_form)
		g_settings_reset (G_SETTINGS (settings), "show-transaction-selected-in-form");
	else
		g_settings_set_boolean (G_SETTINGS (settings), "show-transaction-selected-in-form", TRUE);

	if (a_conf->transactions_list_primary_sorting)
		g_settings_set_int (G_SETTINGS (settings),
							"transactions-list-primary-sorting",
							a_conf->transactions_list_primary_sorting);
	else
		g_settings_reset (G_SETTINGS (settings), "transactions-list-primary-sorting");

	if (a_conf->transactions_list_secondary_sorting)
		g_settings_set_int (G_SETTINGS (settings),
							"transactions-list-secondary-sorting",
							a_conf->transactions_list_secondary_sorting);
	else
		g_settings_reset (G_SETTINGS (settings), "transactions-list-secondary-sorting");
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void grisbi_settings_save_settings_geometry (GSettings *settings,
													GrisbiAppConf *a_conf)
{
	if (a_conf->full_screen)
		g_settings_set_boolean (G_SETTINGS (settings), "fullscreen", TRUE);
	else
		g_settings_reset (G_SETTINGS (settings), "fullscreen");

	if (a_conf->low_definition_screen)
		g_settings_set_boolean (G_SETTINGS (settings), "low-definition-screen", TRUE);
	else
		g_settings_reset (G_SETTINGS (settings), "low-definition-screen");

	if (a_conf->main_height == WIN_MIN_HEIGHT)
		g_settings_reset (G_SETTINGS (settings), "main-height");
	else
		g_settings_set_int (G_SETTINGS (settings), "main-height", a_conf->main_height);

	if (a_conf->main_width == WIN_MIN_WIDTH)
		g_settings_reset (G_SETTINGS (settings), "main-width");
	else
		g_settings_set_int (G_SETTINGS (settings), "main-width", a_conf->main_width);

	if (a_conf->maximize_screen)
		g_settings_set_boolean (G_SETTINGS (settings), "maximized", TRUE);
	else
		g_settings_reset (G_SETTINGS (settings), "maximized");

	if (a_conf->x_position == WIN_POS_X)
		g_settings_reset (G_SETTINGS (settings), "x-position");
	else
		g_settings_set_int (G_SETTINGS (settings), "x-position", a_conf->x_position);

	if (a_conf->y_position == WIN_POS_Y)
		g_settings_reset (G_SETTINGS (settings), "y-position");
	else
		g_settings_set_int (G_SETTINGS (settings), "y-position", a_conf->y_position);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void grisbi_settings_save_settings_messages_delete (GSettings *settings,
														   GrisbiAppConf *a_conf)
{
	ConditionalMsg *warning;
	gint i = 0;

	warning = (ConditionalMsg*) dialogue_get_tab_delete_msg ();
    for (i = 0; (warning+i)->name; i ++)
    {
		if ((warning+i)->hidden)
			g_settings_set_boolean (G_SETTINGS (settings), (warning+i)->name, FALSE);
		else
			g_settings_reset (G_SETTINGS (settings), (warning+i)->name);
    }
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void grisbi_settings_save_settings_messages_tips (GSettings *settings,
														 GrisbiAppConf *a_conf)
{
	if (a_conf->show_tip)
	{
		g_settings_reset (G_SETTINGS (settings), "show-tip");
		if (a_conf->last_tip > 0)
			g_settings_set_int (G_SETTINGS (settings), "last-tip", a_conf->last_tip);
		else
			g_settings_reset (G_SETTINGS (settings), "last-tip");
	}
	else
	{
		g_settings_set_boolean (G_SETTINGS (settings), "show-tip", FALSE);
		g_settings_reset (G_SETTINGS (settings), "last-tip");
	}
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void grisbi_settings_save_settings_messages_warnings (GSettings *settings,
															 GrisbiAppConf *a_conf)
{
	ConditionalMsg *warning;
	gint i = 0;

	warning = (ConditionalMsg*) dialogue_get_tab_warning_msg ();
    for (i = 0; (warning+i)->name; i ++)
    {
		if ((warning+i)->hidden)
			g_settings_set_boolean (G_SETTINGS (settings), (warning+i)->name, FALSE);
		else
			g_settings_reset (G_SETTINGS (settings), (warning+i)->name);
	}
}

/**
 *
 * \param
 * \param
 *
 * \return
 **/
static void grisbi_settings_save_settings_panel (GSettings *settings,
												 GrisbiAppConf *a_conf)
{
	if (a_conf->active_scrolling_left_pane)
		g_settings_set_boolean (G_SETTINGS (settings), "active-scrolling-left-pane", TRUE);
	else
		g_settings_reset (G_SETTINGS (settings), "active-scrolling-left-pane");

	if (a_conf->panel_width == PANEL_MIN_WIDTH)
		g_settings_reset (G_SETTINGS (settings), "panel-width");
	else
		g_settings_set_int (G_SETTINGS (settings), "panel-width", a_conf->panel_width);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void grisbi_settings_save_settings_prefs (GSettings *settings,
												 GrisbiAppConf *a_conf)
{
	if (a_conf->prefs_archives_sort_order == GTK_SORT_DESCENDING)
		g_settings_set_int (G_SETTINGS (settings), "prefs-archives-sort-order", TRUE);
	else
		g_settings_reset (G_SETTINGS (settings), "prefs-archives-sort-order");

	if (a_conf->prefs_fyear_sort_order == GTK_SORT_DESCENDING)
		g_settings_set_int (G_SETTINGS (settings), "prefs-fyear-sort-order", TRUE);
	else
		g_settings_reset (G_SETTINGS (settings), "prefs-fyear-sort-order");

	if (a_conf->prefs_height == PREFS_WIN_MIN_HEIGHT)
		g_settings_reset (G_SETTINGS (settings), "prefs-height");
	else
		g_settings_set_int (G_SETTINGS (settings), "prefs-height", a_conf->prefs_height);

	if (a_conf->prefs_panel_width  == PREFS_PANED_MIN_WIDTH)
		g_settings_reset (G_SETTINGS (settings), "prefs-panel-width");
	else
		g_settings_set_int (G_SETTINGS (settings), "prefs-panel-width", a_conf->prefs_panel_width);

	if (a_conf->prefs_width == PREFS_WIN_MIN_WIDTH)
		g_settings_reset (G_SETTINGS (settings), "prefs-width");
	else
		g_settings_set_int (G_SETTINGS (settings), "prefs-width", a_conf->prefs_width);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void grisbi_settings_save_settings_scheduled (GSettings *settings,
													 GrisbiAppConf *a_conf)
{
	if (a_conf->balances_with_scheduled)
		g_settings_set_boolean (G_SETTINGS (settings), "balances-with-scheduled", TRUE);

	if (a_conf->execute_scheduled_of_month)
		g_settings_set_boolean (settings, "execute-scheduled-of-month", TRUE);
	else
		g_settings_reset (G_SETTINGS (settings), "execute-scheduled-of-month");

	if (a_conf->last_selected_scheduler)
		g_settings_reset (G_SETTINGS (settings), "last-selected-scheduler");
	else
		g_settings_set_boolean (settings, "last-selected-scheduler", FALSE);

	if (a_conf->nb_days_before_scheduled)
		g_settings_set_int (settings, "nb-days-before-scheduled", TRUE);
	else
		g_settings_reset (G_SETTINGS (settings), "nb-days-before-scheduled");

	if (a_conf->scheduler_set_fixed_day)
		g_settings_set_boolean (settings, "scheduler-set-fixed-day", TRUE);
	else
		g_settings_reset (G_SETTINGS (settings), "scheduler-set-fixed-day");

	if (a_conf->scheduler_fixed_day == 1)
		g_settings_reset (G_SETTINGS (settings), "scheduler-fixed-day");
	else
		g_settings_set_int (settings,
							"scheduler-fixed-day",
							a_conf->scheduler_fixed_day);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation de l'objet                            */
/******************************************************************************/
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

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
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
 *
 *
 * \param
 *
 * \return
 **/
void grisbi_settings_save_app_config (void)
{
    GrisbiSettingsPrivate *priv;
	GrisbiAppConf *a_conf;

	devel_debug (NULL);

	a_conf = grisbi_app_get_a_conf ();
	priv = grisbi_settings_get_instance_private (grisbi_settings_load_app_config ());

	grisbi_settings_save_settings_root (priv->settings_root, a_conf);
	grisbi_settings_save_settings_backup (priv->settings_backup, a_conf);
	grisbi_settings_save_settings_display (priv->settings_display, a_conf);
	grisbi_settings_save_settings_file (priv->settings_file, a_conf);
	grisbi_settings_save_settings_form (priv->settings_form, a_conf);
	grisbi_settings_save_settings_general (priv->settings_general, a_conf);
	grisbi_settings_save_settings_geometry (priv->settings_geometry, a_conf);
	grisbi_settings_save_settings_messages_delete (priv->settings_messages_delete, a_conf);
	grisbi_settings_save_settings_messages_tips (priv->settings_messages_tips, a_conf);
	grisbi_settings_save_settings_messages_warnings (priv->settings_messages_warnings, a_conf);
	grisbi_settings_save_settings_panel (priv->settings_panel, a_conf);
	grisbi_settings_save_settings_prefs (priv->settings_prefs, a_conf);
	grisbi_settings_save_settings_scheduled (priv->settings_scheduled, a_conf);
}


/**
 * save the config in file PROVISOIRE
 * it uses the glib config utils after 0.6.0
 * if cannot load, try the xml file before that version
 *
 * \param
 *
 * \return TRUE if ok
 **/
void grisbi_settings_save_in_config_file (void)
{
	GKeyFile *config;
	const gchar* backup_path = NULL;
	const gchar *filename;
	gchar **recent_array;
	const gchar *tmp_str;
	gint i;
	GError *error = NULL;
	ConditionalMsg *warning;
	GrisbiAppConf *a_conf;

    devel_debug (NULL);
	a_conf = grisbi_app_get_a_conf ();

	filename = gsb_dirs_get_grisbirc_filename ();
	config = g_key_file_new ();

	/* settings_root */
	g_key_file_set_boolean (config,
							"Root",
							"first-use",
							a_conf->first_use);
	/* settings_backup */
	backup_path = gsb_file_get_backup_path ();
	if (backup_path && strlen (backup_path) > 0)
        g_key_file_set_string (config,
							   "Backup",
                        	   "backup-path",
                        	   backup_path);

	g_key_file_set_boolean (config,
							"Backup",
							"compress-backup",
							a_conf->compress_backup);

	g_key_file_set_boolean (config,
							"Backup",
							"make-backup-every-minutes",
							a_conf->make_backup_every_minutes);
	g_key_file_set_integer (config,
							"Backup",
							"make-backup-nb-minutes",
							a_conf->make_backup_nb_minutes);
	g_key_file_set_boolean (config,
							"Backup",
							"make-backup-single-file",
							a_conf->make_bakup_single_file);
	g_key_file_set_boolean (config,
						    "Backup",
						    "remove-backup-files",
						    a_conf->remove_backup_files);
    g_key_file_set_integer (config,
							"Backup",
							"remove-backup-months",
							a_conf->remove_backup_months);
	g_key_file_set_boolean (config,
							"Backup",
							"sauvegarde-fermeture",
							a_conf->sauvegarde_fermeture);
	g_key_file_set_boolean (config,
							"Backup",
							"sauvegarde-ouverture",
							a_conf->sauvegarde_demarrage);

    /* settings_display */
    switch (a_conf->display_toolbar)
    {
        case GTK_TOOLBAR_TEXT:
            tmp_str = "Text";
            break;
        case GTK_TOOLBAR_ICONS:
            tmp_str = "Icons";
            break;
        default:
            tmp_str = (gchar*)"Text + Icons";
    }
    g_key_file_set_string (config,
						   "Display",
						   "display-toolbar",
						   tmp_str);
    switch (a_conf->display_window_title)
    {
        case 1:
            tmp_str = "Holder name";
            break;
        case 2:
            tmp_str = "Filename";
            break;
        default:
            tmp_str = "Entity name";
    }
    g_key_file_set_string (config,
						   "Display",
						   "display-window-title",
						   tmp_str);
	g_key_file_set_boolean (config,
						   "Display",
						   "formulaire-toujours-affiche",
						   a_conf->formulaire_toujours_affiche);
	g_key_file_set_boolean (config,
						    "Display",
							"group-partial-balance-under-accounts",
							a_conf->group_partial_balance_under_accounts);
	g_key_file_set_boolean (config,
						   "Display",
						   "show-closed-accounts",
						   a_conf->show_closed_accounts);
	g_key_file_set_boolean (config,
						   "Display",
							"show-headings-bar",
							a_conf->show_headings_bar);

    /* settings_file */
	g_key_file_set_boolean (config,
							"File",
							"archives-check-auto",
							a_conf->archives_check_auto);
	g_key_file_set_boolean (config,
							"File",
							"compress-file",
							a_conf->compress_file);
	g_key_file_set_boolean (config,
						   "File",
						   "dernier-fichier-auto",
						   a_conf->dernier_fichier_auto);
	g_key_file_set_boolean (config,
						   "File",
						   "force-enregistrement",
						   a_conf->force_enregistrement);
    g_key_file_set_boolean (config,
						   "File",
						   "force-import-directory",
						   a_conf->force_import_directory);
	if (a_conf->force_import_directory)
	{
	    g_key_file_set_string (config,
							   "File",
							   "import-directory",
								a_conf->import_directory);
	}
	g_key_file_set_boolean (config,
							"File",
							"import-remove-file",
							a_conf->import_remove_file);
	g_key_file_set_integer (config,
							"File",
							"max-transactions-before-warn-archival",
							a_conf->max_non_archived_transactions_for_check);
	recent_array = grisbi_app_get_recent_files_array ();
	if (recent_array && g_strv_length (recent_array))
	{
		 g_key_file_set_string_list (config,
									 "File",
									 "names-last-files",
									 (const gchar **) recent_array,
                        			 a_conf->nb_derniers_fichiers_ouverts);
	}
	g_key_file_set_integer (config,
							"File",
							"nb-max-derniers-fichiers-ouverts",
							a_conf->nb_max_derniers_fichiers_ouverts);
	g_key_file_set_boolean (config,
						   "File",
						   "sauvegarde-auto",
                           a_conf->sauvegarde_auto);

    /* settings_form */
    g_key_file_set_boolean (config,
							"Form",
                        	"affichage-exercice-automatique",
                        	a_conf->affichage_exercice_automatique);
    g_key_file_set_boolean (config,
							"Form",
							"automatic-completion-payee",
							a_conf->automatic_completion_payee);
	g_key_file_set_boolean (config,
							"Form",
							"automatic-erase-credit-debit",
							a_conf->automatic_erase_credit_debit);
	g_key_file_set_boolean (config,
							"Form",
							"automatic-recover-splits",
							a_conf->automatic_recover_splits);

    g_key_file_set_integer (config,
							"Form",
							"completion-minimum-key-length",
							a_conf->completion_minimum_key_length);
	g_key_file_set_boolean (config,
							"Form",
							"form-enter-key", a_conf->form_enter_key);
	g_key_file_set_boolean (config,
							"Form",
							"form-validate-split",
							a_conf->form_validate_split);
    g_key_file_set_integer (config,
							"Form",
							"fyear-combobox-sort-order",
							a_conf->fyear_combobox_sort_order);
	g_key_file_set_boolean (config,
							"Form",
							"limit-completion-current-account",
    						a_conf->limit_completion_to_current_account);

    /* settings_general */
    if (a_conf->browser_command)
        g_key_file_set_string (config,
							   "General",
							   "browser-command",
							   a_conf->browser_command);
    g_key_file_set_string (config,
						   "General",
						   "current-theme",
						   a_conf->current_theme);

    g_key_file_set_boolean (config,
							"General",
                        	"custom-fonte-listes",
                        	a_conf->custom_fonte_listes );
    if (a_conf->font_string)
        g_key_file_set_string (config,
							   "General",
                        	   "font-string",
                        	   a_conf->font_string);
    g_key_file_set_integer (config,
						   	"General",
							"force-type-theme",
							a_conf->force_type_theme);
	if (a_conf->language_chosen)
		g_key_file_set_string (config,
							   "General",
							   "language-chosen",
							   a_conf->language_chosen);
	g_key_file_set_string (config,
						   "General",
						   "last-path",
						   gsb_file_get_last_path());

	switch (a_conf->metatree_action_2button_press)
    {
        case 1:
            tmp_str = "Edit Category";
            break;
        case 2:
            tmp_str = "Manage division";
            break;
        default:
            tmp_str = "gtk default";
    }
	g_key_file_set_string (config,
						   "General",
                           "metatree-action-button-press",
                           tmp_str);
    g_key_file_set_boolean (config,
						   	"General",
							"pluriel-final",
							a_conf->pluriel_final);
    g_key_file_set_boolean (config,
						    "General",
                            "show-transaction-gives-balance",
                            a_conf->show_transaction_gives_balance);
	g_key_file_set_boolean (config,
							 "General",
							 "show-transaction-selected-in-form",
							 a_conf->show_transaction_selected_in_form);

	switch (a_conf->transactions_list_primary_sorting)
    {
        case 0:
            tmp_str = "Sort by value date";
            break;
        case 2:
            tmp_str = "Forced sort by date";
            break;
        default:
            tmp_str = "default";
	}
    g_key_file_set_string (config,
						   "General",
						   "transactions-list-primary-sorting",
						   tmp_str);

	switch (a_conf->transactions_list_secondary_sorting)
    {
        case 0:
            tmp_str = "Sort by transaction number";
            break;
        case 1:
            tmp_str = "Sort by type of amount";
            break;
        case 2:
            tmp_str = "Sort by payee name";
            break;
        default:
            tmp_str = "default";
	}
    g_key_file_set_string (config,
						   "General",
						   "transactions-list-secondary-sorting",
						   tmp_str);

    /* settings_geometry */
    g_key_file_set_boolean (config,
							"Geometry",
							"low-definition-screen",
							a_conf->low_definition_screen);
	g_key_file_set_boolean (config,
							"Geometry",
							"fullscreen",
                        	a_conf->full_screen);
    g_key_file_set_integer (config,
							"Geometry",
                        	"main-height",
                        	a_conf->main_height);
    g_key_file_set_integer (config,
							"Geometry",
                        	"main-width",
                        	a_conf->main_width);
    g_key_file_set_boolean (config,
							"Geometry",
                        	"maximized",
                        	a_conf->maximize_screen);
    g_key_file_set_integer (config,
							"Geometry",
                        	"x-position",
                        	a_conf->x_position);
    g_key_file_set_integer (config,
							"Geometry",
                        	"y-position",
                        	a_conf->y_position);

    /* settings_messages_delete */
	warning = (ConditionalMsg*) dialogue_get_tab_delete_msg ();
    for (i = 0; (warning+i)->name; i ++)
    {
        g_key_file_set_boolean (config,
								"Delete_msg",
                        		(warning+i)->name,
                        		!(warning+i)->hidden);
    }

    /* settings_messages_tips */
    g_key_file_set_integer (config,
							"Tips",
                        	"last-tip",
                        	a_conf->last_tip);
    g_key_file_set_boolean (config,
							"Tips",
                        	"show-tip",
                        	a_conf->show_tip);

    /* settings_messages_warnings */
	warning = (ConditionalMsg*) dialogue_get_tab_warning_msg ();
    for (i = 0; (warning+i)->name; i ++)
    {
        g_key_file_set_boolean (config,
								"Warnings_msg",
                        		(warning+i)->name,
                        		!(warning+i)->hidden);
    }

    /* settings_panel */
    g_key_file_set_boolean (config,
							"Panel",
							"active-scrolling-left-pane",
							a_conf->active_scrolling_left_pane);
    g_key_file_set_integer (config,
							"Panel",
                        	"panel-width",
                        	a_conf->panel_width);

	/* settings_prefs */
    g_key_file_set_int (config,
						"Prefs",
						"prefs-archives-sort-order",
						a_conf->prefs_archives_sort_order);
   g_key_file_set_int (config,
					   "Prefs",
					   "prefs-fyear-sort-order",
					   a_conf->prefs_fyear_sort_order);
    g_key_file_set_integer (config,
							"Prefs",
                        	"prefs-height",
                        	a_conf->prefs_height);

    g_key_file_set_integer (config,
							"Prefs",
                        	"prefs-panel-width",
                        	a_conf->prefs_panel_width);

    g_key_file_set_integer (config,
							"Prefs",
                        	"prefs-width",
                        	a_conf->prefs_width);

	/* settings_scheduled */
    g_key_file_set_boolean (config,
							"Scheduled",
							"balances-with-scheduled",
							a_conf->balances_with_scheduled);
    g_key_file_set_boolean (config,
							"Scheduled",
                        	"execute-scheduled-of-month",
                        	a_conf->execute_scheduled_of_month);
    g_key_file_set_boolean (config,
							"Scheduled",
							"last-selected-scheduler",
							a_conf->last_selected_scheduler);
    g_key_file_set_integer  (config,
							"Scheduled",
                        	"nb-days-before-scheduled",
                        	a_conf->nb_days_before_scheduled);
    g_key_file_set_integer (config,
							"Scheduled",
							"scheduler-fixed-day",
							a_conf->scheduler_fixed_day);
    g_key_file_set_boolean (config,
							"Scheduled",
							"scheduler-set-fixed-day",
							a_conf->scheduler_set_fixed_day);

    /* save into a file */
	/* verifier l'utilité de  ce test */
	if (!g_file_test (filename, G_FILE_TEST_EXISTS))
	{
		utils_files_create_XDG_dir ();
	}

	if (!g_key_file_save_to_file (config, filename, &error))
	{
		gchar* msg;

        msg = g_strdup_printf (_("Cannot save configuration file '%s': %s"),
								  filename,
								  g_strerror (errno));
        dialogue_error (tmp_str);
        g_free (msg);
        g_key_file_free (config);

		return;
    }

    g_key_file_free (config);
}

#endif /* NOT USE_CONFIG_FILE */
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
