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

#include "include.h"
#include "gsb_dirs.h"
#include "structures.h"

static gchar *locale_dir;
static gchar *pixmaps_dir;
static gchar *plugins_dir;
static gchar *categories_dir;
static gchar *ui_dir;
static gchar *user_config_dir;
static gchar *user_data_dir;
static gchar *user_default_dir;

static gchar *grisbirc_filename;
static gchar *accelerator_filename;


void gsb_dirs_init ( void )
{
#ifdef G_OS_WIN32
    {
        gchar *dir;

        dir = g_win32_get_package_installation_directory_of_module ( NULL );

        categories_dir = g_build_filename ( dir, "share/grisbi/categories", NULL );
        locale_dir = g_strconcat ( dir, "/share/locale", NULL );
        pixmaps_dir = g_strconcat ( dir, "/share/pixmaps/grisbi", NULL );
        plugins_dir = g_strconcat ( dir, "/lib/grisbi", NULL );
        ui_dir = g_strconcat ( dir, "/share/grisbi/ui", NULL );

        g_free ( dir );

        user_config_dir = g_build_filename ( g_get_user_config_dir (), "grisbi", NULL);
        user_data_dir = g_build_filename ( g_get_user_data_dir (), "grisbi", NULL);
        user_default_dir = g_strdup ( win32_get_my_documents_folder_path () );
    }
#else
#ifdef GTKOSXAPPLICATION
    if ( gtkosx_application_get_bundle_id ( ) )
    {
        gchar *res_path;

        res_path = gtkosx_application_get_resource_path ();
        categories_dir = g_build_filename ( res_path, "share/grisbi/categories", NULL );
        locale_dir = g_strconcat (res_path, "/share/locale", NULL );
        pixmaps_dir = g_strconcat (res_path, "/share/pixmaps/grisbi", NULL );
        plugins_dir = g_strconcat (res_path, "/lib/grisbi", NULL );
        ui_dir = g_strconcat (res_path, "/share/grisbi/ui", NULL );

        g_free ( res_path );

        user_config_dir = g_build_filename ( g_get_home_dir (),
                        "Library/Application Support/Grisbi/config", NULL );
        user_data_dir = g_build_filename ( g_get_home_dir (),
                        "Library/Application Support/Grisbi/data", NULL );
        user_default_dir = g_strdup ( g_get_home_dir() );

    }
#else
    {
        categories_dir = g_build_filename ( DATA_PATH, "categories", NULL );
        locale_dir = g_strdup ( LOCALEDIR );
        pixmaps_dir = g_strdup ( PIXMAPS_DIR );
        plugins_dir = g_strdup ( PLUGINS_DIR );
        ui_dir = g_strdup ( UI_DIR );

        user_config_dir = g_build_filename ( g_get_user_config_dir (), "grisbi", NULL);
        user_data_dir = g_build_filename ( g_get_user_data_dir (), "grisbi", NULL);
        user_default_dir = g_strdup ( g_get_home_dir() );
    }
#endif /* GTKOSXAPPLICATION */
#endif

    accelerator_filename = g_build_filename ( user_config_dir, "grisbi-accels", NULL );
}


void gsb_dirs_shutdown ( void )
{
    g_free ( categories_dir );
    g_free ( locale_dir );
    g_free ( pixmaps_dir );
    g_free ( plugins_dir );
    g_free ( ui_dir );
    g_free ( user_config_dir );
    g_free ( user_data_dir );

    g_free ( grisbirc_filename );
    g_free ( accelerator_filename );
    g_free ( user_default_dir );
}


const gchar *gsb_dirs_get_categories_dir ( void )
{
    return categories_dir;
}


const gchar *gsb_dirs_get_locale_dir ( void )
{
    return locale_dir;
}


const gchar *gsb_dirs_get_pixmaps_dir ( void )
{
    return pixmaps_dir;
}


const gchar *gsb_dirs_get_plugins_dir ( void )
{
    return plugins_dir;
}


const gchar *gsb_dirs_get_ui_dir ( void )
{
    return ui_dir;
}


const gchar *gsb_dirs_get_grisbirc_filename ( void )
{
    gchar *filename;

#if IS_DEVELOPMENT_VERSION == 1
    filename = g_strconcat ( PACKAGE, "dev.conf", NULL);
#else
    filename = g_strconcat ( PACKAGE, ".conf", NULL);
#endif

    grisbirc_filename = g_build_filename ( user_config_dir, filename, NULL );
    g_free ( filename );

    return grisbirc_filename;
}


const gchar *gsb_dirs_get_user_config_dir ( void )
{
    return user_config_dir;
}


const gchar *gsb_dirs_get_user_data_dir ( void )
{
    return user_data_dir;
}

/**
 * return the accelerator filename
 *
 * \param
 *
 * \return the accelerator filename with an absolute path
 */
const gchar *gsb_dirs_get_accelerator_filename ( void )
{
    return accelerator_filename;
}


/**
 * return the absolute path of the default accounts files location
 * on Un*x based system return $HOME
 * on Windows based systems return "My Documents"
 *
 * \return the absolute path of the configuration file directory
 */
const gchar *gsb_dirs_get_default_dir ( void )
{
    return user_default_dir;
}

