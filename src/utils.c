/* ************************************************************************** */
/*   									      */
/*  					                                      */
/*                                                                            */
/*                                  fonctions_utiles.c                        */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2004 Benjamin Drieu (bdrieu@april.org)	      */
/*			2003-2004 Alain Portal (dionysos@grisbi.org)	      */
/*			2003-2004 Francois Terrot (francois.terrot@grisbi.org)*/
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
#include "structures.h"
#include "variables-extern.c"
#include "utils.h"


#include "calendar.h"
#include "constants.h"
#include "dialog.h"
#include "traitement_variables.h"



/* ************************************************************************* */
gboolean met_en_prelight ( GtkWidget *event_box,
			   GdkEventMotion *event,
			   gpointer pointeur )
{
    gtk_widget_set_state ( GTK_WIDGET ( GTK_BIN (event_box)->child ), GTK_STATE_PRELIGHT );
    return FALSE;
}
/* ************************************************************************* */

/* ************************************************************************* */
gboolean met_en_normal ( GtkWidget *event_box,
			 GdkEventMotion *event,
			 gpointer pointeur )
{
    gtk_widget_set_state ( GTK_WIDGET ( GTK_BIN (event_box)->child ), GTK_STATE_NORMAL );
    return FALSE;
}
/* ************************************************************************* */



/******************************************************************************/
/* fonction qui retourne la date du jour sous forme de string                 */
/******************************************************************************/
gchar *gsb_today ( void )
{
    GDate *date;
    gchar date_str[SIZEOF_FORMATTED_STRING_DATE];

    date = gdate_today();

    g_date_strftime ( date_str,
		      SIZEOF_FORMATTED_STRING_DATE,
		      "%d/%m/%Y",
		      date );

    return ( g_strdup ( date_str ) );
}
/******************************************************************************/


/******************************************************************************/
/* fonction qui retourne la date du jour au format GDate                      */
/******************************************************************************/
GDate *gdate_today ( void )
{
    GDate *date;

    date = g_date_new ();
    g_date_set_time (  date,
		       time (NULL));
    return ( date );
}
/******************************************************************************/


/******************************************************************************/
/* Fonction modifie_date                                                      */
/* prend en argument une entrée contenant une date                            */
/* vérifie la validité et la modifie si seulement une partie est donnée       */
/* met la date du jour si l'entrée est vide                                   */
/* renvoie TRUE si la date est correcte                                       */
/******************************************************************************/
gboolean modifie_date ( GtkWidget *entree )
{
    gchar *pointeur_entry;
    int jour, mois, annee;
    GDate *date;
    gchar **tab_date;

    /* si l'entrée est grise, on se barre */

    if (( gtk_widget_get_style ( entree ) == style_entree_formulaire[ENGRIS] ))
	return ( FALSE );

    pointeur_entry = g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )) );

    if ( !strlen ( pointeur_entry ))
    {
	/* si on est dans la conf des états, on ne met pas la date du jour, on */
	/* laisse vide */

	if ( entree != entree_date_init_etat &&
	     entree != entree_date_finale_etat )
	    gtk_entry_set_text ( GTK_ENTRY ( entree ),
				 gsb_today() );
    }
    else
    {
	date = g_date_new ();
	g_date_set_time ( date, time(NULL));

	tab_date = g_strsplit ( pointeur_entry, "/", 3 );

	if ( tab_date[2] && tab_date[1] )
	{
	    /*       on a rentré les 3 chiffres de la date */

	    jour = my_strtod ( tab_date[0],  NULL );
	    mois = my_strtod ( tab_date[1], NULL );
	    annee = my_strtod ( tab_date[2], NULL );

	    if ( annee < 100 )
	    {
		if ( annee < 80 ) annee = annee + 2000;
		else annee = annee + 1900;
	    }
	}
	else
	    if ( tab_date[1] )
	    {
		/* 	on a rentré la date sous la forme xx/xx , il suffit de mettre l'année courante */

		jour = my_strtod ( tab_date[0], NULL );
		mois = my_strtod ( tab_date[1], NULL );
		annee = g_date_year ( date );
		if ( g_date_month ( date ) == 1 && mois >= 10 ) annee--;
	    }
	    else
	    {
		/* 	on a rentré que le jour de la date, il faut mettre le mois et l'année courante */
		/* ou bien on a rentré la date sous forme jjmm ou jjmmaa ou jjmmaaaa */

		gchar buffer[3];

		switch ( strlen ( tab_date[0] ))
		{
		    /* 	      forme jj ou j */
		    case 1:
		    case 2:
			jour = my_strtod ( tab_date[0], 	NULL );
			mois = g_date_month ( date );
			annee = g_date_year ( date );
			break;

			/* form jjmm */

		    case 4 :
			buffer[0] = tab_date[0][0];
			buffer[1] = tab_date[0][1];
			buffer[2] = 0;

			jour = my_strtod ( buffer, NULL );
			mois = my_strtod ( tab_date[0] + 2, NULL );
			annee = g_date_year ( date );
			if ( g_date_month ( date ) == 1 && mois >= 10 ) annee--;
			break;

			/* forme jjmmaa */

		    case 6:
			buffer[0] = tab_date[0][0];
			buffer[1] = tab_date[0][1];
			buffer[2] = 0;

			jour = my_strtod ( buffer, NULL );
			buffer[0] = tab_date[0][2];
			buffer[1] = tab_date[0][3];

			mois = my_strtod ( buffer, NULL );
			annee = my_strtod ( tab_date[0] + 4, NULL ) + 2000;

			break;

			/* forme jjmmaaaa */

		    case 8:
			buffer[0] = tab_date[0][0];
			buffer[1] = tab_date[0][1];
			buffer[2] = 0;

			jour = my_strtod ( buffer, NULL );
			buffer[0] = tab_date[0][2];
			buffer[1] = tab_date[0][3];

			mois = my_strtod ( buffer, NULL );
			annee = my_strtod ( tab_date[0] + 4, NULL );
			break;

		    default :
			jour = 0;
			mois = 0;
			annee = 0;
		}
	    }
	g_strfreev ( tab_date );

	if ( g_date_valid_dmy ( jour, mois, annee) )
	    gtk_entry_set_text ( GTK_ENTRY ( entree ),
				 g_strdup_printf ( "%02d/%02d/%04d", jour, mois, annee ));
	else
	    return ( FALSE );
    }
    return ( TRUE );
}
/******************************************************************************/



