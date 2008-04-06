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
 * \file gsb_currency.c
 * contains tools to work with the currencies
 */


#include "include.h"

/*START_INCLUDE*/
#include "gsb_currency.h"
#include "./gsb_data_account.h"
#include "./gsb_data_currency.h"
#include "./gsb_data_currency_link.h"
#include "./gsb_data_transaction.h"
#include "./gsb_real.h"
#include "./utils.h"
#include "./structures.h"
#include "./include.h"
#include "./gsb_currency_config.h"
#include "./gsb_real.h"
#include "./utils_files.h"
/*END_INCLUDE*/

/*START_STATIC*/
static struct cached_exchange_rate *gsb_currency_config_get_cached_exchange ( gint currency1_number, 
								       gint currency2_number );
static void gsb_currency_config_set_cached_exchange ( gint currency1_number, 
					       gint currency2_number,
					       gsb_real change, gsb_real fees );
static gboolean gsb_currency_create_combobox_store ( void );
static gboolean gsb_currency_select_change_currency ( GtkWidget *combobox_1,
					       GtkWidget *combobox_2 );
/*END_STATIC*/

/**
 * the currency list store, contains 3 columns :
 * 1 : the code of the currency
 * 2 : the name(code) of the currency
 * 3 : the number of the currency
 * used to be set in the combobox */
static GtkListStore *combobox_currency_store;

enum currency_list_columns {
    CURRENCY_COL_CODE = 0,
    CURRENCY_COL_NAME,
    CURRENCY_COL_NUMBER,
    CURRENCY_COL_FLAG,
    CURRENCY_NUM_COL,		/** Number of columns */
};

/** Exchange rates cache, used by
 * gsb_currency_config_set_cached_exchange
 * and 
 * gsb_currency_config_get_cached_exchange */
static GSList * cached_exchange_rates = NULL;


/**
 * the 2 next variables are filled by gsb_currency_exchange_dialog
 * and permit to the form to get the result by the functions
 * gsb_currency_get_current_exchange
 * gsb_currency_get_current_exchange_fees
 * */
static gsb_real current_exchange;
static gsb_real current_exchange_fees;




/*START_EXTERN*/
extern gint mise_a_jour_liste_comptes_accueil;
extern gint mise_a_jour_liste_echeances_auto_accueil;
extern gint mise_a_jour_liste_echeances_manuelles_accueil;
extern gsb_real null_real ;
extern GtkWidget *window;
/*END_EXTERN*/


/**
 * set to NULL the static variables
 *
 * \param
 *
 * \return
 * */
void gsb_currency_init_variables ( void )
{
    if (combobox_currency_store
	&&
	GTK_IS_LIST_STORE (combobox_currency_store))
	gtk_list_store_clear (combobox_currency_store);

    combobox_currency_store = NULL;
    current_exchange = null_real;
    current_exchange_fees = null_real;
}

/**
 * return the content of the current exchange
 * that variable should have been filed before with gsb_currency_exchange_dialog
 * \param
 *
 * \return a gsb_real, the current exchange
 * */
gsb_real gsb_currency_get_current_exchange (void)
{
    return current_exchange;
}

/**
 * return the content of the current exchange fees 
 * that variable should have been filed before with gsb_currency_exchange_dialog
 *
 * \param
 *
 * \return a gsb_real, the current exchange fees
 * */
gsb_real gsb_currency_get_current_exchange_fees (void)
{
    return current_exchange_fees;
}


/**
 * Format an amount together with an associated currency and return a
 * string according to locale.
 *
 * \param amount	Amount to format.
 * \param currency	Associated currency.
 *
 * \return		Newly-allocated char, to free when unused.
 */
gchar * gsb_format_amount ( gsb_real amount, gint currency )
{
    /** FIXME: use locale instead of hardcoded european-style format */

    gchar* tmpstr = gsb_real_get_string ( gsb_real_adjust_exponent ( amount,
					gsb_data_currency_get_floating_point ( currency ) ) );
    return g_strconcat ( tmpstr,
			 " ",
			 gsb_data_currency_get_code_or_isocode ( currency ),
			 NULL );
}



