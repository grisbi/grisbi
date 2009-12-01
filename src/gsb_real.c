/* ************************************************************************** */
/*                                                                            */
/*                                  gsb_real                                  */
/*                                                                            */
/*     Copyright (C)    2000-2007 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)	                      */
/*                      2009 Pierre Biava (grisbi@pierre.biava.name)          */
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


#include "include.h"

/*START_INCLUDE*/
#include "gsb_real.h"
#include "./gsb_data_currency.h"
/*END_INCLUDE*/

 
#ifdef _MSC_VER
typedef struct _lldiv_t
{
        long long       quot;
        long long       rem;
} lldiv_t;

lldiv_t lldiv(long long numerator, long long denominator)
{
        //TODO find a standard/efficient impl for this
        lldiv_t result;
        result.quot = numerator / denominator;
        result.rem = numerator % denominator;
        return result;
}
#define lrint(x) (floor(x + 0.5))
#define rint(x) (floor(x + 0.5))
#define strchr wcschr
#endif//_MSC_VER


/*START_STATIC*/
static const frac error_frac = {0, 0};
static const frac null_frac = {0, 1};

static gchar *gsb_real_format_string ( gsb_real number,
                        gint currency_number,
                        gboolean show_symbol );
static gsb_real gsb_real_get_from_string_normalized ( const gchar *string, gint default_exponent );
//static gboolean gsb_real_grow_exponent( gsb_real *num, guint target_exponent );
//static void gsb_real_minimize_exponent ( gsb_real *num );
//static void gsb_real_raw_minimize_exponent ( gint64 *mantissa, gint *exponent );
//static gboolean gsb_real_raw_truncate_number ( gint64 *mantissa, gint *exponent );
//static gdouble gsb_real_real_to_double ( gsb_real number );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

#ifndef exp10
gint64 exp10(gint n){
	gint64 rez = 1;
	while (n>0 && n <= 20){
		rez *= 10;
		n--;
	}
	return rez;
}
#endif

/* START FRAC FUNCTIONS */

gint64 pgcd (gint64 a, gint64 b){
	gint64 c, d, r;
	c=( ABS(a) > ABS(b) ? a : b );
	d=( ABS(a) > ABS(b) ? b : a );
	if(!d)
		return 0;
	r=c%d;
	while(r){
		c = d;
		d = r;
		r = c%d;
	}
	if(ABS(d)<G_MAXINT64)
		return ABS(d);
	else
		return pgcd(d, G_MAXINT32);
}

frac simplify_64 (gint64 num, gint64 denom){
	gint64 numi;
	gint64 denomi;
	frac out;
	gint64 gcd = 0;
	if(!denom)
		return error_frac;
	if(!num)
		return null_frac;
	gcd = pgcd(num, denom);
	if( gcd == 0)
		return error_frac;
	numi = (gint64) (num/gcd);
	if( numi == 0)
		return null_frac;
	denomi = denom/gcd;
	if (denomi <0){
		numi *= -1;
		denomi *= -1;
	}
	while ( ABS (numi) >= G_MAXINT32 ||
		    denomi >= G_MAXINT32 )
	{
		if ( ABS(numi) == 1)
			return null_frac;
		gcd = MIN(ABS(numi), 10);
		numi /= gcd;
		denomi /= gcd ;
	}
	out.num = numi;
	out.denom = denomi;
	return out;
}

frac gsb_real_frac_sum(frac f1, frac f2){
	gint64 fac1, fac2, gcd;
	if(f1.denom == 0 || f2.denom == 0)
		return error_frac;
	gcd = pgcd(f1.num * (gint64)f2.denom, (gint64)f1.denom);
	fac1 = MAX(f2.denom, gcd);
	gcd = pgcd(f2.num * (gint64)f1.denom, (gint64)f2.denom);
	fac2 = MAX((gint64)f1.denom, gcd);
	gcd = pgcd(fac1, fac2);
	if(gcd == 0)
		return null_frac;
	return simplify_64(f1.num * (gint64)f2.denom / gcd + f2.num * (gint64)f1.denom /gcd,
	                   (gint64)f1.denom * (gint64)f2.denom /gcd);
}

