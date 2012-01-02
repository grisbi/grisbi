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

#include "include.h"
#include <stdlib.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include <goffice/goffice.h>

/*START_INCLUDE*/
#include "main.h"
#include "accueil.h"
#include "dialog.h"
#include "gsb_assistant_first.h"
#include "gsb_color.h"
#include "gsb_data_account.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "gsb_file_config.h"
#include "gsb_locale.h"
#include "gsb_plugins.h"
#include "gsb_status.h"
#include "import.h"
#include "menu.h"
#include "structures.h"
#include "tip.h"
#include "traitement_variables.h"
#include "utils.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

#ifdef GTKOSXAPPLICATION
#include <gtkosxapplication.h>

#include "grisbi_osx.h"
#endif  /* GTKOSXAPPLICATION */

/* including resources so that the exe generated with MSVC has the right icon. */
#ifdef G_OS_WIN32
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
#endif /* G_OS_WIN32 */

/*START_STATIC*/
static gboolean gsb_grisbi_change_state_window ( GtkWidget *window,
                        GdkEventWindowState *event,
                        gpointer null );
static GtkWidget *gsb_grisbi_create_main_menu ( GtkWidget *vbox );
static GtkWidget *gsb_main_create_main_window ( void );
static void gsb_main_grisbi_shutdown ( void );
static gint gsb_main_set_debug_level ( void );
static gint gsb_main_setup_command_line_options ( int argc, char **argv );
static void gsb_main_show_version ( void );
static gboolean gsb_grisbi_init_app ( void );
static void gsb_main_load_file_if_necessary ( gchar *filename );
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
static GtkWidget *main_window = NULL;

/* Options pour grisbi */
static gint debug_level = -1;
static gchar *file = NULL;

static const GOptionEntry options [] =
{
    { "version", 'v', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
     gsb_main_show_version, N_("Show the application's version"), NULL },

    { "debug", 'd', 0, G_OPTION_ARG_INT, &debug_level, N_("Debug mode: level 0-5"), NULL },

    { "file", 'f', 0, G_OPTION_ARG_FILENAME, &file, N_("[FILE]"), NULL },

    {NULL}
};

