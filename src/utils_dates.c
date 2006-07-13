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
#include "structures.h"
#include "include.h"
/*END_INCLUDE*/

/*START_STATIC*/
static  gboolean gsb_editable_date_changed ( GtkWidget *entry,
					    gboolean default_func (gint, const GDate *));
static  gboolean gsb_editable_date_check ( GtkWidget *entry,
					  gpointer null );
static gboolean gsb_editable_date_focus_out ( GtkWidget *entry,
				       GdkEventFocus *ev,
				       gpointer null );
static GDate *gsb_editable_date_get_date ( GtkWidget *hbox );
static gboolean gsb_editable_date_popup_calendar ( GtkWidget *button,
					    GtkWidget *entry );
/*END_STATIC*/


/*START_EXTERN*/
extern GtkWidget *entree_date_finale_etat;
extern GtkWidget *entree_date_init_etat;
extern FILE * out;
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

    if ( strlen (string))
    {
	GDate *date;

	date = gsb_parse_date_string (string);
	if (!date)
	    return FALSE;

	gtk_entry_set_text ( GTK_ENTRY ( entry ),
			     gsb_format_gdate (date));
	g_date_free (date);
    }
	else
    {
	/* let the entry empty if we are on the report configuration
	 * FIXME : should set a boolean at this function to choose to let the entry free or not */

	if ( entry != entree_date_init_etat
	     &&
	     entry != entree_date_finale_etat )
	    gtk_entry_set_text ( GTK_ENTRY (entry),
				 gsb_date_today() );
    }
    return ( TRUE );
}



/**
 * Create and try to return a GDate from a string reprensentation of a date.
 * separator can be / . - :
 * and numbers can be stick (ex 01012001)
 *
 * \param a string wich represent a date
 *
 * \return a newly allocated gdate
 */
