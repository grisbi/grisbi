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
#include <config.h>
#endif

#include "include.h"
#include <stdlib.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include <gdk/gdkkeysyms.h>

#ifdef HAVE_GOFFICE
#include <goffice/goffice.h>
#endif /* HAVE_GOFFICE */

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
static GtkWidget *gsb_main_create_main_window ( void );
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


/*START_EXTERN*/
extern gchar *nom_fichier_comptes;
extern gchar *titre_fichier;
/*END_EXTERN*/

/* variables initialisées lors de l'exécution de grisbi */
struct gsb_run_t run;
GtkCssProvider *css_provider;

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

#ifdef GSB_GMEMPROFILE
    g_mem_set_vtable(glib_mem_profiler_table);
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

    /* initialisation des différents répertoires */
    gsb_dirs_init ( argv[0] );

    bindtextdomain ( PACKAGE, gsb_dirs_get_locale_dir ( ) );
    bind_textdomain_codeset ( PACKAGE, "UTF-8" );
    textdomain ( PACKAGE );

    /* Setup locale/gettext */
    setlocale (LC_ALL, "");
    gsb_locale_init ( );

#if IS_DEVELOPMENT_VERSION == 1
    gsb_grisbi_print_environment_var ( );
#endif

    gtk_init ( &argc, &argv );

#ifdef HAVE_GOFFICE
    /* initialisation libgoffice */
    libgoffice_init ( );
    /* Initialize plugins manager */
    go_plugins_init (NULL, NULL, NULL, NULL, TRUE, GO_TYPE_PLUGIN_LOADER_MODULE);
#endif /* HAVE_GOFFICE */

    /* on commence par détourner le signal SIGSEGV */
    gsb_grisbi_trappe_signal_sigsegv ( );

    /* parse command line parameter, exit with correct error code when needed */
    if ( !parse_options (argc, argv, &opt, &status ) )
        exit ( status );
    /* initialise les données de l'application */
    first_use = gsb_grisbi_init_app ( );

    /* create the toplevel window and the main menu */
    vbox = gsb_main_create_main_window ( );
    gsb_grisbi_create_main_menu ( vbox );
    main_window_set_size_and_position ( );

    gtk_widget_show ( run.window );

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

    /* sauvegarde les raccourcis claviers */
    gtk_accel_map_save ( gsb_dirs_get_accelerator_filename () );

    /* libération de mémoire */
    gsb_locale_shutdown ( );
    gsb_dirs_shutdown ( );

#ifdef HAVE_GOFFICE
    /* liberation libgoffice */
    libgoffice_shutdown ( );
#endif /* HAVE_GOFFICE */

