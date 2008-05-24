/* ************************************************************************** */
/* This file manage cvs export format                                         */
/*                                                                            */
/*     Copyright (C)	2004 François Terrot (francois.terrot at grisbi.org) */
/*			2005 Alain Portal (aportal@univ-montp2.fr)	      */
/*			2006 Benjamin Drieu (bdrieu@april.org)		      */
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
 * \todo make the CSV parameter configurable 
 */

#include "include.h"

/*START_INCLUDE*/
#include "export_csv.h"
#include "./dialog.h"
#include "./gsb_data_account.h"
#include "./gsb_data_archive_store.h"
#include "./gsb_data_budget.h"
#include "./gsb_data_currency.h"
#include "./gsb_data_fyear.h"
#include "./gsb_data_payee.h"
#include "./gsb_data_payment.h"
#include "./gsb_data_reconcile.h"
#include "./gsb_data_transaction.h"
#include "./gsb_file_util.h"
#include "./gsb_real.h"
#include "./main.h"
#include "./utils_str.h"
#include "./utils_files.h"
#include "./include.h"
#include "./gsb_real.h"
/*END_INCLUDE*/

/*START_STATIC*/
static  void csv_add_record(FILE* file,
			   gboolean clear_all,
			   gboolean print_balance );
static  void csv_clear_fields(gboolean clear_all);
static  FILE *gsb_csv_export_open_file ( const gchar *filename );
static  gboolean gsb_csv_export_title_line ( FILE *csv_file,
					    gboolean print_balance );
static  gboolean gsb_csv_export_transaction ( gint transaction_number,
					     FILE *csv_file,
					     gboolean print_balance );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/



/**
 * \brief Clear a record field to default empty value.
 *
 * Free the memory used by the field value.
 *
 * \param a filed poointed 
 */
#define CSV_CLEAR_FIELD(a)  if (a) { g_free(a);  a = NULL; }



/** 
 * \brief write a string field.
 * \internal
 *
 * The string field is quoted and converted from utf8 to locale charset.
 * A end of field character id added after the field
 * Also manage to add a empty string if field is empty
 *
 * \param   f   valid file stream to write
 * \param   a   string field to add.
 *
 */ 
#define CSV_STR_FIELD(f,a)  if (a) { fprintf(f,"\"%s\"%c",g_locale_from_utf8(a,-1,NULL,NULL,NULL),g_csv_field_separator); } else { fprintf(f,"\"\"%c",g_csv_field_separator); }



/**
 * \brief Write a numerical field.
 * \internal
 *
 * A end of field character id added after the field
 * A 0 (zero) is put by default if the field is empty.
 *
 * \param   f   valid file stream to write
 * \param   a   numerical field to add.
 */
#define CSV_NUM_FIELD(f,a)  if (a) { fprintf(f,"%s%c",g_locale_from_utf8(a,-1,NULL,NULL,NULL),g_csv_field_separator); } else { fprintf(f,"0%c",g_csv_field_separator); }



/**
 * \brief Write the end of record character
 * 
 * \param   f   valid file stream to write
 */
#define CSV_END_RECORD(f)  fprintf(f,"\n")

static gboolean g_csv_with_title_line = TRUE; /*!< CSV configuration - does the file result contains a title line ?  */
static gchar    g_csv_field_separator = ';';  /*!< CSV configuration - separator charater used between fields of a record */

static gsb_real current_balance;

