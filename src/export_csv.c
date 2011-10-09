/* ************************************************************************** */
/* This file manage CSV export format                                         */
/*                                                                            */
/*     Copyright (C)    2004 François Terrot (francois.terrot at grisbi.org)  */
/*          2005 Alain Portal (aportal@univ-montp2.fr)                        */
/*          2009 Benjamin Drieu (bdrieu@april.org)                            */
/*          2010 Pierre Biava (grisbi@pierre.biava.name)                      */
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
 * \todo make the CSV parameter configurable 
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "export_csv.h"
#include "dialog.h"
#include "gsb_data_account.h"
#include "gsb_data_archive_store.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_currency.h"
#include "gsb_data_fyear.h"
#include "gsb_data_payee.h"
#include "gsb_data_payment.h"
#include "gsb_data_reconcile.h"
#include "gsb_data_transaction.h"
#include "gsb_file_util.h"
#include "gsb_real.h"
#include "main.h"
#include "utils_str.h"
#include "utils_files.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void csv_add_record(FILE* file,
			   gboolean clear_all,
			   gboolean print_balance );
static void csv_clear_fields(gboolean clear_all);
static FILE *gsb_csv_export_open_file ( const gchar *filename );
gboolean gsb_csv_export_sort_by_value_date_or_date ( gpointer transaction_pointer_1, 
                        gpointer transaction_pointer_2 );
static gboolean gsb_csv_export_title_line ( FILE *csv_file,
					    gboolean print_balance );
static gboolean gsb_csv_export_transaction ( gint transaction_number,
					     FILE *csv_file,
					     gboolean print_balance );
static gboolean gsb_csv_export_tree_view_list_foreach_callback ( GtkTreeModel *model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        FILE *csv_file );
static void gsb_csv_export_tree_view_list_export_rows ( FILE *csv_file, GtkTreeView *tree_view );
static void gsb_csv_export_tree_view_list_export_title_line ( FILE *csv_file, GtkTreeView *tree_view );

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
gchar*  csv_field_ventil     = NULL; /*!< is operation a split (string) */
gchar*  csv_field_date       = NULL; /*!< date of operation (of main operation for split) (string) */
gchar*  csv_field_date_val   = NULL; /*!< value date of operation (of main operation for split) (string) */
gchar*  csv_field_pointage   = NULL; /*!< pointed/reconcialiation status (string) */
gchar*  csv_field_tiers      = NULL; /*!< Payee (string) */
gchar*  csv_field_credit     = NULL; /*!< credit (numerical) */
gchar*  csv_field_debit      = NULL; /*!< debit (numerical) */
gchar*  csv_field_montant    = NULL; /*!< amount (numerical) only used for split */
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
 * split operation items.
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
gboolean gsb_csv_export_account ( const gchar *filename, gint account_number )
{
    FILE *csv_file;
    GSList *pTransactionList;
    GSList *tmp_list;

    csv_file = gsb_csv_export_open_file ( filename );

    if ( !csv_file )
        return FALSE;

    if ( g_csv_with_title_line )
        gsb_csv_export_title_line ( csv_file, TRUE );

    /* set the initial balance */
    if ( csv_field_tiers )
        g_free ( csv_field_tiers );

    csv_field_tiers = g_strconcat (_("Initial balance") , " [",
                        gsb_data_account_get_name ( account_number ),
                        "]", NULL );

    /* set the initial current_balance,
     * as we will write all the non archived transactions,
     * we need to get the initial balance of the account, without the archived transactions */
    current_balance = gsb_data_account_get_init_balance ( account_number, -1);

    tmp_list = gsb_data_archive_store_get_archives_list ( );
    while ( tmp_list )
    {
        gint archive_store_number;

        archive_store_number = gsb_data_archive_store_get_number ( tmp_list -> data );

        if ( gsb_data_archive_store_get_account_number ( archive_store_number ) == account_number )
            current_balance = gsb_real_add ( current_balance,
                                gsb_data_archive_store_get_balance ( archive_store_number ) );

        tmp_list = tmp_list -> next;
    }

    /* ok the balance is now good, can write it */
    CSV_CLEAR_FIELD ( csv_field_solde );
    csv_field_solde = gsb_real_get_string ( current_balance );
    if ( current_balance.mantissa >= 0 )
    {
        CSV_CLEAR_FIELD ( csv_field_credit );
        csv_field_credit = gsb_real_get_string ( current_balance );
    }
    else
    {
        CSV_CLEAR_FIELD ( csv_field_debit );
        csv_field_debit = gsb_real_get_string ( gsb_real_abs ( current_balance ) );
    }

    csv_add_record ( csv_file, TRUE, TRUE );

    /* export the transactions */
    pTransactionList = gsb_data_transaction_get_transactions_list ( );
    tmp_list = g_slist_sort ( g_slist_copy ( pTransactionList ),
                        (GCompareFunc) gsb_csv_export_sort_by_value_date_or_date );

    while ( tmp_list )
    {
        gint pTransaction;

        pTransaction = gsb_data_transaction_get_transaction_number ( tmp_list -> data );

        if ( gsb_data_transaction_get_account_number ( pTransaction ) == account_number )
        {
            /* export the transaction */
            /* for now, print the balance. is this usefull ? */
            gsb_csv_export_transaction ( pTransaction, csv_file, TRUE);
        }

        tmp_list = tmp_list -> next;
    }

    fclose ( csv_file );
    g_slist_free ( tmp_list );

    /* return */
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
gboolean gsb_csv_export_archive ( const gchar *filename, gint archive_number )
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

    /* Création du fichier, si pb, on marque l'erreur et passe au fichier suivant */
    csv_file = utf8_fopen ( filename, "w" );
    if ( ! csv_file )
    {
        gchar *sMessage = NULL;

        sMessage = g_strdup_printf ( _("Unable to create file \"%s\" :\n%s"),
                         filename, g_strerror ( errno ) );
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
	gchar* tmpstr;
	const GDate *value_date, *date;
	gint payment_method;

    account_number = gsb_data_transaction_get_account_number (transaction_number);

    /* Si c'est une ventilation d'opération (càd une opération fille),
    elle n'est pas traitée à la base du "if" mais plus loin, quand
    son opé ventilée sera exportée */
    if ( gsb_data_transaction_get_mother_transaction_number (transaction_number) )
        return TRUE;

    return_exponent = gsb_data_currency_get_floating_point (
                        gsb_data_account_get_currency (account_number));

	/* met la date */
	date = gsb_data_transaction_get_date ( transaction_number );
	if ( date )
	{
	    CSV_CLEAR_FIELD (csv_field_date);
	    csv_field_date = g_strdup_printf ("%d/%d/%d", 
					      g_date_get_day ( date ), 
					      g_date_get_month ( date ),
					      g_date_get_year ( date ) );
	}

	value_date = gsb_data_transaction_get_value_date ( transaction_number );
	if ( value_date )
	{
	    CSV_CLEAR_FIELD (csv_field_date_val);
	    csv_field_date_val = g_strdup_printf ("%d/%d/%d", 
						  g_date_get_day ( value_date ), 
						  g_date_get_month ( value_date ),
						  g_date_get_year ( value_date ) );
	}

	/* met le pointage */
        CSV_CLEAR_FIELD (csv_field_pointage);
	switch ( gsb_data_transaction_get_marked_transaction ( transaction_number ) )
	{
	    case OPERATION_NORMALE:
            csv_field_pointage = my_strdup ("");
		break;
	    case OPERATION_POINTEE:
            csv_field_pointage = my_strdup ("P");
		break;
	    case OPERATION_TELERAPPROCHEE:
            csv_field_pointage = my_strdup ("T");
		break;
	    case OPERATION_RAPPROCHEE:
            csv_field_pointage = my_strdup ("R");
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

	/* met le cheque si c'est un type à numerotation automatique */
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
	if ( gsb_data_transaction_get_split_of_transaction ( transaction_number ) )
	{
	    GSList *pSplitTransactionList;

	    CSV_CLEAR_FIELD (csv_field_categ);
	    csv_field_categ = my_strdup (_("Split of transaction"));
        
	    csv_add_record(csv_file,FALSE, print_balance);

	    pSplitTransactionList = gsb_data_transaction_get_transactions_list ();

	    while ( pSplitTransactionList )
	    {
		gint pSplitTransaction;

		pSplitTransaction = gsb_data_transaction_get_transaction_number (
                        pSplitTransactionList -> data );

		if ( gsb_data_transaction_get_account_number (
         pSplitTransaction ) == gsb_data_transaction_get_account_number (transaction_number)
         &&
         gsb_data_transaction_get_mother_transaction_number (
         pSplitTransaction ) == transaction_number )
		{
		    /* on commence par mettre la catég et sous categ de l'opé et de l'opé de ventilation */
		    CSV_CLEAR_FIELD (csv_field_ventil);
		    csv_field_ventil = my_strdup (_("B")); /* -> mark */

		    CSV_CLEAR_FIELD (csv_field_operation);
		    csv_field_operation = g_strdup_printf("%d", pSplitTransaction );

		    if ( gsb_data_transaction_get_contra_transaction_number ( pSplitTransaction )  > 0)
		    {
			/* c'est un virement */
			CSV_CLEAR_FIELD (csv_field_categ);
			csv_field_categ = my_strdup (_("Transfer"));

			tmpstr = g_strconcat ( "[", gsb_data_account_get_name ( gsb_data_transaction_get_contra_transaction_account ( pSplitTransaction ) ), "]", NULL );
			/* TODO dOm : is it necessary to duplicate memory with my_strdup since it was already newly allocated memory ? */
			CSV_CLEAR_FIELD (csv_field_sous_categ);
			csv_field_sous_categ = my_strdup (tmpstr);
			g_free ( tmpstr );
		    }
		    else
		    {
			if ( gsb_data_transaction_get_category_number ( pSplitTransaction ) != -1 )
			{
			    CSV_CLEAR_FIELD (csv_field_categ);
			    csv_field_categ = my_strdup ( gsb_data_category_get_name ( gsb_data_transaction_get_category_number ( pSplitTransaction ), 0, "" ) );

			    if ( gsb_data_transaction_get_sub_category_number ( pSplitTransaction ) != -1 )
			    {
				CSV_CLEAR_FIELD (csv_field_sous_categ);
				csv_field_sous_categ = my_strdup ( gsb_data_category_get_sub_category_name ( gsb_data_transaction_get_category_number ( pSplitTransaction ),
													 gsb_data_transaction_get_sub_category_number ( pSplitTransaction ),
													 NULL ) );
			    }
			}

		    }

		    /* met les notes de la ventilation */
		    if ( gsb_data_transaction_get_notes ( pSplitTransaction ) )
		    {
			CSV_CLEAR_FIELD (csv_field_notes);
			csv_field_notes = my_strdup (gsb_data_transaction_get_notes ( pSplitTransaction ));
		    }

		    /* met le montant de la ventilation */
		    amount = gsb_data_transaction_get_adjusted_amount ( pSplitTransaction, return_exponent );
		    CSV_CLEAR_FIELD (csv_field_montant);
		    csv_field_montant = gsb_real_get_string (amount);

		    /* met le rapprochement */
		    if ( gsb_data_transaction_get_reconcile_number ( pSplitTransaction ) )
		    {
			CSV_CLEAR_FIELD (csv_field_rappro);
			csv_field_rappro = my_strdup ( gsb_data_reconcile_get_name ( gsb_data_transaction_get_reconcile_number ( pSplitTransaction ) ) );
		    }

		    /* met le chèque si c'est un type à numéotation automatique */
		    payment_method = gsb_data_transaction_get_method_of_payment_number ( pSplitTransaction );
		    if (gsb_data_payment_get_automatic_numbering (payment_method))
		    {
			CSV_CLEAR_FIELD (csv_field_cheque);
			csv_field_cheque = my_strdup ( gsb_data_transaction_get_method_of_payment_content ( pSplitTransaction ) );
		    }

		    /* Budgetary lines */
		    if ( gsb_data_transaction_get_budgetary_number ( pSplitTransaction ) != -1 )
		    {
			CSV_CLEAR_FIELD (csv_field_imput);
			csv_field_imput = my_strdup ( gsb_data_budget_get_name ( gsb_data_transaction_get_budgetary_number ( pSplitTransaction ), 0, "" ) );

			if ( gsb_data_transaction_get_sub_budgetary_number ( pSplitTransaction ) != -1 )
			{
			    CSV_CLEAR_FIELD (csv_field_sous_imput);
			    csv_field_sous_imput = my_strdup ( gsb_data_budget_get_sub_budget_name ( gsb_data_transaction_get_budgetary_number ( pSplitTransaction ),
												     gsb_data_transaction_get_sub_budgetary_number ( pSplitTransaction ),
												     NULL ) );
			}
		    }

		    /* Piece comptable */
		    CSV_CLEAR_FIELD (csv_field_piece);
		    csv_field_piece = my_strdup ( gsb_data_transaction_get_voucher ( pSplitTransaction ) );

		    /* Financial Year */
		    if ( gsb_data_transaction_get_financial_year_number ( pSplitTransaction ) != -1 )
		    {
			CSV_CLEAR_FIELD (csv_field_exercice );
			csv_field_exercice  = my_strdup (gsb_data_fyear_get_name(gsb_data_transaction_get_financial_year_number ( pSplitTransaction )));
		    }

		    csv_add_record(csv_file,FALSE, print_balance);
		}

		pSplitTransactionList = pSplitTransactionList -> next;
	    }
	    csv_clear_fields(TRUE);
	}
	else
	{
	    gchar *tmpstr;
	    gint contra_transaction_number = gsb_data_transaction_get_contra_transaction_number ( transaction_number );

	    switch (contra_transaction_number)
	    {
		case 0:
		    /* normal category */
		    if ( gsb_data_transaction_get_category_number ( transaction_number ) != -1 )
		    {
			CSV_CLEAR_FIELD (csv_field_categ);
			csv_field_categ = my_strdup ( gsb_data_category_get_name ( gsb_data_transaction_get_category_number ( transaction_number ), 0, "" ) );

			if ( gsb_data_transaction_get_sub_category_number ( transaction_number ) != -1 )
			{
			    CSV_CLEAR_FIELD (csv_field_sous_categ);
			    csv_field_sous_categ = my_strdup ( gsb_data_category_get_sub_category_name ( gsb_data_transaction_get_category_number ( transaction_number ),
												     gsb_data_transaction_get_sub_category_number ( transaction_number ),
												     NULL ) );
			}
		    }
		    break;
		case -1:
		    /* transfer to deleted account */
		    CSV_CLEAR_FIELD (csv_field_categ);
		    csv_field_categ = my_strdup (_("Transfer"));

		    tmpstr = g_strconcat ( "[", _("Deleted account"), "]", NULL );
		    /* TODO dOm : is it necessary to duplicate memory with my_strdup since it was already newly allocated memory ? */
		    CSV_CLEAR_FIELD (csv_field_sous_categ);
		    csv_field_sous_categ = my_strdup (tmpstr);
		    g_free ( tmpstr );

		    break;
		default:
		    /* transfer */
		    CSV_CLEAR_FIELD (csv_field_categ);
		    csv_field_categ = my_strdup (_("Transfer"));

		    tmpstr = g_strconcat ( "[", gsb_data_account_get_name ( gsb_data_transaction_get_contra_transaction_account ( transaction_number ) ), "]", NULL );
		    /* TODO dOm : is it necessary to duplicate memory with my_strdup since it was already newly allocated memory ? */
		    CSV_CLEAR_FIELD (csv_field_sous_categ);
		    csv_field_sous_categ = my_strdup (tmpstr);
		    g_free ( tmpstr );
	    }
	    csv_add_record(csv_file,TRUE, print_balance);
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
    csv_field_ventil     = my_strdup (_("Split"));
  
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



/**
 *
 *
 *
 *
 * */
gboolean gsb_csv_export_tree_view_list ( const gchar *filename, GtkTreeView *tree_view )
{
    FILE *csv_file;

    csv_file = gsb_csv_export_open_file ( filename );
    if ( !csv_file )
        return FALSE;

    gsb_csv_export_tree_view_list_export_title_line ( csv_file, tree_view );
    gsb_csv_export_tree_view_list_export_rows ( csv_file, tree_view );

    fclose ( csv_file );

    return TRUE;
}


/**
 *
 *
 *
 *
 * */
static void gsb_csv_export_tree_view_list_export_title_line ( FILE *csv_file, GtkTreeView *tree_view )
{
    GList *list;
    GList *list_tmp;

    list = gtk_tree_view_get_columns ( tree_view );
    list_tmp = list;

    while ( list_tmp )
    {
        GtkTreeViewColumn *col;
        const gchar *text;

        col = ( GtkTreeViewColumn * ) list_tmp -> data;

        /* get the text */
        text = gtk_tree_view_column_get_title ( col );

        CSV_STR_FIELD( csv_file, text );

        list_tmp  = list_tmp -> next;
    }
    CSV_END_RECORD( csv_file );
}


/**
 *
 *
 *
 *
 * */
static void gsb_csv_export_tree_view_list_export_rows ( FILE *csv_file, GtkTreeView *tree_view )
{
    GtkTreeModel *model;

    model = gtk_tree_view_get_model ( tree_view );
    g_object_set_data ( G_OBJECT ( model ), "tree_view", tree_view );

    gtk_tree_model_foreach ( model,
                        ( GtkTreeModelForeachFunc ) gsb_csv_export_tree_view_list_foreach_callback,
                        csv_file );

    g_object_steal_data ( G_OBJECT ( model ), "tree_view" );
}



static gboolean gsb_csv_export_tree_view_list_foreach_callback ( GtkTreeModel *model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        FILE *csv_file )
{
    GtkTreeView *tree_view;
    GList *list;
    GList *list_tmp;

    tree_view = g_object_get_data ( G_OBJECT ( model ), "tree_view" );
    list = gtk_tree_view_get_columns ( tree_view );
    list_tmp = list;

    while ( list_tmp )
    {
        GtkTreeViewColumn *col;
        gchar *text;
        gint col_num_model;
        GType col_type_model;

        col = list_tmp -> data;

        col_num_model = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( col ), "num_col_model" ) );
        col_type_model = gtk_tree_model_get_column_type ( model, col_num_model );

        /* get the text */
        if ( col_type_model == G_TYPE_STRING )
            gtk_tree_model_get ( model, iter, col_num_model, &text, -1 );
        else if ( col_type_model == G_TYPE_INT )
        {
            gint number;

            gtk_tree_model_get ( model, iter, col_num_model, &number, -1 );
            text = utils_str_itoa ( number );
        }
        else
            text = NULL;

        CSV_STR_FIELD ( csv_file, text );
        CSV_CLEAR_FIELD ( text );

        list_tmp  = list_tmp -> next;
    }

    CSV_END_RECORD( csv_file );

    return FALSE;
}


/**
 * used to compare 2 iters and sort the by value date or date if not exist
 * always put the white line below
 * 
 * \param iter_1
 * \param iter_2
 * 
 * \return -1 if iter_1 is before iter_2
 * */
gint gsb_csv_export_sort_by_value_date_or_date ( gpointer transaction_pointer_1, 
                        gpointer transaction_pointer_2 )
{
    gint transaction_number_1;
    gint transaction_number_2;
    const GDate *value_date_1;
    const GDate *value_date_2;

    transaction_number_1 = gsb_data_transaction_get_transaction_number (
                        transaction_pointer_1 );
    transaction_number_2 = gsb_data_transaction_get_transaction_number (
                        transaction_pointer_2 );

    value_date_1 = gsb_data_transaction_get_value_date ( transaction_number_1 );
    if ( ! value_date_1 )
        value_date_1 = gsb_data_transaction_get_date ( transaction_number_1 );

    value_date_2 = gsb_data_transaction_get_value_date ( transaction_number_2 );
    if ( ! value_date_2 )
        value_date_2 = gsb_data_transaction_get_date ( transaction_number_2 );

    if ( value_date_1 )
        return ( g_date_compare ( value_date_1, value_date_2 ) );
    else
        return -1;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
