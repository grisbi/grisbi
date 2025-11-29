/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2004-2009 Benjamin Drieu (bdrieu@april.org)                       */
/*          2009-2017 Pierre Biava (grisbi@pierre.biava.name)                 */
/*          https://www.grisbi.org/                                            */
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
/*  along with this program; if not, see <https://www.gnu.org/licenses/>.     */
/*                                                                            */
/* ************************************************************************** */

#include "config.h"

#include "include.h"
#include <glib/gi18n.h>
#include <stdlib.h>

/*START_INCLUDE*/
#include "menu.h"
#include "bet_tab.h"
#include "custom_list.h"
#include "etats_onglet.h"
#include "export.h"
#include "file_obfuscate_qif.h"
#include "file_obfuscate.h"
#include "grisbi_app.h"
#include "grisbi_prefs.h"
#include "gsb_account.h"
#include "gsb_assistant_account.h"
#include "gsb_assistant_archive.h"
#include "gsb_assistant_archive_export.h"
#include "gsb_assistant_file.h"
#include "gsb_data_account.h"
#include "gsb_data_mix.h"
#include "gsb_debug.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "gsb_form.h"
#include "gsb_scheduler_list.h"
#include "gsb_transactions_list.h"
#include "help.h"
#include "import.h"
#include "navigation.h"
#include "structures.h"
#include "traitement_variables.h"
#include "tip.h"
#include "utils.h"
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
 * Start a browser processus with Grisbi bug report page displayed.
 *
 * \return FALSE
 */
static gboolean gsb_menu_help_bugreport (void)
{
    lance_navigateur_web ("https://www.grisbi.org/bugsreports/");

    return FALSE;
}

/**
 * Start a browser processus with local copy of the quick start page
 * on command line.
 *
 * \return FALSE
 */
static gboolean gsb_menu_help_quick_start (void)
{
    gchar *lang = _("en");

    gchar* tmpstr = g_build_filename (gsb_dirs_get_help_dir (), lang, "quickstart.html", NULL);
    lance_navigateur_web (tmpstr);
    g_free (tmpstr);

    return FALSE;
}

/**
 * Start a browser processus with local copy of manual on command
 * line.
 *
 * \return FALSE
 */
static gboolean gsb_menu_help_manual (void)
{
    gchar *lang = _("en");
    gchar *string;
	GrisbiAppConf *a_conf;

	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();

	if (a_conf->display_help)
		string = g_build_filename (gsb_dirs_get_help_dir (), lang, "grisbi-manuel.pdf", NULL);
	else
    	string = g_build_filename (gsb_dirs_get_help_dir (), lang, "grisbi-manuel.html", NULL);

	if (!g_file_test (string, G_FILE_TEST_EXISTS))
    {
		gchar *str_to_free;

		str_to_free = string;
		string = g_strconcat(str_to_free, ".gz", NULL);
		g_free (str_to_free);
    }
	lance_navigateur_web (string);
	g_free(string);

    return FALSE;
}

/**
 * Start a browser processus with Grisbi website displayed.
 *
 * \return FALSE
 */
static gboolean gsb_menu_help_website (void)
{
    lance_navigateur_web ("https://www.grisbi.org/");

    return TRUE;
}

/**
 * Réinitialise la largeur des colonnes de la vue des opérations
 *
 * \param
 *
 * \return  FALSE
 * */
static gboolean gsb_menu_reinit_largeur_col_menu (void)
{
    gint current_page;

    current_page = gsb_gui_navigation_get_current_page ();

    if (current_page == GSB_ACCOUNT_PAGE)
    {
		GtkWidget *account_page;
		gint page_number;

		account_page = grisbi_win_get_account_page ();
		page_number = gtk_notebook_get_current_page (GTK_NOTEBOOK (account_page));
		if (page_number == GSB_TRANSACTIONS_PAGE)
		{
			gsb_transactions_list_init_tab_width_col_treeview (NULL);
			gsb_transactions_list_set_largeur_col ();
		}

		else if (page_number == GSB_ESTIMATE_PAGE)
		{
			bet_array_init_largeur_col_treeview (NULL);
			bet_array_list_set_largeur_col_treeview ();
		}
    }
    else if (current_page == GSB_SCHEDULER_PAGE)
    {
        gsb_scheduler_list_init_tab_width_col_treeview (NULL);
        gsb_scheduler_list_set_largeur_col ();
    }

    return FALSE;
}
/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 * remplace les "_" par "__"
 *
 * \param str
 *
 * \return chaine avec chaine de remplacement
 */