gchar*  csv_field_operation  = NULL; /*!< operation number (numerical) */
gchar*	csv_field_account    = NULL; /*!< account name */
gchar*  csv_field_ventil     = NULL; /*!< is operation a breakdown (string) */
gchar*  csv_field_date       = NULL; /*!< date of operation (of main operation for breakdown) (string) */
gchar*  csv_field_date_val   = NULL; /*!< value date of operation (of main operation for breakdown) (string) */
gchar*  csv_field_pointage   = NULL; /*!< pointed/reconcialiation status (string) */
gchar*  csv_field_tiers      = NULL; /*!< Payee (string) */
gchar*  csv_field_credit     = NULL; /*!< credit (numerical) */
gchar*  csv_field_debit      = NULL; /*!< debit (numerical) */
gchar*  csv_field_montant    = NULL; /*!< amount (numerical) only used for breakdown */
gchar*  csv_field_solde      = NULL; /*!< balance (numerical) */
gchar*  csv_field_categ      = NULL; /*!< category (string) */
gchar*  csv_field_sous_categ = NULL; /*!< sub category (string) */
gchar*  csv_field_imput      = NULL; /*!< budgetary line (string) */
gchar*  csv_field_sous_imput = NULL; /*!< sub budgetary line (string) */
gchar*  csv_field_notes      = NULL; /*!< notes (string) */
gchar*  csv_field_exercice   = NULL; /*!< exercices (string) optional depending of global grisbi configuration */
gchar*  csv_field_piece      = NULL; /*!< (string) */
gchar*  csv_field_cheque     = NULL; /*!< cheques */
gchar*  csv_field_rappro     = NULL; /*!< reconciliation number (string) */
gchar*  csv_field_info_bank  = NULL; /*!< bank references (string) */



/**
 * \brief clear temporary variable used to store field to display.
 *
 * \internal
 *
 * This function is to used before computing a new operation.
 * 
 * The function is able to reset all or just a part of the variable
 * depending of the need - typically the date is not reset when reading
 * breakdown operation items.
 * 
 * \param clear_all partial or complete cleaning.
 *
 */
static void csv_clear_fields(gboolean clear_all)
{ /* {{{ */
  if (clear_all)
  {
    CSV_CLEAR_FIELD(csv_field_date);
    CSV_CLEAR_FIELD(csv_field_date_val);
    CSV_CLEAR_FIELD(csv_field_pointage);
    CSV_CLEAR_FIELD(csv_field_operation);
    CSV_CLEAR_FIELD(csv_field_account);
    CSV_CLEAR_FIELD(csv_field_tiers);
    CSV_CLEAR_FIELD(csv_field_solde);
  }

  CSV_CLEAR_FIELD(csv_field_notes);
  CSV_CLEAR_FIELD(csv_field_debit);
  CSV_CLEAR_FIELD(csv_field_credit);
  CSV_CLEAR_FIELD(csv_field_montant);
  CSV_CLEAR_FIELD(csv_field_ventil);
  CSV_CLEAR_FIELD(csv_field_categ);
  CSV_CLEAR_FIELD(csv_field_sous_categ);
  CSV_CLEAR_FIELD(csv_field_imput);
  CSV_CLEAR_FIELD(csv_field_sous_imput);
  CSV_CLEAR_FIELD(csv_field_exercice);
  CSV_CLEAR_FIELD(csv_field_piece);
  CSV_CLEAR_FIELD(csv_field_cheque);
  CSV_CLEAR_FIELD(csv_field_info_bank);
  CSV_CLEAR_FIELD(csv_field_rappro);
} /* }}} csv_clear_fields */



/**
 * \brief Write down the current csv record.
 * 
 * \internal
 * 
 * The function appends the current csv record values followed by a end of record
 * in the given file and then clean all fields (if requested)
 * Depending of a global grisbi configuration some field may not be written (like exercice one)
 * 
 * \param file      valid file stream to write
 * \param clear_all partial or complete cleaning.
 * \param print_balance print the balance or not
 * 
 */ 
