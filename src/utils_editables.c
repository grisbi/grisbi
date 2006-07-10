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
#include "gsb_data_account.h"
#include "gsb_data_bank.h"
#include "gsb_real.h"
#include "traitement_variables.h"
#include "utils_str.h"
/*END_INCLUDE*/

/*START_STATIC*/
static  gboolean gsb_editable_changed ( GtkWidget *entry,
				       gboolean default_func (gint, const gchar *));
static  gboolean gsb_editable_int_changed ( GtkWidget *entry,
					   gboolean default_func (gint, gint));
static gboolean gsb_editable_text_area_changed ( GtkTextBuffer *buffer,
					  gboolean default_func (gint, const gchar *));
static gboolean set_text (GtkEntry *entry, gchar *value, 
		   gint length, gint * position);
static gboolean set_text_from_area ( GtkTextBuffer *buffer, gpointer dummy );
static void text_area_set_value ( GtkWidget * text_view, gchar ** value );
/*END_STATIC*/

/*START_EXTERN*/
extern gsb_real null_real ;
/*END_EXTERN*/


/*
 * creates a new GtkEntry wich will modify the value according to the entry
 * but made for values in grisbi structure :
 * for each change, will call the corresponding given function : gsb_data_... ( number, string content )
 * ie the target function must be :
 * 	(default_func) ( gint number_for_func,
 * 			 gchar *string )
 * ex : gsb_data_account_set_name ( account_number, name )
 *
 * \param value a string to fill the entry
 * \param hook an optional function to execute as a handler if the
 * 	entry's contents are modified.
 * 	hook should be :
 * 		gboolean hook ( GtkWidget *entry,
 * 				gpointer data )
 *
 * \param data An optional pointer to pass to hooks.
 * \param default_func The function to call to change the value in memory (function must be func ( number, string ) )
 * \param number_for_func a gint wich we be used to call default_func
 *
 * \return a new GtkEntry
 * */
GtkWidget *gsb_editable_new_text_entry ( gchar *value,
					 GCallback hook,
					 gpointer data,
					 GCallback default_func,
					 gint number_for_func )
{
    GtkWidget * entry;

    /* first, create and fill the entry */
    entry = gtk_entry_new ();

    if (value)
	gtk_entry_set_text ( GTK_ENTRY(entry), value );

    /* set the default func :
     * the func will be send to gsb_editable_set_text by the data,
     * the number_for_func will be set as data for object */
    g_object_set_data ( G_OBJECT (entry),
			"number_for_func", GINT_TO_POINTER (number_for_func));
    g_object_set_data ( G_OBJECT ( entry ), "changed", 
			(gpointer) g_signal_connect_after (GTK_OBJECT(entry), "changed",
							   ((GCallback) gsb_editable_changed), default_func ));
    if ( hook )
	g_object_set_data ( G_OBJECT ( entry ), "changed-hook", 
			    (gpointer) g_signal_connect_after (GTK_OBJECT(entry), "changed",
							      ((GCallback) hook), data ));
    return entry;
}


/** 
 * set the value in a gsb_editable_entry
 * a value is in 2 parts :
 * 	a string, wich be showed in the entry
 * 	a number, wich is used when there is a change in that entry (see gsb_editable_new_text_entry)
 *
 * \param entry
 * \param value a string to set in the entry
 * \param number_for_func the number to give to the called function when something is changed
 *
 * \return
 */
void gsb_editable_set_value ( GtkWidget *entry,
			      gchar *value,
			      gint number_for_func )
{
    /* Block everything */
    if ( g_object_get_data (G_OBJECT (entry), "changed") > 0 )
	g_signal_handler_block ( GTK_OBJECT(entry),
				 (gulong) g_object_get_data (G_OBJECT (entry), 
							     "changed"));
    if ( g_object_get_data (G_OBJECT (entry), "changed-hook") > 0 )
	g_signal_handler_block ( GTK_OBJECT(entry),
				 (gulong) g_object_get_data (G_OBJECT (entry), 
							     "changed-hook"));

    /* Fill in value */
    if (value)
	gtk_entry_set_text ( GTK_ENTRY ( entry ), value );
    else
	gtk_entry_set_text ( GTK_ENTRY ( entry ), "" );

    g_object_set_data ( G_OBJECT (entry),
			"number_for_func", GINT_TO_POINTER (number_for_func));

    /* Unblock everything */
    if ( g_object_get_data (G_OBJECT (entry), "changed") > 0 )
	g_signal_handler_unblock ( GTK_OBJECT(entry),
				   (gulong) g_object_get_data (G_OBJECT (entry), 
							       "changed"));
    if ( g_object_get_data (G_OBJECT (entry), "changed-hook") > 0 )
	g_signal_handler_unblock ( GTK_OBJECT(entry),
				   (gulong) g_object_get_data (G_OBJECT (entry), 
							       "changed-hook"));
}



