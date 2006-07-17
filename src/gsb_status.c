/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2005-2006 Benjamin Drieu (bdrieu@april.org)	      */
/* 			http://www.grisbi.org				      */
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

#include "include.h"

/*START_INCLUDE*/
#include "gsb_status.h"
#include "main.h"
#include "utils.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean gsb_status_pulse (  );
static void gsb_status_remove_progress (  );
static void gsb_status_set_progress ( gdouble ratio, gdouble max );
static void gsb_status_show_progress ();
static void gsb_status_start_activity (  );
static void gsb_status_stop_activity (  );
/*END_STATIC*/

/*START_EXTERN*/
extern gint max;
extern GtkWidget *window;
/*END_EXTERN*/

/** Status bar displayed in the bottom of Grisbi window.  */
GtkWidget *main_statusbar = NULL;

/** Context ID from the GtkStatusBar. */
guint context_id;

/** Message ID from the GtkStatusBar.  */
guint message_id = -1;

/** Optional progress bar in main status bar.  */
GtkWidget * progress_bar = NULL;

/** Timer ID of the timeout responsible for updating the
 * GtkProgressBar.  */
int timer_id;

/** Window under cursor at the time the cursor animation changed. */
GdkWindow * tracked_window;


/**
 * Create and return a new GtkStatusBar to hold various status
 * informations.
 *
 * \return	A newly allocated GtkStatusBar.
 */
GtkWidget * gsb_new_statusbar ()
{
    main_statusbar = gtk_statusbar_new ();
    context_id = gtk_statusbar_get_context_id ( GTK_STATUSBAR (main_statusbar), "Grisbi" );
    message_id = -1;

    gtk_widget_show_all ( main_statusbar );

    return main_statusbar;
}



/**
 * Display a message in the status bar.
 *
 * \param message	Message to display.
 */
void gsb_status_message ( gchar * message )
{
    if ( ! main_statusbar || ! GTK_IS_STATUSBAR ( main_statusbar ) )
	return;

    gsb_status_clear ();
    message_id = gtk_statusbar_push ( GTK_STATUSBAR (main_statusbar), context_id, message );

    /** Call gtk_main_iteration() to ensure status message is
     * displayed.  This is done because we need to display it
     * immediately without waiting to return to gtk_main().  */
    while ( gtk_events_pending () ) gtk_main_iteration ( );
}



/**
 * Clear any message in the status bar.
 */
void gsb_status_clear (  )
{
    if ( ! main_statusbar || ! GTK_IS_STATUSBAR ( main_statusbar ) )
	return;

    if ( message_id >= 0 )
    {
	gtk_statusbar_remove ( GTK_STATUSBAR (main_statusbar), context_id, message_id );
	message_id = -1;
    }

    /** Call gtk_main_iteration() to ensure status message is
     * displayed.  This is done because we need to display it
     * immediately without waiting to return to gtk_main().  */
    while ( gtk_events_pending () ) gtk_main_iteration ( );
}



/**
 * Display a progress bar in the status bar.  Does not set any value.
 */
void gsb_status_show_progress ()
{
    if ( progress_bar )
    {
	return;
    }

    progress_bar = gtk_progress_bar_new();
    gtk_box_pack_start ( GTK_BOX ( main_statusbar ), progress_bar, FALSE, FALSE, 6 );
    gtk_widget_show ( progress_bar );
}



/**
 * Set value of the progress bar in status bar.  If no progress bar
 * has been shown precedently (via gsb_status_show_progress()), do
 * nothing.
 *
 * \param ratio		Fraction value to set.
 * \param max		Maximum possible value.
 * 
 * To set the progress bar to half of its size, simply use something
 * like gsb_status_set_progress ( 0.5, 1.0 ).
 */
void gsb_status_set_progress ( gdouble ratio, gdouble max )
{
    if ( ! progress_bar )
    {
	return;
    }

    gtk_progress_bar_set_fraction ( GTK_PROGRESS_BAR (progress_bar), ratio / max );
}



/**
 * Remove progress bar from status bar.
 */
void gsb_status_remove_progress (  )
{
    if ( ! progress_bar )
    {
	return;
    }

    gtk_widget_destroy ( progress_bar );
    progress_bar = NULL;
}



/**
 * Increment the status bar.  This should be called as a timeout
 * function.
 *
 * \return TRUE
 */
gboolean gsb_status_pulse (  )
{
    if ( ! progress_bar )
    {
	return TRUE;
    }

    gtk_progress_bar_pulse ( GTK_PROGRESS_BAR (progress_bar));
    while ( gtk_events_pending () ) gtk_main_iteration ( );

    /* As this is a timeout function, return TRUE so that it
     * continues to get called */
    return TRUE;
}



/**
 * Display the progress bar and add a timeout function to update it
 * continuously.
 */
void gsb_status_start_activity (  )
{
    gsb_status_show_progress ();
    timer_id = g_timeout_add ( 100, gsb_status_pulse, NULL );
}



/**
 * Remove the progress bar and associated timeout.
 */
void gsb_status_stop_activity (  )
{
    g_source_remove ( timer_id );
    gsb_status_remove_progress ();
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
    GdkWindow * current_window;

    gdk_window_set_cursor ( window -> window, 
			    gdk_cursor_new_for_display ( gdk_display_get_default ( ), 
							 GDK_WATCH ) );

    current_window = gdk_display_get_window_at_pointer ( gdk_display_get_default ( ),
							 NULL, NULL );

    if ( current_window && GDK_IS_WINDOW ( current_window ) &&
	 current_window != window -> window )
    {
	GdkWindow * parent = gdk_window_get_toplevel ( current_window );
	
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
    {
	update_ecran ();
    }
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
    gdk_window_set_cursor ( window -> window, NULL );

    if ( tracked_window && gdk_window_is_visible ( tracked_window ) )
    {
	gdk_window_set_cursor ( tracked_window, NULL );
	tracked_window = NULL;
    }

    if ( force_update )
    {
	update_ecran ();
    }
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
