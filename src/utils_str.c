/* ************************************************************************** */
/*                                  utils_str.c                               */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cedric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*          2008-2018 Pierre Biava (grisbi@pierre.biava.name)                 */
/*          https://www.grisbi.org/                                            */
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
#include "config.h"
#endif

#include "include.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <glib/gstdio.h>
#include <math.h>

/*START_INCLUDE*/
#include "utils_str.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "gsb_data_report.h"
#include "gsb_data_currency.h"
#include "gsb_locale.h"
#include "gsb_real.h"
#include "structures.h"
#include "utils_real.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * Return a newly created strings, truncating original.  It should be
 * truncated at the end of the word containing the nth letter.
 *
 * \param string	String to truncate
 * \param n		Max lenght to truncate at.
 * \param hard_trunc	Cut in the middle of a word if needed.
 *
 * \return A newly-created string.
 **/
static gchar *gsb_string_truncate_n (gchar *string,
									 gint n,
									 gboolean hard_trunc)
{
	gchar* result;
    gchar *tmp;
    gchar *trunc;

    if (!string)
		return NULL;

    if ((gint) strlen(string) < n)
		return my_strdup (string);

    tmp = string + n;
    if (!hard_trunc && !(tmp = strchr (tmp, ' ')))
    {
		/* We do not risk splitting the string in the middle of a
		   UTF-8 accent ... the end is probably near btw. */
		return my_strdup (string);
    }
    else
    {
		while (!isascii(*tmp) && *tmp)
			tmp++;

		trunc = g_strndup (string, (tmp - string));
		result = g_strconcat (trunc, "...", NULL);
		g_free(trunc);
		return result;
    }
}

/**
 * renvoie une ligne de longueur maxi trunc en s'arrêtant sur un séparateur;
 *
 * \param nom du fichier
 * \param separateur si NULL regarde les autres séparateurs.
 * \param longueur maxi de la ligne
 *
 * \return une nouvelle chaîne contenant le nom sur une ligne.
 **/
static gchar *utils_string_get_ligne_longueur_fixe (const gchar *string,
													const gchar *separator,
													gint trunc)
{
    gchar *tmp_str = NULL;
    gchar *ptr = NULL;

    if (string == NULL)
        return NULL;

    if (separator)
    {
        if (g_str_has_suffix (string, separator))
            return g_strdup (string);

        ptr = g_strrstr (string, separator);
        if (ptr == NULL)
            return g_strdup (string);

        tmp_str = g_strndup (string, (ptr + 1 - string));

        return tmp_str;
    }
    else
        return g_strdup (string);
}

/**
 * retourne le séparateur s'il est parmi les connu
 *
 * \param
 *
 * \return une nouvelle chaîne contenant le sépaarateur.
 **/
static const gchar *utils_string_get_separator (const gchar *string)
{
    gchar *ptr_1 = NULL;
    gchar *ptr_2 = NULL;
    gchar *ptr_3 = NULL;
    size_t long_1 = 0;
    size_t long_2 = 0;
    size_t long_3 = 0;

    ptr_1 = g_strrstr (string, " ");
    ptr_2 = g_strrstr (string, "-");
    ptr_3 = g_strrstr (string, "_");

    if (ptr_1 == NULL && ptr_2 == NULL && ptr_3 == NULL)
        return NULL;

    if (ptr_1)
        long_1 = ptr_1 - string;
    if (ptr_2)
        long_2 = ptr_2 - string;
    if (ptr_3)
        long_3 = ptr_3 - string;

    if (long_1 > long_2)
    {
        if (long_1 > long_3)
            return " ";
        else
            return "_";
    }
    else
    {
        if (long_2 > long_3)
            return "-";
        else
            return "_";
    }

    return NULL;
}

/**
 * retourne le séparateur s'il est parmi les connus
 *
 * \param
 *
 * \return une nouvelle chaîne contenant le sépaarateur.
 **/
