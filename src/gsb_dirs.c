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

static gchar *locale_dir;
static gchar *pixmaps_dir;
static gchar *plugins_dir;


void gsb_dirs_init ( void )
{
#ifdef GTKOSXAPPLICATION
    if ( gtk_osxapplication_get_bundle_id ( ) )
    {
        gchar *res_path = gtk_osxapplication_get_resource_path ();
        locale_dir = g_strconcat (res_path, "/share/locale", NULL );
        pixmaps_dir = g_strconcat (res_path, "/share/pixmaps/grisbi", NULL );
        plugins_dir = g_strconcat (res_path, "/lib/grisbi", NULL );
        g_free ( res_path );
    }
    else
#endif
    {
        locale_dir = g_strdup ( LOCALEDIR );
        pixmaps_dir = g_strdup ( PIXMAPS_DIR );
        plugins_dir = g_strdup ( PLUGINS_DIR );
    }
}


void gsb_dirs_shutdown ( void )
{
    g_free ( locale_dir );
    g_free ( pixmaps_dir );
    g_free ( plugins_dir );
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


/**
 * return the absolute path of where the configuration file should be located
 * on Un*x based system return $HOME
 * on Windows based systems return APPDATA\Grisbi
 * 
 * \return the absolute path of the configuration file directory
 */
gchar* my_get_grisbirc_dir(void)
{
#ifndef _WIN32
    return (gchar *) g_get_home_dir ();
#else
    return win32_get_grisbirc_folder_path();
#endif
}


/**
 * return the absolute path of where the configuration file should be located
 * On UNIX platforms this is determined using the mechanisms described 
 * in the  XDG Base Directory Specification
 * on Windows based systems return APPDATA\Grisbi
 * 
 * \return the absolute path of the configuration file directory
 */
gchar* my_get_XDG_grisbirc_dir(void)
{
#ifndef _WIN32
    return (gchar *) C_PATH_CONFIG;
#else
    return win32_get_grisbirc_folder_path();
#endif
}


/**
 * return the absolute path of where the data files should be located
 * On UNIX platforms this is determined using the mechanisms described 
 * in the  XDG Base Directory Specification
 * on Windows based systems return APPDATA\Grisbi
 * 
 * \return the absolute path of the home directory
 */
gchar* my_get_XDG_grisbi_data_dir ( void )
{
#ifndef _WIN32
    return (gchar *) C_PATH_DATA_FILES;
#else
    return g_get_home_dir ();
#endif
}

/**
 * return the absolute path of the default accounts files location
 * on Un*x based system return $HOME
 * on Windows based systems return "My Documents"
 * 
 * \return the absolute path of the configuration file directory
 */
gchar* my_get_gsb_file_default_dir ( void )
{
#ifndef _WIN32
    return (gchar *) g_get_home_dir();
#else
    return win32_get_my_documents_folder_path();
#endif
}
