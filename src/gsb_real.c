/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2007 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)	                      */
/*                      2009-2010 Pierre Biava (grisbi@pierre.biava.name)     */
/*                      2009 Mickaël Remars (grisbi@remars.com)               */
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
 * \file gsb_real.c
 * grisbi use a special structure to describe a real number
 * all is defined her
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"
#include <assert.h>

/*START_INCLUDE*/
#include "gsb_real.h"
/*END_INCLUDE*/

gsb_real null_real = { 0 , 0 };
gsb_real error_real = { G_MININT64, 0 };

static gchar *gsb_thousands_sep;
static gchar *gsb_decimal_point;

glong gsb_real_power_10[] = { 1, 10, 100, 1000, 10000, 100000,
                            1000000, 10000000, 100000000, 1000000000 };

#ifdef _MSC_VER
#define rint(x) (floor(x + 0.5))
#endif /*_MSC_VER */

/*START_STATIC*/
static gchar *gsb_real_add_thousands_sep ( gchar *str_number, const gchar *thousands_sep );
static gsb_real gsb_real_double_to_real_add_exponent ( gdouble number, gint exp_add );
static gboolean gsb_real_grow_exponent( gsb_real *num, guint target_exponent );
static void gsb_real_minimize_exponent ( gsb_real *num );
static void gsb_real_raw_minimize_exponent ( gint64 *mantissa, gint *exponent );
static gboolean gsb_real_raw_truncate_number ( gint64 *mantissa, gint *exponent );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/


/**
 * Return the real in a formatted string, according to the currency 
 * regarding decimal separator, thousands separator and positive or
 * negative sign.
 * this is directly the number coded in the real wich is returned
 * usually, gsb_real_get_string_with_currency is better to adapt the format
 * 	of the number to the currency format
 * 
 * \param number	Number to format.
 *
 * \return		A newly allocated string of the number (this
 *			function will never return NULL) 
*/
gchar *gsb_real_get_string ( gsb_real number )
{
    struct lconv *conv = localeconv ();

    return gsb_real_raw_format_string ( number, conv, FALSE );
}


/**
 * Return the real in a formatted string with an optional currency
 * symbol, according to the given locale regarding decimal separator,
 * thousands separator and positive or negative sign.
 * 
 * \param number		    Number to format.
 * \param conv      		the locale obtained with localeconv(), or built manually
 * \param currency_symbol 	the currency symbol
 *
 * \return		A newly allocated string of the number (this
 *			function will never return NULL) 
 */
gchar *gsb_real_raw_format_string (gsb_real number,
                        struct lconv *conv,
                        const gchar *currency_symbol )
{
    gchar buffer[G_ASCII_DTOSTR_BUF_SIZE];
    gchar format[40];
    gchar *result = NULL, *temp = NULL;
	const gchar *cs_start;
    const gchar *cs_start_space;
    const gchar *sign;
    const gchar *mon_decimal_point;
    const gchar *cs_end_space;
    const gchar *cs_end;
    gint nbre_char;
	lldiv_t units;
/*printf ("currency_symbol = %s\n", currency_symbol);*/
    cs_start = (currency_symbol && conv->p_cs_precedes) ? currency_symbol : "";
    cs_start_space = (currency_symbol && conv->p_cs_precedes && conv->p_sep_by_space) ? " " : "";
    sign = (number.mantissa < 0) ? conv->negative_sign : conv->positive_sign;
    mon_decimal_point = gsb_decimal_point;
    cs_end_space = (currency_symbol && !conv->p_cs_precedes && conv->p_sep_by_space) ? " " : "";
    cs_end = (currency_symbol && !conv->p_cs_precedes) ? currency_symbol : "";
    
    units = lldiv ( llabs (number.mantissa), gsb_real_power_10[number.exponent] );

    nbre_char = g_sprintf ( buffer, "%.0f", (gdouble) units.quot );

    temp = g_strndup ( buffer, nbre_char );

    if ( units.quot >= 1000 )
    {
        temp = gsb_real_add_thousands_sep ( temp, gsb_thousands_sep );
    }

    g_snprintf ( format, sizeof ( format ), "%s%d%s",
                                            "%s%s%s%s%s%0",
                                            number.exponent,
                                            "lld%s%s" );

    result = g_strdup_printf ( format, 
                            cs_start,
                            cs_start_space,
                            sign,
                            temp,
                            mon_decimal_point,
                            units.rem,
                            cs_end_space,
                            cs_end );

    g_free ( temp );

    return result;
}


