/* ************************************************************************** */
/*                                  utils_montants.c                         */
/*                                                                            */
/*     Copyright (C)	2000-2003 CÃ©dric Auger (cedric@grisbi.org)	      */
/*			2003-2004 Benjamin Drieu (bdrieu@april.org)	      */
/*			2003-2004 Alain Portal (aportal@univ-montp2.fr)	      */
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

#include "include.h"


/*START_INCLUDE*/
#include "utils_montants.h"
#include "gsb_data_account.h"
#include "gsb_data_transaction.h"
#include "structures.h"
#include "import.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/


/*START_EXTERN*/
extern GtkWidget *bouton_ok_equilibrage;
extern GtkWidget *label_equilibrage_ecart;
extern GtkWidget *label_equilibrage_pointe;
extern gdouble operations_pointees;
extern gdouble solde_final;
extern gdouble solde_initial;
/*END_EXTERN*/







/******************************************************************************/
/* cette fonction calcule le total des opÃ©s P du compte donnÃ© en argument */
/* et met Ã  jour les labels d'Ã©quilibrage */
/******************************************************************************/
void calcule_total_pointe_compte ( gint no_compte )
{
    GSList *list_tmp_transactions;

    list_tmp_transactions = gsb_data_transaction_get_transactions_list ();
    operations_pointees = 0;

    while ( list_tmp_transactions )
    {
	gint transaction_number_tmp;
	transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	if ( gsb_data_transaction_get_account_number (transaction_number_tmp) == no_compte
	     &&
	     gsb_data_transaction_get_marked_transaction (transaction_number_tmp)== OPERATION_POINTEE
	     &&
	     !gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp))
	{
	    gdouble montant;

	    montant = gsb_data_transaction_get_adjusted_amount (transaction_number_tmp);

	    operations_pointees = operations_pointees + montant;
	}
	list_tmp_transactions = list_tmp_transactions -> next;
    }

 
    gtk_label_set_text ( GTK_LABEL ( label_equilibrage_pointe ),
			 g_strdup_printf ( "%4.2f", 
					   operations_pointees ));

    if ( fabs ( solde_final - solde_initial - operations_pointees ) < 0.01 )
    {
	gtk_label_set_text ( GTK_LABEL ( label_equilibrage_ecart ),
			     g_strdup_printf ( "%4.2f",
					       0.0 ));
	gtk_widget_set_sensitive ( GTK_WIDGET ( bouton_ok_equilibrage ),
				   TRUE );
    }
    else
    {
	gtk_label_set_text ( GTK_LABEL ( label_equilibrage_ecart ),
			     g_strdup_printf ( "%4.2f",
					       solde_final - solde_initial - operations_pointees ));
	gtk_widget_set_sensitive ( GTK_WIDGET ( bouton_ok_equilibrage ),
				   FALSE );
    }
}
/******************************************************************************/


/** calculate the amount of the account given in arg
 * \param account_number
 * \return the current amount
 * */
gdouble calcule_solde_compte ( gint account_number )
{
    gdouble amount;
    GSList *list_tmp;

    if ( DEBUG )
	printf ( "calcule_solde_compte %d\n", account_number );

    amount = gsb_data_account_get_init_balance (account_number);

    list_tmp = gsb_data_transaction_get_transactions_list ();

    while ( list_tmp )
    {
	gint no_transaction;

	no_transaction = gsb_data_transaction_get_transaction_number (list_tmp -> data);

	if ( gsb_data_transaction_get_account_number (no_transaction) == account_number)
	{
	    /* if it's a child breakdown, don't use it */

	    if ( !gsb_data_transaction_get_mother_transaction_number (no_transaction))
		amount = amount + gsb_data_transaction_get_adjusted_amount (no_transaction);
	}
	list_tmp = list_tmp -> next;
    }
    return ( amount );
}


/** calculate the marked amount of the account given in arg
 * \param account_number
 * \return the current amount
 * */
gdouble calcule_solde_pointe_compte ( gint account_number )
{
    gdouble amount;
    GSList *list_tmp;

    if ( DEBUG )
	printf ( "calcule_solde_pointe_compte %d\n", account_number );

    amount = gsb_data_account_get_init_balance (account_number);

    list_tmp = gsb_data_transaction_get_transactions_list ();

    while ( list_tmp )
    {
	gint no_transaction;

	no_transaction = gsb_data_transaction_get_transaction_number (list_tmp -> data);

	if ( gsb_data_transaction_get_account_number (no_transaction) == account_number
	     &&
	     gsb_data_transaction_get_marked_transaction (no_transaction))
	{
	    /* if it's a child breakdown, don't use it */

	    if ( !gsb_data_transaction_get_mother_transaction_number (no_transaction))
		amount = amount + gsb_data_transaction_get_adjusted_amount (no_transaction);
	}
	list_tmp = list_tmp -> next;
    }
    return ( amount );
}
/******************************************************************************/



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
