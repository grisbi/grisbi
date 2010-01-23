/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cedric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*                      2009 Pierre Biava (pierre@pierre.biava.name)          */
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

#include "include.h"
#ifdef _MSC_VER
#include <winnls.h>
#else
#include <langinfo.h>
#endif//_MSC_VER

/*START_INCLUDE*/
#include "utils_dates.h"
#include "./dialog.h"
#include "./gsb_form_widget.h"
#include "./utils_str.h"
#include "./gsb_calendar_entry.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gchar **split_unique_datefield ( gchar * string, gchar date_tokens [] );
/*END_STATIC*/


/*START_EXTERN*/
extern gint max;
/*END_EXTERN*/


/* save of the last date entried */
static gchar *last_date = NULL;

struct struct_last_entry_date {
    gchar *date_string;
    GDate *last_entry_date;
};
static struct struct_last_entry_date *buffer_entry_date = NULL;

/**
 * return the last_date if defined, else the date of the day
 * set last_date if unset
 *
 * \param
 *
 * \return a string contains the date (DO NOT free this string)
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
 * \param a string which contains the last date to remain
 * 
 * */
void gsb_date_set_last_date ( const gchar *date )
{
    if ( last_date ) 
        g_free ( last_date );
    last_date = my_strdup (date);
}


/**
 * set last date to null, so the next
 * call to gsb_date_today will return the today date
 * and not the last typed
 *
 * \param
 *
 * */
void gsb_date_free_last_date ( void )
{
    if ( last_date ) 
        g_free ( last_date );
    last_date = NULL;
}


/**
 * return the day date in the gdate format
 *
 * \param
 *
 * \return a newly allocated GDate which represents the date of the day. Use g_date_free to free memory when no more used.
 * */
GDate *gdate_today ( void )
{
    GDate *date;

    date = g_date_new ();
    g_date_set_time_t (  date, time (NULL));
    return ( date );
}



/**
 * copy the date given in param
 * 
 * \param date a GDate to copy
 * 
 * \return a copy or NULL if no date. Use g_date_free to free memory when no more used.
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
		gchar* tmpstr;

        date = gsb_date_get_last_entry_date ( string );
        if (!date)
            return FALSE;

        tmpstr = gsb_format_gdate (date);
        gtk_entry_set_text ( GTK_ENTRY ( entry ), tmpstr);
        if ( buffer_entry_date == NULL )
            buffer_entry_date = g_malloc0 ( sizeof (struct struct_last_entry_date) );
        buffer_entry_date -> date_string = g_strdup ( tmpstr );
        buffer_entry_date -> last_entry_date = date;
        g_free ( tmpstr );
    }
    else
    {
    if (set_today)
        gtk_entry_set_text ( GTK_ENTRY (entry), gsb_date_today() );
    }
    return ( TRUE );
}


/**
 * check the date in entry and return TRUE or FALSE
 *
 * \param entry an entry containing a date
 *
 * \return TRUE date is valid, FALSE date is invalid
 * */
gboolean gsb_date_check_entry ( GtkWidget *entry )
{
    const gchar *string;

    if (!entry)
    return FALSE;
    
    string = gtk_entry_get_text ( GTK_ENTRY (entry));
    if (!string)
    return FALSE;

    if ( strlen (string))
    {
        GDate *date;

        date = gsb_date_get_last_entry_date ( string );
        if (!date)
            return FALSE;
        else
        {
            if ( buffer_entry_date == NULL )
                buffer_entry_date = g_malloc0 ( sizeof (struct struct_last_entry_date) );
            buffer_entry_date -> date_string = g_strdup ( string );
            buffer_entry_date -> last_entry_date = date;
        }
    }
    return ( TRUE );
}

/**
 * try to split a compact date into an understanding date
 * ie 01042000 to 01/04/2000
 *
 * \param string
 * \param date_tokens
 *
 * \return NULL if not a date
 */