/******************************************************************************/
/* Fonction format_date                                                       */
/* Prend en argument une entrée contenant une date                            */
/* Vérifie la validité et la modifie si seulement une partie est donnée       */
/* Met la date du jour si l'entrée est vide                                   */
/* Renvoie TRUE si la date est correcte                                       */
/******************************************************************************/
gboolean format_date ( GtkWidget *entree )
{
    gchar *pEntry;
    int jour, mois, annee;
    GDate *date;
    gchar **tab_date;

    pEntry = g_strstrip ( ( gchar * ) gtk_entry_get_text ( GTK_ENTRY ( entree ) ) );

    if ( !pEntry || !strlen(pEntry) )
    {
	date = gdate_today();
	jour = g_date_day (date);
	mois = g_date_month (date);
	annee = g_date_year (date);
    }
    else 
    {
	date = g_date_new();
	g_date_set_time ( date, time( NULL ) );

	tab_date = g_strsplit ( pEntry, "/", 3 );

	if ( tab_date[2] && tab_date[1] )
	{
	    /* on a rentré les 3 chiffres de la date */
	    jour = gsb_strtod ( tab_date[0],  NULL );
	    mois = gsb_strtod ( tab_date[1], NULL );
	    annee = gsb_strtod ( tab_date[2], NULL );

	    if ( annee < 100 )
	    {
		if ( annee < 80 )
		    annee = annee + 2000;
		else
		    annee = annee + 1900;
	    }
	}
	else
	{
	    if ( tab_date[1] )
	    {
		/* on a rentré la date sous la forme xx/xx,
		   il suffit de mettre l'année courante */
		jour = gsb_strtod ( tab_date[0], NULL );
		mois = gsb_strtod ( tab_date[1], NULL );
		annee = g_date_year ( date );
	    }
	    else
	    {
		/* on a rentré que le jour de la date,
		   il faut mettre le mois et l'année courante
		   ou bien on a rentré la date sous forme
		   jjmm ou jjmmaa ou jjmmaaaa */
		gchar buffer[3];

		switch ( strlen ( tab_date[0] ) )
		{
		    /* forme jj ou j */
		    case 1:
		    case 2:
			jour = gsb_strtod ( tab_date[0], NULL );
			mois = g_date_month ( date );
			annee = g_date_year ( date );
			break;

			/* forme jjmm */

		    case 4 :
			buffer[0] = tab_date[0][0];
			buffer[1] = tab_date[0][1];
			buffer[2] = 0;

			jour = gsb_strtod ( buffer, NULL );
			mois = gsb_strtod ( tab_date[0] + 2, NULL );
			annee = g_date_year ( date );
			break;

			/* forme jjmmaa */

		    case 6:
			buffer[0] = tab_date[0][0];
			buffer[1] = tab_date[0][1];
			buffer[2] = 0;

			jour = gsb_strtod ( buffer, NULL );
			buffer[0] = tab_date[0][2];
			buffer[1] = tab_date[0][3];

			mois = gsb_strtod ( buffer, NULL );
			annee = gsb_strtod ( tab_date[0] + 4, NULL ) + 2000;
			break;

			/* forme jjmmaaaa */

		    case 8:
			buffer[0] = tab_date[0][0];
			buffer[1] = tab_date[0][1];
			buffer[2] = 0;

			jour = gsb_strtod ( buffer, NULL );
			buffer[0] = tab_date[0][2];
			buffer[1] = tab_date[0][3];

			mois = gsb_strtod ( buffer, NULL );
			annee = gsb_strtod ( tab_date[0] + 4, NULL );
			break;

		    default :
			jour = 0;
			mois = 0;
			annee = 0;
			return FALSE;
		}
	    }
	}
	g_strfreev ( tab_date );
    }

    if ( g_date_valid_dmy ( jour, mois, annee) )
	gtk_entry_set_text ( GTK_ENTRY ( entree ),
			     g_strdup_printf ( "%02d/%02d/%04d", jour, mois, annee ));

    return ( TRUE );
}
/******************************************************************************/