static gboolean utils_str_payee_name_is_trouve (const gchar *name,
												gchar **tab_str,
												gint is_prefix,
												gint is_suffix,
												gint ignore_case)
{
	guint nbre_items = 0;
	guint nbre_items_actifs = 0;
	guint trouve = 0;
	guint i;

	nbre_items = g_strv_length (tab_str);
    for (i = 0; i < nbre_items; i++)
	{
		if (!tab_str[i] || strlen (tab_str[i]) == 0)
        {
			continue;
		}
		nbre_items_actifs++;
		if (ignore_case)
		{
			gchar *tmp_name;
			gchar *tmp_str;

			if (g_utf8_validate (name, -1, NULL))
				tmp_name = g_utf8_strup (name, -1);
			else
				return FALSE;
			if (g_utf8_validate (tab_str[i], -1, NULL))
				tmp_str = g_utf8_strup (tab_str[i], -1);
			else
				return FALSE;

			if (i == 0 && is_prefix && g_str_has_prefix (tmp_name, tmp_str))
				trouve++;
			else if (i == nbre_items -1 && is_suffix && g_str_has_suffix (tmp_name, tmp_str))
				trouve++;
			else if (tmp_str && strlen (tmp_str) > 0)
			{
				if (g_strstr_len (tmp_name, -1, tmp_str))
				{
					trouve++;
				}
			}
			g_free (tmp_name);
			g_free (tmp_str);
		}
		else
		{
			if (i == 0 && is_prefix && g_str_has_prefix (name, tab_str[i]))
				trouve++;
			else if (i == nbre_items -1 && is_suffix && g_str_has_suffix (name, tab_str[i]))
				trouve++;
			else if (tab_str[i] && strlen (tab_str[i]) > 0)
			{
				if (g_strstr_len (name, -1, tab_str[i]))
				{
					trouve++;
				}
			}
		}
	}

	if (trouve == nbre_items_actifs)
		return TRUE;
	else
		return FALSE;
}
/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 * @brief convert an integer into a gchar based string
 *
 *
 * @param integer   integer to convert
 *
 * @return  a newly allocated string
 *
 * @caveats You have to unallocate the returned string when you no more use it to save memory
 *
 * @todo: check usage of this function which a cause of memory leak
 *
 **/
gchar *utils_str_itoa (gint integer)
{
    div_t result_div;
    gchar *chaine;
    gint i = 0;
    gint num;

    chaine = g_malloc0 (11*sizeof (gchar));

    num = abs(integer);

    /* Construct the result in the reverse order from right to left, then reverse it. */
    do
    {
		result_div = div (num, 10);
		chaine[i] = result_div.rem + '0';
		i++;
    }
    while ((num = result_div.quot));

    /* Add the sign at the end of the string just before to reverse it to avoid
     to have to insert it at the begin just after... */
    if (integer < 0)
    {
        chaine[i++] = '-';
    }

    chaine[i] = 0;

    g_strreverse (chaine);

    return (chaine);
}

/**
 * locates the decimal dot
 *
 * \param
 *
 * \return
 **/
gchar *utils_str_localise_decimal_point_from_string (const gchar *string)
{
    gchar *ptr_1;
    gchar *ptr_2;
    gchar *new_str;
    gchar *mon_decimal_point;
    gchar *mon_separateur;
    gchar **tab;

    mon_decimal_point = gsb_locale_get_mon_decimal_point ();
    mon_separateur = gsb_locale_get_mon_thousands_sep ();

    if ((ptr_1 = g_strstr_len (string, -1, ",")) &&
		(ptr_2 = g_strrstr (string, ".")))
    {
        if ((ptr_2 - string) > (ptr_1 - string))
            tab = g_strsplit (string, ",", 0);
        else
            tab = g_strsplit (string, ".", 0);

        new_str = g_strjoinv ("", tab);
        g_strfreev (tab);
    }
    else
        new_str = g_strdup (string);

    if (mon_decimal_point && g_strstr_len (new_str, -1, mon_decimal_point) == NULL)
    {
        tab = g_strsplit_set (new_str, ".,", 0);
        g_free (new_str);
        new_str = g_strjoinv (mon_decimal_point, tab);
        g_strfreev (tab);
    }

    if (mon_decimal_point)
        g_free (mon_decimal_point);

    if (mon_separateur && g_strstr_len (new_str, -1, mon_separateur))
    {
        tab = g_strsplit (new_str, mon_separateur, 0);
        g_free (new_str);
        new_str = g_strjoinv ("", tab);
        g_strfreev (tab);
    }

    if (mon_separateur)
        g_free (mon_separateur);

    return new_str;
}

/**
 * @brief Secured version of atoi
 *
 * Encapsulated call of atoi which may crash when it is call with a NULL pointer.
 *
 * @param chaine   pointer to the buffer containing the string to convert
 *
 * @return  the converted string as interger
 * @retval  0 when the pointer is NULL or the string empty.
 *
 **/
gint utils_str_atoi (const gchar *chaine)
{
    if ((chaine) && (*chaine))
    {
        return (atoi (chaine));
    }
    else
    {
        return (0);
    }
}

/**
 * do the same as g_strdelimit but new_delimiters can containes several characters or none
 * ex	my_strdelimit ("a-b", "-", "123") returns a123b
 * 	my_strdelimit ("a-b", "-", "") returns ab
 *
 * \param string the string we want to modify
 * \param delimiters the characters we need to change to new_delimiters
 * \param new_delimiters the replacements characters for delimiters
 *
 * \return a newly allocated string or NULL
 **/
