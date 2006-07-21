/* ************************************************************************** */
/*                                  gsb_autofunc.c                            */
/*                                                                            */
/*     Copyright (C)	2000-2006 Cedric Auger (cedric@grisbi.org)	      */
/*			2003-2006 Benjamin Drieu (bdrieu@april.org)	      */
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

/**
 * \file gsb_autofunc.c
 * this file groups some widget functions wich update value automatickly
 * when the widget change
 * a change in the widget call a given function of kind gsb_data_..._set_... ( number, value )
 * use to set automatickly the content in the grisbi structures
 * (for changes only on variables, see gsb_automem.c)
 */

#include "include.h"


/*START_INCLUDE*/
#include "gsb_autofunc.h"
#include "utils_dates.h"
#include "calendar.h"
#include "gsb_data_account.h"
#include "gsb_data_bank.h"
#include "gsb_data_category.h"
#include "gsb_data_fyear.h"
#include "utils_editables.h"
#include "traitement_variables.h"
#include "utils_str.h"
#include "structures.h"
#include "include.h"
/*END_INCLUDE*/

/*START_STATIC*/
static  gboolean gsb_autofunc_checkbutton_changed ( GtkWidget *button,
						   gboolean default_func (gint, gboolean));
static  gboolean gsb_autofunc_date_changed ( GtkWidget *entry,
					    gboolean default_func (gint, const GDate *));
static  gboolean gsb_autofunc_date_check ( GtkWidget *entry,
					  gpointer null );
static  gboolean gsb_autofunc_date_focus_out ( GtkWidget *entry,
					      GdkEventFocus *ev,
					      gpointer null );
static GDate *gsb_autofunc_date_get ( GtkWidget *hbox );
static  gboolean gsb_autofunc_date_popup_calendar ( GtkWidget *button,
						   GtkWidget *entry );
static  gboolean gsb_autofunc_entry_changed ( GtkWidget *entry,
					     gboolean default_func (gint, const gchar *));
static  gboolean gsb_autofunc_int_changed ( GtkWidget *entry,
					   gboolean default_func (gint, gint));
static  gboolean gsb_autofunc_textview_changed ( GtkTextBuffer *buffer,
						gboolean default_func (gint, const gchar *));
/*END_STATIC*/


/*START_EXTERN*/
extern GtkTreeSelection * selection;
extern GtkWidget *window;
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
 * \param default_func The function to call to change the value in memory (function must be func ( number, string ) ) or NULL
 * \param number_for_func a gint wich we be used to call default_func (will be saved as g_object_set_data with "number_for_func")
 *
 * \return a new GtkEntry
 * */
GtkWidget *gsb_autofunc_entry_new ( const gchar *value,
				    GCallback hook,
				    gpointer data,
				    GCallback default_func,
				    gint number_for_func )
{
    GtkWidget *entry;

    /* first, create and fill the entry */
    entry = gtk_entry_new ();

    if (value)
	gtk_entry_set_text ( GTK_ENTRY(entry), value );

    /* set the default func :
     * the func will be send to gsb_editable_set_text by the data,
     * the number_for_func will be set as data for object */
    g_object_set_data ( G_OBJECT (entry),
			"number_for_func", GINT_TO_POINTER (number_for_func));
    if (default_func)
	g_object_set_data ( G_OBJECT ( entry ), "changed", 
			    (gpointer) g_signal_connect_after (G_OBJECT(entry), "changed",
							       G_CALLBACK (gsb_autofunc_entry_changed), default_func ));
    if ( hook )
	g_object_set_data ( G_OBJECT ( entry ), "changed-hook", 
			    (gpointer) g_signal_connect_after (G_OBJECT(entry), "changed",
							       G_CALLBACK (hook), data ));
    return entry;
}


