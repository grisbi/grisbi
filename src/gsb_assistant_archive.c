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
#include "./gsb_fyear.h"
#include "./gsb_report.h"
#include "./traitement_variables.h"
#include "./utils_str.h"
#include "./etats_calculs.h"
#include "./utils.h"
#include "./include.h"
/*END_INCLUDE*/

/*START_STATIC*/
static  GtkWidget *gsb_assistant_archive_page_menu ( GtkWidget *assistant );
static  GtkWidget *gsb_assistant_archive_page_success ( void );
static  gboolean gsb_assistant_archive_switch_to_intro ( GtkWidget *assistant,
							gint new_page );
static  gboolean gsb_assistant_archive_switch_to_menu ( GtkWidget *assistant,
						       gint new_page );
static  gboolean gsb_assistant_archive_switch_to_succes ( GtkWidget *assistant,
							 gint new_page );
static  gboolean gsb_assistant_archive_update_labels ( GtkWidget *assistant );
static  void gsb_assistant_archive_update_labels_no_archive ( GtkWidget *assistant );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

enum archive_assistant_page
{
    ARCHIVE_ASSISTANT_INTRO= 0,
    ARCHIVE_ASSISTANT_MENU,
    ARCHIVE_ASSISTANT_SUCCESS
};

static GtkWidget *initial_date;
static GtkWidget *final_date;
static GtkWidget *financial_year_button;
static GtkWidget *report_button;
static GtkWidget *name_entry;
static GtkWidget *label_nb_archived;
static GtkWidget *label_nb_loaded;
static GtkWidget *label_warning;