/**
 * create and return a combobox with the currencies
 *
 * \param set_name if TRUE, the currencies in the combobox will be name(code), else it will be only the code
 *
 * \return a widget combobox or NULL
 * */
GtkWidget *gsb_currency_make_combobox ( gboolean set_name )
{
    GtkCellRenderer *text_renderer, *flag_renderer;
    GtkWidget *combo_box;

    if (!combobox_currency_store)
	gsb_currency_create_combobox_store ();

    combo_box = gtk_combo_box_new_with_model (GTK_TREE_MODEL (combobox_currency_store));

    /* Flag renderer */
    flag_renderer = gtk_cell_renderer_pixbuf_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box), flag_renderer, FALSE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box), flag_renderer,
				    "pixbuf", CURRENCY_COL_FLAG, NULL );
    
    GTK_CELL_RENDERER(flag_renderer) -> xpad = 3; /* Ugly but how to set it otherwise ?*/

    text_renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box), text_renderer, FALSE);

    if (set_name)
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box), text_renderer,
					"text", CURRENCY_COL_NAME,
					NULL);
    else
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box), text_renderer,
					"text", CURRENCY_COL_CODE,
					NULL);

    gtk_combo_box_set_active ( GTK_COMBO_BOX (combo_box),
			       0 );
    return (combo_box);
}



/**
 * set the combobox on the currency given in param
 *
 * \combo_box the combo-box to set
 * \currency_number the currency we want to set on the combo-box
 *
 * \return TRUE currency found, FALSE currency not found, nothing change
 * */
gboolean gsb_currency_set_combobox_history ( GtkWidget *combo_box,
					     gint currency_number )
{
    GtkTreeIter iter;
    gint result;

    if (!combobox_currency_store)
	gsb_currency_create_combobox_store ();

    result = gtk_tree_model_get_iter_first ( GTK_TREE_MODEL (combobox_currency_store),
					     &iter );
    while (result)
    {
	gint value;

	gtk_tree_model_get ( GTK_TREE_MODEL (combobox_currency_store),
			     &iter,
			     CURRENCY_COL_NUMBER, &value,
			     -1 );

	if (value == currency_number)
	{
	    gtk_combo_box_set_active_iter ( GTK_COMBO_BOX (combo_box),
					    &iter );
	    return TRUE;
	}
	result = gtk_tree_model_iter_next ( GTK_TREE_MODEL (combobox_currency_store),
					    &iter );
    }
    return FALSE;
}


/** 
 * Get and return the number of the currency in the option_menu given
 * in param
 * 
 * \param currency_option_menu an option menu with the currencies
 * 
 * \return the number of currency
 * */
gint gsb_currency_get_currency_from_combobox ( GtkWidget *combo_box )
{
    gint currency_number;
    GtkTreeIter iter;

    if (!combobox_currency_store)
	gsb_currency_create_combobox_store ();

    gtk_combo_box_get_active_iter ( GTK_COMBO_BOX (combo_box),
				    &iter );
    gtk_tree_model_get ( GTK_TREE_MODEL (combobox_currency_store),
			 &iter,
			 CURRENCY_COL_NUMBER, &currency_number,
			 -1 );
    return currency_number;
}


/**
 * update the list of the currencies for combobox, wich change all
 * the current combobox content
 *
 * \param
 *
 * \return FALSE
 */