/******************************************************************************/
/* Fonction gsb_strtod (string to decimal)                                    */
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
double gsb_strtod ( char *nptr, char **endptr )
{
    double entier=0, mantisse=0, resultat=0;
    int invert = 0;
    char *p;

    if (!nptr)
	return 0;

    /* Pour chacun des caractères de la chaine, du début à la fin de la chaine,
faire : */
    for ( p = nptr; p < nptr + strlen(nptr); p++ )
    {
	/* si c'est un espace ou le signe +, on passe au caractère suivant */
	if ( g_ascii_isspace(*p) || *p == '+' )
	    continue;

	/* si c'est le signe -, on positionne invert à 1 et on passe
	   au caractère suivant */
	if ( *p == '-' )
	{
	    invert = 1;
	    continue;
	}

	/* si c'est un point ou une virgule, alors : */
	if ( *p == ',' || *p == '.' )
	{
	    char *m;
	    /* aller à la fin de la chaine */
	    for ( m = p+1; m <= nptr+strlen(nptr) &&
		  (isdigit(*m) || isspace(*m)); m++)
		/* de la fin de la chaine au dernier caractère avant le point
		   ou la virgule, faire : */
		for ( --m; m > p; m-- )
		{
		    /* si c'est un chiffre, alors : */
		    if (isdigit(*m))
		    {
			/* "décalage" à droite de la variable mantisse */
			mantisse /= 10;
			/* ajout à la variable mantisse de la valeur décimale
			   du caractère pointé par m */
			mantisse += (*m - '0');
		    }
		}
	    /* "décalage" à droite de la variable mantisse */
	    mantisse /= 10;
	}

	/* si c'est un chiffre, alors : */
	if ( isdigit(*p) )
	{
	    /* "décalage" à gauche de la variable entier */
	    entier = entier * 10;
	    /* ajout à la variable entier de la valeur décimale
	       du caractère pointé par p */
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
/******************************************************************************/




/******************************************************************************/
void sens_desensitive_pointeur ( GtkWidget *bouton,
				 GtkWidget *widget )
{
    gtk_widget_set_sensitive ( widget,
			       gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton )));

}
/******************************************************************************/


/******************************************************************************/
void sensitive_widget ( GtkWidget *widget )
{
    gtk_widget_set_sensitive ( widget,
			       TRUE );
}
/******************************************************************************/

/******************************************************************************/
void desensitive_widget ( GtkWidget *widget )
{
    gtk_widget_set_sensitive ( widget,
			       FALSE );
}
/******************************************************************************/



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



/* **************************************************************************************************************************** */
GtkWidget *cree_bouton_url ( const gchar *adr,
			     const gchar *inscription )
{
    GtkWidget *bouton;

    bouton = gtk_button_new_with_label ( inscription );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    g_signal_connect_data ( G_OBJECT ( bouton ),
			    "clicked",
			    G_CALLBACK ( lance_navigateur_web ),
			    g_strdup ( adr),
			    NULL,
			    G_CONNECT_SWAPPED );
    return ( bouton );

}
/* **************************************************************************************************************************** */


/* **************************************************************************************************************************** */
gboolean lance_navigateur_web ( const gchar *url )
{
/*     si la commande du navigateur contient %s, on le remplace par url, */
/*     sinon on ajoute l'url à la fin et & */
/*     sous Windows si la commande est vide ou equale a la valeur par defaut on lance le butineur par defaut (open) */

    gchar **split;
    gchar *chaine;
#ifdef _WIN32
    gboolean use_default_browser = TRUE;

    if ( etat.browser_command && strlen ( etat.browser_command) )
    {
        use_default_browser = !strcmp(etat.browser_command,ETAT_WWW_BROWSER);
    }
    
#else // !_WIN32
    if ( !(etat.browser_command
	   &&
	   strlen ( etat.browser_command )))
    {
	dialogue_error_hint ( g_strdup_printf ( _("Grisbi was unable to execute a web browser to browse url <tt>%s</tt>.  Please adjust your settings to a valid executable."), url ),
			      _("Cannot run web browser") );
    }
#endif // _WIN32


#ifdef _WIN32
    if (!use_default_browser)
    {
#endif // _WIN32

    split = g_strsplit ( etat.browser_command,
			 "%s",
			 0 );

    if ( split[1] )
    {
	/* 	il y a bien un %s dans la commande de lancement */

	chaine = g_strjoinv ( g_strconcat ( " ",
					    url,
					    " ",
					    NULL ),
			      split );
	chaine = g_strconcat ( chaine,
			       "&",
			       NULL );
    }
    else
	chaine = g_strconcat ( etat.browser_command, " ", url, "&", NULL ); 


    if ( system ( chaine ) == -1 )
    {
	dialogue_error_hint ( g_strdup_printf ( _("Grisbi was unable to execute '%s'\nPlease adjust your settings to a valid executable."), chaine ),
			      _("Cannot run web browser") );
    }

#ifdef _WIN32
    }
    else
    {
        win32_shell_execute_open(url);
    } 
#endif // _WIN32

    return FALSE;
}
/* **************************************************************************************************************************** */





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



/**
 * Create a box with a nice bold title and content slightly indented.
 * All content is packed vertically in a GtkVBox.  The paddingbox is
 * also packed in its parent.
 *
 * \param parent Parent widget to pack paddingbox in
 * \param fill Give all available space to padding box or not
 * \param title Title to display on top of the paddingbox
 */
GtkWidget *new_paddingbox_with_title (GtkWidget * parent, gboolean fill, gchar * title)
{
    GtkWidget *vbox, *hbox, *paddingbox, *label;

    vbox = gtk_vbox_new ( FALSE, 0 );
    if ( GTK_IS_BOX(parent) )
    {
	gtk_box_pack_start ( GTK_BOX ( parent ), vbox,
			     fill, fill, 0);
    }

    /* Creating labe */
    label = gtk_label_new ( "" );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 1 );
    gtk_label_set_markup ( GTK_LABEL ( label ), 
			   g_strconcat ("<span weight=\"bold\">",
					title, "</span>", NULL ) );
    gtk_box_pack_start ( GTK_BOX ( vbox ), label,
			 FALSE, FALSE, 0);
    gtk_widget_show ( label );

    /* Creating horizontal box */
    hbox = gtk_hbox_new ( FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox,
			 fill, fill, 0);

    /* Some padding.  ugly but the HiG advises it this way ;-) */
    label = gtk_label_new ( "    " );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label,
			 FALSE, FALSE, 0 );

    /* Then make the vbox itself */
    paddingbox = gtk_vbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), paddingbox,
			 TRUE, TRUE, 0);

    /* Put a label at the end to feed a new line */
    /*   label = gtk_label_new ( "    " ); */
    /*   gtk_box_pack_end ( GTK_BOX ( paddingbox ), label, */
    /* 		     FALSE, FALSE, 0 ); */

    if ( GTK_IS_BOX(parent) )
    {
	gtk_box_set_spacing ( GTK_BOX(parent), 18 );
    }

    return paddingbox;
}


