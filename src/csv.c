/* ************************************************************************** */
/* This file manage cvs export format                                         */
/*                                                                            */
/*                                  csv.c                                     */
/*                                                                            */
/*     Copyright (C)	2004 François Terrot (francois.terrot at grisbi.org)  */
/*			2005 Alain Portal (dionysos at grisbi.org)	      */
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
#include "structures.h"
#include "variables-extern.c"
#include "csv.h"

#include "devises.h"
#include "dialog.h"
#include "search_glist.h"
#include "utils.h"
#include "utils_files.h"
#include "exercice.h"

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
gchar*  csv_field_tiers      = NULL; /*!< Third party (string) */
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
  if (etat.utilise_exercice)
  {
    CSV_STR_FIELD(file,csv_field_exercice);
  }
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
void export_accounts_to_csv (GSList* export_entries_list )
{
  gchar *csv_filename = NULL;
  gchar *sMessage = NULL;
  GSList *pAccountList;
  FILE *csv_file;
  gdouble balance = 0;

  pAccountList = export_entries_list;

  while ( pAccountList )
  {
    /* Ouverture du fichier, si pb, on marque l'erreur et passe au fichier suivant */
    balance = 0.0;
    csv_filename = g_strdup ( gtk_entry_get_text ( GTK_ENTRY ( pAccountList -> data )));

    if ( !( csv_file = utf8_fopen ( csv_filename, "w" ) ))
    {
      sMessage = g_strdup_printf ( _("Error for the file \"%s\" :\n%s"),
				   csv_filename, strerror ( errno ) );
      dialogue ( sMessage );
      g_free ( sMessage );
      sMessage = NULL;
    }
    else
    {
      GSList *pTransactionList;
      struct structure_operation *pTransaction;

      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( pAccountList -> data ),
												   "no_compte" ));

      if (g_csv_with_title_line)
      {
	csv_field_operation  = g_strdup(_("Transactions"));
	csv_field_ventil     = g_strdup(_("Breakdown"));
	csv_field_date       = g_strdup(_("Date"));
	csv_field_date_val   = g_strdup(_("Value date"));
	csv_field_cheque     = g_strdup(_("Cheques"));
	if (etat.utilise_exercice)
	{
	  csv_field_exercice   = g_strdup(_("Financial year"));
	}
	csv_field_pointage   = g_strdup(_("C/R"));
	csv_field_tiers      = g_strdup(_("Third party"));
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
      csv_field_tiers = g_strdup_printf ( g_strconcat (_("Initial balance") , " [", NOM_DU_COMPTE, "]", NULL ) );

      /* met le solde initial */
      balance = SOLDE_INIT;
      csv_field_solde = g_strdup_printf ( "%4.2f", balance );
      if (SOLDE_INIT >= 0)
      {
	csv_field_credit = g_strdup_printf ( "%4.2f", SOLDE_INIT );
      }
      else
      {
	csv_field_debit = g_strdup_printf ( "%4.2f", -SOLDE_INIT );
      }

      csv_add_record(csv_file,TRUE);

      /* on met toutes les opérations */
      if ( LISTE_OPERATIONS )
      {
	pTransactionList = LISTE_OPERATIONS;

	while ( pTransactionList )
	{
	  gdouble amount;

	  pTransaction = pTransactionList -> data;

	  /* Si c'est une ventilation d'opération (càd une opération fille),
	     elle n'est pas traitée à la base du "if" mais plus loin, quand
	     son opé ventilée sera exportée */

	  if ( !pTransaction -> no_operation_ventilee_associee )
	  {
	    GSList* pMiscList = NULL; /* Miscellaneous list */
	    /* met la date */
	    csv_field_date = g_strdup_printf ("%d/%d/%d", pTransaction -> jour, pTransaction -> mois, pTransaction -> annee );
                        
	    if (pTransaction->date_bancaire)
	      csv_field_date_val = g_strdup_printf ("%d/%d/%d", pTransaction -> jour_bancaire, pTransaction -> mois_bancaire, pTransaction -> annee_bancaire );
	    else
	      csv_field_date_val = "";

	    /* met le pointage */
	    if      ( pTransaction -> pointe == 1 ) csv_field_pointage = g_strdup(_("C"));
	    else if ( pTransaction -> pointe == 2 ) csv_field_pointage = g_strdup(_("R"));
	    else if ( pTransaction -> pointe == 3 ) csv_field_pointage = g_strdup(_("T"));

	    /* met les notes */
	    CSV_CLEAR_FIELD(csv_field_notes);
	    if ( pTransaction -> notes )
	      csv_field_notes = g_strdup(pTransaction -> notes );

	    /* met le tiers */
	    pMiscList = g_slist_find_custom ( liste_struct_tiers,
					       GINT_TO_POINTER ( pTransaction -> tiers ),
					       (GCompareFunc) recherche_tiers_par_no );

	    CSV_CLEAR_FIELD(csv_field_tiers);
	    if ( pMiscList )
	      csv_field_tiers = g_strdup ( ((struct struct_tiers *)(pMiscList -> data )) -> nom_tiers );

	    /* met le numéro du rapprochement */
	    if ( pTransaction -> no_rapprochement )
	    {
	      pMiscList = g_slist_find_custom ( liste_no_rapprochements,
						GINT_TO_POINTER ( pTransaction -> no_rapprochement ),
						(GCompareFunc) recherche_no_rapprochement_par_no );

	      CSV_CLEAR_FIELD(csv_field_rappro);
	      if ( pMiscList )
		csv_field_rappro = g_strdup ( ((struct struct_no_rapprochement *)(pMiscList->data)) -> nom_rapprochement );
	    }
 
	    /* Met les informations bancaires de l'opération. Elles n'existent
	       qu'au niveau de l'opération mère */
	    CSV_CLEAR_FIELD(csv_field_info_bank);
	    if ( pTransaction -> info_banque_guichet )
	      csv_field_info_bank = g_strdup(pTransaction -> info_banque_guichet );

	    /* met le montant, transforme la devise si necessaire */

	    amount = calcule_montant_devise_renvoi ( pTransaction -> montant,
						     DEVISE,
						     pTransaction -> devise,
						     pTransaction -> une_devise_compte_egale_x_devise_ope,
						     pTransaction -> taux_change,
						     pTransaction -> frais_change );

	    if (amount > -0.0 )
	    {
	      csv_field_credit = g_strdup_printf ( "%4.2f", amount );
	    }
	    else
	    {
	      csv_field_debit  = g_strdup_printf ( "%4.2f", -amount );
	    }

	    /* met le chèque si c'est un type à numérotation automatique */
	    pMiscList = g_slist_find_custom ( TYPES_OPES,
					      GINT_TO_POINTER ( pTransaction -> type_ope ),
					      (GCompareFunc) recherche_type_ope_par_no );

	    if ( pMiscList )
	    {
	      struct struct_type_ope * type = pMiscList -> data;

	      if ( type -> numerotation_auto )
		csv_field_cheque = pTransaction -> contenu_type ;
	    }

	    /* Budgetary lines */
	    pMiscList = g_slist_find_custom ( liste_struct_imputation,
					      GINT_TO_POINTER ( pTransaction -> imputation ),
					      ( GCompareFunc ) recherche_imputation_par_no );

	    if ( pMiscList )
	    {
	      GSList *pSubBudgetaryLineList;

	      if ((( struct struct_imputation * )( pMiscList -> data )) -> nom_imputation)
	      {
		csv_field_imput = g_strdup((( struct struct_imputation * )( pMiscList -> data )) -> nom_imputation);

		pSubBudgetaryLineList = g_slist_find_custom ( (( struct struct_imputation * )( pMiscList -> data )) -> liste_sous_imputation,
							      GINT_TO_POINTER ( pTransaction -> sous_imputation ),
							      ( GCompareFunc ) recherche_sous_imputation_par_no );
		if ( pSubBudgetaryLineList )
		{
		  if ((( struct struct_sous_imputation * )( pSubBudgetaryLineList -> data )) -> nom_sous_imputation)
		  {
		    csv_field_sous_imput = g_strdup((( struct struct_sous_imputation * )( pSubBudgetaryLineList -> data )) -> nom_sous_imputation);
		  }
		}
	      }
	    }

	    /* Piece comptable */
	    csv_field_piece = g_strdup(pTransaction -> no_piece_comptable );

	    /* Balance */
	    balance += amount;
	    csv_field_solde = g_strdup_printf ( "%4.2f", balance );

	    csv_field_operation = g_strdup_printf("%d",pTransaction -> no_operation);

	    /* Financial Year */
	    if (etat.utilise_exercice)
	    {
	      csv_field_exercice  = g_strdup(exercice_name_by_no(pTransaction -> no_exercice));
	    }

	    /*  on met soit un virement, soit une ventilation, soit les catégories */

	    /* Si c'est une opération ventilée, on recherche toutes les ventilations
	       de cette opération et on les traite immédiatement. */
	    /* et les met à la suite */
	    /* la catégorie de l'opé sera celle de la première opé de ventilation */
	    if ( pTransaction -> operation_ventilee )
	    {
	      GSList *pBreakdownTransactionList;

	      csv_field_categ = g_strdup(_("Breakdown of transaction"));
	      csv_add_record(csv_file,FALSE);

	      pBreakdownTransactionList = LISTE_OPERATIONS;

	      while ( pBreakdownTransactionList )
	      {
		struct structure_operation *pBreakdownTransaction;

		pBreakdownTransaction = pBreakdownTransactionList -> data;

		if ( pBreakdownTransaction -> no_operation_ventilee_associee == pTransaction -> no_operation &&
		     ( pBreakdownTransaction -> categorie ||
		       pBreakdownTransaction -> relation_no_operation ))
		{
		  /* on commence par mettre la catég et sous categ de l'opé et de l'opé de ventilation */
		  csv_field_ventil = g_strdup(_("B")); // -> mark 

		  csv_field_operation = g_strdup_printf("%d",pBreakdownTransaction -> no_operation);

		  if ( pBreakdownTransaction -> relation_no_operation )
		  {
		    /* c'est un virement */

		    gpointer **save_ptab;
		    csv_field_categ = g_strdup(_("Transfer"));

		    save_ptab = p_tab_nom_de_compte_variable;

		    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + pBreakdownTransaction -> relation_no_compte;

		    csv_field_sous_categ = g_strdup(g_strconcat ( "[", NOM_DU_COMPTE, "]", NULL ));

		    p_tab_nom_de_compte_variable = save_ptab;
		  }
		  else
		  {
		    /* c'est du type categ : sous categ */

		    pMiscList = g_slist_find_custom ( liste_struct_categories,
						      GINT_TO_POINTER ( pBreakdownTransaction -> categorie ),
						      (GCompareFunc) recherche_categorie_par_no );

		    if ( pMiscList )
		    {
		      GSList *pCategoryList;
		      struct struct_categ *pCategory;

		      pCategory = pMiscList -> data;

		      pCategoryList = g_slist_find_custom ( pCategory -> liste_sous_categ,
							    GINT_TO_POINTER ( pBreakdownTransaction -> sous_categorie ),
							    (GCompareFunc) recherche_sous_categorie_par_no );
		      csv_field_categ = g_strdup(pCategory -> nom_categ);
		      if ( pCategoryList )
		      {
			csv_field_sous_categ = g_strdup(((struct struct_sous_categ *)(pCategoryList->data)) -> nom_sous_categ);
		      }
		    }
		  }

		  /* met les notes de la ventilation */

		  if ( pBreakdownTransaction -> notes )
		    csv_field_notes = g_strdup(pBreakdownTransaction -> notes);

		  /* met le montant de la ventilation */

		  amount = calcule_montant_devise_renvoi ( pBreakdownTransaction -> montant,
							   DEVISE,
							   pTransaction -> devise,
							   pTransaction -> une_devise_compte_egale_x_devise_ope,
							   pTransaction -> taux_change,
							   pTransaction -> frais_change );

		  csv_field_montant = g_strdup_printf ( "%4.2f", amount );

		  /* met le rapprochement */
		  if ( pBreakdownTransaction -> no_rapprochement )
		  {
		    pMiscList = g_slist_find_custom ( liste_no_rapprochements,
						      GINT_TO_POINTER ( pBreakdownTransaction -> no_rapprochement ),
						      (GCompareFunc) recherche_no_rapprochement_par_no );

		    CSV_CLEAR_FIELD(csv_field_rappro);
		    if ( pMiscList )
		      csv_field_rappro = g_strdup ( ((struct struct_no_rapprochement *)(pMiscList->data)) -> nom_rapprochement );
		  }

		  /* met le chèque si c'est un type à numérotation automatique */
		  pMiscList = g_slist_find_custom ( TYPES_OPES,
						    GINT_TO_POINTER ( pBreakdownTransaction -> type_ope ),
						    (GCompareFunc) recherche_type_ope_par_no );

		  if ( pMiscList )
		  {
		    struct struct_type_ope * type = pMiscList -> data;

		    if ( type -> numerotation_auto )
		      csv_field_cheque = pBreakdownTransaction -> contenu_type ;
		  }


		  /* Budgetary lines */
		  pMiscList = g_slist_find_custom ( liste_struct_imputation,
						    GINT_TO_POINTER ( pBreakdownTransaction -> imputation ),
						    ( GCompareFunc ) recherche_imputation_par_no );

		  if ( pMiscList )
		  {
		    GSList *pSubBudgetaryLineList;

		    if ((( struct struct_imputation * )( pMiscList -> data )) -> nom_imputation)
		    {
		      csv_field_imput = g_strdup((( struct struct_imputation * )( pMiscList -> data )) -> nom_imputation);

		      pSubBudgetaryLineList = g_slist_find_custom ( (( struct struct_imputation * )( pMiscList -> data )) -> liste_sous_imputation,
								    GINT_TO_POINTER ( pBreakdownTransaction -> sous_imputation ),
								    ( GCompareFunc ) recherche_sous_imputation_par_no );
		      if ( pSubBudgetaryLineList )
		      {
			if ((( struct struct_sous_imputation * )( pSubBudgetaryLineList -> data )) -> nom_sous_imputation)
			{
			  csv_field_sous_imput = g_strdup((( struct struct_sous_imputation * )( pSubBudgetaryLineList -> data )) -> nom_sous_imputation);
			}
		      }
		    }
		  }

		  /* Piece comptable */
		  csv_field_piece = g_strdup(pBreakdownTransaction -> no_piece_comptable );

		  /* Financial Year */
		  if (etat.utilise_exercice)
		  {
		    csv_field_exercice  = g_strdup(exercice_name_by_no(pBreakdownTransaction -> no_exercice));
		  }

		  csv_add_record(csv_file,FALSE);
		}

		pBreakdownTransactionList = pBreakdownTransactionList -> next;
	      }
	      csv_clear_fields(TRUE);
	    }
	    else
	    {
	      /* Si c'est un virement ... */
	      if ( pTransaction -> relation_no_operation )
	      {
		csv_field_categ = g_strdup(_("Transfer"));

		/* ... vers un compte existant */
		if ( pTransaction -> relation_no_compte >= 0 )
		{
		  gpointer **save_ptab;

		  save_ptab = p_tab_nom_de_compte_variable;
		  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + pTransaction -> relation_no_compte;

		  csv_field_sous_categ = g_strdup(g_strconcat ( "[", NOM_DU_COMPTE, "]", NULL ));

		  p_tab_nom_de_compte_variable = save_ptab;
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

		pMiscList = g_slist_find_custom ( liste_struct_categories,
						  GINT_TO_POINTER ( pTransaction -> categorie ),
						  (GCompareFunc) recherche_categorie_par_no );

		if ( pMiscList )
		{
		  GSList *pCategoryList;
		  struct struct_categ *pCategory;

		  pCategory = pMiscList -> data;
		  csv_field_categ = g_strdup(pCategory -> nom_categ);

		  pCategoryList = g_slist_find_custom ( pCategory -> liste_sous_categ,
							GINT_TO_POINTER ( pTransaction -> sous_categorie ),
							(GCompareFunc) recherche_sous_categorie_par_no );
		  if ( pCategoryList )
		    csv_field_sous_categ = g_strdup(((struct struct_sous_categ *)(pCategoryList->data)) -> nom_sous_categ);
		  }
		}
		csv_add_record(csv_file,TRUE);
	      }
	    }
	    pTransactionList = pTransactionList -> next;
	  }
	}
      fclose ( csv_file );
    }
    pAccountList = pAccountList -> next;
    g_free ( csv_filename );
    csv_filename = NULL;
  }
}
