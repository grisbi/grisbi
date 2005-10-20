/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	     2005 Benjamin Drieu (bdrieu@april.org)	      */
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
 * \file gsb_assistant.c
 * Various routines that implement the assistant "GnomeDruid-like" window.
 */


#include "include.h"

/*START_INCLUDE*/
#include "gsb_assistant.h"
#include "gsb_file_config.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean gsb_assistant_change_page ( GtkNotebook * notebook, GtkNotebookPage * npage, 
				     gint page, gpointer assistant );
/*END_STATIC*/

/*START_EXTERN*/
extern     gchar * buffer ;
extern GtkWidget *window;
/*END_EXTERN*/



/**
 *
 *
 */
GtkWidget * gsb_assistant_new ( gchar * title, gchar * explanation,
				gchar * image_filename )
{
    GtkWidget * assistant, *notebook, *vbox, *hbox, *label, *image, *view, *eb;
    GtkWidget * button_cancel, * button_prev, * button_next;
    GtkTextIter iter;
    GtkStyle * style;
    GtkTextBuffer * buffer;
    
    assistant = gtk_dialog_new_with_buttons ( title,
					   GTK_WINDOW(NULL),
					   GTK_DIALOG_NO_SEPARATOR,
					   NULL );

    button_cancel = gtk_dialog_add_button ( assistant, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL );
    g_object_set_data ( assistant, "button_cancel", button_cancel );

    button_prev = gtk_dialog_add_button ( assistant, GTK_STOCK_GO_BACK, GTK_RESPONSE_NO );
    g_object_set_data ( assistant, "button_prev", button_prev );
    gtk_widget_set_sensitive ( button_prev, FALSE );

    button_next = gtk_dialog_add_button ( assistant, GTK_STOCK_GO_FORWARD, GTK_RESPONSE_YES );
    g_object_set_data ( assistant, "button_next", button_next );

    eb = gtk_event_box_new ();
    style = gtk_widget_get_style ( eb );
    gtk_widget_modify_bg ( eb, 0, &(style -> bg[GTK_STATE_ACTIVE]) );

    hbox = gtk_hbox_new ( FALSE, 12 );
    gtk_container_add ( GTK_CONTAINER(eb), hbox );
    gtk_container_set_border_width ( GTK_CONTAINER(hbox), 12 );

    label = gtk_label_new ( "" );
    gtk_label_set_markup ( label, g_strconcat ( "<b><span size=\"x-large\">",
						title, "</span></b>", NULL ) );
    gtk_box_pack_start ( GTK_BOX(hbox), label, TRUE, TRUE, 0 );

    image = gtk_image_new_from_file ( g_strconcat ( PIXMAPS_DIR, C_DIRECTORY_SEPARATOR,
						    image_filename, NULL) );
    gtk_box_pack_start ( GTK_BOX(hbox), image, FALSE, FALSE, 0 );

    gtk_box_pack_start ( GTK_DIALOG(assistant) -> vbox, eb, 
			 FALSE, FALSE, 0 );

    notebook = gtk_notebook_new ();
    gtk_notebook_set_show_tabs ( notebook, FALSE );
    gtk_notebook_set_show_border ( notebook, FALSE );
    gtk_box_pack_start ( GTK_DIALOG(assistant) -> vbox, notebook, TRUE, TRUE, 0 );

    view = gtk_text_view_new ();
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (view), GTK_WRAP_WORD);
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
    gtk_text_buffer_set_text (buffer, g_strconcat ( "\n", explanation, "\n", NULL ), -1);
    gtk_text_view_set_editable ( GTK_TEXT_VIEW(view), FALSE );
    gtk_text_view_set_cursor_visible ( GTK_TEXT_VIEW(view), FALSE );
    gtk_text_view_set_left_margin ( GTK_TEXT_VIEW(view), 12 );
    gtk_text_view_set_right_margin ( GTK_TEXT_VIEW(view), 12 );

    gtk_notebook_append_page ( notebook, view, gtk_label_new("") );

    g_signal_connect ( notebook, "switch-page",
		       G_CALLBACK ( gsb_assistant_change_page ), assistant );

    g_object_set_data ( assistant, "next0", 1 );
    g_object_set_data ( assistant, "notebook", notebook );
    g_object_set_data ( assistant, "title", title );

    return assistant;
}



