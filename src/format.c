/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2004 Joao F. (joaof@users.sf.net)	              */
/* 			http://www.grisbi.org		                      */
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
#include "format.h"
#include "configuration.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/


/* ------------------------------------------------------ */
/* Implementation of date formatting functions            */
/* ------------------------------------------------------ */

/* (see strftime)
 * %x is replaced by the locale's appropriate date representation.
 * %y is replaced by the year without century as a decimal number [00,99]
 * %Y is replaced by the year with century as a decimal number
 * %m is replaced by the month as a decimal number [01,12]
 * %d is replaced by the day of the month as a decimal number [01,31]
 * %e is replaced by the day of the month as a decimal number [1,31]; 
 *    a single digit is preceded by a space
 */

gchar *g_predefined_formats[] =
{
	"%x"      , /* Use default locale               */
	"%Y/%m/%d", /* ex. 2004/09/25                   */
	"%d/%m/%Y", /* ex. 25/09/2004                   */
	"%Y-%m-%d", /* simple XML schema (YYYY-MM-DD)   */
	NULL
};

gchar *gsb_format_gdate(GDate *gdate)
{
    gchar buf[32];

    if ( !g_date_strftime( buf,
			   31,
			   get_config_format()->format_date_liste_ope,
			   gdate ))
	return NULL;

    return g_strdup (buf);
}

