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
 * \file gsb_data_mix.c
 * this file contains some functions used to split between transactions
 * and scheduled transactions in mixed calls
 * example :
 * if a function wants to set the account number :
 * for a transaction : gsb_data_mix_set_account_number
 * for a scheduled : gsb_data_scheduled_set_account_number
 *
 * but if the function can work with transactions and scheduled (usually a
 * boolean is_transaction is used), it can call the function here :
 * gsb_data_mix_set_account_number with the is_transaction param (TRUE for transaction,
 * FALSE for scheduled transaction), wich will call the directly gsb_data_mix_set_account_number
 * or gsb_data_scheduled_set_account_number
 *
 * there is no description here, to see the description, go to see the description of
 * the relative functions in transaction or scheduled transaction
 *
 * -2 interestings points with that functions :
 *  -reduce place of code, instead of if (is_transaction) gsb_data_... 4 lines, only one
 *  with gsb_data_mix_...
 *  -it exists also some gsb_data_mix_ wich exist only for transaction. it's a good thing because avoid to fill
 *  a transaction with the number of the scheduled transaction (for example if someone succeed to fill a value date
 *  for a scheduled transaction (normally not possible), if we don't use gsb_data_mix and don't protect before against
 *  that kind of error, the transaction with the number of the scheduled transaction will have its value date changed...
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"


/*START_INCLUDE*/
#include "gsb_data_mix.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_transaction.h"
#include "gsb_real.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/



gint gsb_data_mix_new_transaction ( gint account_number,
				    gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_new_transaction (account_number));
    else
    {
	/* need to set the account number after because doesn't get in param */
	gint number;

	number = gsb_data_scheduled_new_scheduled ();
	gsb_data_scheduled_set_account_number ( number, account_number);
	return (number);
    }
}


gint gsb_data_mix_get_account_number ( gint transaction_number,
				       gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_get_account_number ( transaction_number));
    else
	return (gsb_data_scheduled_get_account_number ( transaction_number));
}

const GDate *gsb_data_mix_get_date ( gint transaction_number,
				     gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_get_date ( transaction_number));
    else
	return (gsb_data_scheduled_get_date ( transaction_number));
}

gboolean gsb_data_mix_set_date ( gint transaction_number,
				 GDate *date,
				 gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_set_date ( transaction_number,
						date));
    else
	return (gsb_data_scheduled_set_date ( transaction_number,
					      date));
}

gboolean gsb_data_mix_set_value_date ( gint transaction_number,
				       GDate *date,
				       gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_set_value_date ( transaction_number,
						      date));
    return FALSE;
}

gsb_real gsb_data_mix_get_amount ( gint transaction_number,
				   gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_get_amount ( transaction_number));
    else
	return (gsb_data_scheduled_get_amount ( transaction_number));
}

gboolean gsb_data_mix_set_amount ( gint transaction_number,
				   gsb_real amount,
				   gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_set_amount ( transaction_number,
						  amount));
    else
	return (gsb_data_scheduled_set_amount ( transaction_number,
						amount));
}

gint gsb_data_mix_get_currency_number ( gint transaction_number,
					gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_get_currency_number ( transaction_number));
    else
	return (gsb_data_scheduled_get_currency_number ( transaction_number));
}

gboolean gsb_data_mix_set_currency_number ( gint transaction_number,
					    gint no_currency,
					    gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_set_currency_number ( transaction_number,
							   no_currency));
    else
	return (gsb_data_scheduled_set_currency_number ( transaction_number,
							 no_currency));
}


gint gsb_data_mix_get_party_number ( gint transaction_number,
				     gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_get_party_number ( transaction_number));
    else
	return (gsb_data_scheduled_get_party_number ( transaction_number));
}

gboolean gsb_data_mix_set_party_number ( gint transaction_number,
					 gint no_party,
					 gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_set_party_number ( transaction_number,
							no_party));
    else
	return (gsb_data_scheduled_set_party_number ( transaction_number,
						      no_party));
}

gint gsb_data_mix_get_category_number ( gint transaction_number,
					gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_get_category_number ( transaction_number));
    else
	return (gsb_data_scheduled_get_category_number ( transaction_number));
}

gboolean gsb_data_mix_set_category_number ( gint transaction_number,
					    gint no_category,
					    gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_set_category_number ( transaction_number,
							   no_category));
    else
	return (gsb_data_scheduled_set_category_number ( transaction_number,
							 no_category));
}

gint gsb_data_mix_get_sub_category_number ( gint transaction_number,
					    gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_get_sub_category_number ( transaction_number));
    else
	return (gsb_data_scheduled_get_sub_category_number ( transaction_number));
}

gboolean gsb_data_mix_set_sub_category_number ( gint transaction_number,
						gint no_sub_category,
						gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_set_sub_category_number ( transaction_number,
							       no_sub_category));
    else
	return (gsb_data_scheduled_set_sub_category_number ( transaction_number,
							     no_sub_category));
}

gint gsb_data_mix_get_split_of_transaction ( gint transaction_number,
						 gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_get_split_of_transaction ( transaction_number));
    else
	return (gsb_data_scheduled_get_split_of_scheduled ( transaction_number));
}

gboolean gsb_data_mix_set_split_of_transaction ( gint transaction_number,
						     gint is_split,
						     gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_set_split_of_transaction ( transaction_number,
								    is_split));
    else
	return (gsb_data_scheduled_set_split_of_scheduled ( transaction_number,
								is_split));
}

const gchar *gsb_data_mix_get_notes ( gint transaction_number,
				      gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_get_notes ( transaction_number));
    else
	return (gsb_data_scheduled_get_notes ( transaction_number));
}

