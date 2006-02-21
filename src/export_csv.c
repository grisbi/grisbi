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
#include "dialog.h"
#include "utils_exercices.h"
#include "gsb_data_account.h"
#include "gsb_data_budget.h"
#include "gsb_data_payee.h"
#include "gsb_data_transaction.h"
#include "gsb_real.h"
#include "main.h"
#include "utils_rapprochements.h"
#include "search_glist.h"
#include "utils_files.h"
#include "include.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static  void csv_add_record(FILE* file,gboolean clear_all);
static  void csv_clear_fields(gboolean clear_all);
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

gchar*  csv_field_operation  = NULL; /*!< operation number (numerical) */
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
 * 
 */ 
static void csv_add_record(FILE* file,gboolean clear_all)
{ /* {{{ */
  CSV_NUM_FIELD(file,csv_field_operation);
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
  CSV_NUM_FIELD(file,csv_field_solde);
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
void csv_export ( gchar * filename, gint account_nb )
{
  gchar *sMessage = NULL;
  FILE *csv_file;
  gsb_real balance;
  struct stat test_file;

  if (utf8_stat ( filename, &test_file ) != -1)
  {
      if ( ! question_yes_no_hint (_("File already exists"),
				   g_strdup_printf (_("Do you want to overwrite file \"%s\"?"), filename) ) )
      {
	  return;
      }
  }

  /* Ouverture du fichier, si pb, on marque l'erreur et passe au fichier suivant */
  if ( !( csv_file = utf8_fopen ( filename, "w" ) ))
  {
      sMessage = g_strdup_printf ( _("Error opening file \"%s\" :\n%s"),
				   filename, strerror ( errno ) );
      dialogue ( sMessage );
      g_free ( sMessage );
      sMessage = NULL;
  }
  else
  {
      GSList *pTransactionList;

      if (g_csv_with_title_line)
      {
	  csv_field_operation  = g_strdup(_("Transactions"));
	  csv_field_ventil     = g_strdup(_("Breakdown"));
	  csv_field_date       = g_strdup(_("Date"));
	  csv_field_date_val   = g_strdup(_("Value date"));
	  csv_field_cheque     = g_strdup(_("Cheques"));
	  csv_field_exercice   = g_strdup(_("Financial year"));
	  csv_field_pointage   = g_strdup(_("C/R"));
	  csv_field_tiers      = g_strdup(_("Payee"));
	  csv_field_credit     = g_strdup(_("Credit"));
	  csv_field_debit      = g_strdup(_("Debit"));
	  csv_field_montant    = g_strdup(_("Amount"));
	  csv_field_solde      = g_strdup(_("Balance"));
	  csv_field_categ      = g_strdup(_("Category"));
	  csv_field_sous_categ = g_strdup(_("Sub-categories"));
	  csv_field_notes      = g_strdup(_("Notes"));
	  csv_field_imput      = g_strdup(_("Budgetary lines"));
	  csv_field_sous_imput = g_strdup(_("Sub-budgetary lines"));
	  csv_field_piece      = g_strdup(_("Voucher"));
	  csv_field_rappro     = g_strdup(_("Reconciliation number"));
	  csv_field_info_bank  = g_strdup(_("Bank references"));
      }

      csv_add_record(csv_file,TRUE);

      /* tiers */
      csv_field_tiers = g_strdup_printf ( g_strconcat (_("Initial balance") , " [", 
						       gsb_data_account_get_name(account_nb),
						       "]", NULL ) );

      /* met le solde initial */
      balance = gsb_data_account_get_init_balance ( account_nb, -1);
      csv_field_solde = gsb_real_get_string (balance);
      if ( balance.mantissa >= 0 )
      {
	  csv_field_credit = gsb_real_get_string (balance);
      }
      else
      {
	  csv_field_debit = gsb_real_get_string (gsb_real_abs (balance));
      }

      csv_add_record(csv_file,TRUE);

      /* on met toutes les opérations */
      pTransactionList = gsb_data_transaction_get_transactions_list ();
      while ( pTransactionList )
      {
	  gsb_real amount;
	  gint pTransaction = gsb_data_transaction_get_transaction_number (pTransactionList -> data);

	  if ( gsb_data_transaction_get_account_number ( pTransaction ) != account_nb )
	  {
	      pTransactionList = pTransactionList -> next;
	      continue;
	  }

	  /* Si c'est une ventilation d'opération (càd une opération fille),
	     elle n'est pas traitée à la base du "if" mais plus loin, quand
	     son opé ventilée sera exportée */
	  if ( ! gsb_data_transaction_get_mother_transaction_number (pTransaction) )
	  {
	      GSList* pMiscList = NULL; /* Miscellaneous list */
	      GDate * value_date, * date;
	      gint payment_method;
	      
	      /* met la date */
	      date = gsb_data_transaction_get_date ( pTransaction );
	      if ( date )
	      {
		  csv_field_date = g_strdup_printf ("%d/%d/%d", 
						    g_date_day ( date ), 
						    g_date_month ( date ),
						    g_date_year ( date ) );
	      }
                        
	      value_date = gsb_data_transaction_get_value_date ( pTransaction );
	      if ( value_date )
	      {
		  csv_field_date_val = g_strdup_printf ("%d/%d/%d", 
							g_date_day ( value_date ), 
							g_date_month ( value_date ),
							g_date_year ( value_date ) );
	      }

	      /* met le pointage */
	      switch ( gsb_data_transaction_get_marked_transaction ( pTransaction ) )
	      {
		  case 0: /* CHECKED_TRANSACTION */
		      csv_field_pointage = g_strdup(_("C"));
		      break;
		  case 1: /* RECONCILED_TRANSACTION */
		      csv_field_pointage = g_strdup(_("R"));
		      break;
		  case 2: /* TELECHECKED_TRANSACTION */
		      csv_field_pointage = g_strdup(_("T"));
		      break;
	      }

	      /* met les notes */
	      CSV_CLEAR_FIELD(csv_field_notes);
	      if ( gsb_data_transaction_get_notes ( pTransaction ) )
		  csv_field_notes = g_strdup(gsb_data_transaction_get_notes ( pTransaction ));

	      /* met le tiers */
	      CSV_CLEAR_FIELD(csv_field_tiers);
	      csv_field_tiers = g_strdup ( gsb_data_payee_get_name ( gsb_data_transaction_get_party_number ( pTransaction ), FALSE ) );

	      /* met le numéro du rapprochement */
	      if ( gsb_data_transaction_get_reconcile_number ( pTransaction ) )
	      {
		  csv_field_rappro = g_strdup ( rapprochement_name_by_no ( gsb_data_transaction_get_reconcile_number ( pTransaction ) ) );
	      }
 
	      /* Met les informations bancaires de l'opération. Elles n'existent
		 qu'au niveau de l'opération mère */
	      CSV_CLEAR_FIELD(csv_field_info_bank);
	      if ( gsb_data_transaction_get_bank_references ( pTransaction ) )
		  csv_field_info_bank = g_strdup ( gsb_data_transaction_get_bank_references ( pTransaction ) );

	      /* met le montant, transforme la devise si necessaire */
	      amount = gsb_data_transaction_get_adjusted_amount ( pTransaction, -1 );
	      if (amount.mantissa >= 0 )
	      {
		  csv_field_credit = gsb_real_get_string (amount);
	      }
	      else
	      {
		  csv_field_debit  = gsb_real_get_string (gsb_real_abs (amount));
	      }

	      /* met le chèque si c'est un type à numérotation automatique */
	      payment_method = gsb_data_transaction_get_method_of_payment_number ( pTransaction );
	      pMiscList = g_slist_find_custom ( gsb_data_account_get_method_payment_list ( account_nb ),
						GINT_TO_POINTER ( payment_method ),
						(GCompareFunc) recherche_type_ope_par_no );

	      if ( pMiscList )
	      {
		  struct struct_type_ope * type = pMiscList -> data;

		  if ( type -> numerotation_auto )
		      csv_field_cheque = g_strdup ( gsb_data_transaction_get_method_of_payment_content ( pTransaction ) );
	      }

	      if ( gsb_data_transaction_get_budgetary_number ( pTransaction ) != -1 )
	      {
		  csv_field_imput = g_strdup ( gsb_data_budget_get_name ( gsb_data_transaction_get_budgetary_number ( pTransaction ), 0, "" ) );

		  if ( gsb_data_transaction_get_sub_budgetary_number ( pTransaction ) != -1 )
		  {
		      csv_field_sous_imput = g_strdup ( gsb_data_budget_get_sub_budget_name ( gsb_data_transaction_get_budgetary_number ( pTransaction ),
											      gsb_data_transaction_get_sub_budgetary_number ( pTransaction ),
											      NULL ) );
		  }
	      }

	      /* Piece comptable */
	      csv_field_piece = g_strdup( gsb_data_transaction_get_voucher ( pTransaction ) );

	      /* Balance */
	      balance = gsb_real_add ( balance,
				       amount );
	      csv_field_solde = gsb_real_get_string (balance);

	      /* Number */
	      csv_field_operation = g_strdup_printf("%d", pTransaction );

	      /* Financial Year */
	      if ( gsb_data_transaction_get_financial_year_number ( pTransaction ) != -1 )
		  csv_field_exercice  = g_strdup(exercice_name_by_no(gsb_data_transaction_get_financial_year_number ( pTransaction )));

	      /*  on met soit un virement, soit une ventilation, soit les catégories */

	      /* Si c'est une opération ventilée, on recherche toutes les ventilations
		 de cette opération et on les traite immédiatement. */
	      /* et les met à la suite */
	      /* la catégorie de l'opé sera celle de la première opé de ventilation */
	      if ( gsb_data_transaction_get_breakdown_of_transaction ( pTransaction ) )
	      {
		  GSList *pBreakdownTransactionList;

		  csv_field_categ = g_strdup(_("Breakdown of transaction"));
		  csv_add_record(csv_file,FALSE);

		  pBreakdownTransactionList = gsb_data_transaction_get_transactions_list ();

		  while ( pBreakdownTransactionList )
		  {
		      gint pBreakdownTransaction;

		      pBreakdownTransaction = (gint) pBreakdownTransactionList -> data;

		      if ( gsb_data_transaction_get_account_number ( pBreakdownTransaction ) == account_nb &&
			   gsb_data_transaction_get_mother_transaction_number ( pBreakdownTransaction ) == pTransaction )
		      {
			  /* on commence par mettre la catég et sous categ de l'opé et de l'opé de ventilation */
			  csv_field_ventil = g_strdup(_("B")); // -> mark 

			  csv_field_operation = g_strdup_printf("%d", pBreakdownTransaction );

			  if ( gsb_data_transaction_get_transaction_number_transfer ( pBreakdownTransaction ) )
			  {
			      /* c'est un virement */
			      csv_field_categ = g_strdup(_("Transfer"));

			      csv_field_sous_categ = g_strdup(g_strconcat ( "[", 
									    gsb_data_account_get_name ( gsb_data_transaction_get_account_number_transfer ( pBreakdownTransaction ) ),
									    "]", NULL ));
			  }
			  else
			  {
			      if ( gsb_data_transaction_get_category_number ( pBreakdownTransaction ) != -1 )
			      {
				  csv_field_categ = g_strdup ( gsb_data_budget_get_name ( gsb_data_transaction_get_category_number ( pBreakdownTransaction ), 0, "" ) );

				  if ( gsb_data_transaction_get_sub_category_number ( pBreakdownTransaction ) != -1 )
				  {
				      csv_field_sous_categ = g_strdup ( gsb_data_budget_get_sub_budget_name ( gsb_data_transaction_get_category_number ( pBreakdownTransaction ),
													      gsb_data_transaction_get_sub_category_number ( pBreakdownTransaction ),
													      NULL ) );
				  }
			      }

			  }

			  /* met les notes de la ventilation */
			  if ( gsb_data_transaction_get_notes ( pBreakdownTransaction ) )
			      csv_field_notes = g_strdup(gsb_data_transaction_get_notes ( pBreakdownTransaction ));

			  /* met le montant de la ventilation */
			  amount = gsb_data_transaction_get_adjusted_amount ( pBreakdownTransaction, -1 );
			  csv_field_montant = gsb_real_get_string (amount);

			  /* met le rapprochement */
			  if ( gsb_data_transaction_get_reconcile_number ( pBreakdownTransaction ) )
			  {
			      csv_field_rappro = g_strdup ( rapprochement_name_by_no ( gsb_data_transaction_get_reconcile_number ( pBreakdownTransaction ) ) );
			  }

			  /* met le ch�que si c'est un type � num�rotation automatique */
			  payment_method = gsb_data_transaction_get_method_of_payment_number ( pBreakdownTransaction );
			  pMiscList = g_slist_find_custom ( gsb_data_account_get_method_payment_list ( account_nb ),
							    GINT_TO_POINTER ( payment_method ),
							    (GCompareFunc) recherche_type_ope_par_no );

			  if ( pMiscList )
			  {
			      struct struct_type_ope * type = pMiscList -> data;

			      if ( type -> numerotation_auto )
				  csv_field_cheque = g_strdup ( gsb_data_transaction_get_method_of_payment_content ( pBreakdownTransaction ) );
			  }

			  /* Budgetary lines */
			  if ( gsb_data_transaction_get_budgetary_number ( pBreakdownTransaction ) != -1 )
			  {
			      csv_field_imput = g_strdup ( gsb_data_budget_get_name ( gsb_data_transaction_get_budgetary_number ( pBreakdownTransaction ), 0, "" ) );

			      if ( gsb_data_transaction_get_sub_budgetary_number ( pBreakdownTransaction ) != -1 )
			      {
				  csv_field_sous_imput = g_strdup ( gsb_data_budget_get_sub_budget_name ( gsb_data_transaction_get_budgetary_number ( pBreakdownTransaction ),
													  gsb_data_transaction_get_sub_budgetary_number ( pBreakdownTransaction ),
													  NULL ) );
			      }
			  }

			  /* Piece comptable */
			  csv_field_piece = g_strdup( gsb_data_transaction_get_voucher ( pBreakdownTransaction ) );

			  /* Financial Year */
			  if ( gsb_data_transaction_get_financial_year_number ( pBreakdownTransaction ) != -1 )
			      csv_field_exercice  = g_strdup(exercice_name_by_no(gsb_data_transaction_get_financial_year_number ( pBreakdownTransaction )));

			  csv_add_record(csv_file,FALSE);
		      }

		      pBreakdownTransactionList = pBreakdownTransactionList -> next;
		  }
		  csv_clear_fields(TRUE);
	      }
	      else
	      {
		  /* Si c'est un virement ... */
		  if ( gsb_data_transaction_get_transaction_number_transfer ( pTransaction ))
		  {
		      csv_field_categ = g_strdup(_("Transfer"));

		      /* ... vers un compte existant */
		      if ( gsb_data_transaction_get_account_number_transfer (  pTransaction ) >= 0 )
		      {
			  csv_field_sous_categ = g_strdup(g_strconcat ( "[", 
									gsb_data_account_get_name ( gsb_data_transaction_get_account_number_transfer ( pTransaction ) ), 
									"]", NULL ));
		      }
		      /* ... vers un compte supprimé */
		      else
		      {
			  csv_field_sous_categ = g_strdup(g_strconcat ( "[", _("Deleted account"), "]", NULL ));
		      }
		  }
		  else
		  {
		      /* c'est du type categ : sous-categ */
		      if ( gsb_data_transaction_get_category_number ( pTransaction ) != -1 )
		      {
			  csv_field_categ = g_strdup ( gsb_data_budget_get_name ( gsb_data_transaction_get_category_number ( pTransaction ), 0, "" ) );

			  if ( gsb_data_transaction_get_sub_category_number ( pTransaction ) != -1 )
			  {
			      csv_field_sous_categ = g_strdup ( gsb_data_budget_get_sub_budget_name ( gsb_data_transaction_get_category_number ( pTransaction ),
												      gsb_data_transaction_get_sub_category_number ( pTransaction ),
												      NULL ) );
			  }
		      }
		  }

		  csv_add_record(csv_file,TRUE);
	      }
	  }
	  pTransactionList = pTransactionList -> next;
      }
      fclose ( csv_file );
  }

}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