/** 
 * set the value in a gsb_editable_entry
 * a value is in 2 parts :
 * 	a string, wich be showed in the entry
 * 	a number, wich is used when there is a change in that entry (see gsb_autofunc_entry_new)
 *
 * \param entry
 * \param value a string to set in the entry
 * \param number_for_func the number to give to the called function when something is changed
 *
 * \return
 */
void gsb_autofunc_entry_set_value ( GtkWidget *entry,
				    const gchar *value,
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
 * by gsb_autofunc_entry_new
 *
 * \param entry The reference GtkEntry
 * \param default_func the function to call to change the value in memory
 *
 * \return FALSE
 */
static gboolean gsb_autofunc_entry_changed ( GtkWidget *entry,
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
 * creates a new GtkTextView wich will automatickly modify the value according to the text in memory
 * in grisbi structure :
 * for each change, will call the corresponding given function : gsb_data_... ( number, string content )
 * ie the target function must be :
 * 	(default_func) ( gint number_for_func,
 * 			 gchar *string )
 * ex : gsb_data_bank_set_bank_note ( account_number, text )
 * rem : do the same as gsb_autofunc_entry_new but for a text_view
 *
 * \param value a string to fill the text_view
 * \param hook an optional function to execute as a handler if the
 * 	text_view's contents are modified : !!! send the text_buffer, and not the text_view
 * 	hook should be :
 * 		gboolean hook ( GtkTextBuffer *text_buffer,
 * 				gpointer data )
 *
 * \param data An optional pointer to pass to hooks.
 * \param default_func The function to call to change the value in memory (function must be func ( number, string ) ) or NULL
 * \param number_for_func a gint wich we be used to call default_func (will be saved as g_object_set_data with "number_for_func")
 *
 * \return a new GtkTextView
 * */
GtkWidget *gsb_autofunc_textview_new ( const gchar *value,
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
    if (default_func)
	g_object_set_data ( G_OBJECT ( buffer ), "changed",
			    (gpointer) g_signal_connect_after (G_OBJECT(buffer), "changed",
							       ((GCallback) gsb_autofunc_textview_changed), default_func ));
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
 * 	a number, wich is used when there is a change in that text_view (see gsb_autofunc_entry_new)
 *
 * \param text_view
 * \param value a string to set in the text_view
 * \param number_for_func the number to give to the called function when something is changed
 *
 * \return
 */
void gsb_autofunc_textview_set_value ( GtkWidget *text_view,
				       const gchar *value,
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
static gboolean gsb_autofunc_textview_changed ( GtkTextBuffer *buffer,
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
 * \param default_func The function to call to change the value in memory (function must be func ( number, gint ) ) or NULL
 * \param number_for_func a gint wich we be used to call default_func (will be saved as g_object_set_data with "number_for_func")
 *
 * \return a new GtkEntry
 * */
GtkWidget *gsb_autofunc_int_new ( gint value,
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
    if (default_func)
	g_object_set_data ( G_OBJECT ( entry ), "changed", 
			    (gpointer) g_signal_connect_after (GTK_OBJECT(entry), "changed",
							       ((GCallback) gsb_autofunc_int_changed), default_func ));
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
 * 	a number, wich is used when there is a change in that entry (see gsb_autofunc_int_new)
 *
 * \param entry
 * \param value a gint to set in the entry
 * \param number_for_func the number to give to the called function when something is changed
 *
 * \return
 */
void gsb_autofunc_int_set_value ( GtkWidget *entry,
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
void gsb_autofunc_int_erase_entry ( GtkWidget *entry )
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
 * by gsb_autofunc_int_new
 *
 * \param entry The reference GtkEntry
 * \param default_func the function to call to change the value in memory
 *
 * \return FALSE
 */
static gboolean gsb_autofunc_int_changed ( GtkWidget *entry,
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
 * Creates a new GtkHBox with an entry to type in a date and a button to show a calendar
 * change value change directly in memory by a call to default_func with number_for_func parameter,
 * the target function must be :
 * 	(default_func) ( gint number_for_func,
 * 			 GDate *date )
 * ex : gsb_data_fyear_set_begining_date ( gint fyear_number, GDate *date )
 *
 * \param date the date we want to fill the entry, or NULL
 * \param An optional hook function to execute if the entry is modified
 * 	hook should be :
 * 		gboolean hook ( GtkWidget *entry,
 * 				gpointer data )
 * \param data some data to pass to hook
 * \param default_func the function to call
 * \param number_fo_func a gint used to call the default_func
 *
 * \return a widget hbox with the entry and a button
 */
GtkWidget *gsb_autofunc_date_new ( const gchar *value,
				   GCallback hook,
				   gpointer data,
				   GCallback default_func,
				   gint number_for_func )
{
    GtkWidget *entry, *hbox, *button;
    GtkStyle *style;
    GdkColor normal_color;
    GdkColor red_color;

    /* the hbox will contain an entry an a button */
    hbox = gtk_hbox_new ( FALSE, 6 );

    /* create and fill the entry */
    entry = gtk_entry_new ();
    gtk_box_pack_start ( GTK_BOX(hbox), entry,
			 TRUE, TRUE, 0 );
    g_object_set_data ( G_OBJECT (entry),
			"number_for_func", GINT_TO_POINTER (number_for_func));

    /* changing the style makes something not beautiful with the frame,
     * i think it's possible to change that, but without frame is pretty
     * for me, so set like that for now */
    gtk_entry_set_has_frame ( GTK_ENTRY (entry),
			      FALSE );

    /* set the red/normal style */
    normal_color.red = COULEUR_NOIRE_RED;
    normal_color.green = COULEUR_NOIRE_GREEN;
    normal_color.blue = COULEUR_NOIRE_BLUE;
    normal_color.pixel = 0;

    red_color.red = COULEUR_ROUGE_RED;
    red_color.green = COULEUR_ROUGE_GREEN;
    red_color.blue = COULEUR_ROUGE_BLUE;
    red_color.pixel = 0;

    /* prepare the style */
    style = gtk_style_new ();
    style -> text[GTK_STATE_NORMAL] = normal_color;
    g_object_set_data ( G_OBJECT (entry),
			"normal_style", style );

    style = gtk_style_new ();
    style -> text[GTK_STATE_NORMAL] = red_color;
    g_object_set_data ( G_OBJECT (entry),
			"red_style", style );

    /* set the function wich check what is writen in real time */
    g_signal_connect_after ( G_OBJECT (entry), "changed",
			     G_CALLBACK (gsb_autofunc_date_check),
			     NULL );

    /* set the focus out function, wich will try to find a good date
     * from the date entried (ie 1/1/1 will give 01/01/2001) */
    g_signal_connect_after ( G_OBJECT (entry), "focus-out-event",
			     G_CALLBACK (gsb_autofunc_date_focus_out),
			     NULL );

    /* set the content, that will check the date too */
    if (value)
	gtk_entry_set_text ( GTK_ENTRY(entry), value );

    /* set the default func */
    if (default_func)
	g_object_set_data ( G_OBJECT ( entry ), "changed", 
			    (gpointer) g_signal_connect_after (G_OBJECT(entry), "changed",
							       G_CALLBACK (gsb_autofunc_date_changed), default_func ));
    if ( hook )
	g_object_set_data ( G_OBJECT ( entry ), "changed-hook", 
			    (gpointer) g_signal_connect_after (G_OBJECT(entry), "changed",
							       ((GCallback) hook), data ));

    /* create the button to show a calendar */
    button = gtk_button_new_with_label ("...");
    gtk_button_set_relief ( GTK_BUTTON (button),
			    GTK_RELIEF_NONE );
    gtk_box_pack_start ( GTK_BOX(hbox), button,
			 FALSE, FALSE, 0 );
    g_signal_connect ( G_OBJECT ( button ), "clicked",
		       G_CALLBACK (gsb_autofunc_date_popup_calendar), entry );

    /* to find it easily, set the adr to the entry  and the hbox */
    g_object_set_data ( G_OBJECT (hbox),
			"entry", entry );

    return hbox;
}

/**
 * set a date in an editable_date
 * 2 parts : a date and a number used to call the default func
 *
 * \param hbox The gsb_editable_date widget.
 * \param value The new date to modify.
 * \param number_for_func to call the default func
 */
void gsb_autofunc_date_set_value ( GtkWidget *hbox,
				   GDate *value,
				   gint number_for_func )
{
    GtkWidget * entry;

    /* Find associated gtkentry */
    entry = g_object_get_data ( G_OBJECT(hbox), "entry" );

    /* Block everything */
    if ( g_object_get_data (G_OBJECT (entry), "changed") > 0 )
	g_signal_handler_block ( G_OBJECT(entry),
				 (gulong) g_object_get_data (G_OBJECT (entry), 
							     "changed"));
    if ( g_object_get_data (G_OBJECT (entry), "changed-hook") > 0 )
	g_signal_handler_block ( G_OBJECT(entry),
				 (gulong) g_object_get_data (G_OBJECT (entry), 
							     "changed-hook"));

    if (value)
	gtk_entry_set_text ( GTK_ENTRY(entry),
			     gsb_format_gdate (value));
    else
	gtk_entry_set_text ( GTK_ENTRY (entry),
			     "" );

    g_object_set_data ( G_OBJECT ( entry ),
			"number_for_func", GINT_TO_POINTER (number_for_func));

    /* Unblock everything */
    if ( g_object_get_data (G_OBJECT (entry), "changed") > 0 )
	g_signal_handler_unblock ( G_OBJECT(entry),
				   (gulong) g_object_get_data (G_OBJECT (entry), 
							       "changed"));
    if ( g_object_get_data (G_OBJECT (entry), "changed-hook") > 0 )
	g_signal_handler_unblock ( G_OBJECT(entry),
				   (gulong) g_object_get_data (G_OBJECT (entry), 
							       "changed-hook"));
}


/**
 * get the date in the entry and return it as a GDate
 * if the date is not valid, return NULL
 *
 * \param hbox the gsb_editable_date
 *
 * \return a GDate or NULL
 * */
GDate *gsb_autofunc_date_get ( GtkWidget *hbox )
{
    GtkWidget * entry;
    GDate *date;

    /* Find associated gtkentry */
    entry = g_object_get_data ( G_OBJECT(hbox), "entry" );

    date = gsb_parse_date_string (gtk_entry_get_text (GTK_ENTRY (entry)));
    return date;
}


/**
 * check the content of the date entry each time something is written
 * set the content red if not a good date
 *
 * \param entry
 * \param null
 *
 * \return FALSE
 * */
static gboolean gsb_autofunc_date_check ( GtkWidget *entry,
					  gpointer null )
{
    GDate *date;
    GtkStyle *style;

    date = gsb_parse_date_string (gtk_entry_get_text ( GTK_ENTRY (entry)));

    if (date)
    {
	/* the content is ok */
	style = g_object_get_data ( G_OBJECT (entry),
				    "normal_style" );
	g_date_free (date);
    }
    else
	/* the date is not correct */
	style = g_object_get_data ( G_OBJECT (entry),
				    "red_style" );

    gtk_widget_set_style ( entry,
			   style );
    return FALSE;
}


/**
 * called when the editable date receive a focus out
 * try to make a good date from an user entry
 * ie 1/1/1 => 01/01/2001
 *
 * \param entry
 * \param ev the GdkEventFocus
 * \param null not used
 *
 * \return FALSE
 * */
static gboolean gsb_autofunc_date_focus_out ( GtkWidget *entry,
					      GdkEventFocus *ev,
					      gpointer null )
{
    GDate *date;

    if (!entry)
	return FALSE;

    date = gsb_parse_date_string (gtk_entry_get_text (GTK_ENTRY (entry)));

    if (!date)
	return FALSE;

    gtk_entry_set_text ( GTK_ENTRY ( entry ),
			 gsb_format_gdate (date));
    g_date_free (date);

    return FALSE;
}


/**
 * called when something change in an entry of a gsb_editable_date
 * by gsb_autofunc_date_new
 *
 * \param entry The reference GtkEntry
 * \param default_func the function to call to change the value in memory
 *
 * \return FALSE
 */
static gboolean gsb_autofunc_date_changed ( GtkWidget *entry,
					    gboolean default_func (gint, const GDate *))
{
    gint number_for_func;

    /* just to be sure... */
    if (!default_func || !entry)
	return FALSE;

    number_for_func = GPOINTER_TO_INT ( g_object_get_data (G_OBJECT (entry), "number_for_func"));
    default_func ( number_for_func,
		   gsb_parse_date_string (gtk_entry_get_text ( GTK_ENTRY (entry))));

    /* Mark file as modified */
    modification_fichier ( TRUE );

    return FALSE;
}



/**
 * Pop up a window with a calendar that allows a date selection.  This
 * calendar runs "date_selectionnee" as a callback if a date is
 * selected.
 *
 * \param button Normally a GtkButton that triggered the handler.
 * This parameter will be used as a base to set popup's position.
 * This widget must also have a parameter (data) of name "entry"
 * which contains a pointer to a GtkEntry used to set initial value of
 * calendar.
 * \param entry the entry of the gsb_editable_date
 */
static gboolean gsb_autofunc_date_popup_calendar ( GtkWidget *button,
						   GtkWidget *entry )
{
    GtkWidget *popup, *popup_boxv, *calendar, *cancel_button, *frame;
    GtkRequisition entry_size, popup_size;
    GDate *date;
    gint x, y;

    /* Find popup position */
    gdk_window_get_origin ( GTK_BUTTON (button) -> event_window, &x, &y );
    gtk_widget_size_request ( GTK_WIDGET (button), &entry_size );
    y = y + entry_size.height;

    /* Create popup */
    popup = gtk_window_new ( GTK_WINDOW_POPUP );
    gtk_window_set_modal ( GTK_WINDOW (popup), TRUE);

    /* Create popup widgets */
    frame = gtk_frame_new ( NULL );
    gtk_container_add ( GTK_CONTAINER (popup), frame);
    popup_boxv = gtk_vbox_new ( FALSE, 5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( popup_boxv ), 5 );
    gtk_container_add ( GTK_CONTAINER ( frame ), popup_boxv);

    /* Set initial date according to entry */
    if ( strlen (gtk_entry_get_text (GTK_ENTRY(entry))))
	date = gsb_parse_date_string (gtk_entry_get_text (GTK_ENTRY (entry)));
    else
	date = gdate_today ();

    /* Creates calendar */
    calendar = gtk_calendar_new();
    gtk_calendar_select_month ( GTK_CALENDAR ( calendar ), g_date_get_month ( date ) - 1,
				g_date_get_year ( date ) );
    gtk_calendar_select_day  ( GTK_CALENDAR ( calendar ), g_date_get_day ( date ) );
    gtk_calendar_display_options ( GTK_CALENDAR ( calendar ),
				   GTK_CALENDAR_SHOW_HEADING |
				   GTK_CALENDAR_SHOW_DAY_NAMES |
				   GTK_CALENDAR_WEEK_START_MONDAY );

    /* Create handlers */
    g_signal_connect ( G_OBJECT (calendar), "day-selected-double-click",
		       G_CALLBACK (gsb_calendar_select_date), entry );
    g_signal_connect ( G_OBJECT (calendar), "key-press-event",
		       G_CALLBACK (gsb_calendar_key_press_event), entry );
    gtk_box_pack_start ( GTK_BOX ( popup_boxv ),
			 calendar,
			 TRUE, TRUE, 0 );

    /* Add the "cancel" button */
    cancel_button = gtk_button_new_with_label ( _("Cancel") );
    g_signal_connect_swapped ( G_OBJECT (cancel_button), "clicked",
			       G_CALLBACK (gtk_widget_destroy),
			       G_OBJECT ( popup ) );
    gtk_box_pack_start ( GTK_BOX ( popup_boxv ), cancel_button,
			 TRUE, TRUE, 0 );


    /* Show everything */
    gtk_widget_show_all ( popup );
    gtk_widget_set_uposition ( GTK_WIDGET ( popup ), x, y );
    gtk_widget_size_request ( GTK_WIDGET ( popup ), &popup_size );
    gtk_widget_set_uposition ( GTK_WIDGET ( popup ), 
			       x-popup_size.width+entry_size.width, y );

    /* Grab pointer */
    gdk_pointer_grab ( popup -> window, TRUE,
		       GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
		       GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK |
		       GDK_POINTER_MOTION_MASK,
		       NULL, NULL, GDK_CURRENT_TIME );
    return FALSE;
}


/*
 * creates a new checkbox associated to a value in a grisbi structure
 * for each change, will call the corresponding given function : gsb_data_... ( number, gboolean )
 * ie the target function must be :
 * 	(default_func) ( gint number_for_func,
 * 			 gboolean yes/no )
 * ex : gsb_data_fyear_set_form_show ( fyear_number, showed_in_form )
 *
 * \param label the text associated to the checkbox
 * \param value a boolean for the state of the checkbox
 * \param hook an optional function to execute as a handler if the
 * 	button is modified.
 * 	hook should be :
 * 		gboolean hook ( GtkWidget *button,
 * 				gpointer data )
 *
 * \param data An optional pointer to pass to hooks.
 * \param default_func The function to call to change the value in memory (function must be func ( number, gboolean ) ) or NULL
 * \param number_for_func a gint wich we be used to call default_func (will be saved as g_object_set_data with "number_for_func")
 *
 * \return a new GtkCheckButton
 * */
GtkWidget *gsb_autofunc_checkbutton_new ( const gchar *label,
					  gboolean value,
					  GCallback hook,
					  gpointer data,
					  GCallback default_func,
					  gint number_for_func )
{
    GtkWidget *button;

    /* first, create and set the button */
    button = gtk_check_button_new_with_mnemonic (label);
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON (button),
				   value );

    /* set the default func :
     * the func will be send to gsb_autofunc_checkbutton_changed by the data,
     * the number_for_func will be set as data for object */
    g_object_set_data ( G_OBJECT (button),
			"number_for_func", GINT_TO_POINTER (number_for_func));
    if (default_func)
	g_object_set_data ( G_OBJECT (button), "changed", 
			    (gpointer) g_signal_connect_after (GTK_OBJECT(button), "toggled",
							       ((GCallback) gsb_autofunc_checkbutton_changed), default_func ));
    if ( hook )
	g_object_set_data ( G_OBJECT (button), "changed-hook", 
			    (gpointer) g_signal_connect_after (GTK_OBJECT(button), "toggled",
							       ((GCallback) hook), data ));
    return button;
}


/** 
 * set the value in a gsb_editable_checkbutton
 * a value is in 2 parts :
 * 	a boolean, so value TRUE or FALSE
 * 	a number, wich is used when there is a change in that button (see gsb_autofunc_checkbutton_new)
 *
 * \param button
 * \param value a gboolean
 * \param number_for_func the number to give to the called function when something is changed
 *
 * \return
 */
void gsb_autofunc_checkbutton_set_value ( GtkWidget *button,
					  gboolean value,
					  gint number_for_func )
{
    /* Block everything */
    if ( g_object_get_data (G_OBJECT (button), "changed") > 0 )
	g_signal_handler_block ( GTK_OBJECT(button),
				 (gulong) g_object_get_data (G_OBJECT (button), 
							     "changed"));
    if ( g_object_get_data (G_OBJECT (button), "changed-hook") > 0 )
	g_signal_handler_block ( GTK_OBJECT(button),
				 (gulong) g_object_get_data (G_OBJECT (button), 
							     "changed-hook"));

    /* Fill in value */
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON (button),
				   value );
    g_object_set_data ( G_OBJECT (button),
			"number_for_func", GINT_TO_POINTER (number_for_func));

    /* Unblock everything */
    if ( g_object_get_data (G_OBJECT (button), "changed") > 0 )
	g_signal_handler_unblock ( GTK_OBJECT(button),
				   (gulong) g_object_get_data (G_OBJECT (button), 
							       "changed"));
    if ( g_object_get_data (G_OBJECT (button), "changed-hook") > 0 )
	g_signal_handler_unblock ( GTK_OBJECT(button),
				   (gulong) g_object_get_data (G_OBJECT (button), 
							       "changed-hook"));
}



/**
 * called when something change in an button of a gsb_editable_checkbutton
 * by gsb_autofunc_entry_new
 *
 * \param button The reference GtkCheckButton
 * \param default_func the function to call to change the value in memory
 *
 * \return FALSE
 */
static gboolean gsb_autofunc_checkbutton_changed ( GtkWidget *button,
						   gboolean default_func (gint, gboolean))
{
    gint number_for_func;

    /* just to be sure... */
    if (!default_func || !button)
	return FALSE;

    number_for_func = GPOINTER_TO_INT ( g_object_get_data (G_OBJECT (button), "number_for_func"));
    default_func ( number_for_func,
		   gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)));

    /* Mark file as modified */
    modification_fichier ( TRUE );

    return FALSE;
}



/**
 * Creates a new radio buttons group with two choices.  Toggling will
 * for each change, will call the corresponding given function : gsb_data_... ( number, gboolean )
 * ie the target function must be :
 * 	(default_func) ( gint number_for_func,
 * 			 gboolean yes/no )
 * ex : gsb_data_category_set_type ( category_number, type )
 *
 * \param choice1 First choice label
 * \param choice2 Second choice label
 * \param value A pointer to an integer that will be set to 0 or 1
 *        according to buttons toggles. (choice 2 selected means boolean = TRUE in the function)
 * \param hook An optional hook to run at each toggle
 * \param data optional data to send to hook
 * \param default_func The function to call to change the value in memory (function must be func ( number, gboolean ) ) or NULL
 * \param number_for_func a gint wich we be used to call default_func (will be saved as g_object_set_data with "number_for_func")
 * 
 * \return a vbox with the 2 radiobuttons
 *
 */
GtkWidget *gsb_autofunc_radiobutton_new ( const gchar *choice1,
					  const gchar *choice2,
					  gboolean value,
					  GCallback hook,
					  gpointer data,
					  GCallback default_func,
					  gint number_for_func )
{
    GtkWidget *button1, *button2, *vbox;

    vbox = gtk_vbox_new ( FALSE, 6 );

    button1 = gtk_radio_button_new_with_mnemonic ( NULL, choice1 );
    gtk_box_pack_start (GTK_BOX(vbox), button1, FALSE, FALSE, 0 );
    button2 = gtk_radio_button_new_with_mnemonic ( gtk_radio_button_group (GTK_RADIO_BUTTON(button1)), 
						   choice2 );
    gtk_box_pack_start (GTK_BOX(vbox), button2, FALSE, FALSE, 0 );

    if (value)
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button2 ), TRUE );
    else
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button1 ), TRUE );

    g_object_set_data (G_OBJECT (button2),
		       "number_for_func", GINT_TO_POINTER (number_for_func));
    g_signal_connect ( G_OBJECT ( button2 ), "toggled",
		       G_CALLBACK (gsb_autofunc_checkbutton_changed), default_func );

    if (hook)
	g_signal_connect ( G_OBJECT ( button2 ), "toggled",
			   G_CALLBACK (hook), data );

    return vbox;
}