gchar *my_strdelimit (const gchar *string,
					  const gchar *delimiters,
					  const gchar *new_delimiters)
{
    gchar **tab_str;
    gchar *retour;

    if (!(string && delimiters && new_delimiters))
	{
		return my_strdup (string);
	}

    tab_str = g_strsplit_set (string, delimiters, 0);
    retour = g_strjoinv (new_delimiters, tab_str);
    g_strfreev (tab_str);

    return (retour);
}

/**
 * compare 2 chaines sensitive que ce soit utf8 ou ascii
 *
 * \param
 * \param
 *
 * \return
 **/
gint my_strcmp (gchar *string_1,
				gchar *string_2)
{
    if (!string_1 && string_2)
	    return 1;
    if (string_1 && !string_2)
	    return -1;
    if (!string_1 && !string_2)
	    return 0;


	if (g_utf8_validate (string_1, -1, NULL) &&
		g_utf8_validate (string_2, -1, NULL))
	{
	    gint retour;
 	    gchar *new_1;
 	    gchar *new_2;

	    new_1 = g_utf8_collate_key (string_1, -1);
	    new_2 = g_utf8_collate_key (string_2, -1);
	    retour = strcmp (new_1, new_2);

	    g_free (new_1);
	    g_free (new_2);
	    return (retour);
	}
	else
	    return (strcmp (string_1, string_2));

    return 0;
}


/**
 * compare 2 strings unsensitive
 * if a string is NULL, it will go after the non NULL
 *
 * \param string_1
 * \param string_2
 *
 * \return 		-1 string_1 before string_2 (or string_2 NULL) ;
 * 				 0 if same or NULL everyone ;
 * 				+1 if string_1 after string_2 (or string_1 NULL)
 **/
gint my_strcasecmp (const gchar *string_1,
					const gchar *string_2)
{
    if (!string_1 && string_2)
	    return 1;
    if (string_1 && !string_2)
	    return -1;

    if (string_1  && string_2)
    {
        if (g_utf8_validate (string_1, -1, NULL) &&
			g_utf8_validate (string_2, -1, NULL))
        {
            gint retour;
			gchar *new_1;
			gchar *new_2;

            new_1 = g_utf8_casefold (string_1, -1);
            new_2 = g_utf8_casefold (string_2, -1);
            retour = g_utf8_collate (new_1, new_2);

            g_free (new_1);
            g_free (new_2);
            return (retour);
        }
        else
            return (g_ascii_strcasecmp (string_1, string_2));
    }

    return 0;
}

/**
 * compare 2 chaines case-insensitive que ce soit utf8 ou ascii
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
gint my_strncasecmp (const gchar *string_1,
					 const gchar *string_2,
					 gint longueur)
{
    if (!string_1 && string_2)
        return 1;
    if (string_1 && !string_2)
        return -1;

    if (string_1 && string_2)
    {
        if (g_utf8_validate (string_1, -1, NULL) &&
			g_utf8_validate (string_2, -1, NULL))
        {
            gint retour;
			gchar *new_1;
			gchar *new_2;

            new_1 = g_utf8_casefold (string_1,longueur);
            new_2 = g_utf8_casefold ( string_2,longueur);
            retour = g_utf8_collate (new_1, new_2);

            g_free (new_1);
            g_free (new_2);
            return (retour);
        }
        else
            return (g_ascii_strncasecmp (string_1, string_2, longueur));
    }

    return 0;
}

/**
 * Protect the my_strdup function if the string is NULL
 *
 * If the length of string is 0 (ie ""), return NULL.  That is just
 * nonsense, but it has been done that way and disabling it would
 * certainly cause side effects. [benj]
 *
 * \param string the string to be dupped
 *
 * \return a newly allocated string (which is a copy of that string)
 * 	or NULL if the parameter is NULL or an empty string.
 **/
gchar *my_strdup (const gchar *string)
{
    if (string && strlen (string))
		return g_strdup (string);
    else
		return NULL;
}

/**
 * check if the string is maximum to the length
 * if bigger, limit it and set ... at the end
 *
 * \param string the string to check
 * \param length the limit length we want
 *
 * \return a duplicate version of the string with max length character (must to be freed)
 **/
gchar *limit_string (const gchar *string,
					 gint length)
{
    gchar *string_return;
    gchar *tmp_str;

    if (!string)
		return NULL;

    if (g_utf8_strlen (string, -1) <= length)
		return my_strdup (string);

	tmp_str = g_malloc0 (length * 4);
    tmp_str = g_utf8_strncpy (tmp_str, string, length-3);

    string_return = g_strconcat (tmp_str, "...", NULL);
    g_free (tmp_str);

	return string_return;
}