/**
 * Function that makes a nice title with an optional icon.  It is
 * mainly used to automate preference tabs with titles.
 * 
 * \param title Title that will be displayed in window
 * \param filename (relative or absolute) to an image in a file format
 * recognized by gtk_image_new_from_file().  Use NULL if you don't
 * want an image to be displayed
 * 
 * \returns A pointer to a vbox widget that will contain all created
 * widgets and user defined widgets
 */
GtkWidget *new_vbox_with_title_and_icon ( gchar * title,
					  gchar * image_filename)
{
    GtkWidget *vbox_title, *vbox_pref, *separator, *hbox, *label, *image;

    vbox_pref = gtk_vbox_new ( FALSE, 5 );
    gtk_widget_show ( vbox_pref );

    vbox_title = gtk_vbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_pref ), vbox_title, FALSE, FALSE, 0);

    /* Title hbox */
    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_title ), hbox, FALSE, FALSE, 0);
    gtk_widget_show ( hbox );

    /* Icon */
    if ( image_filename )
    {
	image = gtk_image_new_from_file (g_strconcat(PIXMAPS_DIR, C_DIRECTORY_SEPARATOR,
						     image_filename, NULL));
	gtk_box_pack_start ( GTK_BOX ( hbox ), image, FALSE, FALSE, 0);
	gtk_widget_show ( image );
    }

    /* Nice huge title */
    label = gtk_label_new ( title );
    gtk_label_set_markup ( GTK_LABEL(label), 
			   g_strconcat ("<span size=\"x-large\" weight=\"bold\">",
					g_markup_escape_text (title,
							      strlen(title)),
					"</span>",
					NULL ) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 0);
    gtk_widget_show ( label );

    /* Separator */
    separator = gtk_hseparator_new ();
    gtk_box_pack_start ( GTK_BOX ( vbox_title ), separator, FALSE, FALSE, 0);
    gtk_widget_show ( separator );

    return vbox_pref;
}



/**
 * Creates a new GtkEntry with a pointer to a string that will be
 * modified according to the entry's value.
 *
 * \param value A pointer to a string
 * \param hook An optional function to execute as a handler if the
 * entry's contents are modified.
 */
GtkWidget * new_text_entry ( gchar ** value, GCallback hook )
{
    GtkWidget * entry;

    entry = gtk_entry_new ();

    if (value && *value)
	gtk_entry_set_text ( GTK_ENTRY(entry), *value );

    g_object_set_data ( G_OBJECT ( entry ), "pointer", value);

    g_object_set_data ( G_OBJECT ( entry ), "insert-text", 
			(gpointer) g_signal_connect_after (GTK_OBJECT(entry), "insert-text",
							   ((GCallback) set_text), NULL));
    g_object_set_data ( G_OBJECT ( entry ), "delete-text", 
			(gpointer) g_signal_connect_after (GTK_OBJECT(entry), "delete-text",
							   ((GCallback) set_text), NULL));
    if ( hook )
    {
	g_object_set_data ( G_OBJECT ( entry ), "insert-hook", 
			    (gpointer) g_signal_connect_after (GTK_OBJECT(entry), 
							       "insert-text",
							       ((GCallback) hook), NULL));
	g_object_set_data ( G_OBJECT ( entry ), "delete-hook", 
			    (gpointer) g_signal_connect_after (GTK_OBJECT(entry), 
							       "delete-text",
							       ((GCallback) hook), NULL));
    }

    return entry;
}



/** 
 * TODO: document
 */
void entry_set_value ( GtkWidget * entry, gchar ** value )
{
    /* Block everything */
    if ( g_object_get_data ((GObject*) entry, "insert-hook") > 0 )
	g_signal_handler_block ( GTK_OBJECT(entry),
				 (gulong) g_object_get_data ((GObject*) entry, 
							     "insert-hook"));
    if ( g_object_get_data ((GObject*) entry, "insert-text") > 0 )
	g_signal_handler_block ( GTK_OBJECT(entry),
				 (gulong) g_object_get_data ((GObject*) entry, 
							     "insert-text"));
    if ( g_object_get_data ((GObject*) entry, "delete-hook") > 0 )
	g_signal_handler_block ( GTK_OBJECT(entry),
				 (gulong) g_object_get_data ((GObject*) entry, 
							     "delete-hook"));
    if ( g_object_get_data ((GObject*) entry, "delete-text") > 0 )
	g_signal_handler_block ( GTK_OBJECT(entry),
				 (gulong) g_object_get_data ((GObject*) entry, 
							     "delete-text"));

    /* Fill in value */
    if (value && *value)
	gtk_entry_set_text ( GTK_ENTRY ( entry ), *value );
    else
	gtk_entry_set_text ( GTK_ENTRY ( entry ), "" );

    g_object_set_data ( G_OBJECT(entry), "pointer", value );

    /* Unblock everything */
    if ( g_object_get_data ((GObject*) entry, "insert-hook") > 0 )
	g_signal_handler_unblock ( GTK_OBJECT(entry),
				   (gulong) g_object_get_data ((GObject*) entry, 
							       "insert-hook"));
    if ( g_object_get_data ((GObject*) entry, "insert-text") > 0 )
	g_signal_handler_unblock ( GTK_OBJECT(entry),
				   (gulong) g_object_get_data ((GObject*) entry, 
							       "insert-text"));
    if ( g_object_get_data ((GObject*) entry, "delete-hook") > 0 )
	g_signal_handler_unblock ( GTK_OBJECT(entry),
				   (gulong) g_object_get_data ((GObject*) entry, 
							       "delete-hook"));
    if ( g_object_get_data ((GObject*) entry, "delete-text") > 0 )
	g_signal_handler_unblock ( GTK_OBJECT(entry),
				   (gulong) g_object_get_data ((GObject*) entry, 
							       "delete-text"));
}



