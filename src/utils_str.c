/* ************************************************************************** */
/*                                  utils_str.c                               */
/*                                                                            */
/*     Copyright (C)	2000-2003 CÃ©dric Auger (cedric@grisbi.org)	      */
/*			2003-2004 Benjamin Drieu (bdrieu@april.org)	      */
/*			2003-2004 Alain Portal (aportal@univ-montp2.fr)	      */
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

#include "include.h"
#include <stdlib.h>
#include <libxml/parser.h>


/*START_INCLUDE*/
#include "utils_str.h"
#include "include.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gint count_char_from_string ( gchar *search_char, gchar *string );
static gint my_strcmp ( gchar *chaine_1,
		 gchar *chaine_2 );
static int myisolat1ToUTF8(unsigned char* out, int *outlen,
		    const unsigned char* in, int *inlen);
/*END_STATIC*/


/*START_EXTERN*/
extern     gchar * buffer ;
extern gint max;
extern FILE * out;
/*END_EXTERN*/



/*!
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
 */
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
/***********************************************************************************************************/



/***********************************************************************************************************/
/* cette fonction protÃšge atoi qui plante quand on lui envoie un null */
/***********************************************************************************************************/
/*!
 * @brief Secured version of atoi
 * 
 * Encapsulated call of atoi which may crash when it is call with a NULL pointer.
 * 
 * @param chaine   pointer to the buffer containing the string to convert
 * 
 * @return  the converted string as interger
 * @retval  0 when the pointer is NULL or the string empty.
 * 
 */
gint utils_str_atoi ( const gchar *chaine )
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
/***********************************************************************************************************/



/******************************************************************************/
/* Fonction my_strtod (string to decimal)                                    */
/* Convertie une chaine de caractÃšres en un nombre                            */
/* ParamÃštres d'entrÃ©e :                                                      */
/*   - nptr : pointeur sur la chaine de caractÃšres Ã  convertir                */
/*   - endptr : n'est pas utilisÃ©, alors Ã  quoi peut-il bien servir ?         */
/* Valeur de retour :                                                         */
/*   - resultat : le rÃ©sultat de la conversion                                */
/* Variables locales :                                                        */
/*   - entier : la partie entiÃšre du rÃ©sultat                                 */
/*   - mantisse : la partie dÃ©cimale du rÃ©sultat                              */
/*   - invert : le signe du rÃ©sultat (0 -> positif, 1 -> nÃ©gatif)             */
/*   - p, m : pointeurs locaux sur la chaine de caractÃšres Ã  convertir        */
/******************************************************************************/
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



gchar * latin2utf8 ( const gchar * inchar)
{
    gchar buffer[1024];
    int outlen, inlen, res;

    if (!inchar)
	return NULL;

    if ( g_utf8_validate ( inchar,
			   -1,
			   NULL ))
	return inchar;
			
    inlen = strlen(inchar);
    outlen = 1024;

    res = myisolat1ToUTF8(buffer, &outlen, inchar, &inlen);
    buffer[outlen] = 0;

    return (my_strdup ( buffer ));
}

int myisolat1ToUTF8(unsigned char* out, int *outlen,
		    const unsigned char* in, int *inlen)
{
    unsigned char* outstart = out;
    const unsigned char* base = in;
    unsigned char* outend = out + *outlen;
    const unsigned char* inend;
    const unsigned char* instop;
    xmlChar c = *in;

    inend = in + (*inlen);
    instop = inend;

    while (in < inend && out < outend - 1) {
	if (c >= 0x80) {
	    *out++= ((c >>  6) & 0x1F) | 0xC0;
	    *out++= (c & 0x3F) | 0x80;
	    ++in;
	    c = *in;
	}
	if (instop - in > outend - out) instop = in + (outend - out); 
	while (c < 0x80 && in < instop) {
	    *out++ =  c;
	    ++in;
	    c = *in;
	}
    }	
    if (in < inend && out < outend && c < 0x80) {
	*out++ =  c;
	++in;
    }
    *outlen = out - outstart;
    *inlen = in - base;
    return(0);
}
/* **************************************************************************************************** */



/* **************************************************************************************************** */
/* remplace les parties delimiters */
/* par new_delimiters */
/* new_delemiters peut contenir plusieurs caratÃšres */
/* **************************************************************************************************** */
gchar *my_strdelimit ( const gchar *string,
		       gchar *delimiters,
		       gchar *new_delimiters )
{
    /* fonction identique Ã  g_strdelimit, sauf que new_delimiters n'est pas limitÃ© Ã  1 caractÃšre */
    /*     et la chaine renvoyÃ©e est une copie, pas l'original */

    gchar **tab_str;
    gchar *retour;

    if ( !( string
	    &&
	    delimiters
	    &&
	    new_delimiters ))
	return (gchar *) string;

    tab_str = g_strsplit ( string,
			   delimiters,
			   0 );
    retour = g_strjoinv ( new_delimiters,
			  tab_str );
    g_strfreev ( tab_str );

    return ( retour );
}
/* **************************************************************************************************** */


/* ******************************************************************************* */
/* my_strcasecmp : compare 2 chaines sensitive que ce soit utf8 ou ascii */
/* ******************************************************************************* */
gint my_strcmp ( gchar *chaine_1,
		 gchar *chaine_2 )
{
    if ( chaine_1
	 &&
	 chaine_2 )
    {
	if ( g_utf8_validate ( chaine_1, -1, NULL )
	     &&
	     g_utf8_validate ( chaine_2, -1, NULL ))
	{
	    gint retour;
 	    gchar *new_1, *new_2;
	    
	    new_1 = g_utf8_collate_key ( chaine_1,
					 -1 );
	    new_2 = g_utf8_collate_key ( chaine_2,
					 -1 );
	    retour = strcmp ( new_1,
			      new_2 );
	    g_free ( new_1 );
	    g_free ( new_2 );
	    return ( retour );
	}
	else
	    return ( strcmp ( chaine_1,
			      chaine_2 ));
    }

    return 0;
}
/* ******************************************************************************* */




