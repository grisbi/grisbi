/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2007 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2007 Benjamin Drieu (bdrieu@april.org)	      */
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
 * \file gsb_assistant_archive.c
 * we find here the complete assistant to archive some transactions
 */


#include "include.h"

/*START_INCLUDE*/
#include "gsb_assistant_archive.h"
#include "./etats_support.h"
#include "./gsb_assistant.h"
#include "./gsb_calendar_entry.h"
#include "./gsb_data_archive.h"
#include "./gsb_data_transaction.h"
#include "./utils_dates.h"
#include "./gsb_fyear.h"
#include "./gsb_report.h"
#include "./traitement_variables.h"
#include "./utils_str.h"
#include "./etats_calculs.h"
#include "./utils.h"
#include "./structures.h"
#include "./include.h"
/*END_INCLUDE*/

/*START_STATIC*/
static  GtkWidget *gsb_assistant_archive_page_archive_name ( GtkWidget *assistant );
static  GtkWidget *gsb_assistant_archive_page_menu ( GtkWidget *assistant );
static  GtkWidget *gsb_assistant_archive_page_success ( void );
static  gboolean gsb_assistant_archive_switch_to_archive_name ( GtkWidget *assistant,
							       gint new_page );
static  gboolean gsb_assistant_archive_switch_to_intro ( GtkWidget *assistant,
							gint new_page );
static  gboolean gsb_assistant_archive_switch_to_menu ( GtkWidget *assistant,
						       gint new_page );
static  gboolean gsb_assistant_archive_switch_to_succes ( GtkWidget *assistant,
							 gint new_page );
static  gboolean gsb_assistant_archive_update_labels ( GtkWidget *assistant );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

enum archive_assistant_page
{
    ARCHIVE_ASSISTANT_INTRO= 0,
    ARCHIVE_ASSISTANT_MENU,
    ARCHIVE_ASSISTANT_ARCHIVE_NAME,
    ARCHIVE_ASSISTANT_SUCCESS
};

static GtkWidget *initial_date;
static GtkWidget *final_date;
static GtkWidget *financial_year_button;
static GtkWidget *report_button;
static GtkWidget *name_entry;
static GtkWidget *label_archived;

static GtkWidget *vbox_congratulation;
static GtkWidget *vbox_failed;
static GtkWidget *congratulations_view;

static GSList *list_transaction_to_archive = NULL;


/**
 * this function is called to launch the assistant to make archives
 *
 * \param
 *
 * \return a GtkResponseType containing the return value at the end of the assistant
 * */
GtkResponseType gsb_assistant_archive_run ( void )
{
    GtkResponseType return_value;
    GtkWidget *assistant;

    /* create the assistant */
    assistant = gsb_assistant_new ( _("Archive transactions"),
				    _("This assistant will guide you through the process of archiving transactions to increase the speed of grisbi.\n"
				      "By default, Grisbi does not export any archive into separate files, it just mark transactions as archted and do not use them.\n"
				      "You can still possibly use them be able to export them intp a separate archive file if necessary.\n"),
				    "archive.png",
				    G_CALLBACK (gsb_assistant_archive_switch_to_intro));
    gsb_assistant_add_page ( assistant,
			     gsb_assistant_archive_page_menu (assistant),
			     ARCHIVE_ASSISTANT_MENU,
			     ARCHIVE_ASSISTANT_INTRO,
			     ARCHIVE_ASSISTANT_ARCHIVE_NAME,
			     G_CALLBACK (gsb_assistant_archive_switch_to_menu));
    gsb_assistant_add_page ( assistant,
			     gsb_assistant_archive_page_archive_name (assistant),
			     ARCHIVE_ASSISTANT_ARCHIVE_NAME,
			     ARCHIVE_ASSISTANT_MENU,
			     ARCHIVE_ASSISTANT_SUCCESS,
			     G_CALLBACK (gsb_assistant_archive_switch_to_archive_name));
    gsb_assistant_add_page ( assistant,
			     gsb_assistant_archive_page_success (),
			     ARCHIVE_ASSISTANT_SUCCESS,
			     ARCHIVE_ASSISTANT_MENU,
			     0,
			     G_CALLBACK (gsb_assistant_archive_switch_to_succes));
    return_value = gsb_assistant_run (assistant);
    gtk_widget_destroy (assistant);

    /* the fyear combobox in grisbi are used with the "Automatic" line,
     * so show it again */
    gsb_fyear_set_automatic (TRUE);
    return return_value;
}