/**
 * Set a string to the value of an GtkEntry.
 *
 * \param entry The reference GtkEntry
 * \param value Handler parameter.  Not used.
 * \param length Handler parameter.  Not used.
 * \param position Handler parameter.  Not used.
 */
gboolean set_text (GtkEntry *entry, gchar *value, 
		   gint length, gint * position)
{
    gchar ** data;

    data = g_object_get_data ( G_OBJECT ( entry ), "pointer");
    if (data)
	*data = g_strdup ((gchar*) gtk_entry_get_text ( GTK_ENTRY (entry) ));

    /* Mark file as modified */
    modification_fichier ( TRUE );

    return FALSE;
}




/**
 * Creates a new GtkTextView with a pointer to a string that will be
 * modified according to the text view's value.
 *
 * \param value A pointer to a string
 * \param hook An optional function to execute as a handler if the
 * textview's contents are modified.
 */
GtkWidget * new_text_area ( gchar ** value, GCallback hook )
{
    GtkWidget * text_view;
    GtkTextBuffer *buffer;

    text_view = gtk_text_view_new ();
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
    gtk_text_view_set_pixels_above_lines (GTK_TEXT_VIEW (text_view), 3);
    gtk_text_view_set_pixels_below_lines (GTK_TEXT_VIEW (text_view), 3);
    gtk_text_view_set_left_margin (GTK_TEXT_VIEW (text_view), 3);
    gtk_text_view_set_right_margin (GTK_TEXT_VIEW (text_view), 3);
    gtk_text_view_set_wrap_mode ( GTK_TEXT_VIEW (text_view), GTK_WRAP_WORD );

    if (value && *value)
	gtk_text_buffer_set_text (buffer, *value, -1);

    g_object_set_data ( G_OBJECT ( buffer ), "pointer", value);

    g_object_set_data ( G_OBJECT ( buffer ), "change-text",
			(gpointer) g_signal_connect (G_OBJECT(buffer),
						     "changed",
						     ((GCallback) set_text_from_area),
						     NULL));
    if ( hook )
	g_object_set_data ( G_OBJECT ( buffer ), "change-hook",
			    (gpointer) g_signal_connect (G_OBJECT(buffer),
							 "changed",
							 ((GCallback) hook),
							 NULL));
    return text_view;
}



/** 
 * TODO: document
 */
void text_area_set_value ( GtkWidget * text_view, gchar ** value )
{
    GtkTextBuffer * buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));

    /* Block everything */
    if ( g_object_get_data (G_OBJECT(buffer), "change-hook") > 0 )
	g_signal_handler_block ( G_OBJECT(buffer),
				 (gulong) g_object_get_data ( G_OBJECT(buffer), "change-hook" ));
    if ( g_object_get_data (G_OBJECT(buffer), "change-text") > 0 )
	g_signal_handler_block ( G_OBJECT(buffer),
				 (gulong) g_object_get_data ( G_OBJECT(buffer), 
							      "change-text" ));

    /* Fill in value */
    if (value && *value)
	gtk_text_buffer_set_text (buffer, *value, -1 );
    else
	gtk_text_buffer_set_text (buffer, "", -1 );

    if ( value )
	g_object_set_data ( G_OBJECT(buffer), "pointer", value );

    /* Unblock everything */
    if ( g_object_get_data ((GObject*) buffer, "change-hook") > 0 )
	g_signal_handler_unblock ( G_OBJECT(buffer),
				   (gulong) g_object_get_data ((GObject*) buffer, 
							       "change-hook" ));
    if ( g_object_get_data ((GObject*) buffer, "change-text") > 0 )
	g_signal_handler_unblock ( G_OBJECT(buffer),
				   (gulong) g_object_get_data ((GObject*) buffer, 
							       "change-text" ));
}




/**
 * Set a string to the value of an GtkTextView
 *
 * \param buffer The reference GtkTextBuffer
 * \param dummy Handler parameter.  Not used.
 */
gboolean set_text_from_area ( GtkTextBuffer *buffer, gpointer dummy )
{
    GtkTextIter start, end;
    gchar ** data;

    data = g_object_get_data ( G_OBJECT ( buffer ), "pointer");

    gtk_text_buffer_get_iter_at_offset ( buffer, &start, 0 );
    gtk_text_buffer_get_iter_at_offset ( buffer, &end, -1 );

    if (data)
	*data = g_strdup ( gtk_text_buffer_get_text (buffer, &start, &end, 0) );

    /* Mark file as modified */
    modification_fichier ( TRUE );

    return FALSE;
}





/**
 * Create a GtkCheckButton with a callback associated.  Initial value
 * of this checkbox is set to the value of *data.  This checkbox calls
 * set_boolean upon toggle, which in turn modifies *data.  If a hook
 * is possibly executed as well.
 *
 * \param label The label for this checkbutton
 * \param data A pointer to a boolean integer
 * \param hook A GCallBack to execute if not null
 * \return A newly allocated GtkVBox
 */
GtkWidget *new_checkbox_with_title ( gchar * label, guint * data, GCallback hook)
{
    GtkWidget * checkbox;

    checkbox = gtk_check_button_new_with_label ( label );
    checkbox_set_value ( checkbox, data, TRUE );

    g_object_set_data ( G_OBJECT (checkbox), "set-boolean", 
			(gpointer) g_signal_connect (checkbox, "toggled",
						     ((GCallback) set_boolean), data));

    if ( hook )
    {
	g_object_set_data ( G_OBJECT ( checkbox ), "hook", 
			    (gpointer) g_signal_connect (checkbox, "toggled",
							 ((GCallback) hook), data ));
    }

    return checkbox;
}



/**
 * Creates a new radio buttons group with two choices.  Toggling will
 * change the content of an integer passed as an argument.
 *
 * \param parent A widget to pack all created radio buttons in
 * \param title The title for this group
 * \param choice1 First choice label
 * \param choice2 Second choice label
 * \param data A pointer to an integer that will be set to 0 or 1
 *        according to buttons toggles.
 * \param hook An optional hook to run at each toggle
 *
 * \return A newly created paddingbox
 */
