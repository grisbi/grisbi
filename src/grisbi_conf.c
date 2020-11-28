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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/**
 * \file gsb_file_config.c
 * save and load the config file
 */
#ifdef USE_CONFIG_FILE

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
static void grisbi_conf_clean_config (GrisbiAppConf *a_conf)
{
	gchar * tmp_str;
	gint i = 0;
	ConditionalMsg *warning;

    devel_debug (NULL);

	/* set settings root */
	a_conf->first_use = TRUE;

	/* set settings_backup */
    a_conf->compress_backup = FALSE;
    a_conf->make_backup_every_minutes = FALSE;
    a_conf->make_backup_nb_minutes = 5;
	a_conf->make_bakup_single_file = FALSE;
	a_conf->remove_backup_files = FALSE;
	a_conf->remove_backup_months = 12;
    a_conf->sauvegarde_demarrage = FALSE;
	a_conf->sauvegarde_fermeture = TRUE;

	/* priv->settings_display */
    a_conf->display_toolbar = GSB_BUTTON_BOTH;				/* "Text + Icons" */
	a_conf->display_window_title = 0;						/* "Entity name" */
	a_conf->formulaire_toujours_affiche = TRUE;
	a_conf->group_partial_balance_under_accounts = TRUE;
	a_conf->show_closed_accounts = FALSE;
	a_conf->show_headings_bar = TRUE;

    /* settings_file */
	a_conf->archives_check_auto = TRUE;
	a_conf->compress_file = FALSE;
	a_conf->dernier_fichier_auto = FALSE;
	a_conf->force_enregistrement = FALSE;
    a_conf->force_import_directory = FALSE;
	a_conf->import_remove_file = FALSE;
	a_conf->max_non_archived_transactions_for_check = 3000;
	grisbi_app_set_recent_files_array (NULL);
	a_conf->nb_max_derniers_fichiers_ouverts = 3;
	a_conf->sauvegarde_auto = FALSE;

    /* settings_form */
    a_conf->affichage_exercice_automatique = FALSE;
    a_conf->automatic_completion_payee = FALSE;
    a_conf->automatic_erase_credit_debit = FALSE;
    a_conf->automatic_recover_splits = FALSE;
	a_conf->completion_minimum_key_length = 1;
	a_conf->form_enter_key = FALSE;
	a_conf->form_validate_split = FALSE;
	a_conf->fyear_combobox_sort_order = 0;
    a_conf->limit_completion_to_current_account = FALSE;

    /* settings_general */
    a_conf->custom_fonte_listes = FALSE;
    a_conf->force_type_theme = 0;
	a_conf->metatree_action_2button_press = 0;				/* "gtk-default" */
    tmp_str = g_strdup (g_get_user_special_dir (G_USER_DIRECTORY_DOCUMENTS));
	gsb_file_init_last_path (tmp_str);
    g_free (tmp_str);

    /* Home page */
    a_conf->pluriel_final = FALSE;

	/* variables for the list of transactions */
	a_conf->show_transaction_gives_balance = FALSE;
    a_conf->show_transaction_selected_in_form = TRUE;
    a_conf->transactions_list_primary_sorting = 0; 		/* "Sort by value date" */
    a_conf->transactions_list_secondary_sorting = 0;		/* "Sort by transaction number" */

    /* settings_geometry */
	a_conf->full_screen = FALSE;
    a_conf->main_height = WIN_MIN_HEIGHT;
    a_conf->main_width = WIN_MIN_WIDTH;
    a_conf->maximize_screen = FALSE;
    a_conf->x_position = WIN_POS_X;
   	a_conf->y_position= WIN_POS_Y;

    /* settings_messages_delete */
	warning = (ConditionalMsg*) dialogue_get_tab_delete_msg ();
	for (i = 0; (warning+i)->name; i ++)
    {
		(warning+i)->hidden = TRUE;
    }

    /* settings_messages_tips */
    a_conf->last_tip = 0;
    a_conf->show_tip = TRUE;

    /* settings_messages_warnings */
	warning = (ConditionalMsg*) dialogue_get_tab_warning_msg ();
	for (i = 0; (warning+i)->name; i ++)
    {
		(warning+i)->hidden = TRUE;
    }

    /* settings_panel */
    a_conf->active_scrolling_left_pane = FALSE;
    a_conf->panel_width = PANEL_MIN_WIDTH;

	/* settings_prefs */
    a_conf->prefs_archives_sort_order = FALSE;
    a_conf->prefs_fyear_sort_order = FALSE;
    a_conf->prefs_height = 600;
	a_conf->prefs_panel_width = 360;
    a_conf->prefs_width = 1050;

	/* settings_scheduled */
    a_conf->balances_with_scheduled = TRUE;
    a_conf->execute_scheduled_of_month = FALSE;
    a_conf->nb_days_before_scheduled = 0;
    a_conf->scheduler_fixed_day = 1;
    a_conf->scheduler_set_fixed_day = FALSE;
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
	GrisbiAppConf *a_conf;

    devel_debug (NULL);

	a_conf = grisbi_app_get_a_conf ();
	grisbi_conf_clean_config (a_conf);

    filename = gsb_dirs_get_grisbirc_filename ();
    config = g_key_file_new ();

    result = g_key_file_load_from_file (config,
										filename,
										G_KEY_FILE_KEEP_COMMENTS,
										NULL);
	if (!result)
		return FALSE;

	/* set settings root */
	a_conf->first_use = g_key_file_get_boolean (config,
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

    a_conf->compress_backup = g_key_file_get_integer (config,
													  "Backup",
													  "compress-backup",
													  NULL);
    a_conf->make_backup_every_minutes = g_key_file_get_integer (config,
																"Backup",
																"make-backup-every-minutes",
																NULL);
    a_conf->make_backup_nb_minutes = g_key_file_get_boolean (config,
															 "Backup",
															 "make-backup-nb-minutes",
															 NULL);
	a_conf->make_bakup_single_file = g_key_file_get_boolean (config,
															 "Backup",
															 "make-backup-single-file",
															 NULL);
	a_conf->remove_backup_files = g_key_file_get_boolean (config,
													  	  "Backup",
														  "remove-backup-files",
														  NULL);
    a_conf->remove_backup_months = g_key_file_get_integer (config,
														  "Backup",
														  "remove-backup-months",
														  NULL);
    a_conf->sauvegarde_demarrage = g_key_file_get_boolean (config,
														  "Backup",
														  "sauvegarde-ouverture",
														  NULL);
	a_conf->sauvegarde_fermeture = g_key_file_get_boolean (config,
														  "Backup",
														  "sauvegarde-fermeture",
														  NULL);

	/* priv->settings_display */
	tmp_str = g_key_file_get_string (config,
								     "Display",
								     "display-toolbar",
								     NULL);
    if (g_strcmp0 (tmp_str, "Text") == 0)
        a_conf->display_toolbar = GSB_BUTTON_TEXT;
    else if (g_strcmp0 (tmp_str, "Icons") == 0)
        a_conf->display_toolbar = GSB_BUTTON_ICON;
    else
        a_conf->display_toolbar = GSB_BUTTON_BOTH;
    g_free (tmp_str);

	tmp_str = g_key_file_get_string (config,
								     "Display",
								     "display-window-title",
								     NULL);
    if (g_strcmp0 (tmp_str, "Holder name") == 0)
        a_conf->display_window_title = 1;
    else if (g_strcmp0 (tmp_str, "Filename") == 0)
        a_conf->display_window_title = 2;
    else
        a_conf->display_window_title = 0;
    g_free (tmp_str);

	a_conf->formulaire_toujours_affiche = g_key_file_get_boolean (config,
																  "Display",
																  "formulaire-toujours-affiche",
																  NULL);
	a_conf->group_partial_balance_under_accounts = g_key_file_get_boolean (config,
																		  "Display",
																		  "group-partial-balance-under-accounts",
																		  NULL);
	a_conf->show_closed_accounts = g_key_file_get_boolean (config,
													      "Display",
													      "show-closed-accounts",
													      NULL);
	a_conf->show_headings_bar = g_key_file_get_boolean (config,
												    	"Display",
														"show-headings-bar",
														NULL);

    /* settings_file */
	a_conf->archives_check_auto = g_key_file_get_boolean (config,
														  "File",
														  "archives-check-auto",
														  NULL);
	a_conf->compress_file = g_key_file_get_boolean (config,
													"File",
													"compress-file",
													NULL);
	a_conf->dernier_fichier_auto = g_key_file_get_boolean (config,
													       "File",
													       "dernier-fichier-auto",
													       NULL);
	a_conf->force_enregistrement = g_key_file_get_boolean (config,
													       "File",
													       "force-enregistrement",
													       NULL);
    a_conf->force_import_directory = g_key_file_get_boolean (config,
														  "File",
														  "force-import-directory",
														  NULL);
	if (a_conf->force_import_directory)
	{
	    a_conf->import_directory = g_key_file_get_string (config,
														  "File",
														  "import-directory",
														  NULL);
	}
	a_conf->import_remove_file = g_key_file_get_boolean (config,
														 "File",
														 "import-remove-file",
														 NULL);
	a_conf->max_non_archived_transactions_for_check = g_key_file_get_integer (config,
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

	a_conf->nb_max_derniers_fichiers_ouverts = g_key_file_get_integer (config,
																	   "File",
																	   "nb-max-derniers-fichiers-ouverts",
																	   NULL);
	a_conf->sauvegarde_auto = g_key_file_get_boolean (config,
													  "File",
													  "sauvegarde-auto",
													  NULL);

    /* settings_form */
    a_conf->affichage_exercice_automatique = g_key_file_get_boolean (config,
																	 "Form",
																	 "affichage-exercice-automatique",
																	 NULL);
    a_conf->automatic_completion_payee = g_key_file_get_boolean (config,
																 "Form",
																 "automatic-completion-payee",
																 NULL);
    a_conf->automatic_erase_credit_debit = g_key_file_get_boolean (config,
																   "Form",
																   "automatic-erase-credit-debit",
																   NULL);
    a_conf->automatic_recover_splits = g_key_file_get_boolean (config,
															   "Form",
															   "automatic-recover-splits",
															   NULL);
	a_conf->completion_minimum_key_length = g_key_file_get_integer (config,
															 		 "Form",
																	 "completion-minimum-key-length",
															 		 NULL);
	a_conf->form_enter_key = g_key_file_get_boolean (config,
													 "Form",
													 "form-enter-key",
													 NULL);
	a_conf->form_validate_split = g_key_file_get_boolean (config,
														  "Form",
														  "form-validate-split",
														  NULL);
	a_conf->fyear_combobox_sort_order = g_key_file_get_integer (config,
																"Form",
																"fyear-combobox-sort-order",
																NULL);
    a_conf->limit_completion_to_current_account = g_key_file_get_boolean (config,
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
        a_conf->browser_command = g_strdup (ETAT_WWW_BROWSER);
    }
    else
    {
        a_conf->browser_command = g_strdup (tmp_str);
        g_free (tmp_str);
    }

    tmp_str = g_key_file_get_string (config,
									 "General",
									 "current-theme",
									 NULL);
	if (tmp_str == NULL || strlen (tmp_str) == 0)
    {
        a_conf->current_theme = g_strdup ("null");
    }
    else
    {
        a_conf->current_theme = g_strdup (tmp_str);
        g_free (tmp_str);
    }

    a_conf->custom_fonte_listes = g_key_file_get_boolean (config,
														  "General",
														  "custom-fonte-listes",
														  NULL);
	if (a_conf->custom_fonte_listes)
    {
        a_conf->font_string = g_key_file_get_string (config,
											    	 "General",
											    	 "font-string",
											    	 NULL);
		if (!a_conf->font_string || strlen (a_conf->font_string) == 0)
		{
			a_conf->font_string = g_strdup ("Monospace 10");
			a_conf->custom_fonte_listes = FALSE;
		}
    }

    a_conf->force_type_theme = g_key_file_get_integer (config,
													   "General",
													   "force-type-theme",
													   NULL);
    tmp_str = g_key_file_get_string (config,
								     "General",
									 "language-chosen",
									 NULL);
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
								     "metatree-action-button-press",
								     NULL);
    if (g_strcmp0 (tmp_str, "Edit Category") == 0)
        a_conf->metatree_action_2button_press = 1;
    else if (g_strcmp0 (tmp_str, "Manage division") == 0)
        a_conf->metatree_action_2button_press = 2;
    else
        a_conf->metatree_action_2button_press = 0;
    g_free (tmp_str);

    /* Home page */
    a_conf->pluriel_final = g_key_file_get_boolean (config,
													"General",
													"pluriel-final",
													NULL);

	/* variables for the list of transactions */
	a_conf->show_transaction_gives_balance = g_key_file_get_boolean (config,
															    	 "General",
																	 "show-transaction-gives-balance",
																	 NULL);
    a_conf->show_transaction_selected_in_form = g_key_file_get_boolean (config,
															      		"General",
																		"show-transaction-selected-in-form",
																		NULL);

    tmp_str = g_key_file_get_string (config,
								     "General",
									 "transactions-list-primary-sorting",
									 NULL);
    if (g_strcmp0 (tmp_str, "Sort by value date") == 0)
        a_conf->transactions_list_primary_sorting = 0;
    else if (g_strcmp0 (tmp_str, "Forced sort by date") == 0)
        a_conf->transactions_list_primary_sorting = 2;
    else
        a_conf->transactions_list_primary_sorting = 1;
    g_free (tmp_str);

    tmp_str = g_key_file_get_string (config,
								     "General",
									 "transactions-list-secondary-sorting",
									 NULL);
    if (g_strcmp0 (tmp_str, "Sort by transaction number") == 0)
        a_conf->transactions_list_secondary_sorting = 0;
    else if (g_strcmp0 (tmp_str, "Sort by type of amount") == 0)
        a_conf->transactions_list_secondary_sorting = 1;
    else if (g_strcmp0 (tmp_str, "Sort by payee name") == 0)
        a_conf->transactions_list_secondary_sorting = 2;
    else
        a_conf->transactions_list_secondary_sorting = 3;
    g_free (tmp_str);

    /* settings_geometry */
    a_conf->low_resolution_screen = g_key_file_get_boolean (config,
															"Geometry",
													    	"low-resolution-screen",
													    	NULL);
	a_conf->full_screen = g_key_file_get_boolean (config,
												  "Geometry",
												  "fullscreen",
												  NULL);
    a_conf->main_height = g_key_file_get_integer (config,
												  "Geometry",
												  "main-height",
												  NULL);
    a_conf->main_width = g_key_file_get_integer (config,
												 "Geometry",
												 "main-width",
												 NULL);
    a_conf->maximize_screen = g_key_file_get_boolean (config,
													  "Geometry",
													  "maximized",
													  NULL);
    a_conf->x_position = g_key_file_get_integer (config,
												 "Geometry",
												 "x-position",
												 NULL);
   	a_conf->y_position= g_key_file_get_integer (config,
												"Geometry",
												"y-position",
												NULL);

    /* settings_messages_delete */
	warning = (ConditionalMsg*) dialogue_get_tab_delete_msg ();
    for (i = 0; (warning+i)->name; i ++)
    {
        (warning+i)->hidden = g_key_file_get_boolean (config,
													  "Delete_msg",
													  (warning+i)->name,
													  NULL);
        if ((warning+i)->hidden == 1)
            (warning+i)->default_answer = 1;
    }

    /* settings_messages_tips */
    a_conf->last_tip = g_key_file_get_boolean (config,
											"Tips",
											"last-tip",
											NULL);
    a_conf->show_tip = g_key_file_get_boolean (config,
											"Tips",
											"show-tip",
											NULL);

    /* settings_messages_warnings */
	warning = (ConditionalMsg*) dialogue_get_tab_warning_msg ();
    for (i = 0; (warning+i)->name; i ++)
    {
        (warning+i)->hidden = g_key_file_get_boolean (config,
													  "Warnings_msg",
													  (warning+i)->name,
													  NULL);
		if ((warning+i)->hidden == 1)
			(warning+i)->default_answer = 1;

    }

    /* settings_panel */
    a_conf->active_scrolling_left_pane = g_key_file_get_boolean (config,
															  "Panel",
															  "active-scrolling-left-pane",
															  NULL);
    a_conf->panel_width = g_key_file_get_integer (config,
											   "Panel",
											   "panel-width",
											   NULL);

	/* settings_prefs */
    a_conf->prefs_archives_sort_order = g_key_file_get_boolean (config,
														     "Prefs",
															 "prefs-archives-sort-order",
															 NULL);
    a_conf->prefs_fyear_sort_order = g_key_file_get_boolean (config,
													      "Prefs",
														  "prefs-fyear-sort-order",
														  NULL);
    a_conf->prefs_height = g_key_file_get_integer (config,
											    "Prefs",
											    "prefs-height",
											    NULL);

   	a_conf->prefs_panel_width = g_key_file_get_integer (config,
												     "Prefs",
												     "prefs-panel-width",
												     NULL);

    a_conf->prefs_width = g_key_file_get_integer (config,
											   "Prefs",
											   "prefs-width",
											   NULL);

	/* settings_scheduled */
    a_conf->balances_with_scheduled = g_key_file_get_boolean (config,
							"Scheduled",
							"balances-with-scheduled",
							NULL);
    a_conf->execute_scheduled_of_month = g_key_file_get_boolean (config,
							"Scheduled",
                        	"execute-scheduled-of-month",
                        	NULL);
    a_conf->nb_days_before_scheduled = g_key_file_get_integer  (config,
							"Scheduled",
                        	"nb-days-before-scheduled",
                        	NULL);
    a_conf->scheduler_fixed_day = g_key_file_get_integer (config,
							"Scheduled",
							"scheduler-fixed-day",
							NULL);
    a_conf->scheduler_set_fixed_day = g_key_file_get_boolean (config,
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
							"low-resolution-screen",
							a_conf->low_resolution_screen);
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
                        		(warning+i)->hidden);
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
                        		(warning+i)->hidden);
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
    g_key_file_set_boolean (config,
							"Prefs",
							"prefs-archives-sort-order",
							a_conf->prefs_archives_sort_order);
   g_key_file_set_boolean (config,
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

#endif	/* USE_CONFIG_FILE */
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
