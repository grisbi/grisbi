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
#include "grisbi_window.h"
#include "grisbi_ui.h"
#include "gsb_dirs.h"
#include "menu.h"
/*END_INCLUDE*/


/*START_STATIC*/
static void grisbi_window_init_menus ( GrisbiWindow *window,
                        GtkWidget *vbox );
/*END_STATIC*/


#define GRISBI_WINDOW_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GRISBI_TYPE_WINDOW, GrisbiWindowPrivate))


struct _GrisbiWindowPrivate
{
    GtkVBox        *vbox;

    GtkWidget *menu_bar;
    GtkUIManager *ui_manager;
    GtkActionGroup *always_sensitive_action_group;
    GtkActionGroup *file_sensitive_action_group;
};


G_DEFINE_TYPE(GrisbiWindow, grisbi_window, GTK_TYPE_WINDOW)


static void grisbi_window_finalize ( GObject *object )
{
    GrisbiWindow *window;

    window = GRISBI_WINDOW ( object );

/*     if ( window->priv->default_location != NULL )
 *         g_object_unref (window->priv->default_location);
 */

    G_OBJECT_CLASS ( grisbi_window_parent_class )->finalize ( object );
}


static void grisbi_window_class_init ( GrisbiWindowClass *klass )
{
    GObjectClass *object_class = G_OBJECT_CLASS ( klass );
/*     GtkWidgetClass *widget_class = GTK_WIDGET_CLASS ( klass );  */

    object_class->finalize = grisbi_window_finalize;

    g_type_class_add_private ( object_class, sizeof ( GrisbiWindowPrivate ) );
}


static void grisbi_window_init ( GrisbiWindow *window )
{
    GtkWidget *main_box;

    window->priv = GRISBI_WINDOW_GET_PRIVATE ( window );

    main_box = gtk_vbox_new ( FALSE, 0 );
    gtk_container_add ( GTK_CONTAINER ( window ), main_box );
    gtk_widget_show ( main_box );

    /* create the menus */
    grisbi_window_init_menus ( window, main_box );

}


static void grisbi_window_init_menus ( GrisbiWindow *window,
                        GtkWidget *vbox )
{
    GtkUIManager *ui_manager;
    GtkActionGroup *actions;
    GError *error = NULL;
    gchar *ui_file;

    ui_manager = gtk_ui_manager_new ( );
    window->priv->ui_manager = ui_manager;

    actions = gtk_action_group_new ( "AlwaysSensitiveActions" );
    gtk_action_group_set_translation_domain ( actions, NULL );
    gtk_action_group_add_actions (actions,
                        always_sensitive_entries,
                        G_N_ELEMENTS ( always_sensitive_entries ),
                        window );

/*     gtk_action_group_add_radio_actions ( actions,
 *                         radio_entries,
 *                         G_N_ELEMENTS ( radio_entries ),
 *                         -1,
 *                         G_CALLBACK ( gsb_gui_toggle_line_view_mode ),
 *                         NULL );
 */

/*     gtk_action_group_add_toggle_actions ( actions,
 *                         toggle_entries,
 *                         G_N_ELEMENTS ( toggle_entries ),
 *                         NULL );
 */

    gtk_ui_manager_insert_action_group ( ui_manager, actions, 0 );
    g_object_unref ( actions );
    window->priv->always_sensitive_action_group = actions;

    actions = gtk_action_group_new ( "FileSensitiveActions" );
    gtk_action_group_set_translation_domain ( actions, NULL );
    gtk_action_group_add_actions (actions,
                        file_sensitive_entries,
                        G_N_ELEMENTS ( file_sensitive_entries ),
                        window );

    gtk_ui_manager_insert_action_group ( ui_manager, actions, 0 );
    g_object_unref ( actions );
    window->priv->file_sensitive_action_group = actions;
    gtk_action_group_set_sensitive ( actions, FALSE );

    /* now load the UI definition */
    ui_file = g_build_filename ( gsb_dirs_get_ui_dir ( ), "grisbi_ui.xml", NULL );
    gtk_ui_manager_add_ui_from_file ( ui_manager, ui_file, &error );
    if ( error != NULL )
    {
        g_warning ( "Could not merge %s: %s", ui_file, error->message );
        g_error_free ( error );
    }
    g_free ( ui_file );

#ifndef GTKOSXAPPLICATION
    gtk_window_add_accel_group ( GTK_WINDOW ( window ),
                        gtk_ui_manager_get_accel_group ( ui_manager ) );
#endif /* !GTKOSXAPPLICATION */

    window->priv->menu_bar = gtk_ui_manager_get_widget ( ui_manager, "/menubar" );
    gtk_box_pack_start ( GTK_BOX ( vbox ),  window->priv->menu_bar, FALSE, TRUE, 0 );

/*     gsb_gui_sensitive_menu_item ( "/menubar/EditMenu/NewTransaction", FALSE );
 *     gsb_menu_transaction_operations_set_sensitive ( FALSE );
 */
}


/**
 *
 *
 *
 *
 **/
GtkUIManager *grisbi_window_get_ui_manager ( GrisbiWindow *window )
{
    return window->priv->ui_manager;
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
