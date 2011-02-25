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

/*START_INCLUDE*/
#include "gsb_file_config.h"
#include "dialog.h"
#include "gsb_file.h"
#include "main.h"
#include "print_config.h"
#include "structures.h"
#include "utils_buttons.h"
#include "utils_files.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gchar *gsb_config_get_old_conf_name ( void );
static void gsb_file_config_clean_config ( void );
static void gsb_file_config_get_xml_text_element ( GMarkupParseContext *context,
                        const gchar *text,
                        gsize text_len,  
                        gpointer user_data,
                        GError **error);
static gboolean gsb_file_config_load_last_xml_config ( gchar *filename );
static void gsb_file_config_remove_old_config_file ( gchar *filename );
/*END_STATIC*/


/*START_EXTERN*/
extern struct conditional_message delete_msg[];
extern gboolean execute_scheduled_of_month;
extern GtkWidget *main_hpaned;
extern struct conditional_message messages[];
extern gint nb_days_before_scheduled;
extern gchar *nom_fichier_comptes;
extern GtkWidget *window;
/*END_EXTERN*/

/* global variable, see structures.h */
struct gsb_conf_t conf;

gint mini_paned_width = 0;

/* contient le nb de derniers fichiers ouverts */
gsize nb_derniers_fichiers_ouverts = 0;

/* contient le nb max que peut contenir nb_derniers_fichiers_ouverts ( réglé dans les paramètres ) */
gint nb_max_derniers_fichiers_ouverts = 0;
gchar **tab_noms_derniers_fichiers_ouverts = NULL;

#if IS_DEVELOPMENT_VERSION == 1
/* flag de chargement du fichier modèle */
static gboolean used_model = FALSE;
#endif


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
    gchar *filename;
    gchar *name;
    gint i;
    gint int_ret;
    GError* err = NULL;
devel_debug (NULL);
    gsb_file_config_clean_config ();

    filename = g_strconcat ( my_get_XDG_grisbirc_dir(), C_GRISBIRC, NULL );
#if IS_DEVELOPMENT_VERSION == 1
    if ( !g_file_test (filename, G_FILE_TEST_EXISTS) )
    {
        g_free ( filename );
        filename = g_strconcat ( my_get_XDG_grisbirc_dir(), "/", PACKAGE, ".conf", NULL );
        used_model = FALSE;
    }
#endif

    config = g_key_file_new ();
    
    result = g_key_file_load_from_file ( config,
                        filename,
                        G_KEY_FILE_KEEP_COMMENTS,
                        NULL );
    /* if key_file couldn't load the conf, it's because it's the last
     * conf (HOME or xml) or no conf... try the HOME conf and the xml conf */
    if (!result)
    {
        /* On recherche le fichier dans HOME */
#ifndef _WIN32
        /* On recherche les fichiers possibles seulement sous linux */
        g_free ( filename );
        filename = gsb_config_get_old_conf_name ( );
        devel_debug (filename);
        if ( ! filename || strlen ( filename ) == 0 )
            return FALSE;
#else
        filename = g_strconcat ( my_get_grisbirc_dir(), C_OLD_GRISBIRC, NULL );
#endif
        
        config = g_key_file_new ();
        
        result = g_key_file_load_from_file ( config,
                        filename,
                        G_KEY_FILE_KEEP_COMMENTS,
                        NULL );
        /* si on ne le trouve pas on recherche le fichier au format xml */
        if (!result)
        {
            result = gsb_file_config_load_last_xml_config ( filename );
            if ( result )
                gsb_file_config_remove_old_config_file ( filename );
            g_free (filename);
            return result;
        }
        else
            g_unlink ( filename );
    }

