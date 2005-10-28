/* ************************************************************************** */
/* \file            utils_file_selection.c                                    */
/*                                                                            */
/*                  GtkFileSelection enhancement API                          */
/*                                                                            */
/*   Manages implicit conversion between UTF8 to sysop locale charset :       */
/*      All input string are given in UTF8 to the API which convert them to   */
/*      the good charset, (same for output from locale to UTF8)               */
/*   Add some more convenient ways to access the GtkFileSelection properties  */
/*                                                                            */
/*                                                                            */
/*     Copyright (C)	2004- François Terrot (grisbi@terrot.net)	      */
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
/*
 * @todo : Add the multi selection management for FileDialog
 * @todo : Add a Windows compliant filter management.
 * @todo : Set the file independant from "include.h" added as an easy convenient
 *  way of resolving compilation warnings .
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "include.h"

/*START_INCLUDE*/
#include "utils_file_selection.h"
#include "dialog.h"
#include "utils_files.h"
#include "utils_file_selection.h"
#include "include.h"
/*END_INCLUDE*/

/*START_STATIC*/
static  gboolean _file_selection_check_filename ( GtkWidget *selection_fichier);
static  gboolean _file_selection_overwrite_file_check( GtkWidget *selection_fichier);
static void file_selection_check_filename_signal(GtkWidget *selection_fichier);
static gchar* file_selection_get_entry(GtkFileSelection* filesel);
static GSList * file_selection_get_selections(GtkFileSelection* filesel);
static void file_selection_overwrite_file_check_signal(GtkWidget *selection_fichier);
/*END_STATIC*/

/*START_EXTERN*/
extern GtkTreeSelection * selection;
/*END_EXTERN*/


#ifdef _WIN32_USE_FILE_DIALOG
#include "win32_file_selection.h"
#endif

#define FILE_SELECTION_PROPERTY(prop,value) ((prop & value) == value) 
    


/** Validate the selected filename from a GtkFileSelection
 *
 * If the user select or enter an existing filename, this function asks her to confirm she wants to
 * overwrite the file.
 * 
 * \param selection_fichier GtkFileSelection widget
 * \return
 *      - TRUE if the filename is ok (file does not exists or user confirm to overwrite it)
 *      - FALSE in all other cases (filename empty, ...)
 *      
 */      
static gboolean _file_selection_overwrite_file_check( GtkWidget *selection_fichier)
{ /* {{{ */
    gboolean result = TRUE;
    gchar* filename = g_strdup ( gtk_file_selection_get_filename ( GTK_FILE_SELECTION ( selection_fichier)) );
    struct stat test_file;

    if ((!filename)||(!strlen(filename)))
    {
        result = FALSE;
    }
    else if (utf8_stat ( filename, &test_file ) != -1)
    {
        // The file already exists and is writeable, ask the user if she wants to overwrite it 
        if ( S_ISREG ( test_file.st_mode ) )
	{
            result = question_yes_no_hint (_("File already exists"),
                                            g_strdup_printf (_("Do you want to overwrite file \"%s\"?"), filename));
        }
        else
        {
            dialogue_error ( g_strdup_printf ( _("Invalid filename: \"%s\"!"), filename ));
            result = FALSE;
        }
        
    }
    return result;

} /* }}} file_selection_check_filename */

/** Handler called by the "Save" GTKFileSelection dialog on button_ok "clicked" signal.
 * 
 * Send:
 * - GTK_RESPONSE_OK   if the selected filename does not exists or if the user confirm the overwrite
 * - GTK_RESPONSE_NONE in all others cases.
 */
void file_selection_overwrite_file_check_signal(GtkWidget *selection_fichier)
{ /* {{{ */
    gtk_dialog_response(GTK_DIALOG(GTK_FILE_SELECTION(selection_fichier)),
			(_file_selection_overwrite_file_check(selection_fichier) == TRUE) ? GTK_RESPONSE_OK : GTK_RESPONSE_NONE);
} /* }}} file_selection_overwrite_file_check_signal */

/** Validate the selected filename from a GtkFileSelection.  Check if file exists.
 *
 * \param selection_fichier GtkFileSelection widget
 * \return
 *      - TRUE if the filename is ok (file exists and is valid)
 *      - FALSE in all other cases (filename empty, ...)
 */      
static gboolean _file_selection_check_filename ( GtkWidget *selection_fichier)
{ /* {{{ */
    gboolean result = TRUE;
    gchar* filename = g_strdup ( gtk_file_selection_get_filename ( GTK_FILE_SELECTION ( selection_fichier)) );
    struct stat test_file;

    if ((!filename)||(!strlen(filename)))
    {
        result = FALSE;
    }
    else if ( utf8_stat ( filename, &test_file ) == -1 || 
	      !S_ISREG ( test_file.st_mode ) )
    {
	dialogue_error_hint ( g_strdup_printf ( _("Either file \"%s\" does not exist or it is not a regular file."),
						filename),
			      g_strdup_printf ( _("Error opening file '%s'." ), filename ) );
	result = FALSE;
    }

    return result;
} /* }}} file_selection_check_filename */

/** Handler called by the "Open" GTKFileSelection dialog on button_ok "clicked" signal.
 * 
 * Send:
 * - GTK_RESPONSE_OK   if the selected filename exists
 * - GTK_RESPONSE_NONE in all others cases.
 */
void file_selection_check_filename_signal(GtkWidget *selection_fichier)
{ /* {{{ */
    gtk_dialog_response(GTK_DIALOG(GTK_FILE_SELECTION(selection_fichier)),
			(_file_selection_check_filename(selection_fichier) == TRUE) ? GTK_RESPONSE_OK : GTK_RESPONSE_NONE);
} /* }}} file_selection_check_filename_signal */



