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
#include "./gsb_form_widget.h"
#include "./utils_str.h"
#include "./parametres.h"
#include "./gsb_calendar_entry.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/


/*START_EXTERN*/
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
GDate *gsb_date_copy ( const GDate *date )
{
    GDate *new_date;

    if ( !date
	 ||
	 !g_date_valid (date))
	return NULL;

    new_date = g_date_new_dmy ( g_date_get_day ( date ),
				g_date_get_month ( date ),
				g_date_get_year ( date ));

    return new_date;
}



    
/**
 * check the entry to find a date
 * if the entry is empty, set gsb_date_today according to set_today
 * if the entry is set, try to understand and fill it if necessary
 * for example : 1/4/5 becomes 01/04/2005
 *
 * \param entry the entry to check
 * \param set_today if TRUE and the entry is empty, will set into the today date
 *
 * \return FALSE if problem with the date, TRUE if ok or entry empty
 * */
gboolean gsb_date_check_and_complete_entry ( GtkWidget *entry,
					     gboolean set_today )
{
    const gchar *string;

    if (!entry)
	return FALSE;
    
    /* if the entry is grey (empty), go away */
    if (gsb_form_widget_check_empty (entry))
	return (TRUE);

    string = gtk_entry_get_text ( GTK_ENTRY (entry));
    if (!string)
	return FALSE;

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
	if (set_today)
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
 * \return a newly allocated gdate or NULL if cannot set
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
    string = my_strdup (date_string);

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
		    tmp_string = my_strdup (tab_date[0]);
		    
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
gchar *gsb_format_date ( gint day, gint month, gint year )
{
    return gsb_format_gdate ( g_date_new_dmy ( day, month, year ) );
}



/**
 * Convenience function that return the string representation of a
 * date based on locale settings.
 *
 * \param date		A GDate structure containing the date to represent.
 *
 * \return		A newly allocated string representing date.
 */
gchar *gsb_format_gdate ( const GDate *date )
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
 * \return		A newly allocated string representing date.
 */
gchar * gsb_format_gdate_safe ( const GDate *date )
{
    gchar retour_str[SIZEOF_FORMATTED_STRING_DATE];

    if ( !date || !g_date_valid ( date ))
    {
	return my_strdup ( "" );
    }

    g_date_strftime ( retour_str, SIZEOF_FORMATTED_STRING_DATE, "%m/%d/%Y", date );

    return my_strdup ( retour_str );
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