gboolean gsb_currency_update_combobox_currency_list ( void )
{
    GSList *list_tmp;

    if (!combobox_currency_store
	||
	!gsb_data_currency_get_currency_list ())
	return FALSE;

    gtk_list_store_clear (GTK_LIST_STORE (combobox_currency_store));
    list_tmp = gsb_data_currency_get_currency_list ();

    while ( list_tmp )
    {
	GtkTreeIter iter;
	GdkPixbuf * pixbuf;
	gchar * string;
	gint currency_number;

	currency_number = gsb_data_currency_get_no_currency (list_tmp -> data);

	string = g_strconcat( PIXMAPS_DIR, C_DIRECTORY_SEPARATOR,
			      "flags", C_DIRECTORY_SEPARATOR,
			      gsb_data_currency_get_code_iso4217 (currency_number),
			      ".png", NULL );
	pixbuf = gdk_pixbuf_new_from_file ( string, NULL );	
	g_free (string);


	gtk_list_store_append ( GTK_LIST_STORE (combobox_currency_store), &iter );
	gchar* tmpstr = g_strconcat ( gsb_data_currency_get_name (currency_number),
				" (",
				gsb_data_currency_get_code_or_isocode (currency_number),
				")",
				NULL );
	gtk_list_store_set ( combobox_currency_store, &iter,
			     CURRENCY_COL_FLAG, pixbuf,
			     CURRENCY_COL_CODE, gsb_data_currency_get_code_or_isocode (currency_number),
			     CURRENCY_COL_NAME, tmpstr,
			     CURRENCY_COL_NUMBER, currency_number,
			     -1 );
	g_free ( tmpstr );
	list_tmp = list_tmp -> next;
    }

    mise_a_jour_liste_comptes_accueil = 1;
    mise_a_jour_liste_echeances_manuelles_accueil = 1;
    mise_a_jour_liste_echeances_auto_accueil = 1;

    return FALSE;
}



/** 
 * Check if a transaction need an exchange rate and fees with its
 * account
 * if yes, ask for that and set the in the transaction.
 * 
 * \param transaction_number
 */
void gsb_currency_check_for_change ( gint transaction_number )
{
    gint transaction_currency_number;
    gint account_currency_number;

    account_currency_number = gsb_data_account_get_currency (gsb_data_transaction_get_account_number (transaction_number));
    transaction_currency_number = gsb_data_transaction_get_currency_number (transaction_number);

    if ( !gsb_data_currency_link_search ( account_currency_number,
					  transaction_currency_number ))
    {
	/* it's a foreign currency, ask for the exchange rate and fees */

	gsb_currency_exchange_dialog ( account_currency_number,
				       transaction_currency_number,
				       1,
				       null_real,
				       null_real,
				       FALSE );

	gsb_data_transaction_set_exchange_rate ( transaction_number,
						 gsb_real_abs (current_exchange));
	gsb_data_transaction_set_exchange_fees ( transaction_number,
						 current_exchange_fees);
	if ( current_exchange.mantissa < 0 )
	    gsb_data_transaction_set_change_between ( transaction_number,
						      1 );
    }
}


/**
 * ask to the user the exchange and the fees for a transaction
 * fill the variables current_exchange and current_exchange_fees with
 * the data of the user
 * if this was asked before, will use the buffer cached exchange rate, except
 * if force is set to TRUE
 *
 * \param account_currency_number
 * \param transaction_currency_number
 * \param link_currency si = 1 : 1 nom_devise = "change" devise_en_rapport 
 * \param exchange_rate
 * \param exchange_fees
 * \param force if TRUE will not get the cached exchange rate and will really ask to the user
 *
 * \return
 * */
