/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2008 Benjamin Drieu (bdrieu@april.org)	      */
/*			http://www.grisbi.org   			      */
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
 * \file gsb_calendar_entry.c
 * this is a new widget, an entry for containing a date
 * that entry controls the calendar, the check of the date...
 *
 * */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

#include <ctype.h>
#include <gdk/gdkkeysyms.h>

/*START_INCLUDE*/
#include "gsb_calendar_entry.h"
#include "utils_dates.h"
#include "gsb_form_widget.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean gsb_calendar_entry_button_press ( GtkWidget *entry,
					   GdkEventButton *event,
					   gpointer null );
static gboolean gsb_calendar_entry_calendar_key_press ( GtkCalendar *pCalendar,
						 GdkEventKey *ev,
						 GtkWidget *entry );
static gboolean gsb_calendar_entry_changed ( GtkWidget *entry,
				      gpointer null );
static gboolean gsb_calendar_entry_focus_out ( GtkWidget *entry,
					GdkEventFocus *event,
					gint *set_today );
static gboolean gsb_calendar_entry_key_press ( GtkWidget *entry,
					GdkEventKey  *ev,
					gpointer null );
static GtkWidget *gsb_calendar_entry_popup ( GtkWidget *entry );
static gboolean gsb_calendar_entry_select_date ( GtkCalendar *pCalendar,
					  GtkWidget *entry );
static void gsb_calendar_entry_step_date ( GtkWidget *entry,
				    gint movement );
/*END_STATIC*/



/*START_EXTERN*/
extern GdkColor calendar_entry_color;
extern GtkWidget *window;
/*END_EXTERN*/

#define ENTRY_NORMAL 0
#define ENTRY_RED 1


/**
 * create a new entry for contain a date
 * if double-click on that entry, popup a calendar
 * when leave the entry, check the date and complete it
 *
 * \param set_today TRUE if we want to set the current day if the entry is left empty
 *
 * \return a GtkEntry widget
 * */
GtkWidget *gsb_calendar_entry_new ( gint set_today )
{
    GtkWidget *entry;

    entry = gtk_entry_new ();
    gtk_widget_set_size_request ( entry,
			   100,
			   -1 );
    g_signal_connect ( G_OBJECT (entry),
		       "button-press-event",
		       G_CALLBACK (gsb_calendar_entry_button_press), NULL );
    g_signal_connect_after ( G_OBJECT (entry),
			     "focus-out-event",
			     G_CALLBACK (gsb_calendar_entry_focus_out), GINT_TO_POINTER (set_today));
    g_signal_connect ( G_OBJECT (entry),
		       "key-press-event",
		       G_CALLBACK (gsb_calendar_entry_key_press),
		       NULL );
    g_signal_connect ( G_OBJECT (entry),
		       "changed",
		       G_CALLBACK (gsb_calendar_entry_changed),
		       NULL );
    return entry;
}


/**
 * set the date in the date entry
 *
 * \param entry
 * \param date a GDate or NULL to set nothing
 *
 * \return FALSE if problem, TRUE if ok
 * */
gboolean gsb_calendar_entry_set_date ( GtkWidget *entry,
				       const GDate *date )
{
    gchar *string;

    if (!entry
	||
	!GTK_IS_ENTRY (entry))
        return FALSE;

    if (!date
	||
	!g_date_valid (date))
    {
        gtk_entry_set_text ( GTK_ENTRY (entry), "" );
        return FALSE;
    }

    string = gsb_format_gdate ( date );
    gtk_entry_set_text ( GTK_ENTRY ( entry ), string );
    g_free ( string );
    return TRUE;
}


/**
 * get the date in the date entry
 *
 * \param entry
 *
 * \return a newly allocated GDate or NULL if invalid date
 * */
