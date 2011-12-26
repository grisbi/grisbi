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

#include <glib.h>
#include "include.h"
#include "gsb_locale.h"

static struct lconv *_locale = NULL;


void gsb_locale_init ( void )
{
    struct lconv *locale;

    locale = localeconv ( );

    _locale = g_malloc0 ( sizeof (*_locale) );
    _locale -> decimal_point     = g_strdup ( locale -> decimal_point );
    _locale -> thousands_sep     = g_strdup ( locale -> thousands_sep );
    _locale -> grouping          = g_strdup ( locale -> grouping );
    _locale -> int_curr_symbol   = g_strdup ( locale -> int_curr_symbol );
    _locale -> currency_symbol   = g_strdup ( locale -> currency_symbol );
    _locale -> mon_decimal_point = g_strdup ( locale -> mon_decimal_point );
    _locale -> mon_thousands_sep = g_strdup ( locale -> mon_thousands_sep );
    _locale -> mon_grouping      = g_strdup ( locale -> mon_grouping );
    _locale -> positive_sign     = g_strdup ( locale -> positive_sign );
    _locale -> negative_sign     = g_strdup ( locale -> negative_sign );
    _locale -> int_frac_digits   = locale -> int_frac_digits;
    _locale -> frac_digits       = locale -> frac_digits;
    _locale -> p_cs_precedes     = locale -> p_cs_precedes;
    _locale -> p_sep_by_space    = locale -> p_sep_by_space;
    _locale -> n_cs_precedes     = locale -> n_cs_precedes;
    _locale -> n_sep_by_space    = locale -> n_sep_by_space;
    _locale -> p_sign_posn       = locale -> p_sign_posn;
    _locale -> n_sign_posn       = locale -> n_sign_posn;
}


void gsb_locale_shutdown ( void )
{
    g_free ( _locale -> decimal_point );
    g_free ( _locale -> thousands_sep );
    g_free ( _locale -> grouping );
    g_free ( _locale -> int_curr_symbol );
    g_free ( _locale -> currency_symbol );
    g_free ( _locale -> mon_decimal_point );
    g_free ( _locale -> mon_thousands_sep );
    g_free ( _locale -> mon_grouping );
    g_free ( _locale -> positive_sign );
    g_free ( _locale -> negative_sign );
    g_free ( _locale );
}


struct lconv *gsb_locale_get_locale ( void )
{
    return _locale;
}


gchar *gsb_locale_get_mon_decimal_point ( void )
{
    return g_strdup ( _locale -> mon_decimal_point );
}


void gsb_locale_set_mon_decimal_point ( const gchar *decimal_point )
{
    g_free ( _locale -> mon_decimal_point );
    _locale -> mon_decimal_point = g_strdup ( decimal_point );
}


gchar *gsb_locale_get_mon_thousands_sep ( void )
{
    return g_strdup ( _locale -> mon_thousands_sep );
}


void gsb_locale_set_mon_thousands_sep ( const gchar *thousands_sep )
{
    g_free ( _locale -> mon_thousands_sep );
    _locale -> mon_thousands_sep = g_strdup ( thousands_sep );
}
