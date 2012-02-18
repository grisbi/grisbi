/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)         2011 Grisbi Development Team                     */
/*             http://www.grisbi.org                                          */
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

#ifdef GTKOSXAPPLICATION
    #include <gtkosxapplication.h>
#endif

#ifdef G_OS_WIN32
    #include <shlobj.h>
    #include <tchar.h>
#endif /* !G_OS_WIN32 */

#include "include.h"
#include "gsb_file_config.h"
#include "gsb_dirs.h"
#include "structures.h"

static gchar *categories_dir;
static gchar *home_dir;
static gchar *locale_dir;
static gchar *pixmaps_dir;
static gchar *plugins_dir;
static gchar *ui_dir;
static gchar *user_config_dir;
static gchar *user_data_dir;


/**
 * Initialise les répertoires utilisés par Grisbi
 *
 * \param
 *
 * \return
 */
void gsb_dirs_init ( void )
{
#ifdef G_OS_WIN32
    gchar *win32_dir;
    TCHAR tcHomePath[MAX_PATH];

    win32_dir = g_win32_get_package_installation_directory_of_module (NULL);

    categories_dir = g_build_filename ( win32_dir, "share/categories", NULL );
    locale_dir = g_build_filename ( win32_dir, "share/locale", NULL );
    pixmaps_dir = g_build_filename ( win32_dir, "pixmaps", NULL );
    plugins_dir = g_build_filename ( win32_dir, "lib/grisbi", NULL );
    ui_dir = g_build_filename ( win32_dir, "share/ui", NULL );

    SHGetSpecialFolderPath ( 0, tcHomePath, CSIDL_PERSONAL, FALSE );

    home_dir = g_strdup ( ( gchar *) tcHomePath );

    user_config_dir = g_build_filename ( g_get_user_config_dir ( ),  "grisbi", NULL );
    user_data_dir = g_build_filename ( g_get_user_data_dir ( ),  "grisbi", NULL );

    g_free (win32_dir);
#else /* !G_OS_WIN32 */
    #ifdef GTKOSXAPPLICATION
        if ( quartz_application_get_bundle_id ( ) )
        {
            gchar *res_path;

            res_path = quartz_application_get_resource_path ();

            categories_dir = g_build_filename ( res_path, "share/grisbi/categories", NULL );
            locale_dir = g_strconcat (res_path, "/share/locale", NULL );
            pixmaps_dir = g_strconcat (res_path, "/share/pixmaps/grisbi", NULL );
            plugins_dir = g_strconcat (res_path, "/lib/grisbi", NULL );
            ui_dir = g_strconcat (res_path, "/share/grisbi/ui", NULL );

            home_dir = g_strdup ( g_getenv ( "HOME" ) );;

            user_config_dir = g_build_filename ( g_get_home_dir ( ),
                        "Library/Application Support/Grisbi/config",
                        NULL);
            user_data_dir = g_build_filename ( g_get_home_dir ( ),
                        "Library/Application Support/Grisbi/data",
                        NULL );

            g_free ( res_path );
        }
        else
    #endif /* !GTKOSXAPPLICATION */
    {
        categories_dir = g_build_filename ( DATA_PATH, "categories", NULL );
        locale_dir = g_strdup ( LOCALEDIR );
        pixmaps_dir = g_strdup ( PIXMAPS_DIR );
        plugins_dir = g_strdup ( PLUGINS_DIR );
        ui_dir = g_strdup ( UI_DIR );

        home_dir = g_strdup ( g_getenv ( "HOME" ) );

        #ifdef OS_OSX
            user_config_dir = g_build_filename ( g_get_home_dir ( ),
                        "Library/Application Support/Grisbi/config",
                        NULL);
        #else
            user_config_dir = g_build_filename ( g_get_user_config_dir ( ),  "grisbi", NULL );
        #endif /* !OS_OSX */

        user_data_dir = g_build_filename ( g_get_user_data_dir ( ),  "grisbi", NULL );
    }
#endif /* !G_OS_WIN32 */
}


/**
 * Libère la mémoire utilisée par les nom des répertoires utiles
 *
 * \param
 *
 * \return
 */
void gsb_dirs_shutdown ( void )
{
    g_free ( categories_dir );
    g_free ( home_dir );
    g_free ( locale_dir );
    g_free ( pixmaps_dir );
    g_free ( plugins_dir );
    g_free ( ui_dir );
    g_free ( user_config_dir );
    g_free ( user_data_dir );
}


/**
 * Renvoie le répertoire pour les catégories
 *
 * \param
 *
 * \return the absolute path of the directory
 */
const gchar *gsb_dirs_get_categories_dir ( void )
{
    return categories_dir;
}


/**
 *  Renvoie le répertoire pour la locale
 *
 * \param
 *
 * \return the absolute path of the directory
 */
const gchar *gsb_dirs_get_locale_dir ( void )
{
    return locale_dir;
}


/**
 * Renvoie le répertoire pour les images
 *
 * \param
 *
 * \return the absolute path of the directory
 */
const gchar *gsb_dirs_get_pixmaps_dir ( void )
{
    return pixmaps_dir;
}


/**
 * Renvoie le répertoire pour les plugins
 *
 * \param
 *
 * \return the absolute path of the directory
 */
const gchar *gsb_dirs_get_plugins_dir ( void )
{
    return plugins_dir;
}


/**
 * Renvoie le répertoire pour fichiers d'IHM
 *
 * \param
 *
 * \return the absolute path of the directory
 */
const gchar *gsb_dirs_get_ui_dir ( void )
{
    return ui_dir;
}


/**
 * return the absolute path of where the configuration file should be located
 * On UNIX platforms this is determined using the mechanisms described 
 * in the  XDG Base Directory Specification
 * on Windows based systems return APPDATA\Grisbi
 *
 * \param
 * 
 * \return the absolute path of the configuration directory
 */
const gchar *gsb_dirs_get_user_config_dir ( void )
{
    return user_config_dir;
}


/**
 * return the absolute path of where the data files should be located
 * On UNIX platforms this is determined using the mechanisms described
 * in the  XDG Base Directory Specification
 * on Windows based systems return APPDATA\Grisbi
 *
 * \param
 *
 * \return the absolute path of the user data directory
 */
const gchar *gsb_dirs_get_user_data_dir ( void )
{
    return user_data_dir;
}

/**
 * return the absolute path of the default accounts files location
 * on Un*x based system return $HOME
 * on Windows based systems return "My Documents"
 * 
 * \param
 *
 * \return the absolute path of home directory
 */
const gchar *gsb_dirs_get_home_dir ( void )
{
    return home_dir;
}


/**
 * retourne une chaine avec les principaux répertoires
 *
 * \param
 *
 * \return must be freed
 */
gchar *gsb_dirs_get_print_dir_var ( void )
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
                        home_dir,
                        user_config_dir,
                        conf_filename,
                        accel_filename,
                        user_data_dir,
                        DATA_PATH,
                        categories_dir,
                        locale_dir,
                        plugins_dir,
                        pixmaps_dir,
                        ui_dir );

    g_free ( accel_filename );

    return path_str;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