/**
 * return a gslist of integer from a string which the elements are separated
 * by the separator
 *
 * \param string the string we want to change to a list
 * \param delimiter the string which is the separator in the list
 *
 * \return a g_slist or NULL
 **/
GSList *gsb_string_get_int_list_from_string (const gchar *string,
											 gchar *delimiter)
{
    GSList *list_tmp;
    gchar **tab;
    gint i=0;

    if (!string || !delimiter || !strlen (string) || !strlen (delimiter))
		return NULL;

    tab = g_strsplit (string, delimiter, 0);

    list_tmp = NULL;

    while (tab[i])
    {
		list_tmp = g_slist_append (list_tmp, GINT_TO_POINTER (atoi (tab[i])));
		i++;
    }

    g_strfreev (tab);

    return list_tmp;
}

/**
 * return a gslist of strings from a string which the elements are separated
 * by the separator
 * (same as gsb_string_get_int_list_from_string but with strings)
 *
 * \param string the string we want to change to a list
 * \param delimiter the string which is the separator in the list
 *
 * \return a g_slist or NULL
 **/
GSList *gsb_string_get_string_list_from_string (const gchar *string,
												gchar *delimiter)
{
    GSList *list_tmp;
    gchar **tab;
    gint i=0;

    if (!string || !delimiter || !strlen (string) || !strlen (delimiter))
		return NULL;

    tab = g_strsplit (string, delimiter, 0);

    list_tmp = NULL;

    while (tab[i])
    {
		list_tmp = g_slist_append (list_tmp, my_strdup  (tab[i]));
		i++;
    }

    g_strfreev (tab);

    return list_tmp;
}

/**
 * return a gslist of CategBudgetSel
 * from a string as no_categ/no_sub_categ/no_sub_categ/no_sub_categ-no_categ/no_sub_categ...
 * (or idem with budget)
 *
 * \param string	the string we want to change to a list
 *
 * \return a g_slist or NULL
 **/
GSList *gsb_string_get_categ_budget_struct_list_from_string (const gchar *string)
{
    GSList *list_tmp = NULL;
    gchar **tab;
    gint i=0;

    if (!string || !strlen (string))
		return NULL;

    tab = g_strsplit (string, "-", 0);

    while (tab[i])
    {
		CategBudgetSel *categ_budget_struct = NULL;
		gchar **sub_tab;
		gint j=0;

		sub_tab = g_strsplit (tab[i], "/", 0);
		while (sub_tab[j])
		{
			if (!categ_budget_struct)
			{
				/* no categ_budget_struct created, so we are on the category */
				categ_budget_struct = g_malloc0 (sizeof (CategBudgetSel));
				categ_budget_struct->div_number = utils_str_atoi(sub_tab[j]);
			}
			else
			{
				/* categ_budget_struct is created, so we are on sub-category */
				categ_budget_struct->sub_div_numbers = g_slist_append (categ_budget_struct->sub_div_numbers,
																	   GINT_TO_POINTER (utils_str_atoi (sub_tab[j])));
			}
			j++;
		}
		g_strfreev (sub_tab);
		list_tmp = g_slist_append (list_tmp, categ_budget_struct);
		i++;
    }
    g_strfreev (tab);

    return list_tmp;
}

/**
 * Return a newly created strings, truncating original.  It should be
 * truncated at the end of the word containing the 20th letter.
 *
 * \param string	String to truncate.
 *
 * \return A newly-created string.
 */
gchar * gsb_string_truncate (gchar * string)
{
    return gsb_string_truncate_n (string, 20, FALSE);
}


/**
 * remplace la chaine old_str par new_str dans str
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
gchar *gsb_string_remplace_string (const gchar *str,
								   const gchar *old_str,
								   const gchar *new_str)
{
    gchar *ptr_debut;
    size_t long_old, str_len;
    gchar *chaine, *ret, *tail;

    ptr_debut = g_strstr_len (str, -1, old_str);
    if (ptr_debut == NULL)
        return g_strdup (str);

    str_len = strlen (str);
    long_old = strlen (old_str);

    chaine = g_strndup (str, (ptr_debut - str));

    tail = ptr_debut + long_old;
    if (tail >= str + str_len)
        ret = g_strconcat (chaine, new_str, NULL);
    else
        ret = g_strconcat (chaine, new_str, tail, NULL);

    g_free (chaine);

    return ret;
}

/**
 * recherche des mots séparés par des jokers "%*" dans une chaine
 *
 * \param haystack
 * \param needle
 *
 * \return TRUE si trouvé FALSE autrement
 **/
