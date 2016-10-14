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
    const gchar *langue;

    locale = localeconv ();
    langue = g_getenv ( "LANG");

    _locale = g_malloc ( sizeof (*_locale) );
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
    _locale -> p_sign_posn       = locale -> p_sign_posn;
    _locale -> n_sign_posn       = locale -> n_sign_posn;

    if ( g_str_has_prefix ( langue, "fr_" ) )
    {
        _locale -> p_cs_precedes     = 0;
        _locale -> p_sep_by_space    = 1;
        _locale -> n_cs_precedes     = 0;
        _locale -> n_sep_by_space    = 1;
    }
    else     if ( g_str_has_prefix ( langue, "de_" ) )
    {
        _locale -> p_cs_precedes     = 0;
        _locale -> p_sep_by_space    = 0;
        _locale -> n_cs_precedes     = 0;
        _locale -> n_sep_by_space    = 0;
    }
    else
    {
        _locale -> p_cs_precedes     = locale -> p_cs_precedes;
        _locale -> p_sep_by_space    = locale -> p_sep_by_space;
        _locale -> n_cs_precedes     = locale -> n_cs_precedes;
        _locale -> n_sep_by_space    = locale -> n_sep_by_space;
    }
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


/**
 *
 *
 * \param
 *
 *  \return string  must be freed
 */
gchar *gsb_locale_get_print_locale_var ( void )
{
    struct lconv *conv;
    gchar *locale_str = NULL;
    gchar *mon_thousands_sep;
    gchar *mon_decimal_point;
    gchar *positive_sign;
    gchar *negative_sign;
    gchar *currency_symbol;

    /* test local pour les nombres */
    conv = localeconv();

    currency_symbol = g_locale_to_utf8 ( conv->currency_symbol, -1, NULL, NULL, NULL );
    mon_thousands_sep = g_locale_to_utf8 ( conv->mon_thousands_sep, -1, NULL, NULL, NULL );
    mon_decimal_point = g_locale_to_utf8 ( conv->mon_decimal_point, -1, NULL, NULL, NULL );
    positive_sign = g_locale_to_utf8 ( conv->positive_sign, -1, NULL, NULL, NULL );
    negative_sign = g_locale_to_utf8 ( conv->negative_sign, -1, NULL, NULL, NULL );

    locale_str = g_strdup_printf ( "LANG = %s\n\n"
                        "Currency\n"
                        "\tcurrency_symbol   = %s\n"
                        "\tmon_thousands_sep = \"%s\"\n"
                        "\tmon_decimal_point = %s\n"
                        "\tpositive_sign     = \"%s\"\n"
                        "\tnegative_sign     = \"%s\"\n"
                        "\tp_cs_precedes     = \"%d\"\n"
                        "\tn_cs_precedes     = \"%d\"\n"
                        "\tp_sep_by_space    = \"%d\"\n"
                        "\tfrac_digits       = \"%d\"\n\n",
                        g_getenv ( "LANG"),
                        currency_symbol,
                        mon_thousands_sep,
                        mon_decimal_point,
                        positive_sign,
                        negative_sign,
                        conv->p_cs_precedes,
                        conv->n_cs_precedes,
                        conv->p_sep_by_space,
                        conv->frac_digits );

    g_free ( currency_symbol );
    g_free ( mon_thousands_sep );
    g_free ( mon_decimal_point );
    g_free ( positive_sign );
    g_free ( negative_sign );

    return locale_str;
}


/**
 *
 *
 * \param
 *
 * \return
 **/
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
