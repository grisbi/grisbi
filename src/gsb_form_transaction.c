/* ************************************************************************** */
/*                                                                            */
/*     copyright (c)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2004-2008 Benjamin Drieu (bdrieu@april.org)                       */
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
 * \file gsb_form_transaction.c
 * this file contains function of form for transactions (not scheduled transactions)
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_form_transaction.h"
#include "etats_calculs.h"
#include "gsb_currency.h"
#include "gsb_data_account.h"
#include "gsb_data_currency_link.h"
#include "gsb_data_form.h"
#include "gsb_data_mix.h"
#include "gsb_data_payee.h"
#include "gsb_data_payment.h"
#include "gsb_data_report.h"
#include "gsb_data_transaction.h"
#include "gsb_form.h"
#include "gsb_form_widget.h"
#include "gsb_payment_method.h"
#include "gsb_real.h"
#include "gsb_transactions_list.h"
#include "gtk_combofix.h"
#include "structures.h"
#include "utils_dates.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
extern GtkWidget *form_button_recover_split;
extern gsb_real null_real;
/*END_EXTERN*/


/**
 * if only the date and payee are filled in the form, fill all the fields with
 * the same datas that the last transaction found with the same payee
 *
 * \param payee_name
 *
 * \return FALSE if no payee name or problem (and the entry payee will be erased), TRUE if ok
 * */
gboolean gsb_form_transaction_complete_form_by_payee ( const gchar *payee_name )
{
    gint payee_number;
    gint transaction_number = 0;
    gint account_number;
    GSList *tmp_list;

    devel_debug (payee_name);

    if ( !strlen (payee_name))
    return FALSE;

    account_number = gsb_form_get_account_number ();

    /* check if another field than date and payee is filled */
    tmp_list = gsb_form_widget_get_list ();
    while (tmp_list)
    {
        struct_element *element;

        element = tmp_list -> data;

        if ( element -> element_number
             &&
             element -> element_number < TRANSACTION_FORM_MAX_WIDGETS
             &&
             element -> element_number != TRANSACTION_FORM_DATE
             &&
             element -> element_number != TRANSACTION_FORM_PARTY
             &&
             element -> element_number != TRANSACTION_FORM_CHEQUE
             &&
              ( ( GTK_IS_ENTRY ( element -> element_widget )
                &&
                !gsb_form_widget_check_empty ( element -> element_widget ) )
               ||
               (GTK_IS_COMBOFIX (element -> element_widget)
                &&
                !gsb_form_widget_check_empty ( GTK_COMBOFIX (
                element -> element_widget ) -> entry ) ) ) )
            return TRUE;

        tmp_list = tmp_list -> next;
    }

    /* get the payee_number */
    payee_number = gsb_data_payee_get_number_by_name ( payee_name, FALSE );

    /* if it's a new payee, go away */
    if ( !payee_number )
        return TRUE;

    /* find the last transaction with that payee */
    if ( conf.automatic_completion_payee )
    transaction_number = gsb_form_transactions_look_for_last_party ( payee_number,
                        0,
                        account_number );

    /* if no same transaction, go away */
    if ( !transaction_number )
        return TRUE;

    /* fill the form */
    tmp_list = gsb_form_widget_get_list ();
    while (tmp_list)
    {
    struct_element *element;

    element = tmp_list -> data;

    if ( element -> element_number != TRANSACTION_FORM_OP_NB
         &&
         element -> element_number != TRANSACTION_FORM_DATE
         &&
         element -> element_number != TRANSACTION_FORM_VALUE_DATE
         &&
         element -> element_number != TRANSACTION_FORM_EXERCICE
         &&
         element -> element_number != TRANSACTION_FORM_PARTY
         &&
         element -> element_number != TRANSACTION_FORM_MODE
         && 
         element -> element_number != TRANSACTION_FORM_TYPE 
         &&
         element -> element_number != TRANSACTION_FORM_DEVISE )
    {
        gsb_form_fill_element ( element -> element_number,
                        account_number,
                        transaction_number,
                        TRUE );

        /* if split of transaction, propose to recover the children */
        if (element -> element_number == TRANSACTION_FORM_CATEGORY
        &&
        gsb_data_transaction_get_split_of_transaction (transaction_number))
        {
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON ( form_button_recover_split ),
                        conf.automatic_recover_splits );
            gtk_widget_show ( form_button_recover_split );
        }
    }
    else if ( element -> element_number == TRANSACTION_FORM_TYPE )
    {
        GtkWidget *widget;
        gint number;

        number = gsb_data_mix_get_mother_transaction_number (transaction_number, TRUE);
        if ( !number )
            /* it's not a child split, so set number to transaction_number */
            number = transaction_number;

        /* ok, now number contains either the transaction_number, either the mother transaction number,
         * we can check the sign with it */
        widget = gsb_form_widget_get_widget ( TRANSACTION_FORM_TYPE );
        if ( gsb_data_mix_get_amount ( number, TRUE ).mantissa < 0 )
            gsb_payment_method_create_combo_list ( widget,
                                   GSB_PAYMENT_DEBIT,
                                   account_number, 0, FALSE );
        else
            gsb_payment_method_create_combo_list ( widget, GSB_PAYMENT_CREDIT, 
                        account_number, 0, FALSE );

        if ( GTK_WIDGET_VISIBLE (widget))
        {
            gint tmp_account;

            number = gsb_data_mix_get_method_of_payment_number (transaction_number, TRUE);
            tmp_account = gsb_data_transaction_get_account_number ( transaction_number );

            if ( account_number != tmp_account )
                number = gsb_data_payment_search_number_other_account_by_name ( number,
                                account_number );

            /* we show the cheque entry only for transactions */
            if (gsb_payment_method_set_combobox_history ( widget, number, TRUE )
                &&
                gsb_data_payment_get_show_entry ( number )
                &&
                !gsb_data_mix_get_mother_transaction_number (transaction_number, TRUE))
                {
                    gsb_form_entry_get_focus ( gsb_form_widget_get_widget (
                            TRANSACTION_FORM_CHEQUE) );
                    gtk_widget_show (gsb_form_widget_get_widget (
                            TRANSACTION_FORM_CHEQUE) );
                }
        }
        else
            gtk_widget_hide ( gsb_form_widget_get_widget ( TRANSACTION_FORM_CHEQUE ) );
    }

    tmp_list = tmp_list -> next;
    }

    return TRUE;
}