/**
 * create the page 2 of the assistant
 * show the menu to choose between archive by date, financial year or budget
 *
 * \param assistant the GtkWidget assistant
 *
 * \return a GtkWidget containing the page
 * */
static GtkWidget *gsb_assistant_archive_page_menu ( GtkWidget *assistant )
{
    GtkWidget *page;
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *button;
    GtkWidget *hbox;

    page = gtk_hbox_new (FALSE, 15);
    gtk_container_set_border_width ( GTK_CONTAINER (page),
				     10 );

    vbox = gtk_vbox_new (FALSE, 5);
    gtk_box_pack_start ( GTK_BOX (page),
			 vbox,
			 FALSE, FALSE, 0 );

    /* set up the menu */
    label = gtk_label_new (_("Please choose the way to make your archive :"));
    gtk_misc_set_alignment ( GTK_MISC (label),
			     0, 0.5 );
    gtk_box_pack_start ( GTK_BOX (vbox),
			 label,
			 FALSE, FALSE, 0 );

    /* archive by date */
    button = gtk_radio_button_new_with_label ( NULL, _("Archive by date"));
    g_signal_connect_object ( G_OBJECT (button),
			       "toggled",
			       G_CALLBACK (gsb_assistant_archive_update_labels),
			       G_OBJECT (assistant),
			       G_CONNECT_AFTER | G_CONNECT_SWAPPED);
    gtk_box_pack_start ( GTK_BOX (vbox),
			 button,
			 FALSE, FALSE, 0 );

    label = gtk_label_new (_("Grisbi will archive all the transactions between the initial and final date."));
    gtk_misc_set_alignment ( GTK_MISC (label),
			     0, 0.5 );
    gtk_box_pack_start ( GTK_BOX (vbox),
			 label,
			 FALSE, FALSE, 0 );

    hbox = gtk_hbox_new ( FALSE, 6 );
    g_signal_connect ( G_OBJECT (button),
		       "toggled",
		       G_CALLBACK  (sens_desensitive_pointeur),
		       G_OBJECT (hbox));
    gtk_box_pack_start ( GTK_BOX (vbox),
			 hbox,
			 FALSE, FALSE, 0 );

    label = gtk_label_new ( COLON ( _("Initial date" ) ) );
    gtk_box_pack_start ( GTK_BOX (hbox),
			 label,
 			 FALSE, FALSE, 0 );
    initial_date = gsb_calendar_entry_new (FALSE);
    g_signal_connect_object ( G_OBJECT (initial_date),
			      "changed",
			      G_CALLBACK (gsb_assistant_archive_update_labels),
			      G_OBJECT (assistant),
			      G_CONNECT_AFTER | G_CONNECT_SWAPPED);
    gtk_box_pack_start ( GTK_BOX (hbox),
			 initial_date,
			 FALSE, FALSE, 0 );

    label = gtk_label_new ( COLON ( _("Final date")) );
    gtk_box_pack_start ( GTK_BOX (hbox),
			 label,
			 FALSE, FALSE, 0 );
    final_date = gsb_calendar_entry_new (FALSE);
    g_signal_connect_object ( G_OBJECT (final_date),
			      "changed",
			      G_CALLBACK (gsb_assistant_archive_update_labels),
			      G_OBJECT (assistant),
			      G_CONNECT_AFTER | G_CONNECT_SWAPPED);
    gtk_box_pack_start ( GTK_BOX (hbox),
			 final_date,
			 FALSE, FALSE, 0 );

    /* archive by financial year */
    button = gtk_radio_button_new_with_label ( gtk_radio_button_get_group (GTK_RADIO_BUTTON (button)),
					       _("Archive by financial year"));
    g_signal_connect_object ( G_OBJECT (button),
			       "toggled",
			       G_CALLBACK (gsb_assistant_archive_update_labels),
			       G_OBJECT (assistant),
			       G_CONNECT_AFTER | G_CONNECT_SWAPPED);
    gtk_box_pack_start ( GTK_BOX (vbox),
			 button,
			 FALSE, FALSE, 0 );

    label = gtk_label_new (_("Grisbi will archive all the transactions belonging to the financial year." ));
    gtk_misc_set_alignment ( GTK_MISC (label),
			     0, 0.5 );
    gtk_box_pack_start ( GTK_BOX (vbox),
			 label,
			 FALSE, FALSE, 0 );

    hbox = gtk_hbox_new ( FALSE, 10 );
    g_signal_connect ( G_OBJECT (button),
		       "toggled",
		       G_CALLBACK  (sens_desensitive_pointeur),
		       G_OBJECT (hbox));
    gtk_box_pack_start ( GTK_BOX (vbox),
			 hbox,
			 FALSE, FALSE, 0 );

    label = gtk_label_new (_("Financial year :"));
    gtk_box_pack_start ( GTK_BOX (hbox),
			 label,
			 FALSE, FALSE, 0 );
    financial_year_button = gsb_fyear_make_combobox (FALSE);
    g_signal_connect_object ( G_OBJECT (financial_year_button),
			      "changed",
			      G_CALLBACK (gsb_assistant_archive_update_labels),
			      G_OBJECT (assistant),
			      G_CONNECT_AFTER | G_CONNECT_SWAPPED);
    gtk_box_pack_start ( GTK_BOX (hbox),
			 financial_year_button,
			 FALSE, FALSE, 0 );
    gtk_widget_set_sensitive ( hbox, FALSE );

    /* archive by budget */
    button = gtk_radio_button_new_with_label ( gtk_radio_button_get_group (GTK_RADIO_BUTTON (button)),
					       _("Archive by report"));
    g_signal_connect_object ( G_OBJECT (button),
			       "toggled",
			       G_CALLBACK (gsb_assistant_archive_update_labels),
			       G_OBJECT (assistant),
			       G_CONNECT_AFTER | G_CONNECT_SWAPPED);
    gtk_box_pack_start ( GTK_BOX (vbox),
			 button,
			 FALSE, FALSE, 0 );

    label = gtk_label_new (_("Grisbi will archive the transactions selected by a report." ));
    gtk_misc_set_alignment ( GTK_MISC (label),
			     0, 0.5 );
    gtk_box_pack_start ( GTK_BOX (vbox),
			 label,
			 FALSE, FALSE, 0 );

    hbox = gtk_hbox_new ( FALSE, 10 );
    g_signal_connect ( G_OBJECT (button),
		       "toggled",
		       G_CALLBACK  (sens_desensitive_pointeur),
		       G_OBJECT (hbox));
    gtk_box_pack_start ( GTK_BOX (vbox),
			 hbox,
			 FALSE, FALSE, 0 );

    label = gtk_label_new ( COLON ( _("Report") ) );
    gtk_box_pack_start ( GTK_BOX (hbox),
			 label,
			 FALSE, FALSE, 0 );
    report_button = gsb_report_make_combobox ();
    g_signal_connect_object ( G_OBJECT (report_button),
			      "changed",
			      G_CALLBACK (gsb_assistant_archive_update_labels),
			      G_OBJECT (assistant),
			      G_CONNECT_AFTER | G_CONNECT_SWAPPED);
    gtk_box_pack_start ( GTK_BOX (hbox),
			 report_button,
			 FALSE, FALSE, 0 );
    gtk_widget_set_sensitive ( hbox, FALSE );

    label_archived = gtk_label_new (NULL);
    gtk_box_pack_start ( GTK_BOX (vbox),
			 label_archived,
			 FALSE, FALSE, 0 );

    gsb_assistant_sensitive_button_next ( assistant, FALSE );

    gtk_widget_show_all (page);
    return page;
}



