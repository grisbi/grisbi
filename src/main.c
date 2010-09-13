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
#include "dialog.h"
#include "gsb_assistant_first.h"
#include "gsb_file.h"
#include "gsb_file_config.h"
#include "gsb_plugins.h"
#include "gsb_status.h"
#include "import.h"
#include "menu.h"
#include "parse_cmdline.h"
#include "structures.h"
#include "tip.h"
#include "traitement_variables.h"
#include "utils.h"
#include "erreur.h"
/*END_INCLUDE*/

#ifdef GTKOSXAPPLICATION
#include <gtkosxapplication.h>

#include "grisbi_osx.h"
#endif  /* GTKOSXAPPLICATION */

/* including resources so that the exe generated with MSVC has the right icon. */
#ifdef _MSC_VER
#include "resource.h"

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
#endif /* _MSC_VER */

/*START_STATIC*/
static gboolean gsb_grisbi_change_state_window ( GtkWidget *window,
                        GdkEventWindowState *event,
                        gpointer null );
static GtkWidget *gsb_grisbi_create_main_menu ( GtkWidget *vbox );
static GtkWidget *gsb_grisbi_create_top_window ( void );
static gboolean gsb_grisbi_init_app ( void );
static void gsb_grisbi_load_file_if_necessary ( cmdline_options *opt );
static gboolean gsb_grisbi_print_environment_var ( void );
static void gsb_grisbi_trappe_signal_sigsegv ( void );
static void main_mac_osx ( int argc, char **argv );
static void main_linux ( int argc, char **argv );
static void main_win_32 (  int argc, char **argv );
static gboolean main_window_delete_event (GtkWidget *window, gpointer data);
static void main_window_destroy_event ( GObject* obj, gpointer data);
static void main_window_set_size_and_position ( void );
/*END_STATIC*/

/* Fenetre principale de grisbi */
G_MODULE_EXPORT GtkWidget *window = NULL;

/*START_EXTERN*/
extern FILE *debug_file;
extern gchar *nom_fichier_comptes;
/*END_EXTERN*/


/**
 * Main function
 *
 * @param argc number of arguments
 * @param argv arguments
 *
 * @return Nothing
 */
int main ( int argc, char **argv )
{
#if IS_DEVELOPMENT_VERSION == 1
        initialize_debugging ( );
#endif

#if GSB_GMEMPROFILE
    g_mem_set_vtable(glib_mem_profiler_table);
#endif

    bindtextdomain (PACKAGE, LOCALEDIR);
    bind_textdomain_codeset (PACKAGE, "UTF-8");
    textdomain (PACKAGE);

    /* Setup locale/gettext */
    setlocale (LC_ALL, "");

#if IS_DEVELOPMENT_VERSION == 1
    gsb_grisbi_print_environment_var ( );
#endif

#ifdef _WIN32
    main_win_32 (  argc, argv );
#else
    #ifdef GTKOSXAPPLICATION
        main_mac_osx ( argc, argv );
    #else
        main_linux ( argc, argv );
    #endif /* GTKOSXAPPLICATION || linux */
#endif /* _WIN32 */

#if GSB_GMEMPROFILE
    g_mem_profile();
#endif

    exit ( 0 );
}


/**
 *
 *
 *
 *
 * */
void main_linux ( int argc, char **argv )
{
    GtkWidget *vbox;
    gboolean first_use = FALSE;
    cmdline_options  opt;
    gint status = CMDLINE_SYNTAX_OK;

    gtk_init ( &argc, &argv );

    /* on commence par détourner le signal SIGSEGV */
    gsb_grisbi_trappe_signal_sigsegv ( );

    /* parse command line parameter, exit with correct error code when needed */
    if ( !parse_options (argc, argv, &opt, &status ) )
        exit ( status );
    /* initialise les données de l'application */
    first_use = gsb_grisbi_init_app ( );

    /* create the toplevel window and the main menu */
    vbox = gsb_grisbi_create_top_window ( );
    gsb_grisbi_create_main_menu ( vbox );
    main_window_set_size_and_position ( );

    gtk_widget_show ( window );

#if IS_DEVELOPMENT_VERSION == 1
    dialog_message ( "development-version", VERSION );
#endif

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
}


/**
 *
 *
 *
 *
 * */
