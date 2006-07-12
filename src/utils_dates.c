/* ************************************************************************** */
/*                                  utils_dates.c                             */
/*                                                                            */
/*     Copyright (C)	2000-2003 CÃ©dric Auger (cedric@grisbi.org)      */
/*			2003-2005 Benjamin Drieu (bdrieu@april.org)	      */
/*			2003-2004 Alain Portal (aportal@univ-montp2.fr)	      */
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

#include "include.h"


/*START_INCLUDE*/
#include "utils_dates.h"
#include "calendar.h"
#include "gsb_data_fyear.h"
#include "gsb_form.h"
#include "traitement_variables.h"
#include "utils_str.h"
#include "include.h"
/*END_INCLUDE*/

/*START_STATIC*/
static  gboolean gsb_editable_date_changed ( GtkWidget *entry,
					    gboolean default_func (gint, const GDate *));
static GDate *gsb_editable_date_get_date ( GtkWidget *hbox );
static gboolean gsb_editable_date_popup_calendar ( GtkWidget *button,
					    GtkWidget *entry );
/*END_STATIC*/


/*START_EXTERN*/
extern GtkWidget *entree_date_finale_etat;
extern GtkWidget *entree_date_init_etat;
extern GtkTreeSelection * selection;
extern GtkWidget *window;
/*END_EXTERN*/


/* save of the last date entried */
static gchar *last_date;




/**
 * return the last_date if defined, else the date of the day
 * set last_date if unset
 *
 * \param
 *
 * \return a string contains the date
 * */
gchar *gsb_date_today ( void )
{
    if (!last_date)
    {
	GDate *date;
	gchar date_str[SIZEOF_FORMATTED_STRING_DATE];

	date = gdate_today();
	g_date_strftime ( date_str, SIZEOF_FORMATTED_STRING_DATE, "%x", date );
	gsb_date_set_last_date (date_str);
    }
    return (last_date);
}


/**
 * set the last_date value
 * that value is dumped in memory
 *
 * \param a string wich contains the last date to remain
 * 
 * \return FALSE
 * */
gboolean gsb_date_set_last_date ( const gchar *date )
{
    last_date = my_strdup (date);
    return FALSE;
}


/**
 * set last date to null, so the next
 * call to gsb_date_today will return the today date
 * and not the last typed
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_date_free_last_date ( void )
{
    last_date = NULL;
    return FALSE;
}


/**
 * return the day date in the gdate format
 *
 * \param
 *
 * \return the date of the day
 * */
GDate *gdate_today ( void )
{
    GDate *date;

    date = g_date_new ();
    g_date_set_time (  date,
		       time (NULL));
    return ( date );
}



/**
 * copy the date given in param
 * 
 * \param date a GDate to copy
 * 
 * \return a copy or NULL if no date
 * */
GDate *gsb_date_copy ( GDate *date )
{
    GDate *new_date;

    if ( !date )
	return NULL;

    new_date = g_date_new_dmy ( g_date_get_day ( date ),
				g_date_get_month ( date ),
				g_date_get_year ( date ));

    return new_date;
}



    
/**
 * check the entry to find a date
 * if the entry is empty, set gsb_date_today
 * if the entry is set, try to understand and fill it if necessary
 * for example : 1/4/5 becomes 01/04/2005
 *
 * \param entry the entry to check
 *
 * \return FALSE if problem with the date, TRUE if ok
 * */
gboolean gsb_date_check_and_complete_entry ( GtkWidget *entry )
{
    const gchar *string;
    
    /* if the entry is grey (empty), go away */
    if (gsb_form_check_entry_is_empty (entry))
	return ( FALSE );

    string = gtk_entry_get_text ( GTK_ENTRY (entry));

    if ( !strlen (string))
    {
	/* let the entry empty if we are on the report configuration */

	if ( entry != entree_date_init_etat
	     &&
	     entry != entree_date_finale_etat )
	    gtk_entry_set_text ( GTK_ENTRY (entry),
				 gsb_date_today() );
    }
    else
    {
	GDate *date;

	date = gsb_parse_date_string (string);
	if (!date)
	    return FALSE;

	gtk_entry_set_text ( GTK_ENTRY ( entry ),
			     gsb_format_gdate (date));
    }
    return ( TRUE );
}