/**
 * Create page 3 of the assistant.
 *
 * \param assistant the GtkWidget assistant
 *
 * \return a GtkWidget containing the page
 * */
static GtkWidget *gsb_assistant_archive_page_archive_name ( GtkWidget *assistant )
{
    GtkWidget *page;
    GtkWidget *vbox;
    GtkWidget *label;

    page = gtk_hbox_new (FALSE, 15);
    gtk_container_set_border_width ( GTK_CONTAINER (page),
				     10 );

    vbox = gtk_vbox_new (FALSE, 5);
    gtk_box_pack_start ( GTK_BOX (page),
			 vbox,
			 FALSE, FALSE, 0 );

    /* set up the menu */
    label = gtk_label_new ( COLON(_("Please choose a name for archive") ) );
    gtk_misc_set_alignment ( GTK_MISC (label),
			     0, 0.5 );
    gtk_box_pack_start ( GTK_BOX (vbox),
			 label,
			 FALSE, FALSE, 0 );

    name_entry = gtk_entry_new ();
    g_signal_connect_object ( G_OBJECT (name_entry),
			      "changed",
			      G_CALLBACK (gsb_assistant_archive_update_labels),
			      G_OBJECT (assistant),
			      G_CONNECT_AFTER | G_CONNECT_SWAPPED);
    gtk_box_pack_start ( GTK_BOX (vbox),
			 name_entry,
			 FALSE, FALSE, 0 );

    gsb_assistant_sensitive_button_next ( assistant, FALSE );

    return page;
}