/**
 * Look for the last transaction with the same party. Begin in the current account,
 * and continue in other accounts if necessary.
 *
 * \param no_party the party we are looking for
 * \param no_new_transaction if the transaction found is that transaction, we don't
 * \param account_number the account we want to find first the party
 * keep it
 *
 * \return the number of the transaction found, or 0 
 * */
gint gsb_form_transactions_look_for_last_party ( gint no_party,
                        gint no_new_transaction,
                        gint account_number )
{
    GSList *list_tmp_transactions;
    gint last_transaction_with_party_in_account = 0;
    gint last_transaction_with_party_not_in_account = 0;

    list_tmp_transactions = gsb_data_transaction_get_complete_transactions_list ();

    while ( list_tmp_transactions )
    {
	gint transaction_number_tmp;
	transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	if ( gsb_data_transaction_get_party_number (transaction_number_tmp) == no_party
	     &&
	     transaction_number_tmp != no_new_transaction
	     &&
	     !gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp))
	{
	    /* we are on a transaction with the same party, it's also a split, so we keep it */
	    if ( gsb_data_transaction_get_account_number (transaction_number_tmp) == account_number)
		last_transaction_with_party_in_account = transaction_number_tmp;
	    else
		last_transaction_with_party_not_in_account = transaction_number_tmp;
	}
	list_tmp_transactions = list_tmp_transactions -> next;
    }

    if ( last_transaction_with_party_in_account )
	return last_transaction_with_party_in_account;

    /* if we don't want to complete with a transaction in another account,
     * go away here */
    if ( conf.limit_completion_to_current_account )
	return 0;

    return last_transaction_with_party_not_in_account;
}


/**
 * Clone the children of a split transaction to add the to the new split
 * 
 * \param new_transaction_number	the number of the new mother of the cloned transaction
 * \param no_last_split		the no of last split mother
 *
 * \return FALSE
 * */