GDate *gsb_calendar_entry_get_date ( GtkWidget *entry )
{
    GDate *date;

    if (!entry
	||
	!GTK_IS_ENTRY (entry)
	||
	!gsb_date_check_entry (entry))
	return NULL;

    date = gsb_date_get_last_entry_date (gtk_entry_get_text (GTK_ENTRY (entry)));
    return date;
}



/**
 * manual set of the color of the entry
 * used for example in reconciliation, if cancel it, to set back the good color
 * of the entry
 *
 * \param entry
 * \param normal_color TRUE if we want to set the normal color, FALSE to set to red
 *
 * \return FALSE
 * */
gboolean gsb_calendar_entry_set_color ( GtkWidget *entry,
					gboolean normal_color )
{
    if (!entry)
	return FALSE;

    if (normal_color)
    {
	gtk_widget_modify_base ( entry, 
				 GTK_STATE_NORMAL,
				 NULL );
    }
    else
    {
	gtk_widget_modify_base ( entry, 
				 GTK_STATE_NORMAL,
				 &calendar_entry_color );
    }

    return FALSE;
}



/**
 * callback called when press button on the date entry
 * used to popup a calendar if double click
 *
 * \param entry the entry wich receive the signal
 * \param event
 * \param null not used
 *
 * \return FALSE
 * */
gboolean gsb_calendar_entry_button_press ( GtkWidget *entry,
					   GdkEventButton *event,
					   gpointer null )
{
    if ( event -> type == GDK_2BUTTON_PRESS )
	gsb_calendar_entry_popup (entry);

    return FALSE;
}


/**
 * callback called when press a key on the date entry
 * used to increase/decrease the date and popup the calendar
 *
 * \param entry the entry wich receive the signal
 * \param event
 * \param null not used
 *
 * \return FALSE
 * */
gboolean gsb_calendar_entry_key_press ( GtkWidget *entry,
					GdkEventKey  *ev,
					gpointer null )
{
    switch ( ev -> keyval )
    {
	case GDK_KP_Enter :
	case GDK_Return :

	    /* CONTROL ENTER opens the calendar */
	    if ( ( ev -> state & GDK_CONTROL_MASK ) == GDK_CONTROL_MASK)
		gsb_calendar_entry_popup (entry);
	    break;

	case GDK_KP_Add:
	case GDK_plus:
	case GDK_equal:		/* This should make all our US users happy */

	    /* increase the date of 1 day/week */
	    if ( ( ev -> state & GDK_CONTROL_MASK ) != GDK_CONTROL_MASK ||
		 ev -> keyval != GDK_KP_Add )
		gsb_calendar_entry_step_date ( entry, ONE_DAY );
	    else
		gsb_calendar_entry_step_date ( entry, ONE_WEEK );
	    return TRUE;
	    break;

	case GDK_KP_Subtract:
	case GDK_minus:

	    /* decrease the date of 1 day/week, or the check of 1 */
	    if ( ( ev -> state & GDK_CONTROL_MASK ) != GDK_CONTROL_MASK ||
		 ev -> keyval != GDK_KP_Subtract  )
		gsb_calendar_entry_step_date ( entry, -ONE_DAY );
	    else
		gsb_calendar_entry_step_date ( entry, -ONE_WEEK );
	    return TRUE;
	    break;

	case GDK_Page_Up :
	case GDK_KP_Page_Up :

	    /* increase the date of 1 month/year */
	    if ( ( ev -> state & GDK_CONTROL_MASK ) != GDK_CONTROL_MASK )
		gsb_calendar_entry_step_date ( entry,
			       ONE_MONTH );
	    else
		gsb_calendar_entry_step_date ( entry,
			       ONE_YEAR );

	    return TRUE;
	    break;

	case GDK_Page_Down :
	case GDK_KP_Page_Down :

	    /* decrease the date of 1 month/year */
	    if ( ( ev -> state & GDK_CONTROL_MASK ) != GDK_CONTROL_MASK )
		gsb_calendar_entry_step_date ( entry,
			       -ONE_MONTH );
	    else
		gsb_calendar_entry_step_date ( entry,
			       -ONE_YEAR );

	    return TRUE;
	    break;
    }
    return FALSE;
}