/**
 * create the page showed after the click on the "Create" button
 * this is a success page or a failed page
 *
 * \param
 *
 * \return a GtkWidget, the page to the assistant
 * */
static GtkWidget *gsb_assistant_archive_page_success ( void )
{
    GtkWidget * page, * label;
    GtkTextBuffer * buffer;
    GtkTextIter iter;

    page = gtk_vbox_new ( FALSE, 0 );
    gtk_container_set_border_width ( GTK_CONTAINER (page),
				     10 );

    /* we create 2 vbox, one for congratulation, one for failed,
     * only one will be showed */
    vbox_congratulation = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start ( GTK_BOX (page),
			 vbox_congratulation,
			 FALSE, FALSE, 0 );

    congratulations_view = gtk_text_view_new ();
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (congratulations_view), GTK_WRAP_WORD);
    gtk_text_view_set_editable ( GTK_TEXT_VIEW(congratulations_view), FALSE );
    gtk_text_view_set_cursor_visible ( GTK_TEXT_VIEW(congratulations_view), FALSE );
    gtk_text_view_set_left_margin ( GTK_TEXT_VIEW(congratulations_view), 12 );
    gtk_text_view_set_right_margin ( GTK_TEXT_VIEW(congratulations_view), 12 );

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (congratulations_view));
    gtk_text_buffer_create_tag ( buffer, "bold", "weight", PANGO_WEIGHT_BOLD, NULL);  
    gtk_text_buffer_create_tag ( buffer, "x-large", "scale", PANGO_SCALE_X_LARGE, NULL);
    gtk_text_buffer_create_tag ( buffer, "indented", "left-margin", 24, NULL);

    gtk_text_buffer_get_iter_at_offset (buffer, &iter, 1);
    gtk_text_buffer_insert_with_tags_by_name (buffer, &iter,
					      _("Congratulations !"), -1,
					      "x-large", "bold", NULL);
    gtk_text_buffer_insert ( buffer, &iter, "\n\n", -1 );
    
    gtk_box_pack_start ( GTK_BOX (vbox_congratulation),
			 congratulations_view,
			 FALSE, FALSE,
			 0 );
    gtk_text_buffer_get_end_iter ( buffer, &iter );
    gtk_text_buffer_create_mark ( buffer, "status", &iter, TRUE );

    gtk_text_buffer_insert ( buffer, &iter, 
			     _("In case grisbi is still too slow after you created archives,"
			       "remember that you can configure it no to load "
			       "the marked transactions (R) at startup, to increase speed.\n\n"
			       "Press the 'Previous' button to create another archive of 'Close' "
			       "to exit this assistant."),
			     -1 );

    /* make the failed box */
    vbox_failed = gtk_vbox_new (FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX (page),
			 vbox_failed,
			 FALSE, FALSE, 0 );

    label = gtk_label_new (_("An error occurred while creating the archive...\n"
			     "Please try to find the problem and contact the grisbi team to correct it.\n\n"
			     "Please press the Previous or Close button."));
    gtk_misc_set_alignment ( GTK_MISC (label),
			     0, 0.5 );
    gtk_box_pack_start ( GTK_BOX (vbox_failed),
			 label,
			 FALSE, FALSE,
			 0 );

    gtk_widget_show_all (page);
    gtk_widget_hide (vbox_failed);
    return page;
}


/**
 * called when switch page between to the initial page
 *
 * \param assistant
 * \param new_page
 *
 * \return FALSE
 * */
static gboolean gsb_assistant_archive_switch_to_intro ( GtkWidget *assistant,
							gint new_page )
{
    /* enter into the intro page */
    gsb_assistant_change_button_next ( assistant,
				       GTK_STOCK_GO_FORWARD, GTK_RESPONSE_YES );
    gsb_assistant_sensitive_button_next ( assistant,
					  TRUE );
    return FALSE;
}



