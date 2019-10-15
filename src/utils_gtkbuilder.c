/* ************************************************************************** */
/*                                                                            */
/*     copyright (c)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2004-2009 Benjamin Drieu (bdrieu@april.org)                       */
/*                      2008-2011 Pierre Biava (grisbi@pierre.biava.name)     */
/*          https://www.grisbi.org/                                            */
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
#include "config.h"
#endif

#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "utils_gtkbuilder.h"
#include "dialog.h"
#include "gsb_dirs.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/


/**
 * returns the full name of the GUI file
 *
 * \param name  name of Xml file
 *
 * \return      full filename
 *
 * */
gchar *utils_gtkbuilder_get_full_path ( const gchar *name )
{
    gchar *filename;

    filename = g_build_filename ( gsb_dirs_get_ui_dir ( ), name, NULL );

    return filename;
}


/**
 * returns the desired widget from an ui file
 *
 * \param builder
 * \param name of the widget in ui file
 * \param name of the child of gtkbuilder widget (optional)
 *
 * \return the desired widget
 *
 * */
GtkWidget *utils_gtkbuilder_get_widget_by_name ( GtkBuilder *builder,
                        const gchar *gtk_builder_name,
                        const gchar *optional_name )
{
    GtkWidget *w_1;
    GtkWidget *w_2 = NULL;

    w_1 = GTK_WIDGET ( gtk_builder_get_object ( builder, gtk_builder_name ) );

    if ( optional_name == NULL )
        return w_1;

    w_2 = GTK_WIDGET ( g_object_get_data ( G_OBJECT ( w_1 ), optional_name ) );

    return w_2;
}


/**
 *
 *
 * \param
 * \param
 *
 * \return
 * */
gint utils_gtkbuilder_merge_ui_data_in_builder ( GtkBuilder *builder,
                    const gchar *ui_name )
{
    gchar *filename;
    gint result;
    GError *error = NULL;

    /* Chargement du XML dans bet_graph_builder */
    filename = utils_gtkbuilder_get_full_path ( ui_name );
    if ( !g_file_test ( filename, G_FILE_TEST_EXISTS ) )
    {
        gchar* tmpstr;

        tmpstr = g_strdup_printf ( _("Cannot open file '%s': %s"),
                        filename,
                        _("File does not exist") );
        dialogue_error ( tmpstr );
        g_free ( tmpstr );
        g_free ( filename );
        return 0;
    }

    result = gtk_builder_add_from_file ( builder, filename, &error );
    if ( result == 0 )
    {
        g_error ("%s", error->message);
        g_error_free ( error );
    }
    g_free ( filename );

    return result;
}


/**
 *  return a new GtkBuilder initiate with a file
 *
 * \param gchar         short name of UI file
 *
 * \return GtkBuilder    new GtkBuildeR
 * */
GtkBuilder *utils_gtk_builder_new_from_file ( const gchar *ui_name )
{
    GtkBuilder *builder;
    gchar *filename;

    /* obtention du nom long du fichier UI */
    filename = utils_gtkbuilder_get_full_path ( ui_name );
    if ( !g_file_test ( filename, G_FILE_TEST_EXISTS ) )
    {
        gchar* tmpstr;

        tmpstr = g_strdup_printf ( _("Cannot open file '%s': %s"),
                        filename,
                        _("File does not exist") );
        dialogue_error ( tmpstr );
        g_free ( tmpstr );
        g_free ( filename );

        return NULL;
    }

    builder = gtk_builder_new_from_file ( filename );

    g_free ( filename );

    return builder;
}


/**
 *
 *
 * \param
 *
 * \return
 * */
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
