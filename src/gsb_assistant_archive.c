/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2007 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*                      2008-2009 Pierre Biava (grisbi@pierre.biava.name)     */
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

/**
 * \file gsb_assistant_archive.c
 * we find here the complete assistant to archive some transactions
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_assistant_archive.h"
#include "etats_support.h"
#include "gsb_assistant.h"
#include "gsb_calendar_entry.h"
#include "gsb_data_archive.h"
#include "gsb_data_archive_store.h"
#include "gsb_data_fyear.h"
#include "gsb_data_report.h"
#include "gsb_data_transaction.h"
#include "utils_dates.h"
#include "gsb_fyear.h"
#include "navigation.h"
#include "gsb_report.h"
#include "gsb_transactions_list.h"
#include "dialog.h"
#include "traitement_variables.h"
#include "utils_str.h"
#include "etats_calculs.h"
#include "utils.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void gsb_assistant_archive_add_children_to_list ( gint transaction_number );
static void gsb_assistant_archive_add_contra_transaction_to_list ( gint transaction_number );
static void gsb_assistant_archive_add_transaction_to_list ( gpointer transaction_pointer );
static GtkWidget *gsb_assistant_archive_page_archive_name ( GtkWidget *assistant );
static GtkWidget *gsb_assistant_archive_page_menu ( GtkWidget *assistant );
static GtkWidget *gsb_assistant_archive_page_success ( void );
static gboolean gsb_assistant_archive_switch_to_archive_name ( GtkWidget *assistant,
                        gint new_page );
static gboolean gsb_assistant_archive_switch_to_intro ( GtkWidget *assistant,
                        gint new_page );
static gboolean gsb_assistant_archive_switch_to_menu ( GtkWidget *assistant,
                        gint new_page );
static gboolean gsb_assistant_archive_switch_to_success ( GtkWidget *assistant,
                        gint new_page );
static gboolean gsb_assistant_archive_update_labels ( GtkWidget *assistant );
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

static GtkWidget *initial_date = NULL;
static GtkWidget *final_date = NULL;
static GtkWidget *financial_year_button = NULL;
static GtkWidget *report_button = NULL;
static GtkWidget *name_entry = NULL;
static GtkWidget *label_archived = NULL;

static GtkWidget *vbox_congratulation = NULL;
static GtkWidget *vbox_failed = NULL;
static GtkWidget *congratulations_view = NULL;
static GtkWidget *failed_view = NULL;

static GSList *list_transaction_to_archive = NULL;


/**
 * cannot send param by menu, so come here to launch the assistant
 *
 * \param
 *
 * \return the GtkResponseType of the assistant
 * */
GtkResponseType gsb_assistant_archive_run_by_menu (void)
{
    return gsb_assistant_archive_run (FALSE);
}


/**
 * this function is called to launch the assistant to make archives
 *
 * \param origin : FALSE by menu, TRUE automatically after a file opening
 *
 * \return a GtkResponseType containing the return value at the end of the assistant
 * */