/**
 * called when switch page to the menu page
 *
 * \param assistant
 * \param new_page
 *
 * \return FALSE
 * */
static gboolean gsb_assistant_archive_switch_to_menu ( GtkWidget *assistant,
						       gint new_page )
{
    /* enter into the menu page */
    gtk_label_set_text ( GTK_LABEL (label_archived), NULL );
    gsb_assistant_change_button_next ( assistant,
				       GTK_STOCK_APPLY, GTK_RESPONSE_YES );
    gsb_assistant_archive_update_labels ( assistant );
    gsb_assistant_sensitive_button_next ( assistant, FALSE );

    return FALSE;
}



/**
 * Switch to the "name archive" page.
 *
 * \param assistant
 * \param new_page
 *
 * \return FALSE
 * */
static gboolean gsb_assistant_archive_switch_to_archive_name ( GtkWidget *assistant,
							       gint new_page )
{
    gchar * string = NULL;

    gsb_assistant_change_button_next ( assistant,
				       GTK_STOCK_APPLY, GTK_RESPONSE_YES );

    if ( GTK_WIDGET_IS_SENSITIVE (initial_date) )
    {
	gchar * sdate, * fdate;
	sdate = gsb_format_gdate ( gsb_calendar_entry_get_date (initial_date) );
	fdate = gsb_format_gdate ( gsb_calendar_entry_get_date (final_date) );
	string = g_strdup_printf ( _("Archive from %s to %s"), sdate, fdate );
	g_free ( sdate );
	g_free ( fdate );
    }

    if ( string )
    {
	gtk_entry_set_text ( GTK_ENTRY ( name_entry ), string );
	g_free ( string );
    }

    gsb_assistant_sensitive_button_next ( assistant, TRUE );

    return FALSE;
}



/**
 * called when switch page to the success page
 * will create the archived transactions
 *
 * \param assistant
 * \param new_page
 *
 * \return FALSE
 * */
static gboolean gsb_assistant_archive_switch_to_succes ( GtkWidget *assistant,
							 gint new_page )
{
    /* if we come here, we are sure that :
     * there is a own name to the archive
     * there is some transasctions to be archived
     * all the necessary stuff is filled */
    GSList *tmp_list;
    gint archive_number;
    gchar *string;
    GtkTextBuffer * buffer;
    GtkTextIter     iter;
printf ( "ça passe\n");
    if (!list_transaction_to_archive)
    {
	/* should not happen */
	gtk_widget_hide (vbox_congratulation);
	gtk_widget_show (vbox_failed);
	return FALSE;
    }

    /* first, create the archive */
    archive_number = gsb_data_archive_new (gtk_entry_get_text (GTK_ENTRY (name_entry)));
    printf ( "%d\n", archive_number);
    if (!archive_number)
    {
	gtk_widget_hide (vbox_congratulation);
	gtk_widget_show (vbox_failed);
	return FALSE;
    }

    /* fill the archive */
    if (GTK_WIDGET_IS_SENSITIVE (initial_date))
    {
	GDate *init_gdate;
	GDate *final_gdate;
	init_gdate = gsb_calendar_entry_get_date (initial_date);
	final_gdate = gsb_calendar_entry_get_date (final_date);

	gsb_data_archive_set_begining_date ( archive_number,
					     init_gdate );
	gsb_data_archive_set_end_date ( archive_number,
					final_gdate );
	g_date_free (init_gdate);
	g_date_free (final_gdate);
    }
    if (GTK_WIDGET_IS_SENSITIVE (financial_year_button))
	gsb_data_archive_set_fyear ( archive_number,
				     gsb_fyear_get_fyear_from_combobox (financial_year_button,NULL));

    if (GTK_WIDGET_IS_SENSITIVE (report_button))
    {
	gint report_number;

	report_number = gsb_report_get_report_from_combobox (report_button);

	gsb_data_archive_set_report_title ( archive_number,
					    etats_titre (report_number));
    }

    /* mark the transactions as archived */
    tmp_list = list_transaction_to_archive;
    while (tmp_list)
    {
	gint transaction_number;

	transaction_number = gsb_data_transaction_get_transaction_number (tmp_list -> data);
	gsb_data_transaction_set_archive_number ( transaction_number,
						  archive_number );
	tmp_list = tmp_list -> next;
    }

    /* set the message */
    string = g_strdup_printf ( _("Archive '%s' was successfully created and %d transactions out of %d were archived.\n"),
			       gsb_data_archive_get_name (archive_number),
			       g_slist_length (list_transaction_to_archive),
			       g_slist_length (list_transaction_to_archive) +
			       g_slist_length (gsb_data_transaction_get_transactions_list ()));

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (congratulations_view));
    gtk_text_buffer_get_iter_at_mark ( buffer, &iter, 
				       gtk_text_buffer_get_mark ( buffer, "status" ) );
    gtk_text_buffer_insert ( buffer, &iter, string, strlen(string) );

    g_free (string);

    gtk_widget_hide (vbox_failed);
    gtk_widget_show (vbox_congratulation);

    /* free the transactions list to archive */
    g_slist_free (list_transaction_to_archive);
    list_transaction_to_archive = NULL;

    /* erase all the previous entries */
    gtk_entry_set_text ( GTK_ENTRY (name_entry),
			 "" );
    gtk_entry_set_text ( GTK_ENTRY (initial_date),
			 "" );
    gtk_entry_set_text ( GTK_ENTRY (final_date),
			 "" );

    /* propose to go in one or another way */
    gsb_assistant_sensitive_button_prev ( assistant,
					  TRUE );
    gsb_assistant_sensitive_button_next ( assistant,
					  TRUE );

    modification_fichier (TRUE);
    return FALSE;
}