/**
 * get a real number from a string
 * the string can be formatted :
 * - handle , or . as separator
 * - spaces are ignored
 * - another character makes a 0 return
 *
 *   there is no ask for any exponent, so the gsb_real will be exactly the
 *   same as the string
 *
 * \param string
 *
 * \return the number in the string transformed to gsb_real
 * */
gsb_real gsb_real_get_from_string ( const gchar *string )
{
    gsb_real result;

    result =  gsb_real_raw_get_from_string ( string, gsb_thousands_sep, gsb_decimal_point );

    return result;
}


/**
 * get a real number from an integer
 *
 * \param mantissa
 * \param exponent -1 for no limit
 *
 * \return a gsb_real from the integer
 */
gsb_real gsb_real_new ( gint64 mantissa, gint exponent )
{
    gsb_real number = null_real;

    number.mantissa = mantissa;
    number.exponent = exponent;

    return number;
}


/**
 * get a gsb_real number from a string
 * the string can be formatted :
 * - spaces and the given utf8-encoded thousands separators are ignored
 * - handle ",", "." and the given utf8-encoded decimal separator
 * - another character makes a error_real return
 *
 * \param string
 * \param mon_thousands_sep, can be NULL or empty, but only one utf8 sequence
 * \param mon_decimal_point, can be NULL or empty, but only one utf8 sequence
 *
 * \return the number in the string transformed to gsb_real
 */
gsb_real gsb_real_raw_get_from_string ( const gchar *string,
                                        const gchar *mon_thousands_sep,
                                        const gchar *mon_decimal_point )
{
    static gchar *space_chars;
    static gchar *decimal_chars;
    static const gchar *positive_chars = "+";
    static const gchar *negative_chars = "-";
    static const gchar *decimal_char_dot = ".";
    static const gchar *decimal_char_comma = ",";
    static const gchar *empty_char = "" ;
    const gchar *default_decimal_char_dot = decimal_char_dot;
    const gchar *default_decimal_char_comma = decimal_char_comma;
    unsigned mts_len;
    unsigned mdp_len;
    unsigned nb_digits = 0;
    gint64 mantissa = 0;
    gint8 sign = 0;
    gint8 dot_position = -1;
    const gchar *p = string;
    gboolean success = FALSE;

    if ( !string)
        return error_real;

    mts_len = mon_thousands_sep ? strlen ( mon_thousands_sep ) : 0;
    mdp_len = mon_decimal_point ? strlen ( mon_decimal_point ) : 0;

    if ( mon_thousands_sep )
    {
        if ( g_strstr_len ( mon_thousands_sep, -1, decimal_char_dot ) )
            default_decimal_char_dot = empty_char;
        if ( g_strstr_len ( mon_thousands_sep, -1, decimal_char_comma ) )
            default_decimal_char_comma = empty_char ;
    }

    decimal_chars = g_strconcat(default_decimal_char_dot,
                                default_decimal_char_comma,
                                mon_decimal_point,
                                NULL);
    space_chars = g_strconcat(" ", mon_thousands_sep, NULL);

    for ( ; ; )
    {
        if ( g_ascii_isdigit ( *p ) )
        {
            mantissa *= 10;
            mantissa += ( *p - '0' );
            if ( mantissa > G_MAXINT64 )
            {
                break;
            }
            if ( sign == 0 ) sign = 1; /* no sign found yet ==> positive */
            ++nb_digits;
            ++p;
        }
        else if ( *p == 0 ) /* terminal zero */
        {
            success = TRUE;
            break;
        }
        else if ( decimal_chars && strchr ( decimal_chars, *p ) )
        {
            if ( dot_position >= 0 ) /* already found a decimal separator */
            {
                break;
            }
            dot_position = nb_digits;
            p = g_utf8_find_next_char ( p, NULL );
        }
        else if ( g_utf8_strchr ( space_chars, -1,  g_utf8_get_char( p ) ) )
        {
            /* just skip spaces and thousands separators */
            p = g_utf8_find_next_char ( p, NULL );
        }
        else if ( strchr ( negative_chars, *p ) )
        {
            if ( sign != 0 ) /* sign already set */
            {
                break;
            }
            sign = -1;
            ++p;
        }
        else if ( strchr ( positive_chars, *p ) )
        {
            if ( sign != 0 ) /* sign already set */
            {
                break;
            }
            sign = 1;
            ++p;
        }
        else /* unknown char ==> error */
        {
            break;
        }
    }
    /* Free memory */
    g_free ( decimal_chars );
    g_free ( space_chars );
    if (success == TRUE)
    {
        gsb_real result;
        result.mantissa = sign * mantissa;
        result.exponent = ( dot_position >= 0 )
                          ? nb_digits - dot_position
                          : 0;
        return result;
    }
    else
    {
        return error_real;
    }
}