/**
 * Create and return a GDate from a string reprensentation of a date.
 *
 * \param a sting wich represent a date
 *
 * \return a newly allocated gdate
 */
GDate *gsb_parse_date_string ( const gchar *date_string )
{
    gchar * separators[4] = { "/", ".", "-", NULL };
    int jour = -1, mois = -1, annee = -1, i;
    gchar **tab_date;
    GDate * date;

    if ( !date_string
	 ||
	 !strlen (date_string))
	return NULL;

    date = (GDate *) g_malloc ( sizeof (GDate) );

    g_date_set_parse ( date, date_string );
    if ( g_date_valid ( date ) )
	return date;

    for ( i = 0; separators[i]; i++ )
    {
	date = gdate_today();
	
	tab_date = g_strsplit ( date_string, separators[i], 3 );

	if ( tab_date[2] && tab_date[1] )
	{
	    /* on a rentrÃ© les 3 chiffres de la date */
	    jour = utils_str_atoi ( tab_date[0] );
	    mois = utils_str_atoi ( tab_date[1] );
	    annee = utils_str_atoi ( tab_date[2] );
	    
	    if ( annee < 100 )
	    {
		if ( annee < 80 )
		    annee = annee + 2000;
		else
		    annee = annee + 1900;
	    }
	    if ( g_date_valid_dmy (jour, mois, annee ) )
		return g_date_new_dmy ( jour, mois, annee );
	    else
		return NULL;
	}
	else
	{
	    if ( tab_date[1] )
	    {
		/* on a rentrÃ© la date sous la forme xx/xx,
		   il suffit de mettre l'annÃ©e courante */
		jour = utils_str_atoi ( tab_date[0] );
		mois = utils_str_atoi ( tab_date[1] );
		annee = g_date_year ( date );
		if ( g_date_valid_dmy (jour, mois, annee ) )
		    return g_date_new_dmy ( jour, mois, annee );
		else
		    return NULL;

	    }
	    else
	    {
		/* on a rentrÃ© que le jour de la date, il
		   faut mettre le mois et l'annÃ©e courante
		   ou bien on a rentrÃ© la date sous forme
		   jjmm ou jjmmaa ou jjmmaaaa */
		gchar buffer[3];
		
		switch ( strlen ( tab_date[0] ) )
		{
		/* forme jj ou j */
		case 1:
		case 2:
		    jour = utils_str_atoi ( tab_date[0] );
		    mois = g_date_month ( date );
		    annee = g_date_year ( date );
		    if ( g_date_valid_dmy (jour, mois, annee ) )
			return g_date_new_dmy ( jour, mois, annee );
		    else
			return NULL;

		/* forme jjmm */
		case 4 :
		    buffer[0] = tab_date[0][0];
		    buffer[1] = tab_date[0][1];
		    buffer[2] = 0;

		    jour = utils_str_atoi ( buffer );
		    mois = utils_str_atoi ( tab_date[0] + 2 );
		    annee = g_date_year ( date );
		    if ( g_date_valid_dmy (jour, mois, annee ) )
			return g_date_new_dmy ( jour, mois, annee );
		    else
			return NULL;

		/* forme jjmmaa */
		case 6:
		    buffer[0] = tab_date[0][0];
		    buffer[1] = tab_date[0][1];
		    buffer[2] = 0;

		    jour = utils_str_atoi ( buffer );
		    buffer[0] = tab_date[0][2];
		    buffer[1] = tab_date[0][3];

		    mois = utils_str_atoi ( buffer );
		    annee = utils_str_atoi ( tab_date[0] + 4 ) + 2000;
		    if ( g_date_valid_dmy (jour, mois, annee ) )
			return g_date_new_dmy ( jour, mois, annee );
		    else
			return NULL;

		/* forme jjmmaaaa */
		case 8:
		    buffer[0] = tab_date[0][0];
		    buffer[1] = tab_date[0][1];
		    buffer[2] = 0;

		    jour = utils_str_atoi ( buffer );
		    buffer[0] = tab_date[0][2];
		    buffer[1] = tab_date[0][3];

		    mois = utils_str_atoi ( buffer );
		    annee = utils_str_atoi ( tab_date[0] + 4 );
		    if ( g_date_valid_dmy (jour, mois, annee ) )
			return g_date_new_dmy ( jour, mois, annee );
		    else
			return NULL;
		}
	    }
	}
	g_strfreev ( tab_date );
    }

    return NULL;
}