/**
 * increase or decrease the date in the entry date
 *
 * \param entry
 * \param movement + or - ONE_DAY, ONE_WEEK, ONE_MONTH, ONE_YEAR
 *
 * \return
 * */
void gsb_calendar_entry_step_date ( GtkWidget *entry,
				    gint movement )
{
    GDate *date;
    gchar *string;

    /* on commence par vérifier que la date est valide */

    if ( !gsb_date_check_and_complete_entry ( entry, TRUE ) )
	return;

    date = gsb_date_get_last_entry_date ( gtk_entry_get_text ( GTK_ENTRY ( entry )));

    switch ( movement )
    {
	case ONE_DAY :
	case ONE_WEEK :

	    g_date_add_days ( date, movement ) ;
	    break ;

	case -ONE_DAY :
	case -ONE_WEEK :

	    g_date_subtract_days ( date, -movement ) ;
	    break ;

	case ONE_MONTH :

	    g_date_add_months ( date, 1 ) ;
	    break ;

	case -ONE_MONTH :

	    g_date_subtract_months ( date, 1 ) ;
	    break ;

	case ONE_YEAR :

	    g_date_add_years ( date, 1 ) ;
	    break ;

	case -ONE_YEAR :

	    g_date_subtract_years ( date, 1 ) ;
	    break ;

	default :
	    break ;
    }

    string = gsb_format_gdate (date);
    gtk_entry_set_text ( GTK_ENTRY ( entry ), string );
    g_date_free (date);
    g_free (string);
}


/**
 * callback called on focus-out on the date entry
 * complete and check the date
 *
 * \param entry the entry wich receive the signal
 * \param event
 * \param set_today TRUE (but pointer) if we want to set the current day if the entry is left empty
 *
 * \return FALSE
 * */
gboolean gsb_calendar_entry_focus_out ( GtkWidget *entry,
					GdkEventFocus *event,
					gint *set_today )
{
    gint valid;

    valid = gsb_date_check_and_complete_entry (entry, GPOINTER_TO_INT (set_today));
    gsb_calendar_entry_set_color ( entry, valid ) ;    

    return FALSE;
}

/**
 * called when date changed
 * check the date and set the entry red/invalid if not a good date
 *
 * \param entry
 * \param null
 *
 * \return FALSE
 * */
gboolean gsb_calendar_entry_changed ( GtkWidget *entry,
				      gpointer null )
{
    GDate *date;

    /* if we are in the form and the entry is empty, do nothing
     * because it's a special style too */
    if (gsb_form_widget_check_empty (entry))
	return FALSE;

    /* if nothing in the entry, keep the normal style */
    if (!strlen (gtk_entry_get_text (GTK_ENTRY (entry))))
    {
	gsb_calendar_entry_set_color ( entry, TRUE );
	return FALSE;
    }

    /* to check the date, we just try to see if can have a dote from the entry */
    date = gsb_date_get_last_entry_date (gtk_entry_get_text (GTK_ENTRY (entry)));

    if (date)
    {
	/* the date is valid, make it normal */
	gsb_calendar_entry_set_color ( entry, TRUE );
	g_date_free (date);
    }
    else
    {
	/* the date is not valid, make it red */
	gsb_calendar_entry_set_color ( entry, FALSE );
    }

    return FALSE;
}


/** popup a calendar next to the entry
 *
 * \param entry the date entry
 *
 * \return a GtkWindow wich contains the calendar
 * */
