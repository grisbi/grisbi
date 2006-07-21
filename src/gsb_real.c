/* ************************************************************************** */
/*                                                                            */
/*                                  gsb_real	                              */
/*                                                                            */
/*     Copyright (C)	2000-2006 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2006 Benjamin Drieu (bdrieu@april.org)	      */
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

/**
 * \file gsb_real.c
 * grisbi use a special structure to describe a real number
 * all is defined her
 */


#include "include.h"

/*START_INCLUDE*/
#include "gsb_real.h"
#include "utils_str.h"
/*END_INCLUDE*/

gsb_real null_real = { 0 , 0 };


/*START_STATIC*/
static gsb_real gsb_real_double_to_real ( gdouble number );
static gboolean gsb_real_normalize ( gsb_real *number_1,
			      gsb_real *number_2 );
static gdouble gsb_real_real_to_double ( gsb_real number );
/*END_STATIC*/

/*START_EXTERN*/
extern gint max;
/*END_EXTERN*/


/**
 * return the real in a formatted string :
 * - the separator is a . or , according to the currency/locale (TODO)
 * - a space between the thousand
 *
 * \param number
 *
 * \return a newly allocated string of the number
 * */
gchar *gsb_real_get_string ( gsb_real number )
{
    div_t result_div;
    gchar *string;
    gint i = 0, j=0;
    glong num;

    /* for a long int : max 11 char
     * so with the possible -, the spaces and the .
     * we arrive to maximum 14 char : -21 474 836.48 + 1 for the 0 terminal */
    string = g_malloc0 ( 15*sizeof (gchar) );
    
    num = labs(number.mantissa);

    /*     Construct the result in the reverse order from right to left, then reverse it. */
    do
    {
	if ( i
	     &&
	     i == number.exponent)
	{
	    string[i] = '.';
	    result_div.quot = num;
	}
	else
	{
	    if (i > number.exponent)
		j++;

	    if ( j==4 )
	    {
		j=0;
		string[i] = ' ';
		result_div.quot = num;
	    }
	    else
	    {
		result_div = div ( num, 10 );
		string[i] = result_div.rem + '0';
	    }
	}
	i++;
	/* we check also i < (number.exponent+2)
	 * the +2 is for 0. at the left of the separator,
	 * with that check, 0 will be 0.00 if exponent = 2,
	 * and 0.51 will be 0.51 and no 51 without that check */
    }
    while ( ( num = result_div.quot )
	    ||
	    i < (number.exponent+2) );

    /* Add the sign at the end of the string just before to reverse it to avoid
       to have to insert it at the begin just after... */
    if (number.mantissa < 0)
    {
        string[i++] = '-';
    }
    
    string[i] = 0;

    g_strreverse ( string );

    return ( string );
}



/**
 * get a real number from a string
 * the string can be formatted :
 * - handle , or . as separator
 * - spaces are ignored
 * - another character makes a 0 return
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
 * get a real number from a string
 * the string can be formatted :
 * - handle , or . as separator
 * - spaces are ignored
 * - another character makes a 0 return
 *
 * \param string
 *
 * \return the number in the string transformed to gsb_real
 */
gsb_real gsb_real_get_from_string_normalized ( const gchar *string, gint default_mantissa )
{
    gsb_real number = null_real;
    gint i = 0, sign;
    gchar * separator, * tmp;
    gchar *string_tmp;

    if ( !string)
	return number;

    string_tmp = my_strdup (string);

    if ( default_mantissa > 0 )
    {
	separator = strrchr ( string_tmp, '.' );
	if ( ! separator )
	    separator = strrchr ( string_tmp, ',' );

	if ( separator )
	{
	    tmp = string_tmp + strlen ( string_tmp ) - 1;
	    while ( * tmp == '0' && ( tmp - separator > default_mantissa ) &&
		    tmp >= string_tmp ) 
	    {
		* tmp = '\0';
		tmp --;
	    }
	}
    }

    if (string_tmp[0] == '-')
    {
	sign = -1;
	i++;
    }
    else
    {
	sign = 1;
	/* sometimes we can have "+12" so we pass the + */
	if (string_tmp[0] == '+' )
	    i++;
    }

    while (string_tmp[i])
    {
	switch (string_tmp[i])
	{
	    case ',':
	    case '.':
		number.exponent = strlen (string_tmp) -i -1;
		break;

	    case ' ':
		break;

	    default:
		if (string_tmp[i] >= '0'
		    &&
		    string_tmp[i] <= '9' )
		{
		    number.mantissa = number.mantissa * 10;
		    number.mantissa = number.mantissa + string_tmp[i] - '0';
		}
		else
		{
		    /* if there is another char, we do nothing */
		    g_free (string_tmp);
		    return null_real;
		}
	}
	i++;
    }
    number.mantissa = sign * number.mantissa;
    g_free (string_tmp);
    return number;
}



