/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2004-2009 Benjamin Drieu (bdrieu@april.org)                       */
/*          2009-2016 Pierre Biava (grisbi@pierre.biava.name)                 */
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
#include <config.h>
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
#include "grisbi_win.h"
#include "gsb_account.h"
#include "gsb_assistant_account.h"
#include "gsb_assistant_archive.h"
#include "gsb_assistant_archive_export.h"
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
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/


/* fonctions statiques */
/**
 * Show or hide closed accounts.
 *
 * \return FALSE
 */
static gboolean gsb_gui_toggle_show_closed_accounts ( void )
{
    conf.show_closed_accounts = !conf.show_closed_accounts;

    gsb_gui_navigation_create_account_list ( gsb_gui_navigation_get_model ( ) );
    gsb_gui_navigation_update_home_page ( );

    gsb_file_set_modified ( TRUE );

    return FALSE;
}

/**
 * Start a browser processus with Grisbi bug report page displayed.
 *
 * \return FALSE
 */
static gboolean gsb_menu_help_bugreport ( void )
{
    lance_navigateur_web ( "http://www.grisbi.org/bugsreports/" );

    return FALSE;
}

/**
 * Start a browser processus with local copy of the quick start page
 * on command line.
 *
 * \return FALSE
 */
static gboolean gsb_menu_help_quick_start ( void )
{
    gchar *lang = _("en");

    gchar* tmpstr = g_build_filename ( HELP_PATH, lang, "quickstart.html", NULL );
    lance_navigateur_web ( tmpstr );
    g_free ( tmpstr );

    return FALSE;
}

/**
 * Start a browser processus with local copy of manual on command
 * line.
 *
 * \return FALSE
 */
static gboolean gsb_menu_help_manual ( void )
{
    gchar *lang = _("en");
    gchar *string;

    string = g_build_filename ( gsb_dirs_get_help_dir (), lang, "manual.html", NULL );

    if (g_file_test ( string,
		      G_FILE_TEST_EXISTS ))
    {
	lance_navigateur_web (string);
	g_free (string);
    }
    else
    {
	g_free (string);
	string = g_build_filename ( gsb_dirs_get_help_dir (), lang, "grisbi-manuel.html", NULL );
	lance_navigateur_web (string);
	g_free (string);
    }

    return FALSE;
}

/**
 * Start a browser processus with local copy of the translation page
 * on command line.
 *
 * \return FALSE
 */
static gboolean gsb_menu_help_translation ( void )
{
    gchar *lang = _("en");

    gchar* tmpstr = g_build_filename ( HELP_PATH, lang, "translation.html", NULL );
    lance_navigateur_web ( tmpstr );
    g_free ( tmpstr );

    return FALSE;
}

/**
 * Start a browser processus with Grisbi website displayed.
 *
 * \return FALSE
 */
static gboolean gsb_menu_help_website ( void )
{
    lance_navigateur_web ( "http://www.grisbi.org/" );

    return TRUE;
}

/**
 * Réinitialise la largeur des colonnes de la vue des opérations
 *
 * \param
 *
 * \return  FALSE
 * */
