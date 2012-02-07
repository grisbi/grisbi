/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2005-2008 Benjamin Drieu (bdrieu@april.org)           */
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


/**
 * \file gsb_status.c
 * Various routines that implement the status bar.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_status.h"
#include "grisbi_app.h"
#include "grisbi_window.h"
#include "main.h"
#include "structures.h"
#include "utils.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/** Window under cursor at the time the cursor animation changed. */
static GdkWindow *tracked_window;


/**
 * Display a message in the status bar.
 *
 * \param message   Message to display.
 */
void gsb_status_message ( gchar *message )
{
    GrisbiApp *app;
    GrisbiWindow *window;

    app = grisbi_app_get_default ( );
    window = grisbi_app_get_active_window ( app );

    gsb_status_clear ();
    grisbi_window_statusbar_push ( window, message );

    /* force status message to be displayed NOW */
    update_gui ( );
}



/**
 * Clear any message in the status bar.
 */
void gsb_status_clear (  )
{
    GrisbiApp *app;
    GrisbiWindow *window;

    app = grisbi_app_get_default ( );
    window = grisbi_app_get_active_window ( app );
    grisbi_window_statusbar_remove ( window );

    /* force status message to be displayed NOW */
    update_gui ( );
}


/**
 * Change current cursor to a animated watch (if animation supported
 * by environment).
 * 
 * \param force_update		Call a gtk iteration to ensure cursor
 *				is updated.  May cause trouble if
 *				called from some signal handlers.
 */
void gsb_status_wait ( gboolean force_update )
{
    GdkWindow *current_window;
    GdkWindow *run_window;

    run_window = gtk_widget_get_window ( run.window );

    gdk_window_set_cursor ( run_window,
                        gdk_cursor_new_for_display ( gdk_display_get_default ( ),
                        GDK_WATCH ) );

    current_window = gdk_display_get_window_at_pointer ( gdk_display_get_default ( ), NULL, NULL );

    if ( current_window && GDK_IS_WINDOW ( current_window )
     &&
     current_window != run_window )
    {
        GdkWindow *parent = gdk_window_get_toplevel ( current_window );
        
        if ( parent && parent != current_window )
        {
            current_window = parent;
        }

        gdk_window_set_cursor ( current_window,
                        gdk_cursor_new_for_display ( gdk_display_get_default ( ),
                        GDK_WATCH ) );

        tracked_window = current_window;
    }

    if ( force_update )
        update_gui ( );
}


/**
 * Change current cursor to default cursor.
 * 
 * \param force_update		Call a gtk iteration to ensure cursor
 *				is updated.  May cause trouble if
 *				called from some signal handlers.
 */
void gsb_status_stop_wait ( gboolean force_update )
{
    if ( !run.window )
        return;

    gdk_window_set_cursor ( gtk_widget_get_window ( run.window ), NULL );

    if ( tracked_window && gdk_window_is_visible ( tracked_window ) )
    {
        gdk_window_set_cursor ( tracked_window, NULL );
        tracked_window = NULL;
    }

    if ( force_update )
        update_gui ( );
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