/**
 * compare 2 gsb_real and return the result (-1, 0, 1)
 *
 * \param number_1
 * \param number_2
 *
 * \return -1 if number_1 < number_2 ; 0 if number_1 = number_2 ; 1 if number_1 > number_2
 * */
gint gsb_real_cmp ( gsb_real number_1,
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
 * normalize the 2 numbers to be able to work with them later
 * for that transform the 2 numbers to have the same exponent
 * and after that we can work on the mantissa
 *
 * \param number_1 a pointer to gsb_real wich contains the number_1 to transform
 * \param number_2 a pointer to gsb_real wich contains the number_2 to transform
 *
 * \return TRUE
 * */
gboolean gsb_real_normalize ( gsb_real *number_1,
			      gsb_real *number_2 )
{
    glong limit_number;
    gboolean invert = FALSE;

    limit_number = G_MAXLONG / 10;

    while ( number_1 -> exponent != number_2 -> exponent )
    {
	/* if we go over the upper limit we change the sense */
	if ( !invert
	     &&
	     ( labs (number_1 -> mantissa) > limit_number
	      ||
	      labs (number_2 -> mantissa) > limit_number ))
	    invert = TRUE;

	if (number_1 -> exponent > number_2 -> exponent)
	{
	    if (invert)
	    {
		number_1 -> exponent--;
		number_1 -> mantissa = number_1 -> mantissa / 10;
	    }
	    else
	    {
		number_2 -> exponent++;
		number_2 -> mantissa = number_2 -> mantissa * 10;
	    }
	}
	else
	{
	    if (invert)
	    {
		number_2 -> exponent--;
		number_2 -> mantissa = number_2 -> mantissa / 10;
	    }
	    else
	    {
		number_1 -> exponent++;
		number_1 -> mantissa = number_1 -> mantissa * 10;
	    }
	}
    }
    return TRUE;
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
    number.mantissa = labs (number.mantissa);
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
    gdouble tmp;

    if (return_exponent == -1)
	return number;

    tmp = number.mantissa;

    while ( number.exponent != return_exponent )
    {
	if (number.exponent < return_exponent)
	{
	    tmp = tmp * 10;
	    number.exponent++;
	}
	else
	{
	    tmp = tmp / 10;
	    number.exponent--;
	}
    }
    number.mantissa = lrint (tmp);
    return number;
}



/**
 * add 2 gsb_real
 *
 * \param number_1
 * \param number_2
 *
 * \return a gsb_real = number_1 + number_2
 * */
gsb_real gsb_real_add ( gsb_real number_1,
			gsb_real number_2 )
{
    gsb_real number = number_1;
    
    gsb_real_normalize ( &number,
			 &number_2 );

    number.mantissa = number.mantissa + number_2.mantissa;
    return number;
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
    gsb_real number = number_1;
    
    gsb_real_normalize ( &number,
			 &number_2 );

    number.mantissa = number.mantissa - number_2.mantissa;
    return number;
}


/**
 * return the opposite of the number
 * ie 5 returns -5 in gsb_real number
 *
 * \param number
 *
 * \return its opposite
 * */
gsb_real gsb_real_opposite ( gsb_real number )
{
    number.mantissa = -number.mantissa;
    return number;
}


/**
 * multiply 2 gsb_reals
 *
 * \param number_1
 * \param number_2
 *
 * \return the multiplication between the 2
 * */
gsb_real gsb_real_mul ( gsb_real number_1,
			gsb_real number_2 )
{
    gsb_real number;

    number = gsb_real_double_to_real ( gsb_real_real_to_double (number_1)
				       *
				       gsb_real_real_to_double (number_2));

    return number;
}


/**
 * divide 2 gsb_reals
 *
 * \param number_1
 * \param number_2
 *
 * \return the multiplication between the 2
 * */
gsb_real gsb_real_div ( gsb_real number_1,
			gsb_real number_2 )
{
    gsb_real number;

    number = gsb_real_double_to_real ( gsb_real_real_to_double (number_1)
				       /
				       gsb_real_real_to_double (number_2));

    return number;
}


/**
 * convert a gsb_real to a double
 *
 * \param number a gsb_real number
 *
 * \return the number in double format
 * */
gdouble gsb_real_real_to_double ( gsb_real number )
{
    gdouble double_number;
    gint i;

    double_number = number.mantissa;
    for ( i=0 ; i<number.exponent ; i++ )
	double_number = double_number / 10;
    return double_number;
}


/**
 * convert a double to a gsb_real
 *
 * \param number a gdouble number
 *
 * \return the number in gsb_real format
 * */
gsb_real gsb_real_double_to_real ( gdouble number )
{
    gdouble tmp_double;
    gdouble maxlong;

    gsb_real real_number = null_real;
    maxlong = G_MAXLONG / 10;

    while (modf (number, &tmp_double))
    {
	number = number * 10;
	real_number.exponent++;

	if (fabs (number) > maxlong)
	    number = rint (number);
    }
    real_number.mantissa = number;
    return real_number;
}