GtkResponseType gsb_assistant_archive_run ( gboolean origin )
{
    GtkResponseType return_value;
    GtkWidget *assistant;
    gchar *tmpstr;

    if (origin)
	/* come from check while opening file */
	tmpstr = g_strdup_printf ( _("There are a lot of transactions in your file (%d) and it is advised not to keep more than about %d transactions unarchived.\n"
				     "To increase speed, you should archive them "
				     "(the current limit and the opening check-up can be changed in the Preferences)\n\n"
				     "This assistant will guide you through the process of archiving transactions "
				     "By default, Grisbi does not export any archive into separate files, "
				     "it just mark transactions as archted and do not use them.\n\n"
				     "You can still export them into a separate archive file if necessary.\n\n" 
				     "Press Cancel if you don't want make an archive now\n"),
				   g_slist_length (gsb_data_transaction_get_transactions_list ()),
				   conf.max_non_archived_transactions_for_check );
    else
	/* come by menu action */
	tmpstr = my_strdup (_("This assistant will guide you through the process of archiving transactions "
			      "to increase the speed of grisbi.\n\n"
			      "By default, Grisbi does not export any archive into separate files, "
			      "it just mark transactions as archted and do not use them.\n\n"
			      "You can still export them into a separate archive file if necessary.\n"));

    /* create the assistant */
    assistant = gsb_assistant_new ( _("Archive transactions"),
				    tmpstr,
				    "archive.png",
				    G_CALLBACK (gsb_assistant_archive_switch_to_intro));
    g_free (tmpstr);

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
			     G_CALLBACK (gsb_assistant_archive_switch_to_success));
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
    gtk_container_set_border_width ( GTK_CONTAINER (page), 10 );

    vbox = gtk_vbox_new (FALSE, 5);
    gtk_box_pack_start ( GTK_BOX (page), vbox, FALSE, FALSE, 0 );

    /* set up the menu */
    label = gtk_label_new (_("Please select a way to select transactions for this archive"));
    gtk_misc_set_alignment ( GTK_MISC (label), 0, 0.5 );
    gtk_box_pack_start ( GTK_BOX (vbox), label,  FALSE, FALSE, 0 );

    /* archive by date */
    button = gtk_radio_button_new_with_label ( NULL, _("Archive by date"));
    g_signal_connect_object ( G_OBJECT (button),
                        "toggled",
                        G_CALLBACK (gsb_assistant_archive_update_labels),
                        G_OBJECT (assistant),
                        G_CONNECT_AFTER | G_CONNECT_SWAPPED);
    gtk_box_pack_start ( GTK_BOX (vbox), button, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("Grisbi will archive all transactions between initial and "
                        "final dates.") );
    gtk_misc_set_alignment ( GTK_MISC (label), 0, 0.5 );
    gtk_misc_set_padding ( GTK_MISC (label), 24, 0 );
    gtk_box_pack_start ( GTK_BOX (vbox), label, FALSE, FALSE, 0 );

    hbox = gtk_hbox_new ( FALSE, 6 );
    g_signal_connect ( G_OBJECT (button),
                        "toggled",
                        G_CALLBACK  (sens_desensitive_pointeur),
                        G_OBJECT (hbox));
    gtk_box_pack_start ( GTK_BOX (vbox), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("Initial date: " ) );
    gtk_misc_set_padding ( GTK_MISC (label), 24, 0 );
    gtk_box_pack_start ( GTK_BOX (hbox), label, FALSE, FALSE, 0 );

    initial_date = gsb_calendar_entry_new (FALSE);
    g_signal_connect_object ( G_OBJECT (initial_date),
                        "changed",
                        G_CALLBACK (gsb_assistant_archive_update_labels),
                        G_OBJECT (assistant),
                        G_CONNECT_AFTER | G_CONNECT_SWAPPED);
    gtk_box_pack_start ( GTK_BOX (hbox), initial_date, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("Final date: ") );
    gtk_misc_set_padding ( GTK_MISC (label), 24, 0 );
    gtk_box_pack_start ( GTK_BOX (hbox), label, FALSE, FALSE, 0 );

    final_date = gsb_calendar_entry_new (FALSE);
    g_signal_connect_object ( G_OBJECT (final_date),
                        "changed",
                        G_CALLBACK (gsb_assistant_archive_update_labels),
                        G_OBJECT (assistant),
                        G_CONNECT_AFTER | G_CONNECT_SWAPPED);
    gtk_box_pack_start ( GTK_BOX (hbox), final_date, FALSE, FALSE, 0 );

    /* archive by financial year */
    button = gtk_radio_button_new_with_label ( 
                        gtk_radio_button_get_group ( GTK_RADIO_BUTTON ( button ) ),
                        _("Archive by financial year") );
    g_signal_connect_object ( G_OBJECT (button),
                        "toggled",
                        G_CALLBACK (gsb_assistant_archive_update_labels),
                        G_OBJECT (assistant),
                        G_CONNECT_AFTER | G_CONNECT_SWAPPED);
    gtk_box_pack_start ( GTK_BOX (vbox), button, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("Grisbi will archive all transactions belonging to a "
                        "financial year." ) );
    gtk_misc_set_padding ( GTK_MISC (label), 24, 0 );
    gtk_misc_set_alignment ( GTK_MISC (label), 0, 0.5 );
    gtk_box_pack_start ( GTK_BOX (vbox), label, FALSE, FALSE, 0 );

    hbox = gtk_hbox_new ( FALSE, 10 );
    g_signal_connect ( G_OBJECT (button),
                        "toggled",
                        G_CALLBACK  ( sens_desensitive_pointeur ),
                        G_OBJECT (hbox));
    gtk_box_pack_start ( GTK_BOX (vbox), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("Financial year: ") );
    gtk_misc_set_padding ( GTK_MISC (label), 24, 0 );
    gtk_box_pack_start ( GTK_BOX (hbox), label, FALSE, FALSE, 0 );

    financial_year_button = gsb_fyear_make_combobox (FALSE);
    g_signal_connect_object ( G_OBJECT (financial_year_button),
                        "changed",
                        G_CALLBACK (gsb_assistant_archive_update_labels),
                        G_OBJECT (assistant),
                        G_CONNECT_AFTER | G_CONNECT_SWAPPED);
    gtk_box_pack_start ( GTK_BOX (hbox), financial_year_button, FALSE, FALSE, 0 );
    gtk_widget_set_sensitive ( hbox, FALSE );

    /* archive by budget */
    button = gtk_radio_button_new_with_label ( gtk_radio_button_get_group (
                        GTK_RADIO_BUTTON ( button ) ),
                        _("Archive by report"));
    g_signal_connect_object ( G_OBJECT (button),
                        "toggled",
                        G_CALLBACK (gsb_assistant_archive_update_labels),
                        G_OBJECT (assistant),
                        G_CONNECT_AFTER | G_CONNECT_SWAPPED);
    gtk_box_pack_start ( GTK_BOX (vbox), button, FALSE, FALSE, 0 );

    label = gtk_label_new (_("Grisbi will archive transactions selected by a report." ));
    gtk_misc_set_padding ( GTK_MISC (label), 24, 0 );
    gtk_misc_set_alignment ( GTK_MISC (label), 0, 0.5 );
    gtk_box_pack_start ( GTK_BOX (vbox), label, FALSE, FALSE, 0 );

    hbox = gtk_hbox_new ( FALSE, 10 );
    g_signal_connect ( G_OBJECT (button),
                        "toggled",
                        G_CALLBACK  (sens_desensitive_pointeur),
                        G_OBJECT (hbox));
    gtk_box_pack_start ( GTK_BOX (vbox), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("Report: ") );
    gtk_misc_set_padding ( GTK_MISC (label), 24, 0 );
    gtk_box_pack_start ( GTK_BOX (hbox), label, FALSE, FALSE, 0 );
    report_button = gsb_report_make_combobox ();
    g_signal_connect_object ( G_OBJECT (report_button),
                        "changed",
                        G_CALLBACK (gsb_assistant_archive_update_labels),
                        G_OBJECT (assistant),
                        G_CONNECT_AFTER | G_CONNECT_SWAPPED);
    gtk_box_pack_start ( GTK_BOX (hbox), report_button, FALSE, FALSE, 0 );
    gtk_widget_set_sensitive ( hbox, FALSE );

    label_archived = gtk_label_new (NULL);
    gtk_misc_set_alignment ( GTK_MISC (label_archived), 0, 0 );
    gtk_box_pack_start ( GTK_BOX (vbox), label_archived, FALSE, FALSE, 0 );

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
    label = gtk_label_new ( _("Please choose a name for archive: ") );
    gtk_misc_set_alignment ( GTK_MISC (label),
			     0, 0.5 );
    gtk_box_pack_start ( GTK_BOX (vbox),
			 label,
			 FALSE, FALSE, 0 );

    name_entry = gtk_entry_new ();
    gtk_widget_set_size_request ( name_entry, 400, -1 );
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
    GtkWidget * page;
    GtkTextBuffer * buffer;
    GtkTextIter iter;

    page = gtk_vbox_new ( FALSE, 0 );
    gtk_container_set_border_width ( GTK_CONTAINER (page),
				     0 );

    /* we create 2 vbox, one for congratulation, one for failed,
     * only one will be showed */
    vbox_congratulation = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start ( GTK_BOX (page),
			 vbox_congratulation,
			 TRUE, TRUE, 0 );

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
    gtk_text_buffer_insert ( buffer, &iter, "\n", -1 );
    gtk_text_buffer_insert_with_tags_by_name (buffer, &iter,
					      _("Congratulations !"), -1,
					      "x-large", "bold", NULL);
    gtk_text_buffer_insert ( buffer, &iter, "\n\n", -1 );
    
    gtk_box_pack_start ( GTK_BOX (vbox_congratulation),
			 congratulations_view,
			 TRUE, TRUE,
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
			 TRUE, TRUE, 0 );

    failed_view = gtk_text_view_new ();
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (failed_view), GTK_WRAP_WORD);
    gtk_text_view_set_editable ( GTK_TEXT_VIEW(failed_view), FALSE );
    gtk_text_view_set_cursor_visible ( GTK_TEXT_VIEW(failed_view), FALSE );
    gtk_text_view_set_left_margin ( GTK_TEXT_VIEW(failed_view), 12 );
    gtk_text_view_set_right_margin ( GTK_TEXT_VIEW(failed_view), 12 );

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (failed_view));
    gtk_text_buffer_create_tag ( buffer, "bold", "weight", PANGO_WEIGHT_BOLD, NULL);  
    gtk_text_buffer_create_tag ( buffer, "x-large", "scale", PANGO_SCALE_X_LARGE, NULL);
    gtk_text_buffer_create_tag ( buffer, "indented", "left-margin", 24, NULL);

    gtk_text_buffer_get_iter_at_offset (buffer, &iter, 1);
    gtk_text_buffer_insert ( buffer, &iter, "\n", -1 );
    gtk_text_buffer_insert_with_tags_by_name (buffer, &iter,
					      _("Failed !"), -1,
					      "x-large", "bold", NULL);
    gtk_text_buffer_insert ( buffer, &iter, "\n\n", -1 );
    
    gtk_box_pack_start ( GTK_BOX (vbox_failed),
			 failed_view,
			 TRUE, TRUE,
			 0 );
    gtk_text_buffer_get_end_iter ( buffer, &iter );
    gtk_text_buffer_create_mark ( buffer, "status", &iter, TRUE );

    gtk_text_buffer_insert ( buffer, &iter, 
			     _("An error occurred while creating the archive...\n"
			       "Most probably, you are trying to create an empty archive "
			       "that grisbi cowardly refused to create.\n\n"
			       "Press the Previous button to try again or cancel this process."),
			     -1 );

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
    GSList *tmp_list;
    const GDate *date = NULL;

    /* enter into the menu page */
    gtk_label_set_text ( GTK_LABEL (label_archived), NULL );
    gsb_assistant_change_button_next ( assistant,
                        GTK_STOCK_GO_FORWARD, GTK_RESPONSE_YES );
    gsb_assistant_archive_update_labels ( assistant );

    /* set the initial date to the first transaction in grisbi */
    tmp_list = gsb_data_transaction_get_transactions_list ();
    while (tmp_list)
    {
	gint transaction_number;

	transaction_number = gsb_data_transaction_get_transaction_number (tmp_list -> data);
	if (date)
	{
	    if (g_date_compare (date, gsb_data_transaction_get_date (transaction_number)) > 0)
		date = gsb_data_transaction_get_date (transaction_number);
	}
	else
	    date = gsb_data_transaction_get_date (transaction_number);
	tmp_list = tmp_list -> next;
    }
    if (date)
    {
	gtk_entry_set_text ( GTK_ENTRY (initial_date),
			     gsb_format_gdate (date));
	gtk_editable_select_region ( GTK_EDITABLE (initial_date),
				 0, -1);
    }
    gtk_widget_grab_focus (initial_date);

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

        sdate = gsb_format_gdate ( gsb_calendar_entry_get_date (initial_date));
        fdate = gsb_format_gdate (gsb_calendar_entry_get_date (final_date));
        string = g_strdup_printf ( _("Archive from %s to %s"), sdate, fdate );
        g_free ( sdate );
        g_free ( fdate );
    }
    else if ( GTK_WIDGET_IS_SENSITIVE (financial_year_button) )
    {
        gint fyear;

        fyear = gsb_fyear_get_fyear_from_combobox ( financial_year_button, NULL );
        string = g_strdup_printf ( _("Archive of financial year %s"), 
                            gsb_data_fyear_get_name ( fyear ) );
    }
    else if ( GTK_WIDGET_IS_SENSITIVE (report_button) )
    {
        gint report_number;
        report_number = gsb_report_get_report_from_combobox (report_button);
        string = g_strdup_printf ( _("Archive of report %s"), 
                            gsb_data_report_get_report_name ( report_number ) );
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
static gboolean gsb_assistant_archive_switch_to_success ( GtkWidget *assistant,
                        gint new_page )
{
    /* if we come here, we are sure that :
     * there is a own name to the archive
     * there is some transactions to be archived
     * all the necessary stuff is filled */
    GSList *tmp_list;
    gint archive_number;
    gchar *string;
    GtkTextBuffer * buffer;
    GtkTextIter     iter;
    gint account_nb;

    /* This would typically happen if user selected a time period
     * with no transactions related. */
    if (!list_transaction_to_archive)
    {
        gtk_widget_hide (vbox_congratulation);
        gtk_widget_show (vbox_failed);
        gsb_assistant_sensitive_button_prev ( assistant, TRUE );
        gsb_assistant_sensitive_button_next ( assistant, FALSE );
        return FALSE;
    }

    /* first, create the archive */
    archive_number = gsb_data_archive_new (gtk_entry_get_text (GTK_ENTRY (name_entry)));
    if (!archive_number)
    {
        gtk_widget_hide (vbox_congratulation);
        gtk_widget_show (vbox_failed);
        return FALSE;
    }

    gsb_assistant_change_button_next ( assistant,
                        GTK_STOCK_CLOSE, GTK_RESPONSE_APPLY );

    /* fill the archive */
    if (GTK_WIDGET_IS_SENSITIVE (initial_date))
    {
        GDate *init_gdate;
        GDate *final_gdate;

        init_gdate = gsb_calendar_entry_get_date (initial_date);
        final_gdate = gsb_calendar_entry_get_date (final_date);

        gsb_data_archive_set_beginning_date ( archive_number, init_gdate );
        gsb_data_archive_set_end_date ( archive_number, final_gdate );
        g_date_free (init_gdate);
        g_date_free (final_gdate);
    }
    else if (GTK_WIDGET_IS_SENSITIVE (financial_year_button))
        gsb_data_archive_set_fyear ( archive_number,
                        gsb_fyear_get_fyear_from_combobox ( financial_year_button,NULL ) );
    else if (GTK_WIDGET_IS_SENSITIVE (report_button) )
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
        gsb_data_transaction_set_archive_number ( transaction_number, archive_number );

        /* Delete transaction in the tree_view */
        if ( gsb_data_transaction_get_mother_transaction_number ( transaction_number ) == 0 )
            gsb_transactions_list_delete_transaction_from_tree_view ( transaction_number );
        tmp_list = tmp_list -> next;
    }

    /* create again the list of archives by account */
    gsb_data_archive_store_init_variables ();
    gsb_data_archive_store_create_list ( );
    gsb_transactions_list_fill_archive_store ( );

    /* set the message */
    guint archived_transaction_count = g_slist_length (list_transaction_to_archive);
    string = g_strdup_printf ( _("Archive '%s' was successfully created and %d transactions "
                                 "out of %d were archived.\n\n"),
                        gsb_data_archive_get_name (archive_number),
                        archived_transaction_count,
                        archived_transaction_count +
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
    g_signal_handlers_block_by_func ( G_OBJECT ( name_entry ),
 					G_CALLBACK ( gsb_assistant_archive_update_labels ),
 					assistant );
    g_signal_handlers_block_by_func ( G_OBJECT ( initial_date ),
 					G_CALLBACK ( gsb_assistant_archive_update_labels ),
 					assistant );
    g_signal_handlers_block_by_func ( G_OBJECT ( final_date ),
 					G_CALLBACK ( gsb_assistant_archive_update_labels ),
 					assistant );
    gtk_entry_set_text ( GTK_ENTRY (name_entry), "" );
    gtk_entry_set_text ( GTK_ENTRY (initial_date), "" );
    gtk_entry_set_text ( GTK_ENTRY (final_date), "" );

    g_signal_handlers_unblock_by_func ( G_OBJECT ( name_entry ),
 					G_CALLBACK ( gsb_assistant_archive_update_labels ),
 					assistant );
    g_signal_handlers_unblock_by_func ( G_OBJECT ( initial_date ),
 					G_CALLBACK ( gsb_assistant_archive_update_labels ),
 					assistant );
    g_signal_handlers_unblock_by_func ( G_OBJECT ( final_date ),
 					G_CALLBACK ( gsb_assistant_archive_update_labels ),
 					assistant );

    /* propose to go in one or another way */
    gsb_assistant_sensitive_button_prev ( assistant, TRUE );
    gsb_assistant_sensitive_button_next ( assistant, TRUE );

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );

    /* update the display of tree_view if necessary */
    if ( ( account_nb = gsb_gui_navigation_get_current_account ( ) ) != -1 )
        gsb_transactions_list_update_tree_view ( account_nb, TRUE );

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
    GSList *report_transactions_list;

    notebook = g_object_get_data ( G_OBJECT(assistant), "notebook" );
    
    /* erase the last list of transactions to archive */
    if (gtk_notebook_get_current_page (GTK_NOTEBOOK(notebook)) == ARCHIVE_ASSISTANT_MENU
     &&
     list_transaction_to_archive )
    {
        g_slist_free (list_transaction_to_archive);
        list_transaction_to_archive = NULL;
        gsb_assistant_sensitive_button_next ( assistant, TRUE );
    }

    /* ok for now the choice is on initial/final date */
    if ( gtk_notebook_get_current_page (GTK_NOTEBOOK(notebook)) == ARCHIVE_ASSISTANT_MENU )
    {
        if ( GTK_WIDGET_IS_SENSITIVE (initial_date) )
        {
            GDate *init_gdate;
            GDate *final_gdate;

            init_gdate = gsb_calendar_entry_get_date (initial_date);
            final_gdate = gsb_calendar_entry_get_date (final_date);
            string = NULL;

            if ( !strlen ( gtk_entry_get_text ( GTK_ENTRY ( initial_date ) ) ) )
                string = make_red ( _("Please fill the initial date."));
            if ( !string
                &&
                !strlen ( gtk_entry_get_text ( GTK_ENTRY ( final_date ) ) ) )
                string = make_red ( _("Please fill the final date.") );
            if ( !string
                 &&
                 init_gdate
                 &&
                 final_gdate
                 &&
                 g_date_compare (init_gdate, final_gdate) > 0 )
                string = make_red (_("The initial date is after the final date."));
            if ( !string
                 &&
                 !init_gdate)
                string = make_red (_("The initial date is not valid."));
            if ( !string
                 &&
                 !final_gdate )
                string = make_red (_("The final date is not valid."));
            if ( !string
                 &&
                 gsb_data_archive_get_from_date (init_gdate) )
                string = make_red (_("The initial date belongs already to an archive."));
            if ( !string
                 &&
                 gsb_data_archive_get_from_date (final_gdate) )
                string = make_red (_("The final date belongs already to an archive."));

            if (string)
            {
                gtk_label_set_markup ( GTK_LABEL (label_archived), string );
                g_free (string);
                gsb_assistant_sensitive_button_next ( assistant, FALSE );
                return FALSE;
            }

            /* the dates are ok */
            tmp_list = gsb_data_transaction_get_transactions_list ();
            while (tmp_list)
            {
                gint transaction_number;

                transaction_number = gsb_data_transaction_get_transaction_number (
                        tmp_list -> data);

                if ( g_date_compare ( init_gdate,
                        gsb_data_transaction_get_date (transaction_number)) <= 0
                 &&
                 g_date_compare ( final_gdate,
                        gsb_data_transaction_get_date (transaction_number)) >= 0 )
                    /* the transaction is into the dates, we append its address to the list to archive
                     * we could use gsb_assistant_archive_add_transaction_to_list but it's a lost of time
                     * because all the linked transactions will be taken because we work with dates,
                     * so don't use that function and add the transaction directly */
                    list_transaction_to_archive = g_slist_append ( list_transaction_to_archive,
                                tmp_list -> data );

                tmp_list = tmp_list -> next;
            }
            }

        /* ok for now the choice is on fyear */
        else if (  GTK_WIDGET_IS_SENSITIVE ( financial_year_button ) )
        {
            gint fyear_number;
            string = NULL;

            fyear_number = gsb_fyear_get_fyear_from_combobox ( financial_year_button, NULL );

            if (!fyear_number)
                string = make_red (_("Please choose a financial year."));

            if ( !string
             &&
             gsb_data_archive_get_from_fyear ( fyear_number ) )
                string = make_red (_("There is already an archive for that financial year."));

            if (string)
            {
                gtk_label_set_markup ( GTK_LABEL (label_archived), string );
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
                {
                /* the transaction belongs to the fyear, we append its address to the list to archive
                 * all the linked transactions will be taken */
                gsb_assistant_archive_add_transaction_to_list (tmp_list -> data);
                }
                tmp_list = tmp_list -> next;
            }
        }
        /* ok for now the choice is on report */
        else if (  GTK_WIDGET_IS_SENSITIVE (report_button))
        {
            gint report_number;
            string = NULL;

            report_number = gsb_report_get_report_from_combobox (report_button);

            if (!report_number)
                string = make_red (_("Please choose a report."));

            if (string)
            {
                gtk_label_set_markup ( GTK_LABEL (label_archived), string );
                g_free (string);
                gsb_assistant_sensitive_button_next ( assistant, FALSE );
                return FALSE;
            }

            /* the report is ok */
            report_transactions_list = recupere_opes_etat (report_number);

            /* the list from report doesn't contain contra-transaction and mother/children split,
             * so for each transaction of that list, we need to check the contra-transaction, the mother and children
             * and add them to the list */
            tmp_list = report_transactions_list;
            while (tmp_list)
            {
                /* just call add_transaction_to_list, all the linked transactions will be taken */
                gsb_assistant_archive_add_transaction_to_list (tmp_list -> data);
                tmp_list = tmp_list -> next;
            }
            g_slist_free (report_transactions_list);
        }

        /* there is some transactions to archive */
        string = g_strdup_printf (_("%d transactions out of %d will be archived."),
                            g_slist_length (list_transaction_to_archive),
                            g_slist_length (gsb_data_transaction_get_transactions_list () ) );
        gtk_label_set_text ( GTK_LABEL (label_archived), string);
        g_free (string);

        gsb_assistant_sensitive_button_next ( assistant, TRUE );
    }

    else if ( gtk_notebook_get_current_page (GTK_NOTEBOOK(notebook)) == 
     ARCHIVE_ASSISTANT_ARCHIVE_NAME )
    {
        if ( strlen ( gtk_entry_get_text ( GTK_ENTRY ( name_entry ) ) ) )
            gsb_assistant_sensitive_button_next ( assistant, TRUE );
        else
            gsb_assistant_sensitive_button_next ( assistant, FALSE );
    }
    
    return FALSE;
}