/**
 * get a gsb_real number from a string, during file load
 * the string can be formatted :
 * - spaces and the given utf8-encoded thousands separators are ignored
 * - handle only "." as a decimal separator
 * - another character makes a error_real return
 *
 * \param string
 * \param mon_thousands_sep, can be NULL or empty, but only one utf8 sequence
 * \param mon_decimal_point, can be NULL or empty, but only one utf8 sequence
 *
 * \return the number in the string transformed to gsb_real
 */
gsb_real gsb_real_safe_real_from_string ( const gchar *string )
{
    unsigned nb_digits = 0;
    gint64 mantissa = 0;
    gint8 sign = 0;
    gint8 dot_position = -1;
    const gchar *p = string;

    if ( !string)
        return error_real;
    if ( g_strstr_len ( string, -1, ERROR_REAL_STRING ) )
        return error_real;

    for ( ; ; )
    {
        if ( g_ascii_isdigit ( *p ) )
        {
            mantissa *= 10;
            mantissa += ( *p - '0' );
            if ( mantissa > G_MAXINT64 )
                return error_real;
            if ( sign == 0 ) sign = 1; /* no sign found yet ==> positive */
            ++nb_digits;
            ++p;
            /* printf ("mantissa = %lld nb_digits = %d\n", mantissa, nb_digits); */
        }
        else if ( *p == 0 ) /* terminal zero */
        {
			gsb_real result;
			result.mantissa = sign * mantissa;
            if ( mantissa == 0 )
                result.exponent = 0;
            else
                result.exponent = ( dot_position >= 0 )
                              ? nb_digits - dot_position
                              : 0;
            /* printf ("result.mantissa = %ld result.exponent = %d\n", result.mantissa, result.exponent); */
            return result;
        }
        else if ( strchr ( ".", *p ) )
        {
            if ( dot_position >= 0 ) /* already found a decimal separator */
                return error_real;
            dot_position = nb_digits;
            p = g_utf8_find_next_char ( p, NULL );
        }
        else if ( strchr ( "-", *p ) )
        {
            if ( sign != 0 ) /* sign already set */
                return error_real;
            sign = -1;
            ++p;
        }
        else /* unknown char ==> error */
        {
            return error_real;
        }
    }
}


/**
 * compare 2 gsb_real and return the result (-1, 0, 1)
 *
 * \param number_1
 * \param number_2
 *
 * \return -1 if number_1 < number_2 ; 0 if number_1 = number_2 ; 1 if number_1 > number_2
 * */
G_MODULE_EXPORT gint gsb_real_cmp ( gsb_real number_1,
                        gsb_real number_2 )
{
    gsb_real_normalize ( &number_1,
			 &number_2 );
    if (number_1.mantissa < number_2.mantissa)
	return -1;
    if (number_1.mantissa == number_2.mantissa)
	return 0;

    return 1;
}

/**
 * reduce the exponent to its smallest possible value,
 * without losing any precision
 *
 * \param mantissa a pointer to the 64 bits mantissa to be reduced
 * \param exponent a pointer to the exponent to be reduced
 **/
