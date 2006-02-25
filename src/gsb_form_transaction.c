/* ************************************************************************** */
/*                                                                            */
/*     copyright (c)	2000-2006 Cédric Auger (cedric@grisbi.org)	      */
/*			2004-2006 Benjamin Drieu (bdrieu@april.org) 	      */
/*			http://www.grisbi.org   			      */
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
 * working on the transactions and scheduled transactions in the form
 */


#include "include.h"

/*START_INCLUDE*/
#include "gsb_form_transaction.h"
#include "accueil.h"
#include "gsb_transactions_list.h"
#include "gsb_fyear.h"
#include "erreur.h"
#include "dialog.h"
#include "equilibrage.h"
#include "gsb_currency.h"
#include "gsb_data_account.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_currency_link.h"
#include "gsb_data_form.h"
#include "gsb_data_payee.h"
#include "gsb_data_report.h"
#include "gsb_data_transaction.h"
#include "utils_dates.h"
#include "gsb_form.h"
#include "navigation.h"
#include "gsb_payment_method.h"
#include "gsb_real.h"
#include "utils_editables.h"
#include "gtk_combofix.h"
#include "menu.h"
#include "categories_onglet.h"
#include "imputation_budgetaire.h"
#include "tiers_onglet.h"
#include "traitement_variables.h"
#include "utils_str.h"
#include "etats_calculs.h"
#include "utils.h"
#include "utils_operations.h"
#include "include.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean gsb_form_get_categories ( gint transaction_number,
				   gint new_transaction );
static GSList *gsb_form_get_parties_list_from_report ( void );
static void gsb_form_take_datas_from_form ( gint transaction_number );
static void gsb_form_transaction_check_change_button ( gint currency_number,
						gint account_number );
static gint gsb_form_transactions_look_for_last_party ( gint no_party,
						 gint no_new_transaction,
						 gint account_number );
static gboolean gsb_form_validate_form_transaction ( gint transaction_number );
static gboolean gsb_transactions_list_recover_breakdowns_of_transaction ( gint new_transaction_number,
								   gint no_last_breakdown,
								   gint no_account );
/*END_STATIC*/

/*START_EXTERN*/
extern gboolean block_menu_cb ;
extern GtkWidget *formulaire;
extern gint hauteur_ligne_liste_opes;
extern GtkItemFactory *item_factory_menu_general;
extern gint mise_a_jour_combofix_categ_necessaire;
extern gint mise_a_jour_combofix_imputation_necessaire;
extern gint mise_a_jour_combofix_tiers_necessaire;
extern gint mise_a_jour_fin_comptes_passifs;
extern gint mise_a_jour_liste_comptes_accueil;
extern gint mise_a_jour_soldes_minimaux;
extern GtkStyle *style_entree_formulaire[2];
extern GtkWidget *tree_view;
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
    gint transaction_number;
    gint row,column;
    GtkWidget *widget;
    gint account_number;

    devel_debug ( g_strdup_printf ( "gsb_form_transaction_complete_form_by_payee %s",
				    payee_name ));

    if ( !strlen (payee_name))
	return FALSE;

    account_number = gsb_form_get_account_number ();

    /* check if another field than date and payee is filled */
    for ( row=0 ; row < gsb_data_form_get_nb_rows (account_number) ; row++ )
	for ( column=0 ; column < gsb_data_form_get_nb_columns (account_number) ; column++ )
	{
	    gint value;
	    value = gsb_data_form_get_value ( account_number,
					      column,
					      row );

	    if ( value
		 &&
		 value != TRANSACTION_FORM_DATE
		 &&
		 value != TRANSACTION_FORM_PARTY )
	    {
		widget = gsb_form_get_element_widget (value,
						      account_number);

		if ( (GTK_IS_ENTRY (widget)
		     &&
		     !gsb_form_check_entry_is_empty(widget))
		     ||
		     (GTK_IS_COMBOFIX (widget)
		      &&
		      !gsb_form_check_entry_is_empty(GTK_COMBOFIX (widget) -> entry)))
		    return TRUE;
	    }
	}

    /* get the payee_number */
    payee_number = gsb_data_payee_get_number_by_name ( payee_name,
						       FALSE );
    /* if it's a new payee, go away */
    if ( !payee_number )
	return TRUE;

    /* find the last transaction with that payee */
    transaction_number = gsb_form_transactions_look_for_last_party ( payee_number,
								     0,
								     account_number );

    /* if no same transaction, go away */
    if ( !transaction_number )
	return TRUE;

    /* fill the form */
    for ( row=0 ; row < gsb_data_form_get_nb_rows (account_number) ; row++ )
	for ( column=0 ; column <  gsb_data_form_get_nb_columns (account_number) ; column++ )
	{
	    gint value;

	    value = gsb_data_form_get_value ( account_number,
					      column,
					      row );

	    if ( value != TRANSACTION_FORM_OP_NB
		 &&
		 value != TRANSACTION_FORM_DATE
		 &&
		 value != TRANSACTION_FORM_VALUE_DATE
		 &&
		 value != TRANSACTION_FORM_EXERCICE
		 &&
		 value != TRANSACTION_FORM_PARTY
		 &&
		 value != TRANSACTION_FORM_MODE )
	    gsb_form_transaction_fill_form ( value,
					     account_number,
					     transaction_number );
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

    list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

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
	    /* we are on a transaction with the same party, so we keep it */
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
    if ( etat.limit_completion_to_current_account )
	return 0;

    return last_transaction_with_party_not_in_account;
}


/**
 * fill the element_number in the form according to the given transaction
 *
 * \param element_number
 * \param account_number
 * \param transaction_number
 *
 * \return
 * */
