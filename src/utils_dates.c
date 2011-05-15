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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"
#if defined(_MSC_VER) || defined (_MINGW)
#include <winnls.h>
#else
#include <langinfo.h>
#endif /*_MSC_VER */

/*START_INCLUDE*/
#include "utils_dates.h"
#include "dialog.h"
#include "gsb_calendar_entry.h"
#include "gsb_form_widget.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gchar **split_unique_datefield ( gchar * string, gchar **date_tokens );
/*END_STATIC*/


/*START_EXTERN*/
/*END_EXTERN*/

/* format pour les dates */
static gchar *format = NULL;

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
        gchar *date_string;

        date = gdate_today ( );
        date_string = gsb_format_gdate ( date );
        gsb_date_set_last_date ( date_string );
        g_free ( date_string );
        g_date_free ( date );
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
 * return the tomorrow date in the gdate format
 *
 * \param
 *
 * \return a newly allocated GDate which represents the date of the day. Use g_date_free to free memory when no more used.
 * */
GDate *gsb_date_tomorrow ( void )
{
    GDate *date;

    date = gdate_today ( );
    g_date_add_days ( date, 1);

    return ( date );
}


/**
 * adds one month to a date
 *
 * \param date
 * \param free the init date
 *
 * \return a newly allocated GDate which represents the date of the day. 
 * Use g_date_free to free memory when no more used.
 * */
GDate *gsb_date_add_one_month ( GDate *date, gboolean free )
{
    GDate *new_date;

    new_date = gsb_date_copy ( date );
    g_date_add_months ( new_date, 1);

    if ( free )
        g_date_free ( date );

    return ( new_date );
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
gchar **split_unique_datefield ( gchar *string, gchar **date_tokens )
{
    gchar **return_tab;
    gint size;
    gchar *max;
    gint i = 0;
    gint num_fields = 0;

    num_fields = g_strv_length ( date_tokens );
    return_tab = g_new ( gchar *, num_fields + 1 );
    size = strlen ( string );
    max = string + size;

    /* size 1 could be used for example if we write 1 to set 01/mm/yyyy */
    if ( size != 1 && size != 2 && size != 4 && size != 6 && size != 8 )
    {
        return NULL;
    }

    for ( i = 0 ; date_tokens [ i ] && string < max; i ++ )
    {
        if ( size != 8 || strcmp ( date_tokens [ i ], "Y" ) != 0 )
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
    gchar *string;
    gchar *string_ptr;
    gchar **tab_date;
    gchar **date_tokens;
    gint num_tokens = 3;
    gint num_fields = 0;
    gint i;
    gint j;

    if ( !date_string || !strlen ( date_string ) )
        return NULL;

    /* Keep the const gchar in that function */
    string = g_strdup ( date_string );

    /* And keep a pointer to free memory later */
    string_ptr = string;

    /* récupère le format des champs date */
    date_tokens = g_strsplit ( format + 1, "/%", 3 );

    /* delete space char */
    g_strstrip ( string );

    /* replace all separators by . */
    g_strcanon ( string, "0123456789", '.' );

    /* remove the . at the beginning and ending of the string */
    while ( * string == '.' && * string ) string ++;
    while ( string [ strlen ( string ) - 1 ] == '.' && strlen ( string ) ) 
        string [ strlen ( string )  - 1 ] = '\0';

    /* remove if there are some .. */
    tab_date = g_strsplit ( string, "..", 0 );
    g_free ( string_ptr );
    string = g_strjoinv ( ".", tab_date );
    string_ptr = string;
    g_strfreev ( tab_date );

    /* split the parts of the date */
    tab_date = g_strsplit_set ( string, ".", 0 );

    /* From here, string is no more used */
    g_free ( string_ptr );
    string = string_ptr = NULL;

    num_fields = g_strv_length ( tab_date );

    if ( num_fields == 0 )
    {
        g_strfreev ( tab_date );
        return NULL;
    }
    else if ( num_fields == 1 )
    {
        /* there is only 1 field in the date, try to split the number
         * (ie 01042000 gives 01/04/2000) */
        gchar **new_tab_date;

        new_tab_date = split_unique_datefield ( tab_date [0], date_tokens );

        g_strfreev ( tab_date );

        if ( ! new_tab_date )
            return NULL;
        else
        {
            tab_date = new_tab_date;
            num_fields = g_strv_length ( tab_date );
        }
    }

    /* Initialize date */
    date = gdate_today ();

    for ( i = 0, j = 0 ; i < num_tokens && j < num_fields ; i ++ )
    {
        gint nvalue = atoi ( tab_date [ j ] );

        switch ( date_tokens [ i ][0] )
        {
            case 'm':
            if ( g_date_valid_month ( nvalue ) )
            {
                g_date_set_month ( date, nvalue );
                j++;
            }
            else
            {
                g_date_free ( date );
                g_strfreev ( tab_date );
                return NULL;
            }
            break;
            case 'd':
            if ( g_date_valid_day ( nvalue ) )
            {
                g_date_set_day ( date, nvalue );
                j++;
            }
            else
            {
                g_date_free ( date );
                g_strfreev ( tab_date );
                return NULL;
            }
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
            {
                g_date_free ( date );
                g_strfreev ( tab_date );
                return NULL;
            }
            break;
            default:
                g_printerr ( ">> Unknown format '%s'\n", date_tokens [ i ] );
                g_date_free ( date );
                g_strfreev ( tab_date );
                return NULL;
            break;
        }
    }
    /* comment for random crash. Memory allocation problem in split_unique_datefield () */
    g_strfreev ( tab_date );

    /* need here to check if the date is valid, else an error occurs when
     * write for example only 31, and the current month has only 30 days... */
    if ( !g_date_valid ( date ) )
    {
        g_date_free ( date );
        return NULL;
    }
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
    GDate* date;
    gchar* result;

    date = g_date_new_dmy ( day, month, year );
    result = gsb_format_gdate ( date );
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
    guint longueur;

    if ( !date || !g_date_valid ( date ) )
        return my_strdup ( "" );

    longueur = g_date_strftime ( retour_str, SIZEOF_FORMATTED_STRING_DATE, format, date );

    if ( longueur == 0 )
        return my_strdup ( "" );
    else
        return g_strndup ( retour_str, longueur );
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
    guint longueur;

    if ( !date || !g_date_valid ( date ) )
    {
        return g_strdup ( "" );
    }

    longueur = g_date_strftime ( retour_str, SIZEOF_FORMATTED_STRING_DATE, "%m/%d/%Y", date );

    if ( longueur == 0 )
        return NULL;
    else
        return g_strndup ( retour_str, longueur );
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
    str = gsb_format_gdate ( date );
    g_date_free ( date );

    return str;
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


/**
 * returns the format of date.
 * The returned string should be freed with g_free() when no longer needed.
 *
 * \return %d/%m/%Y or %m/%d/%Y 
 * */
gchar *gsb_date_get_format_date ( void )
{
    return g_strdup ( format );
}


/**
 * set the format of date.
 * 
 * */
void gsb_date_set_format_date ( const gchar *format_date )
{
    if ( format && strlen ( format ) )
        g_free ( format );

    if ( format_date == NULL )
        format = NULL;
    else if ( strcmp ( format_date, "%d/%m/%Y" ) == 0 )
        format = g_strdup ( "%d/%m/%Y" );
    else if ( strcmp ( format_date, "%m/%d/%Y" ) == 0 )
        format = g_strdup ( "%m/%d/%Y" );
    else
        format = NULL;

    if ( last_date )
        g_free ( last_date );
    last_date = NULL;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