void gsb_currency_exchange_dialog ( gint account_currency_number,
				    gint transaction_currency_number ,
				    gint link_currency,
				    gsb_real exchange_rate,
				    gsb_real exchange_fees,
				    gboolean force )
{
    GtkWidget *dialog, *label, *entry, *hbox, *fees_entry, *paddingbox;
    struct cached_exchange_rate *cache;
    gint result;
    GtkWidget *combobox_1;
    GtkWidget *combobox_2;


    if ( !force
	 && 
	 (cache = gsb_currency_config_get_cached_exchange ( account_currency_number, transaction_currency_number )) )
    {
	current_exchange = cache -> rate;
	current_exchange_fees = cache -> fees;
	return;
    }

    dialog = gtk_dialog_new_with_buttons ( _("Enter exchange rate"),
					   GTK_WINDOW (window),
					   GTK_DIALOG_MODAL,
					   GTK_STOCK_OK, 0,
					   NULL );

    gchar* tmpstr = g_strdup_printf( _("Please enter exchange rate for %s"), 
						      gsb_data_currency_get_name (transaction_currency_number));
    paddingbox = new_paddingbox_with_title ( GTK_WIDGET ( GTK_DIALOG ( dialog ) -> vbox ), FALSE, 
					     tmpstr );
    g_free ( tmpstr );
    gtk_container_set_border_width ( GTK_CONTAINER ( paddingbox ), 6 );
    gtk_box_set_spacing ( GTK_BOX ( GTK_DIALOG (dialog)->vbox ), 6 );

    /* exchange line */
    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
			 FALSE, FALSE, 0);

    label = gtk_label_new ( POSTSPACIFY(_("A")) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label,
			 FALSE, FALSE, 0);

    combobox_1 = gtk_combo_box_new_text ();
    gtk_combo_box_append_text ( GTK_COMBO_BOX (combobox_1),
				gsb_data_currency_get_name (transaction_currency_number));
    gtk_combo_box_append_text ( GTK_COMBO_BOX (combobox_1),
				gsb_data_currency_get_name (account_currency_number));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 combobox_1,
			 TRUE, TRUE, 0);

    label = gtk_label_new ( SPACIFY(_("equals")) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label,
			 FALSE, FALSE, 0);

    entry = gtk_entry_new ();
    gtk_widget_set_usize ( entry, 100, FALSE );
    gtk_entry_set_activates_default ( GTK_ENTRY ( entry ),
				      TRUE );
    gtk_box_pack_start ( GTK_BOX ( hbox ), entry,
			 TRUE, TRUE, 0);

    combobox_2 = gtk_combo_box_new_text ();
    gtk_combo_box_append_text ( GTK_COMBO_BOX (combobox_2),
				gsb_data_currency_get_name (account_currency_number));
    gtk_combo_box_append_text ( GTK_COMBO_BOX (combobox_2),
				gsb_data_currency_get_name (transaction_currency_number));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 combobox_2,
			 TRUE, TRUE, 0);

    /* set the connections */
    g_signal_connect ( G_OBJECT (combobox_1),
		       "changed",
		       GTK_SIGNAL_FUNC ( gsb_currency_select_change_currency ),
		       combobox_2 );
    g_signal_connect ( G_OBJECT (combobox_2),
		       "changed",
		       GTK_SIGNAL_FUNC ( gsb_currency_select_change_currency ),
		       combobox_1);

    /* exchange fees line */
    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
			 FALSE, FALSE, 5 );

    label = gtk_label_new ( COLON(_("Exchange fees")) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label,
			 FALSE, FALSE, 5 );

    fees_entry = gtk_entry_new ();
    gtk_entry_set_activates_default ( GTK_ENTRY ( fees_entry ),
				      TRUE );
    gtk_box_pack_start ( GTK_BOX ( hbox ), fees_entry,
			 TRUE, TRUE, 5 );

    label = gtk_label_new (gsb_data_currency_get_name (account_currency_number));
    gtk_box_pack_start ( GTK_BOX ( hbox ), label,
			 FALSE, FALSE, 5 );

    gtk_widget_show_all ( dialog );

    /* if the rate or fees exist already, fill them here */
    if ( exchange_rate.mantissa || exchange_fees.mantissa )
    {
	if ( link_currency )
	{
	    gtk_combo_box_set_active ( GTK_COMBO_BOX (combobox_1),
				       link_currency );
	    gtk_combo_box_set_active ( GTK_COMBO_BOX (combobox_2),
				       link_currency );
	}
	else
	{
	    gtk_combo_box_set_active ( GTK_COMBO_BOX (combobox_1),
				       !link_currency );
	    gtk_combo_box_set_active ( GTK_COMBO_BOX (combobox_2),
				       !link_currency );
	}
	gchar* tmpstr = gsb_real_get_string (exchange_rate);
	gtk_entry_set_text ( GTK_ENTRY ( entry ), tmpstr);
	g_free ( tmpstr );

	tmpstr = gsb_real_get_string (gsb_real_abs (exchange_rate));
	gtk_entry_set_text ( GTK_ENTRY ( fees_entry ), tmpstr);
	g_free ( tmpstr );
    }
    else
    {
	gtk_combo_box_set_active ( GTK_COMBO_BOX (combobox_1),
				   0 );
	gtk_combo_box_set_active ( GTK_COMBO_BOX (combobox_2),
				   0 );
    }

    /* show the dialog */
    result = gtk_dialog_run ( GTK_DIALOG ( dialog ));

    if ( result )
    {
	current_exchange = null_real;
	current_exchange_fees = null_real;
    }
    else
    {
	gint tmp_currency_number;

	current_exchange = gsb_real_get_from_string (gtk_entry_get_text ( GTK_ENTRY ( entry )));
	current_exchange_fees = gsb_real_get_from_string (gtk_entry_get_text ( GTK_ENTRY ( fees_entry )));

	tmp_currency_number = gsb_data_currency_get_number_by_name (gtk_combo_box_get_active_text (GTK_COMBO_BOX (combobox_1)));

	if ( tmp_currency_number != transaction_currency_number )
	    current_exchange = gsb_real_opposite (current_exchange);

	gsb_currency_config_set_cached_exchange ( account_currency_number, transaction_currency_number, 
						  current_exchange, current_exchange_fees );
    }
    gtk_widget_destroy ( GTK_WIDGET ( dialog ));
}