gchar **split_unique_datefield ( gchar * string, gchar date_tokens [] )
{
/*TODO dOm : I add a & before date_tokens to avoid warning. Is is correct ? */
    gchar ** return_tab = g_new ( gchar *, g_strv_length ( &date_tokens ) + 1 );
    int size = strlen ( string );
    gchar * max = string + size;
    int i = 0;

    /* size 1 could be used for example if we write 1 to set 01/mm/yyyy */
    if ( size != 1 && size != 2 && size != 4 && size != 6 && size != 8 )
    {
    return NULL;
    }

    for ( i = 0 ; date_tokens [ i ] && string < max; i ++ )
    {
        if ( size != 8 ||
             date_tokens [ i ] != 'Y' )
        {
            return_tab [ i ] = g_strndup ( string, 2 );
            string += 2;
        }
        else
        {
            return_tab [ i ] = g_strndup ( string, 4 );
            string += 4;
        }
    }

    return_tab [ i ] = NULL;

    return return_tab;
}



/**
 * Create and try to return a GDate from a string representation of a date.
 * separator can be / . - :
 * and numbers can be stick (ex 01012001)
 *
 * \param a string wich represent a date
 *
 * \return a newly allocated gdate or NULL if cannot set
 */
GDate *gsb_parse_date_string ( const gchar *date_string )
{
    GDate *date;
    gchar *string, *format;
    gchar **tab_date;
    gchar date_tokens [ 4 ] = { 0, 0, 0, 0 };
    int num_tokens = 0, num_fields = 0, i, j;

    if ( !date_string
    ||
    !strlen (date_string))
    return NULL;

    /* Keep the const gchar in that function */
    string = g_strdup (date_string);
    if ( ! strlen ( string ) )
        return NULL;
    g_strstrip ( string );

    /* Obtain date format tokens to compute order. */
#ifdef _MSC_VER
    gchar *sreturn;
    gchar **tab_format;
    int k;

	sreturn = g_strnfill(81,'\0');
	GetLocaleInfo(GetThreadLocale(), LOCALE_SSHORTDATE, sreturn, 80);
	g_strcanon (sreturn, "dMy", '.');
	tab_format = g_strsplit(sreturn, ".", 3);
	g_free(sreturn);
	format = "";
	while (k < 3)
	{
		if(!strncmp(tab_format[k], "dd", 2) || !strncmp(tab_format[k], "d", 1))
			format = g_strconcat(format, "%d", NULL);
		else if(!strncmp(tab_format[k], "MM", 2) || !strncmp(tab_format[k], "M", 1))
			format = g_strconcat(format, "%m", NULL);
		else if(!strncmp(tab_format[k], "yyyy", 4))
			format = g_strconcat(format, "%Y", NULL);
		else if(!strncmp(tab_format[k], "yy", 2))
			format = g_strconcat(format, "%y", NULL);
		k++;
	}
	g_strfreev(tab_format);
#else
    format = nl_langinfo ( D_FMT );
#endif

    while ( * format )
    {
        if ( * format == '%' )
        {
            switch ( * ++format )
            {
            case 'd': case 'm': case 'y': case 'Y':
                date_tokens [ num_tokens++ ] = *format ;
                if ( num_tokens > 3 )
                {
                    dialogue_error_brain_damage ();
                }
            default:
                break;
            }
        }
        format++;
    }

    /* TODO: Check that m,d,Yy are present. */
    
    /* replace all separators by . */
    g_strcanon ( string, "0123456789", '.' );
    /* remove the . at the beginning and ending of the string */
    while ( * string == '.' && * string ) string ++;
    while ( string [ strlen ( string ) - 1 ] == '.' && strlen ( string ) ) 
        string [ strlen ( string )  - 1 ] = '\0';

    /* remove if there are some .. */
    tab_date = g_strsplit ( string, "..", 0 );
    string = g_strjoinv ( ".", tab_date );
    /* split the parts of the date */
    tab_date = g_strsplit_set ( string, ".", 0 );
	g_free(string);

    num_fields = g_strv_length ( tab_date );

    if ( num_fields == 0 )
        return NULL;
    else if ( num_fields == 1 )
    {
        /* there is only 1 field in the date, try to split the number (ie 01042000 gives 01/04/2000) */
        gchar ** new_tab_date = split_unique_datefield ( tab_date [ 0 ], date_tokens );
        if ( ! new_tab_date )
            return NULL;
        else
        {
            g_strfreev ( tab_date ); 
            tab_date = new_tab_date;
            num_fields = g_strv_length ( tab_date );
        }
    }

    /* Initialize date */
    date = gdate_today ();

    for ( i = 0, j = 0 ; i < num_tokens && j < num_fields ; i ++ )
    {
        int nvalue = atoi ( tab_date [ j ] );
        switch ( date_tokens [ i ] )
        {
            case 'm':
            if ( g_date_valid_month ( nvalue ) )
            {
                g_date_set_month ( date, nvalue );
                j++;
            }
            else
                return NULL;
            break;
            case 'd':
            if ( g_date_valid_day ( nvalue ) )
            {
                g_date_set_day ( date, nvalue );
                j++;
            }
            else
                return NULL;
            break;
            case 'y':
            case 'Y':
            if ( strlen ( tab_date [ j ] ) == 2
                 ||
                 strlen (tab_date[j] ) == 1)
            {
                if ( nvalue < 60 )
                {
                nvalue += 2000;
                }
                else
                {
                nvalue += 1900;
                }
            }
            if ( g_date_valid_year ( nvalue ) && num_fields >= 3 )
            {
                g_date_set_year ( date, nvalue );
                j++;
            }
            else
                return NULL;
            break;
            default:
                g_printerr ( ">> Unknown format '%c'\n", date_tokens [ i ] );
                return NULL;
            break;
        }
    }
    /* comment for random crash. Memory allocation problem in split_unique_datefield () */
    //~ g_strfreev ( tab_date );

    /* need here to check if the date is valid, else an error occurs when
     * write for example only 31, and the current month has only 30 days... */
    if ( !g_date_valid (date) )
        return NULL;
    else
        return date;
}



