/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
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
 * \file gsb_assistant_reconcile_config.c
 * we find here the pages used by the assistant to work with the reconcile configuration
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_assistant_reconcile_config.h"
#include "dialog.h"
#include "gsb_account.h"
#include "gsb_assistant.h"
#include "gsb_calendar_entry.h"
#include "gsb_data_account.h"
#include "gsb_data_payee.h"
#include "gsb_data_reconcile.h"
#include "gsb_data_transaction.h"
#include "utils_dates.h"
#include "gsb_real.h"
#include "gsb_reconcile_config.h"
#include "utils_str.h"
#include "utils.h"
#include "structures.h"
#include "gsb_transactions_list.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean gsb_assistant_reconcile_config_hide_label_error ( GtkWidget *editable,
                        GtkWidget *label );
static gboolean gsb_assistant_reconcile_config_lauch_auto_asso ( GtkWidget *button,
                        GtkWidget *assistant );
static gboolean gsb_assistant_reconcile_config_lauch_manu_asso ( GtkWidget *button,
                        GtkWidget *assistant );
static gboolean gsb_assistant_reconcile_config_page_add_new_reconcile ( GtkWidget *button,
                        GtkWidget *label );
static GtkWidget *gsb_assistant_reconcile_config_page_automatically_associate ( GtkWidget *assistant );
static GtkWidget *gsb_assistant_reconcile_config_page_manually_associate ( GtkWidget *assistant );
static GtkWidget *gsb_assistant_reconcile_config_page_menu ( GtkWidget *assistant );
static gboolean gsb_assistant_reconcile_config_page_menu_toggled ( GtkWidget *button,
                        GtkWidget *assistant );
static GtkWidget *gsb_assistant_reconcile_config_page_new_reconcile ( void );
static GtkWidget *gsb_assistant_reconcile_config_page_success ( void );
static gboolean gsb_assistant_reconcile_config_update_auto_asso ( GtkWidget *assistant,
                        gint new_page );
static gboolean gsb_assistant_reconcile_config_update_manu_asso ( GtkWidget *assistant,
                        gint new_page );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

enum reconcile_assistant_page
{
    RECONCILE_ASSISTANT_INTRO= 0,
    RECONCILE_ASSISTANT_MENU,
    RECONCILE_ASSISTANT_NEW_RECONCILE,
    RECONCILE_ASSISTANT_AUTOMATICALLY_ASSOCIATE,
    RECONCILE_ASSISTANT_MANUALLY_ASSOCIATE,
    RECONCILE_ASSISTANT_SUCCESS
};

/* widgets for the menu page */
/* the label page 2 (the menu) */
static GtkWidget *label_transactions_to_link_1 = NULL;


/* widgets for the page to create a new reconcile */
static GtkWidget *reconcile_name_entry = NULL;
static GtkWidget *reconcile_init_date_entry = NULL;
static GtkWidget *reconcile_final_date_entry = NULL;
static GtkWidget *reconcile_init_balance_entry = NULL;
static GtkWidget *reconcile_final_balance_entry = NULL;
static GtkWidget *reconcile_account_button = NULL;

/* the number of transactions to link, filled when click the button to associate
 * the R marked transactions without a reconcile */
static gint transactions_to_link;



/* variables for the page to automatically associate */
static GtkWidget *label_transactions_to_link_2 = NULL;
static GtkWidget *label_possible_association = NULL;
static GtkWidget *button_run_association = NULL;

/* structure making the link between a transaction and the corresponding reconcile,
 * used for automatically association */
struct association_transaction_reconcile
{
    gint transaction_number;
    gint reconcile_number;
};
/* list containing the above struct */
static GSList *list_association = NULL;

/* variables for the page to manually associate */
static GtkWidget *label_transactions_to_link_3 = NULL;
static GtkWidget *treeview_transactions_to_link = NULL;
enum transactions_column {
    TRANSACTION_DATE = 0,
    TRANSACTION_PAYEE,
    TRANSACTION_AMOUNT,
    TRANSACTION_ACCOUNT,
    TRANSACTION_NUMBER,
    TRANSACTION_NB_COL
};


/**
 * function called to launch the reconcile_config assistant
 * calculate the number of transactions to link, and refuse to run the assistant if none
 *
 * \param
 *
 * \return GtkResponseType, the returned value from the assistant
 * */
