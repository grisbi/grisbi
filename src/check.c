/* ************************************************************************** */
/* Ce fichier s'occupe de la vérification des montants rapprochés.            */
/* Il est introduit dans la version 0.5.4 suite à la découverte d'un bogue    */
/* qui provoquait les rapprochements des contre-opérations lors du            */
/* rapprochement d'opérations (qui étaient donc des virements)                */
/*                                                                            */
/* 				check.c                                       */
/*                                                                            */
/*     Copyright (C)	2004 Alain Portal (dionysos@grisbi.org)		      */
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

#include "devises.h"
#include "dialog.h"



/******************************************************************************/
/* reconciliation_check.                                                      */
/* Cette fonction est appelée après la création de toutes les listes.         */
/* Elle permet de vérifier la cohérence des rapprochements suite à la         */
/* découverte des bogues #466 et #488.                                        */
/******************************************************************************/
void reconciliation_check ( void )
{
  gint affected_accounts = 0;
  gint tested_account = 0;
  GSList *pUserAccountsList = NULL;
  gchar *pHint = NULL, *pText = "";

  /* s'il n'y a pas de compte, on quitte */
  if ( !nb_comptes )
      return;
    
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
	      if ( pTransaction -> pointe == OPERATION_RAPPROCHEE
		   &&
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
						      reconcilied_amount, devise_name_by_no ( DEVISE ),
						      SOLDE_DERNIER_RELEVE, devise_name_by_no ( DEVISE ) ),
				    NULL );
	  }
	  tested_account++;
      }
    }
    while ( (  pUserAccountsList = pUserAccountsList -> next ) );

    if ( !affected_accounts )
    {
	dialogue_hint ( _("Grisbi found no known inconsistency in accounts processed."),
			_("No inconsistency found.") );
    }
    else
    {

	pText = g_strconcat ( 
	    _("Grisbi found accounts where reconciliation totals are inconsistent "
	      "with the sum of reconcilied transactions.  Generally, the cause is "
	      "too many transfers to other accounts are reconciled.\n"
	      "The following accounts are inconsistent:\n\n"), 
	    pText, NULL );

      if ( affected_accounts > 1 )
	  pHint = g_strdup_printf ( _("%d accounts have inconsistencies."), 
				    affected_accounts );
      else
	  pHint = _("An account has inconsistencies.");

      dialogue_warning_hint ( pText, pHint );

      free ( pText );
      free ( pHint );
    }
    
    g_slist_free ( pUserAccountsList );
}