void gsb_form_transaction_fill_form ( gint element_number,
				      gint account_number,
				      gint transaction_number )
{
    GtkWidget *widget;
    gchar *char_tmp;

    widget =  gsb_form_get_element_widget (element_number,
					   account_number);
    if (!widget)
	return;

    switch (element_number)
    {
	case TRANSACTION_FORM_OP_NB:
	    gtk_label_set_text ( GTK_LABEL ( widget ),
				 char_tmp = utils_str_itoa (transaction_number));
	    g_free (char_tmp);
	    break;

	case TRANSACTION_FORM_DATE:
	    gsb_form_entry_get_focus (widget);
	    gtk_entry_set_text ( GTK_ENTRY ( widget ),
				 char_tmp = gsb_format_gdate ( gsb_data_transaction_get_date (transaction_number)));
	    g_free (char_tmp);
	    break;

	case TRANSACTION_FORM_VALUE_DATE:
	    if ( gsb_data_transaction_get_value_date (transaction_number))
	    {
		gsb_form_entry_get_focus (widget);
		gtk_entry_set_text ( GTK_ENTRY ( widget ),
				     char_tmp = gsb_format_gdate ( gsb_data_transaction_get_value_date (transaction_number)));
		g_free (char_tmp);
	    }
	    break;

	case TRANSACTION_FORM_EXERCICE:
	    gtk_option_menu_set_history (  GTK_OPTION_MENU ( widget ),
					   cherche_no_menu_exercice ( gsb_data_transaction_get_financial_year_number (transaction_number),
								      widget ));
	    break;

	case TRANSACTION_FORM_PARTY:
	    if ( gsb_data_transaction_get_party_number (transaction_number))
	    {
		gsb_form_entry_get_focus (widget);
		gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
					gsb_data_payee_get_name ( gsb_data_transaction_get_party_number (transaction_number), TRUE ));
	    }
	    break;

	case TRANSACTION_FORM_DEBIT:
	    if ( gsb_data_transaction_get_amount (transaction_number).mantissa )
	    {
		gsb_form_entry_get_focus (widget);
		gtk_entry_set_text ( GTK_ENTRY ( widget ),
				     char_tmp = gsb_real_get_string (gsb_real_abs (gsb_data_transaction_get_amount (transaction_number))));
		g_free (char_tmp);
	    }
	    break;

	case TRANSACTION_FORM_CREDIT:
	    if ( gsb_data_transaction_get_amount (transaction_number).mantissa >= 0 )
	    {
		gsb_form_entry_get_focus (widget);
		gtk_entry_set_text ( GTK_ENTRY ( widget ),
				     char_tmp = gsb_real_get_string (gsb_data_transaction_get_amount (transaction_number)));
		g_free (char_tmp);
	    }
	    break;

	case TRANSACTION_FORM_CATEGORY:
	    if ( gsb_data_transaction_get_breakdown_of_transaction (transaction_number))
	    {
		/* it's a breakdown of transaction */
		gsb_form_entry_get_focus (widget);
		gtk_combofix_set_text ( GTK_COMBOFIX (widget),
					_("Breakdown of transaction") );
	    }
	    else
	    {
		if ( gsb_data_transaction_get_transaction_number_transfer (transaction_number))
		{
		    /* it's a transfer */
		    gsb_form_entry_get_focus (widget);

		    if ( gsb_data_transaction_get_transaction_number_transfer (transaction_number)!= -1
			 &&
			 gsb_data_transaction_get_account_number_transfer (transaction_number)!= -1 )
		    {
			gtk_combofix_set_text ( GTK_COMBOFIX (widget),
						char_tmp = g_strconcat ( _("Transfer : "),
									 gsb_data_account_get_name (gsb_data_transaction_get_account_number_transfer (transaction_number)),
									 NULL ));
			g_free (char_tmp);
		    }
		    else
			gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
						_("Transfer : Deleted account") );
		}
		else
		{
		    /* it's a normal category */
		    char_tmp = gsb_data_category_get_name ( gsb_data_transaction_get_category_number (transaction_number),
							    gsb_data_transaction_get_sub_category_number (transaction_number),
							    NULL );
		    if (char_tmp)
		    {
			gsb_form_entry_get_focus (widget);
			gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
						char_tmp );
			g_free (char_tmp);
		    }
		}
	    }
	    break;

	case TRANSACTION_FORM_BUDGET:
	    char_tmp = gsb_data_budget_get_name ( gsb_data_transaction_get_budgetary_number (transaction_number),
						  gsb_data_transaction_get_sub_budgetary_number (transaction_number),
						  NULL );
	    if ( char_tmp )
	    {
		gsb_form_entry_get_focus (widget);
		gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
					char_tmp );
		g_free (char_tmp);
	    }
	    break;

	case TRANSACTION_FORM_NOTES:
	    if ( gsb_data_transaction_get_notes (transaction_number))
	    {
		gsb_form_entry_get_focus (widget);
		gtk_entry_set_text ( GTK_ENTRY ( widget ),
				     gsb_data_transaction_get_notes (transaction_number));
	    }
	    break;

	case TRANSACTION_FORM_TYPE:
	    if ( gsb_data_transaction_get_amount (transaction_number).mantissa < 0 )
		gsb_payment_method_create_combo_list ( widget,
						       GSB_PAYMENT_DEBIT,
						       account_number );
	    else
		gsb_payment_method_create_combo_list ( widget,
						       GSB_PAYMENT_CREDIT,
						       account_number );

	    if ( GTK_WIDGET_VISIBLE (widget))
	    {
		gint payment_number;

		payment_number = gsb_data_transaction_get_method_of_payment_number (transaction_number);
		if (gsb_payment_method_set_combobox_history ( widget,
							      payment_number,
							      account_number )
		    &&
		    gsb_payment_method_get_show_entry (payment_number, account_number))
		    gtk_widget_show (gsb_form_get_element_widget (TRANSACTION_FORM_CHEQUE,
								  account_number));
	    }
	    else
		gtk_widget_hide ( gsb_form_get_element_widget (TRANSACTION_FORM_CHEQUE,
							       account_number) );
	    break;

	case TRANSACTION_FORM_DEVISE:
	    gsb_currency_set_combobox_history ( widget,
						gsb_data_transaction_get_currency_number (account_number));
	    gsb_form_transaction_check_change_button ( gsb_data_transaction_get_currency_number (account_number),
						       account_number );
	    break;

	case TRANSACTION_FORM_BANK:
	    if ( gsb_data_transaction_get_bank_references (transaction_number))
	    {
		gsb_form_entry_get_focus (widget);
		gtk_entry_set_text ( GTK_ENTRY ( widget ),
				     gsb_data_transaction_get_bank_references (transaction_number));
	    }
	    break;

	case TRANSACTION_FORM_VOUCHER:
	    if ( gsb_data_transaction_get_voucher (transaction_number))
	    {
		gsb_form_entry_get_focus (widget);
		gtk_entry_set_text ( GTK_ENTRY ( widget ),
				     gsb_data_transaction_get_voucher (transaction_number));
	    }
	    break;

	case TRANSACTION_FORM_CONTRA:
	    if ( gsb_data_transaction_get_transaction_number_transfer (transaction_number)
		 &&
		 gsb_data_transaction_get_account_number_transfer (transaction_number) != -1 )
	    {
		if ( gsb_data_transaction_get_amount (transaction_number).mantissa < 0 )
		    gsb_payment_method_create_combo_list ( widget,
							   GSB_PAYMENT_CREDIT,
							   gsb_data_transaction_get_account_number_transfer (transaction_number));
		else
		    gsb_payment_method_create_combo_list ( widget,
							   GSB_PAYMENT_DEBIT,
							   gsb_data_transaction_get_account_number_transfer (transaction_number));

		if (GTK_WIDGET_VISIBLE (widget))
		{
		    gint contra_transaction_number;

		    contra_transaction_number = gsb_data_transaction_get_transaction_number_transfer (transaction_number);

		    if ( contra_transaction_number )
			gsb_payment_method_set_combobox_history ( widget,
								  gsb_data_transaction_get_method_of_payment_number (contra_transaction_number),
								  gsb_data_transaction_get_account_number (contra_transaction_number));
		}
	    }
	    break;

	case TRANSACTION_FORM_MODE:
	    if ( gsb_data_transaction_get_automatic_transaction (transaction_number))
		gtk_label_set_text ( GTK_LABEL ( widget ),
				     _("Auto"));
	    else
		gtk_label_set_text ( GTK_LABEL ( widget ),
				     _("Manual"));
	    break;
    }
}


