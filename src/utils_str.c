/* ************************************************************************** */
/*                                  utils_str.c                               */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cedric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*          2008-2010 Pierre Biava (grisbi@pierre.biava.name)                 */
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

#include "include.h"
#include <stdlib.h>

/*START_INCLUDE*/
#include "utils_str.h"
#include "./include.h"
#include "./gsb_data_report.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gchar *gsb_string_supprime_joker ( const gchar *chaine );
static gchar * gsb_string_truncate_n ( gchar * string, int n, gboolean hard_trunc );
/*END_STATIC*/


/*START_EXTERN*/
extern gint max;
/*END_EXTERN*/


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
 * */
gchar *utils_str_itoa ( gint integer )
{
    div_t result_div;
    gchar *chaine;
    gint i = 0;
    gint num;

    chaine = g_malloc0 ( 11*sizeof (gchar) );
    
    num = abs(integer);

    // Construct the result in the reverse order from right to left, then reverse it.
    do
    {
	result_div = div ( num, 10 );
	chaine[i] = result_div.rem + '0'; 
	i++;
    }
    while ( ( num = result_div.quot ));

    // Add the sign at the end of the string just before to reverse it to avoid
    // to have to insert it at the begin just after...
    if (integer < 0)
    {
        chaine[i++] = '-';
    }
    
    chaine[i] = 0;

    g_strreverse ( chaine );

    return ( chaine );
}


/**
 * used for now only while loading a file before the 0.6.0 version
 * reduce the exponant IN THE STRING of the amount because before the 0.6.0
 * all the gdouble where saved with an exponent very big
 * ie : in the file we have "12.340000"
 * and that fonction return "12.34" wich will be nicely imported
 * with gsb_data_transaction_set_amount_from_string
 *
 * \param amount_string
 * \param exponent the exponent we want at the end (normally always 2, but if ever...)
 *
 * \return a newly allocated string with only 'exponent' digits after the separator (need to be freed). This function returns NULL if the amount_string parameter is NULL.
 * */
gchar *utils_str_reduce_exponant_from_string ( const gchar *amount_string,
                        gint exponent )
{
    gchar *return_string;
    gchar *p;
    gchar *mon_decimal_point;
    gunichar decimal_point = (gunichar )-2;
    struct lconv *conv = localeconv ( );

    if ( !amount_string )
	    return NULL;

    mon_decimal_point = g_locale_to_utf8 ( conv->mon_decimal_point,
                        -1, NULL, NULL, NULL );
    if ( mon_decimal_point )
        decimal_point = g_utf8_get_char_validated ( mon_decimal_point, -1 );

    return_string = my_strdup ( amount_string );

    if ( ( p = g_utf8_strrchr ( (const gchar *) return_string, -1, '.' ) ) )
    {
        if ( g_unichar_isdefined ( decimal_point )
         &&
         g_utf8_strchr ( p, 1, decimal_point ) == NULL )
        {
            gchar **tab;

            tab = g_strsplit ( return_string, ".", 2 );
            return_string = g_strjoinv ( mon_decimal_point, tab );
            g_strfreev ( tab );
            p = g_utf8_strrchr ( (const gchar *) return_string, -1,
                        decimal_point );
        }
    }
    else if ( ( p = g_utf8_strrchr ( (const gchar *) return_string, -1, ',' ) ) )
    {
        if ( g_unichar_isdefined ( decimal_point )
         &&
         g_utf8_strchr ( p, 1, decimal_point ) == NULL )
        {
            gchar **tab;

            tab = g_strsplit ( return_string, ",", 2 );
            return_string = g_strjoinv ( mon_decimal_point, tab );
            g_strfreev ( tab );
            p = g_utf8_strrchr ( (const gchar *) return_string, -1,
                        decimal_point );
        }
    }
    else
        return NULL;

    p[exponent + 1] = '\0';

    return return_string;
}