/**
 * called when something change in an entry of a gsb_editable_text
 * by gsb_editable_new_text_entry
 *
 * \param entry The reference GtkEntry
 * \param default_func the function to call to change the value in memory
 *
 * \return FALSE
 */
static gboolean gsb_editable_changed ( GtkWidget *entry,
				       gboolean default_func (gint, const gchar *))
{
    gint number_for_func;

    /* just to be sure... */
    if (!default_func || !entry)
	return FALSE;

    number_for_func = GPOINTER_TO_INT ( g_object_get_data (G_OBJECT (entry), "number_for_func"));
    default_func ( number_for_func,
		   gtk_entry_get_text ( GTK_ENTRY (entry)));

    /* Mark file as modified */
    modification_fichier ( TRUE );

    return FALSE;
}




/*
 * Creates a new GtkEntry with a pointer to a string that will be
 * modified according to the entry's value.
 * FIXME : to delete
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
 * FIXME : to delete
 */
void entry_set_value ( GtkWidget * entry, gchar ** value )
{
    printf ( "devrait pas venir ici (entry_set_value), arrêt du prog\n" );
    exit (0);
    /* Block everything */
/*     if ( g_object_get_data ((GObject*) entry, "changed") > 0 ) */
/* 	g_signal_handler_block ( GTK_OBJECT(entry), */
/* 				 (gulong) g_object_get_data ((GObject*) entry,  */
/* 							     "changed")); */
/*     if ( g_object_get_data ((GObject*) entry, "changed-hook") > 0 ) */
/* 	g_signal_handler_block ( GTK_OBJECT(entry), */
/* 				 (gulong) g_object_get_data ((GObject*) entry,  */
/* 							     "changed-hook")); */

    /* Fill in value */
/*     if (value && *value) */
/* 	gtk_entry_set_text ( GTK_ENTRY ( entry ), *value ); */
/*     else */
/* 	gtk_entry_set_text ( GTK_ENTRY ( entry ), "" ); */
/*  */
/*     g_object_set_data ( G_OBJECT(entry), "pointer", value ); */

    /* Unblock everything */
/*     if ( g_object_get_data ((GObject*) entry, "changed") > 0 ) */
/* 	g_signal_handler_unblock ( GTK_OBJECT(entry), */
/* 				   (gulong) g_object_get_data ((GObject*) entry,  */
/* 							       "insert-hook")); */
/*     if ( g_object_get_data ((GObject*) entry, "changed") > 0 ) */
/* 	g_signal_handler_unblock ( GTK_OBJECT(entry), */
/* 				   (gulong) g_object_get_data ((GObject*) entry,  */
/* 							       "insert-text")); */
/*     if ( g_object_get_data ((GObject*) entry, "changed") > 0 ) */
/* 	g_signal_handler_unblock ( GTK_OBJECT(entry), */
/* 				   (gulong) g_object_get_data ((GObject*) entry,  */
/* 							       "delete-hook")); */
/*     if ( g_object_get_data ((GObject*) entry, "changed") > 0 ) */
/* 	g_signal_handler_unblock ( GTK_OBJECT(entry), */
/* 				   (gulong) g_object_get_data ((GObject*) entry,  */
/* 							       "delete-text")); */
}