void gsb_assistant_add_page ( GtkWidget * assistant, GtkWidget * widget, gint position,
			      gint prev, gint next, GCallback enter_callback )
{
    GtkWidget * notebook;

    notebook = g_object_get_data ( assistant, "notebook" );
    gtk_notebook_append_page ( notebook, widget, gtk_label_new("") );

    g_object_set_data ( assistant, g_strdup_printf ( "prev%d", position ), prev );
    g_object_set_data ( assistant, g_strdup_printf ( "next%d", position ), next );
    g_object_set_data ( assistant, g_strdup_printf ( "enter%d", position ), enter_callback );

    gtk_widget_show_all ( widget );
}



GtkResponseType gsb_assistant_run ( GtkWidget * assistant )
{
    gint state = 0;
    GtkWidget * notebook, * button_prev, * button_next;

    button_prev = g_object_get_data ( assistant, "button_prev" );
    button_next = g_object_get_data ( assistant, "button_next" );

    gtk_widget_show_all ( assistant );

    notebook = g_object_get_data ( assistant, "notebook" );

    while ( TRUE )
    {
	gint current = gtk_notebook_get_current_page ( notebook );
	gint result, prev, next;

	gtk_window_set_title ( assistant, 
			       g_strdup_printf ( "%s (%d of %d)", 
						 g_object_get_data ( assistant, "title" ),
						 current + 1,
						 gtk_notebook_get_n_pages ( notebook ) ) );

	result = gtk_dialog_run ( assistant );
	prev = g_object_get_data ( assistant, g_strdup_printf ( "prev%d", current ) );
	next = g_object_get_data ( assistant, g_strdup_printf ( "next%d", current ) );

	switch ( result )
	{
	    case GTK_RESPONSE_YES:
		printf ("??> NTH pages %d\n", gtk_notebook_get_n_pages ( g_object_get_data ( assistant, "notebook" ) ) );


		gtk_widget_set_sensitive ( button_prev, TRUE );
		if ( gtk_notebook_get_n_pages ( notebook ) == ( next + 1 ) )
		{
		    gtk_widget_destroy ( button_next );
		    button_next = gtk_dialog_add_button ( assistant, GTK_STOCK_CLOSE, 
							  GTK_RESPONSE_APPLY );
		}

		gtk_notebook_set_page ( notebook, next );
		break;

	    case GTK_RESPONSE_NO:
		if ( gtk_notebook_get_n_pages ( notebook ) == next )
		{
		    gtk_widget_destroy ( button_next );
		    button_next = gtk_dialog_add_button ( assistant, GTK_STOCK_GO_FORWARD, 
							  GTK_RESPONSE_YES );
		}

		gtk_widget_set_sensitive ( button_next, TRUE );
		if ( current == 0 )
		{
		    gtk_widget_set_sensitive ( button_prev, FALSE );
		}

		gtk_notebook_set_page ( notebook, prev );
		break;

	    case GTK_RESPONSE_APPLY:
		return GTK_RESPONSE_APPLY;

	    default:
	    case GTK_RESPONSE_CANCEL:
		return GTK_RESPONSE_CANCEL;
	}
    }

    return GTK_RESPONSE_CANCEL;
}



/**
 * TODO
 *
 */
gboolean gsb_assistant_change_page ( GtkNotebook * notebook, GtkNotebookPage * npage, 
				     gint page, gpointer assistant )
{
    gboolean (* callback) ( GtkWidget * );
    gpointer padding[32];	/* Don't touch, looks like we have a
				 * buffer overflow problem. */

    callback = (gboolean *) g_object_get_data ( assistant, g_strdup_printf ( "enter%d", page ) );

    if ( callback )
    {
	return callback ( assistant );
    }

    return FALSE;
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