gboolean gsb_data_mix_set_notes ( gint transaction_number,
				  const gchar *notes,
				  gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_set_notes ( transaction_number,
						 notes));
    else
	return (gsb_data_scheduled_set_notes ( transaction_number,
					       notes));
}

gint gsb_data_mix_get_method_of_payment_number ( gint transaction_number,
						 gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_get_method_of_payment_number ( transaction_number));
    else
	return (gsb_data_scheduled_get_method_of_payment_number ( transaction_number));
}

gboolean gsb_data_mix_set_method_of_payment_number ( gint transaction_number,
						     gint number,
						     gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_set_method_of_payment_number ( transaction_number,
								    number));
    else
	return (gsb_data_scheduled_set_method_of_payment_number ( transaction_number,
								  number));
}

gboolean gsb_data_mix_set_method_of_payment_content ( gint transaction_number,
						      const gchar *method_of_payment_content,
						      gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_set_method_of_payment_content ( transaction_number,
								     method_of_payment_content));
    else
	return (gsb_data_scheduled_set_method_of_payment_content ( transaction_number,
								   method_of_payment_content));
}

gint gsb_data_mix_get_automatic_transaction ( gint transaction_number,
					      gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_get_automatic_transaction ( transaction_number));
    return 0;
}

gint gsb_data_mix_get_financial_year_number ( gint transaction_number,
					      gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_get_financial_year_number ( transaction_number));
    else
	return (gsb_data_scheduled_get_financial_year_number ( transaction_number));
}

gboolean gsb_data_mix_set_financial_year_number ( gint transaction_number,
						  gint financial_year_number,
						  gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_set_financial_year_number ( transaction_number,
								 financial_year_number));
    else
	return (gsb_data_scheduled_set_financial_year_number ( transaction_number,
							       financial_year_number));
}

gint gsb_data_mix_get_budgetary_number ( gint transaction_number,
					 gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_get_budgetary_number ( transaction_number));
    else
	return (gsb_data_scheduled_get_budgetary_number ( transaction_number));
}

gboolean gsb_data_mix_set_budgetary_number ( gint transaction_number,
					     gint budgetary_number,
					     gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_set_budgetary_number ( transaction_number,
							    budgetary_number));
    else
	return (gsb_data_scheduled_set_budgetary_number ( transaction_number,
							  budgetary_number));
}

gint gsb_data_mix_get_sub_budgetary_number ( gint transaction_number,
					     gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_get_sub_budgetary_number ( transaction_number));
    else
	return (gsb_data_scheduled_get_sub_budgetary_number ( transaction_number));
}

gboolean gsb_data_mix_set_sub_budgetary_number ( gint transaction_number,
						 gint sub_budgetary_number,
						 gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_set_sub_budgetary_number ( transaction_number,
								sub_budgetary_number));
    else
	return (gsb_data_scheduled_set_sub_budgetary_number ( transaction_number,
							      sub_budgetary_number));
}

const gchar *gsb_data_mix_get_voucher ( gint transaction_number,
					gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_get_voucher ( transaction_number));
    return NULL;
}

gboolean gsb_data_mix_set_voucher ( gint transaction_number,
				    const gchar *voucher,
				    gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_set_voucher ( transaction_number,
						   voucher));
    return FALSE;
}

const gchar *gsb_data_mix_get_bank_references ( gint transaction_number,
						gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_get_bank_references ( transaction_number));
    return NULL;
}

gboolean gsb_data_mix_set_bank_references ( gint transaction_number,
					    const gchar *bank_references,
					    gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_set_bank_references ( transaction_number,
							   bank_references));
    return FALSE;
}

gint gsb_data_mix_get_transaction_number_transfer ( gint transaction_number,
						    gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_get_contra_transaction_number ( transaction_number));
    else
	/* if we come here for a scheduled transaction, usually it's to know if it's a transfer or not
	 * because it's like that we test for normal transactions */
	return gsb_data_scheduled_is_transfer (transaction_number);
}

gboolean gsb_data_mix_set_transaction_number_transfer ( gint transaction_number,
							gint transaction_number_transfer,
							gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_set_contra_transaction_number ( transaction_number,
								       transaction_number_transfer));
    return FALSE;
}

gint gsb_data_mix_get_account_number_transfer ( gint transaction_number,
						gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_get_contra_transaction_account ( transaction_number));
    else
	return (gsb_data_scheduled_get_account_number_transfer ( transaction_number));
}


gint gsb_data_mix_get_mother_transaction_number ( gint transaction_number,
						  gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_get_mother_transaction_number ( transaction_number));
    else
	return (gsb_data_scheduled_get_mother_scheduled_number ( transaction_number));
}

gboolean gsb_data_mix_set_mother_transaction_number ( gint transaction_number,
						      gint mother_transaction_number,
						      gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_set_mother_transaction_number ( transaction_number,
								     mother_transaction_number));
    else
	return (gsb_data_scheduled_set_mother_scheduled_number ( transaction_number,
								   mother_transaction_number));
}

gint gsb_data_mix_get_white_line ( gint transaction_number,
				   gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_get_white_line (transaction_number));
    else
	return (gsb_data_scheduled_get_white_line (transaction_number));
}

GSList *gsb_data_mix_get_children ( gint transaction_number,
				    gboolean return_number,
				    gboolean is_transaction )
{
    if (is_transaction)
	return (gsb_data_transaction_get_children (transaction_number, return_number));
    else
	return (gsb_data_scheduled_get_children (transaction_number, return_number));
}