/**
 * Set a string to the value of an GtkEntry.
 * FIXME : to delete
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
	*data = my_strdup ((gchar*) gtk_entry_get_text ( GTK_ENTRY (entry) ));

    return FALSE;
}


/*
 * creates a new GtkTextView wich will automatickly modify the value according to the text in memory
 * in grisbi structure :
 * for each change, will call the corresponding given function : gsb_data_... ( number, string content )
 * ie the target function must be :
 * 	(default_func) ( gint number_for_func,
 * 			 gchar *string )
 * ex : gsb_data_bank_set_bank_note ( account_number, text )
 * rem : do the same as gsb_editable_new_text_entry but for a text_view
 *
 * \param value a string to fill the text_view
 * \param hook an optional function to execute as a handler if the
 * 	text_view's contents are modified : !!! send the text_buffer, and not the text_view
 * 	hook should be :
 * 		gboolean hook ( GtkTextBuffer *text_buffer,
 * 				gpointer data )
 *
 * \param data An optional pointer to pass to hooks.
 * \param default_func The function to call to change the value in memory (function must be func ( number, string ) )
 * \param number_for_func a gint wich we be used to call default_func
 *
 * \return a new GtkTextView
 * */
GtkWidget *gsb_editable_new_text_area ( gchar *value,
					GCallback hook,
					gpointer data,
					GCallback default_func,
					gint number_for_func )
{
    GtkWidget *text_view;
    GtkTextBuffer *buffer;

    text_view = gsb_editable_text_view_new(value);

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));

    /* set the default function and save the number_for_func */
    g_object_set_data ( G_OBJECT (buffer),
			"number_for_func", GINT_TO_POINTER (number_for_func));
    g_object_set_data ( G_OBJECT ( buffer ), "changed",
			(gpointer) g_signal_connect_after (G_OBJECT(buffer), "changed",
							   ((GCallback) gsb_editable_text_area_changed), default_func ));
    if ( hook )
	g_object_set_data ( G_OBJECT ( buffer ), "changed-hook",
			    (gpointer) g_signal_connect_after  (G_OBJECT(buffer), "changed",
								((GCallback) hook), data ));
    return text_view;
}


/** 
 * set the value in a gsb_editable_text_area
 * a value is in 2 parts :
 * 	a string, wich be showed in the text_view
 * 	a number, wich is used when there is a change in that text_view (see gsb_editable_new_text_entry)
 *
 * \param text_view
 * \param value a string to set in the text_view
 * \param number_for_func the number to give to the called function when something is changed
 *
 * \return
 */
void gsb_editable_text_area_set_value ( GtkWidget *text_view,
					gchar *value,
					gint number_for_func )
{
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));

    /* Block everything */
    if ( g_object_get_data (G_OBJECT(buffer), "changed") > 0 )
	g_signal_handler_block ( G_OBJECT(buffer),
				 (gulong) g_object_get_data ( G_OBJECT(buffer), "changed" ));
    if ( g_object_get_data (G_OBJECT(buffer), "changed-hook") > 0 )
	g_signal_handler_block ( G_OBJECT(buffer),
				 (gulong) g_object_get_data ( G_OBJECT(buffer), 
							      "changed-hook" ));

    /* Fill in value */
    if (value)
	gtk_text_buffer_set_text (buffer, value, -1 );
    else
	gtk_text_buffer_set_text (buffer, "", -1 );

    g_object_set_data ( G_OBJECT (buffer),
			"number_for_func", GINT_TO_POINTER (number_for_func));

    /* Unblock everything */
    if ( g_object_get_data (G_OBJECT(buffer), "changed") > 0 )
	g_signal_handler_unblock ( G_OBJECT(buffer),
				   (gulong) g_object_get_data ( G_OBJECT(buffer), 
							       "changed" ));
    if ( g_object_get_data (G_OBJECT(buffer), "change-hook") > 0 )
	g_signal_handler_unblock ( G_OBJECT(buffer),
				   (gulong) g_object_get_data ( G_OBJECT(buffer), 
							       "change-hook" ));
}


/**
 * Set a string to the value of an GtkTextView
 *
 * \param buffer The reference GtkTextBuffer
 * \param dummy Handler parameter.  Not used.
 */
gboolean gsb_editable_text_area_changed ( GtkTextBuffer *buffer,
					  gboolean default_func (gint, const gchar *))
{
    GtkTextIter start, end;
    gint number_for_func;

    /* just to be sure... */
    if (!default_func || !buffer)
	return FALSE;

    number_for_func = GPOINTER_TO_INT ( g_object_get_data (G_OBJECT (buffer), "number_for_func"));

    gtk_text_buffer_get_iter_at_offset ( buffer, &start, 0 );
    gtk_text_buffer_get_iter_at_offset ( buffer, &end, -1 );

    default_func ( number_for_func,
		   gtk_text_buffer_get_text (buffer, &start, &end, 0));

    /* Mark file as modified */
    modification_fichier ( TRUE );

    return FALSE;
}