GtkWidget *new_radiogroup_with_title (GtkWidget * parent,
				      gchar * title, gchar * choice1, gchar * choice2,
				      guint * data, GCallback hook)
{
    GtkWidget * button1, *button2, *paddingbox;

    paddingbox = new_paddingbox_with_title (parent, FALSE, COLON(title));

    button1 = gtk_radio_button_new_with_label ( NULL, choice1 );
    gtk_box_pack_start (GTK_BOX(paddingbox), button1, FALSE, FALSE, 0 );
    button2 = gtk_radio_button_new_with_label ( gtk_radio_button_group (GTK_RADIO_BUTTON(button1)), 
						choice2 );
    gtk_box_pack_start (GTK_BOX(paddingbox), button2, FALSE, FALSE, 0 );

    if (data)
    {
	if (*data)
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button2 ), TRUE );
	else
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button1 ), TRUE );
    }

    g_object_set_data ( G_OBJECT ( button2 ), "pointer", data);
    g_signal_connect ( GTK_OBJECT ( button2 ), "toggled",
		       (GCallback) set_boolean, NULL );

    if (hook)
    {
	g_signal_connect ( GTK_OBJECT ( button2 ), "toggled",
			   (GCallback) hook, data );
    }

    return paddingbox;
}


/**
 * Update the widget's appearance accordingly.  If update is set, update
 * property as well.
 * 
 * \param checkbox The checkbox to update
 * \param data A pointer to a boolean which contains the new value to
 * fill in checkbox's properties.  This boolean will be modified by
 * checkbox's handlers as well.
 * \param update Whether to update checkbox's data as well.
 */
void checkbox_set_value ( GtkWidget * checkbox, guint * data, gboolean update )
{
    if (data)
    {
	if (g_object_get_data (G_OBJECT(checkbox), "hook") > 0)
	    g_signal_handler_block ( checkbox, 
				     (gulong) g_object_get_data (G_OBJECT(checkbox), 
								 "hook" ));
	if (g_object_get_data (G_OBJECT(checkbox), "set-boolean") > 0)
	    g_signal_handler_block ( checkbox,
				     (gulong) g_object_get_data (G_OBJECT(checkbox),
								 "set-boolean" ));
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( checkbox ), *data );
	if (g_object_get_data (G_OBJECT(checkbox), "hook") > 0)
	    g_signal_handler_unblock ( checkbox, 
				       (gulong) g_object_get_data (G_OBJECT(checkbox),
								   "hook" ));
	if (g_object_get_data (G_OBJECT(checkbox), "set-boolean") > 0)
	    g_signal_handler_unblock ( checkbox,
				       (gulong) g_object_get_data ( G_OBJECT(checkbox),
								    "set-boolean" ));
    }

    if (update)
	g_object_set_data ( G_OBJECT ( checkbox ), "pointer", data);

}




/**
 * Set a boolean integer to the value of a checkbox.  Normally called
 * via a GTK "toggled" signal handler.
 * 
 * \param checkbox a pointer to a checkbox widget.
 * \param data a pointer to an integer that is to be modified.
 */
gboolean set_boolean ( GtkWidget * checkbox, guint * dummy)
{
    gboolean *data;

    data = g_object_get_data ( G_OBJECT ( checkbox ), "pointer");
    if (data)
	*data = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(checkbox));

    /* Mark file as modified */
    modification_fichier ( TRUE );

    return FALSE;
}



/**
 * Sets a GDate according to a date widget
 *
 * \param entry A GtkEntry that triggered this handler
 * \param value Handler parameter.  Not used.
 * \param length Handler parameter.  Not used.
 * \param position Handler parameter.  Not used.
 */
gboolean set_date (GtkEntry *entry, gchar *value, gint length, gint * position)
{
    GDate ** data, temp_date;

    data = g_object_get_data ( G_OBJECT ( entry ), "pointer");

    g_date_set_parse ( &temp_date, gtk_entry_get_text (GTK_ENTRY(entry)) );
    if ( g_date_valid (&temp_date) && data)
    {
	if (!*data)
	    *data = g_date_new ();
	g_date_set_parse ( *data, gtk_entry_get_text (GTK_ENTRY(entry)) );
    }

    /* Mark file as modified */
    modification_fichier ( TRUE );

    return FALSE;
}



/**
 * Creates a new GtkHBox with an entry to type in a date.
 *
 * \param value A pointer to a GDate that will be modified at every
 * change.
 * \param An optional hook to run.
 */
GtkWidget * new_date_entry ( gchar ** value, GCallback hook )
{
    GtkWidget *hbox, *entry, *date_entry;

    hbox = gtk_hbox_new ( FALSE, 6 );

    entry = gtk_entry_new ();
    gtk_box_pack_start ( GTK_BOX(hbox), entry,
			 TRUE, TRUE, 0 );

    g_object_set_data ( G_OBJECT (entry), "pointer", value);

    if ( hook )
    {
	g_object_set_data ( G_OBJECT (entry), "insert-hook", 
			    (gpointer) g_signal_connect_after (GTK_OBJECT(entry), 
							       "insert-text",
							       ((GCallback) hook), 
							       NULL));
	g_object_set_data ( G_OBJECT (entry), "delete-hook", 
			    (gpointer) g_signal_connect_after (GTK_OBJECT(entry), 
							       "delete-text",
							       ((GCallback) hook), 
							       NULL));
    }
    g_object_set_data ( G_OBJECT (entry), "insert-text", 
			(gpointer) g_signal_connect_after (GTK_OBJECT(entry), 
							   "insert-text",
							   ((GCallback) set_date), 
							   NULL));
    g_object_set_data ( G_OBJECT (entry), "delete-text", 
			(gpointer) g_signal_connect_after (GTK_OBJECT(entry), 
							   "delete-text",
							   ((GCallback) set_date), 
							   NULL));

    date_entry = gtk_button_new_with_label ("...");
    gtk_box_pack_start ( GTK_BOX(hbox), date_entry,
			 FALSE, FALSE, 0 );
    g_object_set_data ( G_OBJECT ( date_entry ),
			"entry", entry);

    g_signal_connect ( GTK_OBJECT ( date_entry ), "clicked",
		       ((GCallback) popup_calendar ), NULL );

    return hbox;
}



