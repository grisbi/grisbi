/* ************************************************************************** */
/* Ce fichier s'occupe de la vérification des montants rapprochés.            */
/* Il est introduit dans la version 0.5.4 suite à la découverte d'un bogue    */
/* qui provoquait les rapprochements des contre-opérations lors du            */
/* rapprochement d'opérations (qui étaient donc des virements)                */
/*                                                                            */
/* 				check.c                                       */
/*                                                                            */
/*     Copyright (C)	2004 Alain Portal (aportal@univ-montp2.fr)	      */
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

#include "include.h"
#include "structures.h"
#include "variables-extern.c"
#include "check.h"

#include "categories_onglet.h"
#include "devises.h"
#include "dialog.h"
#include "search_glist.h"


/*START_STATIC*/
gboolean reconciliation_check ( void );
gboolean duplicate_div_check ( void );
gboolean contra_transaction_check ( void );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/



/**
 * Performs various checks on Grisbi files.
 *
 * @return TRUE
 */
gboolean debug_check ( void )
{
    if ( !reconciliation_check() && !duplicate_div_check () && !contra_transaction_check() )
    {
	dialogue_hint ( _("Grisbi found no known inconsistency in accounts processed."),
			_("No inconsistency found.") );
    }
    
    return TRUE;
}



/******************************************************************************/
/* reconciliation_check.                                                      */
/* Cette fonction est appelée après la création de toutes les listes.         */
/* Elle permet de vérifier la cohérence des rapprochements suite à la         */
/* découverte des bogues #466 et #488.                                        */
/******************************************************************************/
gboolean reconciliation_check ( void )
{
  gint affected_accounts = 0;
  gint tested_account = 0;
  GSList *pUserAccountsList = NULL;
  gchar *pHint = NULL, *pText = "";

  /* S'il n'y a pas de compte, on quitte */
  if ( !nb_comptes )
    return 0;
    
  /* On fera la vérification des comptes dans l'ordre préféré
     de l'utilisateur. On fait une copie de la liste. */
  pUserAccountsList = g_slist_copy ( ordre_comptes );
  
  /* Pour chacun des comptes, faire */
  do
  {
    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + GPOINTER_TO_INT ( pUserAccountsList -> data );
      
    /* Si le compte a été rapproché au moins une fois.
       Seule la date permet de l'affirmer. */
    if ( DATE_DERNIER_RELEVE )
    {
      GSList *pTransactionList;
      gdouble reconcilied_amount = 0;

      /* On va recalculer le montant rapproché du compte (c-à-d le solde initial
         plus le montant des opérations rapprochées) et le comparer à la valeur
         stockée dans le fichier. Si les valeurs diffèrent, on affiche une boite
         d'avertissement */
      
      reconcilied_amount = SOLDE_INIT;

      /* On récupère la liste des opérations */
      pTransactionList = LISTE_OPERATIONS;

      while ( pTransactionList )
      {
	struct structure_operation *pTransaction;

	pTransaction = pTransactionList -> data;

	/* On ne prend en compte que les opérations rapprochées.
	   On ne prend pas en compte les opérations de ventilation. */
	if ( pTransaction -> pointe == RECONCILED_TRANSACTION &&
	     !pTransaction -> no_operation_ventilee_associee )
	{
	  reconcilied_amount += calcule_montant_devise_renvoi ( pTransaction -> montant,
								DEVISE,
								pTransaction -> devise,
								pTransaction -> une_devise_compte_egale_x_devise_ope,
								pTransaction -> taux_change,
								pTransaction -> frais_change );
	}
	pTransactionList = pTransactionList -> next;
      }

      if ( fabs ( reconcilied_amount - SOLDE_DERNIER_RELEVE ) >= 0.01 )
      {
	affected_accounts ++;

	pText = g_strconcat ( pText,
			      g_strdup_printf ( _("<span weight=\"bold\">%s</span>\n"
						  "  Last reconciliation amount : %4.2f%s\n"
						  "  Computed reconciliation amount : %4.2f%s\n\n"),
						NOM_DU_COMPTE, 
						SOLDE_DERNIER_RELEVE, devise_name_by_no ( DEVISE ),
						reconcilied_amount, devise_name_by_no ( DEVISE ) ),
			      NULL );
      }
      tested_account++;
    }
  }
  while ( (  pUserAccountsList = pUserAccountsList -> next ) );

  if ( affected_accounts )
  {
    pText = g_strconcat ( _("Grisbi found accounts where reconciliation totals are inconsistent "
			    "with the sum of reconcilied transactions.  Generally, the cause is "
			    "too many transfers to other accounts are reconciled.  You have to "
			    "manually unreconcile some transferts in inconsistent accounts.\n"
			    "The following accounts are inconsistent:\n\n"), 
			  pText, NULL );

    if ( affected_accounts > 1 )
    {
      pHint = g_strdup_printf ( _("%d accounts have inconsistencies."), 
				affected_accounts );
    }
    else
    {
      pHint = _("An account has inconsistencies.");
    }

    dialogue_warning_hint ( pText, pHint );

    free ( pText );
    free ( pHint );
  }
  g_slist_free ( pUserAccountsList );

  return affected_accounts;
}