/*START_EXTERN*/
extern gchar *nom_fichier_comptes;
extern gchar *titre_fichier;
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
int main ( int argc, char **argv )
{
#if GSB_GMEMPROFILE
    g_mem_set_vtable(glib_mem_profiler_table);
#endif

#ifdef G_OS_WIN32
    main_win_32 (  argc, argv );
#else
    #ifdef GTKOSXAPPLICATION
        main_mac_osx ( argc, argv );
    #else
        main_linux ( argc, argv );
    #endif /* GTKOSXAPPLICATION || linux */
#endif /* G_OS_WIN32 */

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
    gint return_value;
    gboolean first_use = FALSE;

    /* Init type system */
    g_type_init ();

    /* initialisation des différents répertoires */
    gsb_dirs_init ( );

    /* Setup locale/gettext */
    setlocale ( LC_ALL, "" );
    bindtextdomain ( PACKAGE, gsb_dirs_get_locale_dir ( ) );
    bind_textdomain_codeset ( PACKAGE, "UTF-8" );
    textdomain ( PACKAGE );

    /* Setup command line options */
    return_value = gsb_main_setup_command_line_options ( argc, argv );
    if ( return_value )
        exit ( return_value );

    /* initialisation de la variable locale pour les devises */
    gsb_locale_init ( );

    /* initialisation du nom du fichier de configuration */
    gsb_config_initialise_conf_filename ( );

    /* initialisation du mode de débogage */
    if ( gsb_main_set_debug_level ( ) )
    {
        initialize_debugging ( );
        gsb_grisbi_print_environment_var ( );
    }

    /* initialisation de gtk. arguments à NULL car traités au dessus */
    gtk_init ( NULL, NULL );

    /* initialisation libgoffice */
    libgoffice_init ( );
    /* Initialize plugins manager pour goffice */
    go_plugins_init (NULL, NULL, NULL, NULL, TRUE, GO_TYPE_PLUGIN_LOADER_MODULE);

    /* on commence par détourner le signal SIGSEGV */
    gsb_grisbi_trappe_signal_sigsegv ( );

    /* initialise les données de l'application */
    first_use = gsb_grisbi_init_app ( );

    /* create the toplevel window and the main menu */
    vbox = gsb_main_create_main_window ( );
    gsb_grisbi_create_main_menu ( vbox );
    main_window_set_size_and_position ( );

    gtk_widget_show ( main_window );

    if ( IS_DEVELOPMENT_VERSION )
        dialog_message ( "development-version", VERSION );

    /* check the command line, if there is something to open */
    gsb_main_load_file_if_necessary ( file );

    if ( first_use && !nom_fichier_comptes )
        gsb_assistant_first_run ();
    else
        display_tip ( FALSE );

    gtk_main ();

    gsb_main_grisbi_shutdown ( );

    exit ( 0 );
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
    GdkPixbuf *pixbuf;
    gboolean first_use = FALSE;
    GtkOSXApplication *theApp;

    /* Init type system */
    g_type_init ();

    /* initialisation des différents répertoires */
    gsb_dirs_init ( );

    /* Setup locale/gettext */
    setlocale ( LC_ALL, "" );
    bindtextdomain ( PACKAGE, gsb_dirs_get_locale_dir ( ) );
    bind_textdomain_codeset ( PACKAGE, "UTF-8" );
    textdomain ( PACKAGE );

    /* Setup command line options */
    return_value = gsb_main_setup_command_line_options ( argc, argv );
    if ( return_value )
        exit ( return_value );

    /* initialisation de la variable locale pour les devises */
    gsb_locale_init ( );

    /* initialisation du nom du fichier de configuration */
    gsb_config_initialise_conf_filename ( );

    /* initialisation du mode de débogage */
    if ( gsb_main_set_debug_level ( ) )
    {
        initialize_debugging ( );
        gsb_grisbi_print_environment_var ( );
    }

    /* initialisation de gtk. arguments à NULL car traités au dessus */
    gtk_init ( NULL, NULL );

    /* initialisation libgoffice */
    libgoffice_init ( );
    /* Initialize plugins manager pour goffice */
    go_plugins_init (NULL, NULL, NULL, NULL, TRUE, GO_TYPE_PLUGIN_LOADER_MODULE);

    /* init the app */
    theApp = g_object_new ( GTK_TYPE_OSX_APPLICATION, NULL );

    /* on commence par détourner le signal SIGSEGV */
    gsb_grisbi_trappe_signal_sigsegv ( );

    /* parse command line parameter, exit with correct error code when needed */
    if ( !parse_options (argc, argv, &opt, &status ) )
        exit ( status );

    /* initialise les données de l'application */
    first_use = gsb_grisbi_init_app ( );

    vbox = gsb_main_create_main_window ( );
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
                        G_CALLBACK ( gsb_main_grisbi_close ),
                        NULL);
        g_signal_connect ( theApp,
                        "NSApplicationWillTerminate",
                        G_CALLBACK ( main_window_destroy_event ),
                        NULL );
    }
    menubar = gsb_grisbi_create_main_menu ( vbox );
    grisbi_osx_init_menus ( main_window, menubar );
    main_window_set_size_and_position ( );

    gtk_widget_show ( main_window );

#if IS_DEVELOPMENT_VERSION == 1
    dialog_message ( "development-version", VERSION );
#endif
    
    /* check the command line, if there is something to open */
    gsb_main_load_file_if_necessary ( file );

    if ( first_use && !nom_fichier_comptes )
        gsb_assistant_first_run ();
    else
        display_tip ( FALSE );

    if ( quartz_application_get_bundle_id ( ) == NULL )
    {
        pixbuf = gdk_pixbuf_new_from_file ( g_build_filename 
                        (gsb_dirs_get_pixmaps_dir ( ), "grisbi-logo.png", NULL), NULL );
        if ( pixbuf )
            gtk_osxapplication_set_dock_icon_pixbuf ( theApp, pixbuf );
    }

    gtk_osxapplication_set_use_quartz_accelerators ( theApp, TRUE );

    gtk_osxapplication_ready ( theApp );

    gtk_main ();

    g_object_unref ( theApp );

    gsb_main_grisbi_shutdown ( );

    exit ( 0 );

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
#ifdef G_OS_WIN32
    GtkWidget *vbox;
    gboolean first_use = FALSE;

    /* Retrieve exception information and store them under grisbi.rpt file!
     * see http://jrfonseca.dyndns.org/projects/gnu-win32/software/drmingw/index.html for more information */
    LoadLibrary("exchndl.dll");

    /* we store the path of the running file to use it for pixmaps, help and locales .... */
