/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cedric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*          2008-2010 Pierre Biava (grisbi@pierre.biava.name)                 */
/*          http://www.grisbi.org                                             */
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"


/*START_INCLUDE*/
#include "utils_editables.h"
#include "gsb_real.h"
#include "utils_str.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
extern gsb_real null_real;
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


/**
 * calcule le total du contenu de l'entrée donnée en argument 
 * accepte les + et les - 
 * 
 * \param entry entrée d'un montant 
 * 
 * \return gsb_real total de ce qui est dans l'entrée 
 */
gsb_real gsb_utils_edit_calculate_entry ( GtkWidget *entry )
{
    gchar *string;
    gchar *pointeur;
    gsb_real total = null_real;
	
    string = my_strdup ( gtk_entry_get_text ( GTK_ENTRY ( entry ) ) );

    if ( string && strlen ( string ) )
        pointeur = string + strlen ( string );
    else
    {
        g_free ( string );
        return total;
    }

    if ( g_utf8_strchr ( string, -1, '-' ) || g_utf8_strchr ( string, -1, '+' ) )
    {
        while ( pointeur != string )
        {
            if ( pointeur[0] == '+'
                 ||
                 pointeur[0] == '-' )
            {
                total = gsb_real_add ( total,
                            gsb_real_get_from_string ( pointeur ) );
                pointeur[0] = 0;
            }
            
            pointeur--;
        }
        total = gsb_real_add ( total,
                        gsb_real_get_from_string ( pointeur ) );
    }
    else if ( g_utf8_strchr ( string, -1, '*' ) )
    {
        total.mantissa = 1;
        total.exponent = 0;

        while ( pointeur != string )
        {
            if ( pointeur[0] == '*' )
            {
                total = gsb_real_mul ( total,
                            gsb_real_get_from_string ( pointeur + 1 ) );
                pointeur[0] = 0;
            }
            
            pointeur--;
        }
        total = gsb_real_mul ( total,
                        gsb_real_get_from_string ( pointeur ) );
    }
    else if ( g_utf8_strchr ( string, -1, '/' ) )
    {
        gchar **tab;

        tab = g_strsplit ( string, "/", 2 );

        total = gsb_real_div ( gsb_real_get_from_string ( tab[0] ),
                        gsb_real_get_from_string ( tab[1] ) );

        g_strfreev ( tab );
    }
    else
        total = gsb_real_get_from_string ( string );

    g_free ( string );

    return total;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