/**
 * check if the currency given in param is the same of the account currency
 * or if there is a link between them
 * if not, show the change button
 *
 * \param currency_number the currency of the transaction
 * \param account_number 
 *
 * \return
 * */
void gsb_form_transaction_check_change_button ( gint currency_number,
						gint account_number )
{
    gint account_currency_number;

    account_currency_number = gsb_data_account_get_currency (account_number);

    if ( gsb_data_currency_link_search ( account_currency_number,
					 currency_number ))
	gtk_widget_hide ( gsb_form_get_element_widget (TRANSACTION_FORM_CHANGE,
						       account_number) );
    else
	gtk_widget_show ( gsb_form_get_element_widget (TRANSACTION_FORM_CHANGE,
						       account_number) );
}



/**
 * Get a breakdown of transactions and ask if we want to clone the daughters
 * do the copy if needed, set for the daugthers the number of the new transaction
 * 
 * \param new_transaction_number the number of the new mother of the cloned transaction
 * \param no_last_breakdown the no of last breakdown mother
 * \param no_account the account of the last breakdown mother (important if it's not the same of the new_transaction)
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_recover_breakdowns_of_transaction ( gint new_transaction_number,
								   gint no_last_breakdown,
								   gint no_account )
{
    gint result;
    GSList *list_tmp_transactions;

    /* xxx check for win */
    result = question_conditional_yes_no ( "recover-breakdown" );

    if ( !result )
	return FALSE;

    /* go around the transactions list to get the daughters of the last breakdown */
    list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

    while ( list_tmp_transactions )
    {
	gint transaction_number_tmp;
	transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	if ( gsb_data_transaction_get_account_number (transaction_number_tmp) == no_account
	     &&
	     gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp) == no_last_breakdown)
	{
	    gsb_data_transaction_set_mother_transaction_number ( gsb_transactions_list_clone_transaction (transaction_number_tmp),
								 new_transaction_number);
	}
	list_tmp_transactions = list_tmp_transactions -> next;
    }
    return FALSE;
}



/** 
 * called when the user finishes the edition of a transaction, 
 * add/modify the transaction shown in the form
 * 
 * \param none
 * 
 * \return FALSE
 * */
