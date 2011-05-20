/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)         2011 Grisbi Development Team                     */
/*             http://www.grisbi.org                                          */
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
#include "gsb_data_currency.h"
#include "gsb_locale.h"

#include "utils_real.h"



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
    struct lconv *locale = gsb_locale_get_locale ();

    return gsb_real_raw_format_string ( number, locale, NULL );
}