GtkResponseType gsb_assistant_reconcile_config_run ( void )
{
    GtkResponseType return_value;
    GSList *transactions_list = NULL;
    GSList *tmp_list;
    gint transaction_number;
    GtkWidget *assistant;
	gchar* tmpstr;

    tmp_list = gsb_data_transaction_get_transactions_list ();
    while (tmp_list)
    {
	transaction_number = gsb_data_transaction_get_transaction_number (tmp_list -> data);

	if ( gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_RAPPROCHEE
	     &&
	     !gsb_data_transaction_get_reconcile_number (transaction_number))
	    transactions_list = g_slist_append ( transactions_list,
						 GINT_TO_POINTER (transaction_number));
	tmp_list = tmp_list -> next;
    }

    if (!transactions_list)
    {
	dialogue (_("No marked transactions without reconciliation found !"));
	return GTK_RESPONSE_CANCEL;
    }

    /* get the number of transactions to associate, we will decrease it for each association */
    transactions_to_link = g_slist_length (transactions_list);

    /* come here if we have some orphan transactions
     * this can happen by 2 ways :
     * for old users of grisbi, before i don't remember what version, there were no reconcile number,
     * 		the reconciled transactions were juste marked R
     * before the 0.6.0, ctrl R didn't permit to choose a reconcile
     *
     * for the 2nd item, no problem, we show a list of reconciles and the user can choose what reconcile he wants
     * for the first item the problem is there is no reconcile number to go with that transactions...
     * so we will use the assistant to
     * 	-permit to create a reconcile directly (not possible normaly in the configuration
     * 	-permit to choose a reconcile number for each transactions without reconcile
     * 	- do an automatic find for reconcile, usefull in the first item, when very much
     * 		transactions without reconcile, but we need to make the old reconciles before,
     * 		and set the good date for all the reconciles (because grisbi set them automatically
     * 		at the first update to grisbi 0.6.0 )*/

    /* first, create the assistant */
    tmpstr = g_strdup_printf (_("Grisbi found %d marked transactions not associated with a reconciliation number, "
						       "this can happen for old users of grisbi or from a misuse of the Ctrl-R shortcut.\n\n"
						       "This assistant will help you make the link between such transactions and a reconciliation.\n\n"
						       "Before continuing, you should first check if all the dates of the existing reconciliations are good "
						       "because grisbi will try to guess them not very precisely "
						       "(you will be able to create new reconciliations in the next step). "
						       "Previous reconciliations will be available too."),
						       transactions_to_link );
    assistant = gsb_assistant_new ( _("Associate orphan transactions to a reconciliation"),
				    tmpstr,
				    "reconat.png",
				    NULL );
    g_free ( tmpstr );

    gsb_assistant_add_page ( assistant,
			     gsb_assistant_reconcile_config_page_menu (assistant),
			     RECONCILE_ASSISTANT_MENU,
			     RECONCILE_ASSISTANT_INTRO,
			     RECONCILE_ASSISTANT_NEW_RECONCILE,
			     NULL );
    gsb_assistant_add_page ( assistant,
			     gsb_assistant_reconcile_config_page_new_reconcile (),
			     RECONCILE_ASSISTANT_NEW_RECONCILE,
			     RECONCILE_ASSISTANT_MENU,
			     RECONCILE_ASSISTANT_MENU,
			     NULL );
    gsb_assistant_add_page ( assistant,
			     gsb_assistant_reconcile_config_page_automatically_associate (assistant),
			     RECONCILE_ASSISTANT_AUTOMATICALLY_ASSOCIATE,
			     RECONCILE_ASSISTANT_MENU,
			     RECONCILE_ASSISTANT_MENU,
			     G_CALLBACK (gsb_assistant_reconcile_config_update_auto_asso));
    gsb_assistant_add_page ( assistant,
			     gsb_assistant_reconcile_config_page_manually_associate (assistant),
			     RECONCILE_ASSISTANT_MANUALLY_ASSOCIATE,
			     RECONCILE_ASSISTANT_MENU,
			     RECONCILE_ASSISTANT_MENU,
			     G_CALLBACK (gsb_assistant_reconcile_config_update_manu_asso));
    gsb_assistant_add_page ( assistant,
			     gsb_assistant_reconcile_config_page_success (),
			     RECONCILE_ASSISTANT_SUCCESS,
			     RECONCILE_ASSISTANT_MENU,
			     RECONCILE_ASSISTANT_MENU,
			     NULL );
    return_value = gsb_assistant_run (assistant);
    gtk_widget_destroy (assistant);
    return return_value;
}


/**
 * create the page 2 of the assistant to make the link between marked transactions and reconciles
 * that page propose to :
 * 	create new reconciles
 * 	automatically associate transactions to known reconciles
 * 	associate transactions to a reconcile, by hand
 * a change to that menu will change the next page, according to it
 *
 * \param assistant the GtkWidget assistant
 *
 * \return a GtkWidget containing the page
 * */
static GtkWidget *gsb_assistant_reconcile_config_page_menu ( GtkWidget *assistant )
{
    GtkWidget *page;
    GtkWidget *separator;
    GtkWidget *label;
    GtkWidget *button;
	gchar* tmpstr;

    page = gtk_vbox_new (FALSE, 5);
    gtk_container_set_border_width ( GTK_CONTAINER(page), 12 );

    tmpstr = g_strdup_printf (_("Still %d transactions to link with a reconciliation."),
								   transactions_to_link);
    label_transactions_to_link_1 = gtk_label_new (tmpstr );
    g_free ( tmpstr );
    gtk_misc_set_alignment ( GTK_MISC (label_transactions_to_link_1),
			     0, 0.5 );

    gtk_box_pack_start ( GTK_BOX (page),
			 label_transactions_to_link_1,
			 FALSE, FALSE, 0 );

    separator = gtk_hseparator_new ();
    gtk_box_pack_start ( GTK_BOX (page),
			 separator,
			 FALSE, FALSE, 0 );

    /* set up the menu */
    label = gtk_label_new (_("Choose the next operation to do: "));
    gtk_misc_set_alignment ( GTK_MISC (label),
			     0, 0.5 );
    gtk_box_pack_start ( GTK_BOX (page),
			 label,
			 FALSE, FALSE, 0 );

    /* create a new reconcile */
    button = gtk_radio_button_new_with_label ( NULL,
					       _("Manually create a new reconciliation"));
    g_object_set_data ( G_OBJECT (button),
			"next_page", GINT_TO_POINTER (RECONCILE_ASSISTANT_NEW_RECONCILE));
    g_signal_connect ( G_OBJECT (button),
		       "toggled",
		       G_CALLBACK (gsb_assistant_reconcile_config_page_menu_toggled),
		       assistant );
    gtk_box_pack_start ( GTK_BOX (page),
			 button,
			 FALSE, FALSE, 0 );

    label = gtk_label_new (_("	After a long use, some transactions may be before the first known reconciliation.\n"
			     "	You can create all the previous needed reconciliations here."));
    gtk_misc_set_alignment ( GTK_MISC (label),
			     0, 0.5 );
    gtk_box_pack_start ( GTK_BOX (page),
			 label,
			 FALSE, FALSE, 0 );

    /* automatically associate the transactions without reconcile number */
    button = gtk_radio_button_new_with_label ( gtk_radio_button_get_group (GTK_RADIO_BUTTON (button)),
					       _("Automatically associate transactions without reconciliation number with the known reconciliations"));
    g_object_set_data ( G_OBJECT (button),
			"next_page", GINT_TO_POINTER (RECONCILE_ASSISTANT_AUTOMATICALLY_ASSOCIATE));
    g_signal_connect ( G_OBJECT (button),
		       "toggled",
		       G_CALLBACK (gsb_assistant_reconcile_config_page_menu_toggled),
		       assistant );
    gtk_box_pack_start ( GTK_BOX (page),
			 button,
			 FALSE, FALSE, 0 );

    label = gtk_label_new (_("	This will make all the work for you if you create previously all the needed reconciles\n"
			     "	and adjust the date of the known reconciles."));
    gtk_misc_set_alignment ( GTK_MISC (label),
			     0, 0.5 );
    gtk_box_pack_start ( GTK_BOX (page),
			 label,
			 FALSE, FALSE, 0 );

    /* associate the transactions without reconcile number by hand */
    button = gtk_radio_button_new_with_label ( gtk_radio_button_get_group (GTK_RADIO_BUTTON (button)),
					       _("Associate by hands the transactions without reconciliation number with the known reconciles"));
    g_object_set_data ( G_OBJECT (button),
			"next_page", GINT_TO_POINTER (RECONCILE_ASSISTANT_MANUALLY_ASSOCIATE));
    g_signal_connect ( G_OBJECT (button),
		       "toggled",
		       G_CALLBACK (gsb_assistant_reconcile_config_page_menu_toggled),
		       assistant );
    gtk_box_pack_start ( GTK_BOX (page),
			 button,
			 FALSE, FALSE, 0 );

    label = gtk_label_new (_("	You control all and have a lot of time to spend on your computer !\n"
			     "	(you will be able to choose a reconciliation for each orphan transaction)."));
    gtk_misc_set_alignment ( GTK_MISC (label),
			     0, 0.5 );
    gtk_box_pack_start ( GTK_BOX (page),
			 label,
			 FALSE, FALSE, 0 );




    gtk_widget_show_all (page);
    return page;
}


