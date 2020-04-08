/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*                      2005-2008 Benjamin Drieu (bdrieu@april.org)           */
/*          2008-2020 Pierre Biava (grisbi@pierre.biava.name)                 */
/*          https://www.grisbi.org                                            */
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


/**
 * \file gsb_file_config.c
 * save and load the config file
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"
#include <errno.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "grisbi_conf.h"
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

/*START_STATIC*/
/*END_STATIC*/


/*START_EXTERN*/
/*END_EXTERN*/

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * Set all the config variables to their default values.
 * called before loading the config
 * or for a new opening
 *
 * \param
 *
 * \return
 **/
static void grisbi_conf_clean_config (void)
{
	gchar * tmp_str;
	gint i = 0;
	ConditionalMsg *warning;

    devel_debug (NULL);

	/* set settings root */
	conf.first_use = TRUE;

	/* set settings_backup */
    conf.compress_backup = FALSE;
    conf.make_backup_every_minutes = FALSE;
    conf.make_backup_nb_minutes = 5;
	conf.make_bakup_single_file = FALSE;
	conf.remove_backup_files = FALSE;
	conf.remove_backup_months = 12;
    conf.sauvegarde_demarrage = FALSE;
	conf.sauvegarde_fermeture = TRUE;

	/* priv->settings_display */
    conf.display_toolbar = GSB_BUTTON_BOTH;				/* "Text + Icons" */
	conf.display_window_title = 0;						/* "Entity name" */
	conf.formulaire_toujours_affiche = TRUE;
	conf.group_partial_balance_under_accounts = TRUE;
	conf.show_closed_accounts = FALSE;
	conf.show_headings_bar = TRUE;

    /* settings_file */
	conf.archives_check_auto = TRUE;
	conf.compress_file = FALSE;
	conf.dernier_fichier_auto = FALSE;
	conf.force_enregistrement = FALSE;
    conf.force_import_directory = FALSE;
	conf.import_remove_file = FALSE;
	conf.max_non_archived_transactions_for_check = 3000;
	grisbi_app_set_recent_files_array (NULL);
	conf.nb_max_derniers_fichiers_ouverts = 3;
	conf.sauvegarde_auto = FALSE;

    /* settings_form */
    conf.affichage_exercice_automatique = FALSE;
    conf.automatic_completion_payee = FALSE;
    conf.automatic_erase_credit_debit = FALSE;
    conf.automatic_recover_splits = FALSE;
	conf.completion_minimum_key_length = 1;
	conf.form_enter_key = FALSE;
	conf.fyear_combobox_sort_order = 0;
    conf.limit_completion_to_current_account = FALSE;

    /* settings_general */
    conf.custom_fonte_listes = FALSE;
    conf.force_type_theme = 0;
	conf.metatree_action_2button_press = 0;				/* "gtk-default" */
    tmp_str = g_strdup (g_get_user_special_dir (G_USER_DIRECTORY_DOCUMENTS));
	gsb_file_init_last_path (tmp_str);
    g_free (tmp_str);

    /* Home page */
    conf.pluriel_final = FALSE;

	/* variables for the list of transactions */
	conf.show_transaction_gives_balance = FALSE;
    conf.show_transaction_selected_in_form = TRUE;
    conf.transactions_list_primary_sorting = 0, 		/* "Sort by value date" */
    conf.transactions_list_secondary_sorting = 0;		/* "Sort by transaction number" */

    /* settings_geometry */
	conf.full_screen = FALSE;
    conf.main_height = 600;
    conf.main_width = 1000;
    conf.maximize_screen = FALSE;
    conf.x_position = 100;
   	conf.y_position= 100;

    /* settings_messages_delete */
	warning = (ConditionalMsg*) dialogue_get_tab_delete_msg ();
	for (i = 0; (warning+i)->name; i ++)
    {
		(warning+i)->hidden = TRUE;
    }

    /* settings_messages_tips */
    conf.last_tip = 0;
    conf.show_tip = TRUE;

    /* settings_messages_warnings */
	warning = (ConditionalMsg*) dialogue_get_tab_warning_msg ();
	for (i = 0; (warning+i)->name; i ++)
    {
		(warning+i)->hidden = TRUE;
    }

    /* settings_panel */
    conf.active_scrolling_left_pane = FALSE;
    conf.panel_width = 250;

	/* settings_prefs */
    conf.prefs_archives_sort_order = FALSE;
    conf.prefs_fyear_sort_order = FALSE;
    conf.prefs_height = 600;
	conf.prefs_panel_width = 360;
    conf.prefs_width = 1050;

	/* settings_scheduled */
    conf.balances_with_scheduled = TRUE;
    conf.execute_scheduled_of_month = FALSE;
    conf.nb_days_before_scheduled = 0;
    conf.scheduler_fixed_day = 1;
    conf.scheduler_set_fixed_day = FALSE;
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 * load the config file
 * it uses the glib config utils after 0.6.0
 * if cannot load, try the xml file before that version
 *
 * \param
 *
 * \return TRUE if ok, FALSE if not found, usually for a new opening
 **/
gboolean grisbi_conf_load_app_config (void)
{
    GKeyFile *config;
    gboolean result;
    const gchar *filename;
	gchar **recent_array;
	gchar *tmp_path;
	gchar *tmp_str;
    gint i;
	ConditionalMsg *warning;

    grisbi_conf_clean_config ();

    filename = gsb_dirs_get_grisbirc_filename ();
    config = g_key_file_new ();

    result = g_key_file_load_from_file (config,
										filename,
										G_KEY_FILE_KEEP_COMMENTS,
										NULL);
	if (!result)
		return FALSE;

	/* set settings root */
	conf.first_use = g_key_file_get_boolean (config,
											 "Root",
											 "first-use",
											 NULL);

	/* set settings_backup */
    /* on fixe le backup path s'il n'est pas initialisé */
	tmp_path = g_key_file_get_string (config,
								      "Backup",
								      "backup-path",
								      NULL);
    if (tmp_path == NULL || strlen (tmp_path) == 0)
    {
        tmp_path = g_strdup (gsb_dirs_get_user_data_dir ());
    }
    gsb_file_set_backup_path (tmp_path);
    g_free (tmp_path);

    conf.compress_backup = g_key_file_get_integer (config,
												   "Backup",
												   "compress-backup",
												   NULL);
    conf.make_backup_every_minutes = g_key_file_get_integer (config,
															 "Backup",
															 "make-backup-every-minutes",
															 NULL);
    conf.make_backup_nb_minutes = g_key_file_get_boolean (config,
														  "Backup",
														  "make-backup-nb-minutes",
														  NULL);
	conf.make_bakup_single_file = g_key_file_get_boolean (config,
														  "Backup",
														  "make-backup-single-file",
														  NULL);
	conf.remove_backup_files = g_key_file_get_boolean (config,
													   "Backup",
													   "remove-backup-files",
													   NULL);
    conf.remove_backup_months = g_key_file_get_integer (config,
														"Backup",
														"remove-backup-months",
														NULL);
    conf.sauvegarde_demarrage = g_key_file_get_boolean (config,
														"Backup",
														"sauvegarde-ouverture",
														NULL);
	conf.sauvegarde_fermeture = g_key_file_get_boolean (config,
														"Backup",
														"sauvegarde-fermeture",
														NULL);

	/* priv->settings_display */
	tmp_str = g_key_file_get_string (config,
								     "Display",
								     "display-toolbar",
								     NULL);
    if (g_strcmp0 (tmp_str, "Text") == 0)
        conf.display_toolbar = GSB_BUTTON_TEXT;
    else if (g_strcmp0 (tmp_str, "Icons") == 0)
        conf.display_toolbar = GSB_BUTTON_ICON;
    else
        conf.display_toolbar = GSB_BUTTON_BOTH;
    g_free (tmp_str);

	tmp_str = g_key_file_get_string (config,
								     "Display",
								     "display-window-title",
								     NULL);
    if (g_strcmp0 (tmp_str, "Holder name") == 0)
        conf.display_window_title = 1;
    else if (g_strcmp0 (tmp_str, "Filename") == 0)
        conf.display_window_title = 2;
    else
        conf.display_window_title = 0;
    g_free (tmp_str);

	conf.formulaire_toujours_affiche = g_key_file_get_boolean (config,
															   "Display",
															   "formulaire-toujours-affiche",
															   NULL);
	conf.group_partial_balance_under_accounts = g_key_file_get_boolean (config,
																		"Display",
																		"group-partial-balance-under-accounts",
																		NULL);
	conf.show_closed_accounts = g_key_file_get_boolean (config,
													    "Display",
													    "show-closed-accounts",
													    NULL);
	conf.show_headings_bar = g_key_file_get_boolean (config,
												     "Display",
													 "show-headings-bar",
													 NULL);

    /* settings_file */
	conf.archives_check_auto = g_key_file_get_boolean (config,
													  "File",
													  "archives-check-auto",
													  NULL);
	conf.compress_file = g_key_file_get_boolean (config,
												 "File",
												 "compress-file",
												 NULL);
	conf.dernier_fichier_auto = g_key_file_get_boolean (config,
													    "File",
													    "dernier-fichier-auto",
													    NULL);
	conf.force_enregistrement = g_key_file_get_boolean (config,
													    "File",
													    "force-enregistrement",
													    NULL);
    conf.force_import_directory = g_key_file_get_boolean (config,
														  "File",
														  "force-import-directory",
														  NULL);
	if (conf.force_import_directory)
	{
	    conf.import_directory = g_key_file_get_string (config,
													   "File",
													   "import-directory",
														NULL);
	}
	conf.import_remove_file = g_key_file_get_boolean (config,
													  "File",
													  "import-remove-file",
													  NULL);
	conf.max_non_archived_transactions_for_check = g_key_file_get_integer (config,
																		   "File",
																		   "max-transactions-before-warn-archival",
																		   NULL);
	recent_array = g_key_file_get_string_list (config,
											   "File",
											   "names-last-files",
                        			 		   NULL,
											   NULL);
	if (recent_array)
	{
        conf.nb_derniers_fichiers_ouverts = g_strv_length (recent_array);
        if (conf.nb_derniers_fichiers_ouverts > 0)
        {
			if (conf.nb_derniers_fichiers_ouverts > conf.nb_max_derniers_fichiers_ouverts)
			{
				conf.nb_derniers_fichiers_ouverts = conf.nb_max_derniers_fichiers_ouverts;
			}

            conf.last_open_file = my_strdup (recent_array[0]);
        }
    }
	grisbi_app_set_recent_files_array (recent_array);

	conf.nb_max_derniers_fichiers_ouverts = g_key_file_get_integer (config,
																	"File",
																	"nb-max-derniers-fichiers-ouverts",
																	NULL);
	conf.sauvegarde_auto = g_key_file_get_boolean (config,
												   "File",
												   "sauvegarde-auto",
												   NULL);

    /* settings_form */
    conf.affichage_exercice_automatique = g_key_file_get_boolean (config,
																  "Form",
																  "affichage-exercice-automatique",
																  NULL);
    conf.automatic_completion_payee = g_key_file_get_boolean (config,
															  "Form",
															  "automatic-completion-payee",
															  NULL);
    conf.automatic_erase_credit_debit = g_key_file_get_boolean (config,
																"Form",
																"automatic-erase-credit-debit",
																NULL);
    conf.automatic_recover_splits = g_key_file_get_boolean (config,
															"Form",
															"automatic-recover-splits",
															NULL);
	conf.completion_minimum_key_length = g_key_file_get_integer (config,
															 	 "Form",
																 "completion-minimum-key-length",
															 	 NULL);
	conf.form_enter_key = g_key_file_get_boolean (config,
												 "Form",
												 "form-enter-key",
												 NULL);
	conf.fyear_combobox_sort_order = g_key_file_get_integer (config,
															 "Form",
															 "fyear-combobox-sort-order",
															 NULL);
    conf.limit_completion_to_current_account = g_key_file_get_boolean (config,
															  		   "Form",
																	   "limit-completion-current-account",
															 		   NULL);

    /* settings_general */
    tmp_str = g_key_file_get_string (config,
								     "General",
								     "browser-command",
								     NULL);
	if (tmp_str == NULL || strlen (tmp_str) == 0)
    {
        conf.browser_command = g_strdup (ETAT_WWW_BROWSER);
    }
    else
    {
        conf.browser_command = g_strdup (tmp_str);
        g_free (tmp_str);
    }

    tmp_str = g_key_file_get_string (config,
									 "General",
									 "current-theme",
									 NULL);
	if (tmp_str == NULL || strlen (tmp_str) == 0)
    {
        conf.current_theme = g_strdup ("null");
    }
    else
    {
        conf.current_theme = g_strdup (tmp_str);
        g_free (tmp_str);
    }

    conf.custom_fonte_listes = g_key_file_get_boolean (config,
													   "General",
													   "custom-fonte-listes",
													   NULL);
	if (conf.custom_fonte_listes)
    {
        conf.font_string = g_key_file_get_string (config,
											      "General",
											      "font-string",
											      NULL);
		if (strlen (conf.font_string) == 0)
		{
			conf.font_string = my_strdup (_("Monospace 10"));
			conf.custom_fonte_listes = FALSE;
		}
    }

    conf.force_type_theme = g_key_file_get_integer (config,
													"General",
													"force-type-theme",
													NULL);
    tmp_str = g_key_file_get_string (config,
								     "General",
									 "language-chosen",
									 NULL);
    if (tmp_str == NULL)
    {
        conf.language_chosen = NULL;
    }
	else if (strlen (tmp_str) == 0)
    {
        conf.language_chosen = NULL;
        g_free (tmp_str);
    }
    else
    {
        conf.language_chosen = g_strdup (tmp_str);
        g_free (tmp_str);
    }

	tmp_str = g_key_file_get_string (config,
									 "General",
									 "last-path",
									 NULL);
    if (tmp_str == NULL || strlen (tmp_str) == 0)
    {
        tmp_str = g_strdup (g_get_user_special_dir (G_USER_DIRECTORY_DOCUMENTS));
    }
	else
	{
		if (!g_file_test (tmp_str, G_FILE_TEST_EXISTS))
		{
			g_free (tmp_str);
			tmp_str = g_strdup (g_get_user_special_dir (G_USER_DIRECTORY_DOCUMENTS));
		}
	}
	gsb_file_init_last_path (tmp_str);
    g_free (tmp_str);

	/* variables for the list of categories */
    tmp_str = g_key_file_get_string (config,
								     "General",
								     "metatree-action-2button-press",
								     NULL);
    if (g_strcmp0 (tmp_str, "Edit Category") == 0)
        conf.metatree_action_2button_press = 1;
    else if (g_strcmp0 (tmp_str, "Manage division") == 0)
        conf.metatree_action_2button_press = 2;
    else
        conf.metatree_action_2button_press = 0;
    g_free (tmp_str);

    /* Home page */
    conf.pluriel_final = g_key_file_get_boolean (config,
												 "General",
												 "pluriel-final",
												 NULL);

	/* variables for the list of transactions */
	conf.show_transaction_gives_balance = g_key_file_get_boolean (config,
															      "General",
																  "show-transaction-gives-balance",
																  NULL);
    conf.show_transaction_selected_in_form = g_key_file_get_boolean (config,
															      	 "General",
																	 "show-transaction-selected-in-form",
																	 NULL);

    tmp_str = g_key_file_get_string (config,
								     "General",
									 "transactions-list-primary-sorting",
									 NULL);
    if (g_strcmp0 (tmp_str, "Sort by value date") == 0)
        conf.transactions_list_primary_sorting = 0;
    else if (g_strcmp0 (tmp_str, "Forced sort by date") == 0)
        conf.transactions_list_primary_sorting = 2;
    else
        conf.transactions_list_primary_sorting = 1;
    g_free (tmp_str);

    tmp_str = g_key_file_get_string (config,
								     "General",
									 "transactions-list-secondary-sorting",
									 NULL);
    if (g_strcmp0 (tmp_str, "Sort by transaction number") == 0)
        conf.transactions_list_secondary_sorting = 0;
    else if (g_strcmp0 (tmp_str, "Sort by type of amount") == 0)
        conf.transactions_list_secondary_sorting = 1;
    else if (g_strcmp0 (tmp_str, "Sort by payee name") == 0)
        conf.transactions_list_secondary_sorting = 2;
    else
        conf.transactions_list_secondary_sorting = 3;
    g_free (tmp_str);

    /* settings_geometry */
    conf.low_resolution_screen = g_key_file_get_boolean (config,
														 "Geometry",
													     "low-resolution-screen",
													     NULL);
	conf.full_screen = g_key_file_get_boolean (config,
											   "Geometry",
											   "fullscreen",
											   NULL);
    conf.main_height = g_key_file_get_integer (config,
											   "Geometry",
											   "main-height",
											   NULL);
    conf.main_width = g_key_file_get_integer (config,
											  "Geometry",
											  "main-width",
											  NULL);
    conf.maximize_screen = g_key_file_get_boolean (config,
												   "Geometry",
												   "maximized",
												   NULL);
    conf.x_position = g_key_file_get_integer (config,
											  "Geometry",
											  "x-position",
											  NULL);
   	conf.y_position= g_key_file_get_integer (config,
											 "Geometry",
											 "y-position",
											 NULL);

    /* settings_messages_delete */
	warning = (ConditionalMsg*) dialogue_get_tab_delete_msg ();
    for (i = 0; (warning+i)->name; i ++)
    {
        (warning+i)->hidden = !g_key_file_get_boolean (config,
													   "Delete_msg",
													   (warning+i)->name,
													   NULL);
        if ((warning+i)->hidden == 1)
            (warning+i)->default_answer = 1;
    }

    /* settings_messages_tips */
    conf.last_tip = g_key_file_get_boolean (config,
											"Tips",
											"last-tip",
											NULL);
    conf.show_tip = g_key_file_get_boolean (config,
											"Tips",
											"show-tip",
											NULL);

    /* settings_messages_warnings */
	warning = (ConditionalMsg*) dialogue_get_tab_warning_msg ();
    for (i = 0; (warning+i)->name; i ++)
    {
        (warning+i)->hidden = !g_key_file_get_boolean (config,
													   "Warnings_msg",
													   (warning+i)->name,
													   NULL);
		if ((warning+i)->hidden == 1)
			(warning+i)->default_answer = 1;

    }

    /* settings_panel */
    conf.active_scrolling_left_pane = g_key_file_get_boolean (config,
															  "Panel",
															  "active-scrolling-left-pane",
															  NULL);
    conf.panel_width = g_key_file_get_integer (config,
											   "Panel",
											   "panel-width",
											   NULL);

	/* settings_prefs */
    conf.prefs_archives_sort_order = g_key_file_get_boolean (config,
														     "Prefs",
															 "prefs-archives-sort-order",
															 NULL);
    conf.prefs_fyear_sort_order = g_key_file_get_boolean (config,
													      "Prefs",
														  "prefs-fyear-sort-order",
														  NULL);
    conf.prefs_height = g_key_file_get_integer (config,
											    "Prefs",
											    "prefs-height",
											    NULL);

   	conf.prefs_panel_width = g_key_file_get_integer (config,
												     "Prefs",
												     "prefs-panel-width",
												     NULL);

    conf.prefs_width = g_key_file_get_integer (config,
											   "Prefs",
											   "prefs-width",
											   NULL);

	/* settings_scheduled */
    conf.balances_with_scheduled = g_key_file_get_boolean (config,
							"Scheduled",
							"balances-with-scheduled",
							NULL);
    conf.execute_scheduled_of_month = g_key_file_get_boolean (config,
							"Scheduled",
                        	"execute-scheduled-of-month",
                        	NULL);
    conf.nb_days_before_scheduled = g_key_file_get_integer  (config,
							"Scheduled",
                        	"nb-days-before-scheduled",
                        	NULL);
    conf.scheduler_fixed_day = g_key_file_get_integer (config,
							"Scheduled",
							"scheduler-fixed-day",
							NULL);
    conf.scheduler_set_fixed_day = g_key_file_get_boolean (config,
							"Scheduled",
							"scheduler-set-fixed-day",
							NULL);

	g_key_file_free (config);
    return TRUE;
}


/**
 * save the config file
 * it uses the glib config utils after 0.6.0
 * if cannot load, try the xml file before that version
 *
 * \param
 *
 * \return TRUE if ok
 **/
gboolean grisbi_conf_save_app_config (void)
{
	GKeyFile *config;
	const gchar* backup_path = NULL;
	const gchar *filename;
	gchar *file_content;
	gchar **recent_array;
	gchar *tmp_str;
	gsize length;
	FILE *conf_file;
	gint i;
	ConditionalMsg *warning;

	devel_debug (NULL);

	filename = gsb_dirs_get_grisbirc_filename ();
	config = g_key_file_new ();

	/* settings_root */
	g_key_file_set_boolean (config,
							"Root",
							"first-use",
							conf.first_use);
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
							conf.compress_backup);
	g_key_file_set_boolean (config,
							"Backup",
							"make-backup-every-minutes",
							conf.make_backup_every_minutes);
	g_key_file_set_integer (config,
							"Backup",
							"make-backup-nb-minutes",
							conf.make_backup_nb_minutes);
	g_key_file_set_boolean (config,
							"Backup",
							"make-backup-single-file",
							conf.make_bakup_single_file);
	g_key_file_set_boolean (config,
						    "Backup",
						    "remove-backup-files",
						    conf.remove_backup_files);
    g_key_file_set_integer (config,
							"Backup",
							"remove-backup-months",
							conf.remove_backup_months);
	g_key_file_set_boolean (config,
							"Backup",
							"sauvegarde-fermeture",
							conf.sauvegarde_fermeture);
	g_key_file_set_boolean (config,
							"Backup",
							"sauvegarde-ouverture",
							conf.sauvegarde_demarrage);

    /* settings_display */
    switch (conf.display_toolbar)
    {
        case GSB_BUTTON_TEXT:
            tmp_str = "Text";
            break;
        case GSB_BUTTON_ICON:
            tmp_str = "Icons";
            break;
        default:
            tmp_str = "Text + Icons";
    }
    g_key_file_set_string (config,
						   "Display",
						   "display-toolbar",
						   tmp_str);
    switch (conf.display_window_title)
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
						   conf.formulaire_toujours_affiche);
	g_key_file_set_boolean (config,
						    "Display",
							"group-partial-balance-under-accounts",
							conf.group_partial_balance_under_accounts);
	g_key_file_set_boolean (config,
						   "Display",
						   "show-closed-accounts",
						   conf.show_closed_accounts);
	g_key_file_set_boolean (config,
						   "Display",
							"show-headings-bar",
							conf.show_headings_bar);

    /* settings_file */
	g_key_file_set_boolean (config,
							"File",
							"archives-check-auto",
							conf.archives_check_auto);
	g_key_file_set_boolean (config,
							"File",
							"compress-file",
							conf.compress_file);
	g_key_file_set_boolean (config,
						   "File",
						   "dernier-fichier-auto",
						   conf.dernier_fichier_auto);
	g_key_file_set_boolean (config,
						   "File",
						   "force-enregistrement",
						   conf.force_enregistrement);
    g_key_file_set_boolean (config,
						   "File",
						   "force-import-directory",
						   conf.force_import_directory);
	if (conf.force_import_directory)
	{
	    g_key_file_set_string (config,
							   "File",
							   "import-directory",
								conf.import_directory);
	}
	g_key_file_set_boolean (config,
							"File",
							"import-remove-file",
							conf.import_remove_file);
	g_key_file_set_integer (config,
							"File",
							"max-transactions-before-warn-archival",
							conf.max_non_archived_transactions_for_check);
	recent_array = grisbi_app_get_recent_files_array ();
	if (recent_array && g_strv_length (recent_array))
	{
		 g_key_file_set_string_list (config,
									 "File",
									 "names-last-files",
									 (const gchar **) recent_array,
                        			 conf.nb_derniers_fichiers_ouverts);
	}
	g_key_file_set_integer (config,
							"File",
							"nb-max-derniers-fichiers-ouverts",
							conf.nb_max_derniers_fichiers_ouverts);
	g_key_file_set_boolean (config,
						   "File",
						   "sauvegarde-auto",
                           conf.sauvegarde_auto);

    /* settings_form */
    g_key_file_set_boolean (config,
							"Form",
                        	"affichage-exercice-automatique",
                        	conf.affichage_exercice_automatique);
    g_key_file_set_boolean (config,
							"Form",
							"automatic-completion-payee",
							conf.automatic_completion_payee);
	g_key_file_set_boolean (config,
							"Form",
							"automatic-erase-credit-debit",
							conf.automatic_erase_credit_debit);
	g_key_file_set_boolean (config,
							"Form",
							"automatic-recover-splits",
							conf.automatic_recover_splits);

    g_key_file_set_integer (config,
							"Form",
							"completion-minimum-key-length",
							conf.completion_minimum_key_length);
	g_key_file_set_boolean (config,
							"Form",
							"form-enter-key", conf.form_enter_key);
    g_key_file_set_integer (config,
							"Form",
							"fyear-combobox-sort-order",
							conf.fyear_combobox_sort_order);
	g_key_file_set_boolean (config,
							"Form",
							"limit-completion-current-account",
    						conf.limit_completion_to_current_account);

    /* settings_general */
    if (conf.browser_command)
        g_key_file_set_string (config,
							   "General",
							   "browser-command",
							   conf.browser_command);
    g_key_file_set_string (config,
						   "General",
						   "current-theme",
						   conf.current_theme);

    g_key_file_set_boolean (config,
							"General",
                        	"custom-fonte-listes",
                        	conf.custom_fonte_listes );
    if (conf.font_string)
        g_key_file_set_string (config,
							   "General",
                        	   "font-string",
                        	   conf.font_string);
    g_key_file_set_integer (config,
						   	"General",
							"force-type-theme",
							conf.force_type_theme);
	if (conf.language_chosen)
		g_key_file_set_string (config,
							   "General",
							   "language-chosen",
							   conf.language_chosen);
	g_key_file_set_string (config,
						   "General",
						   "last-path",
						   gsb_file_get_last_path());

	switch (conf.metatree_action_2button_press)
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
                           "metatree-action-2button-press",
                           tmp_str);
    g_key_file_set_boolean (config,
						   	"General",
							"pluriel-final",
							conf.pluriel_final);
    g_key_file_set_boolean (config,
						    "General",
                            "show-transaction-gives-balance",
                            conf.show_transaction_gives_balance);
	g_key_file_set_boolean (config,
							 "General",
							 "show-transaction-selected-in-form",
							 conf.show_transaction_selected_in_form);

	switch (conf.transactions_list_primary_sorting)
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

	switch (conf.transactions_list_secondary_sorting)
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
							"low-resolution-screen",
							conf.low_resolution_screen);
	g_key_file_set_boolean (config,
							"Geometry",
							"fullscreen",
                        	conf.full_screen);
    g_key_file_set_integer (config,
							"Geometry",
                        	"main-height",
                        	conf.main_height);
    g_key_file_set_integer (config,
							"Geometry",
                        	"main-width",
                        	conf.main_width);
    g_key_file_set_boolean (config,
							"Geometry",
                        	"maximized",
                        	conf.maximize_screen);
    g_key_file_set_integer (config,
							"Geometry",
                        	"x-position",
                        	conf.x_position);
    g_key_file_set_integer (config,
							"Geometry",
                        	"y-position",
                        	conf.y_position);

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
                        	conf.last_tip);
    g_key_file_set_boolean (config,
							"Tips",
                        	"show-tip",
                        	conf.show_tip);

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
							conf.active_scrolling_left_pane);
    g_key_file_set_integer (config,
							"Panel",
                        	"panel-width",
                        	conf.panel_width);

	/* settings_prefs */
    g_key_file_set_boolean (config,
							"Prefs",
							"prefs-archives-sort-order",
							conf.prefs_archives_sort_order);
   g_key_file_set_boolean (config,
						   "Prefs",
						   "prefs-fyear-sort-order",
						   conf.prefs_fyear_sort_order);
    g_key_file_set_integer (config,
							"Prefs",
                        	"prefs-height",
                        	conf.prefs_height);

    g_key_file_set_integer (config,
							"Prefs",
                        	"prefs-panel-width",
                        	conf.prefs_panel_width);

    g_key_file_set_integer (config,
							"Prefs",
                        	"prefs-width",
                        	conf.prefs_width);

	/* settings_scheduled */
    g_key_file_set_boolean (config,
							"Scheduled",
							"balances-with-scheduled",
							conf.balances_with_scheduled);
    g_key_file_set_boolean (config,
							"Scheduled",
                        	"execute-scheduled-of-month",
                        	conf.execute_scheduled_of_month);
    g_key_file_set_integer  (config,
							"Scheduled",
                        	"nb-days-before-scheduled",
                        	conf.nb_days_before_scheduled);
    g_key_file_set_integer (config,
							"Scheduled",
							"scheduler-fixed-day",
							conf.scheduler_fixed_day);
    g_key_file_set_boolean (config,
							"Scheduled",
							"scheduler-set-fixed-day",
							conf.scheduler_set_fixed_day);

    /* save into a file */
    file_content = g_key_file_to_data (config, &length, NULL);

    conf_file = fopen (filename, "w");

    #ifndef _WIN32
    if (!conf_file)
    {
        utils_files_create_XDG_dir ();
        conf_file = fopen (filename, "w");
    }
    #endif

    if (!conf_file || !fwrite (file_content, sizeof (gchar), length, conf_file))
    {
		gchar* tmp_str;

        tmp_str = g_strdup_printf (_("Cannot save configuration file '%s': %s"),
								  filename,
								  g_strerror (errno));
        dialogue_error (tmp_str);
        g_free (tmp_str);
        g_free (file_content);
        g_key_file_free (config);
		if (conf_file)
			fclose (conf_file);

		return FALSE;
    }

    fclose (conf_file);
    g_free (file_content);
    g_key_file_free (config);

    return TRUE;
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
