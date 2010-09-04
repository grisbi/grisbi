/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2010 Pierre Biava (grisbi@pierre.biava.name)                    */
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
# include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "main.h"
#include "menu.h"
#include "dialog.h"
#include "tip.h"
#include "utils.h"
#include "gsb_assistant_first.h"
#include "gsb_file.h"
#include "gsb_file_config.h"
#include "gsb_status.h"
#include "gsb_plugins.h"
#include "traitement_variables.h"
#include "erreur.h"
#include "parse_cmdline.h"
#include "import.h"
#include "parse_cmdline.h"
#include "gsb_file_config.h"
#include "structures.h"
#include "include.h"
#include "erreur.h"
/*END_INCLUDE*/

/* including resources so that the exe generated with MSVC has the right icon. */
#ifdef _MSC_VER
#include "resource.h"
#endif

/*START_STATIC*/
static gboolean gsb_grisbi_change_state_window ( GtkWidget *window,
                        GdkEventWindowState *event,
                        gpointer null );
static void gsb_grisbi_create_top_window ( void );
static gboolean gsb_grisbi_init_app ( void );
static gboolean gsb_grisbi_init_development_mode ( void );
static void gsb_grisbi_load_file_if_necessary ( cmdline_options  *opt );
static void gsb_grisbi_trappe_signal_sigsegv ( void );
static gboolean main_window_delete_event (GtkWidget *window, gpointer data);
static void main_window_destroy_event( GObject* obj, gpointer data);
/*END_STATIC*/

/* Fenetre de base, contient le menu et la fenetre d'utilisation */
G_MODULE_EXPORT GtkWidget *window = NULL;

/*START_EXTERN*/
extern FILE *debug_file;
extern gchar *nom_fichier_comptes;
/*END_EXTERN*/

#ifdef _MSC_VER
int APIENTRY wWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPWSTR    lpCmdLine,
                     int       nCmdShow)
{
	int argc, nLen, i;
	LPWSTR * argvP;
	char ** argv;
	argvP = CommandLineToArgvW(GetCommandLineW(), &(argc));
	argv = malloc (argc* sizeof(char *));
	for (i = 0 ; i<argc ; i++)
	{
		nLen = WideCharToMultiByte(CP_UTF8, 0,argvP[i], -1, NULL, 0, NULL, NULL);
		argv[i] = malloc((nLen + 1) * sizeof(char));
		WideCharToMultiByte(CP_UTF8, 0, argvP[i], -1, argv[i], nLen, NULL, NULL);
	}
	return main(argc, argv);
}
#endif


/**
 * Main function
 *
 * @param argc number of arguments
 * @param argv arguments
 *
 * @return Nothing
 */
int main (int argc, char **argv)
{
    gboolean first_use = FALSE;
    gint status = CMDLINE_SYNTAX_OK; /* be optimistic ;-) */
    cmdline_options  opt;

#if GSB_GMEMPROFILE
    g_mem_set_vtable(glib_mem_profiler_table);
#endif

#if IS_DEVELOPMENT_VERSION == 1
    gsb_grisbi_init_development_mode ( );
#endif

#ifdef _WIN32
    /* Retrieve exception information and store them under grisbi.rpt file!
     * see http://jrfonseca.dyndns.org/projects/gnu-win32/software/drmingw/index.html for more information */
    LoadLibrary("exchndl.dll");

    /* we store the path of the running file to use it for pixmaps, help and locales .... */
    win32_set_app_path(argv[0]);
    /* needed to be able to use the "common" installation of GTK libraries */
    win32_make_sure_the_gtk2_dlls_path_is_in_PATH();
#endif

    bindtextdomain (PACKAGE, LOCALEDIR);
    bind_textdomain_codeset (PACKAGE, "UTF-8");
    textdomain (PACKAGE);

	setlocale (LC_ALL, "");

    gtk_init(&argc, &argv);

#ifdef _WIN32
	win32_parse_gtkrc("gtkrc");
#endif

    /* on commence par détourner le signal SIGSEGV sauf sous Windows */
#ifndef _WIN32
    gsb_grisbi_trappe_signal_sigsegv ( );
#endif

    /* parse command line parameter, exit with correct error code when needed */
    if ( !parse_options (argc, argv, &opt, &status ) )
        exit ( status );

    /* initialise les données de l'application */
    first_use = gsb_grisbi_init_app ( );

    /* create the toplevel window */
    gsb_grisbi_create_top_window ( );
    gtk_widget_show ( window );

    /* check the command line, if there is something to open */
    gsb_grisbi_load_file_if_necessary ( &opt );

    if ( first_use && !nom_fichier_comptes )
        gsb_assistant_first_run ();
    else
        display_tip ( FALSE );

    gtk_main ();

    gsb_plugins_release ( );

    /* sauvegarde les raccourcis claviers */
    gtk_accel_map_save ( C_PATH_CONFIG_ACCELS );

#if GSB_GMEMPROFILE
    g_mem_profile();
#endif

    exit(0);
}


