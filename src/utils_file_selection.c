/* ************************************************************************** */
/*                                                                            */
/*                  GtkFileSelection enhancement API                          */
/*                                                                            */
/*   Manages implicit conversion between UTF8 to sysop locale charset :       */
/*      All input string are given in UTF8 to the API which convert them to   */
/*      the good charset, (same for output from locale to UTF8)               */
/*   Add some more convenient ways to access the GtkFileSelection properties  */
/*                                                                            */
/*                        file_selection.c                                    */
/*                                                                            */
/*     Copyright (C)	2004- xxxx François Terrot (grisbi@terrot.net)	      */
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

#include "utils_file_selection.h"
#include <gtk/gtk.h>

/** file_selection_set_entry.
 *
 *  Set the "filename" entry widget of the given GtkFileSelection dialog with an UTF-8 string
 *
 * \param filesel GtkFileSelection widget
 * \param utf8string a UTF-8 coded string to be put in the netry field
 *  
 */
void file_selection_set_entry(GtkFileSelection* filesel,const gchar* utf8string)
{
    gtk_entry_set_text ( GTK_ENTRY (filesel->selection_entry),g_filename_from_utf8(utf8string,-1,NULL,NULL,NULL));
}

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
{
    return g_filename_to_utf8(gtk_entry_get_text(GTK_ENTRY (filesel->selection_entry)),-1,NULL,NULL,NULL);
}
/** file_selection_set_filename
 *
 * Set the filename property of the GtkFileSelection after having converted in the locale char set
 *
 * \param filesel GtkFileSelection widget handle
 * \param utf8filename the string to set coded with the UTF8 charset
 */
void file_selection_set_filename(GtkFileSelection* filesel,const gchar* utf8filename)
{
    gtk_file_selection_set_filename(filesel, g_locale_from_utf8(utf8filename,-1,NULL,NULL,NULL));
}
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
{
    return g_filename_to_utf8(gtk_file_selection_get_filename(filesel),-1,NULL,NULL,NULL);
}

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
gchar** file_selection_get_selections(GtkFileSelection* filesel)
{
    gchar** gtk_selections = gtk_file_selection_get_selections(filesel);
    gchar** utf8selections = NULL;
    gint    num_selections = -1;
    gint    idx_selection  = 0;
    
    // Count number of selected file
    while (gtk_selections[++num_selections]);

    // alloc new selection table
    if (num_selections > 0)
    {
        utf8selections = (gchar**)g_malloc((num_selections+1)*sizeof(gchar*));
    }

    // fill the new selection table with the uft8 converted selected strings
    // free the gtk allocated strings (no more needed)
    if (utf8selections)
    {
        for (idx_selection=0;idx_selection <num_selections;idx_selection++)
        {
            utf8selections[idx_selection] = g_filename_to_utf8(gtk_selections[idx_selection],-1,NULL,NULL,NULL);
        }
        utf8selections[idx_selection] = NULL;
    }
    if (gtk_selections)
    {
        g_strfreev(gtk_selections);
    }

    return utf8selections;

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
gchar* file_selection_get_last_directory(GtkFileSelection* filesel,gboolean ended) /* {{{ */
{
    gchar*   dirstr      = g_strdup(GTK_LABEL(GTK_BIN(GTK_OPTION_MENU(filesel->history_pulldown))->child)->label);
    gint     dirstr_len  = strlen(dirstr);
    gchar*   sepstr      = g_strdup(G_DIR_SEPARATOR_S);
    gint     sepstr_len  = strlen(sepstr);
    gboolean is_endedstr = (!((dirstr_len >= sepstr_len + 1) && !strncmp(dirstr+dirstr_len-sepstr_len-1,sepstr,sepstr_len)));

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

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