gint gsb_real_frac_cmp (frac f1, frac f2){
	gint n;
	f2.num *= (-1);
	n = (gsb_real_frac_sum (f1, f2)).num;
	return INT_SIGN(n);
}

gint32 frac_get_decimals ( frac f, gint number){
	gint64 prod = ABS (f.num);
	gint32 decimals;
	if( number < 0 )
		return 0;
	while (number){
		prod *= 10;
		number--;
	}
	decimals = (gint32) ( prod/ (gint64) ABS(f.denom));
	if((gdouble)(prod % (gint64) ABS(f.denom) >= 0.5))
		decimals++;
	if(number == 0 && decimals !=0){
		while (decimals % 10 == 0){
			decimals /= 10;
		}
	}
	return decimals;
}

gboolean gsb_real_frac_equal(frac f1, frac f2){
	return ((gboolean) ((!f1.denom && !f2.denom ) ||
	                    (!f1.num && !f2.num && f1.denom && f2.denom) ||
	                    (f1.num == f2.num && f1.denom == f2.denom)));
}

gsb_real frac_to_gsb_real_64 ( gint64 num, gint64 denom){
	gsb_real r;
	frac f = simplify_64 (num, denom);
	if (gsb_real_frac_equal(f, error_frac))
		return error_real;
	r.ent = f.num/f.denom;
	r.f.num = f.num%f.denom;
	r.f.denom = f.denom;
	return r;
}

gsb_real frac_to_gsb_real (frac f){
	return frac_to_gsb_real_64 (f.num, f.denom);
}

gsb_real gsb_real_norm( gsb_real * r){
	gint32 sign;
	if ( INT_SIGN(r->ent) * INT_SIGN(r->f.num) >= 0 )
		return *r;
	else{
		sign = INT_SIGN(r->ent);
		r->ent -= sign;
		r->f.num += sign * r->f.denom;
		return *r;
	}
}


