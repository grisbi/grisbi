/* ************************************************************************** */
/* \file            utils_file_selection.c                                    */
/*                                                                            */
/*                  GtkFileChooser enhancement API                          */
/*                                                                            */
/*   Manages implicit conversion between UTF8 to sysop locale charset :       */
/*      All input string are given in UTF8 to the API which convert them to   */
/*      the good charset, (same for output from locale to UTF8)               */
/*   Add some more convenient ways to access the GtkFileChooser properties  */
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "utils_file_selection.h"
#include "dialog.h"
#include "gsb_file_util.h"
#include "utils_str.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/


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
					    ( properties & FILE_SELECTION_IS_SAVE_DIALOG ?
					      GTK_STOCK_SAVE : GTK_STOCK_OPEN ),
					    GTK_RESPONSE_OK,
					    NULL );

    return GTK_WIDGET (filesel);

} /* }}} file_selection_new */




/** file_selection_get_filename
 *
 * Get the filename property of the GtkFileChooser, converted in UFT-8 charset
 *
 * \param  filesel GtkFileChooser widget handle
 * \return the filename string converted in UFT-8 charset.
 *  The returned string has been allocated in this function are need to be release
 *  where no more used (no need to add any my_strdup string to use it)
 */
gchar* file_selection_get_filename(GtkFileChooser* filesel)
{ /* {{{ */
    return gtk_file_chooser_get_filename ( filesel );
} /* }}} file_selection_get_filename */




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
 * There is no need to use my_strdup before using the returned string.
 *
 * */
gchar* file_selection_get_last_directory(GtkFileChooser* filesel,gboolean ended) 
{/* {{{ */
    gchar * dirstr = gtk_file_chooser_get_current_folder ( filesel );
    gint     dirstr_len  = strlen(dirstr);
    gchar*   sepstr      = my_strdup(G_DIR_SEPARATOR_S);
    gint     sepstr_len  = strlen(sepstr);
    gboolean is_endedstr = FALSE;
    gchar* tmpstr;

    /* Chek if the sirectory string is ended by a separator
     (if directory string  is small than the separator string
     it can ot be ended by the separator string) */
    if ( dirstr_len >= sepstr_len) 
    {
        is_endedstr = (gboolean)(!strncmp( dirstr + dirstr_len - sepstr_len, sepstr, sepstr_len));
    }

    /* We want the dirstr ended by a sepstrarator but there no */
    if (ended&&!is_endedstr)
    {
        tmpstr = g_strconcat(dirstr,G_DIR_SEPARATOR_S,NULL);
	g_free(dirstr);
	dirstr = tmpstr;
    }
    /* We do not want to have a separator at the end, but there is one */
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