/**
 * Change the date that is handled by a date entry.
 *
 * \param hbox The date entry widget.
 * \param value The new date to modify.
 * \param Update GtkEntry value as well.
 */
void date_set_value ( GtkWidget * hbox, GDate ** value, gboolean update )
{
    GtkWidget * entry;


    entry = get_entry_from_date_entry (hbox);
    g_object_set_data ( G_OBJECT ( entry ),
			"pointer", value );

    if ( update )
    {
	if (g_object_get_data ((GObject*) entry, "insert-hook") > 0)
	    g_signal_handler_block ( entry,
				     (gulong) g_object_get_data ((GObject*) entry, 
								 "insert-hook"));
	if (g_object_get_data ((GObject*) entry, "insert-text") > 0)
	    g_signal_handler_block ( entry,
				     (gulong) g_object_get_data ((GObject*) entry, 
								 "insert-text"));
	if (g_object_get_data ((GObject*) entry, "delete-hook") > 0)
	    g_signal_handler_block ( entry,
				     (gulong) g_object_get_data ((GObject*) entry, 
								 "delete-hook"));
	if (g_object_get_data ((GObject*) entry, "delete-text") > 0)
	    g_signal_handler_block ( entry,
				     (gulong) g_object_get_data ((GObject*) entry, 
								 "delete-text"));

	if ( value && *value )
	{
	    gtk_entry_set_text ( GTK_ENTRY(entry),
				 g_strdup_printf ( "%02d/%02d/%04d",
						   g_date_day (*value),
						   g_date_month (*value),
						   g_date_year (*value)));
	}

	if (g_object_get_data ((GObject*) entry, "insert-hook") > 0)
	    g_signal_handler_unblock ( entry,
				       (gulong) g_object_get_data ((GObject*) entry, 
								   "insert-hook"));
	if (g_object_get_data ((GObject*) entry, "insert-text") > 0)
	    g_signal_handler_unblock ( entry,
				       (gulong) g_object_get_data ((GObject*) entry, 
								   "insert-text"));
	if (g_object_get_data ((GObject*) entry, "delete-hook") > 0)
	    g_signal_handler_unblock ( entry,
				       (gulong) g_object_get_data ((GObject*) entry, 
								   "delete-hook"));
	if (g_object_get_data ((GObject*) entry, "delete-text") > 0)
	    g_signal_handler_unblock ( entry,
				       (gulong) g_object_get_data ((GObject*) entry, 
								   "delete-text"));
    }
}



/**
 * Pop up a window with a calendar that allows a date selection.  This
 * calendar runs "date_selectionnee" as a callback if a date is
 * selected.
 *
 * \param button Normally a GtkButton that triggered the handler.
 * This parameter will be used as a base to set popup's position.
 * This widget must also have a parameter (data) of name "entry"
 * which contains a pointer to a GtkEntry used to set initial value of
 * calendar.
 * \param data Handler parameter.  Not used.
 */
gboolean popup_calendar ( GtkWidget * button, gpointer data )
{
    GtkWidget *popup, *entree, *popup_boxv, *calendrier, *bouton, *frame;
    gint x, y, cal_jour, cal_mois, cal_annee;
    GtkRequisition taille_entree, taille_popup;

    /* Find associated gtkentry */
    entree = g_object_get_data ( G_OBJECT(button), "entry" );

    /* Find popup position */
    gdk_window_get_origin ( GTK_BUTTON (button) -> event_window, &x, &y );
    gtk_widget_size_request ( GTK_WIDGET (button), &taille_entree );
    y = y + taille_entree.height;

    /* Create popup */
    popup = gtk_window_new ( GTK_WINDOW_POPUP );
    gtk_window_set_modal ( GTK_WINDOW (popup), TRUE);

    /* Create popup widgets */
    frame = gtk_frame_new ( NULL );
    gtk_container_add ( GTK_CONTAINER (popup), frame);
    popup_boxv = gtk_vbox_new ( FALSE, 5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( popup_boxv ), 5 );
    gtk_container_add ( GTK_CONTAINER ( frame ), popup_boxv);

    /* Set initial date according to entry */
    if ( !( strlen ( g_strstrip ( (gchar *) gtk_entry_get_text (GTK_ENTRY(entree))))
	    &&
	    sscanf ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree )),
		     "%02d/%02d/%04d", &cal_jour, &cal_mois, &cal_annee)))
	sscanf ( gsb_today(), "%02d/%02d/%04d", &cal_jour, &cal_mois, &cal_annee);

    /* Creates calendar */
    calendrier = gtk_calendar_new();
    gtk_calendar_select_month ( GTK_CALENDAR ( calendrier ), cal_mois-1, cal_annee);
    gtk_calendar_select_day  ( GTK_CALENDAR ( calendrier ), cal_jour);
    gtk_calendar_display_options ( GTK_CALENDAR ( calendrier ),
				   GTK_CALENDAR_SHOW_HEADING |
				   GTK_CALENDAR_SHOW_DAY_NAMES |
				   GTK_CALENDAR_WEEK_START_MONDAY );

    /* Create handlers */
    gtk_signal_connect ( GTK_OBJECT ( calendrier), "day-selected-double-click",
			 ((GCallback)  date_selection ), entree );
    gtk_signal_connect_object ( GTK_OBJECT ( calendrier), "day-selected-double-click",
				((GCallback)  close_calendar_popup ), popup );
    gtk_signal_connect ( GTK_OBJECT ( popup ), "key-press-event",
			 ((GCallback)  clavier_calendrier ), NULL );
    gtk_box_pack_start ( GTK_BOX ( popup_boxv ), calendrier,
			 TRUE, TRUE, 0 );

    /* Add the "cancel" button */
    bouton = gtk_button_new_with_label ( _("Cancel") );
    gtk_signal_connect_object ( GTK_OBJECT ( bouton ), "clicked",
				((GCallback)  close_calendar_popup ),
				GTK_WIDGET ( popup ) );
    gtk_box_pack_start ( GTK_BOX ( popup_boxv ), bouton,
			 TRUE, TRUE, 0 );


    /* Show everything */
    gtk_widget_show_all ( popup );
    gtk_widget_set_uposition ( GTK_WIDGET ( popup ), x, y );
    gtk_widget_size_request ( GTK_WIDGET ( popup ), &taille_popup );
    gtk_widget_set_uposition ( GTK_WIDGET ( popup ), 
			       x-taille_popup.width+taille_entree.width, y );

    /* Grab pointer */
    gdk_pointer_grab ( popup -> window, TRUE,
		       GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
		       GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK |
		       GDK_POINTER_MOTION_MASK,
		       NULL, NULL, GDK_CURRENT_TIME );

    return FALSE;
}