/**
 * create the page to create new reconciles
 *
 * \param
 *
 * \return a GtkWidget, the page to the assistant
 * */
static GtkWidget *gsb_assistant_reconcile_config_page_new_reconcile ( void )
{
    GtkWidget *page;
    GtkWidget *label;
    GtkWidget *paddingbox;
    GtkWidget *table;
    GtkWidget *button;
    GtkWidget *hbox;

    page = gtk_vbox_new ( FALSE, 0 );
    gtk_container_set_border_width ( GTK_CONTAINER(page), 12 );

    paddingbox = new_paddingbox_with_title ( page, FALSE,
					     _("Fill the fields and click the button \"Create the new reconcile\""));

    /* create the table, this is the same as the reconcile config,
     * but here it's to create new ones */
	table = gtk_table_new ( 4, 3, FALSE );
	gtk_table_set_row_spacings ( GTK_TABLE ( table ), 6 );
	gtk_table_set_col_spacings ( GTK_TABLE ( table ), 6 );
	gtk_box_pack_start ( GTK_BOX (paddingbox), table, FALSE, FALSE, 0 );

	/* set the name */
	label = gtk_label_new ( _("Name: ") );
	gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
	gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
	gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 0, 1,
			GTK_SHRINK | GTK_FILL, 0, 0, 0 );

	reconcile_name_entry = gtk_entry_new ();
	gtk_table_attach ( GTK_TABLE ( table ), reconcile_name_entry, 1, 2, 0, 1,
			GTK_EXPAND | GTK_FILL, 0, 10, 0 );

	/* set the choice of account */
	label = gtk_label_new ( _("Account: ") );
	gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
	gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
	gtk_table_attach ( GTK_TABLE ( table ), label, 2, 3, 0, 1,
			GTK_SHRINK | GTK_FILL, 0, 10, 0 );

	reconcile_account_button = gsb_account_create_combo_list ( NULL, NULL, TRUE );
	gtk_combo_box_set_active ( GTK_COMBO_BOX (reconcile_account_button), 0 );
	gtk_table_attach ( GTK_TABLE ( table ), reconcile_account_button, 3, 4, 0, 1,
			GTK_EXPAND | GTK_FILL, 0, 0, 0 );

	/* set the initial date */
	label = gtk_label_new ( _("Initial date: ") );
	gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
	gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
	gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 1, 2,
			GTK_SHRINK | GTK_FILL, 0, 0, 0 );

	reconcile_init_date_entry = gsb_calendar_entry_new (FALSE);
	gtk_table_attach ( GTK_TABLE ( table ), reconcile_init_date_entry, 1, 2, 1, 2,
			GTK_EXPAND | GTK_FILL, 0, 10, 0 );

	/* set the final date */
	label = gtk_label_new ( _("Final date: ") );
	gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
	gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
	gtk_table_attach ( GTK_TABLE ( table ), label, 0, 1, 2, 3,
			GTK_SHRINK | GTK_FILL, 0, 0, 0 );

	reconcile_final_date_entry = gsb_calendar_entry_new (FALSE);
	gtk_table_attach ( GTK_TABLE ( table ), reconcile_final_date_entry, 1, 2, 2, 3,
			GTK_EXPAND | GTK_FILL, 0, 10, 0 );

	/* set the initial balance */
	label = gtk_label_new ( _("Initial balance: ") );
	gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
	gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
	gtk_table_attach ( GTK_TABLE ( table ), label, 2, 3, 1, 2,
			GTK_SHRINK | GTK_FILL, 0, 10, 0 );

	reconcile_init_balance_entry = gtk_entry_new ();
	gtk_table_attach ( GTK_TABLE ( table ), reconcile_init_balance_entry, 3, 4, 1, 2,
			GTK_EXPAND | GTK_FILL, 0, 0, 0 );

    /* set the final balance */
	label = gtk_label_new ( _("Final balance: ") );
	gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
	gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
	gtk_table_attach ( GTK_TABLE ( table ), label, 2, 3, 2, 3,
			GTK_SHRINK | GTK_FILL, 0, 10, 0 );

	reconcile_final_balance_entry = gtk_entry_new ();
	gtk_table_attach ( GTK_TABLE ( table ), reconcile_final_balance_entry, 3, 4, 2, 3,
			GTK_EXPAND | GTK_FILL, 0, 0, 0 );

    /* create the button */
    hbox = gtk_hbox_new (TRUE, 0);
    gtk_box_pack_start ( GTK_BOX (page),
			 hbox,
			 FALSE, FALSE,
			 0 );

    button = gtk_button_new_with_label (_("Create the new reconciliation"));
    gtk_box_pack_end ( GTK_BOX (hbox),
		       button,
		       FALSE, FALSE,
		       0 );

    /* create the label */
    label = gtk_label_new (NULL);
    gtk_box_pack_start ( GTK_BOX (page),
			 label,
			 TRUE, TRUE,
			 0 );

    /* if we change anything in the entries, hide the label */
    g_signal_connect ( G_OBJECT (reconcile_name_entry),
		       "changed",
		       G_CALLBACK (gsb_assistant_reconcile_config_hide_label_error),
		       label );
    g_signal_connect ( G_OBJECT (reconcile_init_date_entry),
		       "changed",
		       G_CALLBACK (gsb_assistant_reconcile_config_hide_label_error),
		       label );
    g_signal_connect ( G_OBJECT (reconcile_final_date_entry),
		       "changed",
		       G_CALLBACK (gsb_assistant_reconcile_config_hide_label_error),
		       label );
    g_signal_connect ( G_OBJECT (reconcile_init_balance_entry),
		       "changed",
		       G_CALLBACK (gsb_assistant_reconcile_config_hide_label_error),
		       label );
    g_signal_connect ( G_OBJECT (reconcile_final_balance_entry),
		       "changed",
		       G_CALLBACK (gsb_assistant_reconcile_config_hide_label_error),
		       label );

    /* connect the button and give the message label as param */
    g_signal_connect ( G_OBJECT (button),
		       "clicked",
		       G_CALLBACK (gsb_assistant_reconcile_config_page_add_new_reconcile),
		       label );
    gtk_widget_show_all (page);
    return page;
}


