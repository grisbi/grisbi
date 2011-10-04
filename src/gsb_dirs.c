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
static gchar *_C_GRISBIRC;
static gchar *_C_OLD_GRISBIRC;
static gchar *_C_PATH_CONFIG;
static gchar *_C_PATH_DATA_FILES;
static gchar *_C_PATH_CONFIG_ACCELS;
static gchar *_my_get_XDG_grisbi_data_dir;
static gchar *_my_get_XDG_grisbirc_dir;
static gchar *_my_get_grisbirc_dir;
static gchar *_my_get_gsb_file_default_dir;


void gsb_dirs_init ( void )
{
#ifdef GTKOSXAPPLICATION
    if ( gtk_osxapplication_get_bundle_id ( ) )
    {
        gchar *res_path;

        res_path = gtk_osxapplication_get_resource_path ();
        categories_dir = g_build_filename ( res_path, "share/grisbi/categories", NULL );
        locale_dir = g_strconcat (res_path, "/share/locale", NULL );
        pixmaps_dir = g_strconcat (res_path, "/share/pixmaps/grisbi", NULL );
        plugins_dir = g_strconcat (res_path, "/lib/grisbi", NULL );
        ui_dir = g_strconcat (res_path, "/share/grisbi/ui", NULL );

        g_free ( res_path );
    }
    else
#endif
    {
        categories_dir = g_build_filename ( DATA_PATH, "categories", NULL );
        locale_dir = g_strdup ( LOCALEDIR );
        pixmaps_dir = g_strdup ( PIXMAPS_DIR );
        plugins_dir = g_strdup ( PLUGINS_DIR );
        ui_dir = g_strdup ( UI_DIR );
    }

    /*
     * FIXME: code from gsb_file_config.h
     */
#if IS_DEVELOPMENT_VERSION == 1
    _C_GRISBIRC = g_strconcat ( PACKAGE, "dev.conf", NULL);
#else
    _C_GRISBIRC = g_strconcat ( PACKAGE, ".conf", NULL);
#endif

    _C_OLD_GRISBIRC = g_strconcat ( ".", PACKAGE, "rc", NULL);

#ifndef _WIN32

#ifdef OS_OSX
    _C_PATH_CONFIG = g_strconcat (g_get_home_dir ( ), G_DIR_SEPARATOR_S,
                        "Library/Application Support/Grisbi/config", NULL);
    _C_PATH_DATA_FILES = g_strconcat (g_get_home_dir ( ), G_DIR_SEPARATOR_S,
                        "Library/Application Support/Grisbi/data", NULL);
#else /* OS_OSX */
    _C_PATH_CONFIG = g_strconcat (g_get_user_config_dir ( ), G_DIR_SEPARATOR_S,
                        "grisbi", NULL);
    _C_PATH_DATA_FILES = g_strconcat (g_get_user_data_dir ( ), G_DIR_SEPARATOR_S,
                        "grisbi", NULL);
#endif /* OS_OSX */

#else /* _WIN32 */

/* Some old Windows version have difficulties with dat starting file names */
    _C_PATH_CONFIG = win32_get_grisbirc_folder_path( );
    _C_PATH_DATA_FILES = g_strdup ( g_get_home_dir ( ) );

#endif /* _WIN32 */

    _C_PATH_CONFIG_ACCELS = g_strconcat ( _C_PATH_CONFIG, G_DIR_SEPARATOR_S,
                        "grisbi-accels", NULL );

    /*
     * FIXME: code from utils_files.c
     */
#ifndef _WIN32
    _my_get_XDG_grisbirc_dir = g_strdup ( _C_PATH_CONFIG );
    _my_get_XDG_grisbi_data_dir = g_strdup ( _C_PATH_DATA_FILES );
    _my_get_grisbirc_dir = g_strdup ( g_get_home_dir () );
    _my_get_gsb_file_default_dir = g_strdup ( g_get_home_dir() );
#else
    _my_get_XDG_grisbirc_dir = g_strdup ( win32_get_grisbirc_folder_path() );
    _my_get_XDG_grisbi_data_dir = g_strdup ( g_get_home_dir () );
    _my_get_grisbirc_dir = g_strdup ( win32_get_grisbirc_folder_path() );
    _my_get_gsb_file_default_dir = g_strdup ( win32_get_my_documents_folder_path() );
#endif
}


void gsb_dirs_shutdown ( void )
{
    g_free ( categories_dir );
    g_free ( locale_dir );
    g_free ( pixmaps_dir );
    g_free ( plugins_dir );
    g_free ( ui_dir );
    g_free ( _C_GRISBIRC );
    g_free ( _C_OLD_GRISBIRC );
    g_free ( _C_PATH_CONFIG );
    g_free ( _C_PATH_DATA_FILES );
    g_free ( _C_PATH_CONFIG_ACCELS );
    g_free ( _my_get_XDG_grisbi_data_dir );
    g_free ( _my_get_XDG_grisbirc_dir );
    g_free ( _my_get_grisbirc_dir );
    g_free ( _my_get_gsb_file_default_dir );
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


const gchar *C_GRISBIRC ( void )
{
    return _C_GRISBIRC;
}


const gchar *C_OLD_GRISBIRC ( void )
{
    return _C_OLD_GRISBIRC;
}


const gchar *C_PATH_CONFIG ( void )
{
    return _C_PATH_CONFIG;
}


const gchar *C_PATH_DATA_FILES ( void )
{
    return _C_PATH_DATA_FILES;
}


const gchar *C_PATH_CONFIG_ACCELS ( void )
{
    return _C_PATH_CONFIG_ACCELS;
}


/**
 * return the absolute path of where the configuration file should be located
 * On UNIX platforms this is determined using the mechanisms described 
 * in the  XDG Base Directory Specification
 * on Windows based systems return APPDATA\Grisbi
 * 
 * \return the absolute path of the configuration file directory
 */
const gchar *my_get_XDG_grisbirc_dir ( void )
{
    return _my_get_XDG_grisbirc_dir;
}


/**
 * return the absolute path of where the data files should be located
 * On UNIX platforms this is determined using the mechanisms described 
 * in the  XDG Base Directory Specification
 * on Windows based systems return APPDATA\Grisbi
 * 
 * \return the absolute path of the home directory
 */
const gchar *my_get_XDG_grisbi_data_dir ( void )
{
    return _my_get_XDG_grisbi_data_dir;
}


/**
 * return the absolute path of where the configuration file should be located
 * on Un*x based system return $HOME
 * on Windows based systems return APPDATA\Grisbi
 * 
 * \return the absolute path of the configuration file directory
 */
const gchar *my_get_grisbirc_dir ( void )
{
    return _my_get_grisbirc_dir;
}


/**
 * return the absolute path of the default accounts files location
 * on Un*x based system return $HOME
 * on Windows based systems return "My Documents"
 * 
 * \return the absolute path of the configuration file directory
 */
const gchar *my_get_gsb_file_default_dir ( void )
{
    return _my_get_gsb_file_default_dir;
}
