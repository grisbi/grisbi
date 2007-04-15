/* ************************************************************************** */
/*                                  utils_files.c			      */
/*                                                                            */
/*     Copyright (C)	2000-2003 CÃ©dric Auger (cedric@grisbi.org)      */
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


/*START_INCLUDE*/
#include "utils_files.h"
#include "./dialog.h"
#include "./utils_file_selection.h"
#include "./utils_str.h"
#include "./utils_file_selection.h"
#include "./include.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void browse_file ( GtkButton *button, gpointer data );
static gint utf8_open(gchar* utf8filename,gint mode);
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

    file_selector = file_selection_new (_("Print to file"),FILE_SELECTION_IS_SAVE_DIALOG);
    gtk_window_set_transient_for ( GTK_WINDOW ( file_selector ),
				   GTK_WINDOW ( window ));
    gtk_window_set_modal ( GTK_WINDOW ( file_selector ), TRUE );

    switch ( gtk_dialog_run ( GTK_DIALOG (file_selector)))
    {
	case GTK_RESPONSE_OK:
	    gtk_entry_set_text ( GTK_ENTRY(data),
				 file_selection_get_filename (GTK_FILE_CHOOSER (file_selector)));

	default:
	    gtk_widget_destroy ( file_selector );
	    break;
    }
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

/**
 * \brief utf8 version of fopen (see fopen for more detail about mode)
 * 
 * convert utf8 file path into the locale OS charset before calling fopen
 *
 * \param utf8filename file to open path coded using utf8 charset
 * \param mode fopen mode argument
 *
 * \return file descriptor returned by fopen
 */
FILE* utf8_fopen(const gchar* utf8filename,gchar* mode)
{
    return fopen(g_filename_from_utf8(utf8filename,-1,NULL,NULL,NULL),mode);
}

/**
 * \brief utf8 version of open (see open for more detail about mode)
 * 
 * convert utf8 file path into the locale OS charset before calling open
 *
 * \param utf8filename file to open path coded using utf8 charset
 * \param mode open mode argument
 *
 * \return file descriptor returned by open
 */
gint utf8_open(gchar* utf8filename,gint mode)
{
    return open(g_filename_from_utf8(utf8filename,-1,NULL,NULL,NULL),mode);
}

/**
 * \brief utf8 version of stat (see stat for more detail about mode)
 * 
 * convert utf8 file path into the locale OS charset before calling remove
 *
 * \param utf8filename file to remove path coded using utf8 charset
 *
 * \return remove returned value
 */
gint utf8_remove(const gchar* utf8filename)
{
    return remove(g_filename_from_utf8(utf8filename,-1,NULL,NULL,NULL));
}


/** 
 * Sanitize a safe filename.  All chars that are not normally allowed
 * are replaced by underscores.
 *
 * \param filename Filename to sanitize.
 */
gchar * safe_file_name ( gchar* filename )
{
    return g_strdelimit ( my_strdup(filename), G_DIR_SEPARATOR_S, '_' );
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