gboolean gsb_form_transaction_recover_splits_of_transaction ( gint new_transaction_number,
                        gint no_last_split )
{
    GSList *list_tmp_transactions;

    /* go around the transactions list to get the daughters of the last split */
    list_tmp_transactions = gsb_data_transaction_get_complete_transactions_list ();

    while ( list_tmp_transactions )
    {
	gint transaction_number_tmp;

	transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	if ( gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp) == no_last_split)
	{
	    gint new_child_number;

	    new_child_number = gsb_data_transaction_new_transaction ( gsb_data_transaction_get_account_number (new_transaction_number));
	    gsb_data_transaction_copy_transaction ( transaction_number_tmp,
						    new_child_number, TRUE );
	    gsb_data_transaction_set_mother_transaction_number ( new_child_number,
								 new_transaction_number);
	    gsb_data_transaction_set_date ( new_child_number,
					    gsb_data_transaction_get_date (new_transaction_number));

	    /* if this is a transfer, create the contra transaction */
	    if ( gsb_data_transaction_get_contra_transaction_number (transaction_number_tmp) > 0)
		gsb_form_transaction_validate_transfer ( new_child_number,
							 TRUE,
							 gsb_data_transaction_get_contra_transaction_account (transaction_number_tmp));

	    /* add the transaction to the list */
	    gsb_transactions_list_append_new_transaction (new_child_number, TRUE);
	}
	list_tmp_transactions = list_tmp_transactions -> next;
    }
    return FALSE;
}


/**
 * return a list of numbers of parties if the party in the form is a 
 * report
 * 
 * \param none
 * 
 * \return a g_slist, with -1 if it's a normal party or a list of parties if it's a report
 * */
GSList *gsb_form_transaction_get_parties_list_from_report ( void )
{
    GSList *parties_list;

    parties_list = NULL;

    /*     check that the party's form exist, else, append -1 and go away */
    if ( gsb_data_form_check_for_value (TRANSACTION_FORM_PARTY))
    {
        GtkWidget *combofix;
        const gchar *string;

        combofix = gsb_form_widget_get_widget ( TRANSACTION_FORM_PARTY );
        if ( GTK_IS_COMBOFIX ( combofix ) )
            string = gtk_combofix_get_text ( GTK_COMBOFIX ( combofix ) );
        else
        {
            parties_list = g_slist_append ( parties_list, GINT_TO_POINTER ( - 1) );
            return parties_list;
        }

        if ( string == NULL || strlen ( string ) == 0 )
        {
            parties_list = g_slist_append ( parties_list, GINT_TO_POINTER ( - 1) );
            return parties_list;
        }

        if ( strncmp ( string, _("Report : "), 7 ) )
            /* the party is not a report, set -1 and go away */
            parties_list = g_slist_append ( parties_list, GINT_TO_POINTER ( -1 ) );
        else
        {
            /* it's a report */
            gchar **tab_char;
            gint report_number;

            tab_char = g_strsplit ( string, " : ", 2 );

            report_number = gsb_data_report_get_report_by_name (tab_char[1]);

            if (report_number)
            {
            GSList *list_transactions;
            GSList *list_tmp;

            list_transactions = recupere_opes_etat (report_number);

            list_tmp = list_transactions;

            while ( list_tmp )
            {
                gint transaction_number;

                transaction_number = gsb_data_transaction_get_transaction_number (list_tmp -> data);

                if ( !g_slist_find ( parties_list,
                         GINT_TO_POINTER (gsb_data_transaction_get_party_number (transaction_number))))
                parties_list = g_slist_append ( parties_list,
                                GINT_TO_POINTER ( gsb_data_transaction_get_party_number (transaction_number)));
                list_tmp = list_tmp -> next;
            }
            g_slist_free ( list_transactions );
            }
            else
            /* the report was not found, set -1 */
            parties_list = g_slist_append ( parties_list, GINT_TO_POINTER ( -1 ) );
            g_strfreev ( tab_char );
        }
    }
    else
	/* no party so not a report */
	parties_list = g_slist_append ( parties_list, GINT_TO_POINTER ( - 1) );

    return parties_list;
}



/**
 * validate a transfert from a form :
 * - create the contra-transaction
 * - delete the last contra-transaction if it's a modification
 * - append the contra-transaction to the tree view or update the tree_view
 * 
 * \param transaction_number	the new transaction or the modifed transaction
 * \param new_transaction 	TRUE if it's a new transaction
 * \param account_transfer 	the number of the account we want to create the contra-transaction
 * 
 * \return the number of the contra-transaction
 * */