void main_mac_osx ( int argc, char **argv )
{
#ifdef GTKOSXAPPLICATION
    GtkWidget *vbox;
    GtkWidget *menubar;
    gboolean first_use = FALSE;
    cmdline_options  opt;
    gint status = CMDLINE_SYNTAX_OK;
    GtkOSXApplication *theApp;
    GdkPixbuf *pixbuf;

    devel_debug ("main_mac_osx");

    gtk_init ( &argc, &argv );

    /* init the app */
    theApp = g_object_new ( GTK_TYPE_OSX_APPLICATION, NULL );

    /* on commence par détourner le signal SIGSEGV */
    gsb_grisbi_trappe_signal_sigsegv ( );

    /* parse command line parameter, exit with correct error code when needed */
    if ( !parse_options (argc, argv, &opt, &status ) )
        exit ( status );

    /* initialise les données de l'application */
    first_use = gsb_grisbi_init_app ( );

    vbox = gsb_grisbi_create_top_window ( );
    {
        gboolean falseval = FALSE;
        gboolean trueval = TRUE;

        g_signal_connect ( theApp,
                        "NSApplicationDidBecomeActive",
                        G_CALLBACK ( grisbi_osx_app_active_cb ),
                        &trueval );
        g_signal_connect ( theApp,
                        "NSApplicationWillResignActive",
                        G_CALLBACK ( grisbi_osx_app_active_cb ),
                        &falseval);
        g_signal_connect ( theApp,
                        "NSApplicationBlockTermination",
                        G_CALLBACK ( gsb_grisbi_close ),
                        NULL);
        g_signal_connect ( theApp,
                        "NSApplicationWillTerminate",
                        G_CALLBACK ( main_window_destroy_event ),
                        NULL );
    }
    menubar = gsb_grisbi_create_main_menu ( vbox );
    grisbi_osx_init_menus ( window, menubar );
    main_window_set_size_and_position ( );

    gtk_widget_show ( window );

#if IS_DEVELOPMENT_VERSION == 1
    dialog_message ( "development-version", VERSION );
#endif

    gsb_grisbi_load_file_if_necessary ( &opt );

    if ( first_use && !nom_fichier_comptes )
        gsb_assistant_first_run ();
    else
        display_tip ( FALSE );

    pixbuf = gsb_select_icon_get_default_logo_pixbuf ( );
    if ( pixbuf )
    {
        gtk_osxapplication_set_dock_icon_pixbuf ( theApp, pixbuf );
        printf ("pixbuf existe\n");
    }
    else
        printf ("pixbuf n'existe pas\n");

    if ( quartz_application_get_bundle_id ( ) )
        gtk_osxapplication_set_dock_icon_resource  ( theApp, "grisbi_logo", "png", "share/pixmaps/grisbi" );

    gtk_osxapplication_set_use_quartz_accelerators ( theApp, TRUE );

    gtk_osxapplication_ready ( theApp );
    {
        const gchar *id;

        id = quartz_application_get_bundle_id ( );
        if ( id != NULL )
        {
            g_print ("Error! Bundle Has ID %s\n", id);
        }
    }

    gtk_main ();

    gsb_plugins_release ( );

    /* sauvegarde les raccourcis claviers */
    gtk_accel_map_save ( C_PATH_CONFIG_ACCELS );

    g_object_unref ( theApp );

#endif /* GTKOSXAPPLICATION */
}


/**
 *
 *
 *
 *
 * */
void main_win_32 (  int argc, char **argv )
{
#ifdef _WIN32
    GtkWidget *vbox;
    gboolean first_use = FALSE;
    cmdline_options  opt;
    gint status = CMDLINE_SYNTAX_OK;    /* be optimistic ;-) */

    /* Retrieve exception information and store them under grisbi.rpt file!
     * see http://jrfonseca.dyndns.org/projects/gnu-win32/software/drmingw/index.html for more information */
    LoadLibrary("exchndl.dll");

    /* we store the path of the running file to use it for pixmaps, help and locales .... */
    win32_set_app_path(argv[0]);

     /* needed to be able to use the "common" installation of GTK libraries */
    win32_make_sure_the_gtk2_dlls_path_is_in_PATH();

    gtk_init ( &argc, &argv );

    win32_parse_gtkrc ( "gtkrc" );

    /* parse command line parameter, exit with correct error code when needed */
    if ( !parse_options (argc, argv, &opt, &status ) )
        exit ( status );

    /* initialise les données de l'application */
    first_use = gsb_grisbi_init_app ( );

    /* create the toplevel window and the main menu */
    vbox = gsb_grisbi_create_top_window ( );
    gsb_grisbi_create_main_menu ( vbox );
    main_window_set_size_and_position ( );

    gtk_widget_show ( window );

#if IS_DEVELOPMENT_VERSION == 1
    dialog_message ( "development-version", VERSION );
#endif

    gsb_grisbi_load_file_if_necessary ( &opt );

    if ( first_use && !nom_fichier_comptes )
        gsb_assistant_first_run ();
    else
        display_tip ( FALSE );

    gtk_main ();

    gsb_plugins_release ( );

    /* sauvegarde les raccourcis claviers */
    gtk_accel_map_save ( C_PATH_CONFIG_ACCELS );

#endif /* WIN_32 */
}



