/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2004-2009 Benjamin Drieu (bdrieu@april.org)                       */
/*          2009-2015 Pierre Biava (grisbi@pierre.biava.name)                 */
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
static gboolean gsb_gui_toggle_show_closed_accounts ( void );
static gboolean gsb_gui_toggle_show_form ( void );

static gboolean help_bugreport ( void );
static gboolean help_manual ( void );
static gboolean help_quick_start ( void );
static gboolean help_translation ( void );
static gboolean help_website ( void );
static gboolean gsb_menu_reinit_largeur_col_menu ( void );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/


static gboolean block_menu_cb = FALSE;
static GtkUIManager *ui_manager;
static gint merge_id = -1;
static GtkActionGroup *recent_files_action_group = NULL;
static gint recent_files_merge_id = -1;
static GtkActionGroup *move_to_account_action_group = NULL;
static gint move_to_account_merge_id = -1;

/* fonctions statiques */
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
	help_manual ();
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
	help_quick_start ();
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
	help_website ();
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
	help_bugreport ();
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
						gpointer win )
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
						gpointer win )
{
	gsb_file_open_menu ();
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
void grisbi_cmd_file_open_direct_menu ( GtkRecentManager *recent_manager,
						GtkRecentChooser *chooser )
{
	gchar *uri;
	gchar *tmp_str;
	GError *error = NULL;

	uri = gtk_recent_chooser_get_current_uri ( chooser );
	tmp_str = g_filename_from_uri ( uri, NULL, &error );
	if ( tmp_str )
	{
		if ( gsb_file_open_file ( tmp_str ) == FALSE )
			gtk_recent_manager_remove_item ( recent_manager, uri, NULL );

		g_free ( tmp_str );
	}
	else
	{
		gtk_recent_manager_remove_item ( recent_manager, uri, NULL );
        g_error_free ( error );
	}
	g_free ( uri );
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
						gpointer user_data )
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
						gpointer user_data )
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
						gpointer user_data )
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
						gpointer user_data )
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
						gpointer user_data )
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
						gpointer user_data )
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
						gpointer user_data )
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
						gpointer user_data )
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
						gpointer user_data )
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
						gpointer user_data )
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
						gpointer user_data )
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
						gpointer user_data )
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
						gpointer user_data )
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
						gpointer user_data )
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
						gpointer user_data )
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
						gpointer user_data )
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
						gpointer user_data )
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
void grisbi_cmd_new_acc ( GSimpleAction *action,
						GVariant *parameter,
						gpointer user_data )
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
						gpointer user_data )
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
						gpointer user_data )
{
	devel_debug (NULL);

    /* FIXME benj: ugly but I cannot find a way to block this ... */
    if ( block_menu_cb )
        return;

    gsb_form_switch_expander ( );
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
						gpointer user_data )
{
    gint current_account;

    if ( block_menu_cb )
	    return;

    current_account = gsb_gui_navigation_get_current_account ( );
    if ( current_account == -1 || run.equilibrage == 1 )
        return;

    if ( gsb_data_account_get_r ( current_account ) )
	    change_aspect_liste ( 6 );
    else
	    change_aspect_liste ( 5 );

    return;
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
						gpointer user_data )
{
    gint current_account;

    if ( block_menu_cb )
	    return;

    current_account = gsb_gui_navigation_get_current_account ( );
    if ( current_account == -1 )
        return;

    if ( gsb_data_account_get_l ( current_account ) )
	    change_aspect_liste ( 8 );
    else
	    change_aspect_liste ( 7 );

    return;
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
						gpointer user_data )
{
    conf.show_closed_accounts = !conf.show_closed_accounts;

    gsb_gui_navigation_create_account_list ( gsb_gui_navigation_get_model ( ) );
    gsb_gui_navigation_update_home_page ( );

    gsb_file_set_modified ( TRUE );

    return;
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
void grisbi_cmd_show_ope ( GSimpleAction *action,
						GVariant *parameter,
						gpointer user_data )
{
	printf ("grisbi_cmd_show_ope = %s\n", g_variant_get_string ( parameter, NULL ) );
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
						gpointer user_data )
{
	gsb_menu_reinit_largeur_col_menu ();
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

		tmp_recent_manager = grisbi_app_get_recent_manager ();
		gtk_recent_manager_remove_item ( tmp_recent_manager, uri, &error );
	}

	if ( error )
	{
		g_warning ( "Could not remove recent-files uri \"%s\": %s", uri, error->message );
		g_error_free ( error );
	}

  g_free ( uri );
}

gchar **gsb_menu_recent_manager_get_recent_array ( void )
{
	GtkRecentManager *recent_manager;
	GList *tmp_list;
	gchar **recent_array = NULL;
	gchar *tmp_str = NULL;
	gint index = 0;

	recent_manager = grisbi_app_get_recent_manager ();

	/* initialisation du tableau des fichiers récents */
	recent_array = g_malloc ( ( conf.nb_max_derniers_fichiers_ouverts +1 ) * sizeof ( *recent_array ) );
	recent_array[conf.nb_max_derniers_fichiers_ouverts] = NULL;

	tmp_list = gtk_recent_manager_get_items ( recent_manager );

	while ( tmp_list )
    {
		GtkRecentInfo *info;
		const gchar *uri;

		info = tmp_list->data;

		uri = gtk_recent_info_get_uri ( info );

		if ( g_str_has_suffix ( uri, ".gsb" ) )
		{

			if ( index < conf.nb_max_derniers_fichiers_ouverts )
			{
				tmp_str = g_filename_from_uri ( uri, NULL, NULL );
				if ( tmp_str )
					recent_array[index] = tmp_str;

				index++;
			}
		}
		gtk_recent_info_unref ( info );

		tmp_list = tmp_list->next;
    }
	g_list_free ( tmp_list );

	if ( index < conf.nb_max_derniers_fichiers_ouverts )
	{
		recent_array[index] = NULL;
		conf.nb_derniers_fichiers_ouverts = index;
	}

	else
		conf.nb_derniers_fichiers_ouverts = conf.nb_max_derniers_fichiers_ouverts;

	return recent_array;
}

void gsb_menu_recent_manager_set_recent_array ( gchar **recent_array )
{
	GtkRecentManager *recent_manager;
	gchar *uri = NULL;
	gint i;
	gboolean result;

	recent_manager = grisbi_app_get_recent_manager ();

	if ( !conf.nb_derniers_fichiers_ouverts || !conf.nb_max_derniers_fichiers_ouverts )
		return;

    if ( conf.nb_derniers_fichiers_ouverts > conf.nb_max_derniers_fichiers_ouverts )
    {
        conf.nb_derniers_fichiers_ouverts = conf.nb_max_derniers_fichiers_ouverts;
    }

	for ( i=0 ; i < conf.nb_derniers_fichiers_ouverts ; i++ )
    {
		uri = g_filename_to_uri ( recent_array[i], NULL, NULL );
		result = gtk_recent_manager_add_item (  recent_manager, uri );
	}
}

/* A MODIFIER / SUPPRIMER */
/**
 * remove all actions of an action group
 *
 * \param action_group
 *
 * \return
 * */
static void gsb_menu_action_group_remove_actions ( GtkActionGroup *action_group )
{
}


/**
 * initialise un action_group temporaire
 *
 * \param manager
 * \param action_group
 *
 * \return
 **/
static void gsb_menu_ui_manager_remove_action_group ( GtkUIManager *manager,
                        GtkActionGroup *action_group,
                        gint merge_id )
{
}


/**
 * Blank the "Recent files submenu".
 *
 * \param
 *
 * \return
 * */
void efface_derniers_fichiers_ouverts ( void )
{
}


/**
 * Add menu items to the "Recent files" submenu.
 */
gboolean affiche_derniers_fichiers_ouverts ( void )
{
    return FALSE;
}



/**
 * Start a browser processus with local copy of manual on command
 * line.
 *
 * \return FALSE
 */
gboolean help_manual ( void )
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
 * Start a browser processus with local copy of the quick start page
 * on command line.
 *
 * \return FALSE
 */
gboolean help_quick_start ( void )
{
    gchar *lang = _("en");

    gchar* tmpstr = g_build_filename ( HELP_PATH, lang, "quickstart.html", NULL );
    lance_navigateur_web ( tmpstr );
    g_free ( tmpstr );

    return FALSE;
}



/**
 * Start a browser processus with local copy of the translation page
 * on command line.
 *
 * \return FALSE
 */
gboolean help_translation ( void )
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
gboolean help_website ( void )
{
    lance_navigateur_web ( "http://www.grisbi.org/" );

    return FALSE;
}



/**
 * Start a browser processus with Grisbi bug report page displayed.
 *
 * \return FALSE
 */
gboolean help_bugreport ( void )
{
    lance_navigateur_web ( "http://www.grisbi.org/bugtracking/" );

    return FALSE;
}



/**
 * Set sensitiveness of a menu item according to a string
 * representation of its position in the menu.
 * menu.
 *
 * \param item_name		Path of the menu item.
 * \param state			Whether widget should be 'sensitive' or not.
 *
 * \return TRUE on success.
 */
gboolean gsb_gui_sensitive_menu_item ( gchar *item_name, gboolean state )
{
    GtkWidget * widget;

/*    widget = gtk_ui_manager_get_widget ( ui_manager, item_name );

    if ( widget && GTK_IS_WIDGET(widget) )
    {
	gtk_widget_set_sensitive ( widget, state );
	return TRUE;
    }
    return FALSE;
*/}