GDate *gsb_parse_date_string ( const gchar *date_string )
{
    gchar *separators = "/-:";
    gint day = -1, month = -1, year = -1;
    GDate *date;
    gchar *string;
    gchar **tab_date;

    if ( !date_string
	 ||
	 !strlen (date_string))
	return NULL;

    /* to keep the const gchar in that function */
    string = g_strdup (date_string);

    /* to use the glib parser at its best, we change here all the separators
     * to . */
    g_strdelimit ( string,
		   separators, '.' );

    /* first check : try the parser of glib */
    /* i have a proble with the glib parser :
     * if if type 311206 and i want 31/12/2006, glib returns 06/12/2031...
     * so if there is no separator, i prefer use my own function and don't work with
     * glib */
    if (g_strrstr (string, "."))
    {
	date = g_date_new ();
	g_date_set_parse ( date, string );
	if ( g_date_valid ( date ) )
	{
	    /* the parser of glib worked fine
	     * but can now improve it for years :
	     * it's easier to write 1/1/1 instead of 1/1/2001
	     * so if year is > 70 and <= 99, we add 1900
	     * if year is < 70, we add 2000 to it */

	    year = g_date_get_year (date);

	    if (year < 100)
	    {
		if (year >= 70)
		    g_date_add_years (date, 1900);
		else
		    g_date_add_years (date, 2000);
	    }
	    g_free (string);
	    return date;
	}
	g_date_free (date);
    }

    /* parser didn't work, try to parse ourselves */
    tab_date = g_strsplit ( string, ".", 3 );

    /* needn't anymore string now */
    g_free (string);

    if ( tab_date[2] && tab_date[1] )
    {
	/* we have the 3 numbers of the date,
	 * here we assume that we have dd/mm/yy
	 * FIXME : for other locales ? (for now just
	 * try international, but not enough, should use locales)
	 * or set in preferences ? */
	day = utils_str_atoi (tab_date[0]);
	month = utils_str_atoi (tab_date[1]);
	year = utils_str_atoi (tab_date[2]);

	/* needn't tab_date anymore */
	g_strfreev ( tab_date );

	/* check for quick entry */
	if ( year < 100 )
	{
	    if ( year >= 70 )
		year = year + 1900;
	    else
		year = year + 2000;
	}

	if ( g_date_valid_dmy (day, month, year))
	    return g_date_new_dmy ( day, month, year );
	else
	{
	    /* ok, dd/mm/yy doesn't work, will try international
	     * style : yy/mm/dd */
	    if (g_date_valid_dmy (year, month, day))
		return g_date_new_dmy (year, month, day);
	    else
		return NULL;
	}
    }
    else
    {
	/* we will need current year and month,
	 * set them already */
	date = gdate_today();
	month = g_date_month (date);
	year = g_date_year (date);
	g_date_free (date);

	if (tab_date[1])
	{
	    /* we have xx/xx, as the day is the most important,
	     * we assume it is dd/mm, so we add the year */
	    day = utils_str_atoi (tab_date[0]);
	    month = utils_str_atoi (tab_date[1]);
	    if ( g_date_valid_dmy (day, month, year ) )
		return g_date_new_dmy ( day, month, year );
	    else
	    {
		/* dd/mm doesn't work, i don't know if mm/dd can appen,
		 * do it here because nothing to lose */
		if (g_date_valid_dmy (year, month, day))
		    return g_date_new_dmy (year, month, day);
		else
		    return NULL;
	    }
	}
	else
	{
	    /* there is only 1 number, so can be :
	     * - 1 or 2 digits : it's dd
	     * - 4 digits : it should be ddmm (or mmdd ?)
	     * - 6 digits : it should be ddmmyy (or yymmdd ?)
	     * - 8 digits : it should be ddmmyyyy (or yyyymmdd)
	     *   */
	    gchar *tmp_string;

	    switch (strlen (tab_date[0]))
	    {
		/* d or dd */
		case 1:
		case 2:
		    day = utils_str_atoi (tab_date[0]);
		    if ( g_date_valid_dmy (day, month, year))
			return g_date_new_dmy (day, month, year);
		    else
			return NULL;
		    break;

		    /* ddmm */
		case 4 :
		    month = utils_str_atoi ( tab_date[0] + 2 );
		    tab_date[0][2] = 0;
		    day = utils_str_atoi (tab_date[0]);
		    g_strfreev (tab_date);

		    if ( g_date_valid_dmy (day, month, year))
			return g_date_new_dmy ( day, month, year );
		    else
		    {
			/* we try mmdd */
			if (g_date_valid_dmy (month, day, year))
			    return g_date_new_dmy (month, day, year);
			else
			    return NULL;
		    }
		    break;

		    /* ddmmy */
		case 5:
		    /* ddmmyy */
		case 6:
		    printf ("ça passe\n" );
		    year = utils_str_atoi ( tab_date[0] + 4 );
		    if (year < 70)
			year = year + 2000;
		    else
			year = year + 1900;

		    tab_date[0][4] = 0;
		    month = utils_str_atoi ( tab_date[0] + 2 );
		    tab_date[0][2] = 0;
		    day = utils_str_atoi (tab_date[0]);
		    g_strfreev (tab_date);

		    if ( g_date_valid_dmy (day, month, year ) )
			return g_date_new_dmy ( day, month, year );
		    else
		    {
			/* try yymmdd */
			if (g_date_valid_dmy (year, month, day))
			    return g_date_new_dmy (year, month, day);
			else
			    return NULL;
		    }
		    break;

		    /* ddmmyyyy */
		case 8:
		    /* ddmmyyyy should pass before if we set that case 8: just
		     * after case 6: ; but in that case we cannot have yyyymmdd
		     * so we do something here */

		    /* first we save the string in case ddmmyyyy doesn't work */
		    tmp_string = g_strdup (tab_date[0]);
		    
		    /* the first part is the same as for case 6: */
		    year = utils_str_atoi ( tab_date[0] + 4 );
		    tab_date[0][4] = 0;
		    month = utils_str_atoi ( tab_date[0] + 2 );
		    tab_date[0][2] = 0;
		    day = utils_str_atoi (tab_date[0]);
		    g_strfreev (tab_date);

		    if ( g_date_valid_dmy (day, month, year))
		    {
			g_free (tmp_string);
			return g_date_new_dmy ( day, month, year );
		    }
		     
		    /* it is not ddmmyyyy, so we try yyyymmdd */
		    day = utils_str_atoi ( tmp_string + 6 );
		    tmp_string[6] = 0;
		    month = utils_str_atoi ( tmp_string + 4 );
		    tmp_string[4] = 0;
		    year = utils_str_atoi (tmp_string);
		    g_free (tmp_string);

		    if (g_date_valid_dmy (year, month, day))
			return g_date_new_dmy (year, month, day);
		    else
			return NULL;
		    break;
	    }
	}
    }
    g_strfreev ( tab_date );
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
    GtkStyle *style;
    GdkColor normal_color;
    GdkColor red_color;

    /* the hbox will contain an entry an a button */
    hbox = gtk_hbox_new ( FALSE, 6 );

    /* create and fill the entry */
    entry = gtk_entry_new ();
    gtk_box_pack_start ( GTK_BOX(hbox), entry,
			 TRUE, TRUE, 0 );
    g_object_set_data ( G_OBJECT (entry),
			"number_for_func", GINT_TO_POINTER (number_for_func));

    /* changing the style makes something not beautiful with the frame,
     * i think it's possible to change that, but without frame is pretty
     * for me, so set like that for now */
    gtk_entry_set_has_frame ( GTK_ENTRY (entry),
			      FALSE );

    /* set the red/normal style */
    normal_color.red = COULEUR_NOIRE_RED;
    normal_color.green = COULEUR_NOIRE_GREEN;
    normal_color.blue = COULEUR_NOIRE_BLUE;
    normal_color.pixel = 0;

    red_color.red = COULEUR_ROUGE_RED;
    red_color.green = COULEUR_ROUGE_GREEN;
    red_color.blue = COULEUR_ROUGE_BLUE;
    red_color.pixel = 0;

    /* prepare the style */
    style = gtk_style_new ();
    style -> text[GTK_STATE_NORMAL] = normal_color;
    g_object_set_data ( G_OBJECT (entry),
			"normal_style", style );

    style = gtk_style_new ();
    style -> text[GTK_STATE_NORMAL] = red_color;
    g_object_set_data ( G_OBJECT (entry),
			"red_style", style );

    /* set the function wich check what is writen in real time */
    g_signal_connect_after ( G_OBJECT (entry), "changed",
			     G_CALLBACK (gsb_editable_date_check),
			     NULL );

    /* set the focus out function, wich will try to find a good date
     * from the date entried (ie 1/1/1 will give 01/01/2001) */
    g_signal_connect_after ( G_OBJECT (entry), "focus-out-event",
			     G_CALLBACK (gsb_editable_date_focus_out),
			     NULL );

    /* set the content, that will check the date too */
    if (value)
	gtk_entry_set_text ( GTK_ENTRY(entry), value );

    /* set the default func */
    if (default_func)
	g_object_set_data ( G_OBJECT ( entry ), "changed", 
			    (gpointer) g_signal_connect_after (G_OBJECT(entry), "changed",
							       G_CALLBACK (gsb_editable_date_changed), default_func ));
    if ( hook )
	g_object_set_data ( G_OBJECT ( entry ), "changed-hook", 
			    (gpointer) g_signal_connect_after (G_OBJECT(entry), "changed",
							      ((GCallback) hook), data ));

    /* create the button to show a calendar */
    button = gtk_button_new_with_label ("...");
    gtk_button_set_relief ( GTK_BUTTON (button),
			    GTK_RELIEF_NONE );
    gtk_box_pack_start ( GTK_BOX(hbox), button,
			 FALSE, FALSE, 0 );
    g_signal_connect ( G_OBJECT ( button ), "clicked",
		       G_CALLBACK (gsb_editable_date_popup_calendar), entry );

    /* to find it easily, set the adr to the entry  and the hbox */
    g_object_set_data ( G_OBJECT (hbox),
			"entry", entry );

    return hbox;
}