/*
 * creates a new GtkEntry wich will modify the value according to the entry for a gint
 * but made for values in grisbi structure :
 * for each change, will call the corresponding given function : gsb_data_... ( number, gint value content )
 * ie the target function must be :
 * 	(default_func) ( gint number_for_func,
 * 			 gint value )
 * ex : gsb_data_account_get_column_sort ( gint account_number,
 *						 gint no_column )
 *
 * \param value a gint to fill the entry
 * \param hook an optional function to execute as a handler if the
 * 	entry's contents are modified.
 * 	hook should be :
 * 		gboolean hook ( GtkWidget *entry,
 * 				gpointer data )
 *
 * \param data An optional pointer to pass to hooks.
 * \param default_func The function to call to change the value in memory (function must be func ( number, gint ) )
 * \param number_for_func a gint wich we be used to call default_func
 *
 * \return a new GtkEntry
 * */
GtkWidget *gsb_editable_new_int_entry ( gint value,
					GCallback hook,
					gpointer data,
					GCallback default_func,
					gint number_for_func )
{
    GtkWidget * entry;

    /* first, create and fill the entry */
    entry = gtk_entry_new ();

    gtk_entry_set_text ( GTK_ENTRY(entry),
			 utils_str_itoa (value));

    /* set the default func :
     * the func will be send to gsb_editable_set_text by the data,
     * the number_for_func will be set as data for object */
    g_object_set_data ( G_OBJECT (entry),
			"number_for_func", GINT_TO_POINTER (number_for_func));
    g_object_set_data ( G_OBJECT ( entry ), "changed", 
			(gpointer) g_signal_connect_after (GTK_OBJECT(entry), "changed",
							   ((GCallback) gsb_editable_int_changed), default_func ));
    if ( hook )
	g_object_set_data ( G_OBJECT ( entry ), "changed-hook", 
			    (gpointer) g_signal_connect_after (GTK_OBJECT(entry), "changed",
							       ((GCallback) hook), data ));
    return entry;
}


/** 
 * set the value in a gsb_editable_int_entry
 * a value is in 2 parts :
 * 	a string, wich be showed in the entry
 * 	a number, wich is used when there is a change in that entry (see gsb_editable_new_int_entry)
 *
 * \param entry
 * \param value a gint to set in the entry
 * \param number_for_func the number to give to the called function when something is changed
 *
 * \return
 */
void gsb_editable_int_set_value ( GtkWidget *entry,
				  gint value,
				  gint number_for_func )
{
    /* Block everything */
    if ( g_object_get_data (G_OBJECT (entry), "changed") > 0 )
	g_signal_handler_block ( GTK_OBJECT(entry),
				 (gulong) g_object_get_data (G_OBJECT (entry), 
							     "changed"));
    if ( g_object_get_data (G_OBJECT (entry), "changed-hook") > 0 )
	g_signal_handler_block ( GTK_OBJECT(entry),
				 (gulong) g_object_get_data (G_OBJECT (entry), 
							     "changed-hook"));

    /* Fill in value */
    gtk_entry_set_text ( GTK_ENTRY(entry),
			 utils_str_itoa (value));

    g_object_set_data ( G_OBJECT (entry),
			"number_for_func", GINT_TO_POINTER (number_for_func));

    /* Unblock everything */
    if ( g_object_get_data (G_OBJECT (entry), "changed") > 0 )
	g_signal_handler_unblock ( GTK_OBJECT(entry),
				   (gulong) g_object_get_data (G_OBJECT (entry), 
							       "changed"));
    if ( g_object_get_data (G_OBJECT (entry), "changed-hook") > 0 )
	g_signal_handler_unblock ( GTK_OBJECT(entry),
				   (gulong) g_object_get_data (G_OBJECT (entry), 
							       "changed-hook"));
}