/**
 * create the page to automatically associate the transactions to the reconciles
 *
 * \param assistant
 *
 * \return a GtkWidget, the page to the assistant
 * */
static GtkWidget *gsb_assistant_reconcile_config_page_automatically_associate ( GtkWidget *assistant )
{
    GtkWidget *page;
    GtkWidget *separator;
    GtkWidget *label;
    GtkWidget *hbox;

    page = gtk_vbox_new (FALSE, 5);
    gtk_container_set_border_width ( GTK_CONTAINER(page), 12 );

    /* for now we juste create the widgets, but cannot fill here,
     * we will fill when the user come to that page */
    /* first show the total of transactions to link, same as in the menu assistant */
    label_transactions_to_link_2 = gtk_label_new (NULL);
    gtk_box_pack_start ( GTK_BOX (page),
			 label_transactions_to_link_2,
			 FALSE, FALSE, 0 );

    separator = gtk_hseparator_new ();
    gtk_box_pack_start ( GTK_BOX (page),
			 separator,
			 FALSE, FALSE, 0 );

    label = gtk_label_new ( _("This assistant try to find a reconciliation to each orphan transaction,\n"
			     "according to the date of the transaction and the dates of the reconcile.\n"
			     "If all the orphan transactions cannot be linked here, you should check\n"
			     "if you did create all the necessary reconciles (first choice in the assistant\n"
			     "menu page) or check the transactions themselves."));
    gtk_misc_set_alignment ( GTK_MISC (label),
			     0, 0.5 );
    gtk_box_pack_start ( GTK_BOX (page),
			 label,
			 FALSE, FALSE, 0 );

    /* set the number of possible association, filled when come in that page */
    label_possible_association = gtk_label_new (NULL);
    gtk_box_pack_start ( GTK_BOX (page),
			 label_possible_association,
			 FALSE, FALSE, 12 );

    /* set the button to run the association */
    hbox = gtk_hbox_new (TRUE, 0);
    gtk_box_pack_start ( GTK_BOX (page),
			 hbox,
			 FALSE, FALSE,
			 0 );
    button_run_association = gtk_button_new_with_label ( _("Launch automatic association"));
    g_signal_connect ( G_OBJECT (button_run_association),
		       "clicked",
		       G_CALLBACK (gsb_assistant_reconcile_config_lauch_auto_asso),
		       assistant );
    gtk_box_pack_start ( GTK_BOX (hbox),
			 button_run_association,
			 FALSE, FALSE,
			 0 );

    gtk_widget_show_all (page);
    return page;
}


/**
 * create the page to manually associate the transactions to the reconciles
 * show a complete list of orphans transactions, and for each of them, we propose
 * to link them to a reconcile
 *
 * \param assistant
 *
 * \return a GtkWidget, the page to the assistant
 * */
static GtkWidget *gsb_assistant_reconcile_config_page_manually_associate ( GtkWidget *assistant )
{
    GtkWidget *page;
    GtkWidget *label;
    GtkWidget *separator;
    GtkWidget *scrolled_window;
    gint i;
    GtkWidget *button;
    GtkWidget *hbox;
    GtkListStore *store;
    gchar *titles[] = {
	_("Date"), _("Payee"), _("Amount"), _("Account")
    };
    gfloat alignment[] = {
	COLUMN_CENTER, COLUMN_LEFT, COLUMN_CENTER, COLUMN_CENTER
    };


    page = gtk_vbox_new ( FALSE, 0 );
    gtk_container_set_border_width ( GTK_CONTAINER(page), 12 );

    /* the number of transactions to link */
    label_transactions_to_link_3 = gtk_label_new (NULL);
    gtk_box_pack_start ( GTK_BOX (page),
			 label_transactions_to_link_3,
			 FALSE, FALSE, 0 );

    separator = gtk_hseparator_new ();
    gtk_box_pack_start ( GTK_BOX (page),
			 separator,
			 FALSE, FALSE, 0 );

    /* now we can show the list of orphans transactions */
    label = gtk_label_new (_("Select the transaction(s) you want to associate with a reconciliation: "));
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.0 );
    gtk_box_pack_start ( GTK_BOX (page),
			 label,
			 FALSE, FALSE, 10 );

    scrolled_window = gtk_scrolled_window_new (FALSE, FALSE);
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW (scrolled_window),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX (page),
			 scrolled_window,
			 TRUE, TRUE, 0 );
    gtk_widget_show (scrolled_window);

    /* set up the tree view */
    store = gtk_list_store_new ( TRANSACTION_NB_COL,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_INT );
    treeview_transactions_to_link = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
    g_object_unref (G_OBJECT(store));
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview_transactions_to_link), TRUE);
    gtk_tree_selection_set_mode ( gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview_transactions_to_link)),
				  GTK_SELECTION_MULTIPLE );
    gtk_container_add ( GTK_CONTAINER (scrolled_window),
			treeview_transactions_to_link );
    gtk_widget_show (treeview_transactions_to_link);

    /* set the columns */
    for (i=TRANSACTION_DATE ; i<TRANSACTION_NUMBER ; i++)
    {
	GtkTreeViewColumn *column;
	GtkCellRenderer *cell;
	cell = gtk_cell_renderer_text_new ();
	g_object_set ( G_OBJECT (cell),
		       "xalign", alignment[i],
		       NULL );
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_sizing ( column,
					  GTK_TREE_VIEW_COLUMN_AUTOSIZE );
	gtk_tree_view_column_set_alignment ( column,
					     alignment[i] );
	gtk_tree_view_column_pack_start ( column, cell, TRUE );
	gtk_tree_view_column_set_title ( column, titles[i] );
	gtk_tree_view_column_set_attributes (column, cell,
					     "text", i,
					     NULL);
	gtk_tree_view_column_set_expand ( column, TRUE );
	gtk_tree_view_column_set_resizable ( column,
					     TRUE );
	gtk_tree_view_append_column ( GTK_TREE_VIEW(treeview_transactions_to_link), column);
    }

    /* set the button to associate */
    hbox = gtk_hbox_new (TRUE, 0);
    gtk_box_pack_start ( GTK_BOX (page),
			 hbox,
			 FALSE, FALSE,
			 10 );
    button = gtk_button_new_with_label ( _("Link the selection to a reconcile..."));
    g_signal_connect ( G_OBJECT (button),
		       "clicked",
		       G_CALLBACK (gsb_assistant_reconcile_config_lauch_manu_asso),
		       assistant );
    gtk_box_pack_start ( GTK_BOX (hbox),
			 button,
			 TRUE, TRUE,
			 0 );

    gtk_widget_show_all (page);
    return page;
}