/*     win32_set_app_path ( argv[0] );  */

     /* needed to be able to use the "common" installation of GTK libraries */
    win32_make_sure_the_gtk2_dlls_path_is_in_PATH();

    /* Init type system */
    g_type_init ();

    /* initialisation des différents répertoires */
    gsb_dirs_init ( );

    /* Setup locale/gettext */
    setlocale ( LC_ALL, "" );
    bindtextdomain ( PACKAGE, gsb_dirs_get_locale_dir ( ) );
    bind_textdomain_codeset ( PACKAGE, "UTF-8" );
    textdomain ( PACKAGE );

    /* Setup command line options */
    return_value = gsb_main_setup_command_line_options ( argc, argv );
    if ( return_value )
        exit ( return_value );

    /* initialisation de la variable locale pour les devises */
    gsb_locale_init ( );

    /* initialisation du nom du fichier de configuration */
    gsb_config_initialise_conf_filename ( );

    /* initialisation de gtk. arguments à NULL car traités au dessus */
    gtk_init ( NULL, NULL );
/*     win32_parse_gtkrc ( "gtkrc" );  */

    /* initialisation libgoffice */
    libgoffice_init ( );
    /* Initialize plugins manager pour goffice */
    go_plugins_init (NULL, NULL, NULL, NULL, TRUE, GO_TYPE_PLUGIN_LOADER_MODULE);

    /* initialise les données de l'application */
    first_use = gsb_grisbi_init_app ( );

    /* create the toplevel window and the main menu */
    vbox = gsb_main_create_main_window ( );
    gsb_grisbi_create_main_menu ( vbox );
    main_window_set_size_and_position ( );

    gtk_widget_show ( main_window );

#if IS_DEVELOPMENT_VERSION == 1
    dialog_message ( "development-version", VERSION );
#endif

    /* check the command line, if there is something to open */
    gsb_main_load_file_if_necessary ( file );

    if ( first_use && !nom_fichier_comptes )
        gsb_assistant_first_run ();
    else
        display_tip ( FALSE );

    gtk_main ();

    gsb_main_grisbi_shutdown ( );

    exit ( 0 );

#endif /* G_OS_WIN32 */
}



/**
 * gère le fait d'être en mode developpement sauf pour l'alerte
 *
 *
 *
 * */
gboolean gsb_grisbi_print_environment_var ( void )
{
    gchar *tmp_str;

    g_print ("\nGrisbi version %s\n\n", VERSION );

    g_printf ("Variables d'environnement :\n" );

    tmp_str = gsb_main_get_print_locale_var ( );
    g_printf ("%s", tmp_str);

    g_free ( tmp_str );

    g_printf ( "gint64\n\tG_GINT64_MODIFIER = \"%s\"\n"
                        "\t%"G_GINT64_MODIFIER"d\n\n",
                        G_GINT64_MODIFIER,
                        G_MAXINT64 );

    tmp_str = gsb_main_get_print_dir_var ( );
    g_printf ("%s", tmp_str);

    g_free ( tmp_str );

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
    gsb_plugins_scan_dir ( gsb_dirs_get_plugins_dir ( ) );
#endif

    /* create the icon of grisbi (set in the panel of gnome or other) */
    string = g_build_filename ( gsb_dirs_get_pixmaps_dir ( ), "grisbi-logo.png", NULL );
    if ( g_file_test ( string, G_FILE_TEST_EXISTS ) )
        gtk_window_set_default_icon_from_file ( string, NULL );
    g_free (string);

    /* initialisation of the variables */
    gsb_color_initialise_couleurs_par_defaut ( );
    init_variables ();
    register_import_formats ();

    /* firt use ? */
    if ( ! gsb_file_config_load_config () )
        first_use = TRUE;

    return first_use;
}