/**
 * Create and return a GDate from a string locale independant
 * representation of a date.  It expects format %m/%d/%Y (american
 * style).
 *
 * \param	
 *
 */
GDate *gsb_parse_date_string_safe ( const gchar *date_string )
{
    gchar **tab_date;
    GDate * date;
	
    tab_date = g_strsplit ( date_string, "/", 3 );
    if ( tab_date[0] && tab_date[1] && tab_date[2] )
    {
	date = g_date_new_dmy ( utils_str_atoi ( tab_date[1] ),
				utils_str_atoi ( tab_date[0] ),
				utils_str_atoi ( tab_date[2] ) );
	g_strfreev ( tab_date );
	return date;
    }

    return NULL;
}



/**
 * Convenience function that return the string representation of a
 * date based on locale settings.
 *
 * \param day		Day of the date to represent.
 * \param month		Month of the date to represent.
 * \param year		Year of the date to represent.
 *
 * \return		A string representing date.
 */
gchar * gsb_format_date ( gint day, gint month, gint year )
{
    return gsb_format_gdate ( g_date_new_dmy ( day, month, year ) );
}



/**
 * Convenience function that return the string representation of a
 * date based on locale settings.
 *
 * \param date		A GDate structure containing the date to represent.
 *
 * \return		A string representing date.
 */
gchar * gsb_format_gdate ( GDate *date )
{
    gchar retour_str[SIZEOF_FORMATTED_STRING_DATE];

    if ( !date || !g_date_valid ( date ))
    {
	return my_strdup ( "" );
    }

    g_date_strftime ( retour_str, SIZEOF_FORMATTED_STRING_DATE, "%x", date );

    return my_strdup ( retour_str );
}



/**
 * Convenience function that return the string representation of a
 * date without locale.
 *
 * \param date		A GDate structure containing the date to represent.
 *
 * \return		A string representing date.
 */
gchar * gsb_format_gdate_safe ( GDate *date )
{
    gchar retour_str[SIZEOF_FORMATTED_STRING_DATE];

    if ( !date || !g_date_valid ( date ))
    {
	return my_strdup ( "" );
    }

    g_date_strftime ( retour_str, SIZEOF_FORMATTED_STRING_DATE, "%m/%d/%Y", date );

    return my_strdup ( retour_str );
}




/**
 * Creates a new GtkHBox with an entry to type in a date and a button to show a calendar
 * change value change directly in memory by a call to default_func with number_for_func parameter,
 * the target function must be :
 * 	(default_func) ( gint number_for_func,
 * 			 GDate *date )
 * ex : gsb_data_fyear_set_begining_date ( gint fyear_number, GDate *date )
 *
 * \param date the date we want to fill the entry, or NULL
 * \param An optional hook function to execute if the entry is modified
 * 	hook should be :
 * 		gboolean hook ( GtkWidget *entry,
 * 				gpointer data )
 * \param data some data to pass to hook
 * \param default_func the function to call
 * \param number_fo_func a gint used to call the default_func
 *
 * \return a widget hbox with the entry and a button
 */