/**
 * Find whether echange rate between two currencies is known.  If so,
 * returns a cached_exchange_rate structure with exchange rate
 * information.
 *
 * \param currency1 First currency
 * \param currency2 Second currency
 *
 * \return NULL on failure, a pointer to a cached_exchange_rate
 * structure on success.
 */
struct cached_exchange_rate *gsb_currency_config_get_cached_exchange ( gint currency1_number, 
								       gint currency2_number )
{
    GSList * tmp_list = cached_exchange_rates;

    while ( tmp_list )
    {
	struct cached_exchange_rate * tmp;

	tmp = tmp_list -> data;
	if ( currency1_number == tmp -> currency1_number && currency2_number == tmp -> currency2_number )
	    return tmp;

	tmp_list = tmp_list -> next;
    }
    return NULL;
}


/**
 * Update exchange rate cache according to arguments.
 *
 * \param currency1 First currency.
 * \param currency2 Second currency.
 * \param change    Exchange rate between two currencies.
 * \param fees      Fees of transaction.
 */
void gsb_currency_config_set_cached_exchange ( gint currency1_number, 
					       gint currency2_number,
					       gsb_real change, gsb_real fees )
{
    struct cached_exchange_rate * tmp;

    tmp = (struct cached_exchange_rate *) g_malloc0(sizeof(struct cached_exchange_rate));

    tmp -> currency1_number = currency1_number;
    tmp -> currency2_number = currency2_number;
    tmp -> rate = change;
    tmp -> fees = fees;

    cached_exchange_rates = g_slist_append ( cached_exchange_rates, tmp );
}




/* 
 * Handler that change the second combobox of a window that ask for
 * change.
 *
 * \param combobox_1 the combobox wich receive the signal
 * \param combobox_2 the combobox we want to change
 *
 * \return FALSE
 */
gboolean gsb_currency_select_change_currency ( GtkWidget *combobox_1,
					       GtkWidget *combobox_2 )
{
    /* we just need to set the same active menu on the second combobox */

    /*     g_signal_handlers_block_by_func ( G_OBJECT(combobox_2), */
    /* 				      G_CALLBACK (gsb_currency_select_change_currency),  */
    /* 				      combobox_1 ); */
    gtk_combo_box_set_active ( GTK_COMBO_BOX (combobox_2),
			       gtk_combo_box_get_active ( GTK_COMBO_BOX (combobox_1)));
    /*     g_signal_handlers_unblock_by_func ( G_OBJECT(combobox_2), */
    /* 					G_CALLBACK (gsb_currency_select_change_currency),  */
    /* 					combobox_1 ); */

    return FALSE;
}


/**
 * create and fill the list store of the currency
 * come here mean that combobox_currency_store is NULL
 *
 * \param
 *
 * \return TRUE ok, FALSE problem
 * */
gboolean gsb_currency_create_combobox_store ( void )
{
    /* the currency list store, contains 3 columns :
     * CURRENCY_COL_CODE : the code of the currency
     * CURRENCY_COL_NAME : the name(code) of the currency
     * CURRENCY_COL_NUMBER : the number of the currency */

    combobox_currency_store = gtk_list_store_new ( CURRENCY_NUM_COL,
						   G_TYPE_STRING,
						   G_TYPE_STRING,
						   G_TYPE_INT,
						   GDK_TYPE_PIXBUF );
    gsb_currency_update_combobox_currency_list ();
    return TRUE;
}




/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