static gboolean gsb_menu_reinit_largeur_col_menu ( void )
{
    gint current_page;

    current_page = gsb_gui_navigation_get_current_page ( );

    if ( current_page == GSB_ACCOUNT_PAGE )
    {
        initialise_largeur_colonnes_tab_affichage_ope ( GSB_ACCOUNT_PAGE, NULL );

        gsb_transactions_list_set_largeur_col ( );
    }
    else if ( current_page == GSB_SCHEDULER_PAGE )
    {
        initialise_largeur_colonnes_tab_affichage_ope ( GSB_SCHEDULER_PAGE, NULL );

        gsb_scheduler_list_set_largeur_col ( );
    }

    return FALSE;
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
void grisbi_cmd_new_window ( GSimpleAction *action,
                        GVariant *parameter,
                        gpointer app )
{
	;
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
void grisbi_cmd_about ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app )
{
	a_propos ( NULL, 0 );
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
void grisbi_cmd_quit ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app )
{
	g_application_quit ( G_APPLICATION ( app ) );
}


/* PREFS MENU */
void grisbi_cmd_prefs ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app )
{
	preferences ( -1 );
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
void grisbi_cmd_manual ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app )
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
void grisbi_cmd_quick_start ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app )
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
void grisbi_cmd_web_site ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app )
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
void grisbi_cmd_report_bug ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app )
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
void grisbi_cmd_day_tip ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app )
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
void grisbi_cmd_file_new ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app )
{
	 gsb_file_new ();
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
void grisbi_cmd_file_open_menu ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app )
{
	devel_debug (NULL);

	if ( gsb_file_open_menu () )
    {
        GtkRecentManager *recent_manager;
        GList *tmp_list;
        const gchar *uri;
        const gchar *filename;
        gchar *tmp_uri;
        gint index_gsb = 0;
        gboolean trouve = FALSE;

        conf.nb_derniers_fichiers_ouverts++;
        if ( conf.nb_derniers_fichiers_ouverts > conf.nb_max_derniers_fichiers_ouverts )
            conf.nb_derniers_fichiers_ouverts = conf.nb_max_derniers_fichiers_ouverts;

        filename = grisbi_win_get_filename ( NULL );
        tmp_uri = g_filename_to_uri ( filename, NULL, NULL );

        recent_manager = gtk_recent_manager_get_default ();
        tmp_list = gtk_recent_manager_get_items ( recent_manager );
        while ( tmp_list )
        {
            GtkRecentInfo *info;

            info = tmp_list->data;
            uri = gtk_recent_info_get_uri ( info );
            if ( g_str_has_suffix ( uri, ".gsb" ) )
            {
                index_gsb++;

                if ( g_strcmp0 ( uri, tmp_uri ) == 0 )
                {
                    if ( index_gsb > 1 )
                    {
                        gtk_recent_manager_remove_item ( recent_manager, uri, NULL );
                        gtk_recent_manager_add_item ( recent_manager, tmp_uri );
                    }
                    trouve = TRUE;
                    break;
                }
            }
            tmp_list = tmp_list->next;
        }
        if ( trouve == FALSE )
            gtk_recent_manager_add_item ( recent_manager, tmp_uri );

        g_free ( tmp_uri );
        g_list_free_full ( tmp_list, ( GDestroyNotify ) gtk_recent_info_unref );
        grisbi_app_set_recent_files_menu ( NULL, TRUE );
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
void grisbi_cmd_file_open_direct_menu ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app )
{
    GtkRecentManager *recent_manager;
    GList *tmp_list;
	const gchar *target;
	const gchar *uri;
	gchar *tmp_str;
    gint index_target = 0;
    gint index_gsb = 0;

	devel_debug (NULL);

    /* continue only if can close the current file */
    if ( !gsb_file_close () )
        return;

	target = g_variant_get_string ( parameter, NULL );
    index_target = atoi ( target );

    recent_manager = gtk_recent_manager_get_default ();
    tmp_list = gtk_recent_manager_get_items ( recent_manager );
    while ( tmp_list )
    {
        GtkRecentInfo *info;

        info = tmp_list->data;
        uri = gtk_recent_info_get_uri ( info );
        if ( g_str_has_suffix ( uri, ".gsb" ) )
        {
            index_gsb++;
            if ( index_gsb == index_target )
                break;
        }
        tmp_list = tmp_list->next;
    }
	tmp_str = g_filename_from_uri ( uri, NULL, NULL );

    /* on supprime l'item de la liste. On le mettra en premier si le fichier a été ouvert*/
    gtk_recent_manager_remove_item ( recent_manager, uri, NULL );

    if ( tmp_str )
	{
		if ( gsb_file_open_file ( tmp_str ) )
            gtk_recent_manager_add_item ( recent_manager, uri );
		g_free ( tmp_str );
	}
    g_list_free_full ( tmp_list, ( GDestroyNotify ) gtk_recent_info_unref );

    grisbi_app_set_recent_files_menu ( NULL, TRUE );
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
void grisbi_cmd_file_save ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app )
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
void grisbi_cmd_file_save_as ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app )
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
void grisbi_cmd_importer_fichier ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app )
{
	importer_fichier ();
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
void grisbi_cmd_export_accounts ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app )
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
void grisbi_cmd_create_archive ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app )
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
void grisbi_cmd_export_archive ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app )
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
void grisbi_cmd_debug_acc_file ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app )
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
void grisbi_cmd_obf_acc_file ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app )
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
void grisbi_cmd_obf_qif_file ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app )
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
void grisbi_cmd_debug_mode_toggle ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app )
{
	GVariant *val;
	gboolean state;

	val = g_action_get_state ( G_ACTION ( action ) );
	if ( val )
	{
		state = g_variant_get_boolean ( val );
		g_variant_unref ( val );
		if ( state == FALSE )
		{
			val = g_variant_new_boolean ( TRUE );
			g_action_change_state ( G_ACTION ( action ), val );
			g_simple_action_set_enabled ( action, FALSE );
			g_variant_unref ( val );
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
void grisbi_cmd_file_close ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app )
{
	gsb_file_close ();
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
void grisbi_cmd_edit_ope ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app )
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
void grisbi_cmd_new_ope ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app )
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
void grisbi_cmd_remove_ope ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app )
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
void grisbi_cmd_template_ope ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app )
{
	gsb_transactions_list_clone_template ( NULL, NULL );
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
void grisbi_cmd_clone_ope ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app )
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
void grisbi_cmd_convert_ope ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app )
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
void grisbi_cmd_move_to_account_menu ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app )
{
	const gchar *target;
    gint index_target = 0;

	devel_debug (NULL);

/*	target = g_variant_get_string ( parameter, NULL );
    index_target = atoi ( target );
*/    printf ("grisbi_cmd_move_to_account_menu\n");
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
void grisbi_cmd_new_acc ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app )
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
void grisbi_cmd_remove_acc ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app )
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
void grisbi_cmd_show_form_toggle ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app )
{
    devel_debug (NULL);

    gsb_form_switch_expander ();
    conf.formulaire_toujours_affiche = !conf.formulaire_toujours_affiche;

    g_action_change_state ( G_ACTION ( action ), g_variant_new_boolean ( conf.formulaire_toujours_affiche ) );
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
void grisbi_cmd_show_reconciled_toggle ( GSimpleAction *action,
						GVariant *state,
						gpointer app )
{
    gint current_account;

    current_account = gsb_gui_navigation_get_current_account ( );
    if ( current_account == -1 || run.equilibrage == 1 )
        return;

    if ( gsb_data_account_get_r ( current_account ) )
    {
	    mise_a_jour_affichage_r ( FALSE );
        g_action_change_state ( G_ACTION ( action ), g_variant_new_boolean ( FALSE ) );
    }
    else
    {
	    mise_a_jour_affichage_r ( TRUE );
        g_action_change_state ( G_ACTION ( action ), g_variant_new_boolean ( TRUE ) );
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
void grisbi_cmd_show_archived_toggle ( GSimpleAction *action,
						GVariant *state,
						gpointer app )
{
    gint current_account;

    current_account = gsb_gui_navigation_get_current_account ( );
    if ( current_account == -1 )
        return;

    if ( gsb_data_account_get_l ( current_account ) )
    {
	    gsb_transactions_list_show_archives_lines ( FALSE );
        g_action_change_state ( G_ACTION ( action ), g_variant_new_boolean ( FALSE ) );
    }
    else
    {
	    gsb_transactions_list_show_archives_lines ( TRUE );
        g_action_change_state ( G_ACTION ( action ), g_variant_new_boolean ( TRUE ) );
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
void grisbi_cmd_show_closed_acc_toggle ( GSimpleAction *action,
						GVariant *state,
						gpointer app )
{
    conf.show_closed_accounts = !conf.show_closed_accounts;

    gsb_gui_navigation_create_account_list ( gsb_gui_navigation_get_model ( ) );
    gsb_gui_navigation_update_home_page ( );
    g_action_change_state ( G_ACTION ( action ), g_variant_new_boolean ( conf.show_closed_accounts ) );

    gsb_file_set_modified ( TRUE );
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
void grisbi_cmd_show_ope_radio ( GSimpleAction *action,
						GVariant *parameter,
						gpointer app )
{
	const gchar *target;
    gint index_target = 0;

	devel_debug (NULL);

	target = g_variant_get_string ( parameter, NULL );
    index_target = atoi ( target );
    gsb_transactions_list_set_visible_rows_number ( index_target );

    g_action_change_state ( G_ACTION ( action ), parameter );
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
void grisbi_cmd_reset_width_col ( GSimpleAction *action,
						GVariant *state,
						gpointer app )
{
	gsb_menu_reinit_largeur_col_menu ();
        gsb_file_set_modified ( TRUE );
}

/* RECENT FILES MANAGER */
/**
 * limits the number of gsb files in recent manager
 *
 * \param GtkRecentManager	recent_manager
 *
 * \return
 * */
void gsb_menu_recent_manager_purge_gsb_items ( GtkRecentManager *recent_manager )
{
	GList *tmp_list;
	gint index = 0;

	tmp_list = gtk_recent_manager_get_items ( recent_manager );

	while ( tmp_list )
    {
		GtkRecentInfo *info;
		const gchar *uri;

		info = tmp_list->data;

		uri = gtk_recent_info_get_uri ( info );

		if ( g_str_has_suffix ( uri, ".gsb" ) )
		{
			index++;
			if ( index > conf.nb_max_derniers_fichiers_ouverts )
			{
				gtk_recent_manager_remove_item ( recent_manager, uri, NULL );
			}
		}
		gtk_recent_info_unref ( info );

		tmp_list = tmp_list->next;
    }
	g_list_free ( tmp_list );
}

/**
 * remove an item of an recent_manager
 *
 * \param GtkRecentManager	recent_manager
 * \param const gchar 		path of gsb_file
 *
 * \return
 * */
void gsb_menu_recent_manager_remove_item ( GtkRecentManager *recent_manager,
						const gchar *path )
{
	gchar *uri;
	GError *error = NULL;

	uri = g_filename_to_uri ( path, NULL, &error );
	if ( error )
    {
		g_warning ( "Could not convert uri \"%s\" to a local path: %s", uri, error->message );
		g_error_free ( error );

		return;
    }

	if ( recent_manager )
		gtk_recent_manager_remove_item ( recent_manager, uri, &error );
	else
	{
		GtkRecentManager *tmp_recent_manager;

		tmp_recent_manager = gtk_recent_manager_get_default ();
		gtk_recent_manager_remove_item ( tmp_recent_manager, uri, &error );
	}

	if ( error )
	{
		g_warning ( "Could not remove recent-files uri \"%s\": %s", uri, error->message );
		g_error_free ( error );
	}

  g_free ( uri );
}


/* CALLBACK VIEW MENUS */
/**
 * Show or hide display of reconciled transactions.
 *
 * \return FALSE
 */
gboolean gsb_menu_gui_toggle_show_reconciled ( void )
{
    gint current_account;

    current_account = gsb_gui_navigation_get_current_account ( );
    if ( current_account == -1 || run.equilibrage == 1 )
        return FALSE;

    if ( gsb_data_account_get_r ( current_account ) )
	    change_aspect_liste ( 6 );
    else
	    change_aspect_liste ( 5 );

    return FALSE;
}


/**
 * Show or hide display of lines archives.
 *
 * \return FALSE
 */
gboolean gsb_menu_gui_toggle_show_archived ( void )
{
    gint current_account;

    current_account = gsb_gui_navigation_get_current_account ( );
    if ( current_account == -1 )
        return FALSE;

    if ( gsb_data_account_get_l ( current_account ) )
	    change_aspect_liste ( 8 );
    else
	    change_aspect_liste ( 7 );

    return FALSE;
}


/**
 * Met à jour
 *
 * \return FALSE
 */
gboolean gsb_menu_gui_toggle_show_form ( void )
{
    GrisbiWin *win;
    GAction *action;

    win = grisbi_app_get_active_window ( NULL );
    action = g_action_map_lookup_action ( G_ACTION_MAP ( win ), "show-form" );
    g_action_change_state ( G_ACTION ( action ),
                           g_variant_new_boolean ( conf.formulaire_toujours_affiche ) );

    return FALSE;
}

/**
 * Update the view menu in the menu bar
 *
 * \param account_number	The account used to update the menu
 *
 * \return FALSE
 * */
gboolean gsb_menu_update_view_menu ( gint account_number )
{
    GrisbiWin *win;
    GAction *action;
    GVariant *parameter;

    devel_debug_int (account_number);

    win = grisbi_app_get_active_window ( NULL );

    /* update the showing of reconciled transactions */
    action = g_action_map_lookup_action ( G_ACTION_MAP ( win ), "show-reconciled" );
    g_action_change_state ( G_ACTION ( action ),
                           g_variant_new_boolean ( gsb_data_account_get_r (account_number ) ) );

    /* update the showing of archived transactions */
    action = g_action_map_lookup_action ( G_ACTION_MAP ( win ), "show-archived" );
    g_action_change_state ( G_ACTION ( action ),
                           g_variant_new_boolean ( gsb_data_account_get_l (account_number ) ) );

    /* update the number of line showed */
    switch ( gsb_data_account_get_nb_rows ( account_number ) )
    {
	default:
	case 1 :
        parameter = g_variant_new_string ( "1" );
	    break;
	case 2 :
        parameter = g_variant_new_string ( "2" );
	    break;
	case 3 :
        parameter = g_variant_new_string ( "3" );
	    break;
	case 4 :
        parameter = g_variant_new_string ( "4" );
	    break;
    }

    action = g_action_map_lookup_action ( G_ACTION_MAP ( win ), "show-ope" );
    g_action_change_state ( G_ACTION ( action ), parameter );

    /* return value*/
    return FALSE;
}

/**
 * Update the clickable list of closed accounts and target
 * accounts to move a transaction, in menu.
 *
 * \param
 * \return FALSE
 * */
gboolean gsb_menu_update_accounts_in_menus ( void )
{
    GSList *list_tmp;

	return FALSE;

/*    if ( move_to_account_action_group )
    {
        gsb_menu_ui_manager_remove_action_group ( ui_manager,
                        move_to_account_action_group,
                        move_to_account_merge_id );
        g_object_unref ( G_OBJECT ( move_to_account_action_group ) );
        move_to_account_action_group = NULL;
    }

    move_to_account_merge_id = gtk_ui_manager_new_merge_id ( ui_manager );
    move_to_account_action_group = gtk_action_group_new ( "Group3" );

*/    /* create the closed accounts and accounts in the menu to move a transaction */
/*    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
        gint i;

        i = gsb_data_account_get_no_account ( list_tmp -> data );

        if ( !gsb_data_account_get_closed_account ( i ) )
        {
            gchar *tmp_name;
            gchar *account_name;
            GtkAction *action;

            tmp_name = g_strdup_printf ( "MoveToAccount%d", i );
            account_name = gsb_data_account_get_name ( i );
            if ( !account_name )
                account_name = _("Unnamed account");

            action = gtk_action_new ( tmp_name, account_name, "", "" );

            if ( gsb_gui_navigation_get_current_account () == i )
                gtk_action_set_sensitive ( action, FALSE );

            gtk_action_group_add_action ( move_to_account_action_group, action );
            g_signal_connect ( action,
                        "activate",
                        G_CALLBACK ( move_selected_operation_to_account_nb ),
                        GINT_TO_POINTER ( i ) );

            gtk_ui_manager_add_ui ( ui_manager,
                        move_to_account_merge_id,
                        "/menubar/EditMenu/MoveToAnotherAccount/",
                        tmp_name,
                        tmp_name,
                        GTK_UI_MANAGER_MENUITEM,
                        FALSE );
            g_object_unref ( G_OBJECT ( action ) );
            g_free ( tmp_name );
        }

        list_tmp = list_tmp -> next;
    }

    gtk_ui_manager_insert_action_group ( ui_manager, move_to_account_action_group, 2 );
    gtk_ui_manager_ensure_update ( ui_manager );
*/
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
void gsb_menu_gui_sensitive_win_menu_item ( gchar *item_name,
                        gboolean state )
{
    GrisbiWin *win;
    GAction *action;

    //~ printf ("gsb_menu_gui_sensitive_win_menu_item : \"%s\" sensitive = %d\n", item_name, state );

    win = grisbi_app_get_active_window ( NULL );
    action = g_action_map_lookup_action (G_ACTION_MAP ( win ), item_name );
    g_simple_action_set_enabled ( G_SIMPLE_ACTION ( action ), state );
}

/**
 * Set sensitiveness of all menu items that work on the selected transaction.

 * \param sensitive	Sensitiveness (FALSE for unsensitive, TRUE for
 *			sensitive).
 *
 * \return		FALSE
 */
void gsb_menu_set_menus_select_transaction_sensitive ( gboolean sensitive )
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

    if ( flag_sensitive == sensitive )
        return;

    while ( *tmp )
    {
        gsb_menu_gui_sensitive_win_menu_item ( *tmp, sensitive );
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
void gsb_menu_set_menus_select_scheduled_sensitive ( gboolean sensitive )
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

    if ( flag_sensitive == sensitive )
        return;

    while ( *tmp )
    {
        gsb_menu_gui_sensitive_win_menu_item ( *tmp, sensitive );
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
void gsb_menu_set_menus_with_file_sensitive ( gboolean sensitive )
{
    GAction *action;
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
        NULL
    };
    gchar **tmp = items;

    devel_debug_int (sensitive);

    if ( sensitive )
    {
        grisbi_win_menu_move_to_acc_new ();
    }

    while ( *tmp )
    {
        gsb_menu_gui_sensitive_win_menu_item ( *tmp, sensitive );
        tmp++;
    }
    /* sensibilise le menu preferences */
    action = grisbi_app_get_prefs_action ();
    g_simple_action_set_enabled ( G_SIMPLE_ACTION ( action ), sensitive );
}

/**
 * Initialise la barre de menus si un compte est sélectionné
 *
 * \param sensitif
 *
 * \return
 * */
void gsb_menu_set_menus_view_account_sensitive ( gboolean sensitive )
{
    static gboolean flag_sensitive = FALSE;
    gchar * items[] = {
        "remove-acc",
        "show-form",
        "show-reconciled",
        "show-archived",
        "show-closed-acc",
        "show-ope",
        "reset-width-col",
        NULL
    };
    gchar **tmp = items;

    devel_debug_int (sensitive);

    if ( flag_sensitive == sensitive )
        return;

    while ( *tmp )
    {
        gsb_menu_gui_sensitive_win_menu_item ( *tmp, sensitive );
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