/**
 * create the page showed when all the transactions are associated to a reconcile
 *
 * \param
 *
 * \return a GtkWidget, the page to the assistant
 * */
static GtkWidget *gsb_assistant_reconcile_config_page_success ( void )
{
    GtkWidget *page;
    GtkWidget *label;

    page = gtk_vbox_new ( FALSE, 0 );
    gtk_container_set_border_width ( GTK_CONTAINER(page), 12 );

    label = gtk_label_new (_("Congratulation !\n\n"
			     "You have linked all the transactions without reconcile.\n"
			     "Normally you needn't have to come back to that assistant ; it shouldn't have any way now\n"
			     "	to have some transactions without reconciliation number."));
    gtk_box_pack_start ( GTK_BOX (page),
			 label,
			 FALSE, FALSE,
			 0 );

    gtk_widget_show_all (page);
    return page;
}




/**
 * callback called when there is a change in the menu
 * change the next page to show according to the menu
 *
 * \param button
 * \param assistant
 *
 * \return FALSE
 * */
static gboolean gsb_assistant_reconcile_config_page_menu_toggled ( GtkWidget *button,
                        GtkWidget *assistant )
{
    gint new_next_page;

    new_next_page = GPOINTER_TO_INT ( g_object_get_data (G_OBJECT (button),
							 "next_page"));
    gsb_assistant_set_next ( assistant,
			     RECONCILE_ASSISTANT_MENU,
			     new_next_page );

    return FALSE;
}


/**
 * called when the user click on the button 'create the new reconcile'
 * check the entries and create the corresponding reconcile
 *
 * \param button
 * \param label a message label, hidden at the beginning, that will say ok the reconcile is created
 * 		or if there is a problem...
 *
 * \return FALSE
 * */
static gboolean gsb_assistant_reconcile_config_page_add_new_reconcile ( GtkWidget *button,
                        GtkWidget *label )
{
    gint reconcile_number;
    gchar *string;

    /* first, we check the date are valid */
    if ( !gsb_date_check_entry ( reconcile_init_date_entry ) )
    {
        string = make_red ( _("The initial date is not valid, please check it.") );
        gtk_label_set_markup ( GTK_LABEL ( label) , string );
        gtk_widget_grab_focus (reconcile_init_date_entry);
        g_free ( string );
        return FALSE;
    }

    if ( !gsb_date_check_entry ( reconcile_final_date_entry ) )
    {
        string = make_red ( _("The final date is not valid, please check it.") );
        gtk_label_set_markup ( GTK_LABEL ( label) , string );
        gtk_widget_grab_focus ( reconcile_final_date_entry );
        g_free ( string );
    return FALSE;
    }

    /* check there is a name */
    if ( !strlen (gtk_entry_get_text ( GTK_ENTRY ( reconcile_name_entry ) ) ) )
    {
        string = make_red ( _("Please give a name to the new reconcile.") );
        gtk_label_set_markup ( GTK_LABEL ( label) , string );
        gtk_widget_grab_focus ( reconcile_name_entry );
        g_free ( string );
        return FALSE;
    }

    /* check if already exist the name */
    if ( gsb_data_reconcile_get_number_by_name (
     gtk_entry_get_text (GTK_ENTRY (reconcile_name_entry) ) ) )
    {
        string = make_red ( _("That name already exists, please find another one.") );
        gtk_label_set_markup ( GTK_LABEL ( label) , string );
        gtk_widget_grab_focus ( reconcile_name_entry );
        g_free ( string );
        return FALSE;
    }

    /* ok, now we can create the reconcile */
    reconcile_number = gsb_data_reconcile_new (
                        gtk_entry_get_text ( GTK_ENTRY ( reconcile_name_entry ) ) );
    if ( !reconcile_number )
    if (gsb_data_reconcile_get_number_by_name ( gtk_entry_get_text (
     GTK_ENTRY ( reconcile_name_entry ) ) ) )
    {
        string = make_red ( _("Cannot allocate memory : Bad things will happen soon.") );
        gtk_label_set_markup ( GTK_LABEL ( label) , string );
        gtk_widget_grab_focus ( reconcile_name_entry );
        g_free ( string );
        return FALSE;
    }

    gsb_data_reconcile_set_init_date ( reconcile_number,
                        gsb_calendar_entry_get_date ( reconcile_init_date_entry ) );
    gsb_data_reconcile_set_final_date ( reconcile_number,
                        gsb_calendar_entry_get_date ( reconcile_final_date_entry ) );
    gsb_data_reconcile_set_init_balance ( reconcile_number,
                        gsb_real_get_from_string ( gtk_entry_get_text (
                        GTK_ENTRY (reconcile_init_balance_entry ) ) ) );
    gsb_data_reconcile_set_final_balance ( reconcile_number,
                        gsb_real_get_from_string ( gtk_entry_get_text (
                        GTK_ENTRY ( reconcile_final_balance_entry ) ) ) );
    gsb_data_reconcile_set_account ( reconcile_number,
                        gsb_account_get_combo_account_number ( reconcile_account_button ) );

    /* erase the entries but not the account wich can be used again */
    gtk_entry_set_text ( GTK_ENTRY (reconcile_name_entry), "" );
    gtk_entry_set_text ( GTK_ENTRY (reconcile_init_date_entry), "" );
    gtk_entry_set_text ( GTK_ENTRY (reconcile_final_date_entry), "" );
    gtk_entry_set_text ( GTK_ENTRY (reconcile_init_balance_entry), "" );
    gtk_entry_set_text ( GTK_ENTRY (reconcile_final_balance_entry), "" );

    string = make_blue ( g_strdup_printf ( _("Reconciliation %s successfully appended !"),
                        gsb_data_reconcile_get_name ( reconcile_number ) ) );
    gtk_label_set_markup ( GTK_LABEL ( label ), string );
    g_free ( string );

    /* update the list of reconcile in the configuration list */
    gsb_reconcile_config_fill ( );

    gtk_widget_grab_focus ( reconcile_name_entry );

    return FALSE;
}