#if IS_DEVELOPMENT_VERSION == 1
    /* get config model */
    conf.stable_config_file_model = g_key_file_get_integer ( config,
                        "Model",
                        "Stable_config_file_model",
                        NULL );
    if ( conf.stable_config_file_model )
    {
        used_model = TRUE;
        filename = g_strconcat ( my_get_XDG_grisbirc_dir(), "/", PACKAGE, ".conf", NULL );
        if ( !g_file_test (filename, G_FILE_TEST_EXISTS) )
            return FALSE;

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
                        "Full screen",
                        NULL );

    conf.largeur_colonne_comptes_operation = g_key_file_get_integer ( config,
                        "Geometry",
                        "Panel width",
                        NULL );
    if ( conf.largeur_colonne_comptes_operation < mini_paned_width )
        conf.largeur_colonne_comptes_operation = mini_paned_width;

    conf.prefs_width = g_key_file_get_integer ( config,
                        "Geometry",
                        "Prefs width",
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
    
    conf.latex_command = g_key_file_get_string ( config,
                        "General",
                        "Latex command",
                        NULL );

    conf.dvips_command = g_key_file_get_string ( config,
                        "General",
                        "Dvips command",
                        NULL );

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

    nb_max_derniers_fichiers_ouverts = g_key_file_get_integer ( config,
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
                        &nb_derniers_fichiers_ouverts,
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

    execute_scheduled_of_month = g_key_file_get_integer ( config,
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

    /* get shown section */
    etat.formulaire_toujours_affiche = g_key_file_get_integer ( config,
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

    etat.largeur_auto_colonnes = g_key_file_get_integer ( config,
                        "Display",
                        "Columns width auto",
                        NULL );

    etat.affichage_exercice_automatique = g_key_file_get_integer ( config,
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

    etat.display_toolbar = g_key_file_get_integer ( config,
                        "Display",
                        "Display toolbar",
                        NULL );

    etat.show_toolbar = g_key_file_get_integer ( config,
                        "Display",
                        "Show toolbar",
                        NULL );

    etat.show_headings_bar = g_key_file_get_integer ( config,
                        "Display",
                        "Show headings bar",
                        NULL );

    etat.show_closed_accounts = g_key_file_get_integer ( config,
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

    etat.last_tip = g_key_file_get_integer ( config,
                        "Messages",
                        "Last tip",
                        NULL );

    etat.show_tip = g_key_file_get_integer ( config,
                        "Messages",
                        "Show tip",
                        NULL );

    /* get printer config */

    etat.print_config.printer = g_key_file_get_integer ( config,
                        "Print config",
                        "Printer",
                        NULL );

    etat.print_config.printer_name = g_key_file_get_string ( config,
                        "Print config",
                        "Printer name",
                        NULL );

    etat.print_config.printer_filename = g_key_file_get_string ( config,
                        "Print config",
                        "Printer filename",
                        NULL );

    etat.print_config.filetype = g_key_file_get_integer ( config,
                        "Print config",
                        "Filetype",
                        NULL );

    etat.print_config.orientation = g_key_file_get_integer ( config,
                        "Print config",
                        "Orientation",
                        NULL );

    /* get the paper config */

    etat.print_config.paper_config.name = g_key_file_get_string ( config,
                        "Paper config",
                        "Name",
                        NULL );

    etat.print_config.paper_config.width = g_key_file_get_integer ( config,
                        "Paper config",
                        "Width",
                        NULL );

    etat.print_config.paper_config.height = g_key_file_get_integer ( config,
                        "Paper config",
                        "Height",
                        NULL );

    g_free (filename);
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
    gchar *filename;
    gchar *file_content;
    gchar *name;
    gsize length;
    FILE *conf_file;
    gint i;
    
    devel_debug (NULL);

    filename = g_strconcat ( my_get_XDG_grisbirc_dir(), C_GRISBIRC, NULL );
    config = g_key_file_new ();

#if IS_DEVELOPMENT_VERSION == 1
    /* set config model */
    g_key_file_set_integer ( config,
                        "Model",
                        "Stable_config_file_model",
                        conf.stable_config_file_model );
#endif

    /* get the geometry */
    if ( GTK_WIDGET ( window) -> window ) 
        gtk_window_get_size ( GTK_WINDOW ( window ),
                        &conf.main_width, &conf.main_height);
    else 
    {
        conf.main_width = 0;
        conf.main_height = 0;
    }

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
                        "Full screen",
                        conf.full_screen );

    g_key_file_set_integer ( config,
                        "Geometry",
                        "Prefs width",
                        conf.prefs_width );

    /* Remember size of main panel */
    if (main_hpaned && GTK_IS_WIDGET (main_hpaned))
        conf.largeur_colonne_comptes_operation = gtk_paned_get_position (
                        GTK_PANED ( main_hpaned ) );
    g_key_file_set_integer ( config,
                        "Geometry",
                        "Panel width",
                        conf.largeur_colonne_comptes_operation );

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

    if ( conf.latex_command )
        g_key_file_set_string ( config,
                        "General",
                        "Latex command",
                        conf.latex_command );

    if ( conf.dvips_command )
        g_key_file_set_string ( config,
                        "General",
                        "Dvips command",
                        conf.dvips_command );

    if (conf.browser_command)
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
                        nb_max_derniers_fichiers_ouverts );

    g_key_file_set_integer ( config,
                        "IO",
                        "Compress file",
                        conf.compress_file );

    g_key_file_set_integer ( config,
                        "IO",
                        "Force saving",
                        conf.force_enregistrement );

    if ( nb_derniers_fichiers_ouverts > 0
     &&
     tab_noms_derniers_fichiers_ouverts)
        g_key_file_set_string_list ( config,
                        "IO",
                        "Names last files",
                        (const gchar **) tab_noms_derniers_fichiers_ouverts,
                        nb_derniers_fichiers_ouverts);

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
                        execute_scheduled_of_month );

    g_key_file_set_integer ( config,
                        "Scheduled",
                        "Balances with scheduled",
                        conf.balances_with_scheduled );

    /* save shown section */
    g_key_file_set_integer ( config,
                        "Display",
                        "Show transaction form",
                        etat.formulaire_toujours_affiche );

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
                        etat.affichage_exercice_automatique );

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
                        "Display toolbar",
                        etat.display_toolbar );

    g_key_file_set_integer ( config,
                        "Display",
                        "Show toolbar",
                        etat.show_toolbar );

    g_key_file_set_integer ( config,
                        "Display",
                        "Show headings bar",
                        etat.show_headings_bar );

    g_key_file_set_integer ( config,
                        "Display",
                        "Show closed accounts",
                        etat.show_closed_accounts );

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
                        etat.last_tip );

    g_key_file_set_integer ( config,
                        "Messages",
                        "Show tip",
                        etat.show_tip );

    /* save printer config */
    g_key_file_set_integer ( config,
                        "Print config",
                        "Printer",
                        etat.print_config.printer );

    if ( etat.print_config.printer_name )
        g_key_file_set_string ( config,
                        "Print config",
                        "Printer name",
                        etat.print_config.printer_name );

    if ( etat.print_config.printer_filename )
        g_key_file_set_string ( config,
                        "Print config",
                        "Printer filename",
                        etat.print_config.printer_filename );

    g_key_file_set_integer ( config,
                        "Print config",
                        "Filetype",
                        etat.print_config.filetype );

    g_key_file_set_integer ( config,
                        "Print config",
                        "Orientation",
                        etat.print_config.orientation );

    /* save the paper config */
    if ( etat.print_config.paper_config.name )
        g_key_file_set_string ( config,
                        "Paper config",
                        "Name",
                        etat.print_config.paper_config.name );

    g_key_file_set_integer ( config,
                        "Paper config",
                        "Width",
                        etat.print_config.paper_config.width );

    g_key_file_set_integer ( config,
                        "Paper config",
                        "Height",
                        etat.print_config.paper_config.height );

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
        g_free (filename);
        g_key_file_free (config);
        return ( FALSE );
    }
    
    fclose (conf_file);
    g_free ( file_content);
    g_free (filename);
    g_key_file_free (config);

    return TRUE;
}