/**
 * gère le fait d'être en mode developpement sauf pour l'alerte
 *
 *
 *
 * */
gboolean gsb_grisbi_init_development_mode ( void )
{
	struct lconv *conv;

    initialize_debugging ( );

    /* test local pour les nombres */
	conv = localeconv();
    
    printf ("currency_symbol = %s\n"
            "mon_thousands_sep = \"%s\"\n"
            "mon_decimal_point = %s\n"
            "positive_sign = \"%s\"\n"
            "negative_sign = \"%s\"\n"
            "frac_digits = \"%d\"\n",
            conv->currency_symbol,
            g_locale_to_utf8 ( conv->mon_thousands_sep, -1, NULL, NULL, NULL ),
            g_locale_to_utf8 ( conv->mon_decimal_point, -1, NULL, NULL, NULL ),
            g_locale_to_utf8 ( conv->positive_sign, -1, NULL, NULL, NULL ),
            g_locale_to_utf8 ( conv->negative_sign, -1, NULL, NULL, NULL ),
            conv->frac_digits );

    return FALSE;
}


/**
 * charge le fichier de configuration et initialise les variabes.
 *
 *
 *
 * */
gboolean gsb_grisbi_init_app ( void )
{
    gboolean first_use = FALSE;
    gchar *string;

#ifdef HAVE_PLUGINS
    gsb_plugins_scan_dir ( PLUGINS_DIR );
#endif

    /* create the icon of grisbi (set in the panel of gnome or other) */
    string = g_build_filename ( PIXMAPS_DIR, "grisbi-logo.png", NULL );
    if ( g_file_test ( string, G_FILE_TEST_EXISTS ) )
        gtk_window_set_default_icon_from_file ( string, NULL );
    g_free (string);

    /* initialisation of the variables */
    initialisation_couleurs_listes ();
    init_variables ();
    register_import_formats ();

    /* firt use ? */
    if ( ! gsb_file_config_load_config () )
        first_use = TRUE;

    if ( IS_DEVELOPMENT_VERSION == 1 )
        dialog_message ( "development-version", VERSION );

    /* test version of GTK */
    if ( gtk_check_version ( VERSION_GTK_MAJOR, VERSION_GTK_MINOR, VERSION_GTK_MICRO ) )
    {
        string = g_strdup_printf (  _("You are running Grisbi with GTK version %s"),
                        get_gtk_run_version ( ) );
        dialogue_conditional_hint ( string,
                        _("The version of GTK you are using do not benefit from its "
                        "latest features.\n"
                        "\n"
                        "You should upgrade GTK."),
                        "gtk_obsolete" );
        g_free ( string );
    }

    return first_use;
}


/**
 * detourne le signal SIGSEGV
 *
 *
 *
 * */
void gsb_grisbi_trappe_signal_sigsegv ( void )
{
    struct sigaction sig_sev;

    memset ( &sig_sev, 0, sizeof ( struct sigaction ) );
    sig_sev.sa_handler = traitement_sigsegv;
    sig_sev.sa_flags = 0;
    sigemptyset ( &( sig_sev.sa_mask ) );

    if ( sigaction ( SIGSEGV, &sig_sev, NULL ) )
        g_print ( _("Error on sigaction: SIGSEGV won't be trapped\n") );
}


/**
 * Load file if necessary
 *
 *
 *
 * */
void gsb_grisbi_load_file_if_necessary ( cmdline_options  *opt )
{
    gchar *tmp_str = NULL;

    /* check the command line, if there is something to open */
    if ( opt -> fichier )
    {
        tmp_str = opt -> fichier;

        if ( gsb_file_open_file ( tmp_str ) )
        {
            if ( nom_fichier_comptes )
                g_free ( nom_fichier_comptes );
            nom_fichier_comptes = tmp_str;
        }
        else
        {
            if ( nom_fichier_comptes )
                g_free ( nom_fichier_comptes );
            nom_fichier_comptes = NULL;
        }
    }
    else
    {
        /* open the last file if needed, nom_fichier_comptes was filled while loading the configuration */
        if ( conf.dernier_fichier_auto && nom_fichier_comptes )
        {
            if ( !gsb_file_open_file ( nom_fichier_comptes ) )
                g_free ( nom_fichier_comptes );
        }
    }

    return;
}