void gsb_real_raw_minimize_exponent ( gint64 *mantissa, gint *exponent )
{
    while ( *exponent > 0 )
    {
        lldiv_t d = lldiv ( *mantissa, 10 );
        if ( d.rem != 0 )
            return;
        *mantissa = d.quot;
        --*exponent;
    }
}

/**
 * reduce the exponent to its smallest possible value,
 * without losing any precision
 *
 * \param num a pointer to the number to be reduced
 **/
void gsb_real_minimize_exponent ( gsb_real *num )
{
    gint64 mantissa = num->mantissa;

    gsb_real_raw_minimize_exponent ( &mantissa, &num->exponent);
    num->mantissa = mantissa;
}

/**
 * grow the exponent up to target_exponent
 * (only if possible without losing precision)
 *
 * \param num a pointer to the number
 * \param target_exponent the desired exponent
 **/
gboolean gsb_real_grow_exponent( gsb_real *num, guint target_exponent )
{
    gint64 mantissa = num->mantissa;
    gint exponent = num->exponent;
    gboolean succes = TRUE;

    while ( exponent < target_exponent )
    {
        gint64 new_mantissa;

        new_mantissa = mantissa * 10;
        if ( ( new_mantissa > G_MAXINT64 ) || ( new_mantissa < G_MININT64 ) )
        {
            succes = FALSE;
            break;
        }
        mantissa = new_mantissa;
        ++exponent;
    }
    num->mantissa = mantissa;
    num->exponent = exponent;

    return succes;
}

/**
 * normalize the 2 numbers to be able to add/substract/compare them later
 * for that transform the 2 numbers to have the same exponent
 * and after that we can work on the mantissa
 *
 * \param number_1 a pointer to gsb_real wich contains the number_1 to transform
 * \param number_2 a pointer to gsb_real wich contains the number_2 to transform
 *
 * \return TRUE if normalization occured without loss of precision
 * FALSE if exponents can't be the same without loss of precision
 * */
gboolean gsb_real_normalize ( gsb_real *number_1, gsb_real *number_2 )
{
    gboolean safe_precision = TRUE;

    gsb_real_minimize_exponent ( number_1 );
    gsb_real_minimize_exponent ( number_2 );

    if ( number_1->exponent < number_2->exponent )
    {
        safe_precision = gsb_real_grow_exponent ( number_1,
                                                  number_2->exponent );
        if ( !safe_precision )
		{
			while ( number_2 -> exponent > number_1 -> exponent )
            {
                number_2 -> exponent--;
                number_2 -> mantissa = number_2 -> mantissa / 10;
            }
		}
    }
    else if ( number_2->exponent < number_1->exponent )
    {
        safe_precision = gsb_real_grow_exponent ( number_2,
                                                  number_1->exponent );
        if ( !safe_precision )
		{
            while ( number_1 -> exponent > number_2 -> exponent )
            {
                number_1 -> exponent--;
                number_1 -> mantissa = number_1 -> mantissa / 10;
            }
		}
    }

    return safe_precision;
}


/**
 * give the absolute value of the number
 *
 * \param number
 *
 * \return a gsb_real, the absolute value of the given number
 * */
gsb_real gsb_real_abs ( gsb_real number )
{
    number.mantissa = llabs (number.mantissa);
    return number;
}


/**
 * modify the number to adjust the exponent wanted
 *
 * \param number
 * \param return_exponent the exponent we want to have for the returned number, or -1 if no limit
 *
 * \return the transformed number
 * */
