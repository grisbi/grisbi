/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cedric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*          2008-2010 Pierre Biava (grisbi@pierre.biava.name)                 */
/*          https://www.grisbi.org/                                            */
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
/*  along with this program; if not, see <https://www.gnu.org/licenses/>.     */
/*                                                                            */
/* ************************************************************************** */

#include "config.h"

#include "include.h"
#include <string.h>
#include <ctype.h>

/*START_INCLUDE*/
#include "utils_editables.h"
#include "utils_real.h"
#include "utils_str.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/


/**
 * create a normalised text view for grisbi and add the value inside
 *
 * \param value a pointer to a string
 *
 * \return a GtkWidget which is a text_view with the string inside
 * */
GtkWidget *gsb_editable_text_view_new ( const gchar *value )
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

    if (value)
	gtk_text_buffer_set_text (buffer, value, -1);

    return text_view;
}

/**
 * get the entire content of a text view
 *
 * \param the text_view
 *
 * \return a gchar which is the content of the text view
 * */
gchar *gsb_editable_text_view_get_content ( GtkWidget *text_view )
{
    GtkTextIter start, end;
    GtkTextBuffer *buffer;

    g_return_val_if_fail (text_view,
			  NULL);
    g_return_val_if_fail (GTK_IS_TEXT_VIEW (text_view),
			  NULL);

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
    gtk_text_buffer_get_iter_at_offset ( buffer, &start, 0 );
    gtk_text_buffer_get_iter_at_offset ( buffer, &end, -1 );

    return (gtk_text_buffer_get_text (buffer, &start, &end, 0));
}


/**
 * erase the text view given in param
 *
 * \param button not used
 * \param text_view the text_view to erase
 *
 * \return FALSE
 * */
gboolean gsb_editable_erase_text_view ( GtkWidget *button,
					GtkWidget *text_view )
{
    GtkTextBuffer *buffer;
    GtkTextIter start_iter;
    GtkTextIter end_iter;

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
    gtk_text_buffer_get_start_iter ( GTK_TEXT_BUFFER (buffer),
				     &start_iter );
    gtk_text_buffer_get_end_iter ( GTK_TEXT_BUFFER (buffer),
				   &end_iter );
    gtk_text_buffer_delete ( GTK_TEXT_BUFFER (buffer),
			     &start_iter,
			     &end_iter );
    return FALSE;
}

/**
 *  Increment or decrement the value of a GtkEntry.
 *
 * \param entry Entry to change the value of.
 * \param increment Value to add or substract from the numerical value of entry.
 */
void increment_decrement_champ ( GtkWidget *entry, gint increment )
{
	gchar* tmpstr;

    tmpstr = utils_str_incremente_number_from_str ( gtk_entry_get_text ( GTK_ENTRY ( entry ) ), increment );
    gtk_entry_set_text ( GTK_ENTRY ( entry ), tmpstr );

    g_free ( tmpstr );
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