/**
 * add a transaction (in fact is pointer) to the list of transactions wich will be archived
 * add too all the linked transactions with it
 *  ie :    the contra-transfer if exists
 *      if child, the mother and the other children
 *      if split, the children
 *
 * prevent multiple entry of the transaction, so can just call that function,
 * 	the same transaction won't be added several times
 * a NULL transaction_pointer will just be returned FALSE
 *
 * \param transaction_pointer
 *
 * \return 
 * */
static void gsb_assistant_archive_add_transaction_to_list ( gpointer transaction_pointer )
{
    gint transaction_number;

    if ( !transaction_pointer
     ||
     !(transaction_number = gsb_data_transaction_get_transaction_number (transaction_pointer)) )
    return;

    /* add the transaction itself */
    if ( !g_slist_find ( list_transaction_to_archive,
                        transaction_pointer ) )
    list_transaction_to_archive = g_slist_append ( list_transaction_to_archive,
                        transaction_pointer );

    /* check for contra-transaction */
    gsb_assistant_archive_add_contra_transaction_to_list (transaction_number);
 
    /* check for split */
    gsb_assistant_archive_add_children_to_list (transaction_number);

    /* check for child
     * if the transaction is a child, the best way is to get the mother and check for split */
    gsb_assistant_archive_add_children_to_list (gsb_data_transaction_get_mother_transaction_number (transaction_number));

    return;
}


