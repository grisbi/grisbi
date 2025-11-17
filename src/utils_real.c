/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)         2011 Grisbi Development Team                     */
/*             https://www.grisbi.org/                                         */
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

#include "config.h"

#include "include.h"
#include <string.h>

/*START_INCLUDE*/
#include "utils_real.h"
#include "gsb_data_currency.h"
#include "gsb_locale.h"
#include "utils_str.h"
/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/


/**
 * Return the real in a formatted string, according to the currency
 * regarding decimal separator, thousands separator and positive or
 * negative sign.
 * this is directly the number coded in the real which is returned
 * usually, utils_real_get_string_with_currency is better to adapt the format
 * 	of the number to the currency format
 *
 * \param number	Number to format.
 *
 * \return		A newly allocated string of the number (this
 *			function will never return NULL)
*/
gchar *utils_real_get_string ( GsbReal number )
{
    struct lconv *locale = gsb_locale_get_locale ();

    return gsb_real_raw_format_string ( number, locale, NULL );
}

/**
 * Return the real in a formatted string, according to the currency
 * or negative sign and
 * with "." as decimal separator and "," as thousands separator.
 *
 * This is used to export values in an international format so it
 * can be imported in another application.
 *
 * this is directly the number coded in the real which is returned
 * usually, utils_real_get_string_with_currency is better to adapt the format
 * 	of the number to the currency format
 *
 * \param number	Number to format.
 *
 * \return		A newly allocated string of the number (this
 *			function will never return NULL)
 */
gchar *utils_real_get_string_intl ( GsbReal number )
{
	struct lconv locale = *gsb_locale_get_locale ();
	locale.mon_decimal_point = (gchar*)".";
	locale.mon_thousands_sep = (gchar*)",";

	gchar *a = gsb_real_raw_format_string ( number, &locale, NULL );
	return a;
}

/**
 * Return the real in a formatted string with an optional currency
 * symbol, according to the locale regarding decimal separator,
 * thousands separator and positive or negative sign.
 *
 * \param number		Number to format.
 * \param currency_number 	the currency we want to adapt the number, 0 for no adaptation
 * \param show_symbol 		TRUE to add the currency symbol in the string
 *
 * \return		A newly allocated string of the number (this
 *			function will never return NULL)
 */
gchar *utils_real_get_string_with_currency ( GsbReal number,
                        gint currency_number,
                        gboolean show_symbol )
{
    struct lconv *locale = gsb_locale_get_locale ( );
    gint floating_point;

    const gchar *currency_symbol = (currency_number && show_symbol)
                                   ? gsb_data_currency_get_nickname_or_code_iso (currency_number)
                                   : NULL;

    /* First of all if number = 0 I return 0 with the symbol of the currency if necessary */
    if (number.mantissa == 0)
    {
		gchar *zero_str;
		gchar *str_to_free;
		gchar *tmp_str;

		str_to_free = gsb_locale_get_mon_decimal_point ();
		zero_str = g_strconcat ("0", str_to_free, "00", NULL);
		g_free (str_to_free);

        if (currency_symbol && locale -> p_cs_precedes)
            tmp_str = g_strdup_printf ( "%s %s", currency_symbol, zero_str);
        else if (currency_symbol && ! locale -> p_cs_precedes)
            tmp_str = g_strdup_printf ( "%s %s", zero_str, currency_symbol );
        else
            tmp_str = g_strdup (zero_str);
		g_free (zero_str);

		return tmp_str;
    }
    else if ( (number.exponent < 0)
    || (number.exponent > EXPONENT_MAX )
    || (number.mantissa == error_real.mantissa) )
        return g_strdup ( ERROR_REAL_STRING );

    /* first we need to adapt the exponent to the currency */
    /* if the exponent of the real is not the same of the currency, need to adapt it */
    floating_point = gsb_data_currency_get_floating_point ( currency_number );
    if ( currency_number && number.exponent != floating_point )
        number = gsb_real_adjust_exponent ( number, floating_point );

    return gsb_real_raw_format_string ( number, locale, currency_symbol );
}


/**
 * get a real number from a string
 * the string can be formatted :
 * - handle , or . as separator
 * - spaces are ignored
 * - another character makes a 0 return
 *
 *   there is no ask for any exponent, so the GsbReal will be exactly the
 *   same as the string
 *
 * \param string
 *
 * \return the number in the string transformed to gsb_real
 * */
GsbReal utils_real_get_from_string ( const gchar *string )
{
    GsbReal result;
    gchar *thousands_sep = gsb_locale_get_mon_thousands_sep ( );
    gchar *decimal_point = gsb_locale_get_mon_decimal_point ( );

    result =  gsb_real_raw_get_from_string ( string, thousands_sep, decimal_point );

    g_free ( decimal_point );
    g_free ( thousands_sep );

    return result;
}


/**
 * Retourne le gdouble formaté comme dans la fonction source.
 * Le symbole de la monnaie est présent par défaut.
 *
 * \param number		Number to format.
 * \param account_number
 *
 * \return		A newly allocated string of the number
 * */
gchar *utils_real_get_string_with_currency_from_double ( gdouble number,
                        gint currency_number )
{
    GsbReal real;

    real = gsb_real_double_to_real ( number );

    return utils_real_get_string_with_currency ( real, currency_number, TRUE );
}


/**
 * calcule le total du contenu de l'entrée donnée en argument
 * accepte les + et les - * /
 *
 * \param entry         entrée d'un montant
 *
 * \return GsbReal     total de ce qui est dans l'entrée
 */
GsbReal utils_real_get_calculate_entry ( GtkWidget *entry )
{
    gchar *string;
    gchar *pointeur;
    GsbReal total = null_real;

    string = my_strdup ( gtk_entry_get_text ( GTK_ENTRY ( entry ) ) );

    if ( string && strlen ( string ) )
        pointeur = string + strlen ( string );
    else
    {
        if ( string )
            g_free ( string );
        return total;
    }

    if ( g_utf8_strchr ( string, -1, '-' ) || g_utf8_strchr ( string, -1, '+' ) )
    {
        while ( pointeur != string )
        {
            if ( pointeur[0] == '+'
                 ||
                 pointeur[0] == '-' )
            {
                total = gsb_real_add ( total,
                            utils_real_get_from_string ( pointeur ) );
                pointeur[0] = 0;
            }

            pointeur--;
        }
        total = gsb_real_add ( total,
                        utils_real_get_from_string ( pointeur ) );
    }
    else if ( g_utf8_strchr ( string, -1, '*' ) )
    {
        total.mantissa = 1;
        total.exponent = 0;

        while ( pointeur != string )
        {
            if ( pointeur[0] == '*' )
            {
                total = gsb_real_mul ( total,
                            utils_real_get_from_string ( pointeur + 1 ) );
                pointeur[0] = 0;
            }

            pointeur--;
        }
        total = gsb_real_mul ( total,
                        utils_real_get_from_string ( pointeur ) );
    }
    else if ( g_utf8_strchr ( string, -1, '/' ) )
    {
        gchar **tab;

        tab = g_strsplit ( string, "/", 2 );

        total = gsb_real_div ( utils_real_get_from_string ( tab[0] ),
                        utils_real_get_from_string ( tab[1] ) );

        g_strfreev ( tab );
    }
    else
        total = utils_real_get_from_string ( string );

    g_free ( string );

    return total;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