/**
 * Find if two sub categories are the same
 *
 */
gint find_duplicate_categ ( struct struct_sous_categ * a, struct struct_sous_categ * b )
{
    if ( a != b && a -> no_sous_categ == b -> no_sous_categ )
    {
	return 0;
    }
    return 1;
}



/**
 *
 *
 */
gboolean duplicate_div_check ()
{
    GSList * tmp;
    gint num_duplicate = 0;
    gchar * output = "";

    tmp = liste_struct_categories;
    while ( tmp )
    {
	struct struct_categ * categ = tmp -> data;
	GSList * tmp_sous_categ = categ -> liste_sous_categ;

	while ( tmp_sous_categ )
	{
	    GSList * duplicate;
	    duplicate = g_slist_find_custom ( categ -> liste_sous_categ, 
					      tmp_sous_categ -> data,
					      (GCompareFunc) find_duplicate_categ );
	    /* Second comparison is just there to find only one of them. */
	    if ( duplicate && duplicate > tmp_sous_categ )
	    {
		output = g_strconcat ( output, 
				       g_strdup_printf ( _("Sub-category <i>'%s : %s'</i> is a duplicate of <i>'%s : %s'</i>\n"), 
							 categ -> nom_categ,
							 ((struct struct_sous_categ *) tmp_sous_categ -> data) -> nom_sous_categ,
							 categ -> nom_categ,
							 ((struct struct_sous_categ *) duplicate -> data) -> nom_sous_categ ),
				       NULL );
		num_duplicate ++;
	    }
	    tmp_sous_categ = tmp_sous_categ -> next;
	}
	
	tmp = tmp -> next;
    }

    if ( num_duplicate )
    {
	output = g_strconcat ( output, "\n",
			       _("Due to a bug in previous versions of Grisbi, "
				 "sub-categories may share the same numeric id in some "
				 "cases, resulting in transactions having two sub-categories.  "
				 "If you choose to continue, Grisbi will "
				 "remove one of each sub-categories duplicate and "
				 "recreate it with a new id.\n\n"
				 "No transactions will be lost, but in some cases, you "
				 "will have to manually move transactions to this new "
				 "sub-category."),
			       NULL );
	if ( question_yes_no_hint ( _("Fix inconsistencies in sub-categories?"), output ) )
	{
	    tmp = liste_struct_categories;
	    while ( tmp )
	    {
		struct struct_categ * categ = tmp -> data;
		GSList * tmp_sous_categ = categ -> liste_sous_categ;

		while ( tmp_sous_categ )
		{
		    GSList * duplicate;
		    duplicate = g_slist_find_custom ( categ -> liste_sous_categ, 
						      tmp_sous_categ -> data,
						      (GCompareFunc) find_duplicate_categ );
		    /* Second comparison is just there to find only one of them. */
		    if ( duplicate && duplicate > tmp_sous_categ )
		    {
			struct struct_sous_categ * duplicate_categ = duplicate -> data;

			duplicate_categ -> no_sous_categ = ++(categ -> no_derniere_sous_categ);
		    }
		    tmp_sous_categ = tmp_sous_categ -> next;
		}
	
		tmp = tmp -> next;
	    }

	    mise_a_jour_categ();
	}
    }

    return num_duplicate;
}