/**
 * check the content of the date entry each time something is written
 * set the content red if not a good date
 *
 * \param entry
 * \param null
 *
 * \return FALSE
 * */
static gboolean gsb_editable_date_check ( GtkWidget *entry,
					  gpointer null )
{
    GDate *date;
    GtkStyle *style;

    date = gsb_parse_date_string (gtk_entry_get_text ( GTK_ENTRY (entry)));

    if (date)
    {
	/* the content is ok */
	style = g_object_get_data ( G_OBJECT (entry),
				    "normal_style" );
	g_date_free (date);
    }
    else
	/* the date is not correct */
	style = g_object_get_data ( G_OBJECT (entry),
				    "red_style" );

    gtk_widget_set_style ( entry,
			   style );
    return FALSE;
}

/**
 * called when the editable date receive a focus out
 * try to make a good date from an user entry
 * ie 1/1/1 => 01/01/2001
 *
 * \param entry
 * \param ev the GdkEventFocus
 * \param null not used
 *
 * \return FALSE
 * */
gboolean gsb_editable_date_focus_out ( GtkWidget *entry,
				       GdkEventFocus *ev,
				       gpointer null )
{
    GDate *date;

    if (!entry)
	return FALSE;

    date = gsb_parse_date_string (gtk_entry_get_text (GTK_ENTRY (entry)));

    if (!date)
	return FALSE;

    gtk_entry_set_text ( GTK_ENTRY ( entry ),
			 gsb_format_gdate (date));
    g_date_free (date);

    return FALSE;
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
	g_signal_handler_block ( G_OBJECT(entry),
				 (gulong) g_object_get_data (G_OBJECT (entry), 
							     "changed"));
    if ( g_object_get_data (G_OBJECT (entry), "changed-hook") > 0 )
	g_signal_handler_block ( G_OBJECT(entry),
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
	g_signal_handler_unblock ( G_OBJECT(entry),
				   (gulong) g_object_get_data (G_OBJECT (entry), 
							       "changed"));
    if ( g_object_get_data (G_OBJECT (entry), "changed-hook") > 0 )
	g_signal_handler_unblock ( G_OBJECT(entry),
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
    g_signal_connect ( G_OBJECT (calendar), "day-selected-double-click",
		       G_CALLBACK (gsb_calendar_select_date), entry );
    g_signal_connect ( G_OBJECT (calendar), "key-press-event",
		       G_CALLBACK (gsb_calendar_key_press_event), entry );
    gtk_box_pack_start ( GTK_BOX ( popup_boxv ),
			 calendar,
			 TRUE, TRUE, 0 );

    /* Add the "cancel" button */
    cancel_button = gtk_button_new_with_label ( _("Cancel") );
    g_signal_connect_swapped ( G_OBJECT (cancel_button), "clicked",
			       G_CALLBACK (gtk_widget_destroy),
			       G_OBJECT ( popup ) );
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