/**
 * load the xml file config for grisbi before 0.6.0
 * try to find it, if not, return FALSE
 * */
gboolean gsb_file_config_load_last_xml_config ( gchar *filename )
{
    gchar *file_content;
    gsize length;

    devel_debug (filename);

    /* check if the file exists */
    if ( !g_file_test ( filename,
			G_FILE_TEST_EXISTS ))
	return FALSE;

    /* check here if it's not a regular file */
    if ( !g_file_test ( filename,
			G_FILE_TEST_IS_REGULAR ))
    {
        gchar* tmpstr = g_strdup_printf ( _("%s doesn't seem to be a regular config file,\nplease check it."),
					   filename );
	dialogue_error ( tmpstr);
        g_free ( tmpstr );
	return ( FALSE );
    }

    /* load the file */

    if ( g_file_get_contents ( filename,
			       &file_content,
			       &length,
			       NULL ))
    {
	GMarkupParser *markup_parser = g_malloc0 (sizeof (GMarkupParser));
	GMarkupParseContext *context;

	/* fill the GMarkupParser for the xml structure */
	
	markup_parser -> text = (void *) gsb_file_config_get_xml_text_element;

	context = g_markup_parse_context_new ( markup_parser,
					       0,
					       NULL,
					       NULL );

	g_markup_parse_context_parse ( context,
				       file_content,
				       strlen (file_content),
				       NULL );

	g_markup_parse_context_free (context);
	g_free (markup_parser);
	g_free (file_content);
    }
    else
    {
	gchar* tmpstr = g_strdup_printf (_("Cannot open config file '%s': %s"),
					 filename,
					 g_strerror(errno));
	dialogue_error ( tmpstr );
	g_free ( tmpstr );
	return FALSE;
    }

    return TRUE;
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
void gsb_file_config_get_xml_text_element ( GMarkupParseContext *context,
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
	conf.browser_command = my_strdelimit (text,
					      "\\e",
					      "&" );
	return;
    }
 
    if ( !strcmp ( element_name,
		   "Latex_command" ))
    {
	/* TODO dOm : fix memory leaks in this function (memory used by lvalue before setting its value */
	conf.latex_command = my_strdup (text);
	return;
    }
     if ( !strcmp ( element_name,
		   "Dvips_command" ))
    {
	conf.dvips_command = my_strdup (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Largeur_colonne_comptes_operation" ))
    {
	conf.largeur_colonne_comptes_operation = utils_str_atoi (text);
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
	nb_max_derniers_fichiers_ouverts = utils_str_atoi (text);
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
	    tab_noms_derniers_fichiers_ouverts = g_malloc0 ( nb_max_derniers_fichiers_ouverts * sizeof(gchar *) );

	tab_noms_derniers_fichiers_ouverts[nb_derniers_fichiers_ouverts] = my_strdup (text);
	nb_derniers_fichiers_ouverts++;
	return;
    }
 
    if ( !strcmp ( element_name,
		   "Delai_rappel_echeances" ))
    {
	nb_days_before_scheduled = utils_str_atoi (text);
	execute_scheduled_of_month = FALSE;
	return;
    }

    if ( !strcmp ( element_name,
		   "Affichage_formulaire" ))
    {
	etat.formulaire_toujours_affiche = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Largeur_auto_colonnes" ))
    {
	etat.largeur_auto_colonnes = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Affichage_exercice_automatique" ))
    {
	etat.affichage_exercice_automatique = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "display_toolbar" ))
    {
	etat.display_toolbar = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "show_closed_accounts" ))
    {
	etat.show_closed_accounts = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "show_tip" ))
    {
	etat.show_tip = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "last_tip" ))
    {
	etat.last_tip = utils_str_atoi (text);
	return;
    }

    for ( i = 0; messages[i].name; i++ )
    {
	if ( !strcmp ( element_name, messages[i].name ) )
	{
	    messages[i].hidden = utils_str_atoi (text);
	}
    }

    if ( !strcmp ( element_name,
		   "printer" ))
    {
	etat.print_config.printer = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "printer_name" ))
    {
	etat.print_config.printer_name = my_strdup (text);
	return;
    }
      if ( !strcmp ( element_name,
		   "printer_filename" ))
    {
	etat.print_config.printer_filename = my_strdup (text);
	return;
    }
      if ( !strcmp ( element_name,
		   "filetype" ))
    {
	etat.print_config.filetype = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "orientation" ))
    {
	etat.print_config.orientation = utils_str_atoi (text);
	return;
    }
}


