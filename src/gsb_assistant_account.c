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
 * \file gsb_assistant_account.c
 * we find here the complete assistant to create a new account
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_assistant_account.h"
#include "dialog.h"
#include "gsb_account.h"
#include "gsb_assistant.h"
#include "gsb_bank.h"
#include "gsb_currency_config.h"
#include "gsb_currency.h"
#include "gsb_data_account.h"
#include "gsb_data_bank.h"
#include "gsb_data_currency.h"
#include "gsb_real.h"
#include "gsb_select_icon.h"
#include "utils.h"
#include "structures.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void gsb_assistant_account_change_account_icon ( GtkWidget *button, gpointer data );
static gboolean gsb_assistant_account_enter_page_finish ( GtkWidget * assistant, gint new_page );
static GtkWidget *gsb_assistant_account_page_2 ( GtkWidget *assistant );
static GtkWidget *gsb_assistant_account_page_3 ( GtkWidget *assistant );
static GtkWidget *gsb_assistant_account_page_finish ( GtkWidget *assistant );
static gboolean gsb_assistant_account_toggled_kind_account ( GtkWidget *button,
						      GtkWidget *assistant );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

static gchar * new_icon = NULL;

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
	gboolean result = FALSE;
    GtkResponseType return_value;
    GtkWidget *assistant;

    if ( new_icon && strlen ( new_icon ) > 0)
        g_free ( new_icon );
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

    if (return_value == GTK_RESPONSE_APPLY)
    {
	/* Ok, we create the new account */
	gsb_account_new ( GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT (assistant), "account_kind")),
			  gsb_currency_get_currency_from_combobox (account_combobox_currency),
			  gsb_bank_list_get_bank_number (account_combobox_bank),
			  gsb_real_get_from_string (gtk_entry_get_text (GTK_ENTRY (account_entry_initial_amount))),
			  gtk_entry_get_text (GTK_ENTRY (account_entry_name)),
              new_icon );
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
	_("Cash account\nStandard cash account, to use with a cashier."),
	_("Liabilities account\nSpecial account to represent a debt, like a long-term loan."),
	_("Assets account\nSpecial account to represent an asset, like a car or special subscriptions."),
	NULL };

    page = gtk_hbox_new (FALSE, 15);
    gtk_container_set_border_width ( GTK_CONTAINER (page),
				     10 );

    vbox = new_vbox_with_title_and_icon ( _("Account type selection"),
					  "ac_bank.png" );
    gtk_box_pack_start ( GTK_BOX (page),
			 vbox,
			 TRUE, TRUE, 0 );

    label = gtk_label_new (_("Please select type for this account.\n"
			     "The account will be created with default payment methods chosen according to your choice.\n"
			     "If you are unsure and novice to accounting, we advise that you choose a bank account.\n"));
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
    GtkWidget *page, *label, *button, *table;
    GtkWidget *align;
    GtkWidget *image;
    struct lconv * conv = localeconv();

    page = gtk_hbox_new (FALSE, 15);
    gtk_container_set_border_width ( GTK_CONTAINER (page),
				     10 );

    table = gtk_table_new ( 3, 4, FALSE );
    gtk_table_set_row_spacings ( GTK_TABLE ( table ), 6 );
    gtk_table_set_col_spacings ( GTK_TABLE ( table ), 6 );

    gtk_box_pack_start ( GTK_BOX (page), table, 
			 FALSE, FALSE, 0 );

    /* choose the currency */
    label = gtk_label_new ( _("Currency for the account: ") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.5 );
    gtk_table_attach ( GTK_TABLE ( table ), label, 
		       0, 1, 0, 1,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0, 0 );

    /* Guesstimate default currency from locale.  Default is USD since
     * this would confuse US folks while rest of the world is used to
     * configure stuff to their locale.  */
    if ( ! gsb_data_currency_get_default_currency () &&
	 ! gsb_currency_config_create_currency_from_iso4217list ( conv -> int_curr_symbol ) )
    {
	gsb_currency_config_create_currency_from_iso4217list ( "USD" );
    }

    /* create the currency combobox */
    account_combobox_currency = gsb_currency_make_combobox (TRUE);
    gtk_table_attach ( GTK_TABLE ( table ), account_combobox_currency, 
		       1, 2, 0, 1,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0, 0 );

    /* propose to add a currency */
    button = gtk_button_new_with_label (_("Add/Change..."));
    g_signal_connect ( G_OBJECT (button),
		       "clicked",
		       G_CALLBACK (gsb_currency_config_add_currency_set_combobox),
		       account_combobox_currency );
    gtk_table_attach ( GTK_TABLE ( table ), button, 
		       2, 3, 0, 1,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0, 0 );

    /* choose the bank */
    label = gtk_label_new ( _("Bank for the account: ") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.5 );
    gtk_table_attach ( GTK_TABLE ( table ), label, 
		       0, 1, 1, 2,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0, 0 );
    
    account_combobox_bank = gsb_bank_create_combobox (0, NULL, NULL, NULL, 0);

    if ( gsb_data_bank_max_number() != 0 )
    {
	gsb_bank_list_set_bank ( account_combobox_bank, 1, 0 );
    }
    else
    {
	gsb_bank_list_set_bank ( account_combobox_bank, 0, 0 );
    }
    gtk_table_attach ( GTK_TABLE ( table ), account_combobox_bank, 
		       1, 2, 1, 2,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0, 0 );

    /* set the initial amount */
    label = gtk_label_new ( _("Opening balance: ") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.5 );
    gtk_table_attach ( GTK_TABLE ( table ), label, 
		       0, 1, 2, 3,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0, 0 );
    
    account_entry_initial_amount = gtk_entry_new ();
    gtk_table_attach ( GTK_TABLE ( table ), account_entry_initial_amount, 
		       1, 2, 2, 3,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0, 0 );
    
    /* création du choix de l'icône du compte */
    /* Récupération de l'icône par défaut */
    align = gtk_alignment_new (0.5,0.5,1,1);
    gtk_alignment_set_padding ( GTK_ALIGNMENT ( align ), 0, 0, 20, 20 );
    button = gtk_button_new ( );
    gtk_widget_set_size_request ( button, 80, 80 );
    image = gtk_image_new_from_pixbuf (
                gsb_data_account_get_account_standard_pixbuf ( 0 ) );
    gtk_button_set_image ( GTK_BUTTON ( button ), image);
    gtk_button_set_relief ( GTK_BUTTON ( button ), GTK_RELIEF_NORMAL );
    gtk_container_add (GTK_CONTAINER (align), button);
    gtk_table_attach ( GTK_TABLE ( table ), align, 
		       3, 4, 0, 3,
		       GTK_FILL | GTK_FILL,
		       GTK_FILL | GTK_FILL,
		       0, 0 );
    g_object_set_data ( G_OBJECT (assistant), "bouton_icon", button );
    g_signal_connect ( G_OBJECT( button ), 
                            "clicked", 
                            G_CALLBACK(gsb_assistant_account_change_account_icon), 
                            NULL );

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

    label = gtk_label_new (_("Please enter the name of the new account: "));
    gtk_box_pack_start ( GTK_BOX (hbox),
			 label,
			 FALSE, FALSE, 0 );

    account_entry_name = gtk_entry_new ();
    g_object_set_data ( G_OBJECT (assistant), "account_entry_name", account_entry_name );
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
    gchar * default_name = NULL;
    gint account_type = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT (assistant), "account_kind" ) );

    switch ( account_type )
    {
	case GSB_TYPE_BANK:
	    if ( gsb_bank_list_get_bank_number ( account_combobox_bank ) >= 0 )
	    {
		const gchar * bank_name;

		bank_name = gsb_data_bank_get_name ( gsb_bank_list_get_bank_number ( account_combobox_bank ) );
		/* Do not use a bank name if no bank is set or if its
		 * name has not been changed. */
		if ( bank_name && strcmp ( bank_name, _("New bank") ) )
		    default_name = g_strdup_printf ( _("%s account"), bank_name );
		else
		    default_name = g_strdup ( _("Bank account" ) );
	    }
	    break;

	case GSB_TYPE_CASH:
	    default_name = g_strdup_printf ( _("Cashier") );
	    break;

	case GSB_TYPE_ASSET:
	    default_name = g_strdup_printf ( _("Assets account") );
	    break;

	case GSB_TYPE_LIABILITIES:
	    default_name = g_strdup_printf ( _("Liabilities account"));
	    break;
    }

    gtk_entry_set_text ( GTK_ENTRY ( account_entry_name ), default_name );
    if ( default_name )
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
    GtkWidget *bouton_icon, *image;
    kind_account account_kind;

    account_kind = GPOINTER_TO_INT ( g_object_get_data 
                ( G_OBJECT (button), "account_kind"));
    g_object_set_data ( G_OBJECT (assistant),
                "account_kind", GINT_TO_POINTER ( account_kind ) );

    bouton_icon = g_object_get_data ( G_OBJECT (assistant), "bouton_icon" );
    image = gtk_image_new_from_pixbuf (
                gsb_data_account_get_account_standard_pixbuf ( account_kind ) );
    gtk_button_set_image ( GTK_BUTTON ( bouton_icon ), image);
    return FALSE;
}

void gsb_assistant_account_change_account_icon ( GtkWidget *button, gpointer data )
{
    GdkPixbuf * pixbuf;
    GtkWidget *image;
    gchar * name_icon;
    GError *error = NULL;

    devel_debug ( NULL );
    image = gtk_button_get_image ( GTK_BUTTON ( button ) );
    pixbuf = gtk_image_get_pixbuf ( GTK_IMAGE ( image ) );
    name_icon = g_object_get_data ( G_OBJECT ( pixbuf ), "name_icon" );
    devel_debug (name_icon);
    new_icon = gsb_select_icon_create_window ( name_icon );
    if ( ! new_icon )
        return;
    devel_debug (new_icon);
    pixbuf = gdk_pixbuf_new_from_file_at_size ( new_icon , 32, 32, &error );
    if ( ! pixbuf )
    {
        devel_debug ( error -> message );
        dialogue_error ( error -> message );
        g_error_free ( error );
    }
    else
    {
    image = gtk_image_new_from_pixbuf ( pixbuf );
        if ( image )
            gtk_button_set_image ( GTK_BUTTON ( button ), image );
    }
}