/**
 * crée la fenêtre principale de grisbi.
 *
 *
 *
 * */
void gsb_grisbi_create_top_window ( void )
{
    GtkWidget *window_vbox_principale;
    GtkWidget *statusbar;

    /* create the toplevel window */
    window = gtk_window_new ( GTK_WINDOW_TOPLEVEL );
    g_signal_connect ( G_OBJECT ( window ),
                        "delete_event",
                        G_CALLBACK ( main_window_delete_event ),
                        NULL);
    g_signal_connect ( G_OBJECT ( window ),
                        "destroy",
                        G_CALLBACK ( main_window_destroy_event ),
                        NULL);
    g_signal_connect ( G_OBJECT ( window ),
                        "window-state-event",
                        G_CALLBACK (gsb_grisbi_change_state_window),
                        NULL );
    gtk_window_set_policy ( GTK_WINDOW ( window ), TRUE, TRUE, FALSE );

    /* create the main window : a vbox */
    window_vbox_principale = gtk_vbox_new ( FALSE, 0 );
    g_object_set_data ( G_OBJECT ( window ), "window_vbox_principale", window_vbox_principale );
    gtk_container_add ( GTK_CONTAINER ( window ), window_vbox_principale );
    g_signal_connect ( G_OBJECT ( window_vbox_principale ),
                        "destroy",
                        G_CALLBACK ( gtk_widget_destroyed ),
                        &window_vbox_principale );
    gtk_widget_show ( window_vbox_principale );

    /* We create the statusbar first. */
    statusbar = gsb_new_statusbar ( );
    gtk_box_pack_end ( GTK_BOX ( window_vbox_principale ), statusbar, FALSE, FALSE, 0 );

    /* create the menus */
    init_menus ( window_vbox_principale );

    /* unsensitive the necessaries menus */
    menus_sensitifs ( FALSE );

    /* charge les raccourcis claviers */
    gtk_accel_map_load ( C_PATH_CONFIG_ACCELS );

    /* set the last opened files */
    affiche_derniers_fichiers_ouverts ( );

    /* set the size of the window */
    if ( conf.main_width && conf.main_height )
        gtk_window_set_default_size ( GTK_WINDOW ( window ), conf.main_width, conf.main_height );
    else
        gtk_window_set_default_size ( GTK_WINDOW ( window ), 900, 600 );

    /* display window at position */
    gtk_window_move ( GTK_WINDOW ( window ), conf.root_x, conf.root_y );

    /* set the full screen if necessary */
    if ( conf.full_screen )
        gtk_window_maximize ( GTK_WINDOW ( window ) );
}


/**
 * check on any change on the main window
 * for now, only to check if we set/unset the full-screen
 *
 * \param window
 * \param event
 * \param null
 *
 * \return FALSE
 * */
gboolean gsb_grisbi_change_state_window ( GtkWidget *window,
                        GdkEventWindowState *event,
                        gpointer null )
{
    if (event->changed_mask & GDK_WINDOW_STATE_MAXIMIZED)
    {
	if (event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED)
	    conf.full_screen = TRUE;
	else
	    conf.full_screen = FALSE;
    }
    return FALSE;
}

/**
 * close grisbi by destroying the main window
 * This function is called by the Quit menu option.
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_grisbi_close ( void )
{
    devel_debug (NULL);
    /* sauvegarde la position de la fenetre principale */
    gtk_window_get_position ( GTK_WINDOW ( window ), &conf.root_x, &conf.root_y  );

    if (! main_window_delete_event (window, NULL))
    	gtk_widget_destroy ( window );

    /* clean finish of the debug file */
    if (etat.debug_mode && debug_file)
	fclose (debug_file);

    return FALSE;
}

/**
 * This function is called when the main window is deleted.
 * It proposes to save the file if necessary.
 */
static gboolean main_window_delete_event (GtkWidget *window, gpointer data)
{
    /* need to save the config before gsb_file_close */
    gsb_file_config_save_config();

    if (!gsb_file_close ())
        return TRUE;
    return FALSE;
}

/**
 * exit the gtk main loop when the main window is destroyed.
 */
static void main_window_destroy_event( GObject* obj, gpointer data)
{
   window = NULL;
   gtk_main_quit();
}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