/**
 * Closes the popup specified as an argument.  As a quick but
 * disgusting hack, we also grab focus on "fenetre_preferences", the
 * dialog that contains all tabs, in order to preserve it.
 *
 * \param popup The popup to close.
 */
void close_calendar_popup ( GtkWidget *popup )
{
    gtk_widget_destroy ( popup );
    gtk_grab_remove ( fenetre_preferences );
    gtk_grab_add ( fenetre_preferences );
}



/**
 * Convenience function that returns the first widget child of a
 * GtkBox.  This is specially usefull for date "widgets" that contains
 * one GtkEntry and one GtkButton, both packed in a single GtkHBox.
 *
 * \param hbox A GtkBox that contains at least one child.
 */
GtkWidget * get_entry_from_date_entry (GtkWidget * hbox)
{
    return ((GtkBoxChild *) GTK_BOX(hbox)->children->data)->widget;
}



/**
 * Creates a new GtkSpinButton with a pointer to a string that will be
 * modified according to the spin's value.
 *
 * \param value A pointer to a string
 * \param hook An optional function to execute as a handler if the
 * textview's contents are modified.
 */
GtkWidget * new_spin_button ( gint * value, 
			      gdouble lower, gdouble upper, 
			      gdouble step_increment, gdouble page_increment, 
			      gdouble page_size, 
			      gdouble climb_rate, guint digits,
			      GCallback hook )
{
    GtkWidget * spin;
    GtkAdjustment * adjustment;
    gdouble initial = 0;

    if ( value )  /* Sanity check */
	initial = *value;

    adjustment = GTK_ADJUSTMENT( gtk_adjustment_new ( initial, lower, upper, 
						      step_increment, page_increment,
						      page_size ));
    spin = gtk_spin_button_new ( adjustment, climb_rate, digits );
    gtk_spin_button_set_numeric ( GTK_SPIN_BUTTON (spin), TRUE );
    g_object_set_data ( G_OBJECT (spin), "pointer", value);
    g_object_set_data ( G_OBJECT (spin), "adj", adjustment);

    g_object_set_data ( G_OBJECT (spin), "value-changed",
			(gpointer) g_signal_connect_swapped ( GTK_OBJECT (adjustment),
							      "value-changed", 
							      (GCallback) set_double, spin));
    if ( hook )
    {
	g_object_set_data ( G_OBJECT (spin), "hook",
			    (gpointer) g_signal_connect_swapped ( GTK_OBJECT (adjustment), 
								  "value-changed", 
								  (GCallback) hook, spin ));
    }

    return spin;
}



/**
 *  TODO: document
 *
 */
void spin_button_set_value ( GtkWidget * spin, gdouble * value )
{
    GtkAdjustment * adjustment;

    adjustment = g_object_get_data ( G_OBJECT(spin), "adj" );
    if (!adjustment)
	return;

    /* Block everything */
    if ( g_object_get_data ((GObject*) spin, "value-changed") > 0 )
	g_signal_handler_block ( GTK_OBJECT(adjustment),
				 (gulong) g_object_get_data ((GObject*) spin, 
							     "value-changed"));
    if ( g_object_get_data ((GObject*) spin, "hook") > 0 )
	g_signal_handler_block ( GTK_OBJECT(adjustment),
				 (gulong) g_object_get_data ((GObject*) spin, "hook"));

    if (value)
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(spin), *value);
    else
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(spin), 0);

    g_object_set_data ( G_OBJECT(spin), "pointer", value);

    /* Unblock everything */
    if ( g_object_get_data ((GObject*) spin, "value-changed") > 0 )
	g_signal_handler_unblock ( GTK_OBJECT(adjustment),
				   (gulong) g_object_get_data ((GObject*) spin, 
							       "value-changed"));
    if ( g_object_get_data ((GObject*) spin, "hook") > 0 )
	g_signal_handler_unblock ( GTK_OBJECT(adjustment),
				   (gulong) g_object_get_data ((GObject*) spin, "hook"));
}



/**
 * Set an integer to the value of a spin button.  Normally called via
 * a GTK "changed" signal handler.
 * 
 * \param spin a pointer to a spinbutton widget.
 * \param dummy unused
 */
gboolean set_double ( GtkWidget * spin, gdouble * dummy)
{
   gint *data;

    data = g_object_get_data ( G_OBJECT(spin), "pointer" );

    if ( data )
    {
	*data = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON(spin) );
    }

	/* Mark file as modified */
    modification_fichier ( TRUE );
    return (FALSE);
}








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