/**
 * hide the error/success label when changing something in the entries
 *
 * \param editable
 * \param label
 *
 * \return FALSE
 * */
static gboolean gsb_assistant_reconcile_config_hide_label_error ( GtkWidget *editable,
                        GtkWidget *label )
{
    gtk_label_set_text ( GTK_LABEL (label),
			 NULL );
    return FALSE;
}


/**
 * function called when the user come to the automatically association page
 * fill the label and show the button if possible
 *
 * \param assistant
 * \param new_page
 *
 * \return FALSE
 * */
gboolean gsb_assistant_reconcile_config_update_auto_asso ( GtkWidget *assistant,
                        gint new_page )
{
    gchar *string;
    GSList *tmp_list;
    gint associate_number;

    /* update the string containing the number of transactions to link */
    string = g_strdup_printf (_("Still %d transactions to link with a reconciliation."),
			      transactions_to_link);

    gtk_label_set_text ( GTK_LABEL (label_transactions_to_link_2),
			 string);
    g_free (string);
    gtk_misc_set_alignment ( GTK_MISC (label_transactions_to_link_2),
			     0, 0.5 );

    /* calculate how many transactions can be associated automatically,
     * to avoid to do that 2 times, we set each transactions in a structure with
     * the associated number of reconcile */
    if (list_association)
    {
	g_slist_free (list_association);
	list_association = NULL;
    }

    tmp_list = gsb_data_transaction_get_transactions_list ();
    while (tmp_list)
    {
	gint transaction_number;

	transaction_number = gsb_data_transaction_get_transaction_number (tmp_list -> data);

	if (gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_RAPPROCHEE
	    &&
	    !gsb_data_transaction_get_reconcile_number (transaction_number))
	{
	    /* ok we are on a marked R transaction without reconcile number,
	     * we search for that reconcile */
	    gint reconcile_number;

	    reconcile_number = gsb_data_reconcile_get_number_by_date (gsb_data_transaction_get_date (transaction_number),
								      gsb_data_transaction_get_account_number (transaction_number));
	    if (reconcile_number)
	    {
		struct association_transaction_reconcile *association;

		association = g_malloc0 (sizeof (struct association_transaction_reconcile));
		if (!association)
		{
		    dialogue_error_memory ();
		    return FALSE;
		}
		association -> transaction_number = transaction_number;
		association -> reconcile_number = reconcile_number;
		list_association = g_slist_append ( list_association,
						    association );
	    }
	}
	tmp_list = tmp_list -> next;
    }

    /* set the number of possible links */
    associate_number = g_slist_length (list_association);

    if (associate_number)
    {
	string = g_strdup_printf (_("Grisbi can associate %d transactions to a reconcile.\n"
				    "Please click on the launch button to create the links"),
				  associate_number);
	gtk_widget_set_sensitive ( button_run_association,
				   TRUE );
    }
    else
    {
	string = my_strdup (_("There is no transaction that grisbi can link.\n"
			      "Check if you created all the necesssary reconciles."));
	gtk_widget_set_sensitive ( button_run_association,
				   FALSE );
    }
    gtk_label_set_text ( GTK_LABEL (label_possible_association),
			 string);
    g_free (string);

    return FALSE;
}


/**
 * callback called by the button to launch the automatic association
 * between transactions and reconcile
 *
 * \param button
 * \param assistant
 *
 * \return FALSE
 * */
static gboolean gsb_assistant_reconcile_config_lauch_auto_asso ( GtkWidget *button,
                        GtkWidget *assistant )
{
    GSList *tmp_list;

    /* we will use the list already created */
    tmp_list = list_association;
    while (tmp_list)
    {
	struct association_transaction_reconcile *association;

	association = tmp_list -> data;

	gsb_data_transaction_set_reconcile_number ( association -> transaction_number,
						    association -> reconcile_number );
	tmp_list = tmp_list -> next;
    }

    transactions_to_link = transactions_to_link - g_slist_length (list_association);
    g_slist_free (list_association);
    list_association = NULL;

    /* now there is 2 way :
     * either transactions_to_link is 0, we go directly to the succes page
     * either it's not null, and the user should create more reconciles */
    if (transactions_to_link)
    {
	gchar *string;

	/* update the labels */
	string = g_strdup_printf (_("Still %d transactions to link with a reconciliation."),
				  transactions_to_link);
	gtk_label_set_text ( GTK_LABEL (label_transactions_to_link_2),
			     string);
	g_free (string);
	gtk_misc_set_alignment ( GTK_MISC (label_transactions_to_link_2),
				 0, 0.5 );

	gtk_label_set_text ( GTK_LABEL (label_possible_association),
			     _("There is no transaction that grisbi can link.\n"
			       "Check if you created all the necesssary reconciles."));
	gtk_widget_set_sensitive ( button_run_association,
				   FALSE );
    }
    else
    {
	/* go to the success page */
	gsb_assistant_set_next ( assistant,
				 RECONCILE_ASSISTANT_AUTOMATICALLY_ASSOCIATE,
				 RECONCILE_ASSISTANT_SUCCESS );
	gsb_assistant_next_page (assistant);
    }
    return FALSE;
}

