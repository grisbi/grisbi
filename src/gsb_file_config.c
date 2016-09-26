/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2005-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*                      2008-2012 Pierre Biava (grisbi@pierre.biava.name)     */
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


/**
 * \file gsb_file_config.c
 * save and load the config file
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"
#include <errno.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "gsb_file_config.h"
#include "dialog.h"
#include "fenetre_principale.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "main.h"
#include "structures.h"
#include "utils_buttons.h"
#include "utils_files.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/


/*START_EXTERN*/
extern struct conditional_message delete_msg[];
extern struct conditional_message messages[];
extern gint nb_days_before_scheduled;
extern gchar *nom_fichier_comptes;
/*END_EXTERN*/

/* global variable, see structures.h */
struct gsb_conf_t conf;


gchar **tab_noms_derniers_fichiers_ouverts = NULL;

#if IS_DEVELOPMENT_VERSION == 1
/* flag de chargement du fichier modèle */
static gboolean used_model = FALSE;
#endif


/**
 * Set all the config variables to their default values.
 * called before loading the config
 * or for a new opening
 */
static void gsb_file_config_clean_config ( void )
{
    devel_debug (NULL);

    conf.main_width = 0;
    conf.main_height = 0;
    conf.prefs_width = 600;

    conf.force_enregistrement = 1;

    conf.r_modifiable = 0;       /* we can not change the reconciled transaction */
    conf.dernier_fichier_auto = 1;   /*  on n'ouvre pas directement le dernier fichier */
    conf.sauvegarde_auto = 0;    /* on NE sauvegarde PAS * automatiquement par défaut */
    conf.entree = 1;    /* la touche entree provoque l'enregistrement de l'opération */
    nb_days_before_scheduled = 0;     /* nb de jours avant l'échéance pour prévenir */
    conf.execute_scheduled_of_month = FALSE;
    conf.balances_with_scheduled = TRUE;
    conf.formulaire_toujours_affiche = 0;           /* le formulaire ne s'affiche que lors de l'edition d'1 opé */
    conf.affichage_exercice_automatique = 0;        /* l'exercice est choisi en fonction de la date */
    conf.automatic_completion_payee = 1;            /* by default automatic completion */
    conf.limit_completion_to_current_account = 0;   /* By default, do full search */
    conf.automatic_recover_splits = 1;
    conf.automatic_erase_credit_debit = 0;

    conf.display_grisbi_title = GSB_ACCOUNTS_TITLE; /* show Accounts file title par défaut */
    conf.display_toolbar = GSB_BUTTON_BOTH;         /* How to display toolbar icons. */
    conf.active_scrolling_left_pane = FALSE;        /* Active_scrolling_left_pane or not. */
    conf.show_headings_bar = TRUE;                  /* Show toolbar or not. */
    conf.show_transaction_selected_in_form = 1;     /* show selected transaction in form */
    conf.show_transaction_gives_balance = 1;        /* show transaction that gives the balance of the day */
    conf.transactions_list_primary_sorting = 1;     /* Primary sorting option for the transactions */
    conf.transactions_list_secondary_sorting = 0;   /* Secondary sorting option for the transactions */
    conf.show_closed_accounts = FALSE;

    if ( conf.font_string )
    {
    g_free ( conf.font_string );
    conf.font_string = NULL;
    }

    conf.force_enregistrement = 1;     /* par défaut, on force l'enregistrement */
    gsb_file_update_last_path (g_get_home_dir ());
    gsb_file_set_backup_path ( gsb_dirs_get_user_data_dir () );
    conf.make_backup = 1;
    conf.make_backup_every_minutes = FALSE;
    conf.make_backup_nb_minutes = 0;

#if IS_DEVELOPMENT_VERSION == 1
    conf.stable_config_file_model = 0;
#endif

    conf.nb_derniers_fichiers_ouverts = 0;
    conf.nb_max_derniers_fichiers_ouverts = 3;
    tab_noms_derniers_fichiers_ouverts = NULL;

    /* no compress by default */
    conf.compress_file = 0;
    conf.compress_backup = 0;

    /* archive data */
    conf.check_for_archival = TRUE;
    conf.max_non_archived_transactions_for_check = 3000;

    conf.last_tip = -1;
    conf.show_tip = FALSE;

    /* mise en conformité avec les recommandations FreeDesktop. */
    if ( conf.browser_command )
        g_free ( conf.browser_command );
    conf.browser_command = g_strdup ( ETAT_WWW_BROWSER );

    conf.metatree_action_2button_press = 0;     /* action par défaut pour le double clic sur division */

    memset ( etat.csv_skipped_lines, '\0', sizeof(gboolean) * CSV_MAX_TOP_LINES );
}


