/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2005-2006 Benjamin Drieu (bdrieu@april.org)           */
/*          2008-2017 Pierre Biava (grisbi@pierre.biava.name)                 */
/*          https://www.grisbi.org/                                           */
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
#include "csv_parse.h"
#include "gsb_data_currency.h"
#include "gsb_data_transaction.h"
#include "gsb_real.h"
#include "import.h"
#include "utils_dates.h"
#include "utils_real.h"
#include "utils_str.h"
/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

/*START_STATIC*/
/*END_STATIC*/

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
static gchar *sanitize_field (gchar *begin,
							  gchar *end)
{
	gchar *field;
	gchar *iter;

	g_return_val_if_fail (begin <= end, NULL);

	if (end <= begin)
	{
		return (gchar*)"";
	}

	iter = field = g_malloc0 (end - begin + 1);

	/* Strip out initial white spaces. */
	while (*begin == ' ')
		begin++;

	if (*begin == '"')
	{
		begin++;
		while (*end != '"' && end >= begin)
			end--;
	}

	while (begin < end)
	{
		if (*begin == '"' && *(begin+1) == '"')
			begin++;

		if (*begin == '\\' && *(begin+1) == '"')
			begin++;

		*iter++ = *begin++;
	}
	/* Strip out remaining white spaces. */
	while (*(iter) && (*(iter-1) == ' ' || *(iter-1) == '\r' || *(iter-1) == '\n'))
		iter--;

	*iter = '\0';

	return field;
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 * TODO
 *
 * \param
 * \param
 *
 * \return
 **/
GSList *csv_import_parse_line (gchar **contents,
						const gchar *separator)
{
	GSList *list = NULL;
	gchar *tmp_str;
	gchar *begin;
	gint is_unquoted = FALSE;
	gsize len;

	if (!separator || strlen (separator) == 0)
		return NULL;

	len = strlen (separator);
	tmp_str = (*contents);
	begin = tmp_str;

	if (*tmp_str == '\n')
	{
		*contents = tmp_str + 1;

		return GINT_TO_POINTER (-1);
	}

	if (*tmp_str == '!' || *tmp_str == '#' || *tmp_str == ';')
	{
		*contents = strchr (tmp_str, '\n') + 1;

		return GINT_TO_POINTER(-1);
	}

	while (*tmp_str)
	{
		switch (*tmp_str)
		{
			case '\n':
			list = g_slist_append (list, sanitize_field (begin, tmp_str));
			*contents = tmp_str+1;
			return list;

			case '"':
			if (!is_unquoted)
			{
				tmp_str++;
				while (*tmp_str)
				{
				/* This is lame escaping but we need to support it. */
				if (*tmp_str == '\\' && *(tmp_str+1) == '"')
				{
					tmp_str += 2;
				}

				/* End of quoted string. */
				if (*tmp_str == '"' && *(tmp_str+1) != '"')
				{
					break;
				}

				tmp_str++;
				}
			}
			/* FALLTHRU */
			default:
			is_unquoted = TRUE;
			if (!strncmp (tmp_str, separator, len))
			{
				list = g_slist_append (list, sanitize_field (begin, tmp_str));
				begin = tmp_str + len;
				is_unquoted = FALSE;
			}
			break;
		}

		tmp_str++;
	}

	return NULL;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gboolean csv_import_validate_string (gchar *string)
{
	g_return_val_if_fail (string, FALSE);

	return string != NULL;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gboolean csv_import_validate_date (gchar *string)
{
	GDate *date;
	g_return_val_if_fail (string, FALSE);

	date = gsb_parse_import_date_string (string);
	if (date && g_date_valid (date) && !csv_import_validate_number (string))
	{
		g_date_free (date);

		return TRUE;
	}

	if (date)
		g_date_free (date);

	return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gboolean csv_import_validate_number (gchar *string)
{
	g_return_val_if_fail (string, FALSE);

	while (*string)
	{
		if (!g_ascii_isdigit (*string) &&
			 !g_ascii_isspace (*string))
		{
			return FALSE;
		}

		string ++;
	}

	return TRUE;
}

/**
 * We don't check punctuation signs because mileage may vary with
 * locales.
 *
 * \param
 *
 * \return
 **/
gboolean csv_import_validate_amount (gchar *string)
{
	g_return_val_if_fail (string, FALSE);

	while (*string)
	{
		if (g_ascii_isalpha (* string))
		{
			return FALSE;
		}

		string++;
	}

	return TRUE;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean csv_import_parse_currency (struct ImportTransaction *ope,
									gchar *string)
{
	gint currency_number;

	g_return_val_if_fail (string, FALSE);

	currency_number = gsb_data_currency_get_number_by_name (string);

	return currency_number;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean csv_import_parse_date (struct ImportTransaction *ope,
								gchar *string)
{
	g_return_val_if_fail (string, FALSE);

	if (ope->date)
		g_date_free (ope-> date);
	ope->date = gsb_parse_import_date_string (string);

	if (!ope->date)
	{
		ope->date = gdate_today ();
	}

	return g_date_valid (ope->date);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean csv_import_parse_value_date (struct ImportTransaction *ope,
									  gchar *string)
{
	g_return_val_if_fail (string, FALSE);

	if (ope->date_de_valeur)
		g_free (ope->date_de_valeur);
	ope->date_de_valeur = gsb_parse_import_date_string (string);

	return g_date_valid (ope->date_de_valeur);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean csv_import_parse_payee (struct ImportTransaction *ope,
								 gchar *string)
{
	g_return_val_if_fail (string, FALSE);

	if (ope->tiers)
		g_free (ope->tiers);
	ope->tiers = my_strdup (string);

	return TRUE;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean csv_import_parse_notes (struct ImportTransaction *ope,
								 gchar *string)
{
	g_return_val_if_fail (string, FALSE);

	if (ope->notes)
		g_free (ope->notes);
	ope->notes = my_strdup (string);

	return TRUE;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean csv_import_parse_cheque (struct ImportTransaction *ope,
								  gchar *string)
{
	g_return_val_if_fail (string, FALSE);

	if (ope->cheque)
		g_free (ope->cheque);
	ope->cheque = my_strdup (string);

	return TRUE;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean csv_import_parse_category (struct ImportTransaction *ope,
									gchar *string)
{
	g_return_val_if_fail (string, FALSE);

	if (ope->categ)
		g_free (ope->categ);
	ope->categ = my_strdup (string);

	return TRUE;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean csv_import_parse_sub_category (struct ImportTransaction *ope,
										gchar *string)
{
	g_return_val_if_fail (string, FALSE);

	if (!ope->categ || !strlen (string))
		return FALSE;

	ope->categ = g_strconcat (ope->categ, " : ", string, NULL);

	return TRUE;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean csv_import_parse_budget (struct ImportTransaction *ope,
								  gchar *string)
{
	g_return_val_if_fail (string, FALSE);

	if (ope->budget)
		g_free (ope->budget);
	ope->budget = my_strdup (string);

	return TRUE;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean csv_import_parse_sub_budget (struct ImportTransaction *ope,
									  gchar *string)
{
	g_return_val_if_fail (string, FALSE);
	if (!ope->budget || !strlen (string))
		return FALSE;

	ope->budget = g_strconcat (ope->budget, " : ", string, NULL);

	return TRUE;
}

/**
 * TODO: Use  a real parsing
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean csv_import_parse_balance (struct ImportTransaction *ope,
								   gchar *string)
{
	if (!string)
		return FALSE;

	if (strlen (string) > 0)
	{
		ope->montant = utils_real_get_from_string (string);

		return TRUE;
	}

	return FALSE;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean csv_import_parse_credit (struct ImportTransaction *ope,
								  gchar *string)
{
	if (!string)
		return FALSE;

	if (strlen (string) > 0)
	{
		ope->montant = gsb_real_add (ope->montant, utils_real_get_from_string (string));
	}

	return TRUE;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean csv_import_parse_debit (struct ImportTransaction *ope,
								 gchar *string)
{
	if (!string)
		return FALSE;

	if (strlen (string) > 0)
	{
		ope->montant = gsb_real_sub (ope->montant, utils_real_get_from_string (string));
	}

	return TRUE;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean csv_import_parse_p_r (struct ImportTransaction *ope,
							   gchar *string)
{
	g_return_val_if_fail (string, FALSE);

	if (!strcmp (string, "P"))
	{
		ope->p_r = OPERATION_POINTEE;

		return TRUE;
	}
	else if (!strcmp (string, "T"))
	{
		ope->p_r = OPERATION_TELEPOINTEE;

		return TRUE;
	}
	else if (!strcmp (string, "R"))
	{
		ope->p_r = OPERATION_RAPPROCHEE;

		return TRUE;
	}
	else if (!strlen (string))
	{
		ope->p_r = OPERATION_NORMALE;

		return TRUE;
	}

	return FALSE;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean csv_import_parse_split (struct ImportTransaction *ope,
								 gchar *string)
{
	g_return_val_if_fail (string, FALSE);

	if (!strcmp (string, "V"))
	{
		ope->ope_de_ventilation = TRUE;

		return TRUE;
	}

	return FALSE;
}

/**
 * Valide et integre la chaine passée en paramètre
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean csv_import_parse_payment_method (struct ImportTransaction *ope,
										  gchar *string)
{
	g_return_val_if_fail (string, FALSE);

	if (ope->payment_method)
		g_free (ope->payment_method);
	ope->payment_method = my_strdup (string);

	return TRUE;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