gboolean gsb_form_finish_edition ( void )
{
    gint transaction_number;
    gint new_transaction;
    GSList *list_nb_parties;
    GSList *list_tmp;
    gint account_number;

    /* get the number of the transaction, stored in the form (0 if new) */
    transaction_number = GPOINTER_TO_INT (gtk_object_get_data ( GTK_OBJECT ( formulaire ),
								"transaction_number_in_form" ));
    account_number = gsb_form_get_account_number ();

    /* a new transaction is
     * either transaction_number is 0 (normal transaction)
     * either transaction_number is -2 (new breakdown daughter) */

    if ( transaction_number &&
	 transaction_number >= -1 )
	new_transaction = 0;
    else
	new_transaction = 1;

    /* the current widget has to lose the focus to make all the changes if necessary */
    gtk_widget_grab_focus ( gsb_transactions_list_get_tree_view()  );
    
    /* check if the datas are ok */
    if ( !gsb_form_validate_form_transaction ( transaction_number ))
	return FALSE;

    /* if the party is a report, we make as transactions as the number of parties in the
     * report. So we create a list with the party's numbers or -1 if it's a normal
     * party */
    list_nb_parties = gsb_form_get_parties_list_from_report ();

    /* now we go throw the list */
    list_tmp = list_nb_parties;

    while ( list_tmp )
    {
	if ( GPOINTER_TO_INT (list_tmp -> data) == -1 )
	    /* it's a normal party, we set the list_tmp to NULL */
	    list_tmp = NULL;
	else
	{
	    /* it's a report, so each time we come here we set the parti's combofix to the
	     * party of the report */

	    if ( !list_tmp -> data )
	    {
		dialogue_error ( _("No payee selected for this report."));
		return FALSE;
	    }
	    else
	    {
		gtk_combofix_set_text ( GTK_COMBOFIX ( gsb_form_get_element_widget (TRANSACTION_FORM_PARTY,
										    account_number ) ),
					gsb_data_payee_get_name ( GPOINTER_TO_INT (list_tmp -> data), TRUE ));

		/* if it's not the first party and the method of payment has to change its number (cheque),
		 * we increase the number. as we are in a party's list, it's always a new transactio, 
		 * so we know that it's not the first if transaction_number is not 0 */

		if ( transaction_number )
		{
		    gint payment_number;

		    payment_number = gsb_data_transaction_get_method_of_payment_number (transaction_number);

		    if ( gsb_payment_method_get_automatic_number ( payment_number,
								   account_number)
			 &&
			 gsb_data_form_check_for_value ( TRANSACTION_FORM_CHEQUE ))
			gtk_entry_set_text ( GTK_ENTRY ( gsb_form_get_element_widget (TRANSACTION_FORM_CHEQUE,
										    account_number) ),
					     gsb_payment_method_automatic_numbering_get_new_number ( payment_number,
												     account_number ));
		}
		list_tmp = list_tmp -> next;
	    }
	}

	if ( new_transaction )
	{
	    /* it's a new transaction, we create it, and put the mother if necessary */

	    gint mother_transaction;

	    /* if transaction exists already, it's that we are on a white daughter of a breakdown,
	     * so we keep the no of the mother */

	    if ( transaction_number )
		mother_transaction = gsb_data_transaction_get_mother_transaction_number (transaction_number);
	    else
		mother_transaction = 0;
	    
	    transaction_number = gsb_data_transaction_new_transaction (account_number);

	    gsb_data_transaction_set_mother_transaction_number ( transaction_number,
								 mother_transaction);
	}

	/* take the datas in the form, except the category */

	gsb_form_take_datas_from_form ( transaction_number );

	/* take the category and do the stuff with that (contra-transaction...) */

	gsb_form_get_categories ( transaction_number,
				  new_transaction );

	if ( new_transaction )
	    gsb_transactions_list_append_new_transaction (transaction_number);
	else
	    gsb_transactions_list_update_transaction (transaction_number);
    }
    g_slist_free ( list_nb_parties );

    /* if it's a reconciliation and we modify a transaction, check
     * the amount of marked transactions */
    if ( etat.equilibrage
	 &&
	 !new_transaction )
    {
	gsb_data_account_calculate_marked_balance (account_number);
	gsb_reconcile_update_amounts (account_number);
    }

    /* if it was a new transaction, do the stuff to do another new transaction */

    if ( new_transaction )
    {
	GtkWidget *date_entry;

	/* it was a new transaction, we save the last date entry */

	date_entry = gsb_form_get_element_widget (TRANSACTION_FORM_DATE,
						  account_number);
	gsb_date_set_last_date (gtk_entry_get_text ( GTK_ENTRY ( date_entry )));
	gsb_transactions_list_edit_current_transaction ();
    }
    else
    {
	gsb_form_clean (account_number);
	if ( !etat.formulaire_toujours_affiche )
	    gsb_form_hide ();
    }

    /* update the combofix's lists */
    if ( mise_a_jour_combofix_tiers_necessaire )
	mise_a_jour_combofix_tiers ();
    if ( mise_a_jour_combofix_categ_necessaire )
	mise_a_jour_combofix_categ ();
    if ( mise_a_jour_combofix_imputation_necessaire )
	mise_a_jour_combofix_imputation ();

    /* show the warnings */
    affiche_dialogue_soldes_minimaux ();
    update_transaction_in_trees (transaction_number);

    modification_fichier ( TRUE );
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
GSList *gsb_form_get_parties_list_from_report ( void )
{
    GSList *parties_list;
    gint account_number;

    account_number = gsb_form_get_account_number ();
    parties_list = NULL;

    /*     check that the party's form exist, else, append -1 and go away */

    if ( gsb_data_form_check_for_value (TRANSACTION_FORM_PARTY))
    {
	const gchar *string;

	string = gtk_combofix_get_text ( GTK_COMBOFIX ( gsb_form_get_element_widget (TRANSACTION_FORM_PARTY,
										     account_number)));

	if ( strncmp ( string,
		       _("Report : "),
		       7 ))
	    /* the party is not a report, set -1 and go away */
	    parties_list = g_slist_append (parties_list,
					   GINT_TO_POINTER (-1));
	else
	{
	    /* it's a report */

	    gchar **tab_char;
	    gint report_number;

	    tab_char = g_strsplit ( string,
				    " : ",
				    2 );

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
		parties_list = g_slist_append (parties_list,
					       GINT_TO_POINTER (-1));
	    g_strfreev ( tab_char );
	}
    }
    else
	/* no party so not a report */
	parties_list = g_slist_append ( parties_list,
					GINT_TO_POINTER (-1));
    return parties_list;
}


/**
 * check if the transaction in the form is correct
 *
 * \param transaction_number can be -2 for a new daughter scheduled transaction, 0 for a new transaction
 * 		or any number corresponding to an older transaction
 *
 * \return TRUE or FALSE
 * */
gboolean gsb_form_validate_form_transaction ( gint transaction_number )
{
    gint account_number;
    GtkWidget *widget;

    account_number = gsb_form_get_account_number ();

    /* begin to work with dates */
    widget = gsb_form_get_element_widget (TRANSACTION_FORM_DATE, account_number);

    /* check the date exists */
    if (gsb_form_check_entry_is_empty (widget))
    {
	dialogue_error ( _("You must enter a date.") );
	return (FALSE);
    }

    /* check the date ok */
    if ( !gsb_date_check_and_complete_entry (widget))
    {
	dialogue_error ( _("Invalid date") );
	gtk_entry_select_region ( GTK_ENTRY (widget),
				  0,
				  -1);
	gtk_widget_grab_focus (widget);
	return (FALSE);
    }

    /* work with value date */
    widget = gsb_form_get_element_widget (TRANSACTION_FORM_VALUE_DATE,
					  account_number);
    if ( widget
	 &&
	 !gsb_form_check_entry_is_empty (widget)
	 &&
	 !gsb_date_check_and_complete_entry (widget))
    {
	dialogue_error ( _("Invalid value date.") );
	gtk_entry_select_region ( GTK_ENTRY (widget),
				  0,
				  -1);
	gtk_widget_grab_focus (widget);
	return (FALSE);
    }

    /* now work with the categories */
    widget = gsb_form_get_element_widget (TRANSACTION_FORM_CATEGORY,
					  account_number);

    /* check if it's a daughter breakdown that the category is not a breakdown of transaction */
    if ( widget
	 &&
	 !gsb_form_check_entry_is_empty (GTK_COMBOFIX (widget) -> entry)
	 &&
	 gsb_data_transaction_get_mother_transaction_number (transaction_number)
	 &&
	 !strcmp ( gtk_combofix_get_text (GTK_COMBOFIX (widget)),
		   _("Breakdown of transaction")))
    {
	dialogue_error ( _("You cannot set breakdown of transaction in category for a daughter of a breakdown of transaction.") );
	return (FALSE);
    }

    /* check it's not a transfer on itself and the contra-account exists
     * !!! widget is already set to the category, don't change it before */

    if ( widget
	 &&
	 !gsb_form_check_entry_is_empty (GTK_COMBOFIX (widget) -> entry))
    {
	gint account_transfer;

	account_transfer = gsb_form_check_for_transfer (gtk_combofix_get_text (GTK_COMBOFIX (widget)));

	switch (account_transfer)
	{
	    /* if the check returns -3, it's not a transfer, so it's a normal category */
	    case -3:
		break;

		/* if the check returns -2, it's a deleted account */
	    case -2:
		gsb_data_transaction_set_category_number ( transaction_number,
							   0 );
		gsb_data_transaction_set_sub_category_number ( transaction_number,
							       0 );
		gsb_data_transaction_set_account_number_transfer ( transaction_number,
								   -1);
		gsb_data_transaction_set_transaction_number_transfer ( transaction_number,
								       -1);
		break;

		/* if the check returns -1, it's a non existant account */
	    case -1:
		dialogue_error ( _("There is no associated account for this transfer or associated account is invalid.") );
		return (FALSE);
		break;

		/* all other values are a number of account */
	    default :
		if (account_transfer == account_number)
		{
		    dialogue_error ( _("Can't issue a transfer its own account.") );
		    return (FALSE);
		}
		if ( gsb_data_account_get_closed_account (account_transfer) )
		{
		    dialogue_error ( _("Can't issue a transfer on a closed account." ));
		    return ( FALSE );
		}
	}
    }

    /* for the automatic method of payment entry (especially cheques)
     * check if not used before (only for new transaction) */

    widget = gsb_form_get_element_widget (TRANSACTION_FORM_CHEQUE,
					  account_number);

    if ( widget
	 &&
	 GTK_WIDGET_VISIBLE (widget))
    {
	GtkWidget *combo_box;
	gint payment;

	combo_box = gsb_form_get_element_widget (TRANSACTION_FORM_TYPE,
						 account_number);
	payment = gsb_payment_method_get_selected_number (combo_box);

	if (gsb_payment_method_get_automatic_number (payment, account_number))
	{
	    /* check if there is something in */

	    if (gsb_form_check_entry_is_empty (widget))
	    {
		if (!question ( _("Selected method of payment has an automatic incremental number\nbut doesn't contain any number.\nContinue anyway?")))
		    return (FALSE);
	    }
	    else
	    {
		/* check that the number is not used */
		gint tmp_transaction_number;
		
		tmp_transaction_number = gsb_data_transaction_find_by_payment_content ( gtk_entry_get_text (GTK_ENTRY (widget)),
											account_number );

		if ( tmp_transaction_number
		     &&
		     tmp_transaction_number != transaction_number
		     &&
		     !question ( _("Warning: this cheque number is already used.\nContinue anyway?")))
		    return FALSE;
	    }
	}
    }

    /* check if the payee is a report, it must be a new transaction */

    widget = gsb_form_get_element_widget (TRANSACTION_FORM_PARTY,
					  account_number);

    if (widget)
    {
	if (strncmp ( gtk_combofix_get_text (GTK_COMBOFIX (widget)),
		      _("Report"),
		      strlen (_("Report"))))
	{
	    gchar **tab_char;

	    /* check if it's a new transaction */
	    if (transaction_number > 0)
	    {
		dialogue_error ( _("A transaction with a multiple payee must be a new one.") );
		return (FALSE);
	    }
	    if (transaction_number < 0)
	    {
		dialogue_error ( _("A transaction with a multiple payee cannot be a breakdown child.") );
		return (FALSE);
	    }

	    /* check if the report exists */
	    tab_char = g_strsplit ( gtk_combofix_get_text (GTK_COMBOFIX (widget)),
				    " : ",
				    2 );
	    if (!gsb_data_report_get_report_by_name (tab_char[1]))
	    {
		g_strfreev (tab_char);
		dialogue_error ( _("Invalid multiple payee.") );
		return (FALSE);
	    }
	    g_strfreev (tab_char);
	}
	else
	{
	    dialogue_error  ( _("The word \"Report\" is reserved. Please use another one."));
	    return FALSE;
	}
    }
    return ( TRUE );
}

/**
 * take the datas in the form and set them in the transaction in param
 * 
 * \param transaction_number the transaction to modify
 * 
 * \return
 * */
void gsb_form_take_datas_from_form ( gint transaction_number )
{
    gint row, column;
    gint account_number;

    account_number = gsb_data_transaction_get_account_number (transaction_number);

    for ( row=0 ; row < gsb_data_form_get_nb_rows (account_number) ; row++ )
	for ( column=0 ; column <  gsb_data_form_get_nb_columns (account_number) ; column++ )
	{
	    gint value;
	    GtkWidget *widget;
	    gchar **tab_char;
	    gint tmp_int;

	    value = gsb_data_form_get_value ( account_number,
					      column,
					      row );

	    widget =  gsb_form_get_element_widget (value,
						   account_number);

	    switch (value)
	    {
		case TRANSACTION_FORM_DATE:
		    gsb_data_transaction_set_date ( transaction_number,
						    gsb_parse_date_string ( gtk_entry_get_text ( GTK_ENTRY (widget))));

		    break;

		case TRANSACTION_FORM_VALUE_DATE:

		    if (!gsb_form_check_entry_is_empty (widget)) 
			gsb_data_transaction_set_value_date ( transaction_number,
							      gsb_parse_date_string ( gtk_entry_get_text ( GTK_ENTRY (widget))));
		    else
			gsb_data_transaction_set_value_date ( transaction_number,
							      NULL );
		    break;
/* xxx en suis ici pour continuer à vérifier ce fichier une fois que fait les exercices */
		case TRANSACTION_FORM_EXERCICE:

		    /* if financial year is -1, we are on "not showed", so if it's a modification
		     * of transaction, no problem, if it's a new transaction, set it to 0 */
		    tmp_int = gsb_financial_year_get_number_from_option_menu (widget);
		    if ( tmp_int == -1 )
		    {
			if (transaction_number <= 0)
			    gsb_data_transaction_set_financial_year_number ( transaction_number,
									     0 );
		    }
		    else
			gsb_data_transaction_set_financial_year_number ( transaction_number,
									 tmp_int );

		    break;

		case TRANSACTION_FORM_PARTY:

		    if ( gtk_widget_get_style ( GTK_COMBOFIX ( widget ) -> entry ) == style_entree_formulaire[ENCLAIR] )
			gsb_data_transaction_set_party_number ( transaction_number,
								gsb_data_payee_get_number_by_name ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget )),
											       TRUE ));
		    else
			gsb_data_transaction_set_party_number ( transaction_number,
								0 );

		    break;

		case TRANSACTION_FORM_DEBIT:

		    if ( gtk_widget_get_style ( widget ) == style_entree_formulaire[ENCLAIR] )
			gsb_data_transaction_set_amount ( transaction_number,
							  gsb_real_opposite (gsb_utils_edit_calculate_entry ( widget )));
		    break;

		case TRANSACTION_FORM_CREDIT:

		    if ( gtk_widget_get_style ( widget ) == style_entree_formulaire[ENCLAIR] )
			gsb_data_transaction_set_amount ( transaction_number,
							  gsb_utils_edit_calculate_entry ( widget ));
		    break;

		case TRANSACTION_FORM_BUDGET:

		    tab_char = g_strsplit ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget )),
					    ":",
					    2 );

		    tab_char[0] = g_strstrip ( tab_char[0] );

		    if ( tab_char[1] )
			tab_char[1] = g_strstrip ( tab_char[1] );

		    if ( strlen ( tab_char[0] ) )
		    {
			gint budget_number;

			budget_number = gsb_data_budget_get_number_by_name ( g_strstrip (tab_char[0]),
									     TRUE,
									     gsb_data_transaction_get_amount (transaction_number).mantissa <0 );
			gsb_data_transaction_set_budgetary_number ( transaction_number,
								    budget_number );
			gsb_data_transaction_set_sub_budgetary_number ( transaction_number,
									gsb_data_budget_get_sub_budget_number_by_name ( budget_number,
															tab_char[1],
															TRUE ));
		    }
		    g_strfreev ( tab_char );
		    break;

		case TRANSACTION_FORM_NOTES:

		    if ( gtk_widget_get_style ( widget ) == style_entree_formulaire[ENCLAIR] )
			gsb_data_transaction_set_notes ( transaction_number,
							 g_strstrip ( my_strdup ( gtk_entry_get_text ( GTK_ENTRY ( widget )))));
		    else
			gsb_data_transaction_set_notes ( transaction_number,
							 NULL );
		    break;

		case TRANSACTION_FORM_TYPE:

		    if ( GTK_WIDGET_VISIBLE ( gsb_form_get_element_widget (TRANSACTION_FORM_TYPE,
									   account_number) ))
		    {
			gsb_data_transaction_set_method_of_payment_number ( transaction_number,
									    gsb_payment_method_get_selected_number (widget));

			if ( GTK_WIDGET_VISIBLE ( gsb_form_get_element_widget (TRANSACTION_FORM_CHEQUE,
									       account_number) )
			     &&
			     gtk_widget_get_style ( gsb_form_get_element_widget (TRANSACTION_FORM_CHEQUE,
										 account_number) ) == style_entree_formulaire[ENCLAIR] )
			{
			    struct struct_type_ope *type;

			    type = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget ) -> menu_item ),
							 "adr_type" );

			    gsb_data_transaction_set_method_of_payment_content ( transaction_number,
										 g_strstrip ( my_strdup ( gtk_entry_get_text ( GTK_ENTRY ( gsb_form_get_element_widget (TRANSACTION_FORM_CHEQUE,
																					account_number) )))));

			    if ( type -> numerotation_auto )
				type -> no_en_cours = ( utils_str_atoi ( gsb_data_transaction_get_method_of_payment_content ( transaction_number)));
			}
			else
			    gsb_data_transaction_set_method_of_payment_content ( transaction_number,
										  NULL);
		    }
		    else
		    {
			gsb_data_transaction_set_method_of_payment_number ( transaction_number,
									    0 );
			gsb_data_transaction_set_method_of_payment_content ( transaction_number,
									     NULL);
		    }
		    break;

		case TRANSACTION_FORM_DEVISE:

		    /* récupération de la devise */

		    gsb_data_transaction_set_currency_number ( transaction_number,
							       gsb_currency_get_currency_from_combobox (widget));

		    gsb_currency_check_for_change ( transaction_number );

		    break;

		case TRANSACTION_FORM_BANK:

		    if ( gtk_widget_get_style ( widget ) == style_entree_formulaire[ENCLAIR] )
			gsb_data_transaction_set_bank_references ( transaction_number,
								   g_strstrip ( my_strdup ( gtk_entry_get_text ( GTK_ENTRY ( widget )))));
		    else
			gsb_data_transaction_set_bank_references ( transaction_number,
								   NULL);
		    break;

		case TRANSACTION_FORM_VOUCHER:

		    if ( gtk_widget_get_style ( widget ) == style_entree_formulaire[ENCLAIR] )
			gsb_data_transaction_set_voucher ( transaction_number,
							   g_strstrip ( my_strdup ( gtk_entry_get_text ( GTK_ENTRY ( widget )))));
		    else
			gsb_data_transaction_set_voucher ( transaction_number,
							   NULL);

		    break;
	    }
	}
}