/**
 * function called when the user come to the manually association page
 * update the list of transactions to associate and fill the labels
 *
 * \param assistant
 * \param new_page
 *
 * \return FALSE
 * */
gboolean gsb_assistant_reconcile_config_update_manu_asso ( GtkWidget *assistant,
                        gint new_page )
{
    gchar *string;
    GSList *tmp_list;
    gint transaction_number;
    GtkListStore *store;

    /* update the string containing the number of transactions to link */
    string = g_strdup_printf (_("Still %d transactions to link with a reconciliation."),
			      transactions_to_link);
    gtk_label_set_text ( GTK_LABEL (label_transactions_to_link_3),
			 string);
    g_free (string);
    gtk_misc_set_alignment ( GTK_MISC (label_transactions_to_link_3),
			     0, 0.5 );

    /* fill the list with the transactions to link */
    store = GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (treeview_transactions_to_link)));
    gtk_list_store_clear (GTK_LIST_STORE (store));

    tmp_list = gsb_data_transaction_get_transactions_list ();
    while (tmp_list)
    {
	transaction_number = gsb_data_transaction_get_transaction_number (tmp_list -> data);

	if ( gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_RAPPROCHEE
	     &&
	     !gsb_data_transaction_get_reconcile_number (transaction_number))
	{
	    gchar *amount_str;
	    gchar *date_str;
	    GtkTreeIter iter;

	    date_str = gsb_format_gdate (gsb_data_transaction_get_date (transaction_number));
	    amount_str = gsb_real_get_string (gsb_data_transaction_get_amount (transaction_number));

	    gtk_list_store_append ( GTK_LIST_STORE (store),
				    &iter );
	    gtk_list_store_set ( GTK_LIST_STORE (store),
				 &iter,
				 TRANSACTION_DATE, date_str,
				 TRANSACTION_PAYEE, gsb_data_payee_get_name (gsb_data_transaction_get_party_number (transaction_number), TRUE),
				 TRANSACTION_AMOUNT, amount_str,
				 TRANSACTION_ACCOUNT, gsb_data_account_get_name (gsb_data_transaction_get_account_number (transaction_number)),
				 TRANSACTION_NUMBER, transaction_number,
				 -1 );
	    g_free (amount_str);
	    g_free (date_str);
	}
	tmp_list = tmp_list -> next;
    }
    return FALSE;
}


/**
 * callback called by the button to launch the automatic association
 * between transactions and reconcile
 *
 * \param button
 * \param assistant
 *
 * \return FALSE
 * */
