/*     Copyright (C)	2000-2006 Cédric Auger (cedric@grisbi.org) */
/*			2003-2006 Benjamin Drieu (bdrieu@april.org) */
/* 			http://www.grisbi.org */

/*     This program is free software; you can redistribute it and/or modify */
/*     it under the terms of the GNU General Public License as published by */
/*     the Free Software Foundation; either version 2 of the License, or */
/*     (at your option) any later version. */

/*     This program is distributed in the hope that it will be useful, */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/*     GNU General Public License for more details. */

/*     You should have received a copy of the GNU General Public License */
/*     along with this program; if not, write to the Free Software */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

/**
 * \file gsb_payment_method.c
 * work here with the payment method
 */


#include "include.h"

/*START_INCLUDE*/
#include "gsb_payment_method.h"
#include "gsb_data_account.h"
#include "gsb_data_payment.h"
#include "gsb_form.h"
#include "gsb_form_widget.h"
#include "utils_str.h"
#include "gsb_data_form.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/




/**
 * create a model with the method of payment according to
 * the sign and the account
 * and append it to the combo_box given in param
 * all is done only if the button exists for that form
 *
 * \param combo_box
 * \param sign GSB_PAYMENT_DEBIT or GSB_PAYMENT_CREDIT
 * \param account_number
 * \param exclude if we want to exclude a method of payment from the list, set the number here, else set 0
 *
 * \return FALSE if fail, TRUE if ok
 * */
gboolean gsb_payment_method_create_combo_list ( GtkWidget *combo_box,
						gint sign,
						gint account_number,
						gint exclude )
{
    GtkListStore *store;
    GSList *tmp_list;
    gint store_filled = 0;

    if (!combo_box)
	return FALSE;

    /* we check first the existing model ; if none, we create it */
    store = GTK_LIST_STORE (gtk_combo_box_get_model (GTK_COMBO_BOX (combo_box)));

    if (store)
	gtk_list_store_clear (store);
    else
    {
	GtkCellRenderer *renderer;

	store = gtk_list_store_new ( 2,
				     G_TYPE_STRING,
				     G_TYPE_INT );
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box), renderer,
					"text", 0,
					NULL);
	gtk_combo_box_set_model ( GTK_COMBO_BOX (combo_box),
				  GTK_TREE_MODEL (store));
    }

    /* we check all the method of payment to find wich of them we want in our store */
    tmp_list = gsb_data_payment_get_payments_list ();
    while (tmp_list)
    {
	gint payment_number;

	payment_number = gsb_data_payment_get_number (tmp_list -> data);

	if ( gsb_data_payment_get_account_number (payment_number) == account_number
	     &&
	     ( gsb_data_payment_get_sign (payment_number) == sign
	       ||
	       gsb_data_payment_get_sign (payment_number) == GSB_PAYMENT_NEUTRAL )
	     &&
	     payment_number != exclude )
	{
	    GtkTreeIter iter;

	    gtk_list_store_append ( GTK_LIST_STORE (store),
				    &iter );
	    gtk_list_store_set ( GTK_LIST_STORE (store),
				 &iter,
				 0, gsb_data_payment_get_name (payment_number),
				 1, payment_number,
				 -1 );
	    store_filled = 1;
	}
	tmp_list = tmp_list -> next;
    }
    g_object_set_data ( G_OBJECT (combo_box),
			"combo_sign",
			GINT_TO_POINTER (sign));

    /* if nothing in the store, hide it */
    if (store_filled)
    {
	gtk_combo_box_set_active ( GTK_COMBO_BOX (combo_box),
				   gsb_payment_method_get_payment_location ( combo_box,
									     gsb_data_account_get_default_debit (account_number)));
	gtk_widget_show (combo_box);
    }
    else
    {
	gtk_widget_hide (combo_box);
    }
    return TRUE;
}


/**
 * get the sign of the current combo_box of method of payment
 * in the combo_box given in param
 *
 * \param combo_box the box wich contains the method of payment list
 *
 * \return GSB_PAYMENT_DEBIT or GSB_PAYMENT_CREDIT
 * */
gint gsb_payment_method_get_combo_sign ( GtkWidget *combo_box )
{
    gint sign;

    if (!combo_box)
	return FALSE;

    sign = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT (combo_box),
						 "combo_sign" ));
    return sign;
}