/* END FRAC FUNCTIONS */




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
gint gsb_real_sign (gsb_real r){
	if (r.ent > 0)
		return 1;
	else if (r.ent < 0)
		return -1;
	else
		return INT_SIGN (r.f.num);
	//return ((!r.ent) ? INT_SIGN (r.ent) : INT_SIGN (r.f.num));
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
                        const gchar *currency_symbol,
                        gint floating_point )
{
    gchar format[40];
    gchar *result = NULL;
	const gchar *cs_start;
    const gchar *cs_start_space;
    const gchar *sign;
    const gchar *mon_decimal_point;
    const gchar *cs_end_space;
    const gchar *cs_end;
	gint32 ent, decimals;

	if ( !GSB_REAL_VALID (number) )
      return g_strdup ( "###ERR###" );

    cs_start = (currency_symbol && conv->p_cs_precedes) ? currency_symbol : "";
    cs_start_space = (currency_symbol && conv->p_cs_precedes && conv->p_sep_by_space) ? " " : "";
    sign = GSB_REAL_SIGN (number) < 0 ? conv->negative_sign : conv->positive_sign;
    mon_decimal_point = conv->mon_decimal_point && *conv->mon_decimal_point ? conv->mon_decimal_point : ".";
    cs_end_space = (currency_symbol && !conv->p_cs_precedes && conv->p_sep_by_space) ? " " : "";
    cs_end = (currency_symbol && !conv->p_cs_precedes) ? currency_symbol : "";

    //units = ldiv ( labs (number.mantissa), gsb_real_power_10[number.exponent] );
	gsb_real_norm(&number);
	ent = ABS(number.ent);
	decimals = frac_get_decimals (number.f, floating_point);
	
    if ( ent < 1000 )
    {
        g_snprintf (format, sizeof(format), "%s%d%s",
                                           "%s%s%s%d%s%0",
                                           floating_point,
                                           "d%s%s" );
        result = g_strdup_printf ( format, 
                        cs_start,
                        cs_start_space,
                        sign,
                        ent,
                        mon_decimal_point,
                        decimals,
                        cs_end_space,
                        cs_end );
    }
    else
    {
        gchar *mon_thousands_sep = g_locale_to_utf8 ( conv->mon_thousands_sep, -1,
                                                        NULL, NULL, NULL );
        div_t thousands = div ( ent, 1000 );
        if ( thousands.quot < 1000 )
        {
            g_snprintf ( format, sizeof (format), "%s%d%s",
                                                "%s%s%s%d%s%03d%s%0",
                                                floating_point,
                                                "d%s%s");
            result = g_strdup_printf ( format, 
                            cs_start,
                            cs_start_space,
                            sign,
                            thousands.quot,
                            mon_thousands_sep,
                            thousands.rem,
                            mon_decimal_point,
                            decimals,
                            cs_end_space,
                            cs_end );
        }
        else
        {
            div_t millions = div ( thousands.quot, 1000 );
            if ( millions.quot < 1000 )
            {
                g_snprintf ( format, sizeof (format), "%s%d%s",
                                                    "%s%s%s%d%s%03d%s%03d%s%0",
                                                    floating_point,
                                                    "d%s%s");
                result = g_strdup_printf ( format, 
                                cs_start,
                                cs_start_space,
                                sign,
                                millions.quot,
                                mon_thousands_sep,
                                millions.rem,
                                mon_thousands_sep,
                                thousands.rem,
                                mon_decimal_point,
                                decimals,
                                cs_end_space,
                                cs_end);
            }
            else
            {
                div_t billions = div ( millions.quot, 1000 );
                g_snprintf ( format, sizeof (format), "%s%d%s",
                                                    "%s%s%s%d%s%03d%s%03d%s%03d%s%0",
                                                    floating_point,
                                                    "d%s%s" );
                result = g_strdup_printf ( format, 
                                cs_start,
                                cs_start_space,
                                sign,
                                billions.quot,
                                mon_thousands_sep,
                                billions.rem,
                                mon_thousands_sep,
                                millions.rem,
                                mon_thousands_sep,
                                thousands.rem,
                                mon_decimal_point,
                                decimals,
                                cs_end_space,
                                cs_end );
            }
        }
        g_free ( mon_thousands_sep );
    }
	return result;
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
gchar *gsb_real_format_string ( gsb_real number,
                        gint currency_number,
                        gboolean show_symbol )
{
    struct lconv * conv = localeconv ( );
    gint floating_point;

    const gchar *currency_symbol = (currency_number && show_symbol)
                                   ? gsb_data_currency_get_code_or_isocode (currency_number)
                                   : NULL;

    /* First of all if number = 0 I return 0 with the symbol of the currency if necessary */
    if (GSB_REAL_NULL(number))
    {
        if (currency_symbol && conv -> p_cs_precedes)
            return g_strdup_printf ( "%s %s", currency_symbol, "0" );
        else if (currency_symbol && ! conv -> p_cs_precedes)
            return g_strdup_printf ( "%s %s", "0", currency_symbol );
        else
            return g_strdup ("0");
    }

    /* first we need to adapt the exponent to the currency */
    /* if the exponent of the real is not the same of the currency, need to adapt it */
    floating_point = gsb_data_currency_get_floating_point ( currency_number );
//    if ( currency_number && number.exponent != floating_point )
//        number = gsb_real_adjust_exponent ( number, floating_point );

    return gsb_real_raw_format_string ( number, localeconv(), currency_symbol, floating_point);
}



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
    return gsb_real_format_string ( number, 0, FALSE );
}

/**
 * format a gsb_real into a string from gsb_real_get_string
 *	adapt the format of the real to the currency (nb digits after , ...)
 *	show if asked the currency symbol
 *
 * \param number		A number to format.
 * \param currency_number	Currency to use.
 *
 * \return a newly allocated string of the number
 * */