gboolean gsb_string_is_trouve (const gchar *payee_name,
							   const gchar *needle,
							   gint ignore_case,
							   gint use_regex)
{
	if (use_regex)
	{
		dialogue_warning ("Function not available");
	}
	else
	{
    	gchar **tab_str;
		gboolean is_prefix = FALSE;
		gboolean is_suffix = FALSE;
		gboolean trouve = FALSE;

		if (g_strstr_len (needle, -1, "%") == NULL && g_strstr_len (needle, -1, "*") == NULL)
    	{
			if (my_strcasecmp (payee_name, needle) == 0)
				return TRUE;
			else
				return FALSE;
    	}
		if (g_strstr_len (needle, -1, "||"))
		{
			gchar **tab_rules;
			guint nbre_items = 0;
			guint i;

			tab_rules = g_strsplit (needle, "||", 0);
			nbre_items = g_strv_length (tab_rules);
			for (i = 0; i < nbre_items; i++)
			{
				/* is_prefix = TRUE si needle ne commence pas par % */
				if (g_str_has_prefix (tab_rules[i], "%") == FALSE && g_str_has_prefix (tab_rules[i], "*") == FALSE)
        			is_prefix = TRUE;

				if (g_str_has_suffix (tab_rules[i], "%") == FALSE && g_str_has_suffix (tab_rules[i], "*") == FALSE)
        			is_suffix = TRUE;

				tab_str = g_strsplit_set (tab_rules[i], "%*", 0);	/* modèle = %chaine% */
				trouve = utils_str_payee_name_is_trouve (payee_name, tab_str, is_prefix, is_suffix, ignore_case);
				g_strfreev (tab_str);
				if (trouve)
					break;
    		}
			return trouve;
    	}
		if (g_str_has_prefix (needle, "%") == FALSE && g_str_has_prefix (needle, "*") == FALSE)
			is_prefix = TRUE;

		/* is_suffix = TRUE si needle ne commence pas par % */
		if (g_str_has_suffix (needle, "%") == FALSE && g_str_has_suffix (needle, "*") == FALSE)
			is_suffix = TRUE;

    	tab_str = g_strsplit_set (needle, "%*", 0);
		trouve = utils_str_payee_name_is_trouve (payee_name, tab_str, is_prefix, is_suffix, ignore_case);
		g_strfreev (tab_str);

		return trouve;
 	}

    return FALSE;
}

/**
 * remplace les jokers "%*" par une chaine
 *
 * \param str
 * \param new_str
 *
 * \return chaine avec chaine de remplacement
 **/
gchar * gsb_string_remplace_joker (const gchar *chaine,
								   gchar *new_str)
{
    gchar **tab_str;
    gchar *result;

    tab_str = g_strsplit_set (chaine, "%*", 0);
    result = g_strjoinv (new_str, tab_str);
    g_strfreev (tab_str);

    return result;
}

/*
 * extrait un nombre d'une chaine
 *
 * \param chaine
 *
 * \return a string representing a number
 **/
gchar *gsb_string_extract_int (const gchar *chaine)
{
    gchar *ptr;
    gchar *tmpstr;
    gunichar ch;
    gint i = 0;
    gint long_nbre = 64;

    tmpstr = g_malloc0 (long_nbre * sizeof (gchar));
    ptr = (gchar*) chaine;
    while (g_utf8_strlen (ptr, -1) > 0)
    {
        ch = g_utf8_get_char_validated (ptr, -1);
        if (g_unichar_isdefined (ch) && g_ascii_isdigit (ch))
        {
            if (i == long_nbre)
                break;
            tmpstr[i] = ptr[0];
            i++;
        }
        ptr = g_utf8_next_char (ptr);
    }

	return tmpstr;
}

/**
 * uniformisation des CR+LF dans les fichiers importés
 *
 * \param chaine
 *
 * \return chaine au format unix
 **/
gchar *gsb_string_uniform_new_line (const gchar *chaine,
									gssize nbre_char)
{
    gchar **tab_str = NULL;
    gchar *result = NULL;

    if (chaine == NULL)
        return NULL;

    if (g_strstr_len (chaine, nbre_char, "\r\n"))
    {
        tab_str = g_strsplit_set (chaine, "\r", 0);
        result = g_strjoinv ("", tab_str);
    }
    else if (g_strstr_len (chaine, nbre_char, "\r") &&
			 !g_strstr_len (chaine, nbre_char, "\n"))
    {
        tab_str = g_strsplit_set (chaine, "\r", 0);
        result = g_strjoinv ("\n", tab_str);
    }
    else if (g_strstr_len (chaine, nbre_char, "\n"))
        result = g_strdup (chaine);

    g_strfreev (tab_str);
    return result;
}

