/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2007 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2009 Benjamin Drieu (bdrieu@april.org)                       */
/*                      2009 Pierre Biava (grisbi@pierre.biava.name)          */
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
 * \file gsb_currency.c
 * contains tools to work with the currencies
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_currency.h"
#include "dialog.h"
#include "gsb_autofunc.h"
#include "gsb_data_account.h"
#include "gsb_data_currency.h"
#include "gsb_data_currency_link.h"
#include "gsb_data_transaction.h"
#include "gsb_form_widget.h"
#include "gsb_real.h"
#include "utils.h"
#include "structures.h"
#include "utils_files.h"
#include "gsb_data_form.h"
#include "gsb_currency_config.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static struct cached_exchange_rate *gsb_currency_config_get_cached_exchange ( gint currency1_number,
                        gint currency2_number );
static gboolean gsb_currency_checkbutton_link_changed ( GtkWidget *checkbutton,
						  gboolean *value );
static void gsb_currency_config_set_cached_exchange ( gint currency1_number,
                        gint currency2_number,
                        gsb_real change, gsb_real fees );
static gboolean gsb_currency_create_combobox_store ( void );
static GtkWidget *gsb_currency_make_combobox_exchange_dialog ( gint transaction_currency_number,
                        gint account_currency_number,
                        gint set_index );
static gboolean gsb_currency_select_change_currency ( GtkWidget *combobox_1,
                        GtkWidget *combobox_2 );
static gboolean gsb_currency_select_double_amount ( GtkWidget *entry_1,
                        GtkWidget *entry_2 );
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
extern GdkColor calendar_entry_color;
extern GtkWidget *combo_devise_totaux_categ;
extern GtkWidget *combo_devise_totaux_ib;
extern GtkWidget *combo_devise_totaux_tiers;
extern GtkWidget *detail_devise_compte;
extern gint mise_a_jour_liste_comptes_accueil;
extern gint mise_a_jour_liste_echeances_auto_accueil;
extern gint mise_a_jour_liste_echeances_manuelles_accueil;
extern gsb_real null_real;
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
 * create and return a combobox with the currencies
 * for automatic value changed in memory, see gsb_autofunc_currency_new
 *
 * \param set_name if TRUE, the currencies in the combobox will be name(code), else it will be only the code
 *
 * \return a widget combobox or NULL
 * */