gchar *gsb_real_get_string_with_currency ( gsb_real number,
                        gint currency_number,
                        gboolean show_symbol )
{
    gchar *string;

    string = gsb_real_format_string (number,
				     currency_number,
				     show_symbol);
    return string;
}



/**
 * get a real number from an integer
 *
 * \param mantissa
 * \param exponent -1 for no limit
 *
 * \return a gsb_real from the integer
 */
/*gsb_real gsb_real_new ( gint mantissa, gint exponent )
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
/*gsb_real gsb_real_raw_get_from_string ( const gchar *string,
                                        const gchar *mon_thousands_sep,
                                        const gchar *mon_decimal_point )
{
    static const gchar *space_chars = " ";
    static const gchar *decimal_chars = ".,";
    static const gchar *positive_chars = "+";
    static const gchar *negative_chars = "-";

	unsigned mts_len;
	unsigned mdp_len;
    unsigned nb_digits = 0;
    gint64 mantissa = 0;
    gint8 sign = 0;
    gint8 dot_position = -1;
    const gchar *p = string;

    if ( !string)
        return error_real;

    mts_len = mon_thousands_sep
                       ? strlen ( mon_thousands_sep )
                       : 0;
    mdp_len = mon_decimal_point ? strlen ( mon_decimal_point ) : 0;

    for ( ; ; )
    {
        if ( g_ascii_isdigit ( *p ) )
        {
            mantissa *= 10;
            mantissa += ( *p - '0' );
            if ( mantissa > G_MAXLONG )
                return error_real;
            if ( sign == 0 ) sign = 1; // no sign found yet ==> positive
            ++nb_digits;
            ++p;
        }
        else if ( *p == 0 ) // terminal zero
        {
			gsb_real result;
			result.mantissa = sign * mantissa;
            result.exponent = ( dot_position >= 0 )
                              ? nb_digits - dot_position
                              : 0;
            return result;
        }
        else if ( strchr ( space_chars, *p )
             || ( mts_len && ( strncmp ( p, mon_thousands_sep, mts_len ) == 0 ) ) )
        {
            // just skip spaces and thousands separators
            p = g_utf8_find_next_char ( p, NULL );
        }
        else if ( strchr ( decimal_chars, *p )
             || ( mdp_len && ( strncmp ( p, mon_decimal_point, mdp_len ) == 0 ) ) )
        {
            if ( dot_position >= 0 ) // already found a decimal separator
                return error_real;
            dot_position = nb_digits;
            p = g_utf8_find_next_char ( p, NULL );
        }
        else if ( strchr ( negative_chars, *p ) )
        {
            if ( sign != 0 ) // sign already set
                return error_real;
            sign = -1;
            ++p;
        }
        else if ( strchr ( positive_chars, *p ) )
        {
            if ( sign != 0 ) // sign already set
                return error_real;
            sign = 1;
            ++p;
        }
        else // unknown char ==> error
        {
            return error_real;
        }
    }
}*/




/**
 * get a gsb_real number from a string
 * the string can be formatted :
 * - spaces and the given utf8-encoded thousands separators are ignored
 * - handle "." and the given utf8-encoded as decimal separator
 * - another character makes a error_real return
 *
 * \param string
 * \param mon_thousands_sep, can be NULL or empty, but only one utf8 sequence
 * \param mon_decimal_point, can be NULL or empty, but only one utf8 sequence
 *
 * \return the number in the string transformed to gsb_real
 */