/*
 *
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
gchar *utils_str_dtostr (gdouble number,
						 gint nbre_decimal,
						 gboolean canonical)
{
    gchar buffer[G_ASCII_DTOSTR_BUF_SIZE];
    gchar *str_number;
    gchar *decimal;
    gchar *format;
    gint nbre_char;

    decimal = utils_str_itoa (nbre_decimal);
    format = g_strconcat ("%.", decimal, "f", NULL);
    nbre_char = g_sprintf (buffer, format, number);
    g_free (decimal);
    g_free (format);

    if (nbre_char > G_ASCII_DTOSTR_BUF_SIZE)
        return NULL;

    str_number = g_strndup (buffer, nbre_char);

    if (canonical && g_strrstr (str_number, ","))
    {
        gchar *tmp_str;

        tmp_str = my_strdelimit (str_number, ",", ".");
        g_free (str_number);

        return tmp_str;
    }
    else
        return str_number;
}

/**
 * fonction de conversion de char à double pour chaine avec un . comme séparateur décimal
 * et pas de séparateur de milliers
 *
 * \param
 * \param
 *
 * \return
 **/
gdouble utils_str_safe_strtod (const gchar *str_number,
							   gchar **endptr)
{
    gdouble number;

    if (str_number == NULL)
        return 0.0;

    number = g_ascii_strtod (str_number, endptr);

    return number;
}

/**
 * fonction de conversion de char à double pour chaine en tenant compte du séparateur décimal
 * et du séparateur de milliers configurés dans les préférences.
 *
 * \param
 * \param
 *
 * \return
 **/
