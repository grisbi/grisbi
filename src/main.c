/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2018 Pierre Biava (grisbi@pierre.biava.name)                    */
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
#include "config.h"
#endif

#include "include.h"
#include <gtk/gtk.h>
#include <libintl.h>

#ifdef HAVE_GOFFICE
#include <goffice/goffice.h>
#endif /* HAVE_GOFFICE */

#ifdef G_OS_WIN32
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <unistd.h>
#endif /* WIN32 */

/*START_INCLUDE*/
#include "grisbi_app.h"
#include "gsb_dirs.h"
#include "gsb_locale.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/


/**
 * Main function
 *
 * @param argc number of arguments
 * @param argv arguments
 *
 * @return Nothing
 */
int main (int argc, char **argv)
{
    GrisbiApp *app;
	gint status;

    /* On commence par initialiser les répertoires */
    gsb_dirs_init (argv[0]);

    /* Setup locale/gettext */
    setlocale (LC_ALL, "");
    bindtextdomain (GETTEXT_PACKAGE, gsb_dirs_get_locale_dir ());
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);

#ifdef HAVE_GOFFICE
    /* initialisation libgoffice */
    libgoffice_init ();
    /* Initialize plugins manager */
    go_plugins_init (NULL, NULL, NULL, NULL, TRUE, GO_TYPE_PLUGIN_LOADER_MODULE);
#endif /* HAVE_GOFFICE */

    app = g_object_new (GRISBI_APP_TYPE,
                        "application-id",   "org.gtk.grisbi",
                        "flags",            G_APPLICATION_HANDLES_COMMAND_LINE | G_APPLICATION_NON_UNIQUE,
                        "register-session", TRUE,
                        NULL);

    /* option en attente : G_APPLICATION_HANDLES_OPEN,voir GApplicationFlags pour détails */

    status = g_application_run (G_APPLICATION (app), argc, argv);

    g_object_unref (app);

#ifdef HAVE_GOFFICE
    /* liberation libgoffice */
    libgoffice_shutdown ();
#endif /* HAVE_GOFFICE */

    return status;
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