/**
 * check the transaction
 * if it's a transfer, add the contra-transaction to the archive list
 * if the contra-transaction is a child of split, add too all the split
 *
 * \param transaction_number the transaction we want to check and add its contra-transaction
 *
 * \return
 * */
static void gsb_assistant_archive_add_contra_transaction_to_list ( gint transaction_number )
{
    gint contra_transaction_number;

    if (transaction_number <= 0)
	return;

    contra_transaction_number = gsb_data_transaction_get_contra_transaction_number (
                        transaction_number);
    if (contra_transaction_number > 0)
    {
	gpointer contra_transaction_pointer;

	contra_transaction_pointer = gsb_data_transaction_get_pointer_of_transaction (
                        contra_transaction_number);

	/* add the contra-transaction */
	if ( !g_slist_find ( list_transaction_to_archive,
			    contra_transaction_pointer ) )
	    list_transaction_to_archive = g_slist_append ( list_transaction_to_archive,
							   contra_transaction_pointer );

	/* if the contra-transaction is a child of split,
	 * we need to add the split and all children to the list */
	gsb_assistant_archive_add_children_to_list (
                        gsb_data_transaction_get_mother_transaction_number (
                        contra_transaction_number) );
    }
    return;
}

/**
 * check the transaction
 * if it's a split, add all the children (and their contra-transactions) to the archive list
 * this will add too the split itself
 *
 * \param transaction_number the transaction we want to check and add the children
 *
 * \return
 * */
static void gsb_assistant_archive_add_children_to_list ( gint transaction_number )
{
    if (transaction_number <= 0)
	return;

    if (gsb_data_transaction_get_split_of_transaction (transaction_number))
    {
	GSList *child_list;

	/* add the split */
	if (!g_slist_find ( list_transaction_to_archive,
			    gsb_data_transaction_get_pointer_of_transaction (transaction_number)))
	    list_transaction_to_archive = g_slist_append ( list_transaction_to_archive,
							   gsb_data_transaction_get_pointer_of_transaction (transaction_number));

	/* now add the children */
	child_list = gsb_data_transaction_get_children (transaction_number, FALSE);
	while (child_list)
	{
	    /* first add the child */
	    if (!g_slist_find ( list_transaction_to_archive,
				child_list -> data )
		&&
		/* we don't want white lines */
		gsb_data_transaction_get_transaction_number (child_list -> data) > 0 )
		list_transaction_to_archive = g_slist_append ( list_transaction_to_archive,
							       child_list -> data );

	    /* check if the child is a transfer */
	    gsb_assistant_archive_add_contra_transaction_to_list (gsb_data_transaction_get_transaction_number (child_list -> data));

	    child_list = child_list -> next;
	}
    }
    return;
}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