#ifdef GSB_GMEMPROFILE
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
void main_mac_osx ( int argc, char **argv )
{
#ifdef GTKOSXAPPLICATION
    GtkWidget *vbox;
    GtkWidget *menubar;
    GdkPixbuf *pixbuf;
    cmdline_options  opt;
    gboolean first_use = FALSE;
    gint status = CMDLINE_SYNTAX_OK;
    GtkosxApplication *theApp;

    devel_debug ("main_mac_osx");

    gtk_init ( &argc, &argv );

    /* init the app */
    theApp = g_object_new ( GTKOSX_TYPE_APPLICATION, NULL );

#ifdef HAVE_GOFFICE
    /* initialisation libgoffice */
    libgoffice_init ( );
    /* Initialize plugins manager */
    go_plugins_init (NULL, NULL, NULL, NULL, TRUE, GO_TYPE_PLUGIN_LOADER_MODULE);
#endif /* HAVE_GOFFICE */

    /* initialisation des différents répertoires */
    gsb_dirs_init ( argv[0] );

#if IS_DEVELOPMENT_VERSION == 1
    gsb_grisbi_print_environment_var ( );
#endif

    bindtextdomain ( PACKAGE,  gsb_dirs_get_locale_dir ( ) );
    bind_textdomain_codeset ( PACKAGE, "UTF-8" );
    textdomain ( PACKAGE );

    /* Setup locale/gettext */
    setlocale (LC_ALL, "");
    gsb_locale_init ( );

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
    grisbi_osx_init_menus ( run.window, menubar );
    main_window_set_size_and_position ( );

    gtk_widget_show ( run.window );

#if IS_DEVELOPMENT_VERSION == 1
    dialog_message ( "development-version", VERSION );
#endif

    gsb_grisbi_load_file_if_necessary ( &opt );

    if ( first_use && !nom_fichier_comptes )
        gsb_assistant_first_run ();
    else
        display_tip ( FALSE );

    if ( gtkosx_application_get_bundle_id ( ) == NULL )
    {
        pixbuf = gdk_pixbuf_new_from_file ( g_build_filename
                        (gsb_dirs_get_pixmaps_dir ( ), "grisbi-logo.png", NULL), NULL );
        if ( pixbuf )
            gtkosx_application_set_dock_icon_pixbuf ( theApp, pixbuf );
    }

    gtkosx_application_set_use_quartz_accelerators ( theApp, TRUE );

    gtkosx_application_ready ( theApp );

    gtk_main ();

    /* sauvegarde les raccourcis claviers */
    gtk_accel_map_save ( gsb_dirs_get_accelerator_filename () );

    g_object_unref ( theApp );

    gsb_locale_shutdown ( );
    gsb_dirs_shutdown ( );

#ifdef HAVE_GOFFICE
    /* liberation libgoffice */
    libgoffice_shutdown ( );
#endif /* HAVE_GOFFICE */

#ifdef GSB_GMEMPROFILE
    g_mem_profile();
#endif

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

    /* initialisation des différents répertoires */
    gsb_dirs_init ( argv[0] );

    bindtextdomain ( PACKAGE, gsb_dirs_get_locale_dir ( ) );
    bind_textdomain_codeset ( PACKAGE, "UTF-8" );
    textdomain ( PACKAGE );

    /* Setup locale/gettext */
    setlocale( LC_ALL, NULL );

    gsb_locale_init ( );

#ifdef HAVE_GOFFICE
    /* initialisation libgoffice */
    libgoffice_init ( );
    /* Initialize plugins manager */
    go_plugins_init (NULL, NULL, NULL, NULL, TRUE, GO_TYPE_PLUGIN_LOADER_MODULE);
#endif /* HAVE_GOFFICE */

    gtk_init ( &argc, &argv );

    win32_parse_gtkrc ( "gtkrc" );

    /* parse command line parameter, exit with correct error code when needed */
    if ( !parse_options (argc, argv, &opt, &status ) )
        exit ( status );

    /* initialise les données de l'application */
    first_use = gsb_grisbi_init_app ( );

    /* create the toplevel window and the main menu */
    vbox = gsb_main_create_main_window ( );
    gsb_grisbi_create_main_menu ( vbox );
    main_window_set_size_and_position ( );

    gtk_widget_show ( run.window );

#if IS_DEVELOPMENT_VERSION == 1
    dialog_message ( "development-version", VERSION );
#endif

    gsb_grisbi_load_file_if_necessary ( &opt );

    if ( first_use && !nom_fichier_comptes )
        gsb_assistant_first_run ();
    else
        display_tip ( FALSE );

    gtk_main ();

    /* sauvegarde les raccourcis claviers */
    gtk_accel_map_save ( gsb_dirs_get_accelerator_filename () );

    gsb_locale_shutdown ( );
    gsb_dirs_shutdown ( );

#ifdef HAVE_GOFFICE
    /* liberation libgoffice */
    libgoffice_shutdown ( );
#endif /* HAVE_GOFFICE */

#ifdef GSB_GMEMPROFILE
    g_mem_profile();
#endif

    exit ( 0 );

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
    gchar *tmp_str;

    g_printf ("Variables d'environnement :\n\n" );

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
    GFile *file = NULL;
    gboolean first_use = FALSE;
    gchar *string;
    gchar *tmp_dir;

    /* create the icon of grisbi (set in the panel of gnome or other) */
    string = g_build_filename ( gsb_dirs_get_pixmaps_dir ( ), "grisbi-logo.png", NULL );
    if ( g_file_test ( string, G_FILE_TEST_EXISTS ) )
        gtk_window_set_default_icon_from_file ( string, NULL );
    g_free (string);

    /* initialisation of the variables */
    /* load the CSS properties */
    css_provider = gtk_css_provider_get_default ();
    tmp_dir = g_strconcat ( gsb_dirs_get_ui_dir (), "/grisbi.css", NULL );
    file = g_file_new_for_path ( tmp_dir );
    if ( !gtk_css_provider_load_from_file ( css_provider, file, NULL ) )
        warning_debug (tmp_dir);
    g_free (tmp_dir);
    g_object_unref (file);

    gsb_color_initialise_couleurs_par_defaut ( );
    init_variables ();
    register_import_formats ();

    /* firt use ? */
    if ( ! gsb_file_config_load_config () )
        first_use = TRUE;

    return first_use;
}


