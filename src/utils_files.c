/* ************************************************************************** */
/*                                  utils_files.c                    */
/*                                                                            */
/*     Copyright (C)	2000-2003 CÃ©dric Auger (cedric@grisbi.org)	      */
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
#include "utils_files.h"
#include "print_config.h"
#include "dialog.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void browse_file ( GtkButton *button, gpointer data );
/*END_STATIC*/


/*START_EXTERN*/
extern GtkWidget *window;
/*END_EXTERN*/






/**
 * Handler triggered by clicking on the button of a "print to file"
 * combo.  Pop ups a file selector.
 *
 * \param button GtkButton widget that triggered this handler.
 * \param data A pointer to a GtkEntry that will be filled with the
 *             result of the file selector.
 */
void browse_file ( GtkButton *button, gpointer data )
{
    GtkWidget * file_selector;

    file_selector = gtk_file_selection_new (_("Print to file"));
    gtk_window_set_transient_for ( GTK_WINDOW ( file_selector ),
				   GTK_WINDOW ( window ));
    gtk_window_set_modal ( GTK_WINDOW ( file_selector ), TRUE );

    /* Connect button click to entry change */
    g_object_set_data ( G_OBJECT(GTK_FILE_SELECTION (file_selector)->ok_button),
			"entry", file_selector );
    g_signal_connect ( GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
		       "clicked", G_CALLBACK (change_print_to_file), data);

    /* Ensure that the dialog box is destroyed when the user clicks a button. */
    g_signal_connect_swapped ( GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
			       "clicked", G_CALLBACK (gtk_widget_destroy), 
			       (gpointer) file_selector); 

    g_signal_connect_swapped ( GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->cancel_button),
			       "clicked", G_CALLBACK (gtk_widget_destroy),
			       (gpointer) file_selector); 

    gtk_widget_show_all ( file_selector );
}

/**
 * return the absolute path of where the configuration file should be located
 * on Un*x based system return $HOME
 * on Windows based systems return APPDATA\Grisbi
 * 
 * \return the absolute path of the configuration file directory
 */
gchar* my_get_grisbirc_dir(void)
{
#ifndef _WIN32
    return (gchar *) g_get_home_dir();
#else
    return win32_get_grisbirc_folder_path();
#endif
}


/**
 * return the absolute path of the default accounts files location
 * on Un*x based system return $HOME
 * on Windows based systems return "My Documents"
 * 
 * \return the absolute path of the configuration file directory
 */
gchar* my_get_gsb_file_default_dir(void)
{
#ifndef _WIN32
    return (gchar *) g_get_home_dir();
#else
    return win32_get_my_documents_folder_path();
#endif
}



/* ******************************************************************************* */
/* fonction qui rÃ©cupÃšre une ligne de charactÃšre dans le pointeur de fichier donnÃ© en argument */
/* elle alloue la mÃ©moire nÃ©cessaire et place le pointeur en argument sur la mÃ©moire allouÃ©e */
/* renvoie 0 en cas de pb, eof en cas de fin de fichier, 1 sinon */
/* ******************************************************************************* */
gint get_line_from_file ( FILE *fichier,
			  gchar **string )
{
    gchar c = 0;
    gint i = 0;
    gint j = 0;
    gchar *pointeur_char = NULL;

    if ( !fichier )
	return 0;
	    
    /*     on commence par allouer une taille de 30 caractÃšres, qu'on augment ensuite de 30 par 30 */

    pointeur_char = (gchar*)realloc(pointeur_char,30*sizeof(gchar));

    if ( !pointeur_char )
    {
	/* 	aie, pb de mÃ©moire, on vire */
	dialogue_error ( _("Memory allocation error" ));
	return 0;
    }

    while ( ( c != '\n' ) && (c != '\r'))
    {
	c =(gchar)fgetc(fichier);
	if (feof(fichier)) break;
	pointeur_char[j++] = c;

	if ( ++i == 29 )
	{
	    pointeur_char = (gchar*)realloc(pointeur_char, j + 1 + 30*sizeof(gchar));

	    if ( !pointeur_char )
	    {
		/* 	aie, pb de mÃ©moire, on vire */
		dialogue_error ( _("Memory allocation error" ));
		return 0;
	    }
	    i = 0;
	}
    }
    pointeur_char[j] = 0;

    *string = pointeur_char;

    if ( c == '\r' )
      {
	c =(gchar)fgetc(fichier);
	if ( c != '\n' )
	  {
	    ungetc ( c, fichier );
	  }
      }

    if ( feof(fichier))
	return EOF;
    else
	return 1;
}
/* ******************************************************************************* */




/**
 * Make a GtkEntry that will contain a file name, a GtkButton that
 * will pop up a file selector, pack them in a GtkHbox and return it.
 *
 * \return A newly created GtkHbox.
 */
GtkWidget * my_file_chooser ()
{
    GtkWidget * hbox, *entry, *button;

    hbox = gtk_hbox_new ( FALSE, 6 );

    entry = gtk_entry_new ( );
    gtk_box_pack_start ( GTK_BOX(hbox), entry, TRUE, TRUE, 0 );
    g_object_set_data ( G_OBJECT(hbox), "entry", entry );

    button = gtk_button_new_with_label ( _("Browse") );
    gtk_box_pack_start ( GTK_BOX(hbox), button, FALSE, FALSE, 0 );

    g_signal_connect ( G_OBJECT(button), "clicked",
		       (GCallback) browse_file, entry );

    return hbox;
}


