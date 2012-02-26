/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2012 Pierre Biava (grisbi@pierre.biava.name)                    */
/*          http://www.grisbi.org                                                */
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
#include <config.h>
#endif

#include <stdlib.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include <goffice/goffice.h>

/*START_INCLUDE*/
#include "main.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "gsb_assistant_first.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "gsb_file_config.h"
#include "gsb_locale.h"
#include "gsb_plugins.h"
#include "parse_cmdline.h"
#include "tip.h"
#include "erreur.h"
/*END_INCLUDE*/


/*START_STATIC*/
static void gsb_main_grisbi_shutdown ( GrisbiCommandLine *command_line );
static gboolean gsb_main_init_app ( void );
static void gsb_main_load_file_if_necessary ( GrisbiCommandLine *command_line );
static gboolean gsb_main_print_environment_var ( void );
static gint gsb_main_set_debug_level ( GrisbiCommandLine *command_line );
static void gsb_main_trappe_signal_sigsegv ( void );
static void gsb_main_window_set_size_and_position ( void );
/*END_STATIC*/


/*START_EXTERN*/
/*END_EXTERN*/

/* variables initialisées lors de l'exécution de grisbi */
struct gsb_run_t run;


/**
 * Main function
 *
 * @param argc number of arguments
 * @param argv arguments
 *
 * @return Nothing
 */
gint main ( int argc, char **argv )
{
    GrisbiApp *app;
    GrisbiWindow *window;
    GrisbiCommandLine *command_line;
    gint return_value;

    /* Init type system */
    g_type_init ();

    /* Init glib threads asap */
    if ( g_thread_get_initialized () )
        g_thread_init ( NULL );

    /* initialisation des différents répertoires */
    gsb_dirs_init ();

    /* Setup locale/gettext */
    setlocale ( LC_ALL, "" );
    bindtextdomain ( PACKAGE, gsb_dirs_get_locale_dir () );
    bind_textdomain_codeset ( PACKAGE, "UTF-8" );
    textdomain ( PACKAGE );

    /* Setup command line options */
    command_line = grisbi_command_line_get_default ();

    return_value = grisbi_command_line_parse ( command_line, argc, argv );
    if ( return_value == 1 )
        exit ( return_value );

    /* initialisation du nom du fichier de configuration */
    gsb_config_initialise_conf_filename ( grisbi_command_line_get_config_file ( command_line ) );

    /* initialisation du mode de débogage */
    initialize_debugging ( grisbi_command_line_get_debug_level ( command_line ) );

    /* initialisation de la variable locale pour les devises */
    gsb_locale_init ( );

    /* lancement des information de débogage si nécessaire */
    if ( gsb_debug_get_debug_level () > 0 )
        gsb_main_print_environment_var ();

    /* initialisation de gtk. arguments à NULL car traités au dessus */
    gtk_init ( NULL, NULL );

    /* initialisation de libgoffice et des plugins pour goffice*/
    libgoffice_init ();
    go_plugins_init ( NULL, NULL, NULL, NULL, TRUE, GO_TYPE_PLUGIN_LOADER_MODULE );

    /* on commence par détourner le signal SIGSEGV */
    gsb_main_trappe_signal_sigsegv ();

    /* création de l'application */
    app = grisbi_app_get_default ();

    /* create the toplevel window and the main menu */
    window = grisbi_app_create_window ( app, NULL );

    gtk_widget_show ( GTK_WIDGET ( window ) );

    if ( IS_DEVELOPMENT_VERSION )
        dialog_message ( "development-version", VERSION );

    /* check the command line, if there is something to open */
    gsb_main_load_file_if_necessary ( command_line );

    if ( grisbi_app_get_first_use ( app ) )
        gsb_assistant_first_run ();
    else
        display_tip ( FALSE );

    gtk_main ();

    gsb_main_grisbi_shutdown ( command_line );

    /* return */
    exit ( 0 );
}


/**
 * affiche les variables d'environnement de Grisbi
 *
 *
 *
 * */
gboolean gsb_main_print_environment_var ( void )
{
    gchar *tmp_str;

    g_print ("\nGrisbi version %s\n\n", VERSION );

    g_print ("Variables d'environnement :\n" );

    tmp_str = gsb_locale_get_print_locale_var ( );
    g_print ("%s", tmp_str);

    g_free ( tmp_str );

    g_print ( "gint64\n\tG_GINT64_MODIFIER = \"%s\"\n"
                        "\t%"G_GINT64_MODIFIER"d\n\n",
                        G_GINT64_MODIFIER,
                        G_MAXINT64 );

    tmp_str = gsb_dirs_get_print_dir_var ( );
    g_print ("%s", tmp_str);

    g_free ( tmp_str );

    return FALSE;
}


/**
 * On detourne les signaaux SIGINT, SIGTERM, SIGSEGV
 *
 *
 *
 * */
void gsb_main_trappe_signal_sigsegv ( void )
{
#ifndef G_OS_WIN32
    struct sigaction sig_sev;

    memset ( &sig_sev, 0, sizeof ( struct sigaction ) );
    sig_sev.sa_handler = traitement_sigsegv;
    sig_sev.sa_flags = 0;
    sigemptyset ( &( sig_sev.sa_mask ) );

    if ( sigaction ( SIGINT, &sig_sev, NULL ) )
        g_print ( _("Error on sigaction: SIGINT won't be trapped\n") );

    if ( sigaction ( SIGTERM, &sig_sev, NULL ) )
        g_print ( _("Error on sigaction: SIGTERM won't be trapped\n") );

    if ( sigaction ( SIGSEGV, &sig_sev, NULL ) )
        g_print ( _("Error on sigaction: SIGSEGV won't be trapped\n") );
#endif /* G_OS_WIN32 */
}


/**
 * Load file if necessary
 *
 *
 *
 * */
void gsb_main_load_file_if_necessary ( GrisbiCommandLine *command_line )
{
    GrisbiAppConf *conf;
    GSList *file_liste;

    conf = grisbi_app_get_conf ( );

    /* check the command line, if there is something to open */
    file_liste = grisbi_command_line_get_file_list ( command_line );

    if ( file_liste )
    {
        gsb_file_open_from_commandline ( file_liste );
    }
    else
    {
        /* open the last file if needed */
        if ( conf->load_last_file && 
             conf->nb_derniers_fichiers_ouverts > 0 &&
             conf->tab_noms_derniers_fichiers_ouverts[0] )
        {
            gsb_file_open_file ( conf->tab_noms_derniers_fichiers_ouverts[0] );
        }
    }

    return;
}


/**
 * renvoie la version de Grisbi
 *
 * \param
 *
 * \return
 */
void gsb_main_show_version ( void )
{
#ifdef HAVE_PLUGINS
    gsb_plugins_scan_dir ( gsb_dirs_get_plugins_dir ( ) );
#endif

g_print ( N_("Grisbi version %s, %s\n"), VERSION, gsb_plugin_get_list ( ) );

    exit ( 0 );
}


/**
 *  procédure appelée après gtk_main_quit termine Grisbi
 *
 * \param
 *
 * \return
 */
void gsb_main_grisbi_shutdown ( GrisbiCommandLine *command_line )
{
    devel_debug (NULL);

    g_object_unref ( command_line );

    /* libération de mémoire */
    gsb_config_free_conf_filename ( );
    gsb_locale_shutdown ( );
    gsb_dirs_shutdown ( );

    /* liberation libgoffice */
    libgoffice_shutdown ( );
}


/**
 *
 *
 * \param
 *
 * \return
 */


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