/** 
 * deal with the category in the form, append it in the transaction given in param
 * create the oter transaction if it's a transfer...
 * 
 * \param transaction_number the transaction which work with
 * \param new_transaction 1 if it's a new_transaction
 * 
 * \return FALSE
 * */
gboolean gsb_form_get_categories ( gint transaction_number,
				   gint new_transaction )
{
    GtkWidget *category_entry;

    devel_debug ( g_strdup_printf ( "gsb_form_get_categories %d, new : %d", 
				    transaction_number,
				    new_transaction ));

    if ( !gsb_data_form_check_for_value ( TRANSACTION_FORM_CATEGORY ))
	return FALSE;

    category_entry = GTK_COMBOFIX ( gsb_form_get_element_widget (TRANSACTION_FORM_CATEGORY,
								 gsb_form_get_account_number ()) ) -> entry;

    if (!gsb_form_check_entry_is_empty (category_entry))
    {
	const gchar *string;
	gint contra_transaction_number;

	string = gtk_entry_get_text ( GTK_ENTRY ( category_entry ));

	if ( strcmp ( string,
		      _("Breakdown of transaction") ))
	{
	    /* it's not a breakdown of transaction, if it was, we delete the
	     * transaction's daughters */

	    gint account_transfer;

	    if ( !new_transaction
		 &&
		 gsb_data_transaction_get_breakdown_of_transaction (transaction_number))
	    {
		GSList *list_tmp_transactions;
		list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

		while ( list_tmp_transactions )
		{
		    gint transaction_number_tmp;
		    transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

		    if ( gsb_data_transaction_get_account_number (transaction_number_tmp) == gsb_gui_navigation_get_current_account ()
			 &&
			 gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp) == transaction_number )
		    {
			list_tmp_transactions = list_tmp_transactions -> next;
			gsb_transactions_list_delete_transaction (transaction_number_tmp);
		    }
		    else
			list_tmp_transactions = list_tmp_transactions -> next;
		}
		gsb_data_transaction_set_breakdown_of_transaction ( transaction_number,
								    0 );
	    }

	    /* now, check if it's a transfer or a normal category */

	    account_transfer = gsb_form_check_for_transfer (string);

	    switch (account_transfer)
	    {
		/* if the check returns -2, it's not a transfer, so it's a normal category */
		case -3:
		    /* if it's a modification, check if before it was not a transfer */
		    if ( !new_transaction
			 &&
			 (contra_transaction_number = gsb_data_transaction_get_transaction_number_transfer (transaction_number)))
		    {
			/* it was a transfer, we delete the contra-transaction */

			gsb_data_transaction_set_transaction_number_transfer ( contra_transaction_number,
									       0);
			gsb_transactions_list_delete_transaction (contra_transaction_number );

			gsb_data_transaction_set_transaction_number_transfer ( transaction_number,
									       0);
			gsb_data_transaction_set_account_number_transfer ( transaction_number,
									   0);
		    }
		    gsb_data_category_fill_transaction_by_string ( transaction_number,
								   string );
		    break;

		    /* if the check returns -2, it's a deleted account */
		case -2:
		    gsb_data_transaction_set_account_number_transfer ( transaction_number,
								       -1);
		    gsb_data_transaction_set_transaction_number_transfer ( transaction_number,
									   -1);
		    /* we don't set any break here, so with the case -1 the 
		     * category will be set to 0 (!! let the case -1 after here) */

		    /* if the check returns -1, it's a transfert to non existant account,
		     * so do nothing */
		case -1:
		    gsb_data_transaction_set_category_number ( transaction_number,
							       0 );
		    gsb_data_transaction_set_sub_category_number ( transaction_number,
								   0 );
		    break;

		    /* all other values are a number of account */
		default :
		    gsb_form_validate_transfer ( transaction_number,
						 new_transaction,
						 account_transfer );
	    }
	}
	else
	{
	    /* it's a breakdown of transaction */
	    /* if it was a transfer, we delete the contra-transaction */

	    if ( !new_transaction
		 &&
		 ( contra_transaction_number = gsb_data_transaction_get_transaction_number_transfer (transaction_number)))
	    {
		gsb_data_transaction_set_transaction_number_transfer ( contra_transaction_number,
								       0);
		gsb_transactions_list_delete_transaction (contra_transaction_number );

		gsb_data_transaction_set_transaction_number_transfer ( transaction_number,
								       0);
		gsb_data_transaction_set_account_number_transfer ( transaction_number,
								   0);
	    }

	    gsb_data_transaction_set_breakdown_of_transaction ( transaction_number,
								1 );
	    gsb_data_transaction_set_category_number ( transaction_number,
						       0 );
	    gsb_data_transaction_set_sub_category_number ( transaction_number,
							   0 );

	    /*we will check here if there is another breakdown with the same party,
	     * if yes, we propose to copy the daughters transactions */

	    if ( new_transaction )
	    {
		gint breakdown_transaction_number;

		breakdown_transaction_number = gsb_form_transactions_look_for_last_party ( gsb_data_transaction_get_party_number (transaction_number),
											   transaction_number,
											   gsb_data_transaction_get_account_number(transaction_number));

		gsb_transactions_list_recover_breakdowns_of_transaction ( transaction_number,
									  breakdown_transaction_number,
									  gsb_data_transaction_get_account_number (breakdown_transaction_number));
	    }
	}
    }
    return FALSE;
}