GtkWidget *gsb_currency_make_combobox ( gboolean set_name )
{
    GtkCellRenderer *text_renderer, *flag_renderer;
    GtkWidget *combo_box;

    if ( !combobox_currency_store )
        gsb_currency_create_combobox_store ();

    combo_box = gtk_combo_box_new_with_model (GTK_TREE_MODEL 
                        (combobox_currency_store));

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
    gint currency_number = 0;
    GtkTreeIter iter;

    if (!combobox_currency_store)
	gsb_currency_create_combobox_store ();

    if (gtk_combo_box_get_active_iter ( GTK_COMBO_BOX (combo_box),
					&iter ))
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
    gint handler_id;
    gint old_currency_number = -1;
	gchar* tmpstr;

    devel_debug (NULL);
    if (!combobox_currency_store
	||
	!gsb_data_currency_get_currency_list ())
	return FALSE;

    /* XXX still buggy, very slow on the gtk_list_store_clear() call,
     * try to find why. */
    if ( detail_devise_compte && G_IS_OBJECT ( detail_devise_compte ) )
    {
        handler_id = (gulong) g_object_get_data ( G_OBJECT 
                            (detail_devise_compte), "changed-hook" );
        if ( handler_id > 0 )
        {
            g_signal_handler_block ( (gpointer *) detail_devise_compte,
                            handler_id );
            old_currency_number = gtk_combo_box_get_active (GTK_COMBO_BOX 
                            (detail_devise_compte));
        }
    }
    if ( combo_devise_totaux_tiers && G_IS_OBJECT ( combo_devise_totaux_tiers ) )
    {
        handler_id = (gulong) g_object_get_data ( G_OBJECT 
                            (combo_devise_totaux_tiers), "changed-hook" );
        if ( handler_id > 0 )
            g_signal_handler_block ( (gpointer *) combo_devise_totaux_tiers,
                            (gulong) g_object_get_data ( G_OBJECT 
                            (combo_devise_totaux_tiers), "changed-hook" ) );
    }
    if ( combo_devise_totaux_categ  && G_IS_OBJECT ( combo_devise_totaux_categ ) )
    {
        handler_id = (gulong) g_object_get_data ( G_OBJECT 
                            (combo_devise_totaux_categ), "changed-hook" );
        if ( handler_id > 0 )
            g_signal_handler_block ( (gpointer *) combo_devise_totaux_categ,
                            (gulong) g_object_get_data ( G_OBJECT 
                            (combo_devise_totaux_categ), "changed-hook" ) );
    }
    if ( combo_devise_totaux_ib  && G_IS_OBJECT ( combo_devise_totaux_ib ) )
    {
        handler_id = (gulong) g_object_get_data ( G_OBJECT 
                            (combo_devise_totaux_ib), "changed-hook" );
        if ( handler_id > 0 )
            g_signal_handler_block ( (gpointer *) combo_devise_totaux_ib,
                            (gulong) g_object_get_data ( G_OBJECT 
                            (combo_devise_totaux_ib), "changed-hook" ) );
    }

    gtk_list_store_clear (GTK_LIST_STORE (combobox_currency_store));
    list_tmp = gsb_data_currency_get_currency_list ();

    while ( list_tmp )
    {
        GtkTreeIter iter;
        GdkPixbuf * pixbuf;
        gchar * string;
        gint currency_number;

        currency_number = gsb_data_currency_get_no_currency (list_tmp -> data);
        string = g_strconcat( GRISBI_PIXMAPS_DIR, C_DIRECTORY_SEPARATOR,
                    "flags", C_DIRECTORY_SEPARATOR,
                    gsb_data_currency_get_code_iso4217 (currency_number),
                    ".png", NULL );
        pixbuf = gdk_pixbuf_new_from_file ( string, NULL );
        g_free (string);


        gtk_list_store_append ( GTK_LIST_STORE (combobox_currency_store), &iter );
        tmpstr = g_strconcat ( gsb_data_currency_get_name (currency_number),
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

    if ( detail_devise_compte && G_IS_OBJECT ( detail_devise_compte ) )
    {
        handler_id = (gulong) g_object_get_data ( G_OBJECT 
                            (detail_devise_compte), "changed-hook" );
        {
            gtk_combo_box_set_active ( GTK_COMBO_BOX (detail_devise_compte), 
                            old_currency_number );
            g_signal_handler_unblock ( detail_devise_compte,
                            (gulong) g_object_get_data ( G_OBJECT 
                            (detail_devise_compte), "changed-hook" ) );
        }
    }
    if ( combo_devise_totaux_tiers && G_IS_OBJECT ( combo_devise_totaux_tiers ) )
    {
        handler_id = (gulong) g_object_get_data ( G_OBJECT 
                            (combo_devise_totaux_tiers), "changed-hook" );
        if ( handler_id > 0 )
        {
            gtk_combo_box_set_active ( GTK_COMBO_BOX (combo_devise_totaux_tiers), 
                            old_currency_number );
            g_signal_handler_unblock ( (gpointer *) combo_devise_totaux_tiers,
                            (gulong) g_object_get_data ( G_OBJECT 
                            (combo_devise_totaux_tiers), "changed-hook" ) );
        }
    }
    if ( combo_devise_totaux_categ  && G_IS_OBJECT ( combo_devise_totaux_categ ) )
    {
        handler_id = (gulong) g_object_get_data ( G_OBJECT 
                            (combo_devise_totaux_categ), "changed-hook" );
        if ( handler_id > 0 )
        {
            gtk_combo_box_set_active ( GTK_COMBO_BOX (combo_devise_totaux_categ), 
                            old_currency_number );
            g_signal_handler_unblock ( (gpointer *) combo_devise_totaux_categ,
                            (gulong) g_object_get_data ( G_OBJECT 
                            (combo_devise_totaux_categ), "changed-hook" ) );
        }
    }
    if ( combo_devise_totaux_ib  && G_IS_OBJECT ( combo_devise_totaux_ib ) )
    {
        handler_id = (gulong) g_object_get_data ( G_OBJECT 
                            (combo_devise_totaux_ib), "changed-hook" );
        if ( handler_id > 0 )
        {
            gtk_combo_box_set_active ( GTK_COMBO_BOX (combo_devise_totaux_ib), 
                            old_currency_number );
            g_signal_handler_unblock ( (gpointer *) combo_devise_totaux_ib,
                            (gulong) g_object_get_data ( G_OBJECT 
                            (combo_devise_totaux_ib), "changed-hook" ) );
        }
    }

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
    gint link_number;

    account_currency_number = gsb_data_account_get_currency (
                        gsb_data_transaction_get_account_number ( transaction_number ) );
    transaction_currency_number = gsb_data_transaction_get_currency_number (
                        transaction_number );
    link_number = gsb_data_currency_link_search ( account_currency_number,
                        transaction_currency_number );

    if ( link_number )
    {
        if ( current_exchange_fees.mantissa )
            gsb_data_transaction_set_exchange_fees ( transaction_number,
                        current_exchange_fees );
        else
            gsb_data_transaction_set_exchange_fees ( transaction_number,
                        null_real );

        if ( current_exchange.mantissa == 0 )
            gsb_data_transaction_set_exchange_rate ( transaction_number,
                    gsb_real_abs (
                    gsb_data_currency_link_get_change_rate (
                    link_number ) ) );
        else
            gsb_data_transaction_set_exchange_rate ( transaction_number,
                    current_exchange );

        if ( gsb_data_currency_link_get_first_currency (
         link_number) == account_currency_number )
            gsb_data_transaction_set_change_between (transaction_number, 1 );
        else
            gsb_data_transaction_set_change_between (transaction_number, 0 );

        return;
    }

    if ( current_exchange.mantissa == 0 )
        gsb_currency_exchange_dialog ( account_currency_number,
                        transaction_currency_number,
                        0,
                        null_real,
                        null_real,
                        TRUE );

    gsb_data_transaction_set_exchange_rate ( transaction_number,
                        gsb_real_abs (current_exchange));
    if ( current_exchange_fees.mantissa )
        gsb_data_transaction_set_exchange_fees ( transaction_number,
                        current_exchange_fees );
    else
        gsb_data_transaction_set_exchange_fees ( transaction_number,
                        null_real );

    gsb_data_transaction_set_change_between (transaction_number, 0 );
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
 * \param link_currency si = TRUE : 1 nom_devise = "change" devise_en_rapport
 * \param exchange_rate
 * \param exchange_fees
 * \param force if TRUE will not get the cached exchange rate and will really ask to the user
 *
 * \return
 * */
void gsb_currency_exchange_dialog ( gint account_currency_number,
                        gint transaction_currency_number ,
                        gboolean link_currency,
                        gsb_real exchange_rate,
                        gsb_real exchange_fees,
                        gboolean force )
{
    GtkWidget *dialog, *label, *hbox, *paddingbox, *table, *widget;
    GtkWidget *entry, *amount_entry, *amount_1_entry, *amount_2_entry, *fees_entry;
    GtkWidget *combobox_1;
    GtkWidget *combobox_2;
    struct cached_exchange_rate *cache;
    gchar* tmpstr;
    gint row = 0;
    gint result;
    gint link_number;
    gint change_link_currency = 1;

    if ( account_currency_number == 0 || transaction_currency_number == 0 )
        return;

    if ( !force
     &&
     ( cache = gsb_currency_config_get_cached_exchange (
     account_currency_number, transaction_currency_number ) ) )
    {
        current_exchange = cache -> rate;
        current_exchange_fees = cache -> fees;
        return;
    }

    dialog = gtk_dialog_new_with_buttons ( _("Enter exchange rate"),
                        GTK_WINDOW ( window ),
                        GTK_DIALOG_MODAL,
                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                        GTK_STOCK_OK, GTK_RESPONSE_OK,
                        NULL );

    gtk_window_set_position ( GTK_WINDOW ( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );
    gtk_window_set_resizable ( GTK_WINDOW ( dialog ), FALSE );

    /* text for paddingbox */
    tmpstr = g_strdup_printf ( _("Please enter data for the transaction") );

    /* Ugly dance to avoid side effects on dialog's vbox. */
    hbox = gtk_hbox_new ( FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox, FALSE, FALSE, 0 );
    paddingbox = new_paddingbox_with_title ( hbox, TRUE, tmpstr );
    gtk_container_set_border_width ( GTK_CONTAINER(hbox), 6 );
    gtk_container_set_border_width ( GTK_CONTAINER(paddingbox), 6 );
    g_free ( tmpstr );

    /* table for layout */
    table = gtk_table_new ( 2, 5, FALSE );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), table, FALSE, FALSE, 6 );
    gtk_table_set_col_spacings ( GTK_TABLE(table), 6 );
    gtk_table_set_row_spacings ( GTK_TABLE(table), 6 );

    /* echange line label */
    label = gtk_label_new ( _("Currencies") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.0 );
    gtk_table_attach ( GTK_TABLE(table), label, 0, 1, row, row+1,
                        GTK_SHRINK | GTK_FILL, 0, 0, 0 );

    /* echange line currency 1 */
    combobox_1 = gsb_currency_make_combobox_exchange_dialog (
                        transaction_currency_number,
                        account_currency_number,
                        link_currency );
    gtk_table_attach ( GTK_TABLE(table), combobox_1, 1, 2, row, row+1,
                        GTK_SHRINK | GTK_FILL, 0, 0, 0 );

    /* echange line label */
    label = gtk_label_new ( _("Exchange rate") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0.5, 0.0 );
    gtk_table_attach ( GTK_TABLE(table), label, 2, 3, row, row+1,
                        GTK_SHRINK | GTK_FILL, 0, 0, 0 );

    /* echange line currency 2 */
    combobox_2 = gsb_currency_make_combobox_exchange_dialog (
                        transaction_currency_number,
                        account_currency_number,
                        !link_currency );
    gtk_table_attach ( GTK_TABLE(table), combobox_2, 3, 4, row, row+1,
                        GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    row++;

    link_number = gsb_data_currency_link_search ( account_currency_number,
                        transaction_currency_number );
    if ( link_number )
    {
        gtk_widget_set_sensitive ( combobox_1, FALSE );
        gtk_widget_set_sensitive ( combobox_2, FALSE );
    }

    /* amount line */
    label = gtk_label_new ( _("Amounts: ") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.0 );
    gtk_table_attach ( GTK_TABLE(table), label, 0, 1, row, row+1,
                        GTK_SHRINK | GTK_FILL, 0, 0, 0 );

    amount_1_entry = gtk_entry_new ();
    gtk_entry_set_activates_default ( GTK_ENTRY ( amount_1_entry ), TRUE );
    gtk_table_attach ( GTK_TABLE(table), amount_1_entry, 1, 2, row, row+1,
                        GTK_SHRINK | GTK_FILL, 0, 0, 0 );

    /* echange line input field */
    entry = gtk_entry_new ();
    gtk_widget_set_size_request ( entry, 100, -1 );
    gtk_entry_set_activates_default ( GTK_ENTRY ( entry ), TRUE );
    gtk_table_attach ( GTK_TABLE(table), entry, 2, 3, row, row+1,
                        GTK_SHRINK | GTK_FILL, 0, 0, 0 );

    amount_2_entry = gtk_entry_new ();
    gtk_entry_set_activates_default ( GTK_ENTRY ( amount_2_entry ), TRUE );
    gtk_table_attach ( GTK_TABLE(table), amount_2_entry, 3, 4, row, row+1,
                        GTK_SHRINK | GTK_FILL, 0, 0, 0 );

    /* if amount exist already, fill them here */
    if ( link_currency )
        amount_entry = amount_2_entry;
    else
        amount_entry = amount_1_entry;

    widget = gsb_form_widget_get_widget ( TRANSACTION_FORM_DEBIT );
    if ( !gsb_form_widget_check_empty ( widget ) )
        gtk_entry_set_text ( GTK_ENTRY ( amount_entry ),
                        gtk_entry_get_text ( GTK_ENTRY ( widget ) ) );
    else
    {
        widget = gsb_form_widget_get_widget ( TRANSACTION_FORM_CREDIT );
        if ( !gsb_form_widget_check_empty ( widget ) )
            gtk_entry_set_text ( GTK_ENTRY ( amount_entry ),
                        gtk_entry_get_text ( GTK_ENTRY ( widget ) ) );
    }

    /* set the connections */
    g_signal_connect ( G_OBJECT (combobox_1),
                        "changed",
                        G_CALLBACK ( gsb_currency_select_change_currency ),
                        combobox_2 );
    g_signal_connect ( G_OBJECT (combobox_2),
                        "changed",
                        G_CALLBACK ( gsb_currency_select_change_currency ),
                        combobox_1);
    g_object_set_data ( G_OBJECT ( combobox_1 ),
                        "amount_1_entry", amount_1_entry );
    g_object_set_data ( G_OBJECT ( combobox_1 ),
                        "amount_2_entry", amount_2_entry );

    g_object_set_data ( G_OBJECT ( combobox_2 ),
                        "amount_1_entry", amount_1_entry );
    g_object_set_data ( G_OBJECT ( combobox_2 ),
                        "amount_2_entry", amount_2_entry );

    g_signal_connect ( G_OBJECT ( amount_1_entry ),
                        "changed",
                        G_CALLBACK ( gsb_currency_select_double_amount ),
                        amount_2_entry );
    g_signal_connect_swapped ( G_OBJECT ( amount_2_entry ),
                        "changed",
                        G_CALLBACK ( gsb_currency_select_double_amount ),
                        amount_1_entry );
    g_object_set_data ( G_OBJECT ( amount_1_entry ), "exchange_rate", entry );
    g_object_set_data ( G_OBJECT ( amount_1_entry ), "link_currency",
                        GINT_TO_POINTER ( link_currency ) );
    row++;

    /* exchange fees line label */
    label = gtk_label_new ( _("Exchange fees: ") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.0 );
    gtk_table_attach ( GTK_TABLE(table), label, 0, 1, row, row+1,
                        GTK_SHRINK | GTK_FILL, 0, 0, 0 );

    /* exchange fees line input field */
    fees_entry = gtk_entry_new ();
    gtk_entry_set_activates_default ( GTK_ENTRY ( fees_entry ), TRUE );
    gtk_table_attach ( GTK_TABLE(table), fees_entry, 1, 2, row, row+1,
                        GTK_SHRINK | GTK_FILL, 0, 0, 0 );

    /* exchange fees currency for fees */
    label = gtk_label_new (gsb_data_currency_get_name ( account_currency_number ) );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.0 );
    gtk_table_attach ( GTK_TABLE(table), label, 2, 3, row, row+1,
                        GTK_SHRINK | GTK_FILL, 0, 0, 0 );

    if ( link_number )
    {
        GtkWidget *checkbox;

        change_link_currency = !gsb_data_currency_link_get_fixed_link ( link_number );
        checkbox = gtk_check_button_new_with_label ( _("Change the link") );
        if ( change_link_currency )
            gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( checkbox ), TRUE );
        gtk_widget_set_sensitive ( checkbox, TRUE );
        g_signal_connect ( G_OBJECT ( checkbox ),
                        "toggled",
                        G_CALLBACK ( gsb_currency_checkbutton_link_changed ),
                        &change_link_currency );

        gtk_table_attach ( GTK_TABLE(table), checkbox, 3, 4, row, row+1,
                        GTK_SHRINK | GTK_FILL, 0, 0, 0 );
    }

    /* if the rate or fees exist already, fill them here */
    if ( exchange_rate.mantissa )
    {
        tmpstr = gsb_real_get_string ( exchange_rate );
        gtk_entry_set_text ( GTK_ENTRY ( entry ), tmpstr );
        g_free ( tmpstr );
    }

    if ( exchange_fees.mantissa )
    {
        tmpstr = gsb_real_get_string (gsb_real_abs (exchange_fees));
        gtk_entry_set_text ( GTK_ENTRY ( fees_entry ), tmpstr );
        g_free ( tmpstr );
    }

    gtk_widget_show_all ( dialog );

    /* show the dialog */
dialog_return:
    result = gtk_dialog_run ( GTK_DIALOG ( dialog ));

    if ( result == GTK_RESPONSE_OK )
    {
        gint new_link_number;
        gint new_link_currency;

        current_exchange = gsb_real_get_from_string (
                        gtk_entry_get_text ( GTK_ENTRY ( entry ) ) );

        if ( strlen ( gtk_entry_get_text ( GTK_ENTRY ( fees_entry ) ) ) > 0
         ||
         strcmp ( gtk_entry_get_text ( GTK_ENTRY ( fees_entry ) ), "0" ) != 0 )
        {
            current_exchange_fees = gsb_real_get_from_string (
                        gtk_entry_get_text ( GTK_ENTRY ( fees_entry ) ) );
        }
        else
            current_exchange_fees = null_real ;

        if ( current_exchange.mantissa == 0 )
        {
            tmpstr = g_strdup_printf ( _("The exchange rate or the transaction amount in "
                        "%s must be filled."),
                        gsb_data_currency_get_name ( account_currency_number ) );
            dialogue_warning_hint ( tmpstr, _("One field is not filled in") );

            goto dialog_return;
        }

        gsb_currency_config_set_cached_exchange ( account_currency_number,
                        transaction_currency_number,
                        current_exchange, current_exchange_fees );

        if ( link_number )
        {
            if ( change_link_currency
             &&
             gsb_real_cmp ( current_exchange,
             gsb_data_currency_link_get_change_rate ( link_number ) ) != 0 )
                gsb_data_currency_link_set_change_rate ( link_number,
                        current_exchange );
        }
        else
        {
            new_link_number = gsb_data_currency_link_new ( 0 );
            new_link_currency = gtk_combo_box_get_active (
                        GTK_COMBO_BOX ( combobox_1 ) );
            if ( new_link_currency )
            {
                gsb_data_currency_link_set_first_currency ( new_link_number,
                        account_currency_number );
                gsb_data_currency_link_set_second_currency ( new_link_number,
                        transaction_currency_number );
            }
            else
            {
                gsb_data_currency_link_set_first_currency ( new_link_number,
                        transaction_currency_number );
                gsb_data_currency_link_set_second_currency ( new_link_number,
                        account_currency_number );
            }
            gsb_data_currency_link_set_change_rate ( new_link_number,
                        current_exchange );
        }
    }
    else
        gsb_currency_init_exchanges ( );

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
    /* the currency list store, contains 4 columns :
     * CURRENCY_COL_CODE : the code of the currency
     * CURRENCY_COL_NAME : the name(code) of the currency
     * CURRENCY_COL_NUMBER : the number of the currency 
     * CURRENCY_COL_FLAG :  */

    combobox_currency_store = gtk_list_store_new ( CURRENCY_NUM_COL,
						   G_TYPE_STRING,
						   G_TYPE_STRING,
						   G_TYPE_INT,
						   GDK_TYPE_PIXBUF );
    gsb_currency_update_combobox_currency_list ();
    return TRUE;
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
    GtkWidget *entry_1;
    GtkWidget *entry_2;
    gchar *string = NULL;

    /* we just need to set the same active menu on the second combobox */
    g_signal_handlers_block_by_func ( G_OBJECT ( combobox_1 ),
                        G_CALLBACK (gsb_currency_select_change_currency),
                        combobox_2 );
    g_signal_handlers_block_by_func ( G_OBJECT ( combobox_2 ),
                        G_CALLBACK (gsb_currency_select_change_currency),
                        combobox_1 );

    gtk_combo_box_set_active ( GTK_COMBO_BOX (combobox_2),
			       !gtk_combo_box_get_active ( GTK_COMBO_BOX (combobox_1)));

    entry_1 = g_object_get_data ( G_OBJECT ( combobox_1 ), "amount_1_entry" );
    entry_2 = g_object_get_data ( G_OBJECT ( combobox_1 ), "amount_2_entry" );

    g_signal_handlers_block_by_func ( G_OBJECT (entry_1),
                    G_CALLBACK (gsb_currency_select_double_amount),
                    entry_2 );
    g_signal_handlers_block_by_func ( G_OBJECT (entry_2),
                    G_CALLBACK (gsb_currency_select_double_amount),
                    entry_1 );

    string = g_strdup ( gtk_entry_get_text ( GTK_ENTRY ( entry_1 ) ) );
    if ( string && strlen ( string ) > 0 )
    {
        gtk_entry_set_text ( GTK_ENTRY ( entry_1 ), "" );
        gtk_entry_set_text ( GTK_ENTRY ( entry_2 ), string );
        g_free ( string );
    }
    else
    {
        string = g_strdup ( gtk_entry_get_text ( GTK_ENTRY ( entry_2 ) ) );
        if ( string && strlen ( string ) > 0 )
        {
            gtk_entry_set_text ( GTK_ENTRY ( entry_2 ), "" );
            gtk_entry_set_text ( GTK_ENTRY ( entry_1 ), string );
            g_free ( string );
        }
    }

    g_signal_handlers_unblock_by_func ( G_OBJECT (entry_1),
                    G_CALLBACK (gsb_currency_select_double_amount),
                    entry_2 );
    g_signal_handlers_unblock_by_func ( G_OBJECT (entry_2),
                    G_CALLBACK (gsb_currency_select_double_amount),
                    entry_1 );

    g_signal_handlers_unblock_by_func ( G_OBJECT ( combobox_1 ),
                        G_CALLBACK (gsb_currency_select_change_currency),
                        combobox_2 );
    g_signal_handlers_unblock_by_func ( G_OBJECT ( combobox_2 ),
                        G_CALLBACK (gsb_currency_select_change_currency),
                        combobox_1 );

    return FALSE;
}


/*
 * initialize current_exchange and current_exchange_fees
 *
 */
void gsb_currency_init_exchanges ( void )
{
    current_exchange = null_real;
    current_exchange_fees = null_real;
}


/*
 * Handler that change the entries and calculate the exchange_rate
 * 
 *
 * \param entry_1 the entry wich receive the signal
 * \param entry_2 the other entry
 *
 * \return FALSE
 */
gboolean gsb_currency_select_double_amount ( GtkWidget *entry_1,
                        GtkWidget *entry_2 )
{
    GtkWidget *entry, *entry_3, *entry_4;
    gsb_real amount_1, amount_2, taux;
    gboolean link_currency;
    gboolean valide;

    entry = g_object_get_data ( G_OBJECT ( entry_1 ), "exchange_rate" );
    link_currency = GPOINTER_TO_INT ( g_object_get_data (
                         G_OBJECT ( entry_1 ), "link_currency" ) );

    if ( link_currency )
    {
        entry_3 = entry_2;
        entry_4 = entry_1;
    }
    else
    {
        entry_3 = entry_1;
        entry_4 = entry_2;
    }

    valide = gsb_form_widget_get_valide_amout_entry (
                gtk_entry_get_text ( GTK_ENTRY ( entry_1 ) ) );
    if ( valide )
    {
        /* the entry is valid, make it normal */
        gtk_widget_modify_base ( entry_1, GTK_STATE_NORMAL, NULL );
    }
    else
    {
        /* the entry is not valid, make it red */
        gtk_widget_modify_base ( entry_1, GTK_STATE_NORMAL,
                        &calendar_entry_color );
        return FALSE;
    }
    valide = gsb_form_widget_get_valide_amout_entry (
                gtk_entry_get_text ( GTK_ENTRY ( entry_2 ) ) );
    if ( valide )
    {
        /* the entry is valid, make it normal */
        gtk_widget_modify_base ( entry_2, GTK_STATE_NORMAL, NULL );
    }
    else
    {
        /* the entry is not valid, make it red */
        gtk_widget_modify_base ( entry_2, GTK_STATE_NORMAL,
                        &calendar_entry_color );
        return FALSE;
    }

    if ( strlen ( gtk_entry_get_text ( GTK_ENTRY ( entry_3 ) ) ) > 0 )
    {
        if ( !strlen ( gtk_entry_get_text ( GTK_ENTRY ( entry_4 ) ) ) )
		{
            gtk_entry_set_text ( GTK_ENTRY ( entry ), "");
			gtk_widget_set_sensitive ( GTK_WIDGET ( entry ), TRUE );
		}
		else
        {
            gtk_widget_set_sensitive ( GTK_WIDGET ( entry ), FALSE );
            amount_1 = gsb_real_get_from_string ( gtk_entry_get_text ( GTK_ENTRY ( entry_1 ) ) );
            amount_2 = gsb_real_get_from_string ( gtk_entry_get_text ( GTK_ENTRY ( entry_2 ) ) );
            taux = gsb_real_div ( amount_2, amount_1 );
            gtk_entry_set_text ( GTK_ENTRY ( entry ), gsb_real_get_string ( taux ) );
        }
    }
    return FALSE;
}


/*
 * create the exchange rate dialog
 *
 */
GtkWidget *gsb_currency_make_combobox_exchange_dialog ( gint transaction_currency_number,
                        gint account_currency_number,
                        gint set_index )
{
    GtkWidget *combo_box = NULL;
    GtkListStore *combobox_store;
    GtkCellRenderer *text_renderer, *flag_renderer;
    GtkTreeIter iter;
    GdkPixbuf *pixbuf;
    gchar *string;

    combobox_store = gtk_list_store_new ( 3, G_TYPE_INT, GDK_TYPE_PIXBUF,
						G_TYPE_STRING);

    string = g_strconcat( GRISBI_PIXMAPS_DIR, C_DIRECTORY_SEPARATOR,
                        "flags", C_DIRECTORY_SEPARATOR,
                        gsb_data_currency_get_code_iso4217 (
                        transaction_currency_number ),
                        ".png", NULL );
    pixbuf = gdk_pixbuf_new_from_file ( string, NULL );
    g_free (string);

    gtk_list_store_append ( GTK_LIST_STORE ( combobox_store ), &iter );
    gtk_list_store_set ( combobox_store, &iter,
                    0, transaction_currency_number,
                    1, pixbuf,
                    2, gsb_data_currency_get_name ( transaction_currency_number ),
                    -1 );

    string = g_strconcat( GRISBI_PIXMAPS_DIR, C_DIRECTORY_SEPARATOR,
                        "flags", C_DIRECTORY_SEPARATOR,
                        gsb_data_currency_get_code_iso4217 (
                        account_currency_number ),
                        ".png", NULL );
    pixbuf = gdk_pixbuf_new_from_file ( string, NULL );
    g_free (string);

    gtk_list_store_append ( GTK_LIST_STORE ( combobox_store ), &iter );
    gtk_list_store_set ( combobox_store, &iter,
                    0, account_currency_number,
                    1, pixbuf,
                    2, gsb_data_currency_get_name ( account_currency_number ),
                    -1 );

    combo_box = gtk_combo_box_new_with_model (GTK_TREE_MODEL 
                        (combobox_store));

    /* Flag renderer */
    flag_renderer = gtk_cell_renderer_pixbuf_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box), flag_renderer, FALSE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box), flag_renderer,
				    "pixbuf", 1, NULL );

    GTK_CELL_RENDERER(flag_renderer) -> xpad = 3; /* Ugly but how to set it otherwise ?*/

    text_renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box), text_renderer, FALSE);

	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box), text_renderer,
					"text", 2, NULL);

    gtk_combo_box_set_active ( GTK_COMBO_BOX (combo_box), set_index );

    return (combo_box);
}


/**
 *
 *
**/
gsb_real gsb_currency_get_current_exchange ( void )
{
    return current_exchange;
}


/**
 *
 *
**/
gboolean gsb_currency_set_current_exchange ( gsb_real exchange )
{
    current_exchange.mantissa = exchange.mantissa;
    current_exchange.exponent = exchange.exponent;

    return FALSE;
}


/**
 *
 *
**/
gsb_real gsb_currency_get_current_exchange_fees ( void )
{
    return current_exchange_fees;
}


/**
 *
 *
**/
gboolean gsb_currency_set_current_exchange_fees ( gsb_real fees )
{
    current_exchange_fees.mantissa = fees.mantissa;
    current_exchange_fees.exponent = fees.exponent;

    return FALSE;
}


/**
 * met à jour value à chaque changement du check_button
 *
**/
static gboolean gsb_currency_checkbutton_link_changed ( GtkWidget *checkbutton,
						  gboolean *value )
{
    if ( value )
        *value = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( checkbutton ) );

    return FALSE;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