GtkWidget *gsb_editable_date_new ( gchar *value,
				   GCallback hook,
				   gpointer data,
				   GCallback default_func,
				   gint number_for_func )
{
    GtkWidget *entry, *hbox, *button;

    /* the hbox will contain an entry an a button */
    hbox = gtk_hbox_new ( FALSE, 6 );

    /* create and fill the entry */
    entry = gtk_entry_new ();
    gtk_box_pack_start ( GTK_BOX(hbox), entry,
			 TRUE, TRUE, 0 );
    g_object_set_data ( G_OBJECT (entry),
			"number_for_func", GINT_TO_POINTER (number_for_func));

    if (value)
	gtk_entry_set_text ( GTK_ENTRY(entry), value );

    /* set the default func */
    if (default_func)
	g_object_set_data ( G_OBJECT ( entry ), "changed", 
			    (gpointer) g_signal_connect_after (GTK_OBJECT(entry), "changed",
							       ((GCallback) gsb_editable_date_changed), default_func ));
    if ( hook )
	g_object_set_data ( G_OBJECT ( entry ), "changed-hook", 
			    (gpointer) g_signal_connect_after (GTK_OBJECT(entry), "changed",
							      ((GCallback) hook), data ));

    /* create the button to show a calendar */
    button = gtk_button_new_with_label ("...");
    gtk_button_set_relief ( GTK_BUTTON (button),
			    GTK_RELIEF_NONE );
    gtk_box_pack_start ( GTK_BOX(hbox), button,
			 FALSE, FALSE, 0 );
    g_signal_connect ( GTK_OBJECT ( button ), "clicked",
		       G_CALLBACK (gsb_editable_date_popup_calendar), entry );

    /* to find it easily, set the adr to the entry  and the hbox */
    g_object_set_data ( G_OBJECT (hbox),
			"entry", entry );

    return hbox;
}



/**
 * set a date in an editable_date
 * 2 parts : a date and a number used to call the default func
 *
 * \param hbox The gsb_editable_date widget.
 * \param value The new date to modify.
 * \param number_for_func to call the default func
 */
void gsb_editable_date_set_value ( GtkWidget *hbox,
				   GDate *value,
				   gint number_for_func )
{
    GtkWidget * entry;

    /* Find associated gtkentry */
    entry = g_object_get_data ( G_OBJECT(hbox), "entry" );

    /* Block everything */
    if ( g_object_get_data (G_OBJECT (entry), "changed") > 0 )
	g_signal_handler_block ( GTK_OBJECT(entry),
				 (gulong) g_object_get_data (G_OBJECT (entry), 
							     "changed"));
    if ( g_object_get_data (G_OBJECT (entry), "changed-hook") > 0 )
	g_signal_handler_block ( GTK_OBJECT(entry),
				 (gulong) g_object_get_data (G_OBJECT (entry), 
							     "changed-hook"));

    if (value)
	gtk_entry_set_text ( GTK_ENTRY(entry),
			     gsb_format_gdate (value));
    else
	gtk_entry_set_text ( GTK_ENTRY (entry),
			     "" );

    g_object_set_data ( G_OBJECT ( entry ),
			"number_for_func", GINT_TO_POINTER (number_for_func));

    /* Unblock everything */
    if ( g_object_get_data (G_OBJECT (entry), "changed") > 0 )
	g_signal_handler_unblock ( GTK_OBJECT(entry),
				   (gulong) g_object_get_data (G_OBJECT (entry), 
							       "changed"));
    if ( g_object_get_data (G_OBJECT (entry), "changed-hook") > 0 )
	g_signal_handler_unblock ( GTK_OBJECT(entry),
				   (gulong) g_object_get_data (G_OBJECT (entry), 
							       "changed-hook"));
}


/**
 * called when something change in an entry of a gsb_editable_date
 * by gsb_editable_date_new
 *
 * \param entry The reference GtkEntry
 * \param default_func the function to call to change the value in memory
 *
 * \return FALSE
 */
static gboolean gsb_editable_date_changed ( GtkWidget *entry,
					    gboolean default_func (gint, const GDate *))
{
    gint number_for_func;

    /* just to be sure... */
    if (!default_func || !entry)
	return FALSE;

    number_for_func = GPOINTER_TO_INT ( g_object_get_data (G_OBJECT (entry), "number_for_func"));
    default_func ( number_for_func,
		   gsb_parse_date_string (gtk_entry_get_text ( GTK_ENTRY (entry))));

    /* Mark file as modified */
    modification_fichier ( TRUE );

    return FALSE;
}



/**
 * Pop up a window with a calendar that allows a date selection.  This
 * calendar runs "date_selectionnee" as a callback if a date is
 * selected.
 *
 * \param button Normally a GtkButton that triggered the handler.
 * This parameter will be used as a base to set popup's position.
 * This widget must also have a parameter (data) of name "entry"
 * which contains a pointer to a GtkEntry used to set initial value of
 * calendar.
 * \param entry the entry of the gsb_editable_date
 */