/**
 * gère le fait d'être en mode developpement sauf pour l'alerte
 *
 *
 *
 * */
gboolean gsb_grisbi_print_environment_var ( void )
{
    struct lconv *conv;

    /* test local pour les nombres */
    conv = localeconv();
    
    printf ("Variables d'environnement :\n\n" );
    printf ("Currency\n"
            "\tcurrency_symbol = %s\n"
            "\tmon_thousands_sep = \"%s\"\n"
            "\tmon_decimal_point = %s\n"
            "\tpositive_sign = \"%s\"\n"
            "\tnegative_sign = \"%s\"\n"
            "\tfrac_digits = \"%d\"\n\n",
            conv->currency_symbol,
            g_locale_to_utf8 ( conv->mon_thousands_sep, -1, NULL, NULL, NULL ),
            g_locale_to_utf8 ( conv->mon_decimal_point, -1, NULL, NULL, NULL ),
            g_locale_to_utf8 ( conv->positive_sign, -1, NULL, NULL, NULL ),
            g_locale_to_utf8 ( conv->negative_sign, -1, NULL, NULL, NULL ),
            conv->frac_digits );

    printf ("Paths\n"
            "\tC_GRISBIRC = %s\n"
            "\tC_PATH_CONFIG = %s\n"
            "\tC_PATH_CONFIG_ACCELS = %s\n"
            "\tC_PATH_DATA_FILES = %s\n"
            "\tPLUGINS_DIR = %s\n"
            "\tPIXMAPS_DIR = %s\n\n",
            C_GRISBIRC,
            C_PATH_CONFIG,
            C_PATH_CONFIG_ACCELS,
            C_PATH_DATA_FILES,
            PLUGINS_DIR,
            PIXMAPS_DIR );

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
 * crée la fenêtre principale de grisbi.
 *
 *
 *
 * */
GtkWidget *gsb_grisbi_create_top_window ( void )
{
    GtkWidget *vbox;
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
    vbox = gtk_vbox_new ( FALSE, 0 );
    g_object_set_data ( G_OBJECT ( window ), "window_vbox_principale", vbox );
    gtk_container_add ( GTK_CONTAINER ( window ), vbox );
    g_signal_connect ( G_OBJECT ( vbox ),
                        "destroy",
                        G_CALLBACK ( gtk_widget_destroyed ),
                        &vbox );
    gtk_widget_show ( vbox );

    /* We create the statusbar first. */
    statusbar = gsb_new_statusbar ( );
    gtk_box_pack_end ( GTK_BOX ( vbox ), statusbar, FALSE, FALSE, 0 );

    return vbox;
}

/**
 * crée et initialise le menu de grisbi.
 *
 *
 *
 * */
GtkWidget *gsb_grisbi_create_main_menu ( GtkWidget *vbox )
{
    GtkWidget *menubar;

    /* create the menus */
    menubar = init_menus ( vbox );

    /* unsensitive the necessaries menus */
    menus_sensitifs ( FALSE );

    /* charge les raccourcis claviers */
    gtk_accel_map_load ( C_PATH_CONFIG_ACCELS );

    /* set the last opened files */
    affiche_derniers_fichiers_ouverts ( );

    return menubar;
}

/**
 * set size and position of the main window of grisbi.
 *
 *
 *
 * */
void main_window_set_size_and_position ( void )
{
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
 * detourne le signal SIGSEGV
 *
 *
 *
 * */
void gsb_grisbi_trappe_signal_sigsegv ( void )
{
#ifndef _WIN32
    struct sigaction sig_sev;

    memset ( &sig_sev, 0, sizeof ( struct sigaction ) );
    sig_sev.sa_handler = traitement_sigsegv;
    sig_sev.sa_flags = 0;
    sigemptyset ( &( sig_sev.sa_mask ) );

    if ( sigaction ( SIGSEGV, &sig_sev, NULL ) )
        g_print ( _("Error on sigaction: SIGSEGV won't be trapped\n") );
#endif /* not WIN_32 */
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
static void main_window_destroy_event ( GObject* obj, gpointer data)
{
   window = NULL;
   gtk_main_quit();
}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
