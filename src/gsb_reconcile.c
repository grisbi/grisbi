/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*                      2009-2013 Pierre Biava (grisbi@pierre.biava.name)     */
/*          https://www.grisbi.org/                                           */
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
 * \file gsb_reconcile.c
 * functions used to reconcile an account
 */


#include "config.h"

#include "include.h"

/*START_INCLUDE*/
#include "gsb_reconcile.h"
#include "gsb_data_transaction.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_GLOBAL*/
/*END_GLOBAL*/

/*START_EXTERN*/
/*END_EXTERN*/

/*START_STATIC*/
/* backup the number of the last transaction converted into planned transaction during the reconciliation */
static gint reconcile_save_last_scheduled_convert = 0;
/*END_STATIC*/

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 * return the date max of transactions ready to reconcile
 *
 * \param account_number the number of the current account
 *
 * \return the date max of the  transactions
 **/
const GDate *gsb_reconcile_get_pointed_transactions_max_date (gint account_number)
{
	GSList *list_tmp;
	const GDate *max_date = NULL;

	list_tmp = gsb_data_transaction_get_transactions_list ();
	while (list_tmp)
	{
		gint transaction_number;
		const GDate *transaction_date;

		transaction_number = gsb_data_transaction_get_transaction_number (list_tmp->data);
		if (gsb_data_transaction_get_account_number (transaction_number) == account_number
			&& (gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_POINTEE
				|| gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_TELEPOINTEE))
		{
			transaction_date = gsb_data_transaction_get_date (transaction_number);
			if (max_date == NULL || g_date_compare (transaction_date, max_date) > 0)
			{
				max_date = transaction_date;
			}
		}
		list_tmp = list_tmp->next;
	}

	return max_date;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gint gsb_reconcile_get_last_scheduled_transaction (void)
{
	return reconcile_save_last_scheduled_convert;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gboolean gsb_reconcile_set_last_scheduled_transaction (gint scheduled_transaction)
{
	reconcile_save_last_scheduled_convert = scheduled_transaction;

	return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
