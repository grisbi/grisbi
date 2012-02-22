/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2001-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*          2009-2012 Pierre Biava (grisbi@pierre.biava.name)                 */
/*          http://www.grisbi.org                                             */
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

#include <string.h>
#include <unistd.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
#include "erreur.h"
/*END_INCLUDE*/


/*START_STATIC*/
/*END_STATIC*/


#define GRISBI_PREFS_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GRISBI_TYPE_PREFS, GrisbiPrefsPrivate))

static GtkBuilder *grisbi_prefs_builder = NULL;

struct _GrisbiPrefsPrivate
{
    GList               *windows;
};


G_DEFINE_TYPE(GrisbiPrefs, grisbi_prefs, G_TYPE_OBJECT)

/**
 * finalise GrisbiPrefs
 *
 * \param object
 *
 * \return
 */
static void grisbi_prefs_finalize ( GObject *object )
{
/*     GrisbiPrefs *prefs = GRISBI_PREFS ( object );  */

    devel_debug (NULL);

    /* libération de l'objet prefs */
    G_OBJECT_CLASS ( grisbi_prefs_parent_class )->finalize ( object );
}


/**
 * Initialise GrisbiPrefsClass
 *
 * \param
 *
 * \return
 */
static void grisbi_prefs_class_init ( GrisbiPrefsClass *klass )
{
    GObjectClass *object_class = G_OBJECT_CLASS ( klass );

    object_class->finalize = grisbi_prefs_finalize;

    g_type_class_add_private ( object_class, sizeof( GrisbiPrefsPrivate ) );
}


/**
 * Initialise GrisbiPrefs
 *
 * \param prefs
 *
 * \return
 */
static void grisbi_prefs_init ( GrisbiPrefs *prefs )
{
/*     gchar *string;  */

    prefs->priv = GRISBI_PREFS_GET_PRIVATE ( prefs );


    /* return */
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

