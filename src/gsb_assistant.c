/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2005-2006 Benjamin Drieu (bdrieu@april.org)	      */
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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_assistant.h"
#include "structures.h"
#include "utils.h"
#include "utils_str.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_STATIC*/
static gboolean gsb_assistant_change_page ( GtkNotebook * notebook, GtkNotebookPage * npage,
				     gint page, gpointer assistant );
/*END_STATIC*/

/*START_EXTERN*/
extern GtkWidget *window;
/*END_EXTERN*/



/**
 * Create and initialize a new grisbi assistant.  It is basically
 * composed of a GtkDialog with a notebook that is switched from page
 * to page when user click on dialog buttons.
 *
 * \param title			Title of the assistant.
 * \param explanation		Short text to display in the first
 *				page of the assistant.
 * \param image_filename	Icon to display in the title. (if NULL, use grisbi.png, default logo)
 * \param enter_callback	A callback to connect to the "switch-page" callback when go to the first page
 *				of the Grisbi assistant notebook. (the callback should
 *				be : gboolean callback ( GtkWidget *assistant, gint new_page ) )
 *
 */
GtkWidget * gsb_assistant_new ( const gchar * title, const gchar * explanation,
                        gchar * image_filename,
                        GCallback enter_callback )
{
    GtkWidget * assistant, *notebook, *hbox, *label, *image, *view, *eb;
    GtkWidget * button_cancel, * button_prev, * button_next;
    GtkWidget *button_select;
    GtkStyle * style;
    GtkTextBuffer * buffer;
    gchar *tmpstr;
    gint width = 140;

    assistant = gtk_dialog_new_with_buttons ( title,
                        GTK_WINDOW ( window ),
                        GTK_DIALOG_MODAL | GTK_DIALOG_NO_SEPARATOR,
                        NULL );

    gtk_window_set_default_size ( GTK_WINDOW ( assistant ), 800, 500 );
    gtk_window_set_position ( GTK_WINDOW ( assistant ), GTK_WIN_POS_CENTER_ON_PARENT );
    gtk_window_set_resizable ( GTK_WINDOW ( assistant ), TRUE );
    g_object_set_data ( G_OBJECT ( window ), "assistant", assistant );

    button_select = gtk_toggle_button_new_with_label ( _("Select all") );
    gtk_widget_set_size_request ( button_select, width, -1 );
    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( assistant )->action_area ),
                        button_select, FALSE, FALSE, 0 );
    g_object_set_data ( G_OBJECT(assistant), "button_select", button_select );

    button_cancel = gtk_dialog_add_button ( GTK_DIALOG(assistant),
                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL );
    gtk_widget_set_size_request ( button_cancel, width, -1 );
    g_object_set_data ( G_OBJECT(assistant), "button_cancel", button_cancel );

    button_prev = gtk_dialog_add_button ( GTK_DIALOG(assistant),
                        GTK_STOCK_GO_BACK, GTK_RESPONSE_NO );
    gtk_widget_set_size_request ( button_prev, width, -1 );
    g_object_set_data ( G_OBJECT(assistant), "button_prev", button_prev );
    gtk_widget_set_sensitive ( button_prev, FALSE );

    button_next = gtk_dialog_add_button ( GTK_DIALOG(assistant),
                        GTK_STOCK_GO_FORWARD, GTK_RESPONSE_YES );
    gtk_widget_set_size_request ( button_next, width, -1 );
    g_object_set_data ( G_OBJECT(assistant), "button_next", button_next );

    eb = gtk_event_box_new ();
    style = gtk_widget_get_style ( eb );
    gtk_widget_modify_bg ( eb, 0, &(style -> bg[GTK_STATE_ACTIVE]) );

    hbox = gtk_hbox_new ( FALSE, 12 );
    gtk_container_add ( GTK_CONTAINER(eb), hbox );
    gtk_container_set_border_width ( GTK_CONTAINER(hbox), 12 );

    label = gtk_label_new ( NULL );
    tmpstr = g_markup_printf_escaped (
                        "<b><span size=\"x-large\">%s</span></b>", title );
    gtk_label_set_markup ( GTK_LABEL(label), tmpstr );
    g_free ( tmpstr );
    gtk_box_pack_start ( GTK_BOX(hbox), label, TRUE, TRUE, 0 );

    if (!image_filename)
        image_filename = "grisbi.png";

    tmpstr = g_build_filename ( GRISBI_PIXMAPS_DIR, image_filename, NULL);
    image = gtk_image_new_from_file ( tmpstr );
    g_free ( tmpstr );
    gtk_box_pack_start ( GTK_BOX(hbox), image, FALSE, FALSE, 0 );

    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG(assistant) -> vbox ), eb, FALSE, FALSE, 0 );

    notebook = gtk_notebook_new ();
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK(notebook), FALSE );
    gtk_notebook_set_show_border ( GTK_NOTEBOOK(notebook), FALSE );
    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG(assistant) -> vbox ), notebook, TRUE, TRUE, 0 );

    view = gtk_text_view_new ();
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (view), GTK_WRAP_WORD);
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
    tmpstr = g_strconcat ( "\n", explanation, "\n", NULL );
    gtk_text_buffer_set_text (buffer, tmpstr, -1);
    g_free ( tmpstr );
    gtk_text_view_set_editable ( GTK_TEXT_VIEW(view), FALSE );
    gtk_text_view_set_cursor_visible ( GTK_TEXT_VIEW(view), FALSE );
    gtk_text_view_set_left_margin ( GTK_TEXT_VIEW(view), 12 );
    gtk_text_view_set_right_margin ( GTK_TEXT_VIEW(view), 12 );

    /* Create some handy fonts. */
    gtk_text_buffer_create_tag ( buffer, "bold", "weight", PANGO_WEIGHT_BOLD, NULL );
    gtk_text_buffer_create_tag ( buffer, "x-large", "scale", PANGO_SCALE_X_LARGE, NULL );
    gtk_text_buffer_create_tag ( buffer, "indented", "left-margin", 24, NULL );

    gtk_notebook_append_page ( GTK_NOTEBOOK(notebook), view, gtk_label_new(NULL) );

    g_signal_connect_after ( notebook, "switch-page",
                        G_CALLBACK ( gsb_assistant_change_page ), assistant );

    gsb_assistant_set_next ( assistant, 0, 1 );
    g_object_set_data ( G_OBJECT(assistant), "notebook", notebook );
    /* need to set (gchar *) because title is const gchar, the good way should be my_strdup,
     * but we don't touch/free title later so it should be ok... */
    g_object_set_data ( G_OBJECT(assistant), "title", (gchar *) title );
    g_object_set_data ( G_OBJECT(assistant), "enter0", enter_callback );

    return assistant;
}



