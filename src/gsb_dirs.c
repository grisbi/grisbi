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
#include "config.h"
#endif

#ifdef GTKOSXAPPLICATION
#include <gtkosxapplication.h>
#endif

#include "include.h"
#include "gsb_dirs.h"
#include "structures.h"

static gchar *help_dir;
static gchar *locale_dir;
static gchar *pixmaps_dir;
static gchar *categories_dir;
static gchar *ui_dir;
static gchar *user_config_dir;
static gchar *user_data_dir;
static gchar *user_default_dir;

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 *
 *
 * \param
 *
 * \return
 **/
void gsb_dirs_init (char* gsb_bin_path)
{
	gchar *grisbi_dir;
    gchar *local_ui_dir;
    GDir *local_ui_handle;

    /* Get the grisbi executable directory as it may be useful when running dev instances */
    grisbi_dir = g_path_get_dirname (gsb_bin_path);
    local_ui_dir = g_strconcat (grisbi_dir, "/ui", NULL);

	g_free (grisbi_dir);

#ifdef G_OS_WIN32
{
    gchar *dir;

    dir = g_win32_get_package_installation_directory_of_module (NULL);
    categories_dir = g_build_filename (dir, "\\share\\grisbi\\categories", NULL);
    help_dir = g_build_filename (dir, "\\share\\doc\\grisbi", NULL);
    locale_dir = g_strconcat (dir, "\\share\\locale", NULL);
    pixmaps_dir = g_strconcat (dir, "\\share\\pixmaps\\grisbi", NULL);
    ui_dir = g_strconcat (dir, "\\share\\grisbi\\ui", NULL);

    g_free (dir);

    user_config_dir = g_build_filename (g_get_user_config_dir (), "grisbi", NULL);
    user_data_dir = g_build_filename (g_get_user_data_dir (), "grisbi", NULL);
    user_default_dir = g_strdup (g_get_user_special_dir (G_USER_DIRECTORY_DOCUMENTS));
}
#else
{
    #ifdef OS_OSX
    {
        user_config_dir = g_build_filename (g_get_home_dir (),
                        "Library/Application Support/Grisbi/config", NULL);
        user_data_dir = g_build_filename (g_get_home_dir (),
                        "Library/Application Support/Grisbi/data", NULL);
        user_default_dir = g_strdup (g_get_home_dir());

        #ifdef GTKOSXAPPLICATION
        {
            if (gtkosx_application_get_bundle_id ())
            {
                gchar *res_path = gtkosx_application_get_resource_path ();

                categories_dir = g_build_filename (res_path, "share/grisbi/categories", NULL);
                help_dir = g_build_filename (res_path, "share/doc/grisbi", NULL);
                locale_dir = g_build_filename (res_path, "/share/locale", NULL);
                pixmaps_dir = g_build_filename (res_path, "/share/pixmaps/grisbi", NULL);
                ui_dir = g_build_filename (res_path, "/share/grisbi/ui", NULL);

                g_free (res_path);
            }
            else
            {
                categories_dir = g_build_filename (DATA_PATH, "categories", NULL);
                help_dir = g_strdup (HELP_PATH);
                locale_dir = g_strdup (LOCALEDIR);
                pixmaps_dir = g_strdup (PIXMAPS_DIR);
                ui_dir = g_strdup (UI_DIR);
            }
        }
        #endif /* GTKOSXAPPLICATION */
    }
    #else
    {
        categories_dir = g_build_filename (DATA_PATH, "categories", NULL);
        help_dir = g_strdup (HELP_PATH);
        locale_dir = g_strdup (LOCALEDIR);
        pixmaps_dir = g_strdup (PIXMAPS_DIR);
        ui_dir = g_strdup (UI_DIR);

        user_config_dir = g_build_filename (g_get_user_config_dir (), "grisbi", NULL);
        user_data_dir = g_build_filename (g_get_user_data_dir (), "grisbi", NULL);
        user_default_dir = g_strdup (g_get_home_dir());
    }
    #endif /* OS_OSX */
}
#endif /* G_OS_WIN32 */

    local_ui_handle = g_dir_open (local_ui_dir, 0, NULL);
    if (NULL != local_ui_handle)
    {
        g_free (ui_dir);
        ui_dir = local_ui_dir;
    }
	else
		g_free (local_ui_dir);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void gsb_dirs_shutdown (void)
{
    g_free (categories_dir);
    g_free (help_dir);
    g_free (locale_dir);
    g_free (pixmaps_dir);
    g_free (ui_dir);
    g_free (user_config_dir);
    g_free (user_data_dir);
    g_free (user_default_dir);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
const gchar *gsb_dirs_get_categories_dir (void)
{
    return categories_dir;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
const gchar *gsb_dirs_get_help_dir (void)
{
    return help_dir;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
const gchar *gsb_dirs_get_locale_dir (void)
{
    return locale_dir;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
const gchar *gsb_dirs_get_pixmaps_dir (void)
{
    return pixmaps_dir;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
const gchar *gsb_dirs_get_ui_dir (void)
{
    return ui_dir;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
/**
 *
 *
 * \param
 *
 * \return
 **/
const gchar *gsb_dirs_get_user_config_dir (void)
{
    return user_config_dir;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
const gchar *gsb_dirs_get_user_data_dir (void)
{
    return user_data_dir;
}

/**
 * return the absolute path of the default accounts files location
 * on Un*x based system return $HOME
 * on Windows based systems return "My Documents"
 *
 * \return the absolute path of the configuration file directory
 */
const gchar *gsb_dirs_get_default_dir (void)
{
    return user_default_dir;
}

/**
 *
 *
 * \param
 *
 *  \return string  must be freed
 */
gchar *gsb_dirs_get_print_dir_var (void)
{
    gchar *path_str = NULL;

	path_str = g_strdup_printf ("Paths\n"
								"\tuser_default_dir             = %s\n"
								"\tuser_data_dir                = %s\n"
								"\tuser_config_dir              = %s\n\n"
								"\tG_USER_DIRECTORY_DOCUMENTS   = %s\n"
								"\tG_USER_DIRECTORY_DOWNLOAD    = %s\n\n"
								"\tcategories_dir               = %s\n"
								"\tlocale_dir                   = %s\n"
								"\tpixmaps_dir                  = %s\n"
								"\thelp_dir                     = %s\n"
								"\tui_dir                       = %s\n\n",
								gsb_dirs_get_default_dir (),
								gsb_dirs_get_user_data_dir (),
								gsb_dirs_get_user_config_dir (),
								g_get_user_special_dir (G_USER_DIRECTORY_DOCUMENTS),
								g_get_user_special_dir (G_USER_DIRECTORY_DOWNLOAD),
								gsb_dirs_get_categories_dir (),
								gsb_dirs_get_locale_dir (),
								gsb_dirs_get_pixmaps_dir (),
								gsb_dirs_get_help_dir(),
								gsb_dirs_get_ui_dir ()
								);

    return path_str;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