gsb_real gsb_real_adjust_exponent ( gsb_real number,
                        gint return_exponent )
{
    gint exponent;

    if (return_exponent == -1)
        return number;

    if ( number.exponent == return_exponent )
        return number;

/*    printf ("\nnumber.mantissa = %"G_GINT64_MODIFIER"d number.exponent = %d return_exponent = %d\n",
                number.mantissa, number.exponent, return_exponent);
*/
    exponent = abs ( number.exponent - return_exponent );
/*
    printf ("exponent = %d gsb_real_power_10[exponent] = %ld\n", exponent, gsb_real_power_10[exponent] );
*/
	if ( number.exponent < return_exponent )
	{
        number.mantissa = number.mantissa * gsb_real_power_10[exponent];
        number.exponent = return_exponent;
	}
	else
	{
        lldiv_t tmp_num;
        gint sign;

        sign = ( number.mantissa < 0 ) ? -1 : 1;

        tmp_num = lldiv ( llabs ( number.mantissa ), gsb_real_power_10[exponent] );
        
        if ( tmp_num.rem > ( 0.5 * gsb_real_power_10[exponent] ) )
            number.mantissa = ( tmp_num.quot + 1 ) * sign;
        else
            number.mantissa = tmp_num.quot * sign;

        number.exponent = return_exponent;
    }
/*
    printf ("number.mantissa = %"G_GINT64_MODIFIER"d number.exponent = %d\n",
                number.mantissa, number.exponent);
*/
    return number;
}



/**
 * add 2 gsb_real
 * when an overflow occurs, error_real is returned
 *
 * \param number_1
 * \param number_2
 *
 * \return a gsb_real = number_1 + number_2, or error_real when an error occured
 * */
G_MODULE_EXPORT gsb_real gsb_real_add ( gsb_real number_1,
                        gsb_real number_2 )
{
    gint64 mantissa;

    if ( ( number_1.mantissa == error_real.mantissa )
      || ( number_2.mantissa == error_real.mantissa )
      || !gsb_real_normalize ( &number_1, &number_2 ) )
        return error_real;

	mantissa = ( gint64 ) number_1.mantissa + number_2.mantissa;
    if ( ( mantissa > G_MAXINT64 ) || ( mantissa < G_MININT64 ) )
    {
        if ( ! gsb_real_raw_truncate_number (&mantissa, &number_1.exponent ) )
            return error_real;
    }
    number_1.mantissa = mantissa;

    return number_1;
}

/**
 * substract between 2 gsb_real : number_1 - number_2
 *
 * \param number_1
 * \param number_2
 *
 * \return a gsb_real = number_1 - number_2
 * */
gsb_real gsb_real_sub ( gsb_real number_1,
                        gsb_real number_2 )
{
    number_2.mantissa = -number_2.mantissa;
    return gsb_real_add ( number_1, number_2 );
}


/**
 * return the opposite of the number
 * ie 5 returns -5 in gsb_real number
 *
 * \param number
 *
 * \return its opposite
 * */
G_MODULE_EXPORT gsb_real gsb_real_opposite ( gsb_real number )
{
    number.mantissa = -number.mantissa;
    return number;
}


/**
 * multiply 2 gsb_reals. no overflow possible
 *
 * \param number_1
 * \param number_2
 *
 * \return the multiplication between the 2
 * */
gsb_real gsb_real_mul ( gsb_real number_1,
                        gsb_real number_2 )
{
    gint64 mantissa;

    if ( number_1.mantissa == error_real.mantissa
         || number_2.mantissa == error_real.mantissa)
    {
        return error_real;
    }

    mantissa = ( gint64 ) number_1.mantissa * number_2.mantissa;
    number_1.exponent += number_2.exponent;

    gsb_real_raw_minimize_exponent ( &mantissa, &number_1.exponent );

    if ( ( mantissa > G_MAXINT64 ) || ( mantissa < G_MININT64 ) )
    {
        if ( ! gsb_real_raw_truncate_number (&mantissa, &number_1.exponent ) )
            return error_real;
    }
    number_1.mantissa = mantissa;

    return number_1;
}


/**
 * divide 2 gsb_reals
 *
 * \param number_1
 * \param number_2
 *
 * \return the div between the 2
 * */
gsb_real gsb_real_div ( gsb_real number_1,
                        gsb_real number_2 )
{
    gsb_real number;
	gint64 reste;
	
	if ( number_1.mantissa == error_real.mantissa ||
	     number_2.mantissa == error_real.mantissa ||
	     !number_2.mantissa )
		return error_real;
	
	reste = number_1.mantissa % number_2.mantissa;

	if( ( number_1.mantissa >= number_2.mantissa ) && !reste )
	{
		number.mantissa = number_1.mantissa / number_2.mantissa;
		number.exponent = number_1.exponent - number_2.exponent;
		while (number.exponent < 0)
		{
			number.mantissa *= 10;
			number.exponent ++ ;
		}
	}
	else
	{
		number = gsb_real_double_to_real_add_exponent (
		                      (gdouble) number_1.mantissa / (gdouble) number_2.mantissa,
		                       number_1.exponent - number_2.exponent);
	}
    return number;
}