/**
 * Add a page to the Grisbi assistant.
 *
 * \param assistant		Grisbi assistant to add a page to.
 * \param widget		Widget containing the new page to insert.
 * \param position		Number of the page to insert.  Page 0 is
 *				reserved to the explanation label.
 * \param prev			Page to display when the "Previous" button is
 *				clicked.
 * \param next			Page to display when the "Next" button is clicked.
 * \param enter_callback	A callback to connect to the "switch-page" callback
 *				of the Grisbi assistant notebook. (the callback should
 *				be : gboolean callback ( GtkWidget *assistant, gint new_page ) )
 */
void gsb_assistant_add_page ( GtkWidget * assistant, GtkWidget * widget, gint position,
			      gint prev, gint next, GCallback enter_callback )
{
    GtkWidget * notebook;
    gchar *tmpstr;

    notebook = g_object_get_data ( G_OBJECT(assistant), "notebook" );
    gtk_notebook_insert_page ( GTK_NOTEBOOK(notebook), widget, gtk_label_new(NULL), position );

    gsb_assistant_set_prev ( assistant, position, prev );
    gsb_assistant_set_next ( assistant, position, next );
    tmpstr = g_strdup_printf ( "enter%d", position );
    g_object_set_data ( G_OBJECT(assistant), tmpstr,
			enter_callback );
    g_free (tmpstr);

    gtk_widget_show_all ( widget );
}



/**
 * Run the Grisbi assistant.  This will pop up a new dialog.
 *
 * \param assistant	Grisbi assistant to run.
 *
 * \return		Outcome of the Grisbi assistant.  Can be
 *			GTK_RESPONSE_APPLY for success and
 *			GTK_RESPONSE_CANCEL for failure
 *			(user canceled or closed dialog).
 */