/**
 * Set all the config variables to their default values.
 * called before loading the config
 * or for a new opening
 */
void gsb_file_config_clean_config ( void )
{
    devel_debug (NULL);

    conf.main_width = 0;
    conf.main_height = 0;
    conf.largeur_colonne_comptes_operation = mini_paned_width;
    conf.prefs_width = 600;

    conf.force_enregistrement = 1;

    conf.r_modifiable = 0;       /* we can not change the reconciled transaction */
    conf.dernier_fichier_auto = 1;   /*  on n'ouvre pas directement le dernier fichier */
    conf.sauvegarde_auto = 0;    /* on NE sauvegarde PAS * automatiquement par défaut */
    conf.entree = 1;    /* la touche entree provoque l'enregistrement de l'opération */
    nb_days_before_scheduled = 0;     /* nb de jours avant l'échéance pour prévenir */
    execute_scheduled_of_month = FALSE;
    conf.balances_with_scheduled = TRUE;
    etat.formulaire_toujours_affiche = 0;       /* le formulaire ne s'affiche que lors de l'edition d'1 opé */
    etat.affichage_exercice_automatique = 0;        /* l'exercice est choisi en fonction de la date */
    conf.automatic_completion_payee = 1;        /* by default automatic completion */
    conf.limit_completion_to_current_account = 0;        /* By default, do full search */
    conf.automatic_recover_splits = 1;

    conf.display_grisbi_title = GSB_ACCOUNTS_TITLE;  /* show Accounts file title par défaut */
    etat.display_toolbar = GSB_BUTTON_BOTH;         /* How to display toolbar icons. */
    etat.show_toolbar = TRUE;                       /* Show toolbar or not. */
    etat.show_headings_bar = TRUE;                  /* Show toolbar or not. */
    conf.show_transaction_selected_in_form = 1;     /* show selected transaction in form */
    conf.show_transaction_gives_balance = 1;        /* show transaction that gives the balance of the day */
    conf.transactions_list_primary_sorting = 1;     /* Primary sorting option for the transactions */
    conf.transactions_list_secondary_sorting = 0;   /* Secondary sorting option for the transactions */
    etat.show_closed_accounts = FALSE;

    if ( conf.font_string )
    {
    g_free ( conf.font_string );
    conf.font_string = NULL;
    }
    
    conf.force_enregistrement = 1;     /* par défaut, on force l'enregistrement */
    gsb_file_update_last_path (g_get_home_dir ());
    gsb_file_set_backup_path (my_get_XDG_grisbi_data_dir ());
    conf.make_backup = 1;
    conf.make_backup_every_minutes = FALSE;
    conf.make_backup_nb_minutes = 0;

#if IS_DEVELOPMENT_VERSION == 1
    conf.stable_config_file_model = 0;
#endif

    nb_derniers_fichiers_ouverts = 0;
    nb_max_derniers_fichiers_ouverts = 3;
    tab_noms_derniers_fichiers_ouverts = NULL;

    /* no compress by default */
    conf.compress_file = 0;
    conf.compress_backup = 0;

    /* archive data */
    conf.check_for_archival = TRUE;
    conf.max_non_archived_transactions_for_check = 3000;

    etat.largeur_auto_colonnes = 0;
    etat.retient_affichage_par_compte = 0;

    etat.last_tip = -1;
    etat.show_tip = FALSE;

    /* Commands */
    /* TODO dOm : use a copy of string so that we can free it */
    conf.latex_command = "latex";
    conf.dvips_command = "dvips";

    /* mise en conformité avec les recommandations FreeDesktop. */
    conf.browser_command = g_strdup (ETAT_WWW_BROWSER);

    conf.metatree_action_2button_press = 0;     /* action par défaut pour le double clic sur division */

    /* Print */
    etat.print_config.printer = 0;
#ifndef _WIN32
     etat.print_config.printer_name = "lpr";
#else
    etat.print_config.printer_name = "gsprint";
#endif
    etat.print_config.printer_filename = g_strdup ("");
    etat.print_config.filetype = POSTSCRIPT_FILE;
    etat.print_config.paper_config.name = _("A4");
    etat.print_config.paper_config.width = 21;
    etat.print_config.paper_config.height = 29.7;
    etat.print_config.orientation = LANDSCAPE;

    memset ( etat.csv_skipped_lines, '\0', sizeof(gboolean) * CSV_MAX_TOP_LINES );
}