GtkWidget *gsb_calendar_entry_popup ( GtkWidget *entry )
{
    GtkWidget *popup, *pVBox, *pCalendar, *button, *frame;
    GtkRequisition *popup_size;
    gint x, y;
    gint screen_width = gdk_screen_width ( );
    GDate * date;

    /* make the popup */
    popup = gtk_window_new ( GTK_WINDOW_TOPLEVEL );
    gtk_window_set_modal ( GTK_WINDOW ( popup ), TRUE );
    gtk_window_set_transient_for ( GTK_WINDOW ( popup ),
                        GTK_WINDOW ( window ) );
    gtk_window_set_decorated ( GTK_WINDOW ( popup ), FALSE );
    g_signal_connect_swapped ( G_OBJECT ( popup ),
				"destroy",
				G_CALLBACK ( gdk_pointer_ungrab ),
				GDK_CURRENT_TIME );

    /* set the decoration */
    frame = gtk_frame_new ( NULL );
    gtk_container_add ( GTK_CONTAINER ( popup ), frame );
    gtk_widget_show ( frame );

    pVBox = gtk_vbox_new ( FALSE, 5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( pVBox ), 5 );
    gtk_container_add ( GTK_CONTAINER ( frame ), pVBox );
    gtk_widget_show ( pVBox );

    /* get the date */
    date = gsb_calendar_entry_get_date (entry);
    if (!date)
	date = gdate_today ();

    /* set the calendar */
    pCalendar = gtk_calendar_new();
    gtk_calendar_select_month ( GTK_CALENDAR ( pCalendar ), 
				g_date_get_month ( date ) - 1, 
				g_date_get_year ( date ) );
    gtk_calendar_select_day ( GTK_CALENDAR ( pCalendar ), g_date_get_day ( date ) );

    g_signal_connect ( G_OBJECT ( pCalendar ),
		       "day_selected_double_click",
		       G_CALLBACK ( gsb_calendar_entry_select_date ),
		       entry );
    g_signal_connect ( G_OBJECT ( pCalendar ),
		       "key-press-event",
		       G_CALLBACK ( gsb_calendar_entry_calendar_key_press ),
		       entry );
    gtk_box_pack_start ( GTK_BOX ( pVBox ),
			 pCalendar,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( pCalendar );

    /* cancel button */
    button = gtk_button_new_with_label ( _("Cancel") );
    g_signal_connect_swapped ( G_OBJECT ( button ),
			       "clicked",
			       G_CALLBACK ( gtk_widget_destroy ),
			       G_OBJECT ( popup ));
    gtk_box_pack_start ( GTK_BOX ( pVBox ),
			 button,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( button );

    /* set the position */
    gdk_window_get_origin ( GTK_WIDGET ( entry ) -> window,
			    &x,
			    &y );

    /* on récupère la taille de la popup */
    popup_size = g_malloc0 ( sizeof ( GtkRequisition ));
    gtk_widget_size_request ( GTK_WIDGET ( popup ), popup_size );

    /* pour la soustraire à la position de l'entrée date */
    y -= popup_size -> height;

    /* on décale le popup si on est trop près de bord droit de l'écran */
    if ( x > ( screen_width - popup_size -> width ) )
        x = screen_width - popup_size -> width - 10;

    /* si une des coordonnées est négative, alors la fonction
       gtk_window_move échoue et affiche la popup en 0,0 */
    if ( x < 0 )
	x = 0 ;

    if ( y < 0 )
	y = 0 ;

    gtk_window_move ( GTK_WINDOW ( popup ), x, y );
    gtk_widget_show ( popup );
    gtk_widget_grab_focus ( GTK_WIDGET ( pCalendar ) );
    return ( popup );
}


/**
 * called with a double-click on a date on a calendar
 * set the choosen date in the entry
 *
 * \param pCalendar
 * \param entry
 *
 * \return FALSE
 * */
gboolean gsb_calendar_entry_select_date ( GtkCalendar *pCalendar,
					  GtkWidget *entry )
{
    guint year, month, day;
    GtkWidget *pTopLevelWidget;

    /* get the popup to destroy it if we are in a popup */
    pTopLevelWidget = gtk_widget_get_toplevel ( GTK_WIDGET ( pCalendar ) );

    gtk_calendar_get_date ( pCalendar, &year, &month, &day);

    gtk_entry_set_text ( GTK_ENTRY ( entry ),
			 gsb_format_date ( day, month + 1, year ));
    gsb_form_widget_set_empty ( entry, FALSE );
    if ( GTK_WIDGET_TOPLEVEL ( pTopLevelWidget ) )
        gtk_widget_destroy ( pTopLevelWidget );

    return FALSE;
}


/**
 * called when a calendar receive a key-press-event
 *
 * \param pCalendar
 * \param ev
 * \param entry
 *
 * \return TRUE to block the signal
 * */
gboolean gsb_calendar_entry_calendar_key_press ( GtkCalendar *pCalendar,
						 GdkEventKey *ev,
						 GtkWidget *entry )
{
    guint day, month, year;
    GDate *date;
    GtkWidget *pTopLevelWidget;

    /* get the popup to destroy it if need */
    pTopLevelWidget = gtk_widget_get_toplevel ( GTK_WIDGET ( pCalendar ) );

    /* most of the time, we will use date so can get it here,
     * think about free it if not used */
    gtk_calendar_get_date ( pCalendar, &year, &month, &day );
    month++;
    date = g_date_new_dmy (day, month, year);

    switch ( ev -> keyval )
    {
	case GDK_Escape :
	    /* just close the calendar if it's a popup */
	    if ( GTK_WIDGET_TOPLEVEL ( pTopLevelWidget ) )
		gtk_widget_destroy ( pTopLevelWidget );
	    g_date_free (date);
	    return TRUE;
	    break ;

	case GDK_Return :
	case GDK_KP_Enter :
	    /* get the date an close the calendar */
	    gtk_entry_set_text ( GTK_ENTRY ( entry ),
				 gsb_format_date ( day, month, year ));
	    if ( GTK_WIDGET_TOPLEVEL ( pTopLevelWidget ) )
		gtk_widget_destroy ( pTopLevelWidget );
	    g_date_free (date);
	    return TRUE;
	    break ;

	    /* from now, it will change date so just use date, modify it and fill day, month, year
	     * we will set the calendar at the end of that function
	     * so after now, only keys which change the date */
	case GDK_Left :
	case GDK_KP_Left:
	case GDK_minus:
	case GDK_KP_Subtract:
	    /* day before */
	    g_date_subtract_days (date, 1);
	    break ;

	case GDK_Right :
	case GDK_KP_Right:
	case GDK_plus:
	case GDK_KP_Add:
	    /* day after */
	    g_date_add_days (date, 1);
	    break ;

	case GDK_Up :
	case GDK_KP_Up :
	    /* prev week */
	    g_date_subtract_days (date, 7);
	    break ;

	case GDK_Down :
	case GDK_KP_Down :
	    /* next week */
	    g_date_add_days (date, 7);
	    break ;

	case GDK_Home :
	case GDK_KP_Home :
	    /* go to first day of the month */
	    g_date_set_day (date, 1);
	    break ;

	case GDK_End :
	case GDK_KP_End :
	    /* go to last day of the month */
	    g_date_set_day (date,
			    g_date_get_days_in_month (month, year));
	    break ;

	case GDK_Page_Up :
	case GDK_KP_Page_Up :
	    /* prev month */
	    g_date_subtract_months (date, 1);
	    break ;

	case GDK_Page_Down :
	case GDK_KP_Page_Down :
	    /* next month */
	    g_date_add_months (date, 1);
	    break ;

	default:
	    return TRUE;
    }

    day = g_date_get_day (date);
    month = g_date_get_month (date);
    year = g_date_get_year (date);
    g_date_free (date);

    /* to avoid a warning */
    gtk_calendar_select_day( pCalendar , 15 );

    month--;
    gtk_calendar_select_month ( pCalendar , month, year );
    gtk_calendar_select_day( pCalendar , day );
    return TRUE;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