gsb_real gsb_real_raw_get_from_string ( const gchar * str,
                                       const gchar *mon_thousands_sep,
                                       const gchar *mon_decimal_point )
{
	gchar *str2, **numb, **ff, **ss, *err;
	gchar *sss, *ttt, *f, *s, *a, *b;
	gchar end;
	static gchar *space_chars;
    static gchar *decimal_chars;
	int sign = 1, decimals;
	glong nombre;
	gsb_real resu;
	
    decimal_chars = g_strconcat ( ".", mon_decimal_point, NULL );
    space_chars = g_strconcat ( " ", mon_thousands_sep, NULL );

	/*if ( !g_utf8_validate ( str, -1, NULL ) )
		str2 = g_locale_to_utf8 ( str, -1, NULL, NULL, NULL );
	else*/
		str2 = g_strdup ( str ) ;
	if(!str2 || !strlen( str2 ))
		return null_real;
	sign = 1;
	if( g_strrstr ( str2 , "-") )
		sign = (-1);
	numb = g_strsplit_set(str2, decimal_chars, 2);
	g_free(str2);
	g_free(decimal_chars);
	if( !numb[0] || !strlen( numb[0]  ))
		a = g_strdup ( "0" );
	else
		a = g_strdup(numb[0]);
	if( !numb[1] || !strlen( numb[1] ))
		b = g_strdup ( "0" );
	else
		b = g_strdup(numb[1]);
	g_strfreev(numb);
	ff = g_strsplit ( a , space_chars, -1);
	ss = g_strsplit ( b , space_chars, -1);
	g_free(a);
	g_free(b);
	g_free(space_chars);
	f = g_strjoinv ("", ff);
	s = g_strjoinv ("", ss);
	g_strfreev ( ff );
	g_strfreev ( ss );
	decimals = strlen(s);
	//sss = g_convert(f, -1, "ASCII", "UTF-8", NULL, NULL, NULL);
	//g_free(f);
	errno = 0; end = 'x';
	resu.ent = strtol(f, &err, 10);
	end = *err;
	g_free(f);
	if(errno == ERANGE || end != '\0')
		return error_real;
	errno = 0; end = 'x';
	nombre = strtol(s, &err, 10);
	end = *err;
	g_free(s);
	if(errno == ERANGE || end != '\0' )
		return error_real;
	resu.f = simplify_64 ((gint64)(nombre * sign), (gint64) exp10(decimals));
	gsb_real_norm (&resu);
	return resu;
}


/**
 * get a real number from a string
 * the string can be formatted :
 * - handle , or . as separator
 * - spaces are ignored
 * - another character makes a 0 return
 *
 *   the gsb_real will have the exponent given in default_exponent, except if default_exponent = -1
 *
 * \param string
 * \param default_exponent -1 for no limit
 *
 * \return the number in the string transformed to gsb_real
 */
gsb_real gsb_real_get_from_string_normalized ( const gchar *string, gint default_exponent )
{
    struct lconv *conv = localeconv ( );
    gchar *mon_thousands_sep = g_strdup(conv->mon_thousands_sep);
    gchar *mon_decimal_point = g_strdup(conv->mon_decimal_point);
    gsb_real result =  gsb_real_raw_get_from_string ( string, mon_thousands_sep, mon_decimal_point );
    g_free ( mon_thousands_sep );
    g_free ( mon_decimal_point );
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
                                        const gchar *mon_thousands_sep,
                                        const gchar *mon_decimal_point )
 *
 * \param string
 *
 * \return the number in the string transformed to gsb_real
 * */
gsb_real gsb_real_get_from_string ( const gchar *string )
{
    return gsb_real_get_from_string_normalized ( string, -1 );
}


/**
 * compare 2 gsb_real and return the result (-1, 0, 1)
 *
 * \param r1
 * \param r2
 *
 * \return -1 if r1 < r2 ; 0 if r1 == r2 ; 1 if r1 > r2
 * */
G_MODULE_EXPORT gint gsb_real_cmp ( gsb_real r1,
                        gsb_real r2 )
{
    gsb_real_norm (&r1);
	gsb_real_norm (&r2);
	if(r1.ent != r2.ent)
		return (r1.ent > r2.ent ? 1 : -1);
	else
		return gsb_real_frac_cmp(r1.f, r2.f);
}

/**
 * reduce the exponent to its smallest possible value,
 * without losing any precision
 *
 * \param mantissa a pointer to the 64 bits mantissa to be reduced
 * \param exponent a pointer to the exponent to be reduced
 **/