static void gsb_file_config_remove_old_config_file ( gchar *filename )
{
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *hbox;
    GtkWidget *image;
    GtkWidget *label;
    gint resultat;
	gchar *tmpstr;

    dialog = gtk_dialog_new_with_buttons ( _("Delete config file from a previous version"),
                        GTK_WINDOW ( window ),
                        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                        GTK_STOCK_NO, GTK_RESPONSE_CANCEL,
                        GTK_STOCK_YES, GTK_RESPONSE_OK,
                        NULL );

    gtk_window_set_position ( GTK_WINDOW ( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );
    gtk_window_set_resizable ( GTK_WINDOW ( dialog ), FALSE );

    content_area = GTK_DIALOG(dialog) -> vbox;
    hbox = gtk_hbox_new ( FALSE, 5);
	gtk_container_set_border_width ( GTK_CONTAINER( hbox ), 6 );
    gtk_box_pack_start ( GTK_BOX ( content_area ), hbox, FALSE, FALSE, 5 );

    image = gtk_image_new_from_stock (GTK_STOCK_DIALOG_WARNING, 
                        GTK_ICON_SIZE_DIALOG );
    gtk_box_pack_start ( GTK_BOX ( hbox ), image, FALSE, FALSE, 5 );

    tmpstr = g_strconcat ( 
                        _("Caution, you are about to delete the\n"
                        "configuration file of a previous version of Grisbi.\n"
                        "\n<b>Do you want to delete this file?</b>"),
                        NULL );

    label = gtk_label_new ( tmpstr );
    gtk_label_set_use_markup ( GTK_LABEL( label ), TRUE );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );
    g_free ( tmpstr );

    gtk_widget_show_all ( dialog );

    resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ));

    if ( resultat == GTK_RESPONSE_OK )
        g_unlink ( filename );
    if ( GTK_IS_DIALOG ( dialog ) )
        gtk_widget_destroy ( GTK_WIDGET ( dialog ) );
}