/* ******************************************************************************* */
/* my_strcasecmp : compare 2 chaines case-insensitive que ce soit utf8 ou ascii */
/* ******************************************************************************* */
gint my_strcasecmp ( gchar *chaine_1,
		     gchar *chaine_2 )
{
    if ( chaine_1
	 &&
	 chaine_2 )
    {
	if ( g_utf8_validate ( chaine_1, -1, NULL )
	     &&
	     g_utf8_validate ( chaine_2, -1, NULL ))
	{
	    gint retour;
 	    gchar *new_1, *new_2;
	    
	    new_1 = g_utf8_collate_key ( g_utf8_casefold ( chaine_1,-1 ),
					 -1 );
	    new_2 = g_utf8_collate_key ( g_utf8_casefold (  chaine_2,-1 ),
					 -1 );
	    retour = strcmp ( new_1,
			      new_2 );
	    g_free ( new_1 );
	    g_free ( new_2 );
	    return ( retour );
	}
	else
	    return ( g_ascii_strcasecmp ( chaine_1,
					  chaine_2 ));
    }

    return 0;
}
/* ******************************************************************************* */




/* ******************************************************************************* */
/* my_strncasecmp : compare 2 chaines case-insensitive que ce soit utf8 ou ascii */
/* ******************************************************************************* */
gint my_strncasecmp ( gchar *chaine_1,
		      gchar *chaine_2,
		      gint longueur )
{
    if ( chaine_1
	 &&
	 chaine_2 )
    {
	if ( g_utf8_validate ( chaine_1, -1, NULL )
	     &&
	     g_utf8_validate ( chaine_2, -1, NULL ))
	{
	    gint retour;
 	    gchar *new_1, *new_2;
	    
	    new_1 = g_utf8_collate_key ( g_utf8_casefold ( chaine_1,longueur ),
					 longueur );
	    new_2 = g_utf8_collate_key ( g_utf8_casefold (  chaine_2,longueur ),
					 longueur );
	    retour = strcasecmp ( new_1, new_2);
	    g_free ( new_1 );
	    g_free ( new_2 );
	    return ( retour );
	}
	else
	    return ( g_ascii_strncasecmp ( chaine_1,
					   chaine_2,
					   longueur ));
    }

    return 0;
}
/* ******************************************************************************* */

/**
 * protect the my_strdup function if the string is NULL
 * don't use my_strdup directly !!
 *
 * \param string the string to be dupped
 *
 * \return a copy of that string
 * */
gchar *my_strdup ( const gchar *string )
{
    if ( string )
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

    if ( !string )
	return NULL;

    if ( strlen ( string ) <= length )
	return g_strdup (string);

    string_return = my_strdup ( string );
    string_return[length-3] = '.';
    string_return[length-2] = '.';
    string_return[length-1] = '.';
    string_return[length] = 0;

    return string_return;
}
/* ******************************************************************************* */



/* ******************************************************************************* */
/* fonction qui rÃ©cupÃšre une ligne de charactÃšre dans une chaine */
/* elle alloue la mÃ©moire nÃ©cessaire et n'incorpore pas le \n final */
/* renvoie NULL en cas de pb */
/* ******************************************************************************* */
gchar *get_line_from_string ( gchar *string )
{
    gchar *pointeur_char;

    if ( !string )
	return NULL;
	    
    pointeur_char = my_strdup ( string );

    pointeur_char = g_strdelimit ( pointeur_char,
				   "\n\r",
				   0 );
    return pointeur_char;
}
/* ******************************************************************************* */




/* ******************************************************************************* */
/* fonction qui compte le nombre de caracteres dans une chaine                     */
/* ******************************************************************************* */
gint count_char_from_string ( gchar *search_char, gchar *string )
{
		gint compteur = 0;
		gint i = 0;
		gchar* finchaine = "\0";
	
    if ( !string || !search_char || strlen(search_char)!=1) return 0;

    while (string[i]!=finchaine[0])
		{
			if (string[i]==search_char[0]) compteur++;
			i++;
		}		
    return compteur;
}
/* ******************************************************************************* */

/**
 * give back a gslist of integer from a string which the elements are separated
 * by the separator
 *
 * \param string the string we want to change to a list
 * \param delimiter the string which is the separator in the list
 *
 * \return a g_slist or NULL
 * */
GSList *gsb_string_get_list_from_string ( const gchar *string,
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
 * Create and return a string where underscores are escaped so that
 * gtk_item_factory_get_item won't be confused.
 *
 * \param orig		Original string.
 *
 * \return		A newly-allocated and escaped string.
 */
gchar * gsb_string_escape_underscores ( gchar * orig )
{
    gchar new[1024], *iter;

    for ( iter = new; *orig; orig++ )
    {
	if ( *orig == '_' )
	{
	    *iter++ = '_';
	}
	*iter++ = *orig;
    }
    *iter = 0;

    return my_strdup (new);
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
    gchar * tmp = string;

    if ( strlen(string) < 20 )
    {
	return my_strdup ( string );
    }
    
    tmp = string + 20;
    if ( ! ( tmp = strchr ( tmp, ' ' ) ) )
    {
	/* We do not risk splitting the string in the middle of a
	   UTF-8 accent ... the end is probably near btw. */
	return my_strdup ( string );
    }
    else 
    {
	gchar * trunc = g_strndup ( string, ( tmp - string ) );
	trunc = g_strconcat ( trunc, "...", NULL );
	return trunc;
    }
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