/**
 * locates the decimal dot 
 *
 *
 *
 * */
gchar *utils_str_localise_decimal_point_from_string ( const gchar *string )
{
    struct lconv *conv = localeconv ( );
    gchar *ptr_1, *ptr_2;
    gchar *new_str;
    gchar *mon_decimal_point;
    gchar **tab;

    mon_decimal_point = g_locale_to_utf8 ( conv->mon_decimal_point, -1, NULL, NULL, NULL );

    if ( ( ptr_1 = g_strstr_len ( string, -1, "," ) )
     &&
     ( ptr_2 = g_strrstr ( string, "." ) ) )
    {
        if ( ( ptr_2 - string ) > ( ptr_1 - string ) )
            tab = g_strsplit ( string, ",", 0 );
        else
            tab = g_strsplit ( string, ".", 0 );

        new_str = g_strjoinv ( "", tab );
        g_strfreev ( tab );
    }
    else
        new_str = g_strdup ( string );

    if ( g_strstr_len ( new_str, -1, mon_decimal_point ) == NULL )
    {
        tab = g_strsplit_set ( new_str, ".,", 0 );
        g_free ( new_str );
        new_str = g_strjoinv ( mon_decimal_point, tab );
        g_strfreev ( tab );
    }

    g_free ( mon_decimal_point );

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
 * */
G_MODULE_EXPORT gint utils_str_atoi ( const gchar *chaine )
{
    if ((chaine )&&(*chaine))
    {
        return ( atoi ( chaine ));
    }
    else
    {
        return ( 0 );
    }
}


/**
 * Fonction my_strtod (string to decimal)
 * Convertie une chaine de caractères en un nombre
 * Paramètres d'entrée :
 *   - nptr : pointeur sur la chaine de caractères à convertir
 *   - endptr : n'est pas utilisé, alors à quoi peut-il bien servir ?
 * Valeur de retour :
 *   - resultat : le résultat de la conversion
 * Variables locales :
 *   - entier : la partie entière du résultat
 *   - mantisse : la partie décimale du résultat
 *   - invert : le signe du résultat (0 -> positif, 1 -> négatif)
 *   - p, m : pointeurs locaux sur la chaine de caractères à convertir
 * */
double my_strtod ( const char *nptr, const char **endptr )
{
    double entier=0, mantisse=0, resultat=0;
    int invert = 0;
    const char * p;

    if (!nptr)
	return 0;


    for ( p = nptr; p < nptr + strlen(nptr); p++ )
    {
	if (g_ascii_isspace(*p) || *p == '+' )
	    continue;

	if (*p == '-')
	{
	    invert = 1;
	    continue;
	}

	if ( *p == ',' || *p == '.' )
	{
	    const char * m;
	    for ( m = p+1; m <= nptr+strlen(nptr) && 
		  (g_ascii_isdigit(*m) || g_ascii_isspace(*m)); m++)
		/* Nothing, just loop */ ;
	    for ( --m; m > p; m-- )
	    {
		if (isdigit(*m))
		{
		    mantisse /= 10;
		    mantisse += (*m - '0');
		}
	    }
	    mantisse /= 10;
	}

	if (isdigit(*p))
	{
	    entier = entier * 10;
	    entier += (*p - '0');
	}
	else
	{
	    break;
	}
    }

    resultat = entier + mantisse;
    if ( invert )
	resultat = - resultat;

    return resultat;
}


/**
 *
 *
 *
 *
 * */
G_MODULE_EXPORT gchar * latin2utf8 ( const gchar * inchar)
{
    return g_locale_from_utf8 ( inchar, -1, NULL, NULL, NULL );
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
 * */
gchar *my_strdelimit ( const gchar *string,
                        const gchar *delimiters,
                        const gchar *new_delimiters )
{
    gchar **tab_str;
    gchar *retour;

    if ( !( string
	    &&
	    delimiters
	    &&
	    new_delimiters ))
	return my_strdup (string);

    tab_str = g_strsplit_set ( string,
			       delimiters,
			       0 );
    retour = g_strjoinv ( new_delimiters,
			  tab_str );
    g_strfreev ( tab_str );

    return ( retour );
}


/**
 * compare 2 chaines sensitive que ce soit utf8 ou ascii
 *
 *
 *
 * */
gint my_strcmp ( gchar *string_1, gchar *string_2 )
{
    if ( string_1
	 &&
	 string_2 )
    {
	if ( g_utf8_validate ( string_1, -1, NULL )
	     &&
	     g_utf8_validate ( string_2, -1, NULL ))
	{
	    gint retour;
 	    gchar *new_1, *new_2;
	    
	    new_1 = g_utf8_collate_key ( string_1,
					 -1 );
	    new_2 = g_utf8_collate_key ( string_2,
					 -1 );
	    retour = strcmp ( new_1,
			      new_2 );
	    g_free ( new_1 );
	    g_free ( new_2 );
	    return ( retour );
	}
	else
	    return ( strcmp ( string_1,
			      string_2 ));
    }

    return 0;
}


/**
 * do the same as g_strcasecmp but works alse with the accents on the words
 * 
 * \param string_1 the first string to cmp
 * \param string_2 the second string to cmp
 * 
 * \return -1 if string_1 berfore string_2
 * */
gint gsb_strcasecmp ( gchar *string_1,
                        gchar *string_2 )
{
	gint result;

    if (!string_1 && string_2)
	    return -1;
    if (string_1 && !string_2)
	    return 1;

    string_1 = my_strdup ( string_1 );
    string_1 = g_strdelimit ( string_1, "éÉèÈêÊ", 'e' );
    string_1 = g_strdelimit ( string_1, "çÇ", 'c' );
    string_1 = g_strdelimit ( string_1, "àÀ", 'a' );
    string_1 = g_strdelimit ( string_1, "ùûÙÛ", 'u' );
    string_1 = g_strdelimit ( string_1, "ôÔ", 'o' );
    string_1 = g_strdelimit ( string_1, "îÎ", 'i' );

    string_2 = my_strdup ( string_2 );
    string_2 = g_strdelimit ( string_2, "éÉèÈêÊ", 'e' );
    string_2 = g_strdelimit ( string_2, "çÇ", 'c' );
    string_2 = g_strdelimit ( string_2, "àÀ", 'a' );
    string_2 = g_strdelimit ( string_2, "ùûÙÛ", 'u' );
    string_2 = g_strdelimit ( string_2, "ôÔ", 'o' );
    string_2 = g_strdelimit ( string_2, "îÎ", 'i' );

    result = g_ascii_strcasecmp ( string_1, string_2 );
    g_free(string_1);
    g_free(string_2);

    return result; 
}


/**
 * compare 2 strings unsensitive
 * if a string is NULL, it will go after the non NULL
 *
 * \param string_1
 * \param string_2
 *
 * \return -1 string_1 before string_2 (or string_2 NULL) ; 0 if same or NULL everyone ; +1 if string_1 after string_2 (or string_1 NULL)
 * */
gint my_strcasecmp ( const gchar *string_1,
                        const gchar *string_2 )
{
    if (!string_1 && string_2)
	return 1;
    if (string_1 && !string_2)
	return -1;
    if ( string_1
	 &&
	 string_2 )
    {
	if ( g_utf8_validate ( string_1, -1, NULL )
	     &&
	     g_utf8_validate ( string_2, -1, NULL ))
	{
	    gint retour;
 	    gchar *new_1, *new_2;
	    
	    new_1 = g_utf8_collate_key ( g_utf8_casefold ( string_1,-1 ),
					 -1 );
	    new_2 = g_utf8_collate_key ( g_utf8_casefold (  string_2,-1 ),
					 -1 );
	    retour = strcmp ( new_1,
			      new_2 );
	    g_free ( new_1 );
	    g_free ( new_2 );
	    return ( retour );
	}
	else
	    return ( g_ascii_strcasecmp ( string_1,
					  string_2 ));
    }

    return 0;
}


/**
 * compare 2 chaines case-insensitive que ce soit utf8 ou ascii
 *
 *
 *
 * */
gint my_strncasecmp ( gchar *string_1,
                        gchar *string_2,
                        gint longueur )
{
    if ( string_1
	 &&
	 string_2 )
    {
	if ( g_utf8_validate ( string_1, -1, NULL )
	     &&
	     g_utf8_validate ( string_2, -1, NULL ))
	{
	    gint retour;
 	    gchar *new_1, *new_2;
	    
	    new_1 = g_utf8_casefold ( string_1,longueur );
	    new_2 = g_utf8_casefold (  string_2,longueur );
	    retour = g_utf8_collate ( new_1, new_2);

	    g_free ( new_1 );
	    g_free ( new_2 );
	    return ( retour );
	}
	else
	    return ( g_ascii_strncasecmp ( string_1,
					   string_2,
					   longueur ));
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
 * or NULL if the parameter is NULL or an empty string. 
 * */
G_MODULE_EXPORT gchar *my_strdup ( const gchar *string )
{
    if ( string && strlen (string) )
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
 * \return a dupplicate version of the string with max length character (must to be freed)
 * */
gchar *limit_string ( gchar *string,
                        gint length )
{
    gchar *string_return;
    gchar *tmpstr;
    gint i;

    if ( !string )
	return NULL;

    if ( g_utf8_strlen ( string, -1 ) <= length )
	return my_strdup (string);

    string_return = my_strdup ( string );
    tmpstr = string_return;
    for (i=0 ; i<(length-3) ; i++)
	tmpstr = g_utf8_next_char (tmpstr);

    tmpstr[0] = '.';
    tmpstr = g_utf8_next_char (tmpstr);
    tmpstr[0] = '.';
    tmpstr = g_utf8_next_char (tmpstr);
    tmpstr[0] = '.';
    tmpstr = g_utf8_next_char (tmpstr);
    tmpstr[0] = 0;

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
 * */
GSList *gsb_string_get_int_list_from_string ( const gchar *string,
                        gchar *delimiter )
{
    GSList *list_tmp;
    gchar **tab;
    gint i=0;

    if ( !string
	 ||
	 !delimiter
	 ||
	 !strlen (string)
	 ||
	 !strlen (delimiter))
	return NULL;

    tab = g_strsplit ( string,
		       delimiter,
		       0 );

    list_tmp = NULL;

    while ( tab[i] )
    {
	list_tmp = g_slist_append ( list_tmp,
				    GINT_TO_POINTER ( atoi (tab[i])));
	i++;
    }

    g_strfreev ( tab );

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
 * */
GSList *gsb_string_get_string_list_from_string ( const gchar *string,
                        gchar *delimiter )
{
    GSList *list_tmp;
    gchar **tab;
    gint i=0;

    if ( !string
	 ||
	 !delimiter
	 ||
	 !strlen (string)
	 ||
	 !strlen (delimiter))
	return NULL;

    tab = g_strsplit ( string,
		       delimiter,
		       0 );

    list_tmp = NULL;

    while ( tab[i] )
    {
	list_tmp = g_slist_append ( list_tmp,
				    my_strdup  (tab[i]));
	i++;
    }

    g_strfreev ( tab );

    return list_tmp;
}


/**
 * return a gslist of struct_categ_budget_sel
 * from a string as no_categ/no_sub_categ/no_sub_categ/no_sub_categ-no_categ/no_sub_categ... 
 * (or idem with budget)
 *
 * \param string	the string we want to change to a list
 *
 * \return a g_slist or NULL
 * */
GSList *gsb_string_get_categ_budget_struct_list_from_string ( const gchar *string )
{
    GSList *list_tmp = NULL;
    gchar **tab;
    gint i=0;

    if ( !string
	 ||
	 !strlen (string))
	return NULL;

    tab = g_strsplit ( string,
		       "-",
		       0 );

    while ( tab[i] )
    {
	struct_categ_budget_sel *categ_budget_struct = NULL;
	gchar **sub_tab;
	gint j=0;

	sub_tab = g_strsplit (tab[i], "/", 0);
	while (sub_tab[j])
	{
	    if (!categ_budget_struct)
	    {
		/* no categ_budget_struct created, so we are on the category */
		categ_budget_struct = g_malloc0 (sizeof (struct_categ_budget_sel));
		categ_budget_struct -> div_number = utils_str_atoi(sub_tab[j]);
	    }
	    else
	    {
		/* categ_budget_struct is created, so we are on sub-category */
		categ_budget_struct -> sub_div_numbers = g_slist_append (categ_budget_struct -> sub_div_numbers,
									 GINT_TO_POINTER (utils_str_atoi (sub_tab[j])));
	    }
	    j++;
	}
	g_strfreev (sub_tab);
	list_tmp = g_slist_append (list_tmp, categ_budget_struct);
	i++;
    }
    g_strfreev ( tab );

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
gchar * gsb_string_truncate ( gchar * string )
{
    return gsb_string_truncate_n ( string, 20, FALSE );
}


/**
 * Return a newly created strings, truncating original.  It should be
 * truncated at the end of the word containing the nth letter.
 *
 * \param string	String to truncate
 * \param n		Max lenght to truncate at.
 * \param hard_trunc	Cut in the middle of a word if needed.
 *
 * \return A newly-created string.
 */
gchar * gsb_string_truncate_n ( gchar * string, int n, gboolean hard_trunc )
{
	gchar* result;
    gchar * tmp = string, * trunc;

    if ( ! string )
	return NULL;

    if ( strlen(string) < n )
	return my_strdup ( string );
    
    tmp = string + n;
    if ( ! hard_trunc && ! ( tmp = strchr ( tmp, ' ' ) ) )
    {
	/* We do not risk splitting the string in the middle of a
	   UTF-8 accent ... the end is probably near btw. */
	return my_strdup ( string );
    }
    else 
    {
	while ( ! isascii(*tmp) && *tmp )
	    tmp++;

	trunc = g_strndup ( string, ( tmp - string ) );
	result = g_strconcat ( trunc, "...", NULL );
	g_free(trunc);
	return result;
    }
}


/**
 * remplace la chaine old_str par new_str dans str
 *
 */
gchar * gsb_string_remplace_string ( gchar *str, gchar *old_str, gchar *new_str )
{
    gchar *ptr_debut;
    gint long_old;
    gchar *chaine;

    ptr_debut = g_strstr_len ( str, -1, old_str);
    if ( ptr_debut == NULL )
        return g_strdup ( str );

    long_old = g_utf8_strlen ( old_str, -1 );
    chaine = g_strndup ( str, (ptr_debut - str) );
    if ( ptr_debut + long_old + 1 )
        chaine = g_strconcat ( chaine, new_str, ( ptr_debut + long_old + 1 ), NULL );
    else
        chaine = g_strconcat ( chaine, new_str, NULL );

    return g_strdup ( chaine );
}


/**
 * recherche des mots séparés par des jokers "%*" dans une chaine 
 *
 * \param haystack
 * \param needle
 *
 * \return TRUE si trouvé FALSE autrement
 */
gboolean gsb_string_is_trouve ( const gchar *payee_name, const gchar *needle )
{
    gchar **tab_str;
    gchar *tmpstr;
    gint i;
    gboolean is_prefix = FALSE, is_suffix = FALSE;

    if ( g_strstr_len ( needle, -1, "%" ) == NULL && 
                        g_strstr_len ( needle, -1, "*" ) == NULL )
    {
        if ( my_strcasecmp ( payee_name, needle ) == 0 )
            return TRUE;
        else
            return FALSE;
    }
    if ( g_str_has_prefix ( needle, "%" ) == FALSE &&
                        g_str_has_prefix ( needle, "*" ) == FALSE )
        is_prefix = TRUE;

    if ( g_str_has_suffix ( needle, "%" ) == FALSE &&
                        g_str_has_suffix ( needle, "*" ) == FALSE )
        is_suffix = TRUE;

    if ( is_prefix && is_suffix )
    {
        tab_str = g_strsplit_set ( needle, "%*", 0 );
        is_prefix = g_str_has_prefix ( payee_name, tab_str[0] );
        is_suffix = g_str_has_suffix ( payee_name, tab_str[1] );
        if ( is_prefix && is_suffix )
            return TRUE;
        else
            return FALSE;
    }
    else if ( is_prefix && ! is_suffix )
    {
        tmpstr = gsb_string_supprime_joker ( needle );
        is_prefix = g_str_has_prefix (payee_name, tmpstr);
        g_free (tmpstr);
        return is_prefix;
    }
    else if ( is_suffix && ! is_prefix )
    {
        tmpstr = gsb_string_supprime_joker ( needle );
        is_suffix = g_str_has_suffix (payee_name, tmpstr);
        g_free (tmpstr);
        return is_suffix;
    }

    tab_str = g_strsplit_set ( needle, "%*", 0 );

    for (i = 0; tab_str[i] != NULL; i++)
	{
        if ( tab_str[i] && strlen (tab_str[i]) > 0)
        {
            if ( g_strstr_len (payee_name, -1, tab_str[i]))
                return TRUE;
        }
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
 */
gchar * gsb_string_remplace_joker ( const gchar *chaine, gchar *new_str )
{
    gchar **tab_str;

    tab_str = g_strsplit_set ( chaine, "%*", 0 );
    return g_strjoinv ( new_str, tab_str );
}


/**
 *supprime les jokers "%*" dans une chaine
 *
 * \param chaine
 *
 * \return chaine sans joker
 */
gchar *gsb_string_supprime_joker ( const gchar *chaine )
{
    gchar **tab_str;

    tab_str = g_strsplit_set ( chaine, "%*", 0 );
    return g_strjoinv ( "", tab_str );
}


/*
 * extrait un nombre d'une chaine
 * 
 * \param chaine
 *
 * \return guint
 */
gchar *gsb_string_extract_int ( const gchar *chaine )
{
    gchar *ptr;
    gchar *tmpstr;
    gunichar ch;
    gint i = 0;
    gint long_nbre = 64;

    tmpstr = g_malloc0 ( long_nbre * sizeof (gchar) );
    ptr = g_strdup ( chaine );
    while ( g_utf8_strlen (ptr, -1) > 0 )
    {
        ch = g_utf8_get_char_validated (ptr, -1);
        if ( g_unichar_isdefined ( ch ) && g_ascii_isdigit ( ch ) )
        {
            if ( i == long_nbre )
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
 */
gchar *gsb_string_uniform_new_line ( const gchar *chaine, gint nbre_char )
{
    gchar **tab_str;

    if ( g_strstr_len ( chaine, nbre_char, "\r\n" ) )
    {
        tab_str = g_strsplit_set ( chaine, "\r", 0 );
        return g_strjoinv ( "", tab_str );
    }
    else if ( g_strstr_len ( chaine, nbre_char, "\r" ) 
     && 
     !g_strstr_len ( chaine, nbre_char, "\n" ) )
    {
        tab_str = g_strsplit_set ( chaine, "\r", 0 );
        return g_strjoinv ( "\n", tab_str );
    }
    else if ( g_strstr_len ( chaine, nbre_char, "\n" ) )
        return g_strdup ( chaine );
    else
        return NULL;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