gint gsb_form_transaction_validate_transfer ( gint transaction_number,
                        gint new_transaction,
                        gint account_transfer )
{
    GDate *contra_value_date = NULL;
    const gchar *contra_transaction_content = NULL;
    gint contra_transaction_number;
    gint contra_mother_number = 0;
    gint contra_marked_transaction = 0;
    GtkWidget *contra_payment_button;

    g_return_val_if_fail ( account_transfer >= 0, -1 );

    /* either it's a new transfer or a change of a non-transfer transaction
     * either it was already a transfer, in that case, if we change the target account,
     * we delete the contra-transaction and it's the same as a new transfer */
    if ( !new_transaction )
    {
        /* it's a modification of a transaction */

        /* as we will do a transfer, the category number is null */
        gsb_data_transaction_set_category_number ( transaction_number, 0 );
        gsb_data_transaction_set_sub_category_number ( transaction_number, 0 );
        contra_transaction_number = gsb_data_transaction_get_contra_transaction_number (
                            transaction_number);
        if (contra_transaction_number > 0)
        {
            const GDate *value_date;

            /* the transaction is a transfer */

            /* if the contra transaction was a child of split, copying/deleting it
             * will remove the information of the mother, so we get it here */
            contra_mother_number = gsb_data_transaction_get_mother_transaction_number (
                            contra_transaction_number );

            /* Copying/deleting remove the marked information, so we get it here */
            contra_marked_transaction = gsb_data_transaction_get_marked_transaction (
                            contra_transaction_number );

            /* Copying/deleting remove the content information, so we get it here */
            contra_transaction_content = gsb_data_transaction_get_method_of_payment_content (
                            contra_transaction_number );

            /* Copying/deleting remove the value date, so we get it here */
            value_date = gsb_data_transaction_get_value_date ( contra_transaction_number );
            if ( value_date )
                contra_value_date = gsb_date_copy ( value_date );


            /* check if we change the account target */
            if ( gsb_data_transaction_get_contra_transaction_account (
                            transaction_number) != account_transfer )
            {
                /* it was a transfer and the user changed the target account so we delete
                 * the last contra transaction contra_transaction_transfer has just been set */

                gsb_data_transaction_set_contra_transaction_number (
                                contra_transaction_number, 0);
                gsb_transactions_list_delete_transaction (contra_transaction_number, FALSE);
                new_transaction = 1;
            }
        }
        else
        {
            /* the transaction was not a transfer, so it's the same as a new transaction, to do the contra-transaction */
            new_transaction = 1;
        }
    }

    /* so, now, it's either a new transfer and new_transaction is TRUE,
     * either a transfer without changing the target account and in that case, contra_transaction_number is
     * already set */

    if ( new_transaction )
        contra_transaction_number = gsb_data_transaction_new_transaction ( account_transfer );

    gsb_data_transaction_copy_transaction ( transaction_number,
					    contra_transaction_number, new_transaction );

    /* contra_mother_number contains the mother number of the contra transaction if it was a modification
     * and that contra-transaction was a child of split, and if not it is 0, and it's a good thing because
     * now contra-transaction has a mother number if the transaction was a child of split, and we have to
     * set that mother number to 0 (for the contra-transaction) */
    gsb_data_transaction_set_mother_transaction_number ( contra_transaction_number,
							 contra_mother_number );

    /* If this is not a new transaction it restores the marked statement */
    gsb_data_transaction_set_marked_transaction ( contra_transaction_number,
                        contra_marked_transaction );

    /* If this is not a new transaction it restores the value date */
    if ( contra_value_date )
    {
		gsb_data_transaction_set_value_date ( contra_transaction_number, contra_value_date );

		g_date_free ( contra_value_date );
	}

    /* we have to change the amount by the opposite */
    gsb_data_transaction_set_amount (contra_transaction_number,
				     gsb_real_opposite (gsb_data_transaction_get_amount (transaction_number)));


    /* we have to check the change */
    gsb_currency_check_for_change ( contra_transaction_number );

    /* set the contra-method of payment,
     * there is no place into the transaction structure for that, so it was not taken with the form,
     * we need to get it now from the form */
    contra_payment_button = gsb_form_widget_get_widget (TRANSACTION_FORM_CONTRA);
    if (contra_payment_button
	&&
	GTK_WIDGET_VISIBLE (contra_payment_button))
	gsb_data_transaction_set_method_of_payment_number ( contra_transaction_number,
							    gsb_payment_method_get_selected_number (contra_payment_button));

    /* set the content of the contra-method of paiement if it is not a new transaction */
    if ( !new_transaction )
        gsb_data_transaction_set_method_of_payment_content ( contra_transaction_number,
                        contra_transaction_content );

    /* set the link between the transactions */
    gsb_data_transaction_set_contra_transaction_number ( transaction_number,
							 contra_transaction_number);
    gsb_data_transaction_set_contra_transaction_number ( contra_transaction_number,
							 transaction_number);

    /* show the contra_transaction */
    if ( new_transaction )
	gsb_transactions_list_append_new_transaction (contra_transaction_number, TRUE);
    else
	gsb_transactions_list_update_transaction (contra_transaction_number);

    return contra_transaction_number;
}