/**
 * find and return the curent selected method of payment
 * on combo-box given in param and created by
 * gsb_payment_method_create_combo_list
 *
 * \param combo_box an combo_box filled by gsb_payment_method_create_combo_list
 *
 * \return the number of the method of payment currently selected or 0 if problem
 * */
gint gsb_payment_method_get_selected_number ( GtkWidget *combo_box )
{
    gint payment_method;
    GtkTreeModel *model;
    GtkTreeIter iter;

    if ( !combo_box
	 ||
	 !GTK_WIDGET_VISIBLE (combo_box))
	return 0;

    model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo_box));
    if ( !gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combo_box),
					 &iter))
	return 0;

    gtk_tree_model_get ( GTK_TREE_MODEL (model),
			 &iter,
			 1, &payment_method,
			 -1 );
    return payment_method;
}




/**
 * look for the payment number in the list of the combo_box
 *
 * \param combo_box
 * \param payment_number
 *
 * \return the location of the payment or 0 if problem
 * */
gint gsb_payment_method_get_payment_location ( GtkWidget *combo_box,
					       gint payment_number )
{
    gint payment_location = 0;
    gint i = 0;
    GtkTreeModel *model;
    GtkTreeIter iter;

    if ( !payment_number
	 ||
	 !combo_box )
	return 0;

    model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo_box));
    if (gtk_tree_model_get_iter_first ( GTK_TREE_MODEL (model),
					&iter))
    {
	do
	{
	    gint tmp;

	    gtk_tree_model_get ( GTK_TREE_MODEL (model),
				 &iter,
				 1, &tmp,
				 -1 );
	    if ( tmp == payment_number )
		payment_location = i;
	    i++; 
	}
	while ( gtk_tree_model_iter_next (GTK_TREE_MODEL (model),
					  &iter)
		&&
		!payment_location );
    }
    return payment_location;
}


/**
 * set the given payment number in the combobox
 * if not found, set the default payment number for that account
 *
 * \param combo_box
 * \param payment_number
 * \param account_number
 *
 * \return TRUE if we can set the payment_number, FALSE if it's the default wich is set
 * */
gboolean gsb_payment_method_set_combobox_history ( GtkWidget *combo_box,
						   gint payment_number,
						   gint account_number )
{
    gint location;
    gboolean return_value;

    location = gsb_payment_method_get_payment_location ( combo_box,
							 payment_number );
    if (location)
	return_value = TRUE;
    else
    {
	if ( gsb_payment_method_get_combo_sign (combo_box) == GSB_PAYMENT_CREDIT)
	    location = gsb_payment_method_get_payment_location ( combo_box,
								 gsb_data_account_get_default_credit (account_number));
	else
	    location = gsb_payment_method_get_payment_location ( combo_box,
								 gsb_data_account_get_default_debit (account_number));
	return_value= FALSE;
    }
    gtk_combo_box_set_active ( GTK_COMBO_BOX (combo_box),
			       location );
    return return_value;
}


/**
 * called when change the choice on the combo_box on the form
 * show or hide the entry for cheque in the form
 *
 * \param combo_box
 * \param null
 *
 * \return FALSE
 * */
gboolean gsb_payment_method_changed_callback ( GtkWidget *combo_box,
					       gpointer null )
{
    gint payment_number;
    GtkWidget *cheque_entry;
    gint account_number;

    account_number = gsb_form_get_account_number ();
    cheque_entry = gsb_form_widget_get_widget (TRANSACTION_FORM_CHEQUE);
    if ( !cheque_entry)
	return FALSE;

    payment_number = gsb_payment_method_get_selected_number (combo_box);

    if (!payment_number)
	return FALSE;

    if (gsb_data_payment_get_show_entry (payment_number))
    {
	/* set the next number if needed */
	if (gsb_data_payment_get_automatic_numbering (payment_number))
	{
	    gsb_form_entry_get_focus (cheque_entry);
	    gtk_entry_set_text ( GTK_ENTRY (cheque_entry),
				 utils_str_itoa (gsb_data_payment_get_last_number (payment_number)));
	}
	else
	{
	    gtk_entry_set_text ( GTK_ENTRY (cheque_entry),
				 "" );
	    gsb_form_entry_lose_focus ( cheque_entry,
					FALSE,
					GINT_TO_POINTER ( TRANSACTION_FORM_CHEQUE ));
	}
	gtk_widget_show (cheque_entry);
    }
    else
	gtk_widget_hide (cheque_entry);

    return FALSE;
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