gchar *gsb_menu_normalise_label_name (const gchar *chaine)
{
    gchar **tab_str;
    gchar *result;

    tab_str = g_strsplit_set (chaine, "_", 0);
    result = g_strjoinv ("__", tab_str);
    g_strfreev (tab_str);

    return result;
}

/* fonctions de commande liées aux actions */
/* APP MENU */
/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 * */
void grisbi_cmd_about (GSimpleAction *action,
					   GVariant *parameter,
					   gpointer app)
{
	a_propos (NULL, 0);
}

/* PREFS MENU */
void grisbi_cmd_prefs (GSimpleAction *action,
					   GVariant *parameter,
					   gpointer app)
{
	gint result;
	GrisbiPrefs *prefs;
	GrisbiWin *win;
	GrisbiWinRun *w_run;

	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
	w_run->menu_prefs = TRUE;
	win = grisbi_app_get_active_window (app);
	prefs = grisbi_prefs_new (win);
	grisbi_win_set_prefs_dialog (win, GTK_WIDGET (prefs));

	gtk_window_present (GTK_WINDOW (prefs));
	gtk_widget_show_all (GTK_WIDGET (prefs));
	result = gtk_dialog_run (GTK_DIALOG (prefs));
	grisbi_prefs_dialog_response (GTK_DIALOG (prefs), result);
	w_run->menu_prefs = FALSE;
}

/* HELP MENU */
/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 **/
void grisbi_cmd_manual (GSimpleAction *action,
						GVariant *parameter,
						gpointer app)
{
	gsb_menu_help_manual ();
}

/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 **/
void grisbi_cmd_quick_start (GSimpleAction *action,
						GVariant *parameter,
						gpointer app)
{
	gsb_menu_help_quick_start ();
}

/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 **/
void grisbi_cmd_web_site (GSimpleAction *action,
						  GVariant *parameter,
						  gpointer app)
{
	gsb_menu_help_website ();
}

/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 **/
void grisbi_cmd_report_bug (GSimpleAction *action,
							GVariant *parameter,
							gpointer app)
{
	gsb_menu_help_bugreport ();
}

/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 **/
void grisbi_cmd_day_tip (GSimpleAction *action,
						 GVariant *parameter,
						 gpointer app)
{
	force_display_tip ();
}

/* WIN MENU */
/* MENU FICHIER */
/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 * */
void grisbi_cmd_file_new (GSimpleAction *action,
						  GVariant *parameter,
						  gpointer app)
{
    /* continue only if closing the file is ok */
    if (!gsb_file_close ())
        return;

    /* set up all the default variables */
    init_variables ();

    gsb_assistant_file_run (FALSE, FALSE);

	/* on ajoute un item au menu recent_file */
	utils_files_append_name_to_recent_array (grisbi_win_get_filename (NULL));
}

/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 * */
void grisbi_cmd_file_open_menu (GSimpleAction *action,
								GVariant *parameter,
								gpointer app)
{
	devel_debug (NULL);

	if (gsb_file_open_menu ())
	{
		GrisbiWinRun *w_run;

		w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
		utils_files_append_name_to_recent_array (grisbi_win_get_filename (NULL));
		if (!w_run->file_is_loading)
		{
			gsb_gui_navigation_select_line (NULL, NULL);
			w_run->file_is_loading = TRUE;
		}
	}
}

/**
 * ouvre un fichier récent
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 * */
void grisbi_cmd_file_open_direct_menu (GSimpleAction *action,
									   GVariant *parameter,
									   gpointer app)
{
	gchar **recent_array;
	const gchar *target;
	gchar *tmp_str;
    gint index_target = 0;
	GrisbiWinRun *w_run;

	devel_debug (NULL);
	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();

	/* on sort si un fichier est déjà en cours de chargement */
	if (grisbi_win_status_bar_get_wait_state ())
		return;

	/* continue only if can close the current file */
    if (w_run->file_is_loading && !gsb_file_close ())
		return;

	target = g_variant_get_string (parameter, NULL);
    index_target = atoi (target);

	recent_array = grisbi_app_get_recent_files_array ();
	tmp_str = g_strdup (recent_array[index_target-1]);

	if (tmp_str)
	{
		if (gsb_file_open_file (tmp_str))
		{
			if (!w_run->file_is_loading)
			{
				gsb_gui_navigation_select_line (NULL, NULL);
				w_run->file_is_loading = TRUE;
			}
			utils_files_append_name_to_recent_array (tmp_str);
		}
		g_free (tmp_str);
	}
}

/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 * */
void grisbi_cmd_file_save (GSimpleAction *action,
						   GVariant *parameter,
						   gpointer app)
{
	GrisbiWinRun *w_run;

	/* disable quit. See bug #1969 */
	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
	w_run->menu_save = TRUE;
	gsb_file_save ();
	w_run->menu_save = FALSE;
}

/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 * */
void grisbi_cmd_file_save_as (GSimpleAction *action,
							  GVariant *parameter,
							  gpointer app)
{
	GrisbiWinRun *w_run;

	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
	w_run->menu_save = TRUE;
	gsb_file_save_as ();
	w_run->menu_save = FALSE;
}

/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 * */
void grisbi_cmd_importer_fichier (GSimpleAction *action,
								  GVariant *parameter,
								  gpointer app)
{
	gsb_import_assistant_importer_fichier ();
}

/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 * */
void grisbi_cmd_export_accounts (GSimpleAction *action,
								 GVariant *parameter,
								 gpointer app)
{
	export_accounts ();
}

/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 * */
void grisbi_cmd_create_archive (GSimpleAction *action,
								GVariant *parameter,
								gpointer app)
{
	gsb_assistant_archive_run_by_menu ();
}

/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 * */
void grisbi_cmd_export_archive (GSimpleAction *action,
								GVariant *parameter,
								gpointer app)
{
	gsb_assistant_archive_export_run ();
}

/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 * */
void grisbi_cmd_debug_acc_file (GSimpleAction *action,
								GVariant *parameter,
								gpointer app)
{
	gsb_debug ();
}

/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 * */
void grisbi_cmd_obf_acc_file (GSimpleAction *action,
							  GVariant *parameter,
							  gpointer app)
{
	file_obfuscate_run ();
}

/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 * */
void grisbi_cmd_obf_qif_file (GSimpleAction *action,
							  GVariant *parameter,
							  gpointer app)
{
	file_obfuscate_qif_run ();
}

/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 * */
void grisbi_cmd_debug_mode_toggle (GSimpleAction *action,
								   GVariant *parameter,
								   gpointer app)
{
	GVariant *val;
	gboolean state;

	val = g_action_get_state (G_ACTION (action));
	if (val)
	{
		state = g_variant_get_boolean (val);
		g_variant_unref (val);
		if (state == FALSE)
		{
			val = g_variant_new_boolean (TRUE);
			g_action_change_state (G_ACTION (action), val);
			g_simple_action_set_enabled (action, FALSE);
			debug_start_log ();
		}
	}
}

/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 * */
void grisbi_cmd_file_close (GSimpleAction *action,
							GVariant *parameter,
							gpointer app)
{
	GrisbiWin *win;

	if (gsb_file_close ())
	{
		win = grisbi_app_get_active_window (GRISBI_APP (app));
		grisbi_win_no_file_page_update (win);
		grisbi_win_status_bar_message (_("Start an action!"));
	}
}

/* EDIT MENU */
/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 * */
void grisbi_cmd_edit_ope (GSimpleAction *action,
						  GVariant *parameter,
						  gpointer app)
{
	gsb_data_mix_edit_current_transaction ();
}

/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 * */
void grisbi_cmd_new_ope (GSimpleAction *action,
						 GVariant *parameter,
						 gpointer app)
{
	gsb_data_mix_new_transaction_by_menu ();
}

/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 * */
void grisbi_cmd_remove_ope (GSimpleAction *action,
							GVariant *parameter,
							gpointer app)
{
	gsb_data_mix_delete_current_transaction ();
}

/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 * */
void grisbi_cmd_template_ope (GSimpleAction *action,
							  GVariant *parameter,
							  gpointer app)
{
	gsb_transactions_list_clone_template (NULL, NULL);
}

/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 * */
void grisbi_cmd_clone_ope (GSimpleAction *action,
						   GVariant *parameter,
						   gpointer app)
{
	gsb_data_mix_clone_current_transaction ();
}

/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 * */
void grisbi_cmd_convert_ope (GSimpleAction *action,
						GVariant *parameter,
						gpointer app)
{
	gsb_transactions_list_convert_transaction_to_sheduled ();
}

/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 * */
void grisbi_cmd_move_to_account_menu (GSimpleAction *action,
									  GVariant *parameter,
									  gpointer app)
{
	const gchar *target;
	gchar *tmp_str;
    gint target_account = 0;
	gint source_account;

	target = g_action_get_name (G_ACTION (action));
	tmp_str = gsb_string_extract_int (target);
	if (tmp_str && strlen ( tmp_str ) > 0)
	{
		target_account = atoi (tmp_str);

		source_account = gsb_gui_navigation_get_current_account ();
		gsb_transactions_list_move_transaction_to_account_from_menu (source_account, target_account);
	}
	g_free (tmp_str);
}

/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 * */
void grisbi_cmd_new_acc (GSimpleAction *action,
						 GVariant *parameter,
						 gpointer app)
{
	gsb_assistant_account_run ();
}

/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 * */
void grisbi_cmd_remove_acc (GSimpleAction *action,
							GVariant *parameter,
							gpointer app)
{
	gsb_account_delete ();
}

/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 * */
void grisbi_cmd_search_acc (GSimpleAction *action,
							GVariant *parameter,
							gpointer app)
{
	gint account_number;
	gint page_number;
	gint transaction_number;

	devel_debug (NULL);
	page_number = gsb_gui_navigation_get_current_page ();
	switch (page_number)
	{
		case GSB_ACCOUNT_PAGE:
			account_number = gsb_gui_navigation_get_current_account ();
			transaction_number = gsb_data_account_get_current_transaction_number (account_number);
			gsb_transactions_list_search (NULL, GINT_TO_POINTER (transaction_number));
		break;

		case GSB_REPORTS_PAGE:
			etats_onglet_create_search_report ();
		break;

		case GSB_CATEGORIES_PAGE:
		case GSB_BUDGETARY_LINES_PAGE:
		case GSB_PAYEES_PAGE:
			gsb_gui_navigation_create_search_report_from_ctrl_f (page_number);
		break;

		default:
			gsb_gui_navigation_select_reports_page ();
			etats_onglet_create_search_report ();
	}
}

/* VIEW MENU */
/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 * */
void grisbi_cmd_show_form_toggle (GSimpleAction *action,
								  GVariant *parameter,
								  gpointer app)
{
	GrisbiAppConf *a_conf;

	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
	grisbi_win_switch_form_expander ();
    a_conf->formulaire_toujours_affiche = !a_conf->formulaire_toujours_affiche;

    g_action_change_state (G_ACTION (action), g_variant_new_boolean (a_conf->formulaire_toujours_affiche));
}

/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 * */
void grisbi_cmd_show_reconciled_toggle (GSimpleAction *action,
										GVariant *state,
										gpointer app)
{
	gint current_account;
	GrisbiWinRun *w_run;

		w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
		current_account = gsb_gui_navigation_get_current_account ();
	if (current_account == -1 || w_run->equilibrage == TRUE)
		return;

	if (gsb_data_account_get_r (current_account))
	{
		gsb_transactions_list_mise_a_jour_affichage_r (FALSE);
		g_action_change_state (G_ACTION (action), g_variant_new_boolean (FALSE));
	}
	else
	{
		gsb_transactions_list_mise_a_jour_affichage_r (TRUE);
		g_action_change_state (G_ACTION (action), g_variant_new_boolean (TRUE));
	}
}

/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 * */
void grisbi_cmd_show_archived_toggle (GSimpleAction *action,
									  GVariant *state,
									  gpointer app)
{
    gint current_account;

    current_account = gsb_gui_navigation_get_current_account ();
    if (current_account == -1)
        return;

    if (gsb_data_account_get_l (current_account))
    {
	    gsb_transactions_list_show_archives_lines (FALSE);
        g_action_change_state (G_ACTION (action), g_variant_new_boolean (FALSE));
    }
    else
    {
	    gsb_transactions_list_show_archives_lines (TRUE);
        g_action_change_state (G_ACTION (action), g_variant_new_boolean (TRUE));
    }
}

/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 * */
void grisbi_cmd_show_closed_acc_toggle (GSimpleAction *action,
										GVariant *state,
										gpointer app)
{
	GrisbiAppConf *a_conf;

	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
	a_conf->show_closed_accounts = !a_conf->show_closed_accounts;

    gsb_gui_navigation_create_account_list (gsb_gui_navigation_get_model ());
    gsb_gui_navigation_update_home_page ();
    g_action_change_state (G_ACTION (action), g_variant_new_boolean (a_conf->show_closed_accounts));

    gsb_file_set_modified (TRUE);
}

/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 * */
void grisbi_cmd_show_ope_radio (GSimpleAction *action,
								GVariant *parameter,
								gpointer app)
{
	const gchar *target;
    gint index_target = 0;

	target = g_variant_get_string (parameter, NULL);
    index_target = atoi (target);
    gsb_transactions_list_set_visible_rows_number (index_target);

    g_action_change_state (G_ACTION (action), parameter);
}

/**
 *
 *
 * \param GSimpleAction 	action
 * \param GVariant 			parameter
 * \param gpointer 			app
 *
 * \return
 * */
void grisbi_cmd_reset_width_col (GSimpleAction *action,
								 GVariant *state,
								 gpointer app)
{
	gsb_menu_reinit_largeur_col_menu ();
    gsb_file_set_modified (TRUE);
}

/* CALLBACK VIEW MENUS */
/**
 * Show or hide display of reconciled transactions.
 *
 * \return FALSE
 */
void gsb_menu_gui_toggle_show_reconciled (void)
{
    gint current_account;
	GrisbiWinRun *w_run;

	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
    current_account = gsb_gui_navigation_get_current_account ();
    if (current_account == -1 || w_run->equilibrage == TRUE)
        return;

    if (gsb_data_account_get_r (current_account))
	    gsb_transactions_list_change_aspect_liste (6);
    else
	    gsb_transactions_list_change_aspect_liste (5);
}


/**
 * Show or hide display of lines archives.
 *
 * \return FALSE
 */
void gsb_menu_gui_toggle_show_archived (void)
{
    gint current_account;

    current_account = gsb_gui_navigation_get_current_account ();
    if (current_account == -1)
        return;

    if (gsb_data_account_get_l (current_account))
	    gsb_transactions_list_change_aspect_liste (8);
    else
	    gsb_transactions_list_change_aspect_liste (7);
}


/**
 * Met à jour
 *
 * \return FALSE
 */
void gsb_menu_gui_toggle_show_form (void)
{
    GrisbiWin *win;
    GAction *action;
	GrisbiAppConf *a_conf;

    win = grisbi_app_get_active_window (NULL);
    action = g_action_map_lookup_action (G_ACTION_MAP (win), "show-form");
	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
    g_action_change_state (G_ACTION (action),
                           g_variant_new_boolean (a_conf->formulaire_toujours_affiche));
}

/**
 * Update the view menu in the menu bar
 *
 * \param account_number	The account used to update the menu
 *
 * \return FALSE
 * */
void gsb_menu_update_view_menu (gint account_number)
{
    GrisbiWin *win;
    GAction *action;
    GVariant *parameter;

    win = grisbi_app_get_active_window (NULL);

    /* update the showing of reconciled transactions */
    action = g_action_map_lookup_action (G_ACTION_MAP (win), "show-reconciled");
    g_action_change_state (G_ACTION (action),
                           g_variant_new_boolean (gsb_data_account_get_r (account_number)));

    /* update the showing of archived transactions */
    action = g_action_map_lookup_action (G_ACTION_MAP (win), "show-archived");
    g_action_change_state (G_ACTION (action),
                           g_variant_new_boolean (gsb_data_account_get_l (account_number)));

    /* update the number of line showed */
    switch (gsb_data_account_get_nb_rows (account_number))
    {
	default:
	case 1 :
        parameter = g_variant_new_string ("1");
	    break;
	case 2 :
        parameter = g_variant_new_string ("2");
	    break;
	case 3 :
        parameter = g_variant_new_string ("3");
	    break;
	case 4 :
        parameter = g_variant_new_string ("4");
	    break;
    }

    action = g_action_map_lookup_action (G_ACTION_MAP (win), "show-ope");
    g_action_change_state (G_ACTION (action), parameter);
}

/* SENSITIVE MENUS */
/**
 * sensitive a menu defined by an action
 *
 * \param item_name		name of action.
 * \param state			Whether menu should be 'sensitive' or not.
 *
 * \return
 */
void gsb_menu_gui_sensitive_win_menu_item (const gchar *item_name,
										   gboolean state)
{
    GrisbiWin *win;
    GAction *action;

    win = grisbi_app_get_active_window (NULL);
    action = g_action_map_lookup_action (G_ACTION_MAP (win), item_name);
    g_simple_action_set_enabled (G_SIMPLE_ACTION (action), state);
}

/**
 * Set sensitiveness of all menu items that work on the selected transaction.

 * \param sensitive	Sensitiveness (FALSE for unsensitive, TRUE for
 *			sensitive).
 *
 * \return		FALSE
 */
void gsb_menu_set_menus_select_transaction_sensitive (gboolean sensitive)
{
    static gboolean flag_sensitive = FALSE;
    const gchar * items[] = {
        "edit-ope",
        "new-ope",
        "remove-ope",
        "template-ope",
        "clone-ope",
        "convert-ope",
        "remove-acc",
        NULL
    };
    const gchar **tmp = items;

    if (flag_sensitive == sensitive)
        return;

    while (*tmp)
    {
        gsb_menu_gui_sensitive_win_menu_item (*tmp, sensitive);
        tmp++;
    }
    flag_sensitive = sensitive;
}


/**
 * Set sensitiveness of all menu items that work on the selected scheduled.

 * \param sensitive	Sensitiveness (FALSE for unsensitive, TRUE for
 *			sensitive).
 *
 * \return		FALSE
 */
void gsb_menu_set_menus_select_scheduled_sensitive (gboolean sensitive)
{
    static gboolean flag_sensitive = FALSE;
    const gchar * items[] = {
        "edit-ope",
        "remove-ope",
        "clone-ope",
        NULL
    };
    const gchar **tmp = items;

    if (flag_sensitive == sensitive)
        return;

    while (*tmp)
    {
        gsb_menu_gui_sensitive_win_menu_item (*tmp, sensitive);
        tmp++;
    }

    flag_sensitive = sensitive;
}

/**
 * Initialise la barre de menus en fonction de la présence ou non d'un fichier de comptes
 *
 * \param sensitif
 *
 * \return
 * */
void gsb_menu_set_menus_with_file_sensitive (gboolean sensitive)
{
    const gchar * items[] = {
        "save-as",
        "export-accounts",
        "create-archive",
        "export-archive",
        "debug-acc-file",
        "obf-acc-file",
        "debug-mode",
        "file-close",
        "new-acc",
        "show-closed-acc",
        NULL
    };
    const gchar **tmp = items;

    if (sensitive)
    {
        grisbi_win_menu_move_to_acc_new ();
    }

    while (*tmp)
    {
        gsb_menu_gui_sensitive_win_menu_item (*tmp, sensitive);
        tmp++;
    }
}

/**
 * Initialise la barre de menus si un compte est sélectionné
 *
 * \param sensitif
 *
 * \return
 * */
void gsb_menu_set_menus_view_account_sensitive (gboolean sensitive)
{
    const gchar * items[] = {
        "remove-acc",
        "show-form",
        "show-reconciled",
        "show-archived",
        "show-ope",
        "reset-width-col",
        NULL
    };
    const gchar **tmp = items;

    while (*tmp)
    {
        gsb_menu_gui_sensitive_win_menu_item (*tmp, sensitive);
        tmp++;
    }
}

/**
 *
 *
 * \param
 *
 * \return
 * */
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