/**
 * callback called when the user click on the 'change' button in the form
 * show a popup to modify the values
 *
 * \param button
 * \param null
 *
 * \return FALSE
 * */
gboolean gsb_form_transaction_change_clicked ( GtkWidget *button,
                        gpointer null )
{
    gint transaction_number;
    gint account_number;
    gint currency_number;
    gint account_currency_number;

    account_number = gsb_form_get_account_number ( );
    account_currency_number = gsb_data_account_get_currency ( account_number );
    gtk_widget_grab_focus ( gsb_form_widget_get_widget ( TRANSACTION_FORM_DATE ));

    transaction_number = GPOINTER_TO_INT (g_object_get_data (
                        G_OBJECT ( gsb_form_get_form_widget ( ) ),
                        "transaction_number_in_form" ));

    if ( transaction_number == -1 )
    {
        gint link_number;

        currency_number = gsb_currency_get_currency_from_combobox (
                        gsb_form_widget_get_widget ( TRANSACTION_FORM_DEVISE ) );
        link_number = gsb_data_currency_link_search ( account_currency_number,
                        currency_number );
        if ( link_number 
         && 
         gsb_data_currency_link_get_first_currency ( link_number )
         == account_currency_number )
            gsb_currency_exchange_dialog ( account_currency_number, currency_number,
                        1,
                        gsb_data_currency_link_get_change_rate ( link_number ),
                        null_real, 
                        TRUE );
        else
            gsb_currency_exchange_dialog ( account_currency_number, currency_number,
                        0,
                        gsb_data_currency_link_get_change_rate ( link_number ),
                        null_real, 
                        TRUE );
    }
    else
    {
        currency_number = gsb_data_transaction_get_currency_number ( transaction_number );
        gsb_currency_exchange_dialog ( account_currency_number, currency_number,
                        gsb_data_transaction_get_change_between ( transaction_number ),
                        gsb_data_transaction_get_exchange_rate ( transaction_number ),
                        gsb_data_transaction_get_exchange_fees ( transaction_number ), 
                        TRUE );
    }

    return FALSE;
}


void gsb_form_transaction_currency_changed ( GtkWidget *widget, gpointer null )
{
    gint account_number;
    gint currency_number;
    gint account_currency_number;
    gint transaction_number;
    gint link_number;

    account_number = gsb_form_get_account_number ( );
    gtk_widget_grab_focus ( gsb_form_widget_get_widget ( TRANSACTION_FORM_DATE ) );

    account_currency_number = gsb_data_account_get_currency ( account_number );

    transaction_number = GPOINTER_TO_INT ( g_object_get_data (
                        G_OBJECT ( gsb_form_get_form_widget ( ) ),
                        "transaction_number_in_form" ) );

    currency_number = gsb_currency_get_currency_from_combobox ( widget );
    if ( account_currency_number == currency_number )
        gtk_widget_hide ( gsb_form_widget_get_widget ( TRANSACTION_FORM_CHANGE ) );
    else
        gtk_widget_show ( gsb_form_widget_get_widget ( TRANSACTION_FORM_CHANGE ) );

    link_number = gsb_data_currency_link_search ( account_currency_number,
                        currency_number );
    if ( link_number == 0 )
    {
        if ( gsb_data_transaction_get_marked_transaction ( transaction_number ) == OPERATION_RAPPROCHEE )
        {
            gtk_widget_set_sensitive ( widget, FALSE );
            gtk_widget_hide ( gsb_form_widget_get_widget (
                        TRANSACTION_FORM_CHANGE ) );
        }
        else if ( transaction_number == -1 )
            gsb_currency_exchange_dialog ( account_currency_number,
                        currency_number,
                        0,
                        null_real,
                        null_real,
                        TRUE );
        else
        {
            gsb_currency_exchange_dialog ( account_currency_number,
                        currency_number,
                        gsb_data_transaction_get_change_between ( transaction_number ),
                        gsb_data_transaction_get_exchange_rate ( transaction_number ),
                        gsb_data_transaction_get_exchange_fees ( transaction_number ),
                        TRUE );
        }
    }
    else if ( transaction_number > 0 )
    {
        gsb_currency_set_current_exchange ( gsb_data_transaction_get_exchange_rate ( transaction_number ) );
        gsb_currency_set_current_exchange_fees ( gsb_data_transaction_get_exchange_fees ( transaction_number ) );
    }
}
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
