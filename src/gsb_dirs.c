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