/*void gsb_real_raw_minimize_exponent ( gint64 *mantissa, gint *exponent )
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
/*void gsb_real_minimize_exponent ( gsb_real *num )
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
/*gboolean gsb_real_grow_exponent( gsb_real *num, guint target_exponent )
{
    gint64 mantissa = num->mantissa;
    gint exponent = num->exponent;
    gboolean succes = TRUE;

    while ( exponent < target_exponent )
    {
        gint64 new_mantissa = mantissa * 10;
        if ( ( new_mantissa > G_MAXLONG ) || ( new_mantissa < G_MINLONG ) )
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
 * \return TRUE if normalization occured
 * FALSE if exponents can't be the same without loss of precision
 * */
/*gboolean gsb_real_normalize ( gsb_real *number_1, gsb_real *number_2 )
{
    gsb_real_minimize_exponent ( number_1 );
    gsb_real_minimize_exponent ( number_2 );

    if ( number_1->exponent < number_2->exponent )
    {
        if ( !gsb_real_grow_exponent ( number_1, number_2->exponent ) )
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
        if ( !gsb_real_grow_exponent ( number_2, number_1->exponent ) )
		{
            while ( number_1 -> exponent > number_2 -> exponent )
            {
                number_1 -> exponent--;
                number_1 -> mantissa = number_1 -> mantissa / 10;
            }
		}
    }

    return ( number_1->exponent == number_2->exponent );
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
	gsb_real_norm(&number);
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
G_MODULE_EXPORT gsb_real gsb_real_add ( gsb_real r1,
                                       gsb_real r2 )
{
	gsb_real rout;
	frac sum;
	if (!GSB_REAL_VALID(r1) || !GSB_REAL_VALID(r2))
		return error_real;
	sum = gsb_real_frac_sum(r1.f, r2.f);
	rout.ent = r1.ent + r2.ent + sum.num/sum.denom;
	(rout.f).num = sum.num % sum.denom;
	(rout.f).denom = sum.denom;
	return rout;
}


/**
 * return the opposite of the number
 * ie 5 returns -5 in gsb_real r
 *
 * \param r
 *
 * \return its opposite
 * */
G_MODULE_EXPORT gsb_real gsb_real_opposite ( gsb_real r )
{
	r.ent = -r.ent;
	r.f.num = -r.f.num;
	return r;
}



/**
 * substract between 2 gsb_real : r1 - r2
 *
 * \param r1
 * \param r2
 *
 * \return a gsb_real = r1 - r2
 * */
gsb_real gsb_real_sub ( gsb_real r1,
                        gsb_real r2 )
{
	return gsb_real_add ( r1, gsb_real_opposite(r2) );
}


/**
 * give the absolute value of the number
 *
 * \param number
 *
 * \return a gsb_real, the absolute value of the given number
 * */
gsb_real gsb_real_abs ( gsb_real r )
{
	gsb_real_norm(&r);
	if(GSB_REAL_SIGN(r) >= 0)
		return r;
    return gsb_real_opposite(r);
}


/**
 * Tests equality of gsb_reals. r1 == r2
 *
 * \param r1
 * \param r2
 *
 * \return True if the 2 numbers are equal
 * */
gboolean gsb_real_equal (gsb_real r1, gsb_real r2){
	gsb_real_norm (&r1);
	gsb_real_norm (&r2);
	return (gboolean) (r1.ent == r2.ent && gsb_real_frac_equal(r1.f, r2.f));
}


/**
 * multiply 2 gsb_reals. no overflow possible
 *
 * \param r1
 * \param r2
 *
 * \return the multiplication between the 2
 * */