/**
 * convert a double to a gsb_real
 *
 * \param number a gdouble number
 *
 * \return the number in gsb_real format
 * */
G_MODULE_EXPORT gsb_real gsb_real_double_to_real ( gdouble number )
{
	return gsb_real_double_to_real_add_exponent(number, 0);
}

gsb_real gsb_real_double_to_real_add_exponent ( gdouble number, gint exp_add )
{
    gdouble tmp_double, decimal;
    gdouble maxlong;
	gsb_real real_number = {0, exp_add};

    maxlong = G_MAXINT64 / 10;
/*     printf ("number initial = %f exp_add = %d\n",number, exp_add);  */

	if(exp_add >=9)
		return null_real;

	while ( ( ( decimal = modf ( number, &tmp_double ) ) ||
		real_number.exponent < 0 ) &&
		real_number.exponent < 9)
    {
        number = number * 10;
        real_number.exponent++;

        if (fabs (number) > maxlong)
            number = rint (number);
    }
	decimal = modf ( number, &tmp_double );
/*     printf ("number = %f decimal = %f tmp_double = %f\n", number, decimal, tmp_double);  */

	if ( ( ( real_number.exponent == ( 9 - exp_add ) ) ) && ( fabs ( decimal ) >= 0.5 ) )
    {
        if ( tmp_double < 0 )
		    real_number.mantissa = ((gint64) tmp_double ) - 1;
        else
            real_number.mantissa = ((gint64) tmp_double ) + 1;

        gsb_real_minimize_exponent ( &real_number );
    }
	else
        real_number.mantissa = (gint64) ( tmp_double );
/*     printf ("real_number.mantissa = %lld real_number.exponent = %d\n", real_number.mantissa,real_number.exponent);  */

    return real_number;
}


/**
 * truncate the number. WARNING there loss of accuracy
 *
 * \param mantissa a pointer to the 64 bits mantissa to be truncate
 * \param exponent a pointer to the exponent to be truncate
 **/
gboolean gsb_real_raw_truncate_number ( gint64 *mantissa, gint *exponent )
{
    gint64 new_mantissa = *mantissa;
    gint new_exponent = *exponent;

    if ( new_mantissa > G_MAXINT64 )
    {
        do
        {
            --new_exponent;
            new_mantissa = new_mantissa / 10;
        } while ( new_mantissa > G_MAXINT64 );
    }
    else if ( new_mantissa < G_MININT64 )
    {
        do
        {
            --new_exponent;
            new_mantissa = new_mantissa / 10;
        } while ( new_mantissa < G_MININT64 );
    }
    else
    {
        return FALSE;
    }

    /* exponent must be greater or equal to 0 */
    if (new_exponent < 0)
    {
        return FALSE;
    }
    else
    {
        *exponent = new_exponent;
        *mantissa = new_mantissa;
        return TRUE;
    }
}


/**
 * retourne une chaine représentative d'un nombre avec le point comme séparateur décimal
 * et pas de separateur de milliers
 *
 * The returned string should be freed with g_free() when no longer needed.
 * */