static gboolean gsb_assistant_reconcile_config_lauch_manu_asso ( GtkWidget *button,
                        GtkWidget *assistant )
{
    GList *tmp_list;
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    GList *path_list;
    gint account_number = -1;
    GtkWidget *dialog;
    GtkWidget *label;
    GtkWidget *scrolled_window;
    GtkWidget *dialog_tree_view;
    GtkListStore *dialog_store;
    gint return_value;
    gint i;
    enum dialog_column {
	DIALOG_NAME = 0,
	DIALOG_INIT_DATE,
	DIALOG_FINAL_DATE,
	DIALOG_RECONCILE_NUMBER,
	DIALOG_NB_COL
    };
    gint selected_reconcile_number;
    gint transaction_number;

    /* get the selection */
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview_transactions_to_link));

    /* get the selected transactions */
    path_list = gtk_tree_selection_get_selected_rows ( GTK_TREE_SELECTION (selection),
						       &model );
    if (!path_list)
	return FALSE;

    /* ok, we have a selection, before continuing,
     * we check that all the transactions are on the same account */
    tmp_list = path_list;
    while (tmp_list)
    {
	GtkTreePath *path;

	path = tmp_list -> data;

	if (gtk_tree_model_get_iter ( GTK_TREE_MODEL (model),
				      &iter,
				      path ))
	{
	    gtk_tree_model_get ( GTK_TREE_MODEL (model),
				 &iter,
				 TRANSACTION_NUMBER, &transaction_number,
				 -1 );
	    if (account_number == -1)
		account_number = gsb_data_transaction_get_account_number (transaction_number);
	    else
	    {
		if (gsb_data_transaction_get_account_number (transaction_number) != account_number)
		{
		    dialogue_error (_("All the selected transactions have to belong to the same account !"));
		    /* erase the path_list */
		    g_list_foreach (path_list, (GFunc) gtk_tree_path_free, NULL);
		    g_list_free (path_list);
		    return FALSE;
		}
	    }
	}
	tmp_list = tmp_list -> next;
    }

    if (account_number == -1)
    {
	/* erase the path_list */
	g_list_foreach (path_list, (GFunc) gtk_tree_path_free, NULL);
	g_list_free (path_list);
	return FALSE;
    }

    /* ok, all the transactions belong to the same account, we can
     * show a dialog to select the reconcile */
    dialog = gtk_dialog_new_with_buttons ( _("Selection of a reconciliation"),
					   GTK_WINDOW ( assistant ),
					   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					   GTK_STOCK_OK, GTK_RESPONSE_OK,
					   NULL );

    gtk_window_set_default_size ( GTK_WINDOW ( dialog ), 770, 412 );
    gtk_window_set_position ( GTK_WINDOW ( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );
    gtk_window_set_resizable ( GTK_WINDOW ( dialog ), TRUE );
    gtk_container_set_border_width ( GTK_CONTAINER ( dialog ), 12 );

    label = gtk_label_new ( _("Select the reconciliation to associate to the selected transactions: ") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.0 );
    gtk_box_pack_start ( GTK_BOX (GTK_DIALOG (dialog) -> vbox),
			 label,
			 FALSE, FALSE,
			 10 );

    /* make the list */
    scrolled_window = gtk_scrolled_window_new (FALSE, FALSE);
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW (scrolled_window),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX (GTK_DIALOG (dialog) -> vbox),
			 scrolled_window,
			 TRUE, TRUE,
			 0 );

    dialog_store = gtk_list_store_new ( DIALOG_NB_COL,
					G_TYPE_STRING,
					G_TYPE_STRING,
					G_TYPE_STRING,
					G_TYPE_INT );
    dialog_tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (dialog_store));
    g_object_unref (G_OBJECT(dialog_store));
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (dialog_tree_view), TRUE);
    gtk_container_add ( GTK_CONTAINER (scrolled_window),
			dialog_tree_view );

    /* set the columns */
    for (i=DIALOG_NAME ; i<DIALOG_RECONCILE_NUMBER ; i++)
    {
	GtkTreeViewColumn *column;
	GtkCellRenderer *cell;
	gchar *titles[] = {
	    _("Name"), _("Initial date"), _("Final date")
	};
	gfloat alignment[] = {
	    COLUMN_LEFT, COLUMN_CENTER, COLUMN_CENTER
	};

	cell = gtk_cell_renderer_text_new ();
	g_object_set ( G_OBJECT (cell),
		       "xalign", alignment[i],
		       NULL );
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_sizing ( column,
					  GTK_TREE_VIEW_COLUMN_AUTOSIZE );
	gtk_tree_view_column_set_alignment ( column,
					     alignment[i] );
	gtk_tree_view_column_pack_start ( column, cell, TRUE );
	gtk_tree_view_column_set_title ( column, titles[i] );
	gtk_tree_view_column_set_attributes (column, cell,
					     "text", i,
					     NULL);
	gtk_tree_view_column_set_expand ( column, TRUE );
	gtk_tree_view_column_set_resizable ( column,
					     TRUE );
	gtk_tree_view_append_column ( GTK_TREE_VIEW(dialog_tree_view), column);
    }

    /* fill the tree view */
    tmp_list = gsb_data_reconcile_get_reconcile_list ();
    while (tmp_list)
    {
	gint reconcile_number;

	reconcile_number = gsb_data_reconcile_get_no_reconcile (tmp_list -> data);

	if (gsb_data_reconcile_get_account (reconcile_number) == account_number)
	{
	    gchar *init_date_str;
	    gchar *final_date_str;

	    init_date_str = gsb_format_gdate (gsb_data_reconcile_get_init_date (reconcile_number));
	    final_date_str = gsb_format_gdate (gsb_data_reconcile_get_final_date (reconcile_number));

	    gtk_list_store_append ( GTK_LIST_STORE (dialog_store),
				    &iter );
	    gtk_list_store_set ( GTK_LIST_STORE (dialog_store),
				 &iter,
				 DIALOG_NAME, gsb_data_reconcile_get_name (reconcile_number),
				 DIALOG_INIT_DATE, init_date_str,
				 DIALOG_FINAL_DATE, final_date_str,
				 DIALOG_RECONCILE_NUMBER, reconcile_number,
				 -1 );
	    g_free (init_date_str);
	    g_free (final_date_str);
	}
	tmp_list = tmp_list -> next;
    }

    gtk_widget_show_all (dialog);

    /* launch the dialog */
    return_value = gtk_dialog_run (GTK_DIALOG (dialog));

    if (return_value != GTK_RESPONSE_OK)
    {
	gtk_widget_destroy (dialog);
	return FALSE;
    }

    /* we get the selected reconcile */
    if (!gtk_tree_selection_get_selected ( gtk_tree_view_get_selection (GTK_TREE_VIEW (dialog_tree_view)),
					   NULL,
					   &iter ))
    {
	dialogue_warning (_("No selection found, the transactions are not modified."));
	gtk_widget_destroy (dialog);
	return FALSE;
    }

    gtk_tree_model_get ( GTK_TREE_MODEL (dialog_store),
			 &iter,
			 DIALOG_RECONCILE_NUMBER, &selected_reconcile_number,
			 -1 );
    /* ok we have the reconcile number, we can destroy the dialog */
    gtk_widget_destroy (dialog);

    /* and now, fill the selected transactions with that reconcile number */
    tmp_list = g_list_last (path_list);
    while (tmp_list)
    {
	GtkTreePath *path;

	path = tmp_list -> data;

	if (gtk_tree_model_get_iter ( GTK_TREE_MODEL (model),
				      &iter,
				      path ))
	{
	    gtk_tree_model_get ( GTK_TREE_MODEL (model),
				 &iter,
				 TRANSACTION_NUMBER, &transaction_number,
				 -1 );
	    gtk_list_store_remove ( GTK_LIST_STORE (model),
				    &iter );
	    gsb_data_transaction_set_reconcile_number ( transaction_number,
							selected_reconcile_number );
	    transactions_to_link--;
	}
	tmp_list = tmp_list -> prev;
    }

    /* erase the path_list */
    g_list_foreach (path_list, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (path_list);

    /* now there is 2 way :
     * either transactions_to_link is 0, we go directly to the succes page
     * either it's not null, and the user should create more reconciles */
    if (transactions_to_link)
    {
	gchar *string;

	/* update the labels */
	string = g_strdup_printf (_("Still %d transactions to link with a reconciliation."),
				  transactions_to_link);
	gtk_label_set_text ( GTK_LABEL (label_transactions_to_link_1),
			     string);
	gtk_label_set_text ( GTK_LABEL (label_transactions_to_link_3),
			     string);
	g_free (string);
	gtk_widget_grab_focus (treeview_transactions_to_link);
    }
    else
    {
	/* go to the success page */
	gsb_assistant_set_next ( assistant,
				 RECONCILE_ASSISTANT_MANUALLY_ASSOCIATE,
				 RECONCILE_ASSISTANT_SUCCESS );
	gsb_assistant_next_page (assistant);
    }
    return FALSE;
}


