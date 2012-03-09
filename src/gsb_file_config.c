/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2005-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*                      2008-2011 Pierre Biava (grisbi@pierre.biava.name)     */
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
#include "grisbi_app.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "utils_buttons.h"
#include "utils_files.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void gsb_file_config_clean_config ( GrisbiAppConf *conf );
/*END_STATIC*/


/*START_EXTERN*/
extern struct conditional_message delete_msg[];
extern struct conditional_message messages[];
/*END_EXTERN*/


/* nom du fichier de configuration */
static gchar *conf_filename;

/**
 * load the config file
 * it uses the glib config utils after 0.6.0
 * if cannot load, try the xml file before that version
 *
 * \param
 *
 * \return TRUE if ok, FALSE if not found, usually for a new opening
 * */
gboolean gsb_file_config_load_config ( GrisbiAppConf *conf )
{
    GKeyFile *config;
    gboolean result;
    gchar *name;
    gint i;
    gint int_ret;
    GError* err = NULL;

    gsb_file_config_clean_config ( conf );

    if ( !g_file_test ( conf_filename, G_FILE_TEST_EXISTS ) )
        return FALSE;

    config = g_key_file_new ();
    
    result = g_key_file_load_from_file ( config,
                        conf_filename,
                        G_KEY_FILE_KEEP_COMMENTS,
                        NULL );
    if ( !result )
    {
        return FALSE;
    }

    if ( IS_DEVELOPMENT_VERSION )
    {
        gchar *tmp_filename;

        /* get config model */
        conf->stable_config_file_model = g_key_file_get_integer ( config,
                        "Model",
                        "Stable_config_file_model",
                        NULL );

        if ( conf->stable_config_file_model )
        {
            gchar *tmp_str;

            tmp_str = g_strconcat ( PACKAGE, ".conf", NULL );
            tmp_filename = g_build_filename ( gsb_dirs_get_user_config_dir (), tmp_str, NULL );

            if ( g_file_test ( tmp_filename, G_FILE_TEST_EXISTS ) )
            {
                g_key_file_free ( config );
                config = g_key_file_new ( );
                result = g_key_file_load_from_file ( config,
                        tmp_filename,
                        G_KEY_FILE_KEEP_COMMENTS,
                        NULL );
            }
            g_free ( tmp_str );
            g_free ( tmp_filename );
        }
    }

    /* get the geometry */
    conf->root_x = g_key_file_get_integer ( config,
                        "Geometry",
                        "Root_x",
                        NULL );

    conf->root_y = g_key_file_get_integer ( config,
                        "Geometry",
                        "Root_y",
                        NULL );

    conf->main_width = g_key_file_get_integer ( config,
                        "Geometry",
                        "Width",
                        NULL );

    conf->main_height = g_key_file_get_integer ( config,
                        "Geometry",
                        "Height",
                        NULL );

    conf->full_screen = g_key_file_get_integer ( config,
                        "Geometry",
                        "Full_screen",
                        NULL );

    conf->maximize_screen = g_key_file_get_integer ( config,
                        "Geometry",
                        "Maximize_screen",
                        NULL );

    int_ret = g_key_file_get_integer ( config,
                        "Geometry",
                        "Panel_width",
                        &err );
    if ( err == NULL )
        conf->panel_width = int_ret;
    else
    {
        conf->panel_width = -1;
        err = NULL;
    }

    int_ret = g_key_file_get_integer ( config,
                        "Geometry",
                        "Prefs_width",
                        &err );
    if ( err == NULL )
        conf->prefs_width = int_ret;
    else
        err = NULL;

    int_ret = g_key_file_get_integer ( config,
                        "Geometry",
                        "Prefs_height",
                        &err );
    if ( err == NULL )
        conf->prefs_height = int_ret;
    else
        err = NULL;

    int_ret = g_key_file_get_integer ( config,
                        "Geometry",
                        "Prefs_panel_width",
                        &err );
    if ( err == NULL )
        conf->prefs_panel_width = int_ret;
    else
        err = NULL;

    /* get general */
    conf->r_modifiable = g_key_file_get_integer ( config,
                        "General",
                        "Can modify R",
                        NULL );

    gsb_file_update_last_path ( g_key_file_get_string ( config,
                        "General",
                        "Path",
                        NULL ));

    conf->alerte_permission = g_key_file_get_integer ( config,
                        "General",
                        "Show permission alert",
                        NULL );

    conf->entree = g_key_file_get_integer ( config,
                        "General",
                        "Function of enter",
                        NULL );

    conf->alerte_mini = g_key_file_get_integer ( config,
                        "General",
                        "Show alert messages",
                        NULL );

    conf->utilise_fonte_listes = g_key_file_get_integer ( config,
                        "General",
                        "Use user font",
                        NULL );
    
    conf->font_string = g_key_file_get_string ( config, "General", "Font name", NULL );
    
    conf->browser_command = g_key_file_get_string ( config,
                        "General",
                        "Web",
                        NULL );

    conf->pluriel_final = g_key_file_get_integer ( config,
                        "General",
                        "Pluriel_final",
                        NULL );
                        
    conf->metatree_action_2button_press = g_key_file_get_integer ( config,
                        "General",
                        "Metatree_action_2button_press",
                        NULL );

    /* get backup part */
    conf->make_backup = g_key_file_get_integer ( config,
                        "Backup",
                        "Make backup",
                        NULL );

    conf->make_bakup_single_file = g_key_file_get_integer ( config,
                        "Backup",
                        "Make backup single file",
                        NULL );

    conf->make_backup_every_minutes = g_key_file_get_integer ( config,
                        "Backup",
                        "Make backup every x minutes",
                        NULL );

    conf->make_backup_nb_minutes = g_key_file_get_integer ( config,
                        "Backup",
                        "Make backup nb minutes",
                        NULL );

    /* exec gsb_file_automatic_backup_start ( ) if necessary */
    if ( conf->make_backup_every_minutes
     &&
     conf->make_backup_nb_minutes )
        gsb_file_automatic_backup_start ( NULL, NULL );

    conf->compress_backup = g_key_file_get_integer ( config,
                        "Backup",
                        "Compress backup",
                        NULL );

    conf->sauvegarde_demarrage = g_key_file_get_integer ( config,
                        "Backup",
                        "Save_at_opening",
                        NULL );

    gsb_file_set_backup_path ( g_key_file_get_string ( config,
                        "Backup",
                        "Backup path",
                        NULL ),
                        conf );

    /* get input/output */
    conf->load_last_file = g_key_file_get_integer ( config,
                        "IO",
                        "Load last file",
                        NULL );

    conf->sauvegarde_auto = g_key_file_get_integer ( config,
                        "IO",
                        "Save at closing",
                        NULL );

    conf->nb_max_derniers_fichiers_ouverts = g_key_file_get_integer ( config,
                        "IO",
                        "Nb_max_derniers_fichiers_ouverts",
                        NULL );

    conf->compress_file = g_key_file_get_integer ( config,
                        "IO",
                        "Compress file",
                        NULL );

    conf->force_enregistrement = g_key_file_get_integer ( config,
                        "IO",
                        "Force saving",
                        NULL );

    conf->tab_noms_derniers_fichiers_ouverts = g_key_file_get_string_list ( config,
                        "IO",
                        "Names last files",
                        ( gsize *) &conf->nb_derniers_fichiers_ouverts,
                        NULL );

    gsb_file_set_account_files_path ( g_key_file_get_string ( config,
                        "IO",
                        "Account_files_path",
                        NULL ), conf );

    /* archival part */
    conf->check_for_archival = g_key_file_get_integer ( config,
                        "IO",
                        "Check_archival_at_opening",
                        NULL );

    conf->max_non_archived_transactions_for_check = g_key_file_get_integer ( config,
                        "IO",
                        "Max_transactions_before_warn_archival",
                        NULL );
    
    /* get scheduled section */
    conf->nb_days_before_scheduled = g_key_file_get_integer ( config,
                        "Scheduled",
                        "Days before remind",
                        NULL );

    conf->execute_scheduled_of_month = g_key_file_get_integer ( config,
                        "Scheduled",
                        "Execute scheduled of month",
                        NULL );

    int_ret = g_key_file_get_integer ( config,
                        "Scheduled",
                        "Balances with scheduled",
                        &err );
    if ( err == NULL )
        conf->balances_with_scheduled = int_ret;
    else
        err = NULL;

    /* get shown section */
    conf->formulaire_toujours_affiche = g_key_file_get_integer ( config,
                        "Display",
                        "Show transaction form",
                        NULL );

    conf->show_transaction_selected_in_form = g_key_file_get_integer ( config,
                        "Display",
                        "Show selected transaction in form",
                        NULL );

    conf->show_transaction_gives_balance = g_key_file_get_integer ( config,
                        "Display",
                        "Show transaction gives balance",
                        NULL );

    conf->transactions_list_primary_sorting = g_key_file_get_integer ( config,
                        "Display",
                        "Transactions_list_primary_sorting",
                        NULL );

    conf->transactions_list_secondary_sorting = g_key_file_get_integer ( config,
                        "Display",
                        "Transactions_list_secondary_sorting",
                        NULL );

    conf->affichage_exercice_automatique = g_key_file_get_integer ( config,
                        "Display",
                        "Show automatic financial year",
                        NULL );
    
    int_ret = g_key_file_get_integer ( config,
                        "Display",
                        "Automatic completion payee",
                        &err );
    if ( err == NULL )
        conf->automatic_completion_payee = int_ret;
    else
        err = NULL;

    conf->limit_completion_to_current_account = g_key_file_get_integer ( config,
                        "Display",
                        "Limit payee completion",
                        NULL );

    conf->automatic_recover_splits = g_key_file_get_integer ( config,
                        "Display",
                        "Automatic_recover_splits",
                        NULL );

    conf->automatic_erase_credit_debit = g_key_file_get_integer ( config,
                        "Display",
                        "Automatic_erase_credit_debit",
                        NULL );

    conf->display_toolbar = g_key_file_get_integer ( config,
                        "Display",
                        "Display toolbar",
                        NULL );

    conf->active_scrolling_left_pane = g_key_file_get_integer ( config,
                        "Display",
                        "Active_scrolling_left_pane",
                        NULL );

    conf->show_headings_bar = g_key_file_get_integer ( config,
                        "Display",
                        "Show headings bar",
                        NULL );

    conf->show_closed_accounts = g_key_file_get_integer ( config,
                        "Display",
                        "Show closed accounts",
                        NULL );

    conf->display_grisbi_title = g_key_file_get_integer ( config,
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

    conf->last_tip = g_key_file_get_integer ( config,
                        "Messages",
                        "Last tip",
                        NULL );

    conf->show_tip = g_key_file_get_integer ( config,
                        "Messages",
                        "Show tip",
                        NULL );

    g_key_file_free ( config );

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
gboolean gsb_file_config_save_config ( GrisbiAppConf *conf )
{
    GKeyFile *config;
    gchar *file_content;
    gchar *name;
    gsize length;
    FILE *conf_file;
    gint i;
    
    devel_debug ( conf_filename );

    config = g_key_file_new ();

    if ( IS_DEVELOPMENT_VERSION )
    {
        /* set config model */
        g_key_file_set_integer ( config,
                        "Model",
                        "Stable_config_file_model",
                        conf->stable_config_file_model );
    }

    g_key_file_set_integer ( config,
                        "Geometry",
                        "Root_x",
                        conf->root_x );

    g_key_file_set_integer ( config,
                        "Geometry",
                        "Root_y",
                        conf->root_y );

    g_key_file_set_integer ( config,
                        "Geometry",
                        "Width",
                        conf->main_width );

    g_key_file_set_integer ( config,
                        "Geometry",
                        "Height",
                        conf->main_height );

    g_key_file_set_integer ( config,
                        "Geometry",
                        "Full_screen",
                        conf->full_screen );

    g_key_file_set_integer ( config,
                        "Geometry",
                        "Maximize_screen",
                        conf->maximize_screen );

    /* Remember size of main panel */
    g_key_file_set_integer ( config,
                        "Geometry",
                        "Panel_width",
                        conf->panel_width );

    /* prefs */
    g_key_file_set_integer ( config,
                        "Geometry",
                        "Prefs_width",
                        conf->prefs_width );

    g_key_file_set_integer ( config,
                        "Geometry",
                        "Prefs_height",
                        conf->prefs_height );

    /* Remember size of main panel */
    g_key_file_set_integer ( config,
                        "Geometry",
                        "Prefs_panel_width",
                        conf->prefs_panel_width );

    /* save general */
    g_key_file_set_integer ( config,
                        "General",
                        "Can modify R",
                        conf->r_modifiable );

    g_key_file_set_string ( config,
                        "General",
                        "Path",
                        gsb_file_get_last_path () );

    g_key_file_set_integer ( config,
                        "General",
                        "Show permission alert",
                        conf->alerte_permission );

    g_key_file_set_integer ( config,
                        "General",
                        "Function of enter",
                        conf->entree );

    g_key_file_set_integer ( config,
                        "General",
                        "Show alert messages",
                        conf->alerte_mini );

    g_key_file_set_integer ( config,
                        "General",
                        "Use user font",
                        conf->utilise_fonte_listes );

    if ( conf->font_string )
        g_key_file_set_string ( config,
                        "General",
                        "Font name",
                        conf->font_string );

    if ( conf->browser_command )
    {
        gchar *string;

        string = my_strdelimit ( conf->browser_command, "&", "\\e" );
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
                        conf->pluriel_final );

    g_key_file_set_integer ( config,
                        "General",
                        "Metatree_action_2button_press",
                        conf->metatree_action_2button_press );

     /* save backup part */
    g_key_file_set_integer ( config,
                        "Backup",
                        "Make backup",
                        conf->make_backup );

    g_key_file_set_integer ( config,
                        "Backup",
                        "Make backup single file",
                        conf->make_bakup_single_file );

    g_key_file_set_integer ( config,
                        "Backup",
                        "Make backup every x minutes",
                        conf->make_backup_every_minutes );

    g_key_file_set_integer ( config,
                        "Backup",
                        "Compress backup",
                        conf->compress_backup );

    g_key_file_set_integer ( config,
                        "Backup",
                        "Make backup nb minutes",
                        conf->make_backup_nb_minutes );

    g_key_file_set_string ( config,
                        "Backup",
                        "Backup path",
                        gsb_file_get_backup_path () );

    g_key_file_set_integer ( config,
                        "Backup",
                        "Save_at_opening",
                        conf->sauvegarde_demarrage );

    /* save input/output */
    g_key_file_set_integer ( config,
                        "IO",
                        "Load last file",
                        conf->load_last_file );

    g_key_file_set_integer ( config,
                        "IO",
                        "Save at closing",
                        conf->sauvegarde_auto );

    g_key_file_set_integer ( config,
                        "IO",
                        "Nb_max_derniers_fichiers_ouverts",
                        conf->nb_max_derniers_fichiers_ouverts );

    g_key_file_set_integer ( config,
                        "IO",
                        "Compress file",
                        conf->compress_file );

    g_key_file_set_integer ( config,
                        "IO",
                        "Force saving",
                        conf->force_enregistrement );

    if ( conf->nb_derniers_fichiers_ouverts > 0
     &&
     conf->tab_noms_derniers_fichiers_ouverts )
        g_key_file_set_string_list ( config,
                        "IO",
                        "Names last files",
                        (const gchar **) conf->tab_noms_derniers_fichiers_ouverts,
                        conf->nb_derniers_fichiers_ouverts);

    g_key_file_set_string ( config,
                        "IO",
                        "Account_files_path",
                        gsb_file_get_account_files_path () );

    /* archival part */
    g_key_file_set_integer ( config,
                        "IO",
                        "Check_archival_at_opening",
                        conf->check_for_archival );

    g_key_file_set_integer ( config,
                        "IO",
                        "Max_transactions_before_warn_archival",
                        conf->max_non_archived_transactions_for_check );

    /* save scheduled section */
    g_key_file_set_integer ( config,
                        "Scheduled",
                        "Days before remind",
                        conf->nb_days_before_scheduled );

    g_key_file_set_integer ( config,
                        "Scheduled",
                        "Execute scheduled of month",
                        conf->execute_scheduled_of_month );

    g_key_file_set_integer ( config,
                        "Scheduled",
                        "Balances with scheduled",
                        conf->balances_with_scheduled );

    /* save shown section */
    g_key_file_set_integer ( config,
                        "Display",
                        "Show transaction form",
                        conf->formulaire_toujours_affiche );

    g_key_file_set_integer ( config,
                        "Display",
                        "Show selected transaction in form",
                        conf->show_transaction_selected_in_form );

    g_key_file_set_integer ( config,
                        "Display",
                        "Show transaction gives balance",
                        conf->show_transaction_gives_balance );

    g_key_file_set_integer ( config,
                        "Display",
                        "Transactions_list_primary_sorting",
                        conf->transactions_list_primary_sorting );

    g_key_file_set_integer ( config,
                        "Display",
                        "Transactions_list_secondary_sorting",
                        conf->transactions_list_secondary_sorting );

    g_key_file_set_integer ( config,
                        "Display",
                        "Show automatic financial year",
                        conf->affichage_exercice_automatique );

    g_key_file_set_integer ( config,
                        "Display",
                        "Automatic completion payee",
                        conf->automatic_completion_payee );

    g_key_file_set_integer ( config,
                        "Display",
                        "Limit payee completion",
                        conf->limit_completion_to_current_account );

    g_key_file_set_integer ( config,
                        "Display",
                        "Automatic_recover_splits",
                        conf->automatic_recover_splits );

    g_key_file_set_integer ( config,
                        "Display",
                        "Automatic_erase_credit_debit",
                        conf->automatic_erase_credit_debit );

    g_key_file_set_integer ( config,
                        "Display",
                        "Display toolbar",
                        conf->display_toolbar );

    g_key_file_set_integer ( config,
                        "Display",
                        "Active_scrolling_left_pane",
                        conf->active_scrolling_left_pane );

    g_key_file_set_integer ( config,
                        "Display",
                        "Show headings bar",
                        conf->show_headings_bar );

    g_key_file_set_integer ( config,
                        "Display",
                        "Show closed accounts",
                        conf->show_closed_accounts );

    g_key_file_set_integer ( config,
                        "Display",
                        "Display grisbi title",
                        conf->display_grisbi_title );

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
                        conf->last_tip );

    g_key_file_set_integer ( config,
                        "Messages",
                        "Show tip",
                        conf->show_tip );

    /* save into a file */
    file_content = g_key_file_to_data ( config, &length, NULL );

    conf_file = fopen ( conf_filename, "w" );

    #ifndef _WIN32
    if ( !conf_file )
    {
        utils_files_create_XDG_dir ( );
        conf_file = fopen ( conf_filename, "w" );
    }
    #endif

    if ( !conf_file
     ||
     !fwrite ( file_content, sizeof ( gchar ), length, conf_file ) )
    {
        gchar* tmpstr = g_strdup_printf ( _("Cannot save configuration file '%s': %s"),
                        conf_filename,
                        g_strerror ( errno ) );
        dialogue_error ( tmpstr );
        g_free ( tmpstr );
        g_free ( file_content);
        g_key_file_free (config);
        return ( FALSE );
    }
    
    fclose (conf_file);
    g_free ( file_content);
    g_key_file_free (config);

    return TRUE;
}



/**
 * Set all the config variables to their default values.
 * called before loading the config
 * or for a new opening
 *
 * \param
 *
 * \return
 */
void gsb_file_config_clean_config ( GrisbiAppConf *conf )
{
    devel_debug (NULL);

    conf->main_width = 800;
    conf->main_height = 600;
    conf->panel_width = 250;
    conf->prefs_width = 720;
    conf->main_height = 450;

    conf->force_enregistrement = 1;

    conf->r_modifiable = 0;                         /* we can not change the reconciled transaction */
    conf->load_last_file = 0;                       /* on n'ouvre pas directement le dernier fichier */
    conf->sauvegarde_auto = 1;                      /* on sauvegarde automatiquement par défaut */
    conf->entree = 1;                               /* la touche entree provoque l'enregistrement de l'opération */
    conf->nb_days_before_scheduled = 0;             /* nb de jours avant l'échéance pour prévenir */
    conf->execute_scheduled_of_month = FALSE;       /* exécution automatique à l'échéance. */
    conf->balances_with_scheduled = TRUE;
    conf->formulaire_toujours_affiche = 0;           /* le formulaire ne s'affiche que lors de l'edition d'1 opé */
    conf->affichage_exercice_automatique = 0;        /* l'exercice est choisi en fonction de la date */
    conf->automatic_completion_payee = 1;            /* by default automatic completion */
    conf->limit_completion_to_current_account = 0;   /* By default, do full search */
    conf->automatic_recover_splits = 1;
    conf->automatic_erase_credit_debit = 0;

    conf->display_grisbi_title = GSB_ACCOUNTS_TITLE; /* show Accounts file title par défaut */
    conf->display_toolbar = GSB_BUTTON_BOTH;         /* How to display toolbar icons. */
    conf->active_scrolling_left_pane = FALSE;        /* Active_scrolling_left_pane or not. */
    conf->show_headings_bar = TRUE;                  /* Show toolbar or not. */
    conf->show_transaction_selected_in_form = 1;     /* show selected transaction in form */
    conf->show_transaction_gives_balance = 1;        /* show transaction that gives the balance of the day */
    conf->transactions_list_primary_sorting = 1;     /* Primary sorting option for the transactions */
    conf->transactions_list_secondary_sorting = 0;   /* Secondary sorting option for the transactions */
    conf->show_closed_accounts = FALSE;

    if ( conf->font_string )
    {
        g_free ( conf->font_string );
        conf->font_string = NULL;
    }

    conf->force_enregistrement = 1;                     /* par défaut, on force l'enregistrement */
    gsb_file_update_last_path ( g_get_home_dir () );
    gsb_file_set_account_files_path ( g_get_home_dir (), conf );
    gsb_file_set_backup_path ( gsb_dirs_get_user_data_dir (), conf );
    conf->make_backup = 1;
    conf->make_backup_every_minutes = FALSE;
    conf->make_backup_nb_minutes = 0;

    conf->stable_config_file_model = 0;

    conf->nb_derniers_fichiers_ouverts = 0;
    conf->nb_max_derniers_fichiers_ouverts = 3;

    /* no compress by default */
    conf->compress_file = 0;
    conf->compress_backup = 0;

    /* archive data */
    conf->check_for_archival = TRUE;
    conf->max_non_archived_transactions_for_check = 3000;

    conf->last_tip = -1;
    conf->show_tip = FALSE;

    /* mise en conformité avec les recommandations FreeDesktop. */
    if ( conf->browser_command )
        g_free ( conf->browser_command );
    conf->browser_command = g_strdup ( ETAT_WWW_BROWSER );

    conf->metatree_action_2button_press = 0;     /* action par défaut pour le double clic sur division */
}


/**
 * renvoie le nom du fichier de configuration
 *
 * \param
 *
 * \return conf_filename
 */
const gchar *gsb_config_get_conf_filename ( void )
{
    return conf_filename;
}


/**
 * initialise le nom du fichier de configuration
 *
 * \param
 *
 * \return
 */
void gsb_config_initialise_conf_filename ( const gchar *config_file )
{
    gchar *tmp_str;

    if ( config_file )
    {
        conf_filename = g_strdup ( config_file );
        return;
    }

    if ( IS_DEVELOPMENT_VERSION )
    {
        tmp_str = g_strconcat ( PACKAGE, "dev.conf", NULL );
        conf_filename = g_build_filename ( gsb_dirs_get_user_config_dir ( ), tmp_str, NULL );
        g_free ( tmp_str );
    }
    else
    {
        tmp_str = g_strconcat ( PACKAGE, ".conf", NULL );
        conf_filename = g_build_filename ( gsb_dirs_get_user_config_dir ( ), tmp_str, NULL );
        g_free ( tmp_str );
    }
}


/**
 * libère la mémoire occupée par le nom du fichier de configuration
 *
 * \param
 *
 * \return
 */
void gsb_config_free_conf_filename ( void )
{
    if ( conf_filename )
        g_free ( conf_filename );
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