/**
 * crée la fenêtre principale de grisbi.
 *
 *
 *
 * */
GtkWidget *gsb_main_create_main_window ( void )
{
    GtkWidget *vbox;
    GtkWidget *status_bar;

    /* create the toplevel window */
    main_window = gtk_window_new ( GTK_WINDOW_TOPLEVEL );
    run.window = main_window;

    g_signal_connect ( G_OBJECT ( main_window ),
                        "delete_event",
                        G_CALLBACK ( main_window_delete_event ),
                        NULL);
    g_signal_connect ( G_OBJECT ( main_window ),
                        "destroy",
                        G_CALLBACK ( main_window_destroy_event ),
                        NULL);
    g_signal_connect ( G_OBJECT ( main_window ),
                        "window-state-event",
                        G_CALLBACK (gsb_grisbi_change_state_window),
                        NULL );
    gtk_window_set_policy ( GTK_WINDOW ( main_window ), TRUE, TRUE, FALSE );
/*     gtk_window_set_resizable ( GTK_WINDOW ( main_window ), TRUE );  */

    /* create the main window : a vbox */
    vbox = gtk_vbox_new ( FALSE, 0 );
    g_object_set_data ( G_OBJECT ( main_window ), "main_vbox", vbox );
    gtk_container_add ( GTK_CONTAINER ( main_window ), vbox );
    g_signal_connect ( G_OBJECT ( vbox ),
                        "destroy",
                        G_CALLBACK ( gtk_widget_destroyed ),
                        &vbox );
    gtk_widget_show ( vbox );

    /* We create the statusbar first. */
    status_bar = gsb_new_statusbar ( );
    gtk_box_pack_end ( GTK_BOX ( vbox ), status_bar, FALSE, FALSE, 0 );

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
    GtkWidget *menu_bar;
    gchar *accel_filename;

    /* create the menus */
    menu_bar = init_menus ( vbox );

    /* unsensitive the necessaries menus */
    menus_sensitifs ( FALSE );

    /* charge les raccourcis claviers */
    accel_filename = g_build_filename ( gsb_dirs_get_user_config_dir ( ), "grisbi-accels", NULL );
    gtk_accel_map_load ( accel_filename );
    g_free ( accel_filename );

    /* set the last opened files */
    affiche_derniers_fichiers_ouverts ( );

    return menu_bar;
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
        gtk_window_set_default_size ( GTK_WINDOW ( main_window ), conf.main_width, conf.main_height );
    else
        gtk_window_set_default_size ( GTK_WINDOW ( main_window ), 900, 600 );

    /* display window at position */
    gtk_window_move ( GTK_WINDOW ( main_window ), conf.root_x, conf.root_y );

    /* set the full screen if necessary */
    if ( conf.full_screen )
        gtk_window_maximize ( GTK_WINDOW ( main_window ) );
}


/**
 * detourne le signal SIGSEGV
 *
 *
 *
 * */
