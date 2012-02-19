/* *******************************************************************************/
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

/* Gedit a servi de modèle pour cet objet */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "parse_cmdline.h"
#include "dialog.h"
#include "gsb_dirs.h"
#include "main.h"
/*END_INCLUDE*/

#define GRISBI_COMMAND_LINE_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GRISBI_TYPE_COMMAND_LINE, GrisbiCommandLinePrivate))

struct _GrisbiCommandLinePrivate
{
    gint debug_level;
    gchar *config_file;
    gchar **files_args;

    gchar *conf_filename;
    GSList *file_list;
};

G_DEFINE_TYPE (GrisbiCommandLine, grisbi_command_line, G_TYPE_INITIALLY_UNOWNED)

static void grisbi_command_line_finalize ( GObject *object )
{
    GrisbiCommandLinePrivate *priv = GRISBI_COMMAND_LINE ( object )->priv;

    g_free ( priv->config_file );
    g_strfreev ( priv->files_args );
    g_slist_free_full ( priv->file_list, g_object_unref );

    G_OBJECT_CLASS ( grisbi_command_line_parent_class)->finalize ( object );
}


static void grisbi_command_line_class_init ( GrisbiCommandLineClass *klass )
{
    GObjectClass *object_class = G_OBJECT_CLASS ( klass );

    object_class->finalize = grisbi_command_line_finalize;

    g_type_class_add_private ( object_class, sizeof ( GrisbiCommandLinePrivate ) );
}


static void grisbi_command_line_init ( GrisbiCommandLine *self )
{
    self->priv = GRISBI_COMMAND_LINE_GET_PRIVATE ( self );

    self->priv->debug_level = -1;
}


GrisbiCommandLine *grisbi_command_line_get_default ( void )
{
    GrisbiCommandLine *command_line;

    command_line = g_object_new ( GRISBI_TYPE_COMMAND_LINE, NULL );

    if ( g_object_is_floating ( command_line ) )
    {
        g_object_ref_sink ( command_line );
    }
    else
    {
        g_object_unref ( command_line );
    }

    return command_line;
}


/**
 * cherche le nom_long d'un fichier de configuration passé en paramètre
 *
 * \param nom de l'option
 *
 * \return un nom valide ou NULL;
 **/
static gchar *grisbi_command_line_make_conf_filename ( gchar *config_file )
{
    gchar *tmp_name = NULL;

    if ( config_file == NULL )
        return NULL;

    if ( g_file_test ( config_file, G_FILE_TEST_EXISTS ) )
    {
        tmp_name = g_strdup ( config_file );

        return tmp_name;
    }
    else
    {
        tmp_name = g_build_filename ( gsb_dirs_get_user_config_dir (), config_file, NULL );
        if ( g_file_test ( tmp_name, G_FILE_TEST_EXISTS ) )
        {
            return tmp_name;
        }
        else
        {
            g_free ( tmp_name );
            tmp_name = g_build_filename ( gsb_dirs_get_home_dir (), config_file, NULL );
            if ( g_file_test ( tmp_name, G_FILE_TEST_EXISTS ) )
            {
                return tmp_name;
            }
            else
            {
                g_free ( tmp_name );
                return NULL;
            }
        }
    }

    return NULL;
}


static void grisbi_command_line_init_file_list ( GrisbiCommandLine *command_line )
{
    gint i;

    if (!command_line->priv->files_args)
    {
        return;
    }

    for (i = 0; command_line->priv->files_args[i]; i++)
    {
        gchar *filename;

        filename = g_strdup ( command_line->priv->files_args[i] );

        command_line->priv->file_list = g_slist_prepend ( command_line->priv->file_list, filename );
    }

    command_line->priv->file_list = g_slist_reverse ( command_line->priv->file_list );
}


gint grisbi_command_line_parse ( GrisbiCommandLine *command_line,
                        gint argc,
                        gchar **argv )
{
    GOptionContext *context;
    GError *error = NULL;
    gchar *conf_filename;
    gint return_value = 0;
    const GOptionEntry options[] =
    {
        /* Version */
        {
            "version", 'v', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
            gsb_main_show_version, N_("Show the application's version"), NULL
        },

        /* debug level */
        {
            "debug", 'd', 0, G_OPTION_ARG_INT, &command_line->priv->debug_level,
            N_("Debug mode: level 0-5"), NULL
        },

        /* config file */
        {
            "file.conf", 'c', 0, G_OPTION_ARG_FILENAME, &command_line->priv->config_file,
            N_("[FILE.CONF]"), NULL
        },

        /* Open a new window */
/*         {
 *             "new-window", 'w', 0, G_OPTION_ARG_NONE, &command_line->priv->new_window,
 *             N_("Create a new top-level window in an existing instance of grisbi"), NULL
 *         },
 */

        /* Run in the background */
/*         {
 *             "background", 'b', 0, G_OPTION_ARG_NONE, &command_line->priv->background,
 *             N_("Run grisbi in the background"), NULL
 *         },
 */

        /* collects files arguments */
        {
            G_OPTION_REMAINING, '\0', 0, G_OPTION_ARG_FILENAME_ARRAY, &command_line->priv->files_args,
            NULL, N_("[FILE...]")
        },

        {NULL}
    };

    /* Setup command line options */
    context = g_option_context_new ( _("- Personnal finances manager") );
    g_option_context_set_summary ( context,
                        N_("Grisbi can manage the accounts of a family or a small association.") );
    g_option_context_set_translation_domain ( context, PACKAGE );

    g_option_context_add_main_entries ( context, options, PACKAGE );
    g_option_context_add_group ( context, gtk_get_option_group ( FALSE ) );
    g_option_context_set_translation_domain ( context, PACKAGE );

    if ( !g_option_context_parse ( context, &argc, &argv, &error ) )
    {
        if (error)
        {
            g_print ("option parsing failed: %s\n", error->message);
            g_error_free ( error );
        }

        return_value = 1;
    }

    g_option_context_free (context);

    /* test de la validité du fichier de configuration */
    conf_filename = grisbi_command_line_make_conf_filename ( command_line->priv->config_file );
    command_line->priv->conf_filename = conf_filename;

    /* Constitution de la liste des fichiers */
    grisbi_command_line_init_file_list ( command_line );

    return return_value;
}


gchar *grisbi_command_line_get_config_file ( GrisbiCommandLine *command_line )
{
    g_return_val_if_fail ( GRISBI_IS_COMMAND_LINE ( command_line ), NULL );

    return command_line->priv->conf_filename;
}


gint grisbi_command_line_get_debug_level ( GrisbiCommandLine *command_line )
{
    g_return_val_if_fail ( GRISBI_IS_COMMAND_LINE ( command_line ), 0 );

    return command_line->priv->debug_level;
}


GSList *grisbi_command_line_get_file_list ( GrisbiCommandLine *command_line )
{
    g_return_val_if_fail ( GRISBI_IS_COMMAND_LINE ( command_line ), NULL );

    return command_line->priv->file_list;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
