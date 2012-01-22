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
/*END_INCLUDE*/


#define GRISBI_WINDOW_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GRISBI_TYPE_WINDOW, GrisbiWindowPrivate))


struct _GrisbiWindowPrivate
{
    GtkBox        *vbox;

};


G_DEFINE_TYPE(GrisbiWindow, grisbi_window, GTK_TYPE_WINDOW)


static void grisbi_window_class_init ( GrisbiWindowClass *klass )
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);


    g_type_class_add_private ( object_class, sizeof ( GrisbiWindowPrivate ) );
}


static void grisbi_window_init ( GrisbiWindow *window )
{
    GtkWidget *main_box;

    window->priv = GRISBI_WINDOW_GET_PRIVATE ( window );

    main_box = gtk_vbox_new ( FALSE, 0 );
    gtk_container_add ( GTK_CONTAINER ( window ), main_box );
    gtk_widget_show ( main_box );
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
