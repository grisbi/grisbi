/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2008 Benjamin Drieu (bdrieu@april.org)	      */
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
 * \file gsb_assistant_account.c
 * we find here the complete assistant to create a new account
 */


#include "include.h"

/*START_INCLUDE*/
#include "gsb_assistant_account.h"
#include "./gsb_account.h"
#include "./gsb_assistant.h"
#include "./gsb_bank.h"
#include "./gsb_currency_config.h"
#include "./gsb_currency.h"
#include "./gsb_real.h"
#include "./utils.h"
#include "./include.h"
#include "./gsb_data_bank.h"
/*END_INCLUDE*/

/*START_STATIC*/
static  GtkWidget *gsb_assistant_account_page_2 ( GtkWidget *assistant );
static  GtkWidget *gsb_assistant_account_page_3 ( GtkWidget *assistant );
static  GtkWidget *gsb_assistant_account_page_finish ( GtkWidget *assistant );
static gboolean gsb_assistant_account_toggled_kind_account ( GtkWidget *button,
						      GtkWidget *assistant );
static gboolean gsb_assistant_account_enter_page_finish ( GtkWidget *assistant, gint new_page );
/*END_STATIC*/

/*START_EXTERN*/
extern GtkTreeSelection * selection ;
/*END_EXTERN*/

enum first_assistant_page
{
    ACCOUNT_ASSISTANT_INTRO= 0,
    ACCOUNT_ASSISTANT_PAGE_2,
    ACCOUNT_ASSISTANT_PAGE_3,
    ACCOUNT_ASSISTANT_PAGE_FINISH,
};


static GtkWidget *account_combobox_currency = NULL;
static GtkWidget *account_combobox_bank = NULL;
static GtkWidget *account_entry_initial_amount = NULL;
static GtkWidget *account_entry_name = NULL;


/**
 * this function is called to launch the account assistant
 * called directly by menu
 *
 * \param
 *
 * \return TRUE on success, FALSE otherwise (i.e. user cancelled).
 */
GtkResponseType gsb_assistant_account_run ( void )
{
    GtkResponseType return_value;
    GtkWidget *assistant;

    assistant = gsb_assistant_new ( _("Create a new account"),
				    _("This assistant will help you to create a new account.\n"
				      "All that you do here can be changed later in the account configuration page." ),
				    NULL,
				    NULL );
    gsb_assistant_add_page ( assistant,
			     gsb_assistant_account_page_2 (assistant),
			     ACCOUNT_ASSISTANT_PAGE_2,
			     ACCOUNT_ASSISTANT_INTRO,
			     ACCOUNT_ASSISTANT_PAGE_3,
			     NULL );
    gsb_assistant_add_page ( assistant,
			     gsb_assistant_account_page_3 (assistant),
			     ACCOUNT_ASSISTANT_PAGE_3,
			     ACCOUNT_ASSISTANT_PAGE_2,
			     ACCOUNT_ASSISTANT_PAGE_FINISH,
			     NULL );
    gsb_assistant_add_page ( assistant,
			     gsb_assistant_account_page_finish (assistant),
			     ACCOUNT_ASSISTANT_PAGE_FINISH,
			     ACCOUNT_ASSISTANT_PAGE_3,
			     0,
			     G_CALLBACK ( gsb_assistant_account_enter_page_finish ) );

    return_value = gsb_assistant_run (assistant);

    gboolean result = FALSE;
    if (return_value == GTK_RESPONSE_APPLY)
    {
	/* Ok, we create the new account */
	gsb_account_new ( GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT (assistant), "account_kind")),
			  gsb_currency_get_currency_from_combobox (account_combobox_currency),
			  gsb_bank_list_get_bank_number (account_combobox_bank),
			  gsb_real_get_from_string (gtk_entry_get_text (GTK_ENTRY (account_entry_initial_amount))),
			  gtk_entry_get_text (GTK_ENTRY (account_entry_name)));
        result = TRUE; /* assistant was not cancelled */
    }

    gtk_widget_destroy (assistant);
    return result;
}



/**
 * create the page 2 of the account assistant
 * this page ask for the kind of account
 *
 * \param assistant the GtkWidget assistant
 *
 * \return a GtkWidget containing the page
 * */