/******************************************************************************/
/* contra_transaction_check.                                                  */
/* Cette fonction est appelée après la création de toutes les listes.         */
/* Elle permet de vérifier la cohérence des virements entre comptes           */
/* suite à la découverte du bogue #542                                        */
/******************************************************************************/
gboolean contra_transaction_check ( void )
{
  gint affected_accounts = 0;
  gboolean corrupted_file = FALSE;
  GSList *pUserAccountsList = NULL;
  gchar *pHint = NULL, *pText = "";

  /* S'il n'y a pas de compte, on quitte */
  if ( !nb_comptes )
    return FALSE;
    
  /* On fera la vérification des comptes dans l'ordre préféré
     de l'utilisateur. On fait une copie de la liste. */
  pUserAccountsList = g_slist_copy ( ordre_comptes );
  
  /* Pour chacun des comptes, faire */
  do
  {
    gboolean corrupted_account = FALSE;
    GSList *pTransactionList;
    gchar *account_name = NULL;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + GPOINTER_TO_INT ( pUserAccountsList -> data );
      
    /* On affiche le nom du compte testé. Si le compte n'est pas affecté,
       on libèrera la mémoire */
    account_name = g_strdup_printf ("%s", NOM_DU_COMPTE);
    
    /* On récupère la liste des opérations */
    pTransactionList = LISTE_OPERATIONS;

    while ( pTransactionList )
    {
      struct structure_operation *pTransaction;

      pTransaction = pTransactionList -> data;

      /* Si l'opération est un virement vers un compte non supprimé */
      if ( pTransaction -> relation_no_operation != 0 &&
	   pTransaction -> relation_no_compte != -1 )
      {
	GSList *pList;
	gpointer **save_ptab;

	save_ptab = p_tab_nom_de_compte_variable;

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + pTransaction -> relation_no_compte;

	pList = g_slist_find_custom ( LISTE_OPERATIONS,
				      GINT_TO_POINTER ( pTransaction -> relation_no_operation ),
				      (GCompareFunc) recherche_operation_par_no ) ;
	
	if ( !pList )
	{
	  /* S'il n'y avait pas eu encore d'erreur dans ce compte,
	     on affiche son nom */
	  if ( !corrupted_account ) {
	    pText = g_strconcat ( pText,
				  g_strdup_printf ( "\n<span weight=\"bold\">%s</span>\n",
						    account_name), 
				  NULL );
	  }
	  pText = g_strconcat ( pText,
				g_strdup_printf ( _("Transaction #%d should have a contra #%d, "
						    "but this one doesn't exist.\n"),
						    pTransaction -> no_operation,
						    pTransaction -> relation_no_operation),
				NULL );
	  corrupted_account = TRUE;
	}
	else
	{
	  struct structure_operation *pContraTransaction;
	  
	  pContraTransaction = pList -> data;
	
	  if ( pTransaction -> relation_no_operation != pContraTransaction -> no_operation ||
	       pContraTransaction -> relation_no_operation != pTransaction -> no_operation )
	  {
	    /* S'il n'y avait pas eu encore d'erreur dans ce compte,
	       on affiche son nom */
	    if ( !corrupted_account ) {
	      pText = g_strconcat ( pText,
				    g_strdup_printf ( "\n<span weight=\"bold\">%s</span>\n",
						      account_name), 
				    NULL );
	    }
	    pText = g_strconcat ( pText,
				  g_strdup_printf ( _("Transaction #%d have a contra #%d, "
						      "but transaction #%d have a contra #%d "
						      "instead of #%d.\n"),
						    pTransaction -> no_operation,
						    pTransaction -> relation_no_operation,
						    pContraTransaction -> no_operation,
						    pContraTransaction -> relation_no_operation,
						    pTransaction -> no_operation),
				  NULL );
	    corrupted_account = TRUE;
	  }
	}
	p_tab_nom_de_compte_variable = save_ptab;
      }
      pTransactionList = pTransactionList -> next;
    }
    if ( corrupted_account ) {
      corrupted_file = TRUE;
      affected_accounts++;
    }
    g_free ( account_name );
  }
  while ( ( pUserAccountsList = pUserAccountsList -> next ) );

  if ( affected_accounts )
  {
    pText = g_strconcat ( _("Grisbi found transfer transactions where links are inconsistent "
			    "among themselves.  Unfortunately, we don't know at the moment "
			    "how it has happened.\n"
			    "The following accounts are inconsistent:\n"), 
			  pText, NULL );

    if ( affected_accounts > 1 )
    {
      pHint = g_strdup_printf ( _("%d accounts have inconsistencies."), 
				affected_accounts );
    }
    else
    {
      pHint = _("An account has inconsistencies.");
    }

    dialogue_warning_hint ( pText, pHint );

    free ( pText );
    free ( pHint );
  }
  g_slist_free ( pUserAccountsList );

  return corrupted_file;
}

