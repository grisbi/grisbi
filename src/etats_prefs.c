/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)           */
/*          2008-2012 Pierre Biava (grisbi@pierre.biava.name)                 */
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

#include <glib/gi18n.h>
#include <gtk/gtk.h>


/*START_INCLUDE*/
#include "etats_prefs.h"
#include "etats_config.h"
#include "structures.h"
#include "utils.h"
#include "utils_buttons.h"
#include "utils_gtkbuilder.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/


/*START_STATIC*/
/*END_STATIC*/


#define ETATS_PREFS_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), ETATS_TYPE_PREFS, EtatsPrefsPrivate))

/* builder */
static GtkBuilder *etats_prefs_builder = NULL;


struct _EtatsPrefsPrivate
{
    GtkWidget           *hpaned;
};


G_DEFINE_TYPE(EtatsPrefs, etats_prefs, GTK_TYPE_DIALOG)

/**
 *  called when destroy EtatsPrefs
 *
 * \param object
 *
 * \return
 */
static void etats_prefs_dispose ( GObject *object )
{
    devel_debug (NULL);

    /* libération de l'objet prefs */
    G_OBJECT_CLASS ( etats_prefs_parent_class )->dispose ( object );
}


/**
 * finalise EtatsPrefs
 *
 * \param object
 *
 * \return
 */
static void etats_prefs_finalize ( GObject *object )
{
    devel_debug (NULL);
/*    etats_prefs_dialog = NULL;
*/
    /* libération de l'objet prefs */
    G_OBJECT_CLASS ( etats_prefs_parent_class )->finalize ( object );
}


/**
 * Initialise EtatsPrefsClass
 *
 * \param
 *
 * \return
 */
static void etats_prefs_class_init ( EtatsPrefsClass *klass )
{
    GObjectClass *object_class = G_OBJECT_CLASS ( klass );

    object_class->dispose = etats_prefs_dispose;
    object_class->finalize = etats_prefs_finalize;

    g_type_class_add_private ( object_class, sizeof( EtatsPrefsPrivate ) );
}


/* CREATE OBJECT */
/**
 * Initialise EtatsPrefs
 *
 * \param prefs
 *
 * \return
 */
static void etats_prefs_init ( EtatsPrefs *prefs )
{
    GtkWidget *dialog = NULL;
    GtkWidget *tree_view;
    prefs->priv = ETATS_PREFS_GET_PRIVATE ( prefs );

    devel_debug (NULL);

    /* Creation d'un nouveau GtkBuilder */
    etats_prefs_builder = gtk_builder_new ( );

    if ( etats_prefs_builder == NULL )
        return;

    /* Chargement du XML dans etats_config_builder */
    if ( !utils_gtkbuilder_merge_ui_data_in_builder ( etats_prefs_builder, "etats_config.ui" ) )
        return;

    /* Recuparation d'un pointeur sur la fenetre. */
    dialog = GTK_WIDGET ( gtk_builder_get_object ( etats_config_builder, "config_etats_dialog" ) );
    gtk_window_set_transient_for ( GTK_WINDOW ( dialog ),
                        GTK_WINDOW ( grisbi_app_get_active_window ( NULL ) ) );
}


GtkWidget *etats_prefs_new ( GtkWidget *parent )
{
    GtkWidget *etats_prefs_dialog = NULL;

    etats_prefs_dialog = g_object_new ( ETATS_TYPE_PREFS, NULL );

    return etats_prefs_dialog;
}


/**
 *
 *
 * \param
 *
 * \return TRUE
 * */
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