gchar *gsb_config_get_old_conf_name ( void )
{
    GtkWidget *dialog;
    GtkWidget *hbox;
    GtkWidget *image;
    GtkWidget *label;
    GtkWidget *combo;
    GtkWidget *content_area;
    GDir *dir;
    gchar *filename = NULL;
    GSList *liste = NULL;
    GtkListStore *store;
    GtkTreeIter iter;
    GtkCellRenderer *renderer;
    GError *error = NULL;
    gint resultat;
    gint i = 0, j = 0;
	gchar *tmpstr;
    
    dir = g_dir_open ( my_get_grisbirc_dir ( ), 0, &error );
    if ( dir )
    {
        const gchar *name = NULL;
        
        while ( (name = g_dir_read_name ( dir ) ) )
        {
            if ( g_strstr_len ( name, -1, ".grisbi" ) &&
                        g_str_has_suffix ( name, "rc" ) )
                liste = g_slist_append ( liste, g_strdup ( name ) );
        }
    }
    else
    {
        dialogue_error ( error -> message );
        g_error_free ( error );
    }

    if ( g_slist_length ( liste ) == 0 )
        return NULL;

    if ( g_slist_length ( liste ) == 1 )
        return g_strconcat ( my_get_grisbirc_dir ( ),
                            G_DIR_SEPARATOR_S,
                            g_slist_nth_data ( liste, 0 ), 
                            NULL );

    dialog = gtk_dialog_new_with_buttons ( _("Choose a file"),
                        GTK_WINDOW ( window ),
                        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                        GTK_STOCK_OK, GTK_RESPONSE_OK,
                        NULL );

    gtk_window_set_position ( GTK_WINDOW ( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );
    gtk_window_set_resizable ( GTK_WINDOW ( dialog ), FALSE );
    gtk_window_set_position ( GTK_WINDOW ( dialog ), 
                        GTK_WIN_POS_CENTER_ON_PARENT );

    content_area = GTK_DIALOG(dialog) -> vbox;

    hbox = gtk_hbox_new ( FALSE, 5);
	gtk_container_set_border_width ( GTK_CONTAINER( hbox ), 6 );
    gtk_box_pack_start ( GTK_BOX ( content_area ), hbox, FALSE, FALSE, 5 );

    image = gtk_image_new_from_stock (GTK_STOCK_DIALOG_INFO, 
                        GTK_ICON_SIZE_DIALOG );
    gtk_box_pack_start ( GTK_BOX ( hbox ), image, FALSE, FALSE, 5 );

    tmpstr = g_strconcat ( 
                        _("Please Choose the name of file\n"
                        "of configuration.\n\n"
                        "and press the 'OK' button."),
                        NULL );

    label = gtk_label_new ( tmpstr );
    gtk_label_set_use_markup ( GTK_LABEL( label ), TRUE );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );
    g_free ( tmpstr );

    store = gtk_list_store_new ( 2, G_TYPE_STRING, G_TYPE_INT );
    while ( liste )
    {
        gtk_list_store_append ( store, &iter );
        gtk_list_store_set (store, &iter, 
                        0, (gchar *) liste -> data,
                        -1);
        if ( g_strcmp0 ( (gchar *) liste -> data, 
                        ( C_OLD_GRISBIRC + 1 ) ) == 0 )
            j = i;
        liste = liste -> next;
        i++;
    }

    combo = gtk_combo_box_new_with_model ( GTK_TREE_MODEL ( store ) );
    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo), renderer,
                                "text", 0,
                                NULL);
    gtk_combo_box_set_active ( GTK_COMBO_BOX ( combo ), j );
    gtk_box_pack_start ( GTK_BOX ( content_area ), combo, FALSE, FALSE, 5 );

    gtk_widget_show_all ( dialog );

    resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ));
    if ( resultat == GTK_RESPONSE_OK )
    {
        gtk_combo_box_get_active_iter ( GTK_COMBO_BOX ( combo ), &iter );
        gtk_tree_model_get ( GTK_TREE_MODEL ( store ),
                        &iter,
                        0, &filename,
                        -1 );
        filename = g_strconcat ( my_get_grisbirc_dir ( ),
                            G_DIR_SEPARATOR_S,
                            filename, 
                            NULL );
    }
    if ( GTK_IS_DIALOG ( dialog ) )
        gtk_widget_destroy ( GTK_WIDGET ( dialog ) );

    return filename;
}
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