gdouble utils_str_strtod (const gchar *str_number,
						  gchar **endptr)
{
    gdouble number;
    GsbReal real;

    if (str_number == NULL)
        return 0.0;

    real = utils_real_get_from_string (str_number);

    number = gsb_real_real_to_double (real);

    return number;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gint utils_str_get_nbre_motifs (const gchar *chaine,
								const gchar *motif)
{
    gchar **tab_str;
    gint nbre_motifs = 0;

    if (chaine == NULL || motif == NULL)
        return -1;

    tab_str = g_strsplit (chaine, motif, 0);
    nbre_motifs = g_strv_length (tab_str) -1;
    g_strfreev (tab_str);

    return nbre_motifs;
}

/**
 * adapte l'utilisation de : en fonction de la langue de l'utilisateur
 *
 * \param
 * \param
 *
 * \return
 **/
gchar *utils_str_incremente_number_from_str (const gchar *str_number,
											 gint increment)
{
    gchar *new_str_number;
    gchar *prefix = NULL;
    gint number = 0;
    gint i = 0;

    if (str_number && strlen (str_number) > 0)
    {
        while (str_number[i] == '0')
        {
            i++;
        }
        if (i > 0)
            prefix = g_strndup (str_number, i);

        number = utils_str_atoi (str_number);
    }

    number += increment;

    new_str_number = utils_str_itoa (number);

    if (prefix && strlen (prefix) > 0)
    {
        new_str_number = g_strconcat (prefix, new_str_number, NULL);
        g_free (prefix);
    }

    return new_str_number;
}

/**
 * coupe le nom des fichiers passé en paramètre
 * quelque soit la longueur du nom
 *
 * \param nom du fichier
 * \param longueur maxi de la ligne
 *
 * \return une nouvelle chaîne contenant le nom sur une ou plusieurs lignes.
 **/
gchar *utils_str_break_filename (const gchar *string,
	                             gint trunc)
{
    gchar *tmp_dir = NULL;
    gchar *basename = NULL;
    gchar *tmp_base = NULL;
    gchar *tmp_str1 = NULL;
    gchar *tmp_str2;
    gchar *end = NULL;
    gchar *ptr = NULL;
    const gchar *separator;
    gint i = 0;
    ssize_t n = 0;
    ssize_t size1;
    ssize_t size2;

    if ((gint) g_utf8_strlen (string, -1) <= trunc)
        return g_strdup (string);

    basename = g_path_get_basename (string);
    size1 = g_utf8_strlen (basename, -1);

	/* maxi GSB_NBRE_LIGNES_BOUTON (6) lignes */
	n = GSB_NBRE_LIGNES_BOUTON;
	if (size1 > n*trunc+1)
	{
		tmp_str1 = g_malloc0 (n*trunc+1);
		tmp_str1 = g_utf8_strncpy (tmp_str1, basename, n*trunc);
		size1 = g_utf8_strlen (tmp_str1, -1);
		g_free (basename);
		basename = tmp_str1;
	}

	tmp_base = g_malloc0 (size1 + n);
	tmp_base = g_utf8_strncpy (tmp_base, basename, trunc);

	separator = utils_string_get_separator (tmp_base);
	tmp_str1 = utils_string_get_ligne_longueur_fixe (tmp_base, separator, trunc);
	g_free (tmp_base);
	tmp_base = tmp_str1;
	size2 = g_utf8_strlen (tmp_base, -1);
	i = 1;
	do
	{
		end = g_utf8_offset_to_pointer (basename, size2);

		if (size2 + trunc <= size1)
			ptr = g_utf8_offset_to_pointer (basename, (size2 + trunc));

		if (ptr)
		{
			tmp_str1 = g_strndup (end, (ptr - end));
			separator = utils_string_get_separator (tmp_str1);
			tmp_str2 = utils_string_get_ligne_longueur_fixe (tmp_str1, separator, trunc);
			size2 += g_utf8_strlen (tmp_str2, -1);
			g_free (tmp_str1);
			tmp_str1 = g_strconcat (tmp_base, "\n", tmp_str2, NULL);
			g_free (tmp_base);
			g_free (tmp_str2);
			tmp_base = tmp_str1;
		}
		else
		{
			tmp_str1 = g_strconcat (tmp_base, "\n", end, NULL);
			g_free (tmp_base);
			tmp_base = tmp_str1;
			break;
		}

		ptr = NULL;
		i++;
	} while (i < n);

    g_free (tmp_dir);
	g_free (basename);

    /* return */
    return tmp_base;
}

/**
 * protect the new line char to save text
 *
 * \param	text to protect or to unprotect
 * \param	TRUE protect FALSE unprotect
 *
 * \return a newly string which should be freed with g_free().
 **/
gchar *utils_str_protect_unprotect_multilines_text (const gchar *text,
													 gboolean protect)
{
	gchar *tmp_str;
	gchar **tmp_tab;

	tmp_str = g_strdup (text);
	if (protect)
	{
		if (tmp_str && g_strstr_len (tmp_str, -1, NEW_LINE))
		{
			tmp_tab = g_strsplit (tmp_str, NEW_LINE, 0);
			g_free (tmp_str);
			tmp_str = g_strjoinv ("\\n", tmp_tab);

			g_strfreev (tmp_tab);
		}
	}
	else
	{
		if (g_strstr_len (tmp_str, -1, "\\n" ) )
		{
			tmp_tab = g_strsplit (tmp_str, "\\n", 0);
			g_free (tmp_str);
			tmp_str = g_strjoinv (NEW_LINE, tmp_tab);

			g_strfreev (tmp_tab);
		}
	}

	return tmp_str;
}

/**
 * supprime les accents dans une phrase.
 *
 * \param 	text à corriger
 *
 * \return a new string
 **/
gchar *utils_str_remove_accents (const gchar *text)
{
	gchar *new_text;
    gchar *ptr;
    gchar *tmp_str;
    gunichar ch;
    gulong i = 0;
    size_t nbre_bytes;
    gulong nbre_chars;

	if (!text)
		return NULL;

	new_text = g_utf8_normalize (text, -1, G_NORMALIZE_ALL);
	nbre_bytes = strlen (new_text);
	nbre_chars = g_utf8_strlen (new_text, -1);

	if (nbre_bytes == nbre_chars)
	{
		g_free (new_text);

		return g_strdup (text);
	}

    tmp_str = g_malloc0 (nbre_bytes * sizeof (gchar));
    ptr = (gchar*) new_text;
    while (g_utf8_strlen (ptr, -1) > 0)
    {
		ch = g_utf8_get_char_validated (ptr, -1);
		if (g_unichar_isdefined (ch) && g_ascii_isalpha (ch))
		{
			if (i == nbre_chars)
				break;
			tmp_str[i] = ptr[0];
			i++;
		}
        ptr = g_utf8_next_char (ptr);
    }

	g_free (new_text);

	return tmp_str;
}

/**
 *	recherche needle après suppression des accents et mise en majuscule
 *
 * \param 	chaine pour la recherche
 * \param	chaine recherchée
 *
 * \return un ptr vers la première occurence
 **/
gchar *utils_str_my_case_strstr (const gchar *haystack,
								 const gchar *needle)
{
	gchar *new_haystack;
	gchar *new_needle;
	gchar *str_to_free;
	gchar *ptr;

	/* uniformise needle */
	str_to_free = utils_str_remove_accents (needle);
	new_needle = g_ascii_strup (str_to_free, -1);
	g_free (str_to_free);

	/* uniformise needle */
	str_to_free = utils_str_remove_accents (haystack);
	new_haystack = g_ascii_strup (str_to_free, -1);
	g_free (str_to_free);

	ptr = g_strdup (g_strstr_len (new_haystack, -1, new_needle));
	g_free (new_haystack);
	g_free (new_needle);

	return ptr;
}

/**
 * Remplace un espace par un "\n" à partir de nbre de caractères
 *
 * \param
 * \param
 *
 * \return
 **/
gchar *utils_str_break_form_name_field (const gchar *text,
										gint trunc)
{
	gchar *tmp_str = NULL;
	glong nbre_char;

	if (!text)
		return NULL;

	if (!grisbi_app_get_low_resolution_screen ())
	{
		tmp_str = g_strdup (text);
		return (tmp_str);
	}

	if (g_utf8_validate (text, -1, NULL))
	{
		nbre_char = g_utf8_strlen (text, -1);
		if (nbre_char <= trunc)
			return g_strdup (text);

		if (g_utf8_strchr (text, -1, ':'))
		{
			gchar **tab;

			tab= g_strsplit (text, ":", 2);
			tmp_str = g_strconcat (tab[0], ":\n", tab[1]+1, NULL);
		}
		else if (g_utf8_strchr (text, -1, '|'))
		{
			gchar **tab;

			tab= g_strsplit (text, "||", 0);
			tmp_str = g_strjoinv ("||\n", tab);
			g_strfreev (tab);
		}
		else if (g_utf8_strchr (text, -1, ' '))
		{
			gchar *tmp_str2 = NULL;
			gchar *ptr_free;
			gchar *ptr_free2;
			gchar **tab;
			gint i;
			gint nbre_champs;

			tab= g_strsplit (text, " ", 0);
			nbre_champs = g_strv_length (tab);

			for (i = 0; tab[i]; i++)
			{
				if (i == 0)
				{
					tmp_str = g_strdup (tab[i]);
					nbre_char = g_utf8_strlen (tmp_str, -1);
					if (nbre_char > trunc-1)
					{
						ptr_free = tmp_str;
						tmp_str = g_strconcat (tmp_str, "\n", NULL);
						g_free (ptr_free);
					}
					else
					{
						if (tab[i+1])
						{
							nbre_char = g_utf8_strlen (tmp_str, -1) + g_utf8_strlen (tab[i+1], -1) + 1;
							if (nbre_char < trunc)
								i++;
							while (nbre_char < trunc)
							{
								ptr_free = tmp_str;
								if (g_utf8_strlen (tab[i], -1) == 0)
									i++;
								else
								{
									tmp_str = g_strconcat (tmp_str, " ", tab[i], NULL);
									g_free (ptr_free);
								}
								if (!tab[i+1])
									break;

								nbre_char = g_utf8_strlen (tmp_str, -1) + g_utf8_strlen (tab[i+1], -1) + 1;
								if (nbre_char < trunc)
									i++;
							};
						}
						if (i < nbre_champs-1)
						{
							ptr_free = tmp_str;
							tmp_str = g_strconcat (tmp_str, "\n", NULL);
							g_free (ptr_free);
						}
					}
				}
				else
				{
					tmp_str2 = g_strdup (tab[i]);
					nbre_char = g_utf8_strlen (tmp_str2, -1);
					if (nbre_char > trunc-1)
					{
						ptr_free = tmp_str;
						ptr_free2 = tmp_str2;
						tmp_str = g_strconcat (tmp_str, tmp_str2, "\n", NULL);
						g_free (ptr_free);
						g_free (ptr_free2);
					}
					else
					{
						if (tab[i+1])
						{
							nbre_char = g_utf8_strlen (tmp_str2, -1) + g_utf8_strlen (tab[i+1], -1) + 1;
							if (nbre_char < trunc)
								i++;
							while (nbre_char < trunc)
							{
								ptr_free2 = tmp_str2;
								if (g_utf8_strlen (tab[i], -1) == 0)
									i++;
								else
								{
									tmp_str2 = g_strconcat (tmp_str2, " ", tab[i], NULL);
									g_free (ptr_free2);
								}
								if (!tab[i+1])
									break;

								nbre_char = g_utf8_strlen (tmp_str2, -1) + g_utf8_strlen (tab[i+1], -1) + 1;
								if (nbre_char < trunc)
									i++;
							};
						}
						ptr_free = tmp_str;
						ptr_free2 = tmp_str2;
						if (i < nbre_champs-1)
						{
							tmp_str = g_strconcat (tmp_str, tmp_str2, "\n", NULL);
						}
						else
						{
							tmp_str = g_strconcat (tmp_str, tmp_str2, NULL);
						}
						g_free (ptr_free);
						g_free (ptr_free2);
					}
				}
			}
			g_strfreev (tab);
		}
		else if (g_utf8_strchr (text, -1, '/'))
		{
			gchar *ptr;

			/* on recherche le dernier slash */
			ptr = g_utf8_strrchr (text, -1, '/');
			ptr[0] = '\n';
		}
		else
		{
			tmp_str = g_strdup (text);
		}
	}
	else
	{
		tmp_str = g_strdup (text);
	}

	return tmp_str;
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
