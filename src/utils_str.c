/* ************************************************************************** */
/*                                  utils_str.c                               */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2004 Benjamin Drieu (bdrieu@april.org)	      */
/*			2003-2004 Alain Portal (dionysos@grisbi.org)	      */
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


/*START_INCLUDE*/
#include "utils_str.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gchar *my_itoa ( gint integer );
static int myisolat1ToUTF8(unsigned char* out, int *outlen,
		    const unsigned char* in, int *inlen);
/*END_STATIC*/


/*START_EXTERN*/
extern FILE * out;
/*END_EXTERN*/




/* ************************************************************************************************** */
/* itoa : transforme un integer en chaine ascii */
/* ************************************************************************************************** */

gchar *itoa ( gint integer )
{
    div_t result_div;
    gchar *chaine;
    gint i = 0;
    gint num;

    chaine = malloc ( 11*sizeof (char) );
    num = abs(integer);

    do
    {
	result_div = div ( num, 10 );
	chaine[i] = result_div.rem + 48;
	i++;
    }
    while ( ( num = result_div.quot ));

    chaine[i] = 0;

    g_strreverse ( chaine );

    if ( integer < 0 )
	chaine = g_strconcat ( "-",
			       chaine,
			       NULL );

    return ( chaine );
}
/***********************************************************************************************************/


/***********************************************************************************************************/
/* identique à itoa */
/***********************************************************************************************************/
gchar *my_itoa ( gint integer )
{
    return ( itoa ( integer ));
}
/***********************************************************************************************************/




/***********************************************************************************************************/
/* cette fonction protège atoi qui plante quand on lui envoie un null */
/***********************************************************************************************************/

gint my_atoi ( gchar *chaine )
{
    if ( chaine )
	return ( atoi ( chaine ));
    else
	return ( 0 );

}
/***********************************************************************************************************/



/******************************************************************************/
/* Fonction my_strtod (string to decimal)                                    */
/* Convertie une chaine de caractères en un nombre                            */
/* Paramètres d'entrée :                                                      */
/*   - nptr : pointeur sur la chaine de caractères à convertir                */
/*   - endptr : n'est pas utilisé, alors à quoi peut-il bien servir ?         */
/* Valeur de retour :                                                         */
/*   - resultat : le résultat de la conversion                                */
/* Variables locales :                                                        */
/*   - entier : la partie entière du résultat                                 */
/*   - mantisse : la partie décimale du résultat                              */
/*   - invert : le signe du résultat (0 -> positif, 1 -> négatif)             */
/*   - p, m : pointeurs locaux sur la chaine de caractères à convertir        */
/******************************************************************************/
double my_strtod ( char *nptr, char **endptr )
{
    double entier=0, mantisse=0, resultat=0;
    int invert = 0;
    char * p;

    if (!nptr)
	return 0;


    for ( p = nptr; p < nptr + strlen(nptr); p++ )
    {
	if (isspace(*p) || *p == '+' )
	    continue;

	if (*p == '-')
	{
	    invert = 1;
	    continue;
	}

	if ( *p == ',' || *p == '.' )
	{
	    char * m;
	    for ( m = p+1; m <= nptr+strlen(nptr) && 
		  (isdigit(*m) || isspace(*m)); m++)
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



gchar * latin2utf8 (char * inchar)
{
    char buffer[1024];
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

    return (g_strdup ( buffer ));
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
/* new_delemiters peut contenir plusieurs caratères */
/* **************************************************************************************************** */
gchar *my_strdelimit ( gchar *string,
		       gchar *delimiters,
		       gchar *new_delimiters )
{
    /* fonction identique à g_strdelimit, sauf que new_delimiters n'est pas limité à 1 caractère */
    /*     et la chaine renvoyée est une copie, pas l'original */

    gchar **tab_str;
    gchar *retour;

    if ( !( string
	    &&
	    delimiters
	    &&
	    new_delimiters ))
	return string;

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



/* ******************************************************************************* */
/* cette fonction s'assure que la chaine donnée en argument ne dépasse pas la taille */
/* donnée en argument. si elle dépasse, renvoie la chaine, avec la taille voulue, et */
/* ... à la fin */
/* ******************************************************************************* */
gchar *limit_string ( gchar *string,
		      gint length )
{
    gchar *string_return;

    if ( !string )
	return NULL;

    if ( strlen ( string ) <= length )
	return string;

    string_return = g_strdup ( string );
    string_return[length-3] = '.';
    string_return[length-2] = '.';
    string_return[length-1] = '.';
    string_return[length] = 0;

    return string_return;
}
/* ******************************************************************************* */



/* ******************************************************************************* */
/* fonction qui récupère une ligne de charactère dans une chaine */
/* elle alloue la mémoire nécessaire et n'incorpore pas le \n final */
/* renvoie NULL en cas de pb */
/* ******************************************************************************* */
gchar *get_line_from_string ( gchar *string )
{
    gchar *pointeur_char;

    if ( !string )
	return NULL;
	    
    pointeur_char = g_strdup ( string );

    pointeur_char = g_strdelimit ( pointeur_char,
				   "\n\r",
				   0 );
    return pointeur_char;
}
/* ******************************************************************************* */


