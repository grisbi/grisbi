/* ************************************************************************** */
/*                                  classement_echeances.c                    */
/*                                                                            */
/*     Copyright (C)    2000-2003 CÃ©dric Auger (cedric@grisbi.org)           */
/*          2003-2004 Benjamin Drieu (bdrieu@april.org)                       */
/*          2003-2004 Alain Portal (aportal@univ-montp2.fr)                   */
/*          2008-2009 Pierre Biava (grisbi@pierre.biava.name)                 */
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

#include "config.h"

#include "include.h"


/*START_INCLUDE*/
#include "classement_echeances.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_transaction.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/


/*START_EXTERN*/
/*END_EXTERN*/




/* ************************************************************************* */
/* Classement de deux échéances d'opérations par date                        */
/* ************************************************************************* */
gint classement_gslist_echeance_par_date ( gpointer data_1,
                        gpointer data_2,
                        gpointer manual )
{
    const GDate *date;
    gint scheduled_number_1 = GPOINTER_TO_INT(data_1);
    gint scheduled_number_2 = GPOINTER_TO_INT(data_2);

    if ( manual )
    {
        date = gsb_data_scheduled_get_date ( scheduled_number_1 );
        if ( date )
            return ( g_date_compare ( date,
                            gsb_data_scheduled_get_date (scheduled_number_2) ));
        else
            return -1;
    }
    else
    {
        date = gsb_data_transaction_get_date ( scheduled_number_1 );
        if ( date )
            return ( g_date_compare ( date,
                            gsb_data_transaction_get_date (scheduled_number_2) ));
        else
            return -1;
    }

}

/**
 * Classement de deux opérations par date croissante
 *
 * \param
 * \param
 *
 * \return
 **/
gint classement_gslist_transactions_par_date (gpointer transaction_pointer_1,
											  gpointer transaction_pointer_2)
{
	TransactionStruct *transaction_1;
	TransactionStruct *transaction_2;

	transaction_1 = transaction_pointer_1;
	transaction_2 = transaction_pointer_2;

	if (transaction_1->date)
        return (g_date_compare (transaction_1->date, transaction_2->date));
    else
        return -1;
}


/**
 * Classement de deux opérations par date décroissante
 *
 * \param
 * \param
 *
 * \return
 **/
gint classement_gslist_transactions_par_date_decroissante (gpointer transaction_pointer_1,
														   gpointer transaction_pointer_2)
{
	TransactionStruct *transaction_1;
	TransactionStruct *transaction_2;

	transaction_1 = transaction_pointer_1;
	transaction_2 = transaction_pointer_2;

	if (transaction_1->date)
		return - (g_date_compare (transaction_1->date, transaction_2->date));
	else
		return -1;

}

/**
 *
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
gint classement_gslist_transactions_par_number (gpointer number_pointer_1,
												gpointer number_pointer_2,
												GtkSortType sens)
{
	gint transaction_number_1;
	gint transaction_number_2;

	transaction_number_1 = GPOINTER_TO_INT (number_pointer_1);
	transaction_number_2 = GPOINTER_TO_INT (number_pointer_2 );

	if (sens == GTK_SORT_ASCENDING)
	{
		return transaction_number_1 - transaction_number_2;
	}
	else
	{
		return transaction_number_2 - transaction_number_1;
	}
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
