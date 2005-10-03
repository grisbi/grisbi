/* ************************************************************************** */
/*                                  utils_editables.c                    */
/*                                                                            */
/*     Copyright (C)	2000-2003 CÃ©dric Auger (cedric@grisbi.org)	      */
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
#include "utils_editables.h"
#include "traitement_variables.h"
#include "utils_str.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean set_text (GtkEntry *entry, gchar *value, 
		   gint length, gint * position);
static gboolean set_text_from_area ( GtkTextBuffer *buffer, gpointer dummy );
/*END_STATIC*/


/*START_EXTERN*/
extern     gchar * buffer ;
/*END_EXTERN*/





/*
 * Creates a new GtkEntry with a pointer to a string that will be
 * modified according to the entry's value.
 *
 * \param value A pointer to a string
 * \param hook An optional function to execute as a handler if the
 * entry's contents are modified.
 * \param data An optional pointer to pass to hooks.
 */
GtkWidget * new_text_entry ( gchar ** value, GCallback hook, gpointer data )
{
    GtkWidget * entry;

    entry = gtk_entry_new ();

    if (value && *value)
	gtk_entry_set_text ( GTK_ENTRY(entry), *value );

    g_object_set_data ( G_OBJECT ( entry ), "pointer", value);

    g_object_set_data ( G_OBJECT ( entry ), "insert-text", 
			(gpointer) g_signal_connect_after (GTK_OBJECT(entry), "insert-text",
							   ((GCallback) set_text), data));
    g_object_set_data ( G_OBJECT ( entry ), "delete-text", 
			(gpointer) g_signal_connect_after (GTK_OBJECT(entry), "delete-text",
							   ((GCallback) set_text), data));
    if ( hook )
    {
	g_object_set_data ( G_OBJECT ( entry ), "insert-hook", 
			    (gpointer) g_signal_connect_after (GTK_OBJECT(entry), 
							       "insert-text",
							       ((GCallback) hook), data));
	g_object_set_data ( G_OBJECT ( entry ), "delete-hook", 
			    (gpointer) g_signal_connect_after (GTK_OBJECT(entry), 
							       "delete-text",
							       ((GCallback) hook), data));
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

    if (value)
	text_view = gsb_new_text_view(*value);
    else
	text_view = gsb_new_text_view(NULL);

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));

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
 * create a normalised text view for grisbi and add the value inside
 *
 * \param value a pointer to a string
 *
 * \return a GtkWidget which is a text_view with the string inside
 * */
GtkWidget *gsb_new_text_view ( gchar *value )
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
gchar *gsb_text_view_get_content ( GtkWidget *text_view )
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
 *  Increment or decrement the value of a GtkEntry.
 *
 * \param entry Entry to change the value of.
 * \param increment Value to add or substract from the numerical value of entry.
 */
void increment_decrement_champ ( GtkWidget *entry, gint increment )
{
    double number;

    number = my_strtod ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entry ))), NULL );
    number += increment;

    gtk_entry_set_text ( GTK_ENTRY ( entry ), utils_str_itoa ( number ) );
}




/**
 * calcule le total du contenu de l'entrÃ©e donnÃ©e en argument 
 * accepte les + et les - 
 * \param entry entrÃ©e d'un montant 
 * \return gdouble total de ce qui est dans l'entrÃ©e 
 */
gdouble calcule_total_entree ( GtkWidget *entry )
{
    gchar *string;
    gchar *pointeur;
    gdouble total;

    total = 0;
	
    string = g_strdup ( gtk_entry_get_text ( GTK_ENTRY ( entry )));

    pointeur = string + strlen (string);

    while ( pointeur != string )
    {
	if ( pointeur[0] == '+'
	     ||
	     pointeur[0] == '-' )
	{
	    total = total + my_strtod ( pointeur,
					NULL );
	    pointeur[0] = 0;
	}
	pointeur--;
    }
    total = total + my_strtod ( pointeur,
				NULL );


    return total;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
