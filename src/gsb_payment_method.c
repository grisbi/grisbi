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
#include "gsb_form.h"
#include "utils_str.h"
#include "gsb_payment_method.h"
#include "gsb_data_form.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static struct struct_type_ope *gsb_payment_method_get_structure ( gint payment_number,
							   gint account_number );
/*END_STATIC*/

/*START_EXTERN*/
extern gint max;
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
 *
 * \return FALSE if problem, TRUE if ok
 * */
gboolean gsb_payment_method_create_combo_list ( GtkWidget *combo_box,
						gint sign,
						gint account_number )
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

    tmp_list = gsb_data_account_get_method_payment_list (account_number);

    while ( tmp_list )
    {
	struct struct_type_ope *method_ptr;


	method_ptr = tmp_list -> data;

	/* we add the method of payment for the same sign or for neutrals */
	if ( method_ptr -> signe_type == sign
	     ||
	     method_ptr -> signe_type == GSB_PAYMENT_NEUTRAL)
	{
	    GtkTreeIter iter;

	    gtk_list_store_append ( GTK_LIST_STORE (store),
				    &iter );
	    gtk_list_store_set ( GTK_LIST_STORE (store),
				 &iter,
				 0, method_ptr -> nom_type,
				 1, method_ptr -> no_type,
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

    if ( !combo_box )
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
 * check if the selected payment has automatic number
 *
 * \param payment_number
 * \param account_number
 *
 * \return TRUE : has automatic number, FALSE else
 * */
gboolean gsb_payment_method_get_automatic_number ( gint payment_number,
						   gint account_number )
{
    struct struct_type_ope *method_ptr;

    method_ptr = gsb_payment_method_get_structure ( payment_number,
						    account_number );
    if (method_ptr)
	return method_ptr -> numerotation_auto;

    return FALSE;
}

/**
 * check if the selected payment need to show the entry
 *
 * \param payment_number
 * \param account_number
 *
 * \return TRUE if need to show the entry, FALSE else
 * */
gboolean gsb_payment_method_get_show_entry ( gint payment_number,
					     gint account_number )
{
    struct struct_type_ope *method_ptr;

    method_ptr = gsb_payment_method_get_structure ( payment_number,
						    account_number );
    if (method_ptr)
	return method_ptr -> affiche_entree;

    return FALSE;
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
    struct struct_type_ope *method_ptr;
    gint payment_number;
    GtkWidget *cheque_entry;
    gint account_number;

    account_number = gsb_form_get_account_number_from_origin (gsb_form_get_origin ());
    cheque_entry = gsb_form_get_element_widget_2 ( TRANSACTION_FORM_CHEQUE,
						   account_number );
    if ( !cheque_entry)
	return FALSE;

    payment_number = gsb_payment_method_get_selected_number (combo_box);
    method_ptr = gsb_payment_method_get_structure ( payment_number,
						    account_number );

    if (!method_ptr)
	return FALSE;

    if ( method_ptr -> affiche_entree )
    {
	/* set the next number if needed */

	if ( method_ptr -> numerotation_auto )
	{
	    gsb_form_entry_get_focus (cheque_entry, NULL, NULL );
	    gtk_entry_set_text ( GTK_ENTRY (cheque_entry),
				 gsb_payment_method_get_automatic_current_number ( method_ptr ));
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


/**
 * look for a payment number in a given account and return
 * a pointer to its structure
 *
 * \param payment_number wich we look for
 * \param account_number
 *
 * \return a pointer to the structure of the payment_number, NULL if problem
 * */
struct struct_type_ope *gsb_payment_method_get_structure ( gint payment_number,
							   gint account_number )
{
    GSList *tmp_list;
    tmp_list = gsb_data_account_get_method_payment_list (account_number);

    while ( tmp_list )
    {
	struct struct_type_ope *method_ptr;

	method_ptr = tmp_list -> data;
	if ( method_ptr -> no_type == payment_number )
	    return method_ptr;

	tmp_list = tmp_list -> next;
    }
    return NULL;
}



/**
 * Get the max content number associated to a method_ptr structure,
 * increment it and return it.  Handy to find the next number to fill
 * in the cheque field of the transaction form.
 *
 * \param payment_number
 * \param account_number
 *
 * \return	A textual representation of the maximum + 1
 */
gchar *gsb_payment_method_automatic_numbering_get_new_number ( gint payment_number,
							       gint account_number )
{
    struct struct_type_ope *method_ptr;

    method_ptr = gsb_payment_method_get_structure ( payment_number,
						    account_number );
    if ( method_ptr )
    {
 	return utils_str_itoa ( method_ptr -> no_en_cours + 1 );
    }
  
    return "1";
}



/**
 * Get the max content number associated to a method_ptr structure and
 * return it.
 *
 * \param method_ptr	The method_ptr structure to compute.
 *
 * \return	A textual representation of the maximum.
 */
gchar *gsb_payment_method_get_automatic_current_number ( struct struct_type_ope * method_ptr )
{
    if ( method_ptr )
    {
 	return utils_str_itoa ( method_ptr -> no_en_cours );
    }
  
    return "1";
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