/**
 * erase the entry of the int_editable
 * used because if we give 0 0 to set value, will show 0 in the entry but
 * cannot erase it
 * this function will delete too the number_for_func associated with the entry
 *
 * \param entry
 *
 * \return
 * */
void gsb_editable_int_erase_entry ( GtkWidget *entry )
{
    /* Block everything */
    if ( g_object_get_data (G_OBJECT (entry), "changed") > 0 )
	g_signal_handler_block ( GTK_OBJECT(entry),
				 (gulong) g_object_get_data (G_OBJECT (entry), 
							     "changed"));
    if ( g_object_get_data (G_OBJECT (entry), "changed-hook") > 0 )
	g_signal_handler_block ( GTK_OBJECT(entry),
				 (gulong) g_object_get_data (G_OBJECT (entry), 
							     "changed-hook"));

    /* Fill in value */
    gtk_entry_set_text ( GTK_ENTRY(entry),
			 "" );

    g_object_set_data ( G_OBJECT (entry),
			"number_for_func", NULL );

    /* Unblock everything */
    if ( g_object_get_data (G_OBJECT (entry), "changed") > 0 )
	g_signal_handler_unblock ( GTK_OBJECT(entry),
				   (gulong) g_object_get_data (G_OBJECT (entry), 
							       "changed"));
    if ( g_object_get_data (G_OBJECT (entry), "changed-hook") > 0 )
	g_signal_handler_unblock ( GTK_OBJECT(entry),
				   (gulong) g_object_get_data (G_OBJECT (entry), 
							       "changed-hook"));
}


/**
 * called when something change in an entry of a gsb_editable_int
 * by gsb_editable_new_int_entry
 *
 * \param entry The reference GtkEntry
 * \param default_func the function to call to change the value in memory
 *
 * \return FALSE
 */
static gboolean gsb_editable_int_changed ( GtkWidget *entry,
					   gboolean default_func (gint, gint))
{
    gint number_for_func;

    /* just to be sure... */
    if (!default_func || !entry)
	return FALSE;

    number_for_func = GPOINTER_TO_INT ( g_object_get_data (G_OBJECT (entry), "number_for_func"));
    default_func ( number_for_func,
		   utils_str_atoi (gtk_entry_get_text ( GTK_ENTRY (entry))));

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
	text_view = gsb_editable_text_view_new(*value);
    else
	text_view = gsb_editable_text_view_new(NULL);

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
	*data = my_strdup ( gtk_text_buffer_get_text (buffer, &start, &end, 0) );

    /* Mark file as modified */
    modification_fichier ( TRUE );

    return FALSE;
}


/**
 * create a normalised text view for grisbi and add the value inside
 *
 * \param value a pointer to a string
 *
 * \return a GtkWidget which is a text_view with the string inside
 * */
GtkWidget *gsb_editable_text_view_new ( gchar *value )
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
 *  Increment or decrement the value of a GtkEntry.
 *
 * \param entry Entry to change the value of.
 * \param increment Value to add or substract from the numerical value of entry.
 */
void increment_decrement_champ ( GtkWidget *entry, gint increment )
{
    gdouble number;

    number = utils_str_atoi (gtk_entry_get_text ( GTK_ENTRY ( entry )));
    number += increment;

    gtk_entry_set_text ( GTK_ENTRY ( entry ), utils_str_itoa ( number ) );
}


/**
 * calcule le total du contenu de l'entrÃ©e donnÃ©e en argument 
 * accepte les + et les - 
 * 
 * \param entry entrÃ©e d'un montant 
 * 
 * \return gsb_real total de ce qui est dans l'entrÃ©e 
 */
gsb_real gsb_utils_edit_calculate_entry ( GtkWidget *entry )
{
    gchar *string;
    gchar *pointeur;
    gsb_real total = null_real;
	
    string = g_strdup (gtk_entry_get_text ( GTK_ENTRY (entry)));
    pointeur = string + strlen (string);

    while ( pointeur != string )
    {
	if ( pointeur[0] == '+'
	     ||
	     pointeur[0] == '-' )
	{
	    total = gsb_real_add ( total,
				   gsb_real_get_from_string (pointeur));
	    pointeur[0] = 0;
	}
	pointeur--;
    }
    total = gsb_real_add ( total,
			   gsb_real_get_from_string (pointeur));
    g_free (string);
    return total;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