void gsb_grisbi_trappe_signal_sigsegv ( void )
{
#ifndef G_OS_WIN32
    struct sigaction sig_sev;

    memset ( &sig_sev, 0, sizeof ( struct sigaction ) );
    sig_sev.sa_handler = traitement_sigsegv;
    sig_sev.sa_flags = 0;
    sigemptyset ( &( sig_sev.sa_mask ) );

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
void gsb_main_load_file_if_necessary ( gchar *filename )
{
    /* check the command line, if there is something to open */
    if ( filename )
    {
        if ( gsb_file_open_file ( filename ) )
        {
            if ( nom_fichier_comptes )
                g_free ( nom_fichier_comptes );
            nom_fichier_comptes = g_strdup ( filename );
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
gboolean gsb_main_grisbi_close ( void )
{
    devel_debug (NULL);
    /* sauvegarde la position de la fenetre principale */
    gtk_window_get_position ( GTK_WINDOW ( main_window ), &conf.root_x, &conf.root_y  );

    if ( !main_window_delete_event ( main_window, NULL ) )
        gtk_widget_destroy ( main_window );

    /* clean finish of the debug file */
    if ( etat.debug_mode )
        gsb_debug_finish_log ( );

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
    free_variables();
    main_window = NULL;
    gtk_main_quit();
}


/**
 * set Grisbi title
 *
 *
 *
 * */
gboolean gsb_main_set_grisbi_title ( gint account_number )
{
    gchar *titre_grisbi = NULL;
    gchar *titre = NULL;
    gint tmp_number;
    gboolean return_value;
    
    devel_debug_int ( account_number );

    if ( nom_fichier_comptes == NULL )
    {
        titre_grisbi = g_strdup ( _("Grisbi") );
        return_value = TRUE;
    }
    else
    {
        switch ( conf.display_grisbi_title )
        {
            case GSB_ACCOUNTS_TITLE:
                if ( titre_fichier && strlen ( titre_fichier ) )
                    titre = g_strdup ( titre_fichier );
            break;
            case GSB_ACCOUNT_HOLDER:
            {
                if ( account_number == -1 )
                    tmp_number = gsb_data_account_first_number ( );
                else
                    tmp_number = account_number;

                if ( tmp_number == -1 )
                {
                    if ( titre_fichier && strlen ( titre_fichier ) )
                        titre = g_strdup ( titre_fichier );
                }
                else
                {
                    titre = g_strdup ( gsb_data_account_get_holder_name ( tmp_number ) );

                    if ( titre == NULL )
                        titre = g_strdup ( gsb_data_account_get_name ( tmp_number ) );
                }
            break;
            }
            case GSB_ACCOUNTS_FILE:
                if ( nom_fichier_comptes && strlen ( nom_fichier_comptes ) )
                    titre = g_path_get_basename ( nom_fichier_comptes );
            break;
        }

        if ( titre && strlen ( titre ) > 0 )
        {
            titre_grisbi = g_strconcat ( titre, " - ", _("Grisbi"), NULL );
            g_free ( titre );

            return_value = TRUE;
        }
        else
        {
            titre_grisbi = g_strconcat ( "<", _("unnamed"), ">", NULL );
            return_value = FALSE;
        }
    }
    gtk_window_set_title ( GTK_WINDOW ( main_window ), titre_grisbi );

    gsb_main_page_update_homepage_title ( titre_grisbi );

    if ( titre_grisbi && strlen ( titre_grisbi ) > 0 )
        g_free ( titre_grisbi );

    return return_value;
}


/**
 *
 *  \return must be freed
 *
 */
gchar *gsb_main_get_print_locale_var ( void )
{
    struct lconv *conv;
    gchar *locale_str = NULL;
    gchar *positive_sign;
    gchar *negative_sign;

    /* test local pour les nombres */
    conv = gsb_locale_get_locale ( );
    positive_sign = g_strdup ( conv->positive_sign );
    negative_sign = g_strdup ( conv->negative_sign );

    locale_str = g_strdup_printf ( "LANG = %s\n"
                        "Currency\n"
                        "\tcurrency_symbol   = %s\n"
                        "\tmon_thousands_sep = \"%s\"\n"
                        "\tmon_decimal_point = %s\n"
                        "\tpositive_sign     = \"%s\"\n"
                        "\tnegative_sign     = \"%s\"\n"
                        "\tp_cs_precedes     = \"%d\"\n"
                        "\tfrac_digits       = \"%d\"\n\n",
                        g_getenv ( "LANG"),
                        conv->currency_symbol,
                        gsb_locale_get_mon_thousands_sep ( ),
                        gsb_locale_get_mon_decimal_point ( ),
                        positive_sign,
                        negative_sign,
                        conv->p_cs_precedes,
                        conv->frac_digits );

    g_free ( positive_sign );
    g_free ( negative_sign );

    return locale_str;
}


/**
 *
 *  \return must be freed
 *
 */
gchar *gsb_main_get_print_dir_var ( void )
{
    gchar *path_str = NULL;
    const gchar *conf_filename;
    gchar *accel_filename;

    conf_filename = gsb_config_get_conf_filename ( );
    accel_filename = g_build_filename ( gsb_dirs_get_user_config_dir ( ), "grisbi-accels", NULL );

    path_str = g_strdup_printf ( "Paths\n"
                        "\thome_dir                         = %s\n"
                        "\tuser_config_dir                  = %s\n"
                        "\tuser_config_filename             = %s\n"
                        "\tuser_accels_filename             = %s\n"
                        "\tuser_data_pathname               = %s\n\n"
                        "\tsys_data_pathname                = %s\n\n"
                        "\tgsb_dirs_get_categories_dir ( )  = %s\n"
                        "\tgsb_dirs_get_locale_dir ( )      = %s\n"
                        "\tgsb_dirs_get_plugins_dir ( )     = %s\n"
                        "\tgsb_dirs_get_pixmaps_dir ( )     = %s\n"
                        "\tgsb_dirs_get_ui_dir ( )          = %s\n\n",
                        gsb_dirs_get_home_dir ( ),
                        gsb_dirs_get_user_config_dir ( ),
                        conf_filename,
                        accel_filename,
                        gsb_dirs_get_user_data_dir ( ),
                        DATA_PATH,
                        gsb_dirs_get_categories_dir ( ),
                        gsb_dirs_get_locale_dir ( ),
                        gsb_dirs_get_plugins_dir ( ),
                        gsb_dirs_get_pixmaps_dir ( ),
                        gsb_dirs_get_ui_dir ( ) );

    g_free ( accel_filename );

    return path_str;
}


/**
 * renvoie la fenêtre principale de Grisbi
 *
 *
 */
GtkWidget *gsb_main_get_main_window ( void )
{
    return main_window;
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
 * renvoie le niveau de débug
 *
 * \param
 *
 * \return debug_level
 */
gint gsb_main_get_debug_level ( void )
{
    return debug_level;
}


/**
 * positionne la variable debug_mode à 0 si version stable et
 * à DEBUG_GRISBI ou une valeur passée en paramètre
 *
 * \param
 *
 * \return debug_mode
 */
gint gsb_main_set_debug_level ( void )
{
    gchar **tab;
    gint number = 0;
    gint number_1;

    tab = g_strsplit ( VERSION, ".", 3 );
    number_1 = utils_str_atoi ( tab[1] );

    number = number_1 % 2;
    /* on garde le niveau mini de débogage si $DEBUG_GRISBI n'existe pas */
    if ( number )
    {
        if ( getenv ( "DEBUG_GRISBI" ) )
            number = utils_str_atoi ( getenv ( "DEBUG_GRISBI" ) );
    }
    if ( debug_level == -1 )
        debug_level = number;

    if ( debug_level > MAX_DEBUG_LEVEL )
        debug_level = MAX_DEBUG_LEVEL;

    return debug_level;
}


/**
 * traite les arguments de la ligne de commande
 *
 * \param
 *
 * \return code de sortie 1 si erreur de traitement
 */
gint gsb_main_setup_command_line_options ( int argc, char **argv )
{
    GOptionContext *context;
    gint return_value = 0;
    GError *error = NULL;

    context = g_option_context_new ( _("- Personnal finances manager") );

    g_option_context_set_summary ( context,
                        N_("Grisbi can manage the accounts of a family or a small association.") );
    g_option_context_set_translation_domain ( context, PACKAGE );

    g_option_context_add_main_entries ( context, options, PACKAGE );
    g_option_context_add_group ( context, gtk_get_option_group ( FALSE ));
    g_option_context_set_translation_domain ( context, PACKAGE );

    if ( !g_option_context_parse ( context, &argc, &argv, &error ) )
    {
        g_option_context_free ( context );
        if (error)
        {
            g_print ("option parsing failed: %s\n", error->message);
            g_error_free ( error );
        }

      return_value = 1;
    }

    g_option_context_free ( context );

    return return_value;
}


/**
 *  procédure appelée après gtk_main_quit termine Grisbi
 *
 * \param
 *
 * \return
 */
void gsb_main_grisbi_shutdown ( void )
{
    gchar *filename;

    /* sauvegarde les raccourcis claviers */
    filename = g_build_filename ( gsb_dirs_get_user_config_dir ( ), "grisbi-accels", NULL );
    gtk_accel_map_save ( filename );
    g_free ( filename );

    /* libération de mémoire */
    gsb_locale_shutdown ( );
    gsb_dirs_shutdown ( );

    /* liberation libgoffice */
    libgoffice_shutdown ( );

#if GSB_GMEMPROFILE
    g_mem_profile();
#endif
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