static void csv_add_record(FILE* file,
			   gboolean clear_all,
			   gboolean print_balance )
{ /* {{{ */
  CSV_NUM_FIELD(file,csv_field_operation);
  CSV_STR_FIELD(file,csv_field_account);
  CSV_STR_FIELD(file,csv_field_ventil);
  CSV_STR_FIELD(file,csv_field_date);
  CSV_STR_FIELD(file,csv_field_date_val);
  CSV_STR_FIELD(file,csv_field_cheque);
  CSV_STR_FIELD(file,csv_field_exercice);
  CSV_STR_FIELD(file,csv_field_pointage);
  CSV_STR_FIELD(file,csv_field_tiers);
  CSV_NUM_FIELD(file,csv_field_credit);
  CSV_NUM_FIELD(file,csv_field_debit);
  CSV_NUM_FIELD(file,csv_field_montant);
  if (print_balance)
  {
      CSV_NUM_FIELD(file,csv_field_solde);
  }
  CSV_STR_FIELD(file,csv_field_categ);
  CSV_STR_FIELD(file,csv_field_sous_categ);
  CSV_STR_FIELD(file,csv_field_imput);
  CSV_STR_FIELD(file,csv_field_sous_imput);
  CSV_STR_FIELD(file,csv_field_notes);
  CSV_STR_FIELD(file,csv_field_piece);
  CSV_STR_FIELD(file,csv_field_rappro);
  CSV_STR_FIELD(file,csv_field_info_bank);
  CSV_END_RECORD(file);
  csv_clear_fields(clear_all);
} /* }}} csv_add_record */



#define EMPTY_STR_FIELD fprintf(csv_file,"\"\""); /*!< empty string field value */
#define EMPTY_NUM_FIELD fprintf(csv_file,"0");    /*!< empty numerical field value */



/**
 * \brief export all account from the provided list in a csv file per account.
 *
 * \param export_entries_list list of selected account.
 *
 */
/**
 * export an account into a csv file
 *
 * \param filename
 * \param account_nb the account to export
 *
 * \return TRUE if ok, FALSE if problem
 * */
gboolean gsb_csv_export_account ( const gchar *filename, gint account_nb )
{
    FILE *csv_file;
    GSList *pTransactionList;
    GSList *tmp_list;

    csv_file = gsb_csv_export_open_file (filename);

    if (!csv_file)
	return FALSE;

    if (g_csv_with_title_line)
	gsb_csv_export_title_line (csv_file, TRUE);

    /* set the initial balance */
    gchar* tmpstr = g_strconcat (_("Initial balance") , " [", 
						     gsb_data_account_get_name(account_nb),
						     "]", NULL );
    if ( csv_field_tiers )
        g_free ( csv_field_tiers );
    /* TODO dOm : is it necessary to duplicate memory with g_strdup_printf since it was already newly allocated memory ? */
    csv_field_tiers = g_strdup_printf ( tmpstr );
    g_free ( tmpstr );

    /* set the initial current_balance,
     * as we will write all the non archived transactions,
     * we need to get the initial balance of the account, without the archived transactions */
    current_balance = gsb_data_account_get_init_balance ( account_nb, -1);
    tmp_list = gsb_data_archive_store_get_archives_list ();
    while (tmp_list)
    {
	gint archive_store_number;

	archive_store_number = gsb_data_archive_store_get_number ( tmp_list -> data );

	if (gsb_data_archive_store_get_account_number (archive_store_number) == account_nb)
	    current_balance = gsb_real_add ( current_balance,
					     gsb_data_archive_store_get_balance (archive_store_number));
	tmp_list = tmp_list -> next;
    }

    /* ok the balance is now good, can write it */
    CSV_CLEAR_FIELD (csv_field_solde);
    csv_field_solde = gsb_real_get_string (current_balance);
    if ( current_balance.mantissa >= 0 )
    {
	CSV_CLEAR_FIELD (csv_field_credit);
	csv_field_credit = gsb_real_get_string (current_balance);
    }
    else
    {
	CSV_CLEAR_FIELD (csv_field_debit);
	csv_field_debit = gsb_real_get_string (gsb_real_abs (current_balance));
    }

    csv_add_record(csv_file,TRUE, TRUE);

    /* export the transactions */
    pTransactionList = gsb_data_transaction_get_transactions_list ();
    while ( pTransactionList )
    {
	gint pTransaction = gsb_data_transaction_get_transaction_number (pTransactionList -> data);

	if ( gsb_data_transaction_get_account_number ( pTransaction ) == account_nb )
	    /* export the transaction */
	    /* for now, print the balance. is this usefull ? */
	    gsb_csv_export_transaction ( pTransaction, csv_file, TRUE);

	pTransactionList = pTransactionList -> next;
    }
    fclose ( csv_file );

    return TRUE;
}

/**
 * export an archive into the csv format
 *
 * \param filename name of the csv file
 * \param archive_number the archive to export
 *
 * \return TRUE ok, FALSE problem
 * */
gboolean gsb_csv_export_archive ( const gchar *filename,
				  gint archive_number )
{
    FILE *csv_file;
    GSList *pTransactionList;

    csv_file = gsb_csv_export_open_file (filename);

    if (!csv_file)
	return FALSE;

    if (g_csv_with_title_line)
	gsb_csv_export_title_line (csv_file, FALSE);

    /* set all the transactions for that archive */
    pTransactionList = gsb_data_transaction_get_complete_transactions_list ();
    while ( pTransactionList )
    {
	gint pTransaction = gsb_data_transaction_get_transaction_number (pTransactionList -> data);

	if (gsb_data_transaction_get_archive_number (pTransaction) == archive_number )
	/* export the transaction */
	gsb_csv_export_transaction ( pTransaction, csv_file, FALSE);

	pTransactionList = pTransactionList -> next;
    }
    fclose ( csv_file );

    return TRUE;
}



/**
 * try to open the csv file in w mode
 *
 * \param filename
 *
 * \return a FILE pointer or NULL if problem
 * */
static FILE *gsb_csv_export_open_file ( const gchar *filename )
{
    FILE *csv_file;
    gchar *sMessage = NULL;

    if (!gsb_file_util_test_overwrite (filename))
	return NULL;

    /* Ouverture du fichier, si pb, on marque l'erreur et passe au fichier suivant */
    if ( !( csv_file = utf8_fopen ( filename, "w" ) ))
    {
	sMessage = g_strdup_printf ( _("Error opening file \"%s\" :\n%s"),
				     filename, strerror ( errno ) );
	dialogue ( sMessage );
	g_free ( sMessage );
	return NULL;
    }

    return csv_file;
}


/**
 * export a transaction given in param in the file given in param
 *
 * \param transaction_number
 * \param csv_file
 * \param print_balance will set a balance for each transaction in the csv file
 * 		not set for archive export, set (but usefull ?) for account export
 *
 * \return TRUE ok, FALSE problem
 * */
static gboolean gsb_csv_export_transaction ( gint transaction_number,
					     FILE *csv_file,
					     gboolean print_balance )
{
    gsb_real amount;
    gint return_exponent;
    gint account_number;

    account_number = gsb_data_transaction_get_account_number (transaction_number);
    return_exponent = gsb_data_currency_get_floating_point (gsb_data_account_get_currency (account_number));

    /* Si c'est une ventilation d'opération (càd une opération fille),
       elle n'est pas traitée à la base du "if" mais plus loin, quand
       son opé ventilée sera exportée */
    if ( ! gsb_data_transaction_get_mother_transaction_number (transaction_number) )
    {
	GDate * value_date, * date;
	gint payment_method;

	/* met la date */
	date = gsb_data_transaction_get_date ( transaction_number );
	if ( date )
	{
	    CSV_CLEAR_FIELD (csv_field_date);
	    csv_field_date = g_strdup_printf ("%d/%d/%d", 
					      g_date_day ( date ), 
					      g_date_month ( date ),
					      g_date_year ( date ) );
	}

	value_date = gsb_data_transaction_get_value_date ( transaction_number );
	if ( value_date )
	{
	    CSV_CLEAR_FIELD (csv_field_date_val);
	    csv_field_date_val = g_strdup_printf ("%d/%d/%d", 
						  g_date_day ( value_date ), 
						  g_date_month ( value_date ),
						  g_date_year ( value_date ) );
	}

	/* met le pointage */
        CSV_CLEAR_FIELD (csv_field_pointage);
	switch ( gsb_data_transaction_get_marked_transaction ( transaction_number ) )
	{
	    case 0: /* CHECKED_TRANSACTION */
		csv_field_pointage = my_strdup (_("C"));
		break;
	    case 1: /* RECONCILED_TRANSACTION */
		csv_field_pointage = my_strdup (_("R"));
		break;
	    case 2: /* TELECHECKED_TRANSACTION */
		csv_field_pointage = my_strdup (_("T"));
		break;
	}

	/* met les notes */
	CSV_CLEAR_FIELD(csv_field_notes);
	if ( gsb_data_transaction_get_notes ( transaction_number ) )
	    csv_field_notes = my_strdup (gsb_data_transaction_get_notes ( transaction_number ));

	/* met le tiers */
	CSV_CLEAR_FIELD(csv_field_tiers);
	csv_field_tiers = g_strdup ( gsb_data_payee_get_name ( gsb_data_transaction_get_party_number ( transaction_number ), FALSE ) );

	/* met le numero du rapprochement */
	if ( gsb_data_transaction_get_reconcile_number ( transaction_number ) )
	{
	    CSV_CLEAR_FIELD (csv_field_rappro);
	    csv_field_rappro = my_strdup ( gsb_data_reconcile_get_name ( gsb_data_transaction_get_reconcile_number ( transaction_number ) ) );
	}

	/* Met les informations bancaires de l'opération. Elles n'existent
	   qu'au niveau de l'opération mère */
	CSV_CLEAR_FIELD(csv_field_info_bank);
	if ( gsb_data_transaction_get_bank_references ( transaction_number ) )
	{
	    csv_field_info_bank = my_strdup ( gsb_data_transaction_get_bank_references ( transaction_number ) );
	}

	/* met le montant, transforme la devise si necessaire */
	amount = gsb_data_transaction_get_adjusted_amount ( transaction_number,
							    return_exponent);
	CSV_CLEAR_FIELD (csv_field_credit);
	if (amount.mantissa >= 0 )
	    csv_field_credit = gsb_real_get_string (amount);
	else
	    csv_field_debit  = gsb_real_get_string (gsb_real_abs (amount));

	/* met le cheque si c'est un type a�numerotation automatique */
	payment_method = gsb_data_transaction_get_method_of_payment_number ( transaction_number );
	CSV_CLEAR_FIELD (csv_field_cheque);
	if (gsb_data_payment_get_automatic_numbering (payment_method))
	    csv_field_cheque = my_strdup ( gsb_data_transaction_get_method_of_payment_content ( transaction_number ) );

	if ( gsb_data_transaction_get_budgetary_number ( transaction_number ) != -1 )
	{
	    CSV_CLEAR_FIELD (csv_field_imput);
	    csv_field_imput = my_strdup ( gsb_data_budget_get_name ( gsb_data_transaction_get_budgetary_number ( transaction_number ), 0, "" ) );

	    if ( gsb_data_transaction_get_sub_budgetary_number ( transaction_number ) != -1 )
	    {
		CSV_CLEAR_FIELD (csv_field_sous_imput);
		csv_field_sous_imput = my_strdup ( gsb_data_budget_get_sub_budget_name ( gsb_data_transaction_get_budgetary_number ( transaction_number ),
											 gsb_data_transaction_get_sub_budgetary_number ( transaction_number ),
											 NULL ) );
	    }
	}

	/* Piece comptable */
	CSV_CLEAR_FIELD (csv_field_piece);
	csv_field_piece = my_strdup ( gsb_data_transaction_get_voucher ( transaction_number ) );

	/* Balance */
	if (print_balance)
	{
	    current_balance = gsb_real_add ( current_balance,
					     amount );
	    CSV_CLEAR_FIELD (csv_field_solde);
	    csv_field_solde = gsb_real_get_string (current_balance);
	}

	/* Number */
	CSV_CLEAR_FIELD (csv_field_operation);
	csv_field_operation = g_strdup_printf("%d", transaction_number );

	/* Account name */
	CSV_CLEAR_FIELD (csv_field_account);
	csv_field_account = my_strdup (gsb_data_account_get_name (account_number));

	/* Financial Year */
	if ( gsb_data_transaction_get_financial_year_number ( transaction_number ) != -1 )
	{
	    CSV_CLEAR_FIELD (csv_field_exercice );
	    csv_field_exercice  = my_strdup (gsb_data_fyear_get_name(gsb_data_transaction_get_financial_year_number ( transaction_number )));
	}

	/*  on met soit un virement, soit une ventilation, soit les catégories */

	/* Si c'est une opération ventilée, on recherche toutes les ventilations
	   de cette opération et on les traite immédiatement. */
	/* et les met à la suite */
	/* la catégorie de l'opé sera celle de la première opé de ventilation */
	if ( gsb_data_transaction_get_breakdown_of_transaction ( transaction_number ) )
	{
	    GSList *pBreakdownTransactionList;

	    CSV_CLEAR_FIELD (csv_field_categ);
	    csv_field_categ = my_strdup (_("Breakdown of transaction"));
	    csv_add_record(csv_file,FALSE, print_balance);

	    pBreakdownTransactionList = gsb_data_transaction_get_transactions_list ();

	    while ( pBreakdownTransactionList )
	    {
		gint pBreakdownTransaction;

		pBreakdownTransaction = (gint) pBreakdownTransactionList -> data;

		if ( gsb_data_transaction_get_account_number ( pBreakdownTransaction ) == gsb_data_transaction_get_account_number (transaction_number)
		     &&
		     gsb_data_transaction_get_mother_transaction_number ( pBreakdownTransaction ) == transaction_number )
		{
		    /* on commence par mettre la catég et sous categ de l'opé et de l'opé de ventilation */
		    CSV_CLEAR_FIELD (csv_field_ventil);
		    csv_field_ventil = my_strdup (_("B")); // -> mark 

		    CSV_CLEAR_FIELD (csv_field_operation);
		    csv_field_operation = g_strdup_printf("%d", pBreakdownTransaction );

		    if ( gsb_data_transaction_get_contra_transaction_number ( pBreakdownTransaction ) )
		    {
			/* c'est un virement */
			CSV_CLEAR_FIELD (csv_field_categ);
			csv_field_categ = my_strdup (_("Transfer"));

			gchar* tmpstr = g_strconcat ( "[", gsb_data_account_get_name ( gsb_data_transaction_get_contra_transaction_account ( pBreakdownTransaction ) ), "]", NULL );
			/* TODO dOm : is it necessary to duplicate memory with my_strdup since it was already newly allocated memory ? */
			CSV_CLEAR_FIELD (csv_field_sous_categ);
			csv_field_sous_categ = my_strdup (tmpstr);
			g_free ( tmpstr );
		    }
		    else
		    {
			if ( gsb_data_transaction_get_category_number ( pBreakdownTransaction ) != -1 )
			{
			    CSV_CLEAR_FIELD (csv_field_categ);
			    csv_field_categ = my_strdup ( gsb_data_budget_get_name ( gsb_data_transaction_get_category_number ( pBreakdownTransaction ), 0, "" ) );

			    if ( gsb_data_transaction_get_sub_category_number ( pBreakdownTransaction ) != -1 )
			    {
				CSV_CLEAR_FIELD (csv_field_sous_categ);
				csv_field_sous_categ = my_strdup ( gsb_data_budget_get_sub_budget_name ( gsb_data_transaction_get_category_number ( pBreakdownTransaction ),
													 gsb_data_transaction_get_sub_category_number ( pBreakdownTransaction ),
													 NULL ) );
			    }
			}

		    }

		    /* met les notes de la ventilation */
		    if ( gsb_data_transaction_get_notes ( pBreakdownTransaction ) )
		    {
			CSV_CLEAR_FIELD (csv_field_notes);
			csv_field_notes = my_strdup (gsb_data_transaction_get_notes ( pBreakdownTransaction ));
		    }

		    /* met le montant de la ventilation */
		    amount = gsb_data_transaction_get_adjusted_amount ( pBreakdownTransaction, return_exponent );
		    CSV_CLEAR_FIELD (csv_field_montant);
		    csv_field_montant = gsb_real_get_string (amount);

		    /* met le rapprochement */
		    if ( gsb_data_transaction_get_reconcile_number ( pBreakdownTransaction ) )
		    {
			CSV_CLEAR_FIELD (csv_field_rappro);
			csv_field_rappro = my_strdup ( gsb_data_reconcile_get_name ( gsb_data_transaction_get_reconcile_number ( pBreakdownTransaction ) ) );
		    }

		    /* met le ch�que si c'est un type � num�rotation automatique */
		    payment_method = gsb_data_transaction_get_method_of_payment_number ( pBreakdownTransaction );
		    if (gsb_data_payment_get_automatic_numbering (payment_method))
		    {
			CSV_CLEAR_FIELD (csv_field_cheque);
			csv_field_cheque = my_strdup ( gsb_data_transaction_get_method_of_payment_content ( pBreakdownTransaction ) );
		    }

		    /* Budgetary lines */
		    if ( gsb_data_transaction_get_budgetary_number ( pBreakdownTransaction ) != -1 )
		    {
			CSV_CLEAR_FIELD (csv_field_imput);
			csv_field_imput = my_strdup ( gsb_data_budget_get_name ( gsb_data_transaction_get_budgetary_number ( pBreakdownTransaction ), 0, "" ) );

			if ( gsb_data_transaction_get_sub_budgetary_number ( pBreakdownTransaction ) != -1 )
			{
			    CSV_CLEAR_FIELD (csv_field_sous_imput);
			    csv_field_sous_imput = my_strdup ( gsb_data_budget_get_sub_budget_name ( gsb_data_transaction_get_budgetary_number ( pBreakdownTransaction ),
												     gsb_data_transaction_get_sub_budgetary_number ( pBreakdownTransaction ),
												     NULL ) );
			}
		    }

		    /* Piece comptable */
		    CSV_CLEAR_FIELD (csv_field_piece);
		    csv_field_piece = my_strdup ( gsb_data_transaction_get_voucher ( pBreakdownTransaction ) );

		    /* Financial Year */
		    if ( gsb_data_transaction_get_financial_year_number ( pBreakdownTransaction ) != -1 )
		    {
			CSV_CLEAR_FIELD (csv_field_exercice );
			csv_field_exercice  = my_strdup (gsb_data_fyear_get_name(gsb_data_transaction_get_financial_year_number ( pBreakdownTransaction )));
		    }

		    csv_add_record(csv_file,FALSE, print_balance);
		}

		pBreakdownTransactionList = pBreakdownTransactionList -> next;
	    }
	    csv_clear_fields(TRUE);
	}
	else
	{
	    /* Si c'est un virement ... */
	    if ( gsb_data_transaction_get_contra_transaction_number ( transaction_number ))
	    {
		CSV_CLEAR_FIELD (csv_field_categ);
		csv_field_categ = my_strdup (_("Transfer"));

		/* ... vers un compte existant */
		if ( gsb_data_transaction_get_contra_transaction_account (  transaction_number ) >= 0 )
		{
		    gchar* tmpstr = g_strconcat ( "[", gsb_data_account_get_name ( gsb_data_transaction_get_contra_transaction_account ( transaction_number ) ), "]", NULL );
		    /* TODO dOm : is it necessary to duplicate memory with my_strdup since it was already newly allocated memory ? */
		    CSV_CLEAR_FIELD (csv_field_sous_categ);
		    csv_field_sous_categ = my_strdup (tmpstr);
		    g_free ( tmpstr );
		}
		/* ... vers un compte supprimé */
		else
		{
		    gchar* tmpstr = g_strconcat ( "[", _("Deleted account"), "]", NULL );
		    /* TODO dOm : is it necessary to duplicate memory with my_strdup since it was already newly allocated memory ? */
		    CSV_CLEAR_FIELD (csv_field_sous_categ);
		    csv_field_sous_categ = my_strdup (tmpstr);
		    g_free ( tmpstr );
		}
	    }
	    else
	    {
		/* c'est du type categ : sous-categ */
		if ( gsb_data_transaction_get_category_number ( transaction_number ) != -1 )
		{
		    CSV_CLEAR_FIELD (csv_field_categ);
		    csv_field_categ = my_strdup ( gsb_data_budget_get_name ( gsb_data_transaction_get_category_number ( transaction_number ), 0, "" ) );

		    if ( gsb_data_transaction_get_sub_category_number ( transaction_number ) != -1 )
		    {
			CSV_CLEAR_FIELD (csv_field_sous_categ);
			csv_field_sous_categ = my_strdup ( gsb_data_budget_get_sub_budget_name ( gsb_data_transaction_get_category_number ( transaction_number ),
												 gsb_data_transaction_get_sub_category_number ( transaction_number ),
												 NULL ) );
		    }
		}
	    }

	    csv_add_record(csv_file,TRUE, print_balance);
	}
    }
    return TRUE;
}


/**
 * add the title line to the csv file
 *
 * \param csv_file a FILE struct
 * \param print_balance if set the balance or not in the export file
 *
 * \return TRUE ok, FALSE problem
 * */
static gboolean gsb_csv_export_title_line ( FILE *csv_file,
					    gboolean print_balance )
{
    CSV_CLEAR_FIELD (csv_field_operation );
    csv_field_operation  = my_strdup (_("Transactions"));
    
    CSV_CLEAR_FIELD (csv_field_account   );
    csv_field_account    = my_strdup (_("Account name"));
   
    CSV_CLEAR_FIELD (csv_field_ventil    );
    csv_field_ventil     = my_strdup (_("Breakdown"));
  
    CSV_CLEAR_FIELD (csv_field_date      );
    csv_field_date       = my_strdup (_("Date"));
 
    CSV_CLEAR_FIELD (csv_field_date_val  );
    csv_field_date_val   = my_strdup (_("Value date"));

    CSV_CLEAR_FIELD (csv_field_cheque    );
    csv_field_cheque     = my_strdup (_("Cheques"));
    
    CSV_CLEAR_FIELD (csv_field_exercice  );
    csv_field_exercice   = my_strdup (_("Financial year"));
    
    CSV_CLEAR_FIELD (csv_field_pointage  );
    csv_field_pointage   = my_strdup (_("C/R"));
    
    CSV_CLEAR_FIELD (csv_field_tiers     );
    csv_field_tiers      = my_strdup (_("Payee"));
    
    CSV_CLEAR_FIELD (csv_field_credit    );
    csv_field_credit     = my_strdup (_("Credit"));
    
    CSV_CLEAR_FIELD (csv_field_debit     );
    csv_field_debit      = my_strdup (_("Debit"));
    
    CSV_CLEAR_FIELD (csv_field_montant   );
    csv_field_montant    = my_strdup (_("Amount"));
    
    CSV_CLEAR_FIELD (csv_field_solde     );
    csv_field_solde      = my_strdup (_("Balance"));
    
    CSV_CLEAR_FIELD (csv_field_categ     );
    csv_field_categ      = my_strdup (_("Category"));
    
    CSV_CLEAR_FIELD (csv_field_sous_categ);
    csv_field_sous_categ = my_strdup (_("Sub-categories"));
    
    CSV_CLEAR_FIELD (csv_field_notes     );
    csv_field_notes      = my_strdup (_("Notes"));
    
    CSV_CLEAR_FIELD (csv_field_imput     );
    csv_field_imput      = my_strdup (_("Budgetary lines"));
    
    CSV_CLEAR_FIELD (csv_field_sous_imput);
    csv_field_sous_imput = my_strdup (_("Sub-budgetary lines"));
    
    CSV_CLEAR_FIELD (csv_field_piece     );
    csv_field_piece      = my_strdup (_("Voucher"));
    
    CSV_CLEAR_FIELD (csv_field_rappro    );
    csv_field_rappro     = my_strdup (_("Reconciliation number"));
    
    CSV_CLEAR_FIELD (csv_field_info_bank );
    csv_field_info_bank  = my_strdup (_("Bank references"));

    csv_add_record(csv_file,TRUE, print_balance);
    return TRUE;
}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