gchar *gsb_real_safe_real_to_string ( gsb_real number, gint default_exponent )
{
    gchar buffer[G_ASCII_DTOSTR_BUF_SIZE];
    gchar format[40];
    gchar *result = NULL;
    const gchar *sign;
    const gchar *mon_decimal_point;
    gint nbre_char;
    lldiv_t units;

    if ( (number.exponent < 0)
    || (number.exponent > EXPONENT_MAX )
    || (number.mantissa == error_real.mantissa) )
      return g_strdup ( ERROR_REAL_STRING );

    if (number.mantissa == 0)
        return g_strdup ("0.00");

    if ( default_exponent != -1 )
        number = gsb_real_adjust_exponent ( number, default_exponent );

    sign = (number.mantissa < 0) ? "-" : "";
    mon_decimal_point = ".";

    units = lldiv ( llabs (number.mantissa), gsb_real_power_10[number.exponent] );

    nbre_char = g_sprintf ( buffer, "%.0f", (gdouble) units.quot );

    result = g_strndup ( buffer, nbre_char );

    g_snprintf ( format, sizeof ( format ), "%s%d%s",
                                            "%s%s%s%0",
                                            number.exponent,
                                            "lld" );

    result = g_strdup_printf ( format, 
                            sign,
                            result,
                            mon_decimal_point,
                            units.rem );

/*     printf ("number.mantissa = %lld number.exponent = %d résultat = %s\n",
 *                         number.mantissa, number.exponent, result );
 */
    return result;
}


gdouble gsb_real_real_to_double ( gsb_real number )
{
    gdouble result;

    result = number.mantissa;

    while ( number.exponent > 0 )
    {
        result /= 10;
        number.exponent--;
    }

    return result;
}


/**
 * ajoute le séparateur des milliers passé en paramètre
 *
 * \param string to modify WARNING string is free
 *
 * \return a new allocated sring
 * */
gchar *gsb_real_add_thousands_sep ( gchar *str_number, const gchar *thousands_sep )
{
    gchar *mon_thousands_sep;
    gchar *result = NULL;
    gchar *ptr;
    gchar *dest;
    gchar *tmp_ptr;
    gchar **tab_str = NULL;
    gint nbre_char;
    gint i = 0;
    gint j = 0;
    gint sep = 0;
    gint longueur;

    if ( thousands_sep == NULL )
        return str_number;

    nbre_char = strlen ( str_number );
    str_number = g_strreverse ( str_number );
    ptr = str_number;

    if ( ( longueur = strlen ( thousands_sep ) ) == 1 )
        mon_thousands_sep = g_strndup ( thousands_sep, 1 );
    else
        mon_thousands_sep = g_strndup ( "&", 1 );

    dest = g_malloc0 ( 128 * sizeof ( gchar ) );
    tmp_ptr = dest;

    while ( i < nbre_char )
    {
        dest[i+sep] = ptr[0];

        ptr++;
        tmp_ptr++;
        i++;
        j++;
        if ( i < nbre_char && j == 3 )
        {
            tmp_ptr = g_stpcpy ( tmp_ptr, mon_thousands_sep );
            j = 0;
            sep++;
        }
    };

    result = g_strndup ( dest, nbre_char + sep );
    result = g_strreverse ( result );

    /* on met le bon séparateur si necessaire */
    if ( longueur > 1 )
    {
        tab_str = g_strsplit ( result, "&", 0 );
        g_free ( result );

        result = g_strjoinv ( thousands_sep, tab_str );
        g_strfreev ( tab_str );
    }

    g_free ( str_number );
    g_free ( dest );

    return result;
}


/**
 *
 * The returned string should be freed with g_free() when no longer needed.
 *
 *
 * */
gchar *gsb_real_get_decimal_point ( void )
{
    return g_strdup ( gsb_decimal_point );
}


/**
 *
 *
 *
 *
 * */
void gsb_real_set_decimal_point ( const gchar *decimal_point )
{
    if ( gsb_decimal_point && strlen ( gsb_decimal_point ) )
        g_free ( gsb_decimal_point );

    if ( decimal_point == NULL )
        gsb_decimal_point = NULL;
    else
		gsb_decimal_point = g_strdup ( decimal_point );
}


/**
 *
 * The returned string should be freed with g_free() when no longer needed.
 *
 *
 * */
gchar *gsb_real_get_thousands_sep ( void )
{
    return g_strdup ( gsb_thousands_sep );
}


/**
 *
 *
 *
 *
 * */
void gsb_real_set_thousands_sep ( const gchar *thousands_sep )
{
    if ( gsb_thousands_sep && strlen ( gsb_thousands_sep ) )
        g_free ( gsb_thousands_sep );

    if ( thousands_sep == NULL )
        gsb_thousands_sep = NULL;
    else
        gsb_thousands_sep = g_strdup ( thousands_sep );
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */

