/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2004-2009 Benjamin Drieu (bdrieu@april.org)                       */
/*          2009-2017 Pierre Biava (grisbi@pierre.biava.name)                 */
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
#include "config.h"
#endif

#include "include.h"
#include <glib/gi18n.h>
#include <stdlib.h>

/*START_INCLUDE*/
#include "menu.h"
#include "custom_list.h"
#include "export.h"
#include "fenetre_principale.h"
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
#include "parametres.h"
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
    lance_navigateur_web ("http://www.grisbi.org/bugsreports/");

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

    string = g_build_filename (gsb_dirs_get_help_dir (), lang, "manual.html", NULL);

    if (g_file_test (string,
		      G_FILE_TEST_EXISTS))
    {
	lance_navigateur_web (string);
	g_free (string);
    }
    else
    {
	g_free (string);
	string = g_build_filename (gsb_dirs_get_help_dir (), lang, "grisbi-manuel.html", NULL);
	lance_navigateur_web (string);
	g_free (string);
    }

    return FALSE;
}

/**
 * Start a browser processus with Grisbi website displayed.
 *
 * \return FALSE
 */
static gboolean gsb_menu_help_website (void)
{
    lance_navigateur_web ("http://www.grisbi.org/");

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
        initialise_largeur_colonnes_tab_affichage_ope (GSB_ACCOUNT_PAGE, NULL);

        gsb_transactions_list_set_largeur_col ();
    }
    else if (current_page == GSB_SCHEDULER_PAGE)
    {
        initialise_largeur_colonnes_tab_affichage_ope (GSB_SCHEDULER_PAGE, NULL);

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
	GrisbiPrefs *prefs;
	GrisbiWin *win;
	gint result;

	win = grisbi_app_get_active_window (app);
	prefs = grisbi_prefs_new (win);
	grisbi_win_set_prefs_dialog (win, GTK_WIDGET (prefs));

	gtk_window_present (GTK_WINDOW (prefs));
	gtk_widget_show_all (GTK_WIDGET (prefs));
	result = gtk_dialog_run (GTK_DIALOG (prefs));
	grisbi_prefs_dialog_response (GTK_DIALOG (prefs), result);
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
		utils_files_append_name_to_recent_array (grisbi_win_get_filename (NULL));
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

	devel_debug (NULL);

    /* continue only if can close the current file */
    if (!gsb_file_close ())
		return;

	target = g_variant_get_string (parameter, NULL);
    index_target = atoi (target);

	recent_array = grisbi_app_get_recent_files_array ();
	tmp_str = g_strdup (recent_array[index_target-1]);

	if (tmp_str)
	{
		if (gsb_file_open_file (tmp_str))
			utils_files_append_name_to_recent_array (tmp_str);
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
	gsb_file_save ();
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
	gsb_file_save_as ();
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
			g_variant_unref (val);
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
		grisbi_win_stack_box_show (win, "accueil_page");
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
	schedule_selected_transaction ();
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

	devel_debug (NULL);

	target = g_action_get_name (G_ACTION (action));
	tmp_str = gsb_string_extract_int (target);
	if (tmp_str && strlen ( tmp_str ) > 0)
	{
		target_account = atoi (tmp_str);

		source_account = gsb_gui_navigation_get_current_account ();
		move_selected_operation_to_account_nb (source_account, target_account);
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
    devel_debug (NULL);

    gsb_form_switch_expander ();
    conf.formulaire_toujours_affiche = !conf.formulaire_toujours_affiche;

    g_action_change_state (G_ACTION (action), g_variant_new_boolean (conf.formulaire_toujours_affiche));
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

    current_account = gsb_gui_navigation_get_current_account ();
    if (current_account == -1 || run.equilibrage == 1)
        return;

    if (gsb_data_account_get_r (current_account))
    {
	    mise_a_jour_affichage_r (FALSE);
        g_action_change_state (G_ACTION (action), g_variant_new_boolean (FALSE));
    }
    else
    {
	    mise_a_jour_affichage_r (TRUE);
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
    conf.show_closed_accounts = !conf.show_closed_accounts;

    gsb_gui_navigation_create_account_list (gsb_gui_navigation_get_model ());
    gsb_gui_navigation_update_home_page ();
    g_action_change_state (G_ACTION (action), g_variant_new_boolean (conf.show_closed_accounts));

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

	devel_debug (NULL);

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
gboolean gsb_menu_gui_toggle_show_reconciled (void)
{
    gint current_account;

    current_account = gsb_gui_navigation_get_current_account ();
    if (current_account == -1 || run.equilibrage == 1)
        return FALSE;

    if (gsb_data_account_get_r (current_account))
	    change_aspect_liste (6);
    else
	    change_aspect_liste (5);

    return FALSE;
}


/**
 * Show or hide display of lines archives.
 *
 * \return FALSE
 */
gboolean gsb_menu_gui_toggle_show_archived (void)
{
    gint current_account;

    current_account = gsb_gui_navigation_get_current_account ();
    if (current_account == -1)
        return FALSE;

    if (gsb_data_account_get_l (current_account))
	    change_aspect_liste (8);
    else
	    change_aspect_liste (7);

    return FALSE;
}


/**
 * Met à jour
 *
 * \return FALSE
 */
gboolean gsb_menu_gui_toggle_show_form (void)
{
    GrisbiWin *win;
    GAction *action;

    win = grisbi_app_get_active_window (NULL);
    action = g_action_map_lookup_action (G_ACTION_MAP (win), "show-form");
    g_action_change_state (G_ACTION (action),
                           g_variant_new_boolean (conf.formulaire_toujours_affiche));

    return FALSE;
}

/**
 * Update the view menu in the menu bar
 *
 * \param account_number	The account used to update the menu
 *
 * \return FALSE
 * */
gboolean gsb_menu_update_view_menu (gint account_number)
{
    GrisbiWin *win;
    GAction *action;
    GVariant *parameter;

    devel_debug_int (account_number);

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

    /* return value*/
    return FALSE;
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
void gsb_menu_gui_sensitive_win_menu_item (gchar *item_name,
										   gboolean state)
{
    GrisbiWin *win;
    GAction *action;

    //~ printf ("gsb_menu_gui_sensitive_win_menu_item : \"%s\" sensitive = %d\n", item_name, state);

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
    gchar * items[] = {
        "edit-ope",
        "new-ope",
        "remove-ope",
        "template-ope",
        "clone-ope",
        "convert-ope",
        "remove-acc",
        NULL
    };
    gchar **tmp = items;

    devel_debug_int (sensitive);

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
    gchar * items[] = {
        "edit-ope",
        "remove-ope",
        "clone-ope",
        NULL
    };
    gchar **tmp = items;

    devel_debug_int (sensitive);

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
    gchar * items[] = {
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
    gchar **tmp = items;

    devel_debug_int (sensitive);

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
    static gboolean flag_sensitive = FALSE;
    gchar * items[] = {
        "remove-acc",
        "show-form",
        "show-reconciled",
        "show-archived",
        "show-ope",
        "reset-width-col",
        NULL
    };
    gchar **tmp = items;

    devel_debug_int (sensitive);

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
 *
 *
 * \param
 *
 * \return
 * */
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
