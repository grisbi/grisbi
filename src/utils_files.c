/* ************************************************************************** */
/*                                  utils_files.c			      */
/*                                                                            */
/*     Copyright (C)	2000-2003 CÃƒÂ©dric Auger (cedric@grisbi.org)      */
/*			2003-2004 Benjamin Drieu (bdrieu@april.org)	      */
/*			2003-2004 Alain Portal (aportal@univ-montp2.fr)	      */
/*			2004-     Francois Terrot (francois.terrot@grisbi.org)*/
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
#include "constants.h"

/*START_INCLUDE*/
#include "utils_files.h"
#include "print_config.h"
#include "dialog.h"
#include "utils.h"
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
 * \param button GtkButton widget that triggered this handler.:625
 *
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
/* fonction qui récupère une ligne de charactère dans le pointeur de fichier donné en argument */
/* elle alloue la mémoire nécessaire et place le pointeur en argument sur la mémoire allouée */
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
	    
    /*     on commence par allouer une taille de 30 caractères, qu'on augment ensuite de 30 par 30 */

    pointeur_char = (gchar*)realloc(pointeur_char,30*sizeof(gchar));

    if ( !pointeur_char )
    {
	/* 	aie, pb de mémoire, on vire */
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
		/* 	aie, pb de mémoire, on vire */
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
 * \retval NULL if the utf8filename is not a valid utf8 string
 * 
 */
FILE* utf8_fopen(gchar* utf8filename,gchar* mode)
{
    FILE*  pfd               = NULL;
    
    gchar* syslocale_filename = g_locale_from_utf8(utf8filename,-1,NULL,NULL,NULL);

    if (syslocale_filename != NULL)
    {
        pfd = fopen(syslocale_filename,mode);
        g_free(syslocale_filename);
    }
    return pfd;
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
 * \retval -1 if the utf8filename is not a valid utf8 string
 */
gint utf8_open(gchar* utf8filename,gint mode)
{
    gint fd                  = -1;
    gchar* syslocale_filename = g_locale_from_utf8(utf8filename,-1,NULL,NULL,NULL);
    if (syslocale_filename != NULL)
    {
        fd = open(syslocale_filename,mode);
        g_free(syslocale_filename);
    }
    return fd;
}

/**
 * \brief utf8 version of stat (see stat for more detail about mode)
 * 
 * convert utf8 file path into the locale OS charset before calling stat
 *
 * \param utf8filename file to get information path coded using utf8 charset
 * \param stat pointer to a stat struct
 *
 * \return stat returned value
 * \retval -1 if the utf8filename is not a valid utf8 string
 */
gint utf8_stat(gchar* utf8filename,struct stat* filestat)
{
    gint status               = -1;
    gchar* syslocale_filename = g_locale_from_utf8(utf8filename,-1,NULL,NULL,NULL);
    if (syslocale_filename != NULL)
    {
        status = stat(syslocale_filename,filestat);
        g_free(syslocale_filename);
    }
    return status;
}

/**
 * \brief utf8 version of xmlParseFile 
 * 
 * convert utf8 file path into the locale OS charset before calling xmlParseFile
 *
 * \param utf8filename file to  path coded using utf8 charset
 *
 * \return xmlParseFile returned value
 * \retval NULL if the utf8filename is not a valid utf8 string
 */
xmlDocPtr utf8_xmlParseFile(const gchar *utf8filename)
{ 
    xmlDocPtr ptr = NULL;
    gchar* syslocale_filename = g_locale_from_utf8(utf8filename,-1,NULL,NULL,NULL);
    if (syslocale_filename != NULL)
    {
        ptr = xmlParseFile(syslocale_filename);
        g_free(syslocale_filename);
    }
    return ptr;
}
/**
 * \brief utf8 version of xmlSaveFormatFile 
 * 
 * convert utf8 file path into the locale OS charset before calling xmlSaveFormatFile
 *
 * \param utf8filename file to  path coded using utf8 charset
 * \param cur                   xmlDocPtr structure to save
 * \param format                file format to use
 *
 * \return xmlSaveFormatFile returned value
 * \retval -1 if the utf8filename is not a valid utf8 string
 */
gint utf8_xmlSaveFormatFile(const gchar *utf8filename, xmlDocPtr cur, gint format)
{ 
    gint   status             = -1;
    gchar* syslocale_filename = g_locale_from_utf8(utf8filename,-1,NULL,NULL,NULL);
    if (syslocale_filename != NULL)
    {
        status = xmlSaveFormatFile(syslocale_filename,cur,format);
        g_free(syslocale_filename);
    }
    return status;
}
/**
 * \brief utf8 version of remove (see remove for more detail about mode)
 * 
 * convert utf8 file path into the locale OS charset before calling remove
 *
 * \param utf8filename file to remove path coded using utf8 charset
 *
 * \return remove returned value
 * \retval -1 if the utf8filename is not a valid utf8 string
 */
gint utf8_remove(const gchar* utf8filename)
{
    gint   status             = -1;
    gchar* syslocale_filename = g_locale_from_utf8(utf8filename,-1,NULL,NULL,NULL);
    if (syslocale_filename != NULL)
    {
        status = remove(syslocale_filename);
        g_free(syslocale_filename);
    }
    return status;
}



/** 
 * Sanitize a safe filename.  All chars that are not normally allowed
 * are replaced by underscores.
 *
 * \note: there is no convertion from utf8 to locale here. the result string is to
 * be used by GTK dialog. Internal filename string should stay in utf8. only the utf8_xxx have
 * to convert the filename to syslocale charset.
 * 
 * \param filename Filename to sanitize.
 */
gchar * safe_file_name ( gchar* filename )
{
    return g_strdelimit( g_strdup(filename), "/\\:*?\"'<>|", '_' );
}

/**
 * Compute the full (absolute and long version nuder Windows) of a path.
 * 
 * The function use utf8 charset for path as entry and output.
 * A path can be a directory or a file name.
 * 
 * \caveats Under linux, be aware the function is using the current directory value to estimate the absolute path.
 * If you call this function after having change the current directory, the result may be incosnsistent with the reality!
 * example : you file is /home/foo/bar 
 *  If the current directory is /home/foo  utf8_full_path(bar) will return "/home/foo/bar"
 *  If you change directory  to /tmp utf8_full_path(bar) will return "/tmp/bar"
 * 
 * \param utf8_short_path path (file or directory) to expand
 *
 * \return In any case a new allocated buffer to free using g_free after use.
 * At least the function return a copy of the path given as parameter.
 *
 * \note As I do not remember if there is any equivalent Windows _fullpath() function 
 *  I have implemented a more than very light algorithm.
 */
gboolean utf8_path_is_ended_by_dir_separator(gchar* utf8_path)
{
    return ((g_strrstr_len(utf8_path,1,"/"))||(g_strrstr_len(utf8_path,1,"\\")));
}
gchar* utf8_full_path(gchar* utf8_short_path)
{
    gchar* utf8_full_path       = NULL;
    gchar* syslocale_full_path  = NULL;
    gchar* syslocale_short_path = g_locale_from_utf8(utf8_short_path,-1,NULL,NULL,NULL);
    
    if ((syslocale_short_path)&&(*syslocale_short_path))
    {

#ifdef _WIN32 
        gchar* absolute_short_path  = NULL;

        absolute_short_path = win32_full_path(syslocale_short_path);
        syslocale_full_path = win32_long_name(absolute_short_path);

        utils_free(absolute_short_path);
#else 
        // Note:When utf8_short_path is already an absolute one , we do nothing as utf8_short_path
        // will be returned by default when syslocale_full_path is NULL
        // 
        if (!g_path_is_absolute(utf8_short_path))
        {
            gchar* current_dir = g_get_current_dir();
	    gboolean is_directory = g_file_test ( utf8_short_path, G_FILE_TEST_IS_DIR );

            syslocale_full_path = g_strconcat(current_dir,"/", utf8_short_path, (is_directory) ? "/" : NULL , NULL);
            utils_free(current_dir);
        }
#endif

        utils_free(syslocale_short_path);
    }

    if (syslocale_full_path)
    {
        utf8_full_path = g_locale_to_utf8(syslocale_full_path,-1,NULL,NULL,NULL);
        utils_free(syslocale_full_path);
    }
    else
    {
        utf8_full_path = g_strdup(utf8_short_path);
    }

    // Be sure that the 'last' dir separator is still present when needed
    if (utf8_path_is_ended_by_dir_separator(utf8_short_path)||g_file_test(utf8_short_path,G_FILE_TEST_IS_DIR))
    {
        if (!utf8_path_is_ended_by_dir_separator(utf8_full_path))
        {
            gchar* old_utf8_full_path = utf8_full_path;
            utf8_full_path = g_strconcat(old_utf8_full_path,C_DIRECTORY_SEPARATOR,NULL);
            utils_free(old_utf8_full_path);
        }
    }
    
    return utf8_full_path;
}