/**
 * Callback called when an item of the "View/Show ... lines" menu is
 * triggered.
 */
void gsb_gui_toggle_line_view_mode ( GtkRadioAction *action,
                        GtkRadioAction *current,
                        gpointer user_data )
{
    /* FIXME benj: ugly but I cannot find a way to block this ... */
    if ( block_menu_cb ) return;

    switch ( gtk_radio_action_get_current_value(current) )
    {
	case ONE_LINE_PER_TRANSACTION:
	    change_aspect_liste (1);
	    break;
	case TWO_LINES_PER_TRANSACTION:
	    change_aspect_liste (2);
	    break;
	case THREE_LINES_PER_TRANSACTION:
	    change_aspect_liste (3);
	    break;
	case FOUR_LINES_PER_TRANSACTION:
	    change_aspect_liste (4);
	    break;
    }
}



/**
 * Show or hide display of reconciled transactions.
 *
 * \return FALSE
 */
gboolean gsb_gui_toggle_show_reconciled ( void )
{
    gint current_account;

    if ( block_menu_cb )
	    return FALSE;

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
gboolean gsb_gui_toggle_show_archived ( void )
{
    gint current_account;

    if ( block_menu_cb )
	    return FALSE;

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
 * Show or hide closed accounts.
 *
 * \return FALSE
 */
gboolean gsb_gui_toggle_show_closed_accounts ( void )
{
    conf.show_closed_accounts = !conf.show_closed_accounts;

    gsb_gui_navigation_create_account_list ( gsb_gui_navigation_get_model ( ) );
    gsb_gui_navigation_update_home_page ( );

    gsb_file_set_modified ( TRUE );

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
    gchar * item_name = NULL;
    gchar *tmpstr;

    devel_debug_int (account_number);

    block_menu_cb = TRUE;

    /* update the showing of reconciled transactions */
    tmpstr = "/menubar/ViewMenu/ShowReconciled";
    gtk_toggle_action_set_active ( GTK_TOGGLE_ACTION (
                        gtk_ui_manager_get_action ( ui_manager, tmpstr) ),
				        gsb_data_account_get_r ( account_number ) );

    tmpstr = "/menubar/ViewMenu/ShowTransactionForm";
    gtk_toggle_action_set_active ( GTK_TOGGLE_ACTION (
                        gtk_ui_manager_get_action ( ui_manager, tmpstr) ),
				        gsb_form_is_visible ( ) );

    /* update the showing of archived transactions */
    tmpstr = "/menubar/ViewMenu/ShowArchived";
    gtk_toggle_action_set_active ( GTK_TOGGLE_ACTION (
                        gtk_ui_manager_get_action ( ui_manager, tmpstr) ),
				        gsb_data_account_get_l ( account_number ) );

    /* update the number of line showed */
    switch ( gsb_data_account_get_nb_rows (account_number))
    {
	default:
	case 1 :
	    item_name = "/menubar/ViewMenu/ShowOneLine";
	    break;
	case 2 :
	    item_name = "/menubar/ViewMenu/ShowTwoLines";
	    break;
	case 3 :
	    item_name = "/menubar/ViewMenu/ShowThreeLines";
	    break;
	case 4 :
	    item_name = "/menubar/ViewMenu/ShowFourLines";
	    break;
    }

    gtk_toggle_action_set_active ( GTK_TOGGLE_ACTION (
                        gtk_ui_manager_get_action ( ui_manager, item_name ) ),
				        TRUE );
    block_menu_cb = FALSE;

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

    if ( move_to_account_action_group )
    {
        gsb_menu_ui_manager_remove_action_group ( ui_manager,
                        move_to_account_action_group,
                        move_to_account_merge_id );
        g_object_unref ( G_OBJECT ( move_to_account_action_group ) );
        move_to_account_action_group = NULL;
    }

    move_to_account_merge_id = gtk_ui_manager_new_merge_id ( ui_manager );
    move_to_account_action_group = gtk_action_group_new ( "Group3" );

    /* create the closed accounts and accounts in the menu to move a transaction */
    list_tmp = gsb_data_account_get_list_accounts ();

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

    return FALSE;
}



/**
 * Set sensitiveness of all menu items that work on the selected transaction.

 * \param sensitive	Sensitiveness (FALSE for unsensitive, TRUE for
 *			sensitive).
 *
 * \return		FALSE
 */
gboolean gsb_menu_set_menus_select_transaction_sensitive ( gboolean sensitive )
{
    devel_debug ( sensitive ? "item sensitive" : "item unsensitive" );

    gsb_gui_sensitive_menu_item ( "/menubar/EditMenu/EditTransaction", sensitive );
    gsb_gui_sensitive_menu_item ( "/menubar/EditMenu/RemoveTransaction", sensitive );
    gsb_gui_sensitive_menu_item ( "/menubar/EditMenu/TemplateTransaction", sensitive );
    gsb_gui_sensitive_menu_item ( "/menubar/EditMenu/CloneTransaction", sensitive );
    gsb_gui_sensitive_menu_item ( "/menubar/EditMenu/ConvertToScheduled", sensitive );
    gsb_gui_sensitive_menu_item ( "/menubar/EditMenu/MoveToAnotherAccount", sensitive );

    return FALSE;
}


/**
 * Set sensitiveness of all menu items that work on the selected scheduled.

 * \param sensitive	Sensitiveness (FALSE for unsensitive, TRUE for
 *			sensitive).
 *
 * \return		FALSE
 */
gboolean gsb_menu_set_menus_select_scheduled_sensitive ( gboolean sensitive )
{
    devel_debug ( sensitive ? "item sensitive" : "item unsensitive" );

    gsb_gui_sensitive_menu_item ( "/menubar/EditMenu/EditTransaction", sensitive );
    gsb_gui_sensitive_menu_item ( "/menubar/EditMenu/RemoveTransaction", sensitive );
    gsb_gui_sensitive_menu_item ( "/menubar/EditMenu/CloneTransaction", sensitive );

    return FALSE;
}


/**
 *
 *
 *
 *
 **/
GtkUIManager *gsb_menu_get_ui_manager ( void )
{
    return ui_manager;
}


/**
 *
 *
 *
 *
 **/
void gsb_menu_free_ui_manager ( void )
{
    if ( ! ui_manager )
        return;

    g_object_unref ( G_OBJECT ( ui_manager ) );
    ui_manager = NULL;
}


/**
 *
 *
 *
 *
 **/
gboolean gsb_menu_reinit_largeur_col_menu ( void )
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


/**
 *
 *
 *
 */
gboolean gsb_menu_set_block_menu_cb ( gboolean etat )
{

    block_menu_cb = etat;

    return FALSE;
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
    gchar * items[] = {
        "/menubar/FileMenu/Save",
        "/menubar/FileMenu/SaveAs",
        "/menubar/FileMenu/DebugFile",
        "/menubar/FileMenu/Obfuscate",
        "/menubar/FileMenu/DebugMode",
        "/menubar/FileMenu/ExportFile",
        "/menubar/FileMenu/CreateArchive",
        "/menubar/FileMenu/ExportArchive",
        "/menubar/FileMenu/Close",
        "/menubar/EditMenu/NewTransaction",
        "/menubar/EditMenu/RemoveTransaction",
        "/menubar/EditMenu/TemplateTransaction",
        "/menubar/EditMenu/CloneTransaction",
        "/menubar/EditMenu/EditTransaction",
        "/menubar/EditMenu/ConvertToScheduled",
        "/menubar/EditMenu/MoveToAnotherAccount",
        "/menubar/EditMenu/Preferences",
        "/menubar/EditMenu/RemoveAccount",
        "/menubar/EditMenu/NewAccount",
        "/menubar/ViewMenu/ShowTransactionForm",
        "/menubar/ViewMenu/ShowReconciled",
        "/menubar/ViewMenu/ShowArchived",
        "/menubar/ViewMenu/ShowClosed",
        "/menubar/ViewMenu/ShowOneLine",
        "/menubar/ViewMenu/ShowTwoLines",
        "/menubar/ViewMenu/ShowThreeLines",
        "/menubar/ViewMenu/ShowFourLines",
        "/menubar/ViewMenu/InitwidthCol",
        NULL
    };
    gchar ** tmp = items;

    devel_debug_int (sensitive);

    while ( *tmp )
    {
        gsb_gui_sensitive_menu_item ( *tmp, sensitive );
        tmp++;
    }

    /* As this function may only be called when a new account is
     * created and the like, it is unlikely that we want to sensitive
     * transaction-related menus. */
    gsb_gui_sensitive_menu_item ( "/menubar/EditMenu/NewTransaction", FALSE );
    gsb_menu_set_menus_select_transaction_sensitive ( FALSE );
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
    gchar * items[] = {
        "/menubar/ViewMenu/ShowTransactionForm",
        "/menubar/ViewMenu/ShowReconciled",
        "/menubar/ViewMenu/ShowArchived",
        "/menubar/ViewMenu/ShowOneLine",
        "/menubar/ViewMenu/ShowTwoLines",
        "/menubar/ViewMenu/ShowThreeLines",
        "/menubar/ViewMenu/ShowFourLines",
        "/menubar/ViewMenu/InitwidthCol",
        NULL
    };
    gchar **tmp = items;

    devel_debug_int (sensitive);

    while ( *tmp )
    {
        gsb_gui_sensitive_menu_item ( *tmp, sensitive );
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