/**
 * fonction appellée lorsqu'on appuie sur une touche
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
static gboolean main_window_key_press_event ( GtkWidget *widget,
                        GdkEventKey *event,
                        gpointer data )
{
    switch ( event -> keyval )
    {
        case GDK_KEY_F11 :
            if ( conf.full_screen )
                gtk_window_unfullscreen ( GTK_WINDOW ( widget ) );
            else
                gtk_window_fullscreen ( GTK_WINDOW ( widget ) );
        return TRUE;
    }

    return FALSE;
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
    GdkGeometry size_hints = {
    1200, 600, -10, -1, 1300, 750, 10, 10, 1.5, 1.5, GDK_GRAVITY_NORTH_WEST
  };

    /* create the toplevel window */
    run.window = gtk_window_new ( GTK_WINDOW_TOPLEVEL );

    g_signal_connect ( G_OBJECT ( run.window ),
                        "delete_event",
                        G_CALLBACK ( main_window_delete_event ),
                        NULL);
    g_signal_connect ( G_OBJECT ( run.window ),
                        "destroy",
                        G_CALLBACK ( main_window_destroy_event ),
                        NULL);
    g_signal_connect ( G_OBJECT ( run.window ),
                        "window-state-event",
                        G_CALLBACK (gsb_grisbi_change_state_window),
                        NULL );

    g_signal_connect ( G_OBJECT ( run.window ),
                        "key-press-event",
                        G_CALLBACK ( main_window_key_press_event ),
                        NULL );

    gtk_widget_set_size_request ( GTK_WIDGET ( run.window ), 1100, 800 );
    gtk_window_set_resizable ( GTK_WINDOW ( run.window ), TRUE );
    /* set the CSS properties */
    if ( css_provider )
        gtk_style_context_add_provider_for_screen ( gdk_display_get_default_screen (
                                                    gdk_display_get_default () ),
                                                    GTK_STYLE_PROVIDER ( css_provider ),
                                                    GTK_STYLE_PROVIDER_PRIORITY_USER );

    /* create the main window : a vbox */
    vbox = gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );
    g_object_set_data ( G_OBJECT ( run.window ), "main_vbox", vbox );
    gtk_container_add ( GTK_CONTAINER ( run.window ), vbox );
    g_signal_connect ( G_OBJECT ( vbox ),
                        "destroy",
                        G_CALLBACK ( gtk_widget_destroyed ),
                        &vbox );

    /* on fixe la taille mini remplacer main_box par plus pertinent.*/
    gtk_window_set_geometry_hints (GTK_WINDOW (run.window),
                                   vbox,
                                   &size_hints,
                                   GDK_HINT_RESIZE_INC |
                                   GDK_HINT_MIN_SIZE |
                                   GDK_HINT_BASE_SIZE);
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

    /* create the menus */
    menu_bar = init_menus ( vbox );

    /* unsensitive the necessaries menus */
    gsb_menu_set_menus_with_file_sensitive ( FALSE );

    /* charge les raccourcis claviers */
    gtk_accel_map_load ( gsb_dirs_get_accelerator_filename () );

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
        gtk_window_set_default_size ( GTK_WINDOW ( run.window ), conf.main_width, conf.main_height );
    else
        gtk_window_set_default_size ( GTK_WINDOW ( run.window ), 900, 600 );

    /* display window at position */
    gtk_window_move ( GTK_WINDOW ( run.window ), conf.root_x, conf.root_y );

    /* set the full screen if necessary */
    if ( conf.full_screen )
        gtk_window_fullscreen ( GTK_WINDOW ( run.window ) );

    /* put up the screen if necessary */
    if ( conf.maximize_screen )
        gtk_window_maximize ( GTK_WINDOW ( run.window ) );
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
    gboolean show;

    if ( event->changed_mask & GDK_WINDOW_STATE_MAXIMIZED )
    {
        show = !( event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED );

        gtk_window_set_has_resize_grip ( GTK_WINDOW ( window ), show );
        conf.maximize_screen = !show;
    }
    else if ( event->changed_mask & GDK_WINDOW_STATE_FULLSCREEN )
    {
        show = !( event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN );

        if ( show )
            gtk_widget_show  ( gsb_status_get_status_bar () );
        else
            gtk_widget_hide  ( gsb_status_get_status_bar () );

        conf.full_screen = !show;
    }

    /* return value */
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
    if ( conf.full_screen == 0 && conf.maximize_screen == 0 )
        gtk_window_get_position ( GTK_WINDOW ( run.window ), &conf.root_x, &conf.root_y );

    /* sauvegarde de la taille de la fenêtre si nécessaire */
    if ( conf.full_screen == 0 && conf.maximize_screen == 0 )
        gtk_window_get_size ( GTK_WINDOW ( run.window ), &conf.main_width, &conf.main_height );

    if ( !main_window_delete_event ( run.window, NULL ) )
        gtk_widget_destroy ( run.window );

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
    /* to prevent double destroy events on MacOS */
    if (run.window)
    {
        free_variables();
        run.window = NULL;

        gtk_main_quit();
    }
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
    gtk_window_set_title ( GTK_WINDOW ( run.window ), titre_grisbi );

    if ( titre_grisbi && strlen ( titre_grisbi ) > 0 )
    {
        gsb_main_page_update_homepage_title ( titre_grisbi );
        g_free ( titre_grisbi );
    }

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
    gchar *mon_thousands_sep;
    gchar *mon_decimal_point;
    gchar *positive_sign;
    gchar *negative_sign;
    gchar *currency_symbol;

    /* test local pour les nombres */
    conv = localeconv();

    currency_symbol = g_locale_to_utf8 ( conv->currency_symbol, -1, NULL, NULL, NULL );
    mon_thousands_sep = g_locale_to_utf8 ( conv->mon_thousands_sep, -1, NULL, NULL, NULL );
    mon_decimal_point = g_locale_to_utf8 ( conv->mon_decimal_point, -1, NULL, NULL, NULL );
    positive_sign = g_locale_to_utf8 ( conv->positive_sign, -1, NULL, NULL, NULL );
    negative_sign = g_locale_to_utf8 ( conv->negative_sign, -1, NULL, NULL, NULL );

    locale_str = g_strdup_printf ( "LANG = %s\n\n"
                        "Currency\n"
                        "\tcurrency_symbol   = %s\n"
                        "\tmon_thousands_sep = \"%s\"\n"
                        "\tmon_decimal_point = %s\n"
                        "\tpositive_sign     = \"%s\"\n"
                        "\tnegative_sign     = \"%s\"\n"
                        "\tp_cs_precedes     = \"%d\"\n"
                        "\tn_cs_precedes     = \"%d\"\n"
                        "\tp_sep_by_space    = \"%d\"\n"
                        "\tfrac_digits       = \"%d\"\n\n",
                        g_getenv ( "LANG"),
                        currency_symbol,
                        mon_thousands_sep,
                        mon_decimal_point,
                        positive_sign,
                        negative_sign,
                        conv->p_cs_precedes,
                        conv->n_cs_precedes,
                        conv->p_sep_by_space,
                        conv->frac_digits );

    g_free ( currency_symbol );
    g_free ( mon_thousands_sep );
    g_free ( mon_decimal_point );
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

    path_str = g_strdup_printf ( "Paths\n"
                        "\tg_get_user_data_dir ()               = %s\n"
                        "\tgsb_dirs_get_user_data_dir ()        = %s\n\n"
                        "\tg_get_user_config_dir ()             = %s\n"
                        "\tgsb_dirs_get_user_config_dir ()      = %s\n"
                        "\tgsb_dirs_get_grisbirc_filename ()    = %s\n"
                        "\tgsb_dirs_get_accelerator_filename () = %s\n\n"
                        "\tgsb_dirs_get_categories_dir ()       = %s\n"
                        "\tgsb_dirs_get_locale_dir ()           = %s\n"
                        "\tgsb_dirs_get_pixmaps_dir ()          = %s\n"
                        "\tgsb_dirs_get_ui_dir ()               = %s\n\n",
                        g_get_user_data_dir (),
                        gsb_dirs_get_user_data_dir (),
                        g_get_user_config_dir (),
                        gsb_dirs_get_user_config_dir (),
                        gsb_dirs_get_grisbirc_filename (),
                        gsb_dirs_get_accelerator_filename (),
                        gsb_dirs_get_categories_dir ( ),
                        gsb_dirs_get_locale_dir ( ),
                        gsb_dirs_get_pixmaps_dir ( ),
                        gsb_dirs_get_ui_dir ( ) );

    return path_str;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
