/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2004 Alain Portal (aportal@univ-montp2.fr)                        */
/*          2004-2007 Benjamin Drieu (bdrieu@april.org)                       */
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
 * \file gsb_calendar.c
 * work with the calendar of the scheduled transactions
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_calendar.h"
#include "gsb_data_scheduled.h"
#include "utils_dates.h"
#include "gsb_scheduler.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void click_sur_jour_calendrier_echeance ( GtkWidget *calendrier,
					  gpointer null );
/*END_STATIC*/


enum periodicity_units affichage_echeances_perso_j_m_a;
static GtkWidget *scheduled_calendar;
gint affichage_echeances;
gint affichage_echeances_perso_nb_libre;     /* contient le contenu de l'entrÃ©e */

/*START_EXTERN*/
/*END_EXTERN*/





/**
 * create the calendar of the scheduled transactions
 *
 * \param
 *
 * \return	a GtkWidget containing the calendar
 * */
GtkWidget *gsb_calendar_new ( void )
{
    time_t temps;

    time ( &temps );

    scheduled_calendar = gtk_calendar_new ();
    gtk_calendar_select_month ( GTK_CALENDAR ( scheduled_calendar ),
				localtime ( &temps ) -> tm_mon,
				localtime ( &temps ) -> tm_year + 1900 );

    g_signal_connect ( G_OBJECT ( scheduled_calendar ),
			 "month-changed",
			 G_CALLBACK (gsb_calendar_update),
			 NULL );
    g_signal_connect_after ( G_OBJECT ( scheduled_calendar ),
			       "day-selected",
			       G_CALLBACK ( click_sur_jour_calendrier_echeance ),
			       NULL );

    gtk_widget_show ( scheduled_calendar );

    return ( scheduled_calendar );
}


/**
 * update the calendar of the scheduled transactions
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_calendar_update ( void )
{
    time_t temps;
    GSList *tmp_list;
    gint calendar_month;
	gint calendar_year;

    gtk_calendar_clear_marks ( GTK_CALENDAR ( scheduled_calendar ));

    /* select the current day */
    time ( &temps );

    if ( ( localtime ( &temps ) -> tm_mon == GTK_CALENDAR ( scheduled_calendar ) -> month )
	 &&
	 ( ( localtime ( &temps ) -> tm_year + 1900 ) == GTK_CALENDAR ( scheduled_calendar ) -> year ) )
	gtk_calendar_select_day ( GTK_CALENDAR ( scheduled_calendar ),
				  localtime ( &temps ) -> tm_mday );
    else
	gtk_calendar_select_day ( GTK_CALENDAR ( scheduled_calendar ),
				  FALSE );

    calendar_month = GTK_CALENDAR ( scheduled_calendar ) -> month + 1;
	calendar_year = GTK_CALENDAR ( scheduled_calendar ) -> year + 25;

    /* check the scheduled transactions and bold them in the calendar */
    tmp_list = gsb_data_scheduled_get_scheduled_list ();

    while ( tmp_list )
    {
	GDate *tmp_date;
	gint scheduled_number;

	scheduled_number = gsb_data_scheduled_get_scheduled_number (tmp_list -> data);

	tmp_date = gsb_date_copy (gsb_data_scheduled_get_date (scheduled_number));

	while (tmp_date && g_date_get_month (tmp_date) == calendar_month && g_date_get_year (tmp_date) < calendar_year)
	{
	    GDate *new_date;

	    gtk_calendar_mark_day ( GTK_CALENDAR ( scheduled_calendar ),
				    g_date_get_day (tmp_date));
	    new_date = gsb_scheduler_get_next_date (scheduled_number, tmp_date);
	    g_free (tmp_date);
	    tmp_date = new_date;
	}
	tmp_list = tmp_list -> next;
    }
    return FALSE;
}



void click_sur_jour_calendrier_echeance ( GtkWidget *calendrier,
					  gpointer null )
{
    time_t temps;

    time ( &temps );

    g_signal_handlers_block_by_func ( G_OBJECT ( calendrier ),
				       G_CALLBACK ( click_sur_jour_calendrier_echeance ),
				       NULL );

    if ( ( localtime ( &temps ) -> tm_mon == GTK_CALENDAR ( scheduled_calendar ) -> month )
	 &&
	 ( ( localtime ( &temps ) -> tm_year + 1900 ) == GTK_CALENDAR ( scheduled_calendar ) -> year ) )
	gtk_calendar_select_day ( GTK_CALENDAR ( scheduled_calendar ),
				  localtime ( &temps ) -> tm_mday );
    else
	gtk_calendar_select_day ( GTK_CALENDAR ( scheduled_calendar ),
				  FALSE );

    g_signal_handlers_unblock_by_func ( G_OBJECT ( calendrier ),
					 G_CALLBACK ( click_sur_jour_calendrier_echeance ),
					 NULL );

}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