static GtkWidget *gsb_assistant_account_page_2 ( GtkWidget *assistant )
{
    GtkWidget *page;
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *button;
    gint i;
    gchar *account_type[] = {
	_("Bank account\nStandard account with credit card and cheques."),
	_("Cash account\nStandard cash account, to use with a cashier."), /* xxx */
	_("Liabilities account\nxxx"),
	_("Assets account\nxxx"),
	NULL };

    page = gtk_hbox_new (FALSE, 15);
    gtk_container_set_border_width ( GTK_CONTAINER (page),
				     10 );

    vbox = new_vbox_with_title_and_icon ( _("Account type selection"),
					  "bank-account.png" );
    gtk_box_pack_start ( GTK_BOX (page),
			 vbox,
			 TRUE, TRUE, 0 );

    label = gtk_label_new (_("Please select type for this account.\nThe account will be created with default payment methods chosen according to your choice.\n"));
    gtk_misc_set_alignment ( GTK_MISC (label),
			     0, 0.5 );
    gtk_box_pack_start ( GTK_BOX (vbox),
			 label,
			 FALSE, FALSE, 0 );

    /* show the kind of accounts,
     * the current kind will be saved as g_object_set in assistant widget */
    i = 0;
    button = NULL;
    while (account_type[i])
    {
	if (button)
	    button = gtk_radio_button_new_with_label_from_widget ( GTK_RADIO_BUTTON (button),
								   account_type[i]);
	else
	    button = gtk_radio_button_new_with_label ( NULL,
						       account_type[i] );
	g_object_set_data ( G_OBJECT (button),
			    "account_kind", GINT_TO_POINTER (i));
	g_signal_connect ( G_OBJECT (button),
			   "toggled",
			   G_CALLBACK (gsb_assistant_account_toggled_kind_account),
			   G_OBJECT (assistant));
	gtk_box_pack_start ( GTK_BOX (vbox),
			     button,
			     FALSE, FALSE, 0 );
	i++;
    }

    gtk_widget_show_all (page);
    return page;
}


/**
 * create the page 3 of the account assistant
 * this page ask for the currency, the bank and the initial amount
 *
 * \param assistant the GtkWidget assistant
 *
 * \return a GtkWidget containing the page
 * */
static GtkWidget *gsb_assistant_account_page_3 ( GtkWidget *assistant )
{
    struct lconv * conv = localeconv();
    GtkWidget *page;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *button;

    page = gtk_hbox_new (FALSE, 15);
    gtk_container_set_border_width ( GTK_CONTAINER (page),
				     10 );

    vbox = gtk_vbox_new (FALSE, 5);
    gtk_box_pack_start ( GTK_BOX (page),
			 vbox,
			 FALSE, FALSE, 0 );

    /* choose the currency */
    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX (vbox),
			 hbox,
			 FALSE, FALSE, 0 );

    label = gtk_label_new (_("Currency for the account."));
    gtk_box_pack_start ( GTK_BOX (hbox),
			 label,
			 FALSE, FALSE, 0 );
    

    /* make it pretty ? (benj your job ;-) )*/
    label = gtk_label_new (NULL);
    gtk_box_pack_start ( GTK_BOX (hbox),
			 label,
			 FALSE, FALSE, 0 );

    /* Guesstimate default currency from locale.  Default is USD since
     * this would confuse US folks while rest of the world is used to
     * configure stuff to their locale.  */
    if ( ! gsb_currency_config_create_currency_from_iso4217list ( conv -> int_curr_symbol ) )
    {
	gsb_currency_config_create_currency_from_iso4217list ( "USD" );
    }

    /* create the currency combobox */
    account_combobox_currency = gsb_currency_make_combobox (TRUE);
    g_signal_connect ( G_OBJECT (account_combobox_currency ), "destroy",
    		G_CALLBACK ( gtk_widget_destroyed), &account_combobox_currency );
    gtk_box_pack_start ( GTK_BOX (hbox),
			 account_combobox_currency,
			 FALSE, FALSE, 0 );

    /* propose to add a currency */
    button = gtk_button_new_with_label (_("Add/Change..."));
    g_signal_connect ( G_OBJECT (button),
		       "clicked",
		       G_CALLBACK (gsb_currency_config_add_currency_set_combobox),
		       account_combobox_currency );
    gtk_box_pack_start ( GTK_BOX (hbox),
			 button,
			 FALSE, FALSE, 0 );

    /* choose the bank */
    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX (vbox),
			 hbox,
			 FALSE, FALSE, 0 );

    label = gtk_label_new (_("Bank for the account."));
    gtk_box_pack_start ( GTK_BOX (hbox),
			 label,
			 FALSE, FALSE, 0 );
    
    /* make it pretty ? (benj your job ;-) )*/
    label = gtk_label_new (NULL);
    gtk_box_pack_start ( GTK_BOX (hbox),
			 label,
			 FALSE, FALSE, 0 );

    account_combobox_bank = gsb_bank_create_combobox ();
    g_signal_connect ( G_OBJECT (account_combobox_bank ), "destroy",
    		G_CALLBACK ( gtk_widget_destroyed), &account_combobox_bank );
    gsb_bank_list_set_bank ( account_combobox_bank, 0 );
    gtk_box_pack_start ( GTK_BOX (hbox),
			 account_combobox_bank,
			 FALSE, FALSE, 0 );

    /* set the initial amount */
    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX (vbox),
			 hbox,
			 FALSE, FALSE, 0 );

    label = gtk_label_new (_("Opening balance"));
    gtk_box_pack_start ( GTK_BOX (hbox),
			 label,
			 FALSE, FALSE, 0 );
    
    account_entry_initial_amount = gtk_entry_new ();
    g_signal_connect ( G_OBJECT (account_entry_initial_amount ), "destroy",
    		G_CALLBACK ( gtk_widget_destroyed), &account_entry_initial_amount );
    gtk_box_pack_start ( GTK_BOX (hbox),
			 account_entry_initial_amount,
			 FALSE, FALSE, 0 );


    gtk_widget_show_all (page);
    return page;
}