/* file_selection_new a enhanced version of gtk_file_selection_new.
 *
 * Include the overwrite check feature in the save mode dialog
 * Include the file must exist feature in the open mode dialog
 *
 * \param title title of the dialog
 * \param properties properties of the dialog behaviour
 *
 *  Authorised to hijack the gtk file dialog under Windows to use
 *  the Windows standard one.
 * */
GtkWidget* file_selection_new ( const gchar *title, const gint properties )
{ /* {{{ */
    GtkWidget *filesel;

    filesel = gtk_file_chooser_dialog_new ( title, NULL, 
					    ( properties & FILE_SELECTION_IS_SAVE_DIALOG ?
					      GTK_FILE_CHOOSER_ACTION_SAVE :
					      GTK_FILE_CHOOSER_ACTION_OPEN ),
					    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					    GTK_STOCK_OPEN, GTK_RESPONSE_OK,
					    NULL );

    return GTK_WIDGET (filesel);

} /* }}} file_selection_new */



/** file_selection_set_entry.
 *
 *  Set the "filename" entry widget of the given GtkFileSelection dialog with an UTF-8 string
 *
 * \param filesel GtkFileSelection widget
 * \param utf8string a UTF-8 coded string to be put in the netry field
 *  
 */
void file_selection_set_entry(GtkFileSelection* filesel,const gchar* utf8string)
{ /* {{{ */
    gtk_file_chooser_set_filename ( filesel, 
				    g_filename_from_utf8(utf8string,-1,NULL,NULL,NULL) );
} /* }}} file_selection_set_entry */



/** file_selection_get_entry.
 * 
 * Get the text (converted in UTF-8 string) of the filename entry field of a GtkFileSelection dialog
 *
 * \param filesel GtkFileSelection widget
 *
 * \return the entry string converted in UFT-8 charset.
 *  The returned string has been allocated in this function are need to be release
 *  where no more used (no need to add any g_strdup string to use it)
 *  
 */
gchar* file_selection_get_entry(GtkFileSelection* filesel)
{ /* {{{ */
    return g_filename_to_utf8 ( gtk_file_chooser_get_filename ( filesel ),-1,NULL,NULL,NULL);
} /* }}} file_selection_get_entry */



/** file_selection_set_filename
 *
 * Set the filename property of the GtkFileSelection after having converted in the locale char set
 *
 * \param filesel GtkFileSelection widget handle
 * \param utf8filename the string to set coded with the UTF8 charset
 */
void file_selection_set_filename(GtkFileSelection* filesel,const gchar* utf8filename)
{ /* {{{ */
    gtk_file_chooser_set_filename ( filesel, 
				    g_filename_from_utf8(utf8filename,-1,NULL,NULL,NULL) );
} /* }}} file_selection_set_filename */

/** file_selection_get_filename
 *
 * Get the filename property of the GtkFileSelection, converted in UFT-8 charset
 *
 * \param  filesel GtkFileSelection widget handle
 * \return the filename string converted in UFT-8 charset.
 *  The returned string has been allocated in this function are need to be release
 *  where no more used (no need to add any g_strdup string to use it)
 */
gchar* file_selection_get_filename(GtkFileSelection* filesel)
{ /* {{{ */
    return gtk_file_chooser_get_filename ( filesel );
} /* }}} file_selection_get_filename */

/**
 * file_selection_get_selections.
 * 
 * @param filesel GtkFileSelection widget descriptor
 *
 * Provided the exactly the same service as gtk_file_selection_get_selections, except that the
 * results string of the list are already coded using UTF-8 charset.
 *
 * @return table of selected filenames (with path) using UTF8 charset
 *
 *   The returned table is not the one returned by the gtk function but a newly allocated
 *   It should be unallocated by the caller. Use g_strfreev() to free it.
 *   
 * */
GSList * file_selection_get_selections(GtkFileSelection* filesel)
{ /* {{{ */
    return gtk_file_chooser_get_filenames  ( filesel );
} /* }}} file_selection_get_selections */



/** 
 * file_selection_get_last_directory
 * @filesel
 * @ended    should the return string be terminated by a directory separator character
 *
 * Get the last directory from the history pulldown menu, add or remove (depending
 * of the ended argument) a last directory separator character at the end of the 
 * UTF8 returned string.
 * 
 * \return newly allocated utf-8 string which should be freed after no more needed.
 * There is no need to use g_strdup before using the returned string.
 *
 * */
gchar* file_selection_get_last_directory(GtkFileSelection* filesel,gboolean ended) 
{/* {{{ */
    gchar * dirstr = gtk_file_chooser_get_current_folder ( filesel );
    gint     dirstr_len  = strlen(dirstr);
    gchar*   sepstr      = g_strdup(G_DIR_SEPARATOR_S);
    gint     sepstr_len  = strlen(sepstr);
    gboolean is_endedstr = FALSE;

    // Chek if the sirectory string is ended by a separator
    // (if directory string  is small than the separator string
    //  it can ot be ended by the separator string)
    if ( dirstr_len >= sepstr_len) 
    {
        is_endedstr = (gboolean)(!strncmp( dirstr + dirstr_len - sepstr_len, sepstr, sepstr_len));
    }

    // We want the dirstr ended by a sepstrarator but there no
    if (ended&&!is_endedstr)
    {
        dirstr = g_strconcat(dirstr,G_DIR_SEPARATOR_S,NULL);
    }
    // We do not want to have a separator at the end, but there is one 
    else if ((!ended)&&is_endedstr)
    {
        dirstr[dirstr_len-sepstr_len-1] = 0;
    }
    
    return g_filename_to_utf8(dirstr,-1,NULL,NULL,NULL);

} /* }}} file_selection_get_last_directory */

#ifdef __cplusplus
}
#endif /* __cplusplus */


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