gboolean gsb_editable_date_popup_calendar ( GtkWidget *button,
					    GtkWidget *entry )
{
    GtkWidget *popup, *popup_boxv, *calendar, *cancel_button, *frame;
    GtkRequisition entry_size, popup_size;
    GDate *date;
    gint x, y;

    /* Find popup position */
    gdk_window_get_origin ( GTK_BUTTON (button) -> event_window, &x, &y );
    gtk_widget_size_request ( GTK_WIDGET (button), &entry_size );
    y = y + entry_size.height;

    /* Create popup */
    popup = gtk_window_new ( GTK_WINDOW_POPUP );
    gtk_window_set_modal ( GTK_WINDOW (popup), TRUE);

    /* Create popup widgets */
    frame = gtk_frame_new ( NULL );
    gtk_container_add ( GTK_CONTAINER (popup), frame);
    popup_boxv = gtk_vbox_new ( FALSE, 5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( popup_boxv ), 5 );
    gtk_container_add ( GTK_CONTAINER ( frame ), popup_boxv);

    /* Set initial date according to entry */
    if ( strlen (gtk_entry_get_text (GTK_ENTRY(entry))))
	date = gsb_parse_date_string (gtk_entry_get_text (GTK_ENTRY (entry)));
    else
	date = gdate_today ();

    /* Creates calendar */
    calendar = gtk_calendar_new();
    gtk_calendar_select_month ( GTK_CALENDAR ( calendar ), g_date_get_month ( date ) - 1,
				g_date_get_year ( date ) );
    gtk_calendar_select_day  ( GTK_CALENDAR ( calendar ), g_date_get_day ( date ) );
    gtk_calendar_display_options ( GTK_CALENDAR ( calendar ),
				   GTK_CALENDAR_SHOW_HEADING |
				   GTK_CALENDAR_SHOW_DAY_NAMES |
				   GTK_CALENDAR_WEEK_START_MONDAY );

    /* Create handlers */
    gtk_signal_connect ( GTK_OBJECT (calendar), "day-selected-double-click",
			 G_CALLBACK (gsb_calendar_select_date), entry );
    gtk_signal_connect ( GTK_OBJECT (calendar), "key-press-event",
			 G_CALLBACK (gsb_calendar_key_press_event), entry );
    gtk_box_pack_start ( GTK_BOX ( popup_boxv ),
			 calendar,
			 TRUE, TRUE, 0 );

    /* Add the "cancel" button */
    cancel_button = gtk_button_new_with_label ( _("Cancel") );
    gtk_signal_connect_object ( GTK_OBJECT (cancel_button), "clicked",
				G_CALLBACK (gtk_widget_destroy),
				GTK_WIDGET ( popup ) );
    gtk_box_pack_start ( GTK_BOX ( popup_boxv ), cancel_button,
			 TRUE, TRUE, 0 );


    /* Show everything */
    gtk_widget_show_all ( popup );
    gtk_widget_set_uposition ( GTK_WIDGET ( popup ), x, y );
    gtk_widget_size_request ( GTK_WIDGET ( popup ), &popup_size );
    gtk_widget_set_uposition ( GTK_WIDGET ( popup ), 
			       x-popup_size.width+entry_size.width, y );

    /* Grab pointer */
    gdk_pointer_grab ( popup -> window, TRUE,
		       GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
		       GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK |
		       GDK_POINTER_MOTION_MASK,
		       NULL, NULL, GDK_CURRENT_TIME );
    return FALSE;
}


/**
 * get the date in the entry and return it as a GDate
 * if the date is not valid, return NULL
 *
 * \param hbox the gsb_editable_date
 *
 * \return a GDate or NULL
 * */
GDate *gsb_editable_date_get_date ( GtkWidget *hbox )
{
    GtkWidget * entry;
    GDate *date;

    /* Find associated gtkentry */
    entry = g_object_get_data ( G_OBJECT(hbox), "entry" );

    date = gsb_parse_date_string (gtk_entry_get_text (GTK_ENTRY (entry)));
    return date;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
