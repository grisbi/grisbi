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
#include "fichier_configuration_constants.h"


/*START_INCLUDE*/
#include "utils.h"
#include "dialog.h"
#include "main.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/





/*START_EXTERN*/
extern gint compte_courant;
extern gint id_fonction_idle;
extern gint nb_comptes;
extern gpointer **p_tab_nom_de_compte;
extern gpointer **p_tab_nom_de_compte_variable;
extern GtkWidget *window;
/*END_EXTERN*/




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
			      _("Cannot execute web browser") );
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
	dialogue_error_hint ( g_strdup_printf ( _("Grisbi was unable to execute a web browser to browse url <tt>%s</tt>.\nThe command was: %s.\nPlease adjust your settings to a valid executable."), url, chaine ),
			      _("Cannot execute web browser") );
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



/* ******************************************************************************* */
/* fonction qui vérifie la validité de la variable p_tab_nom_de_compte_variable */
/* si elle n'est pas valide, la place sur le compte courant, et si pas encore nécessaire, */
/*     la place sur le 1er compte. dans ce cas, si nb_comptes = 0 on met un message  */
/* comme quoi ça va sûrement crasher derrière */
/*     si p_tab_nom_de_compte_variable n'est pas valide, affiche un message d'erreur */
/* return le p_tab_nom_de_compte_variable ou NULL si pas de comptes */
/* ******************************************************************************* */

gpointer **verification_p_tab ( gchar *fonction_appelante )
{
    gchar *message=NULL;

    if ( !fonction_appelante )
	fonction_appelante = _("I don't know !!!");

    if ( !assert_account_loaded() )
    {
	dialogue_error_hint ( _("There is no account in memory ! But if you see that error, it should...\nPlease send a bug report to the grisbi team, this is a very big bug !\nGrisbi will probably crash after that message..."),
			      _("No account in memory"));
	return NULL;
    }

    if ( p_tab_nom_de_compte_variable > (p_tab_nom_de_compte + nb_comptes )
	 ||
	 p_tab_nom_de_compte_variable < p_tab_nom_de_compte )
    {
	/* 	p_tab était mal placée */

	message = _( "Warning : variable p_tab_nom_de_compte_variable is abnormal.");

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;

	if ( p_tab_nom_de_compte_variable > (p_tab_nom_de_compte + nb_comptes )
	     ||
	     p_tab_nom_de_compte_variable < p_tab_nom_de_compte )
	{
	    /* 	    cette fois le compte_courant aussi c'est n'importe quoi... */

	    message = g_strconcat ( message,
				    "\nAnd so the variable compte_courant...\nThis is a bug, please contact the grisbi team to fix it.\nWhat you wanted to do will be done on the first account, so normally you can correct it.\nThe problem comes from the function :\n",
				    fonction_appelante,
				    NULL );

	    p_tab_nom_de_compte_variable = p_tab_nom_de_compte;
	}
	else
	{
	    /* 	    bon, au moins compte_courant est valide... */
	     
		    message = g_strconcat ( message,
				    "\nThis is a bug, please contact the grisbi team to fix it.\nWhat you wanted to do will be done on the current account, so normally you can correct it.\nThe problem comes from the function :\n",
				    fonction_appelante,
				    NULL );
	}
    }
    return (p_tab_nom_de_compte_variable);
}
/* ******************************************************************************* */



/**
 * Returns TRUE if an account is loaded in memory.  Usefull to be sure
 * there is data to process.
 *
 * \return TRUE if an account is loaded in memory.
 */
gboolean assert_account_loaded ()
{
  return nb_comptes != 0;
}



/******************************************************************************/
/* cette fonction démarre la vérification idle si elle ne l'est pas déjà */
/* l'idle se terminera d'elle même ou forcé */
/******************************************************************************/
void demarrage_idle ( void )
{
    if ( DEBUG )
	printf ( "demarrage_idle\n" );

    if ( !id_fonction_idle )
	id_fonction_idle = g_idle_add ( (GSourceFunc) utilisation_temps_idle,
					NULL );


}
/******************************************************************************/


/******************************************************************************/
/* cette fonction arrête la vérification idle */
/* renvoie TRUE si la fonction était en cours de fonctionnement */
/******************************************************************************/
gboolean termine_idle ( void )
{
    if ( DEBUG )
	printf ( "termine_idle\n" );

    if ( id_fonction_idle )
    {
	g_source_remove ( id_fonction_idle );
	id_fonction_idle = 0;
	return TRUE;
    }

    return FALSE;
}
/******************************************************************************/


/******************************************************************************/
/* cette fonction rafraichit l'écran pendant les traitements d'information */
/******************************************************************************/
void update_ecran ( void )
{
    gint idle_en_cours;

    if ( DEBUG )
	printf ( "update_ecran\n" );

    idle_en_cours = termine_idle ();

    while ( g_main_iteration (FALSE));

    if ( idle_en_cours )
	demarrage_idle ();
}
/******************************************************************************/




/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