/** validate a transfert from a form :
 * - create the contra-transaction
 * - delete the last contra-transaction if it's a modification
 * - append the contra-transaction to the tree view or update the tree_view
 * 
 * \param transaction_number the new transaction or the modify transaction
 * \param new_transaction TRUE if it's a new transaction
 * \param account_transfer the number of the account we want to create the contra-transaction
 * 
 * \return the number of the contra-transaction
 * */
gint gsb_form_validate_transfer ( gint transaction_number,
				  gint new_transaction,
				  gint account_transfer )
{
    gint contra_transaction_number;
    gint account_number;

    account_number = gsb_form_get_account_number ();

    g_return_val_if_fail ( account_transfer, -1 );

    /* either it's a new transfer or a change of a non-transfer transaction
     * either it was already a transfer, in that case, if we change the target account,
     * we delete the contra-transaction and it's the same as a new transfer */

    if ( !new_transaction )
    {
	/* it's a modification of a transaction */
	gsb_data_transaction_set_category_number ( transaction_number,
						   0 );
	gsb_data_transaction_set_sub_category_number ( transaction_number,
						       0 );

	/* as we will do a transfer, the category number is null */

	if ((contra_transaction_number = gsb_data_transaction_get_transaction_number_transfer (transaction_number)))
	{
	    /* the transaction is a transfer */

	    if ( gsb_data_transaction_get_account_number_transfer (transaction_number) != account_transfer )
	    {
		/* it was a transfer and the user changed the target account so we delete the last contra transaction
		 * contra_transaction_transfer has just been set */

		gsb_data_transaction_set_transaction_number_transfer ( contra_transaction_number,
								       0);
		gsb_transactions_list_delete_transaction (contra_transaction_number);
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
	contra_transaction_number = gsb_data_transaction_new_transaction (account_transfer);

    gsb_data_transaction_copy_transaction ( transaction_number,
					    contra_transaction_number );

    /* we have to change the amount by the opposite */

    gsb_data_transaction_set_amount (contra_transaction_number,
				     gsb_real_opposite (gsb_data_transaction_get_amount (transaction_number)));;

    /* we have to check the change */

    gsb_currency_check_for_change ( contra_transaction_number );

    /* set the method of payment
     * FIXME : for the scheduled transactions, we arrive here but there is no button for that,
     * so for now, TRANSACTION_FORM_CONTRA won't be visible so he just copy the method of the scheduled transaction */

    if ( gsb_form_get_element_widget (TRANSACTION_FORM_CONTRA,
				      account_number)
	 &&
	 GTK_IS_WIDGET ( gsb_form_get_element_widget (TRANSACTION_FORM_CONTRA,
						      account_number) )
	 &&
	 gsb_data_form_check_for_value ( TRANSACTION_FORM_CONTRA )
	 &&
	 GTK_WIDGET_VISIBLE ( gsb_form_get_element_widget (TRANSACTION_FORM_CONTRA,
							   account_number) ))
    {
	gsb_data_transaction_set_method_of_payment_number ( contra_transaction_number,
							    gsb_payment_method_get_selected_number (gsb_form_get_element_widget (TRANSACTION_FORM_CONTRA,
																 account_number)));
    }

    /* set the link between the transactions */

    gsb_data_transaction_set_transaction_number_transfer ( transaction_number,
							   contra_transaction_number);
    gsb_data_transaction_set_account_number_transfer ( transaction_number,
						       gsb_data_transaction_get_account_number (contra_transaction_number));
    gsb_data_transaction_set_transaction_number_transfer ( contra_transaction_number,
							   transaction_number);
    gsb_data_transaction_set_account_number_transfer ( contra_transaction_number,
						       gsb_data_transaction_get_account_number (transaction_number));

    /* show the contra_transaction */

    if ( new_transaction )
	gsb_transactions_list_append_new_transaction (contra_transaction_number);
    else
	gsb_transactions_list_update_transaction (contra_transaction_number);

    return contra_transaction_number;
}



/** 
 * append a new transaction in the tree_view
 * if the transaction is a breakdown, append a white line and open
 * the breakdown to see the daughters
 *
 * \param transaction_number
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_append_new_transaction ( gint transaction_number )
{
    GtkTreeStore *store;
    gint account_number;
    
    devel_debug ( g_strdup_printf ("gsb_transactions_list_append_new_transaction %d",
				   transaction_number ));

    store = gsb_transactions_list_get_store ();

    if ( !store)
	return FALSE;

    account_number = gsb_data_transaction_get_account_number (transaction_number);

    /* append the transaction to the tree view */

    gsb_transactions_list_append_transaction ( transaction_number,
					       store);

    /* if the transaction is a breakdown mother, we happen a white line,
     * which is a normal transaction but with nothing and with the breakdown
     * relation to the last transaction */

    if ( gsb_data_transaction_get_breakdown_of_transaction (transaction_number))
    {
	gint white_line_number;
	
	white_line_number = gsb_transactions_list_append_white_line ( transaction_number,
								      store);

	/* we select the white line of that breakdown */

	gsb_transactions_list_set_current_transaction (white_line_number);
    }	

    gsb_transactions_list_set_visibles_rows_on_transaction (transaction_number);
    gsb_transactions_list_set_background_color (account_number);
    gsb_transactions_list_set_transactions_balances (account_number);
    gsb_transactions_list_move_to_current_transaction (account_number);

    /*     calcul du solde courant */

    gsb_data_account_set_current_balance ( account_number,
					   gsb_real_add ( gsb_data_account_get_current_balance (account_number),
							  gsb_data_transaction_get_adjusted_amount (transaction_number, -1)));

    /* on met à jour les labels des soldes */

    mise_a_jour_labels_soldes ();

    /* on réaffichera l'accueil */

    mise_a_jour_liste_comptes_accueil = 1;
    mise_a_jour_soldes_minimaux = 1;
    mise_a_jour_fin_comptes_passifs = 1;
    return FALSE;
}




/** 
 * update the transaction given in the tree_view
 * 
 * \param transaction transaction to update
 * 
 * \return FALSE
 * */
gboolean gsb_transactions_list_update_transaction ( gint transaction_number )
{
    gint j;
    GtkTreeStore *store;
    GtkTreeIter *iter;
    gint account_number;

    devel_debug ( g_strdup_printf ( "gsb_transactions_list_update_transaction no %d",
				    transaction_number));

    account_number = gsb_data_transaction_get_account_number (transaction_number);
    store = gsb_transactions_list_get_store();
    iter = gsb_transactions_list_get_iter_from_transaction (transaction_number,
							    0 );

    for ( j = 0 ; j < TRANSACTION_LIST_ROWS_NB ; j++ )
    {
	gsb_transactions_list_fill_row ( transaction_number,
					 iter,
					 store,
					 j );

	/* if it's a breakdown, there is only 1 line */

	if ( transaction_number != -1
	     &&
	     gsb_data_transaction_get_mother_transaction_number (transaction_number))
	    j = TRANSACTION_LIST_ROWS_NB;

	gtk_tree_model_iter_next ( GTK_TREE_MODEL (store),
				   iter );
    }

    gtk_tree_iter_free ( iter );

    gsb_transactions_list_set_transactions_balances (account_number);
    /*     calcul du solde courant */

    gsb_data_account_set_current_balance ( account_number,
					   gsb_real_add ( gsb_data_account_get_current_balance (account_number),
							  gsb_data_transaction_get_adjusted_amount (transaction_number, -1)));

    /* on met à jour les labels des soldes */

    mise_a_jour_labels_soldes ();

    /* on réaffichera l'accueil */

    mise_a_jour_liste_comptes_accueil = 1;
    mise_a_jour_soldes_minimaux = 1;
    mise_a_jour_fin_comptes_passifs = 1;

    return FALSE;
}



/******************************************************************************/
/* Fonction affiche_cache_le_formulaire                                       */
/* si le formulaire était affichÃ©, le cache et vice-versa                     */
/******************************************************************************/


void affiche_cache_le_formulaire ( void )
{
    GtkWidget * widget;

    if ( etat.formulaire_toujours_affiche )
    {
	etat.formulaire_toujours_affiche = 0;
    }
    else
    {
	etat.formulaire_toujours_affiche = 1;

	update_ecran ();

/* 	ajustement = gtk_tree_view_get_vadjustment ( GTK_TREE_VIEW ( gsb_transactions_list_get_tree_view() ) )); */
	
/* 	position_ligne_selectionnee = ( cherche_ligne_operation ( gsb_data_account_get_current_transaction (gsb_data_account_get_current_account ()), */
/* 								  gsb_data_account_get_current_account () ) */
/* 					+ gsb_data_account_get_nb_rows ( gsb_data_account_get_current_account () ) ) * hauteur_ligne_liste_opes; */

/* 	if ( position_ligne_selectionnee  > (ajustement->value + ajustement->page_size)) */
/* 	    gtk_adjustment_set_value ( ajustement, */
/* 				       position_ligne_selectionnee - ajustement->page_size ); */
    }

    /* FIXME : ça va changer pour le form */
    if ( etat.formulaire_toujours_affiche )
	gsb_form_show (TRUE);
    else
	gsb_form_hide ();

    block_menu_cb = TRUE;
    widget = gtk_item_factory_get_item ( item_factory_menu_general,
					 menu_name(_("View"), _("Show transaction form"), NULL) );
    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM (widget), etat.formulaire_toujours_affiche );
    block_menu_cb = FALSE;

}



/******************************************************************************/
/* Fonction click_sur_bouton_voir_change  */
/* permet de modifier un change établi pour une opération */
/******************************************************************************/
void click_sur_bouton_voir_change ( void )
{
    gint transaction_number;
    gint account_number;
    gint currency_number;
    gint account_currency_number;
    gsb_real exchange, exchange_fees;

    account_number = gsb_form_get_account_number ();
    gtk_widget_grab_focus ( gsb_form_get_element_widget (TRANSACTION_FORM_DATE,
							 account_number ));

    transaction_number = GPOINTER_TO_INT (gtk_object_get_data ( GTK_OBJECT ( formulaire ),
								"transaction_number_in_form" ));

    account_currency_number = gsb_data_account_get_currency (account_number);
    currency_number = gsb_data_transaction_get_currency_number (transaction_number);

    gsb_currency_exchange_dialog ( account_currency_number, currency_number,
				   gsb_data_transaction_get_change_between (transaction_number),
				   gsb_data_transaction_get_exchange_rate (transaction_number),
				   gsb_data_transaction_get_exchange_fees (transaction_number), 
				   TRUE );

    exchange = gsb_currency_get_current_exchange ();
    exchange_fees = gsb_currency_get_current_exchange_fees ();

    if ( exchange.mantissa || exchange_fees.mantissa )
    {
	gsb_data_transaction_set_exchange_rate (transaction_number,
						 gsb_real_abs (exchange));
	gsb_data_transaction_set_exchange_fees (transaction_number,
						exchange_fees );

	if ( exchange.mantissa < 0 )
	    gsb_data_transaction_set_change_between (transaction_number,
						     1 );
	else
	    gsb_data_transaction_set_change_between (transaction_number,
						      0 );
    }
}
/******************************************************************************/


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