/**
 * create the last page of the account assistant
 * it will ask the name of the new account and propose to create it
 *
 * \param assistant the GtkWidget assistant
 *
 * \return a GtkWidget containing the page
 * */
static GtkWidget *gsb_assistant_account_page_finish ( GtkWidget *assistant )
{
    GtkWidget *page;
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *hbox;

    page = gtk_hbox_new (FALSE, 15);
    gtk_container_set_border_width ( GTK_CONTAINER (page),
				     10 );

    vbox = gtk_vbox_new (FALSE, 5);
    gtk_box_pack_start ( GTK_BOX (page),
			 vbox,
			 FALSE, FALSE, 0 );

    /* set up the menu */
    label = gtk_label_new (_("You are about to validate the new account.\n"
			     "You juste have to enter a name and finish the assistant.\n"
			     "More options are available in the account configuration page once you created it.\n"));
    gtk_misc_set_alignment ( GTK_MISC (label),
			     0.5, 0.5 );
    gtk_box_pack_start ( GTK_BOX (vbox),
			 label,
			 FALSE, FALSE, 0 );

    /* enter the name */
    hbox = gtk_hbox_new ( FALSE, 5);
    gtk_box_pack_start ( GTK_BOX (vbox),
			 hbox,
			 FALSE, FALSE, 0 );

    label = gtk_label_new (_("Please enter the name of the new account : "));
    gtk_box_pack_start ( GTK_BOX (hbox),
			 label,
			 FALSE, FALSE, 0 );

    account_entry_name = gtk_entry_new ();
    g_object_set_data ( G_OBJECT (assistant), "account_entry_name", account_entry_name );
    g_signal_connect ( G_OBJECT (account_entry_name ), "destroy",
    		G_CALLBACK ( gtk_widget_destroyed), &account_entry_name );
    gtk_box_pack_start ( GTK_BOX (hbox),
			 account_entry_name,
			 FALSE, FALSE, 0 );

    gtk_widget_show_all (page);
    return page;
}



/**
 *
 *
 *
 *
 */
static gboolean gsb_assistant_account_enter_page_finish ( GtkWidget * assistant, gint new_page )
{
    GtkWidget * account_entry_name = g_object_get_data ( G_OBJECT (assistant), "account_entry_name" );
    gchar * default_name;
    gint account_type = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT (assistant), "account_kind" ) );

    switch ( account_type )
    {
	case GSB_TYPE_BANK:
	    if ( gsb_bank_list_get_bank_number ( account_combobox_bank ) >= 0 )
	    {
		const gchar * bank_name;

		bank_name = gsb_data_bank_get_name ( gsb_bank_list_get_bank_number ( account_combobox_bank ) );
		if ( bank_name )
		    default_name = g_strdup_printf ( _("%s account"), bank_name );
		else
		    default_name = g_strdup ( _("Bank account" ) );
	    }
	    break;

	case GSB_TYPE_CASH:
	    default_name = g_strdup_printf ( _("Cashier") ); /* xxx */
	    break;

	case GSB_TYPE_ASSET:
	    default_name = g_strdup_printf ( _("xxx asset") ); /* xxx */
	    break;

	case GSB_TYPE_LIABILITIES:
	    default_name = g_strdup_printf ( _("xxx liabilities") ); /* xxx */
	    break;
    }

    gtk_entry_set_text ( GTK_ENTRY ( account_entry_name ), default_name );
    g_free ( default_name );

    return FALSE;
}



/**
 * callback called when toggle the choice of type of account
 * it will save the choice in the assistant widget
 *
 * \param button
 * \param assistant
 *
 * \return FALSE
 * */
gboolean gsb_assistant_account_toggled_kind_account ( GtkWidget *button,
						      GtkWidget *assistant )
{
    g_object_set_data ( G_OBJECT (assistant),
			"account_kind", g_object_get_data ( G_OBJECT (button),
							    "account_kind" ));
    return FALSE;
}