/**
 * called for each event to update the labels of number of transactions to archive
 * and check if all is ok, to sensitive the "Create" button
 *
 * \param assistant
 *
 * \return FALSE
 * */
static gboolean gsb_assistant_archive_update_labels ( GtkWidget *assistant )
{
    gchar *string;
    GSList *tmp_list;
    GtkWidget * notebook;

    notebook = g_object_get_data ( G_OBJECT(assistant), "notebook" );
    
    if (gtk_notebook_get_current_page (GTK_NOTEBOOK(notebook)) == ARCHIVE_ASSISTANT_MENU
	&&
	list_transaction_to_archive)
    {
	g_slist_free (list_transaction_to_archive);
	list_transaction_to_archive = NULL;
    }

    if ( gtk_notebook_get_current_page (GTK_NOTEBOOK(notebook)) == ARCHIVE_ASSISTANT_MENU &&
	 GTK_WIDGET_IS_SENSITIVE (initial_date))
    {
	/* ok for now the choice is on initial/final date */
	GDate *init_gdate;
	GDate *final_gdate;
	init_gdate = gsb_calendar_entry_get_date (initial_date);
	final_gdate = gsb_calendar_entry_get_date (final_date);
	string = NULL;

	if (!strlen ( gtk_entry_get_text (GTK_ENTRY (initial_date))))
	    string = my_strdup (_("<span foreground=\"red\">Please fill the initial date</span>"));
	if (!string
	    &&
	    !strlen ( gtk_entry_get_text (GTK_ENTRY (final_date))))
	    string = my_strdup (_("<span foreground=\"red\">Please fill the final date</span>"));
	if ( !string
	     &&
	     init_gdate
	     &&
	     final_gdate
	     &&
	     g_date_compare (init_gdate, final_gdate) > 0 )
	    string = my_strdup (_("<span foreground=\"red\">The initial date is after the final date</span>"));
	if ( !string
	     &&
	     !init_gdate)
	    string = my_strdup (_("<span foreground=\"red\">The initial date is not valid.</span>"));
	if ( !string
	     &&
	     !final_gdate)
	    string = my_strdup (_("<span foreground=\"red\">The final date is not valid</span>"));
	if ( !string
	     &&
	     gsb_data_archive_get_from_date (init_gdate))
	    string = my_strdup (_("<span foreground=\"red\">The initial date belongs already to an archive.</span>"));
	if ( !string
	     &&
	     gsb_data_archive_get_from_date (final_gdate))
	    string = my_strdup (_("<span foreground=\"red\">The final date belongs already to an archive.</span>"));

	if (string)
	{
	    gtk_label_set_markup ( GTK_LABEL (label_archived),
				   string );
	    g_free (string);
	    gsb_assistant_sensitive_button_next ( assistant, FALSE );
	    return FALSE;
	}

	/* the dates are ok */
	tmp_list = gsb_data_transaction_get_transactions_list ();
	while (tmp_list)
	{
	    gint transaction_number;

	    transaction_number = gsb_data_transaction_get_transaction_number (tmp_list -> data);

	    if ( g_date_compare ( init_gdate,
				  gsb_data_transaction_get_date (transaction_number)) <= 0
		 &&
		 g_date_compare ( final_gdate,
				  gsb_data_transaction_get_date (transaction_number)) >= 0 )
		/* the transaction is into the dates, we append its address to the list to reconcile */
		list_transaction_to_archive = g_slist_append ( list_transaction_to_archive,
								 tmp_list -> data );
	    tmp_list = tmp_list -> next;
	}
    }

    if ( gtk_notebook_get_current_page (GTK_NOTEBOOK(notebook)) == ARCHIVE_ASSISTANT_MENU &&
	 GTK_WIDGET_IS_SENSITIVE (financial_year_button))
    {
	/* ok for now the choice is on fyear */
	gint fyear_number;
	string = NULL;

	fyear_number = gsb_fyear_get_fyear_from_combobox (financial_year_button,NULL);

	if (!fyear_number)
	    string = my_strdup (_("<span foreground=\"red\">Please choose a financial year</span>"));

	if ( !string
	     &&
	     gsb_data_archive_get_from_fyear (fyear_number))
	    string = my_strdup (_("<span foreground=\"red\">There is already an archive for that financial year.</span>"));

	if (string)
	{
	    gtk_label_set_markup ( GTK_LABEL (label_archived),
				   string );
	    g_free (string);
	    gsb_assistant_sensitive_button_next ( assistant, FALSE );
	    return FALSE;
	}

	/* the fyear is ok */
	tmp_list = gsb_data_transaction_get_transactions_list ();
	while (tmp_list)
	{
	    gint transaction_number;

	    transaction_number = gsb_data_transaction_get_transaction_number (tmp_list -> data);

	    if (gsb_data_transaction_get_financial_year_number (transaction_number) == fyear_number)
		/* the transaction belongs to the fyear, we append its address to the list to reconcile */
		list_transaction_to_archive = g_slist_append ( list_transaction_to_archive,
								 tmp_list -> data );
	    tmp_list = tmp_list -> next;
	}
    }

    if ( gtk_notebook_get_current_page (GTK_NOTEBOOK(notebook)) == ARCHIVE_ASSISTANT_MENU &&
	 GTK_WIDGET_IS_SENSITIVE (report_button))
    {
	/* ok for now the choice is on report */
	gint report_number;
	string = NULL;

	report_number = gsb_report_get_report_from_combobox (report_button);

	if (!report_number)
	    string = my_strdup (_("<span foreground=\"red\">Please choose a report</span>"));

	if (string)
	{
	    gtk_label_set_markup ( GTK_LABEL (label_archived),
				   string );
	    g_free (string);
	    gsb_assistant_sensitive_button_next ( assistant, FALSE );
	    return FALSE;
	}

	/* the report is ok */
	list_transaction_to_archive = recupere_opes_etat (report_number);
    }

    /* update the labels */
    if (gtk_notebook_get_current_page (GTK_NOTEBOOK(notebook)) == ARCHIVE_ASSISTANT_MENU
	&&
	list_transaction_to_archive)
    {
	/* there is some transactions to archive */
	string = g_strdup_printf (_("%d transactions out of %d will be archived."),
				  g_slist_length (list_transaction_to_archive),
				  g_slist_length (gsb_data_transaction_get_transactions_list () ) );
	gtk_label_set_text ( GTK_LABEL (label_archived),
			     string);

	if ( gtk_notebook_get_current_page (GTK_NOTEBOOK(notebook)) != 
	     ARCHIVE_ASSISTANT_ARCHIVE_NAME )
	{
	    gsb_assistant_sensitive_button_next ( assistant, TRUE );
	}

	g_free (string);
    }

    if ( gtk_notebook_get_current_page (GTK_NOTEBOOK(notebook)) == 
	 ARCHIVE_ASSISTANT_ARCHIVE_NAME )
    {
	if ( strlen ( gtk_entry_get_text ( GTK_ENTRY ( name_entry ) ) ) )
	{
	    gsb_assistant_sensitive_button_next ( assistant, TRUE );
	}
	else
	{
	    gsb_assistant_sensitive_button_next ( assistant, FALSE );
	}
    }
    
    return FALSE;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
