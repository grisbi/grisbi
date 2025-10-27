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
/*  along with this program; if not, see <https://www.gnu.org/licenses/>.     */
/*                                                                            */
/* ************************************************************************** */

#include "config.h"

#include <glib.h>
#include <string.h>
#include <stdio.h>

/*START_INCLUDE*/
#include "include.h"
#include "gsb_locale.h"
#include "erreur.h"
/*END_INCLUDE*/

static struct lconv *_locale = NULL;
static gchar *langue;

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 *
 *
 * \param
 *
 * \return
 **/
void gsb_locale_init_lconv_struct (void)
{
	struct lconv *locale;

	locale = localeconv ();
	_locale = g_malloc (sizeof (*_locale));

	/* thousands_sep */
	if (locale->thousands_sep && strlen (locale->thousands_sep) > 0)
	{
		if (g_utf8_validate (locale->thousands_sep, -1, NULL))
			_locale->thousands_sep = g_strdup (locale->thousands_sep);
		else
			_locale->thousands_sep = g_locale_to_utf8 (locale->thousands_sep, -1, NULL, NULL, NULL);
	}
	else
	{
		_locale->thousands_sep = NULL;
	}

	/* mon_thousands_sep */
	if (locale->mon_thousands_sep && strlen (locale->mon_thousands_sep) > 0)
	{
		if (g_utf8_validate (locale->mon_thousands_sep, -1, NULL))
			_locale->mon_thousands_sep = g_strdup (locale->mon_thousands_sep);
		else
			_locale->mon_thousands_sep = g_locale_to_utf8 (locale->mon_thousands_sep, -1, NULL, NULL, NULL);
	}
	else
	{
		_locale->mon_thousands_sep = NULL;
	}

	/* currency_symbol */
	if (g_utf8_validate (locale->currency_symbol, -1, NULL))
		_locale->currency_symbol = g_strdup (locale->currency_symbol);
	else
		_locale->currency_symbol = g_locale_to_utf8 (locale->currency_symbol, -1, NULL, NULL, NULL);

	/* negative sign */
	if (locale->negative_sign && strlen (locale->negative_sign) > 0)
		_locale->negative_sign     = g_strdup (locale->negative_sign);
	else
		_locale->negative_sign     = g_strdup ("-");

	/* others data */
	_locale->decimal_point     = g_strdup (locale->decimal_point);
    _locale->grouping          = g_strdup (locale->grouping);
    _locale->int_curr_symbol   = g_strdup (locale->int_curr_symbol);
    _locale->mon_decimal_point = g_strdup (locale->mon_decimal_point);
    _locale->mon_grouping      = g_strdup (locale->mon_grouping);
    _locale->positive_sign     = g_strdup (locale->positive_sign);
    _locale->int_frac_digits   = locale->int_frac_digits;
    _locale->frac_digits       = locale->frac_digits;
    _locale->p_sign_posn       = locale->p_sign_posn;
    _locale->n_sign_posn       = locale->n_sign_posn;
	_locale->p_cs_precedes     = locale->p_cs_precedes;
	_locale->p_sep_by_space    = locale->p_sep_by_space;
	_locale->n_cs_precedes     = locale->n_cs_precedes;
	_locale->n_sep_by_space    = locale->n_sep_by_space;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void gsb_locale_shutdown (void)
{
    g_free (_locale->decimal_point);
    g_free (_locale->thousands_sep);
    g_free (_locale->grouping);
    g_free (_locale->int_curr_symbol);
    g_free (_locale->currency_symbol);
    g_free (_locale->mon_decimal_point);
	g_free (_locale->mon_thousands_sep);
    g_free (_locale->mon_grouping);
    g_free (_locale->positive_sign);
    g_free (_locale->negative_sign);
    g_free (_locale);
    g_free (langue);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
struct lconv *gsb_locale_get_locale (void)
{
    return _locale;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gchar *gsb_locale_get_mon_decimal_point (void)
{
    return g_strdup (_locale->mon_decimal_point);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void gsb_locale_set_mon_decimal_point (const gchar *decimal_point)
{
    g_free (_locale->mon_decimal_point);
    _locale->mon_decimal_point = g_strdup (decimal_point);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gchar *gsb_locale_get_mon_thousands_sep (void)
{
    return g_strdup (_locale->mon_thousands_sep);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void gsb_locale_set_mon_thousands_sep (const gchar *thousands_sep)
{
    g_free (_locale->mon_thousands_sep);
	if (thousands_sep == NULL)
		_locale->mon_thousands_sep = NULL;
	else
		_locale->mon_thousands_sep = g_strdup (thousands_sep);
}

/**
 *
 *
 * \param
 *
 *  \return string  must be freed
 */
gchar *gsb_locale_get_print_locale_var (void)
{
    gchar *locale_str = NULL;
    gchar *mon_thousands_sep = NULL;
    gchar *mon_decimal_point;
    gchar *positive_sign;
    gchar *negative_sign;
    gchar *currency_symbol;

	if (NULL == _locale)
		return g_strdup("locale not yet configured\n");

    /* test local pour les nombres */
	currency_symbol = g_strdup (_locale->currency_symbol);
	mon_thousands_sep = g_strdup (_locale->mon_thousands_sep);
    mon_decimal_point = g_locale_to_utf8 (_locale->mon_decimal_point, -1, NULL, NULL, NULL);
    positive_sign = g_locale_to_utf8 (_locale->positive_sign, -1, NULL, NULL, NULL);
    negative_sign = g_locale_to_utf8 (_locale->negative_sign, -1, NULL, NULL, NULL);

    locale_str = g_strdup_printf ("LANGUAGE = %s\n\n"
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
								  langue,
								  currency_symbol,
								  mon_thousands_sep,
								  mon_decimal_point,
								  positive_sign,
								  negative_sign,
								  _locale->p_cs_precedes,
								  _locale->n_cs_precedes,
								  _locale->p_sep_by_space,
								  _locale->frac_digits);

    g_free (currency_symbol);
    g_free (mon_thousands_sep);
    g_free (mon_decimal_point);
    g_free (positive_sign);
    g_free (negative_sign);

    return locale_str;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
const gchar *gsb_locale_get_language (void)
{
	return langue;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void gsb_locale_init_language (const gchar *new_language)
{
	gchar *tmp_str;
	gchar *language = FALSE;

	if (!new_language)
	{
#ifdef G_OS_WIN32
		tmp_str = g_win32_getlocale ();
		if (tmp_str)
		{
			gchar *ptr;

			ptr = g_strrstr (tmp_str, ".");
			if (ptr)
			{
				language = g_strndup (tmp_str, (ptr - tmp_str));
			}
			else
			{
				language = g_strdup (tmp_str);
			}
			g_setenv ("LANGUAGE", language, TRUE);
			g_setenv ("LANG", tmp_str, TRUE);
			langue = g_strdup (language);
			setlocale (LC_ALL, "");
			g_free (language);
			g_free (tmp_str);
			return;
		}
#else
		const gchar *tmp_language = g_getenv ("LANGUAGE");

		if (tmp_language && strlen (tmp_language))
		{
			language = g_strdup (tmp_language);
		}
		else
		{
			tmp_language = g_getenv ("LANG");
			if (tmp_language)
			{
				gchar *ptr;

				ptr = g_strrstr (tmp_language, ".");
				if (ptr)
				{
					language = g_strndup (tmp_language, (ptr - tmp_language));
				}
				else
				{
					language = g_strdup (tmp_language);
				}
				if (!g_setenv ("LANGUAGE", language, TRUE))
					important_debug("Unable to set LANGUAGE");
				langue = g_strdup (language);
				setlocale (LC_ALL, "");
				g_free (language);
				return;
			}
		}
#endif
	}
	else
		language = g_strdup (new_language);

	if (!language)
		return;

	/* set LANGUAGE for gtk3 */
	if (!g_setenv ("LANGUAGE", language, TRUE))
		important_debug("g_setenv(LANGUAGE) failed");
	/* set LANG for devise and numbers */
	if (2 == strlen(language))
	{
		/* "it" -> "it_IT.UTF-8" */
		gchar *LANGUAGE = g_ascii_strup(language, -1);
		tmp_str = g_strconcat (language, "_", LANGUAGE,".UTF-8", NULL);
		g_free(LANGUAGE);
	}
	else
		/* language should already be in the form "en_GB" */
		tmp_str = g_strconcat (language, ".UTF-8", NULL);
	if (!g_setenv ("LANG", tmp_str, TRUE))
		important_debug("g_setenv(LANG) failed");
	g_free (tmp_str);
	langue = g_strdup (language);
	g_free (language);
	setlocale (LC_ALL, "");
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