static GtkWidget *vbox_congratulation;
static GtkWidget *vbox_failed;
static GtkWidget *label_congratulation;

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
    assistant = gsb_assistant_new ( _("Archive the transactions"),
				    _("This assistant will help you to file some transactions, to increase the speed of grisbi.\n"
				      "By default, Grisbi doesn't export an archive. It keeps the transactions into the file,\n"
				      "but don't work with them anymore.\n"
				      "Obviously, you will be able to show them again and work on them, and you will be able\n"
				      "to export them in an archive file if necessary.\n"),
				    "grisbi-logo.png",
				    G_CALLBACK (gsb_assistant_archive_switch_to_intro));
    gsb_assistant_add_page ( assistant,
			     gsb_assistant_archive_page_menu (assistant),
			     ARCHIVE_ASSISTANT_MENU,
			     ARCHIVE_ASSISTANT_INTRO,
			     ARCHIVE_ASSISTANT_SUCCESS,
			     G_CALLBACK (gsb_assistant_archive_switch_to_menu));
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
    GtkWidget *vbox_2;
    GtkWidget *label;
    GtkWidget *button;
    GtkWidget *hbox;
    GtkWidget *separator;

    page = gtk_hbox_new (FALSE, 15);
    gtk_container_set_border_width ( GTK_CONTAINER (page),
				     10 );

    vbox = gtk_vbox_new (FALSE, 5);
    gtk_box_pack_start ( GTK_BOX (page),
			 vbox,
			 FALSE, FALSE, 0 );

    /* set up the menu */
    label = gtk_label_new (_("Please choose the way to make your archive :\n"));
    gtk_misc_set_alignment ( GTK_MISC (label),
			     0, 0.5 );
    gtk_box_pack_start ( GTK_BOX (vbox),
			 label,
			 FALSE, FALSE, 0 );

    /* archive by date */
    button = gtk_radio_button_new_with_label ( NULL,
					       _("Archive by date"));
    g_signal_connect_object ( G_OBJECT (button),
			       "toggled",
			       G_CALLBACK (gsb_assistant_archive_update_labels),
			       G_OBJECT (assistant),
			       G_CONNECT_AFTER | G_CONNECT_SWAPPED);
    gtk_box_pack_start ( GTK_BOX (vbox),
			 button,
			 FALSE, FALSE, 0 );

    label = gtk_label_new (_("	Grisbi will archive all the transactions between the initial and final date.\n"));
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

    label = gtk_label_new (_("		Initial date :"));
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

    label = gtk_label_new (_("Final date :"));
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

    label = gtk_label_new (_("	Grisbi will archive all the transactions belonging to the financial year.\n" ));
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

    label = gtk_label_new (_("		Chosen financial year :"));
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

    label = gtk_label_new (_("	Grisbi will archive the transactions selected by a report.\n" ));
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

    label = gtk_label_new (_("		Chosen report :"));
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

    /* now the name of the archive */
    separator = gtk_vseparator_new ();
    gtk_box_pack_start ( GTK_BOX (page),
			 separator,
			 FALSE, FALSE, 0 );

    vbox = gtk_vbox_new (FALSE, 5);
    gtk_box_pack_start ( GTK_BOX (page),
			 vbox,
			 FALSE, FALSE, 0 );


    label = gtk_label_new (_("Choose a name for the archive : "));
    gtk_misc_set_alignment ( GTK_MISC (label),
			     0, 0.5 );
    gtk_box_pack_start ( GTK_BOX (vbox),
			 label,
			 FALSE, FALSE, 0 );

    hbox = gtk_hbox_new ( FALSE, 10 );
    gtk_box_pack_start ( GTK_BOX (vbox),
			 hbox,
			 FALSE, FALSE, 0 );

    name_entry = gtk_entry_new ();
    g_signal_connect_object ( G_OBJECT (name_entry),
			      "changed",
			      G_CALLBACK (gsb_assistant_archive_update_labels),
			      G_OBJECT (assistant),
			      G_CONNECT_AFTER | G_CONNECT_SWAPPED);
    gtk_box_pack_end ( GTK_BOX (hbox),
			 name_entry,
			 FALSE, FALSE, 0 );

    /* the vbox now with the informations */

    vbox_2 = gtk_vbox_new ( FALSE, 10 );
    gtk_box_pack_start ( GTK_BOX (vbox),
			 vbox_2,
			 TRUE, FALSE, 10 );

    /* how much transactions will be archived ? */
    label_nb_archived = gtk_label_new (NULL);
    gtk_box_pack_start ( GTK_BOX (vbox_2),
			 label_nb_archived,
			 FALSE, FALSE, 0 );

    /* how much transactions it remains ? */
    label_nb_loaded = gtk_label_new (NULL);
    gtk_box_pack_start ( GTK_BOX (vbox_2),
			 label_nb_loaded,
			 FALSE, FALSE, 0 );

    /* why the button is still unsensitive ? */
    label_warning = gtk_label_new (NULL);
    gtk_box_pack_start ( GTK_BOX (vbox_2),
			 label_warning,
			 FALSE, FALSE, 0 );

    gtk_widget_show_all (page);
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
    GtkWidget *page;
    GtkWidget *label;

    page = gtk_vbox_new ( FALSE, 0 );
    gtk_container_set_border_width ( GTK_CONTAINER (page),
				     10 );

    /* we create 2 vbox, one for congratulation, one for failed,
     * only one will be showed */
    vbox_congratulation = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start ( GTK_BOX (page),
			 vbox_congratulation,
			 FALSE, FALSE, 0 );

    label = gtk_label_new (_("Congratulation !\n"));
    gtk_misc_set_alignment ( GTK_MISC (label),
			     0, 0.5 );
    gtk_box_pack_start ( GTK_BOX (vbox_congratulation),
			 label,
			 FALSE, FALSE,
			 0 );

    label_congratulation = gtk_label_new (NULL);
    gtk_box_pack_start ( GTK_BOX (vbox_congratulation),
			 label_congratulation,
			 FALSE, FALSE,
			 0 );

    label = gtk_label_new (_("You can click on the 'Previous' button to make another archive,\n"
			     "or 'Close' the assistant.\n\n"
			     "If ever grisbi is still too slow after making all the archives,\n"
			     "remember that it exists a function in the configuration to avoid to load\n"
			     "the R marked transactions at the begining, to increase the speed.\n\n"
			     "	Please press the Previous or Close button."));
    gtk_misc_set_alignment ( GTK_MISC (label),
			     0, 0.5 );
    gtk_box_pack_start ( GTK_BOX (vbox_congratulation),
			 label,
			 FALSE, FALSE,
			 0 );

    /* make the failed box */
    vbox_failed = gtk_vbox_new (FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX (page),
			 vbox_failed,
			 FALSE, FALSE, 0 );

    label = gtk_label_new (_("An error occurred while creating the archive...\n"
			     "Please try to find the problem and contact the grisbi team to correct it.\n\n"
			     "	Please press the Previous or Close button."));
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
/* xxx voir ici devrait pouvoir faire 1 seule fonction de ces 3, avec new_page qui change pour chacune, à vérif */

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
    gtk_label_set_text ( GTK_LABEL (label_warning),
			 NULL );
    gsb_assistant_change_button_next ( assistant,
				       GTK_STOCK_APPLY, GTK_RESPONSE_YES );
    gsb_assistant_archive_update_labels (assistant);

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

    if (!list_transaction_to_archive)
    {
	/* should not happen */
	gtk_widget_hide (vbox_congratulation);
	gtk_widget_show (vbox_failed);
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
    string = g_strdup_printf ( _("The archive %s was successfully created, and %d transactions were associated to it.\n(now, %d transactions will be loaded into Grisbi)\n"),
			       gsb_data_archive_get_name (archive_number),
			       g_slist_length (list_transaction_to_archive),
			       g_slist_length (gsb_data_transaction_get_transactions_list ()));
    gtk_label_set_text ( GTK_LABEL (label_congratulation),
			 string );
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

    if (list_transaction_to_archive)
    {
	g_slist_free (list_transaction_to_archive);
	list_transaction_to_archive = NULL;
    }

    if (GTK_WIDGET_IS_SENSITIVE (initial_date))
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
	    gtk_label_set_markup ( GTK_LABEL (label_warning),
				   string );
	    g_free (string);
	    gsb_assistant_archive_update_labels_no_archive (assistant);
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

    if (GTK_WIDGET_IS_SENSITIVE (financial_year_button))
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
	    gtk_label_set_markup ( GTK_LABEL (label_warning),
				   string );
	    g_free (string);
	    gsb_assistant_archive_update_labels_no_archive (assistant);
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

    if (GTK_WIDGET_IS_SENSITIVE (report_button))
    {
	/* ok for now the choice is on fyear */
	gint report_number;
	string = NULL;

	report_number = gsb_report_get_report_from_combobox (report_button);

	if (!report_number)
	    string = my_strdup (_("<span foreground=\"red\">Please choose a report</span>"));

	if (string)
	{
	    gtk_label_set_markup ( GTK_LABEL (label_warning),
				   string );
	    g_free (string);
	    gsb_assistant_archive_update_labels_no_archive (assistant);
	    return FALSE;
	}

	/* the report is ok */
	list_transaction_to_archive = recupere_opes_etat (report_number);
    }

    /* update the labels */
    if (list_transaction_to_archive)
    {
	/* there is some transactions to archive */
	gint length;

	length = g_slist_length (list_transaction_to_archive);

	string = g_strdup_printf (_("%d transactions will be archived"),
				  length);
	gtk_label_set_text ( GTK_LABEL (label_nb_archived),
			     string);
	g_free (string);

	string = g_strdup_printf (_("And %d will be loaded into Grisbi."),
				  g_slist_length (gsb_data_transaction_get_transactions_list ()) - length);
	gtk_label_set_text ( GTK_LABEL (label_nb_loaded),
			     string );
	g_free (string);
    }
    else
    {
	/* there is no transaction to archive */
	gtk_label_set_text ( GTK_LABEL (label_warning),
			     NULL );
	gsb_assistant_archive_update_labels_no_archive (assistant);
	return FALSE;
    }

    /* we show the next button only if the name is ok */
    if (strlen ( gtk_entry_get_text (GTK_ENTRY (name_entry))))
    {
	if (gsb_data_archive_get_number_by_name (gtk_entry_get_text (GTK_ENTRY (name_entry))))
	{
	    string = my_strdup (_("<span foreground=\"red\">The name is already use, please change it.</span>"));
	    gtk_label_set_markup ( GTK_LABEL (label_warning),
				   string );
	    g_free (string);
	    gsb_assistant_sensitive_button_next ( assistant, FALSE );
	}
	else
	{
	    gtk_label_set_text ( GTK_LABEL (label_warning),
				 NULL );
	    gsb_assistant_sensitive_button_next ( assistant, TRUE );
	}
    }
    else
    {
	string = my_strdup (_("<span foreground=\"red\">Please fill the name of the archive</span>"));
	gtk_label_set_markup ( GTK_LABEL (label_warning),
			       string );
	g_free (string);

	gsb_assistant_sensitive_button_next ( assistant, FALSE );
    }

    return FALSE;
}


/**
 * set the labels to the default position when no archive can be done
 * 
 * \param assistant
 *
 * \return
 * */
static void gsb_assistant_archive_update_labels_no_archive ( GtkWidget *assistant )
{
    gchar *string;

    gtk_label_set_text ( GTK_LABEL (label_nb_archived),
			 _("For now, no transaction will be archived"));

    string = g_strdup_printf (_("And %d will be loaded into Grisbi."),
			      g_slist_length (gsb_data_transaction_get_transactions_list ()));
    gtk_label_set_text ( GTK_LABEL (label_nb_loaded),
			 string );
    g_free (string);

    gsb_assistant_sensitive_button_next ( assistant, FALSE );
    return ;
}