/**
 * Create and return a GDate from a string locale independant
 * representation of a date.  It expects format %m/%d/%Y (american
 * style).
 *
 * \param
 *
 * \return return a newly allocated string or NULL if the format of the date_string 
 * parameter is invalid.
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
 * \return		A newly allocated string representing date.
 */
gchar *gsb_format_date ( gint day, gint month, gint year )
{
    GDate* date = g_date_new_dmy ( day, month, year );
    gchar* result = gsb_format_gdate ( date );
    g_date_free ( date );
    return result;
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
 * date without locale. It returns an empty string if date is not valid.
 *
 * \param date		A GDate structure containing the date to represent.
 *
 * \return		A newly allocated string representing date.
 */
gchar *gsb_format_gdate_safe ( const GDate *date )
{
    gchar retour_str[SIZEOF_FORMATTED_STRING_DATE];

    if ( !date || !g_date_valid ( date ))
    {
    return g_strdup ( "" );
    }

    g_date_strftime ( retour_str, SIZEOF_FORMATTED_STRING_DATE, "%m/%d/%Y", date );

    return g_strdup ( retour_str );
}


/**
 * retourne la date bufferisée si les deux chaines correspondent 
 * sinon renvoie une date issue de la chaine passée en paramètre
 * 
 * */
GDate *gsb_date_get_last_entry_date ( const gchar *string )
{
    if ( buffer_entry_date && g_strcmp0 ( string , buffer_entry_date -> date_string) == 0 )
        return gsb_date_copy ( buffer_entry_date -> last_entry_date );
    else
        return gsb_parse_date_string ( string );
}


/**
 * retourne la date de compilation conforme à la locale
 * 
 * */
gchar *gsb_date_get_compiled_time ( void )
{
    GDate *date;
    gchar **tab;
    gchar *str;
    const gchar *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug",
                            "Sep", "Oct", "Nov", "Dec"};
    gint mois = 0;
    gint i;

    str = g_strdup ( __DATE__ );
    if ( g_strstr_len ( str, -1, "  " ) )
    {
        tab = g_strsplit ( str, "  ", -1 );
        str = g_strjoinv  (" ", tab);
        g_strfreev (tab);
    }
    tab = g_strsplit ( str, " ", -1 );
    g_free ( str );

    for (i = 0; i < 12; i++)
    {
        if ( !strcmp ( tab[0], months[i] ) )
        {
          mois = i + 1;
          break;
        }
    }

    date = g_date_new_dmy ( atoi ( tab[1] ), mois, atoi ( tab[2] ) );
    g_strfreev (tab);

    return gsb_format_gdate ( date );
}


/**
 * returns a date with the last day of the month.
 * 
 * */
GDate *gsb_date_get_last_day_of_month ( const GDate *date )
{
    GDate *tmp_date;

    tmp_date = gsb_date_copy ( date );
    g_date_set_day ( tmp_date, 1 );
    g_date_add_months ( tmp_date, 1 );
    g_date_subtract_days ( tmp_date, 1 );

    return tmp_date;
}
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