/**
 * called for each new element in the last xml config file
 * see the g_lib doc for the description of param
 *
 * \param context
 * \param text
 * \param text_len
 * \param user_data
 * \param error
 *
 * \return
 * */
static void gsb_file_config_get_xml_text_element ( GMarkupParseContext *context,
                        const gchar *text,
                        gsize text_len,
                        gpointer user_data,
                        GError **error)
{
    const gchar *element_name;
    gint i;

    element_name = g_markup_parse_context_get_element ( context );

    if ( !strcmp ( element_name,
		   "Width" ))
    {
	conf.main_width = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Height" ))
    {
	conf.main_height = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Modification_operations_rapprochees" ))
    {
	conf.r_modifiable = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Dernier_chemin_de_travail" ))
    {
	gsb_file_update_last_path (text);

	if ( !gsb_file_get_last_path ()
	     ||
	     !strlen (gsb_file_get_last_path ()))
	    gsb_file_update_last_path (g_get_home_dir ());
	return;
    }

    if ( !strcmp ( element_name,
		   "Affichage_alerte_permission" ))
    {
	 conf.alerte_permission = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Force_enregistrement" ))
    {
	conf.force_enregistrement = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Fonction_touche_entree" ))
    {
	conf.entree = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Affichage_messages_alertes" ))
    {
	conf.alerte_mini = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Utilise_fonte_des_listes" ))
    {
	conf.utilise_fonte_listes = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Fonte_des_listes" ))
    {
	conf.font_string = my_strdup (text);
	return;
    }
     if ( !strcmp ( element_name,
		   "Navigateur_web" ))
    {
        if ( conf.browser_command )
            g_free ( conf.browser_command );
	conf.browser_command = my_strdelimit (text,
					      "\\e",
					      "&" );
	return;
    }

    if ( !strcmp ( element_name,
		   "Largeur_colonne_echeancier" ))
    {
	etat.largeur_colonne_echeancier = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Largeur_colonne_comptes_comptes" ))
    {
	etat.largeur_colonne_comptes_comptes = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Largeur_colonne_etats" ))
    {
	etat.largeur_colonne_etat = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Chargement_auto_dernier_fichier" ))
    {
	conf.dernier_fichier_auto = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Nom_dernier_fichier" ))
    {
	nom_fichier_comptes = my_strdup (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Enregistrement_automatique" ))
    {
	conf.sauvegarde_auto = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Enregistrement_au_demarrage" ))
    {
	conf.sauvegarde_demarrage = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Nb_max_derniers_fichiers_ouverts" ))
    {
	conf.nb_max_derniers_fichiers_ouverts = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Compression_fichier" ))
    {
	conf.compress_file = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Compression_backup" ))
    {
	conf.compress_backup = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "fichier" ))
    {
	if (!tab_noms_derniers_fichiers_ouverts)
	    tab_noms_derniers_fichiers_ouverts = g_malloc0 ( conf.nb_max_derniers_fichiers_ouverts * sizeof(gchar *) );

	tab_noms_derniers_fichiers_ouverts[conf.nb_derniers_fichiers_ouverts] = my_strdup (text);
	conf.nb_derniers_fichiers_ouverts++;
	return;
    }

    if ( !strcmp ( element_name,
		   "Delai_rappel_echeances" ))
    {
	nb_days_before_scheduled = utils_str_atoi (text);
	conf.execute_scheduled_of_month = FALSE;
	return;
    }

    if ( !strcmp ( element_name,
		   "Affichage_formulaire" ))
    {
	conf.formulaire_toujours_affiche = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Affichage_exercice_automatique" ))
    {
	conf.affichage_exercice_automatique = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "display_toolbar" ))
    {
	conf.display_toolbar = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "show_closed_accounts" ))
    {
	conf.show_closed_accounts = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "show_tip" ))
    {
	conf.show_tip = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "last_tip" ))
    {
	conf.last_tip = utils_str_atoi (text);
	return;
    }

    for ( i = 0; messages[i].name; i++ )
    {
	if ( !strcmp ( element_name, messages[i].name ) )
	{
	    messages[i].hidden = utils_str_atoi (text);
	}
    }
}


/**
 * load the config file
 * it uses the glib config utils after 0.6.0
 * if cannot load, try the xml file before that version
 *
 * \param
 *
 * \return TRUE if ok, FALSE if not found, usually for a new opening
 * */
gboolean gsb_file_config_load_config ( void )
{
    GKeyFile *config;
    gboolean result;
    const gchar *filename;
    gchar *name;
    gint i;
    gint int_ret;
    GError* err = NULL;

    gsb_file_config_clean_config ();

    filename = gsb_dirs_get_grisbirc_filename ();

    config = g_key_file_new ();

    result = g_key_file_load_from_file ( config,
                        filename,
                        G_KEY_FILE_KEEP_COMMENTS,
                        NULL );

    if ( !result )
        return FALSE;

#if IS_DEVELOPMENT_VERSION == 1
    /* get config model */
    conf.stable_config_file_model = g_key_file_get_integer ( config,
                        "Model",
                        "Stable_config_file_model",
                        NULL );
    if ( conf.stable_config_file_model )
    {
        used_model = TRUE;
        filename = g_strconcat ( gsb_dirs_get_user_config_dir (), PACKAGE, ".conf", NULL );
        if ( !g_file_test (filename, G_FILE_TEST_EXISTS) )
        {
            g_key_file_free ( config );
            return FALSE;
        }

        g_key_file_free (config);
        config = g_key_file_new ();
        result = g_key_file_load_from_file ( config,
                        filename,
                        G_KEY_FILE_KEEP_COMMENTS,
                        NULL );
    }
#endif

    /* get the geometry */
    conf.root_x = g_key_file_get_integer ( config,
                        "Geometry",
                        "Root_x",
                        NULL );

    conf.root_y = g_key_file_get_integer ( config,
                        "Geometry",
                        "Root_y",
                        NULL );

    conf.main_width = g_key_file_get_integer ( config,
                        "Geometry",
                        "Width",
                        NULL );

    conf.main_height = g_key_file_get_integer ( config,
                        "Geometry",
                        "Height",
                        NULL );

    conf.full_screen = g_key_file_get_integer ( config,
                        "Geometry",
                        "Full_screen",
                        NULL );

    conf.maximize_screen = g_key_file_get_integer ( config,
                        "Geometry",
                        "Maximize_screen",
                        NULL );

    /* Remember size of main panel */
    int_ret = g_key_file_get_integer ( config,
                        "Geometry",
                        "Panel_width",
                        &err );
    if ( err == NULL )
        conf.panel_width = int_ret;
    else
    {
        conf.panel_width = -1;
        err = NULL;
    }

    /* preferences size */
    conf.prefs_width = g_key_file_get_integer ( config,
                        "Geometry",
                        "Prefs_width",
                        NULL );

    /* preferences sort */
    conf.prefs_sort = g_key_file_get_integer ( config,
                        "Geometry",
                        "Prefs_sort",
                        NULL );

    /* get general */
    conf.r_modifiable = g_key_file_get_integer ( config,
                        "General",
                        "Can modify R",
                        NULL );

    gsb_file_update_last_path ( g_key_file_get_string ( config,
                        "General",
                        "Path",
                        NULL ));

    conf.alerte_permission = g_key_file_get_integer ( config,
                        "General",
                        "Show permission alert",
                        NULL );

    conf.entree = g_key_file_get_integer ( config,
                        "General",
                        "Function of enter",
                        NULL );

    conf.alerte_mini = g_key_file_get_integer ( config,
                        "General",
                        "Show alert messages",
                        NULL );

    conf.utilise_fonte_listes = g_key_file_get_integer ( config,
                        "General",
                        "Use user font",
                        NULL );

    conf.font_string = g_key_file_get_string ( config, "General", "Font name", NULL );

    conf.browser_command = g_key_file_get_string ( config,
                        "General",
                        "Web",
                        NULL );

    conf.pluriel_final = g_key_file_get_integer ( config,
                        "General",
                        "Pluriel_final",
                        NULL );

    conf.metatree_action_2button_press = g_key_file_get_integer ( config,
                        "General",
                        "Metatree_action_2button_press",
                        NULL );

    /* get backup part */
    conf.make_backup = g_key_file_get_integer ( config,
                        "Backup",
                        "Make backup",
                        NULL );

    conf.make_bakup_single_file = g_key_file_get_integer ( config,
                        "Backup",
                        "Make backup single file",
                        NULL );

    conf.make_backup_every_minutes = g_key_file_get_integer ( config,
                        "Backup",
                        "Make backup every x minutes",
                        NULL );

    conf.make_backup_nb_minutes = g_key_file_get_integer ( config,
                        "Backup",
                        "Make backup nb minutes",
                        NULL );

    /* exec gsb_file_automatic_backup_start ( ) if necessary */
    if ( conf.make_backup_every_minutes
     &&
     conf.make_backup_nb_minutes )
        gsb_file_automatic_backup_start ( NULL, NULL );

    conf.compress_backup = g_key_file_get_integer ( config,
                        "Backup",
                        "Compress backup",
                        NULL );

    gsb_file_set_backup_path ( g_key_file_get_string ( config,
                        "Backup",
                        "Backup path",
                        NULL ));

    /* get input/output */
    conf.dernier_fichier_auto = g_key_file_get_integer ( config,
                        "IO",
                        "Load last file",
                        NULL );

    conf.sauvegarde_auto = g_key_file_get_integer ( config,
                        "IO",
                        "Save at closing",
                        NULL );

    conf.sauvegarde_demarrage = g_key_file_get_integer ( config,
                        "IO",
                        "Save at opening",
                        NULL );

    conf.nb_max_derniers_fichiers_ouverts = g_key_file_get_integer ( config,
                        "IO",
                        "Nb last opened files",
                        NULL );

    conf.compress_file = g_key_file_get_integer ( config,
                        "IO",
                        "Compress file",
                        NULL );

    conf.force_enregistrement = g_key_file_get_integer ( config,
                        "IO",
                        "Force saving",
                        NULL );

    tab_noms_derniers_fichiers_ouverts = g_key_file_get_string_list ( config,
                        "IO",
                        "Names last files",
                        &conf.nb_derniers_fichiers_ouverts,
                        NULL );
    if (tab_noms_derniers_fichiers_ouverts)
        nom_fichier_comptes = my_strdup (tab_noms_derniers_fichiers_ouverts [ 0 ]);
    else
        nom_fichier_comptes = NULL;

    conf.check_for_archival = g_key_file_get_integer ( config,
                        "IO",
                        "Check_archival_at_opening",
                        NULL );

    conf.max_non_archived_transactions_for_check = g_key_file_get_integer ( config,
                        "IO",
                        "Max_transactions_before_warn_archival",
                        NULL );

    /* get scheduled section */
    nb_days_before_scheduled = g_key_file_get_integer ( config,
                        "Scheduled",
                        "Days before remind",
                        NULL );

    conf.execute_scheduled_of_month = g_key_file_get_integer ( config,
                        "Scheduled",
                        "Execute scheduled of month",
                        NULL );

    int_ret = g_key_file_get_integer ( config,
                        "Scheduled",
                        "Balances with scheduled",
                        &err );
    if ( err == NULL )
        conf.balances_with_scheduled = int_ret;
    else
        err = NULL;

    conf.group_partial_balance_under_accounts = g_key_file_get_integer ( config,
                        "Scheduled",
                        "Group_partial_balance",
                        NULL );

    /* get shown section */
    conf.formulaire_toujours_affiche = g_key_file_get_integer ( config,
                        "Display",
                        "Show transaction form",
                        NULL );

    conf.show_transaction_selected_in_form = g_key_file_get_integer ( config,
                        "Display",
                        "Show selected transaction in form",
                        NULL );

    conf.show_transaction_gives_balance = g_key_file_get_integer ( config,
                        "Display",
                        "Show transaction gives balance",
                        NULL );

    conf.transactions_list_primary_sorting = g_key_file_get_integer ( config,
                        "Display",
                        "Transactions_list_primary_sorting",
                        NULL );

    conf.transactions_list_secondary_sorting = g_key_file_get_integer ( config,
                        "Display",
                        "Transactions_list_secondary_sorting",
                        NULL );

    conf.affichage_exercice_automatique = g_key_file_get_integer ( config,
                        "Display",
                        "Show automatic financial year",
                        NULL );

    int_ret = g_key_file_get_integer ( config,
                        "Display",
                        "Automatic completion payee",
                        &err );
    if ( err == NULL )
        conf.automatic_completion_payee = int_ret;
    else
        err = NULL;

    conf.limit_completion_to_current_account = g_key_file_get_integer ( config,
                        "Display",
                        "Limit payee completion",
                        NULL );

    conf.automatic_recover_splits = g_key_file_get_integer ( config,
                        "Display",
                        "Automatic_recover_splits",
                        NULL );

    conf.automatic_erase_credit_debit = g_key_file_get_integer ( config,
                        "Display",
                        "Automatic_erase_credit_debit",
                        NULL );

    conf.display_toolbar = g_key_file_get_integer ( config,
                        "Display",
                        "Display toolbar",
                        NULL );

    conf.active_scrolling_left_pane = g_key_file_get_integer ( config,
                        "Display",
                        "Active_scrolling_left_pane",
                        NULL );

    conf.show_headings_bar = g_key_file_get_integer ( config,
                        "Display",
                        "Show headings bar",
                        NULL );

    conf.show_closed_accounts = g_key_file_get_integer ( config,
                        "Display",
                        "Show closed accounts",
                        NULL );

    conf.display_grisbi_title = g_key_file_get_integer ( config,
                        "Display",
                        "Display grisbi title",
                        NULL );

    /* get messages */
    for ( i = 0; messages[i].name; i ++ )
    {
        name = g_strconcat ( messages[i].name , "-answer", NULL );
        messages[i].hidden = g_key_file_get_integer ( config, "Messages",
                        messages[i].name, NULL );
        messages[i].default_answer = g_key_file_get_integer ( config, "Messages",
                        name, NULL );
        g_free ( name );
    }

    for ( i = 0; delete_msg[i].name; i ++ )
    {
        name = g_strconcat ( delete_msg[i].name , "-answer", NULL );
        delete_msg[i].hidden = g_key_file_get_integer ( config, "Messages",
                        delete_msg[i].name, NULL );
        if ( delete_msg[i].hidden == 1 )
            delete_msg[i].default_answer = 1;
        g_free ( name );
    }

    conf.last_tip = g_key_file_get_integer ( config,
                        "Messages",
                        "Last tip",
                        NULL );

    conf.show_tip = g_key_file_get_integer ( config,
                        "Messages",
                        "Show tip",
                        NULL );

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
 * */
gboolean gsb_file_config_save_config ( void )
{
    GKeyFile *config;
    const gchar *filename;
    gchar *file_content;
    gchar *name;
    gsize length;
    FILE *conf_file;
    gint i;

    devel_debug (NULL);

    filename = gsb_dirs_get_grisbirc_filename ();
    config = g_key_file_new ();

#if IS_DEVELOPMENT_VERSION == 1
    /* set config model */
    g_key_file_set_integer ( config,
                        "Model",
                        "Stable_config_file_model",
                        conf.stable_config_file_model );
#endif

    g_key_file_set_integer ( config,
                        "Geometry",
                        "Root_x",
                        conf.root_x );

    g_key_file_set_integer ( config,
                        "Geometry",
                        "Root_y",
                        conf.root_y );

    g_key_file_set_integer ( config,
                        "Geometry",
                        "Width",
                        conf.main_width );

    g_key_file_set_integer ( config,
                        "Geometry",
                        "Height",
                        conf.main_height );

    g_key_file_set_integer ( config,
                        "Geometry",
                        "Full_screen",
                        conf.full_screen );

    g_key_file_set_integer ( config,
                        "Geometry",
                        "Maximize_screen",
                        conf.maximize_screen );

    /* Remember size of main panel */
    g_key_file_set_integer ( config,
                        "Geometry",
                        "Panel_width",
                        conf.panel_width );

    /* preferences size */
    g_key_file_set_integer ( config,
                        "Geometry",
                        "Prefs_width",
                        conf.prefs_width );

    /* preferences sort */
    g_key_file_set_integer ( config,
                        "Geometry",
                        "Prefs_sort",
                        conf.prefs_sort );

    /* save general */
    g_key_file_set_integer ( config,
                        "General",
                        "Can modify R",
                        conf.r_modifiable );

    g_key_file_set_string ( config,
                        "General",
                        "Path",
                        gsb_file_get_last_path () );

    g_key_file_set_integer ( config,
                        "General",
                        "Show permission alert",
                        conf.alerte_permission );

    g_key_file_set_integer ( config,
                        "General",
                        "Function of enter",
                        conf.entree );

    g_key_file_set_integer ( config,
                        "General",
                        "Show alert messages",
                        conf.alerte_mini );

    g_key_file_set_integer ( config,
                        "General",
                        "Use user font",
                        conf.utilise_fonte_listes );

    if ( conf.font_string )
        g_key_file_set_string ( config,
                        "General",
                        "Font name",
                        conf.font_string );

    if ( conf.browser_command )
    {
        gchar *string;

        string = my_strdelimit ( conf.browser_command, "&", "\\e" );
        if ( string )
            g_key_file_set_string ( config,
                        "General",
                        "Web",
                        string );
        g_free (string);
    }

    g_key_file_set_integer ( config,
                        "General",
                        "Pluriel_final",
                        conf.pluriel_final );

    g_key_file_set_integer ( config,
                        "General",
                        "Metatree_action_2button_press",
                        conf.metatree_action_2button_press );

     /* save backup part */
    g_key_file_set_integer ( config,
                        "Backup",
                        "Make backup",
                        conf.make_backup );

    g_key_file_set_integer ( config,
                        "Backup",
                        "Make backup single file",
                        conf.make_bakup_single_file );

    g_key_file_set_integer ( config,
                        "Backup",
                        "Make backup every x minutes",
                        conf.make_backup_every_minutes );

    g_key_file_set_integer ( config,
                        "Backup",
                        "Compress backup",
                        conf.compress_backup );

    g_key_file_set_integer ( config,
                        "Backup",
                        "Make backup nb minutes",
                        conf.make_backup_nb_minutes );

    if (gsb_file_get_backup_path ())
        g_key_file_set_string ( config,
                        "Backup",
                        "Backup path",
                        gsb_file_get_backup_path ());

    /* save input/output */
    g_key_file_set_integer ( config,
                        "IO",
                        "Load last file",
                        conf.dernier_fichier_auto );

    g_key_file_set_integer ( config,
                        "IO",
                        "Save at closing",
                        conf.sauvegarde_auto );

    g_key_file_set_integer ( config,
                        "IO",
                        "Save at opening",
                        conf.sauvegarde_demarrage );

    g_key_file_set_integer ( config,
                        "IO",
                        "Nb last opened files",
                        conf.nb_max_derniers_fichiers_ouverts );

    g_key_file_set_integer ( config,
                        "IO",
                        "Compress file",
                        conf.compress_file );

    g_key_file_set_integer ( config,
                        "IO",
                        "Force saving",
                        conf.force_enregistrement );

    if ( conf.nb_derniers_fichiers_ouverts > 0
     &&
     tab_noms_derniers_fichiers_ouverts)
        g_key_file_set_string_list ( config,
                        "IO",
                        "Names last files",
                        (const gchar **) tab_noms_derniers_fichiers_ouverts,
                        conf.nb_derniers_fichiers_ouverts);

    g_key_file_set_integer ( config,
                        "IO",
                        "Check_archival_at_opening",
                        conf.check_for_archival );

    g_key_file_set_integer ( config,
                        "IO",
                        "Max_transactions_before_warn_archival",
                        conf.max_non_archived_transactions_for_check );

    /* save scheduled section */
    g_key_file_set_integer ( config,
                        "Scheduled",
                        "Days before remind",
                        nb_days_before_scheduled );

    g_key_file_set_integer ( config,
                        "Scheduled",
                        "Execute scheduled of month",
                        conf.execute_scheduled_of_month );

    g_key_file_set_integer ( config,
                        "Scheduled",
                        "Balances with scheduled",
                        conf.balances_with_scheduled );

    g_key_file_set_integer ( config,
                        "Scheduled",
                        "Group_partial_balance",
                        conf.group_partial_balance_under_accounts );

    /* save shown section */
    g_key_file_set_integer ( config,
                        "Display",
                        "Show transaction form",
                        conf.formulaire_toujours_affiche );

    g_key_file_set_integer ( config,
                        "Display",
                        "Show selected transaction in form",
                        conf.show_transaction_selected_in_form );

    g_key_file_set_integer ( config,
                        "Display",
                        "Show transaction gives balance",
                        conf.show_transaction_gives_balance );

    g_key_file_set_integer ( config,
                        "Display",
                        "Transactions_list_primary_sorting",
                        conf.transactions_list_primary_sorting );

    g_key_file_set_integer ( config,
                        "Display",
                        "Transactions_list_secondary_sorting",
                        conf.transactions_list_secondary_sorting );

    g_key_file_set_integer ( config,
                        "Display",
                        "Show automatic financial year",
                        conf.affichage_exercice_automatique );

    g_key_file_set_integer ( config,
                        "Display",
                        "Automatic completion payee",
                        conf.automatic_completion_payee );

    g_key_file_set_integer ( config,
                        "Display",
                        "Limit payee completion",
                        conf.limit_completion_to_current_account );

    g_key_file_set_integer ( config,
                        "Display",
                        "Automatic_recover_splits",
                        conf.automatic_recover_splits );

    g_key_file_set_integer ( config,
                        "Display",
                        "Automatic_erase_credit_debit",
                        conf.automatic_erase_credit_debit );

    g_key_file_set_integer ( config,
                        "Display",
                        "Display toolbar",
                        conf.display_toolbar );

    g_key_file_set_integer ( config,
                        "Display",
                        "Active_scrolling_left_pane",
                        conf.active_scrolling_left_pane );

    g_key_file_set_integer ( config,
                        "Display",
                        "Show headings bar",
                        conf.show_headings_bar );

    g_key_file_set_integer ( config,
                        "Display",
                        "Show closed accounts",
                        conf.show_closed_accounts );

    g_key_file_set_integer ( config,
                        "Display",
                        "Display grisbi title",
                        conf.display_grisbi_title );

    /* save messages */
    for ( i = 0; messages[i].name; i ++ )
    {
        name = g_strconcat ( messages[i].name , "-answer", NULL );

        g_key_file_set_integer ( config, "Messages", messages[i].name, messages[i].hidden );
        g_key_file_set_integer ( config, "Messages", name, messages[i].default_answer );
        g_free ( name );
    }

    for ( i = 0; delete_msg[i].name; i ++ )
    {
        g_key_file_set_integer ( config, "Messages", delete_msg[i].name,
                        delete_msg[i].hidden );
    }

    g_key_file_set_integer ( config,
                        "Messages",
                        "Last tip",
                        conf.last_tip );

    g_key_file_set_integer ( config,
                        "Messages",
                        "Show tip",
                        conf.show_tip );

    /* save into a file */
    file_content = g_key_file_to_data ( config, &length, NULL );

    conf_file = fopen ( filename, "w" );

    #ifndef _WIN32
    if ( !conf_file )
    {
        utils_files_create_XDG_dir ( );
        conf_file = fopen ( filename, "w" );
    }
    #endif

    if ( !conf_file
     ||
     !fwrite ( file_content, sizeof ( gchar ), length, conf_file ) )
    {
        gchar* tmpstr = g_strdup_printf ( _("Cannot save configuration file '%s': %s"),
                        filename,
                        g_strerror ( errno ) );
        dialogue_error ( tmpstr );
        g_free ( tmpstr );
        g_free ( file_content);
        g_key_file_free (config);
        return ( FALSE );
    }

    fclose ( conf_file );
    g_free ( file_content);
    g_key_file_free (config);

    return TRUE;
}



/**
 * load the xml file config for grisbi before 0.6.0
 * try to find it, if not, return FALSE
 * */
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
