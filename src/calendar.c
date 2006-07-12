/* ************************************************************************** */
/* Ce fichier s'occupe de la gestion des calendriers                          */
/* 			gsbcalendar.c                                         */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2004 Alain Portal (aportal@univ-montp2.fr) 	      */
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

#include <ctype.h>
#include "include.h"
#include <gdk/gdkkeysyms.h>

/*START_INCLUDE*/
#include "calendar.h"
#include "utils_dates.h"
#include "calendar.h"
#include "include.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/



/*START_EXTERN*/
extern GtkWidget *window;
/*END_EXTERN*/




/******************************************************************************/
/* Crée une fenètre (popup) calendrier.                                       */
/* Cette fenètre est « rattachée » au widget (une entrée de texte) que l'on   */
/* passe en paramètre et qui récupèrera une date, à moins que l'on abandonne  */
/* la saisie de la date par un appui sur la touche ESC lorsque le calendrier  */
/* est ouvert.                                                                */
/******************************************************************************/
GtkWidget *gsb_calendar_new ( GtkWidget *entry )
{
    GtkWidget *popup, *pVBox, *pCalendar, *bouton, *frame;
    GtkRequisition *taille_popup;
    gint x, y;
    GDate * date;

    /* création de la popup */

    popup = gtk_window_new ( GTK_WINDOW_POPUP );
    gtk_window_set_modal ( GTK_WINDOW ( popup ), TRUE );
    gtk_signal_connect_object ( GTK_OBJECT ( popup ),
				"destroy",
				GTK_SIGNAL_FUNC ( gdk_pointer_ungrab ),
				GDK_CURRENT_TIME );
/*     gtk_signal_connect_object ( GTK_OBJECT ( popup ), */
/* 				"destroy", */
/* 				GTK_SIGNAL_FUNC ( gtk_grab_remove ), */
/* 				GTK_OBJECT ( entree )); */

  /* création de l'intérieur de la popup */

    frame = gtk_frame_new ( NULL );
    gtk_container_add ( GTK_CONTAINER ( popup ), frame );
    gtk_widget_show ( frame );

    pVBox = gtk_vbox_new ( FALSE, 5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( pVBox ), 5 );
    gtk_container_add ( GTK_CONTAINER ( frame ), pVBox );
    gtk_widget_show ( pVBox );

    if ( strlen ( g_strstrip ( ( gchar * ) gtk_entry_get_text ( GTK_ENTRY ( entry )))) )
	date = gsb_parse_date_string ( ( gchar * ) gtk_entry_get_text ( GTK_ENTRY ( entry ) ) );
    else
	date = gdate_today ();

    pCalendar = gtk_calendar_new();

    /* parce qu'il y a une différence de type et un décalage de valeur
       pour identifier le month :
       - pour g_date, janvier = 1
       - pour gtk_calendar, janvier = 0 */
    gtk_calendar_select_month ( GTK_CALENDAR ( pCalendar ), 
				g_date_get_month ( date ) - 1, 
				g_date_get_year ( date ) );
    gtk_calendar_select_day ( GTK_CALENDAR ( pCalendar ), g_date_get_day ( date ) );
    gtk_calendar_display_options ( GTK_CALENDAR ( pCalendar ),
				   GTK_CALENDAR_SHOW_HEADING |
				   GTK_CALENDAR_SHOW_DAY_NAMES |
				   GTK_CALENDAR_WEEK_START_MONDAY );

    gtk_signal_connect ( GTK_OBJECT ( pCalendar ),
			 "day_selected_double_click",
			 GTK_SIGNAL_FUNC ( gsb_calendar_select_date ),
			 entry );
    gtk_signal_connect ( GTK_OBJECT ( pCalendar ),
			 "key-press-event",
			 GTK_SIGNAL_FUNC ( gsb_calendar_key_press_event ),
			 entry );
    gtk_box_pack_start ( GTK_BOX ( pVBox ),
			 pCalendar,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( pCalendar );

    /* ajoute le bouton annuler */
    bouton = gtk_button_new_with_label ( _("Cancel") );
    gtk_signal_connect_object ( GTK_OBJECT ( bouton ),
				"clicked",
				GTK_SIGNAL_FUNC ( gtk_widget_destroy ),
				GTK_OBJECT ( popup ));

    gtk_box_pack_start ( GTK_BOX ( pVBox ),
			 bouton,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( bouton );

    /* cherche la position où l'on va mettre la popup */
    /* on récupère la position de l'entrée date par rapport à laquelle on va placer la popup */

    gdk_window_get_origin ( GTK_WIDGET ( entry ) -> window,
			    &x,
			    &y );

    /* on récupère la taille de la popup */

    taille_popup = g_malloc ( sizeof ( GtkRequisition ));
    gtk_widget_size_request ( GTK_WIDGET ( popup ), taille_popup );

    /* pour la soustraire à la position de l'entrée date */

    y -= taille_popup -> height;

    /* si une des coordonnées est négative, alors la fonction
       gtk_widget_set_uposition échoue et affiche la popup en 0,0 */

    if ( x < 0 )
	x = 0 ;

    if ( y < 0 )
	y = 0 ;

    /* on place la popup */

    gtk_widget_set_uposition ( GTK_WIDGET ( popup ),
			       x,
			       y );

    /* et on la montre */

    gtk_widget_show ( popup );
    gtk_widget_grab_focus ( GTK_WIDGET ( pCalendar ) );
    return ( popup );
}
/******************************************************************************/

/**
 * called with a double-click on a date on a calendar
 * set the choosen date in the entry
 *
 * \param pCalendar
 * \param entry
 *
 * \return FALSE
 * */
gboolean gsb_calendar_select_date ( GtkCalendar *pCalendar,
				    GtkWidget *entry )
{
    guint year, month, day;
    GtkWidget *pTopLevelWidget;

    /* get the popup to destroy it if we are in a popup */
    pTopLevelWidget = gtk_widget_get_toplevel ( GTK_WIDGET ( pCalendar ) );

    gtk_calendar_get_date ( pCalendar, &year, &month, &day);

    gtk_entry_set_text ( GTK_ENTRY ( entry ),
			 gsb_format_date ( day, month + 1, year ));
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
gboolean gsb_calendar_key_press_event ( GtkCalendar *pCalendar,
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
				 gsb_format_date ( day, month + 1, year ));
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

    day = g_date_day (date);
    month = g_date_month (date);
    year = g_date_year (date);
    g_date_free (date);

    /* to avoid a warning */
    gtk_calendar_select_day( pCalendar , 15 );

    month--;
    gtk_calendar_select_month ( pCalendar , month, year );
    gtk_calendar_select_day( pCalendar , day );
    return TRUE;
}


/******************************************************************************/
/* appelée lors de l'appui des touche + ou - sur les formulaires              */
/* augmente ou diminue la date entrée de 1 day, 1 semaine, 1 month, 1 an,     */
/* suivant la valeur du paramètre « demande »                                 */
/******************************************************************************/
void inc_dec_date ( GtkWidget *entree, gint demande )
{
    GDate *date;

    /* on commence par vérifier que la date est valide */

    if ( !gsb_date_check_and_complete_entry ( entree ) )
	return;

    date = gsb_parse_date_string ( g_strstrip ( ( gchar * ) gtk_entry_get_text ( GTK_ENTRY ( entree ) ) ) );

    switch ( demande )
    {
	case ONE_DAY :
	case ONE_WEEK :

	    g_date_add_days ( date, demande ) ;
	    break ;

	case -ONE_DAY :
	case -ONE_WEEK :

	    g_date_subtract_days ( date, -demande ) ;
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

    gtk_entry_set_text ( GTK_ENTRY ( entree ), gsb_format_gdate ( date ) );
}
/******************************************************************************/


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