gsb_real gsb_real_mul ( gsb_real r1,
                        gsb_real r2 )
{
	gsb_real r, a, b, c, d;
	if (gsb_real_equal(r1, error_real) || gsb_real_equal(r2, error_real))
		return error_real;
	if (gsb_real_equal(r1, null_real) || gsb_real_equal(r2, null_real))
		return null_real;
	a = frac_to_gsb_real_64 ( r1.ent * r2.f.num , r2.f.denom );
	b = frac_to_gsb_real_64 ( r2.ent * r1.f.num , r1.f.denom );
	c = frac_to_gsb_real_64 ( r1.f.num * r2.f.num , r1.f.denom * r2.f.denom );
	d = frac_to_gsb_real ( gsb_real_frac_sum ( gsb_real_frac_sum ( a.f, b.f ), c.f ) );
	r.ent = r1.ent * r2.ent + a.ent + b.ent + c.ent + d.ent;
	r.f = d.f;
	return r;
}


/**
 * Inverts r
 *
 * \param r
 *
 * \return reverse of r
 * */
gsb_real gsb_real_inverse ( gsb_real r )
{
	frac temp;
	frac f = simplify_64( (gint64)r.f.num + (gint64)r.ent * (gint64)r.f.denom, (gint64) r.f.denom );
	if( gsb_real_frac_equal(f, null_frac) || gsb_real_frac_equal (f, error_frac))
		return error_real;
	temp.num = (f.num > 0 ? (gint32) f.denom : - ((gint32)f.denom) );
	temp.denom = (f.num > 0 ? (gint32) f.num : - ((gint32)f.num) );
	return frac_to_gsb_real ( temp );
}


/**
 * divide 2 gsb_reals
 *
 * \param r1
 * \param r2
 *
 * \return the div between the 2
 * */
gsb_real gsb_real_div ( gsb_real r1,
                        gsb_real r2 )
{
    return gsb_real_mul (r1, gsb_real_inverse (r2));
}


/**
 * convert a gsb_real to a double
 *
 * \param r a gsb_real number
 *
 * \return the number in double format
 * */
/*gdouble gsb_real_real_to_double ( gsb_real r )
{
   	return ( ((gdouble) r.f.num) / ((gdouble) r.f.denom) + ((gdouble) r.ent) );
}*/


/**
 * convert a double to a gsb_real
 *
 * \param number a gdouble number
 *
 * \return the number in gsb_real format
 * */
G_MODULE_EXPORT gsb_real gsb_real_double_to_real ( gdouble number )
{
	gsb_real r;
	r.ent = ((gint32) number)/1;
	r.f = simplify_64 ( (gint64)( modf(number, NULL) * exp10(9)), 1000000000);
	return r;
}

/*gsb_real gsb_real_double_to_real_add_exponent ( gdouble number, gint exp_add )
{
    gdouble tmp_double, decimal;
    gdouble maxlong;

	gsb_real real_number = {0, exp_add};
    maxlong = G_MAXLONG / 10;

	if(exp_add >=9)
		return null_real;

	while ( (modf (number, &tmp_double) ||
		real_number.exponent < 0 ) &&
		real_number.exponent < (9-exp_add))
    {
	number = number * 10;
	real_number.exponent++;

	if (fabs (number) > maxlong)
	    number = rint (number);
    }
	decimal = modf ( number, &tmp_double );
	if ( ( (real_number.exponent == (9-exp_add)) ) && (decimal >= 0.5) )
		real_number.mantissa = ((glong) number ) + 1;
	
    real_number.mantissa = (glong) (number);
    return real_number;
}


/**
 * truncate the number. WARNING there loss of accuracy
 *
 * \param mantissa a pointer to the 64 bits mantissa to be truncate
 * \param exponent a pointer to the exponent to be truncate
 **/
/*gboolean gsb_real_raw_truncate_number ( gint64 *mantissa, gint *exponent )
{
    if ( *mantissa > G_MAXLONG )
    {
        do
        {
            --*exponent;
            *mantissa = *mantissa / 10;
        } while ( *mantissa > G_MAXLONG );
        return TRUE;
    }
    else if ( *mantissa < G_MINLONG )
    {
        do
        {
            --*exponent;
            *mantissa = *mantissa / 10;
        } while ( *mantissa < G_MINLONG );
        return TRUE;
    }

    return FALSE;
}*/
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */

