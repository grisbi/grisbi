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
gchar*  csv_field_solde      = NULL; /*!< solde (numerical) */
gchar*  csv_field_categ      = NULL; /*!< category (string) */
gchar*  csv_field_sous_categ = NULL; /*!< sub category (string) */
gchar*  csv_field_imput      = NULL; /*!< budgetary line (string) */
gchar*  csv_field_sous_imput = NULL; /*!< sub budgetary line (string) */
gchar*  csv_field_notes      = NULL; /*!< notes (string) */
gchar*  csv_field_exercice   = NULL; /*!< exercices (string) optional depending of global grisbi configuration */
gchar*  csv_field_piece      = NULL; /*!< (string) */
gchar*  csv_field_cheque     = NULL; /*!< cheques */
gchar*  csv_field_rappro     = NULL; /*!< reconciliation number (string) */

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
  CSV_END_RECORD(file);
  csv_clear_fields(clear_all);
} /* }}} csv_add_record */

#define EMPTY_STR_FIELD fprintf(fichier_csv,"\"\""); /*!< empty string field value */
#define EMPTY_NUM_FIELD fprintf(fichier_csv,"0");    /*!< empty numerical field value */

/**
 * \brief export all account from the provided list in a csv file per account.
 *
 * \param export_entries_list list of selected account.
 *
 */
void export_accounts_to_csv (GSList* export_entries_list )
{
  gchar *nom_fichier_csv;
  GSList *liste_tmp;
  FILE *fichier_csv;
  gdouble solde = 0;

  liste_tmp = export_entries_list;

  while ( liste_tmp )
  {
    /*       ouverture du fichier, si pb, on marque l'erreur et passe au fichier suivant */
    solde = 0.0;
    nom_fichier_csv = g_strdup ( gtk_entry_get_text ( GTK_ENTRY ( liste_tmp -> data )));

    if ( !( fichier_csv = utf8_fopen ( nom_fichier_csv,
                                       "w" ) ))
    {
      dialogue ( g_strdup_printf ( _("Error for the file \"%s\" :\n%s"),
                                   nom_fichier_csv, strerror ( errno ) ));
    }
    else
    {
      GSList *pointeur_tmp;
      struct structure_operation *operation;

      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( liste_tmp -> data ),
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
      }

      csv_add_record(fichier_csv,TRUE);

      /* tiers */
      csv_field_tiers = g_strdup_printf ( g_strconcat (_("Initial balance") , " [", NOM_DU_COMPTE, "]", NULL ) );

      /* met le solde initial */
      solde = SOLDE_INIT;
      csv_field_solde = g_strdup_printf ( "%4.2f", solde );
      if (SOLDE_INIT >= 0)
      {
	csv_field_credit = g_strdup_printf ( "%4.2f", SOLDE_INIT );
      }
      else
      {
	csv_field_debit = g_strdup_printf ( "%4.2f", -SOLDE_INIT );
      }

      csv_add_record(fichier_csv,TRUE);

      /* on met toutes les opérations */
      if ( LISTE_OPERATIONS )
      {
	pointeur_tmp = LISTE_OPERATIONS;

	while ( pointeur_tmp )
	{
	  GSList *pointeur;
	  gdouble montant;

	  operation = pointeur_tmp -> data;

	  /* Si c'est une ventilation d'opération (càd une opération fille),
	     elle n'est pas traitée à la base du "if" mais plus loin, quand
	     son opé ventilée sera exportée */

	  if ( !operation -> no_operation_ventilee_associee )
	  {
	    GSList* pointer = NULL;
	    /* met la date */
	    csv_field_date = g_strdup_printf ("%d/%d/%d", operation -> jour, operation -> mois, operation -> annee );
                        
	    if (operation->date_bancaire)
	      csv_field_date_val = g_strdup_printf ("%d/%d/%d", operation -> jour_bancaire, operation -> mois_bancaire, operation -> annee_bancaire );
	    else
	      csv_field_date_val = "";

	    /* met le pointage */
	    if      ( operation -> pointe == 1 ) csv_field_pointage = g_strdup(_("C"));
	    else if ( operation -> pointe == 2 ) csv_field_pointage = g_strdup(_("R"));
	    else if ( operation -> pointe == 3 ) csv_field_pointage = g_strdup(_("T"));

	    /* met les notes */
	    CSV_CLEAR_FIELD(csv_field_notes);
	    if ( operation -> notes )
	      csv_field_notes = g_strdup(operation -> notes );

	    /* met le tiers */
	    pointeur = g_slist_find_custom ( liste_struct_tiers,
					     GINT_TO_POINTER ( operation -> tiers ),
					     (GCompareFunc) recherche_tiers_par_no );

	    CSV_CLEAR_FIELD(csv_field_tiers);
	    if ( pointeur )
	      csv_field_tiers = g_strdup ( ((struct struct_tiers *)(pointeur -> data )) -> nom_tiers );

	    /* met le rapprochement */
	    if ( operation -> no_rapprochement )
	    {
	      pointeur = g_slist_find_custom ( liste_no_rapprochements,
					       GINT_TO_POINTER ( operation -> no_rapprochement ),
					       (GCompareFunc) recherche_no_rapprochement_par_no );

	      CSV_CLEAR_FIELD(csv_field_rappro);
	      if ( pointeur )
		csv_field_rappro = g_strdup ( ((struct struct_no_rapprochement *)(pointeur->data)) -> nom_rapprochement );
	    }

	    /* met le montant, transforme la devise si necessaire */

	    montant = calcule_montant_devise_renvoi ( operation -> montant,
						      DEVISE,
						      operation -> devise,
						      operation -> une_devise_compte_egale_x_devise_ope,
						      operation -> taux_change,
						      operation -> frais_change );

	    if (montant > -0.0 )
	    {
	      csv_field_credit = g_strdup_printf ( "%4.2f", montant );
	    }
	    else
	    {
	      csv_field_debit  = g_strdup_printf ( "%4.2f", -montant );
	    }

	    /* met le chèque si c'est un type à numérotation automatique */
	    pointeur = g_slist_find_custom ( TYPES_OPES,
					     GINT_TO_POINTER ( operation -> type_ope ),
					     (GCompareFunc) recherche_type_ope_par_no );

	    if ( pointeur )
	    {
	      struct struct_type_ope * type = pointeur -> data;

	      if ( type -> numerotation_auto )
		csv_field_cheque = operation -> contenu_type ;
	    }

	    /* Budgetary lines */
	    pointer = g_slist_find_custom ( liste_struct_imputation,
					    GINT_TO_POINTER ( operation -> imputation ),
					    ( GCompareFunc ) recherche_imputation_par_no );

	    if ( pointer )
	    {
	      GSList *liste_tmp_2;

	      if ((( struct struct_imputation * )( pointer -> data )) -> nom_imputation)
	      {
		csv_field_imput = g_strdup((( struct struct_imputation * )( pointer -> data )) -> nom_imputation);

		liste_tmp_2 = g_slist_find_custom ( (( struct struct_imputation * )( pointer -> data )) -> liste_sous_imputation,
						    GINT_TO_POINTER ( operation -> sous_imputation ),
						    ( GCompareFunc ) recherche_sous_imputation_par_no );
		if ( liste_tmp_2 )
		{
		  if ((( struct struct_sous_imputation * )( liste_tmp_2 -> data )) -> nom_sous_imputation)
		  {
		    csv_field_sous_imput = g_strdup((( struct struct_sous_imputation * )( liste_tmp_2 -> data )) -> nom_sous_imputation);
		  }
		}
	      }
	    }

	    /* Piece comptable */
	    csv_field_piece = g_strdup(operation -> no_piece_comptable );

	    /* Balance */
	    solde += montant;
	    csv_field_solde = g_strdup_printf ( "%4.2f", solde );

	    csv_field_operation = g_strdup_printf("%d",operation -> no_operation);

	    /* Financial Year */
	    if (etat.utilise_exercice)
	    {
	      csv_field_exercice  = g_strdup(exercice_name_by_no(operation -> no_exercice));
	    }

	    /*  on met soit un virement, soit une ventilation, soit les catégories */

	    /* Si c'est une opération ventilée, on recherche toutes les ventilations
	       de cette opération et on les traite immédiatement. */
	    /* et les met à la suite */
	    /* la catégorie de l'opé sera celle de la première opé de ventilation */
	    if ( operation -> operation_ventilee )
	    {
	      GSList *liste_ventil;
	      gint categ_ope_mise;

	      csv_field_categ = g_strdup(_("Breakdown of transaction"));
	      csv_add_record(fichier_csv,FALSE);

	      categ_ope_mise = 0;
	      liste_ventil = LISTE_OPERATIONS;

	      while ( liste_ventil )
	      {
		struct structure_operation *ope_test;

		ope_test = liste_ventil -> data;

		if ( ope_test -> no_operation_ventilee_associee == operation -> no_operation &&
		     ( ope_test -> categorie ||
		       ope_test -> relation_no_operation ))
		{
		  /* on commence par mettre la catég et sous categ de l'opé et de l'opé de ventilation */
		  csv_field_ventil = g_strdup(_("B")); // -> mark 

		  csv_field_operation = g_strdup_printf("%d",ope_test -> no_operation);

		  if ( ope_test -> relation_no_operation )
		  {
		    /* c'est un virement */

		    gpointer **save_ptab;
		    csv_field_categ = g_strdup(_("Transfer"));

		    save_ptab = p_tab_nom_de_compte_variable;

		    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + ope_test -> relation_no_compte;

		    csv_field_sous_categ = g_strdup(g_strconcat ( "[", NOM_DU_COMPTE, "]", NULL ));

		    p_tab_nom_de_compte_variable = save_ptab;
		  }
		  else
		  {
		    /* c'est du type categ : sous categ */

		    pointeur = g_slist_find_custom ( liste_struct_categories,
						     GINT_TO_POINTER ( ope_test -> categorie ),
						     (GCompareFunc) recherche_categorie_par_no );

		    if ( pointeur )
		    {
		      GSList *pointeur_2;
		      struct struct_categ *categorie;

		      categorie = pointeur -> data;

		      pointeur_2 = g_slist_find_custom ( categorie -> liste_sous_categ,
							 GINT_TO_POINTER ( ope_test -> sous_categorie ),
							 (GCompareFunc) recherche_sous_categorie_par_no );
		      csv_field_categ = g_strdup(categorie -> nom_categ);
		      if ( pointeur_2 )
		      {
			csv_field_sous_categ = g_strdup(((struct struct_sous_categ *)(pointeur_2->data)) -> nom_sous_categ);
		      }
		    }
		  }

		  /* met les notes de la ventilation */

		  if ( ope_test -> notes )
		    csv_field_notes = g_strdup(ope_test -> notes);

		  /* met le montant de la ventilation */

		  montant = calcule_montant_devise_renvoi ( ope_test -> montant,
							    DEVISE,
							    operation -> devise,
							    operation -> une_devise_compte_egale_x_devise_ope,
							    operation -> taux_change,
							    operation -> frais_change );

		  csv_field_montant = g_strdup_printf ( "%4.2f", montant );

		  /* met le rapprochement */
		  if ( ope_test -> no_rapprochement )
		  {
		    pointeur = g_slist_find_custom ( liste_no_rapprochements,
						     GINT_TO_POINTER ( ope_test -> no_rapprochement ),
						     (GCompareFunc) recherche_no_rapprochement_par_no );

		    CSV_CLEAR_FIELD(csv_field_rappro);
		    if ( pointeur )
		      csv_field_rappro = g_strdup ( ((struct struct_no_rapprochement *)(pointeur->data)) -> nom_rapprochement );
		  }

		  /* met le chèque si c'est un type à numérotation automatique */
		  pointeur = g_slist_find_custom ( TYPES_OPES,
						   GINT_TO_POINTER ( ope_test -> type_ope ),
						   (GCompareFunc) recherche_type_ope_par_no );

		  if ( pointeur )
		  {
		    struct struct_type_ope * type = pointeur -> data;

		    if ( type -> numerotation_auto )
		      csv_field_cheque = ope_test -> contenu_type ;
		  }


		  /* Budgetary lines */
		  pointer = g_slist_find_custom ( liste_struct_imputation,
						  GINT_TO_POINTER ( ope_test -> imputation ),
						  ( GCompareFunc ) recherche_imputation_par_no );

		  if ( pointer )
		  {
		    GSList *liste_tmp_2;

		    if ((( struct struct_imputation * )( pointer -> data )) -> nom_imputation)
		    {
		      csv_field_imput = g_strdup((( struct struct_imputation * )( pointer -> data )) -> nom_imputation);

		      liste_tmp_2 = g_slist_find_custom ( (( struct struct_imputation * )( pointer -> data )) -> liste_sous_imputation,
							  GINT_TO_POINTER ( ope_test -> sous_imputation ),
							  ( GCompareFunc ) recherche_sous_imputation_par_no );
		      if ( liste_tmp_2 )
		      {
			if ((( struct struct_sous_imputation * )( liste_tmp_2 -> data )) -> nom_sous_imputation)
			{
			  csv_field_sous_imput = g_strdup((( struct struct_sous_imputation * )( liste_tmp_2 -> data )) -> nom_sous_imputation);
			}
		      }
		    }
		  }

		  /* Piece comptable */
		  csv_field_piece = g_strdup(ope_test -> no_piece_comptable );

		  /* Financial Year */
		  if (etat.utilise_exercice)
		  {
		    csv_field_exercice  = g_strdup(exercice_name_by_no(ope_test -> no_exercice));
		  }

		  csv_add_record(fichier_csv,FALSE);
		}

		liste_ventil = liste_ventil -> next;
	      }
	      csv_clear_fields(TRUE);
	    }
	    else
	    {
	      /* si c'est un virement vers un compte supprimé, ça sera pris comme categ normale vide */

	      if ( operation -> relation_no_operation &&
		   operation -> relation_no_compte >= 0 )
	      {
		/* c'est un virement */

		gpointer **save_ptab;
		csv_field_categ = g_strdup(_("Transfer"));

		save_ptab = p_tab_nom_de_compte_variable;

		p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> relation_no_compte;

		csv_field_sous_categ = g_strdup(g_strconcat ( "[", NOM_DU_COMPTE, "]", NULL ));

		p_tab_nom_de_compte_variable = save_ptab;
	      }
	      else
	      {
		/* c'est du type categ : sous-categ */

		pointeur = g_slist_find_custom ( liste_struct_categories,
						 GINT_TO_POINTER ( operation -> categorie ),
						 (GCompareFunc) recherche_categorie_par_no );

		if ( pointeur )
		{
		  GSList *pointeur_2;
		  struct struct_categ *categorie;

		  categorie = pointeur -> data;
		  csv_field_categ = g_strdup(categorie -> nom_categ);

		  pointeur_2 = g_slist_find_custom ( categorie -> liste_sous_categ,
						     GINT_TO_POINTER ( operation -> sous_categorie ),
						     (GCompareFunc) recherche_sous_categorie_par_no );
		  if ( pointeur_2 )
		    csv_field_sous_categ = g_strdup(((struct struct_sous_categ *)(pointeur_2->data)) -> nom_sous_categ);
		  }
		}
		csv_add_record(fichier_csv,TRUE);
	      }
	    }
	    pointeur_tmp = pointeur_tmp -> next;
	  }
	}
      fclose ( fichier_csv );
    }
    liste_tmp = liste_tmp -> next;
  }
}