GtkResponseType gsb_assistant_run ( GtkWidget * assistant )
{
    GtkWidget * notebook, * button_prev;
    GtkWidget *button_select;

    button_prev = g_object_get_data ( G_OBJECT(assistant), "button_prev" );
    button_select = g_object_get_data ( G_OBJECT ( assistant ), "button_select" );

    gtk_widget_show_all ( assistant );
    gtk_widget_hide ( button_select );

    notebook = g_object_get_data ( G_OBJECT(assistant), "notebook" );
    gtk_notebook_set_current_page ( GTK_NOTEBOOK (notebook),
			    0 );
    gtk_widget_grab_focus (GTK_WIDGET (g_object_get_data (G_OBJECT (assistant),
							  "button_next")));

    while ( TRUE )
    {
	gint current = gtk_notebook_get_current_page ( GTK_NOTEBOOK(notebook) );
	gint result, prev, next;

        gchar* tmpstr = g_strdup_printf ( _("%s (%d of %d)"),
						 (gchar *) g_object_get_data ( G_OBJECT(assistant),
									       "title" ),
						 current + 1,
						 gtk_notebook_get_n_pages ( GTK_NOTEBOOK(notebook) ) );
	gtk_window_set_title ( GTK_WINDOW(assistant), tmpstr);
	g_free ( tmpstr );

	result = gtk_dialog_run ( GTK_DIALOG(assistant) );

	tmpstr = g_strdup_printf ( "prev%d", current );
	prev = GPOINTER_TO_INT( g_object_get_data ( G_OBJECT(assistant), tmpstr ));
	g_free ( tmpstr );

    tmpstr = g_strdup_printf ( "next%d", current );
	next = GPOINTER_TO_INT( g_object_get_data ( G_OBJECT(assistant), tmpstr));
	g_free ( tmpstr );

	 switch ( result )
	 {
	     case GTK_RESPONSE_YES:
		gtk_widget_set_sensitive ( button_prev, TRUE );
		if ( gtk_notebook_get_n_pages ( GTK_NOTEBOOK(notebook) ) == ( next + 1 ) )
		{
		    gsb_assistant_change_button_next ( assistant, GTK_STOCK_CLOSE,
						       GTK_RESPONSE_APPLY );
		    gsb_assistant_sensitive_button_prev ( assistant,
							  FALSE );
		}

		gtk_notebook_set_current_page ( GTK_NOTEBOOK(notebook), next );
		break;

	    case GTK_RESPONSE_NO:
		if ( next == -1 )
		{
		    gsb_assistant_change_button_next ( assistant, GTK_STOCK_GO_FORWARD,
						       GTK_RESPONSE_YES );
		}

		gsb_assistant_sensitive_button_next ( assistant, TRUE );

		if ( prev == 0 )
		{
		    gtk_widget_set_sensitive ( button_prev, FALSE );
		}

		gtk_notebook_set_current_page ( GTK_NOTEBOOK(notebook), prev );
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
 * Call a user-defined optional callback when user change page.  Note
 * that it is called AFTER stock callbacks for various reasons.
 *
 * \param notebook	This Grisbi assistant notebook.
 * \param npage		Not used.
 * \param page		Page selected.
 * \param assistant	Grisbi assistant containing the notebook.
 *
 * \return		Result from user-defined callback or FALSE if
 *			no callback defined.
 */
gboolean gsb_assistant_change_page ( GtkNotebook * notebook, GtkNotebookPage * npage,
				     gint page, gpointer assistant )
{
    typedef gboolean ( * gsb_assistant_callback ) ( GtkWidget *, gint );
    gsb_assistant_callback callback;
/*     gpointer padding[32];	/\* Don't touch, looks like we have a */
/* 				 * buffer overflow problem. *\/ */

    gchar* tmpstr = g_strdup_printf ( "enter%d", page );
    callback = ( gsb_assistant_callback ) g_object_get_data ( G_OBJECT (assistant), tmpstr );
    g_free ( tmpstr );

    if ( callback )
    {
	return callback ( assistant, page );
    }

    return FALSE;
}



/**
 * Change the previous page associated to a notebook page.  This can
 * be used to insert a page between two others.
 *
 * \param assistant	A pointer to a Grisbi assistant.
 * \param page		Page to change previous link.
 * \param prev		Number of the new previous page.
 */
void gsb_assistant_set_prev ( GtkWidget * assistant, gint page, gint prev )
{
    gchar * string;

    string = g_strdup_printf ( "prev%d", page );
    g_object_set_data ( G_OBJECT(assistant), string, GINT_TO_POINTER (prev));
    g_free ( string );
}



/**
 * Change the next page associated to a notebook page.  This can
 * be used to insert a page between two others.
 *
 * \param assistant	A pointer to a Grisbi assistant.
 * \param page		Page to change next link.
 * \param next		Number of the new next page.
 */
void gsb_assistant_set_next ( GtkWidget * assistant, gint page, gint next )
{
    gchar * string;

    string = g_strdup_printf ( "next%d", page );
    g_object_set_data ( G_OBJECT(assistant), string, GINT_TO_POINTER (next));
    g_free ( string );
}




/**
 *
 *
 */
gboolean gsb_assistant_sensitive_button_next ( GtkWidget * assistant, gboolean state )
{
    gtk_widget_set_sensitive ( g_object_get_data ( G_OBJECT (assistant), "button_next" ),
			       state );

    return FALSE;
}

/**
 *
 *
 */
gboolean gsb_assistant_sensitive_button_prev ( GtkWidget * assistant, gboolean state )
{
    gtk_widget_set_sensitive ( g_object_get_data ( G_OBJECT (assistant), "button_prev" ),
			       state );

    return FALSE;
}

/**
 * force the assistant to go to the next page
 *
 * \param assistant
 *
 * \return
 * */
void gsb_assistant_next_page ( GtkWidget *assistant )
{
    GtkWidget * button_next;

    button_next = g_object_get_data ( G_OBJECT (assistant), "button_next" );
    gtk_button_clicked ( GTK_BUTTON (button_next));
}


/**
 *
 *
 */
void gsb_assistant_change_button_next ( GtkWidget * assistant, gchar * title,
					GtkResponseType response )
{
    GtkWidget * button_next;

    button_next = g_object_get_data ( G_OBJECT (assistant), "button_next" );
    gtk_widget_destroy ( button_next );
    button_next = gtk_dialog_add_button ( GTK_DIALOG (assistant), title, response );
    g_object_set_data ( G_OBJECT (assistant), "button_next", button_next );
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
