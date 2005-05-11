/* ************************************************************************** */
/*                                  utils_ib.c                    */
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
#include "utils_ib.h"
#include "gsb_account.h"
#include "gsb_transaction_data.h"
#include "search_glist.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void reset_budgetary_line_counters ();
/*END_STATIC*/


/*START_EXTERN*/
extern GSList *liste_struct_imputation;
extern gint mise_a_jour_combofix_imputation_necessaire;
extern gint nb_enregistrements_imputations;
extern gint no_derniere_imputation;
extern struct struct_imputation * without_budgetary_line;
/*END_EXTERN*/




/* **************************************************************************************************** */
/* retourne l'adr de la ib dont le nom est donnÃ© en argument */
/* si pas trouvÃ©e : */
/* renvoie NULL si creer = 0*/
/* la crÃ©e et renvoie son adr avec le type_ib et la derniere sous_ib donnÃ©s si creer = 1 */
/* type_ib = 0=crÃ©dit ; 1 = dÃ©bit  */
/* si on ajoute une ib, on met mise_a_jour_combofix_imputation_necessaire Ã  1 */
/* **************************************************************************************************** */

struct struct_imputation *imputation_par_nom ( gchar *nom_ib,
					       gboolean creer,
					       gint type_ib,
					       gint no_derniere_sous_ib )
{
    if ( nom_ib
	 &&
	 strlen (g_strstrip ( nom_ib )))
    {
	GSList *liste_tmp;

	liste_tmp = g_slist_find_custom ( liste_struct_imputation,
					  g_strstrip (nom_ib),
					  (GCompareFunc) recherche_imputation_par_nom );

	if ( liste_tmp )
	    return liste_tmp -> data;
	else
	{
	    if ( creer )
	    {
		struct struct_imputation *nouvelle_imputation;

		nouvelle_imputation = calloc ( 1,
					      sizeof ( struct struct_imputation ));

		nouvelle_imputation -> no_imputation = ++no_derniere_imputation;
		nouvelle_imputation -> nom_imputation = g_strdup ( g_strstrip ( nom_ib ));
		nouvelle_imputation -> type_imputation = type_ib;
		nouvelle_imputation -> no_derniere_sous_imputation = no_derniere_sous_ib;
		nouvelle_imputation -> nb_transactions = 0;
		nouvelle_imputation -> nb_direct_transactions = 0;
		nouvelle_imputation -> balance = 0;

		liste_struct_imputation = g_slist_append ( liste_struct_imputation,
							   nouvelle_imputation );
		nb_enregistrements_imputations++;
		mise_a_jour_combofix_imputation_necessaire =1;

		return ( nouvelle_imputation );
	    }
	}
    }
    return NULL;
}
/* **************************************************************************************************** */
   



/* **************************************************************************************************** */
/* retourne l'adr de la sous imputation dont le nom est donnÃ© en argument */
/* si pas trouvÃ©e : */
/* la crÃ©e et renvoie son adr si creer=1 */
/* renvoie NULL si creer = 0 */
/* si on ajoute une imputation, on met mise_a_jour_combofix_imputation_necessaire Ã  1 */
/* **************************************************************************************************** */

struct struct_sous_imputation *sous_imputation_par_nom ( struct struct_imputation *imputation,
							 gchar *nom_sous_imputation,
							 gboolean creer )
{
    if ( imputation
	 &&
	 nom_sous_imputation
	 &&
	 strlen ( g_strstrip ( nom_sous_imputation )))
    {
	GSList *liste_tmp;

	liste_tmp = g_slist_find_custom ( imputation -> liste_sous_imputation,
					  g_strstrip (nom_sous_imputation),
					  (GCompareFunc) recherche_sous_imputation_par_nom );

	if ( liste_tmp )
	    return liste_tmp -> data;
	else
	{
	    if ( creer )
	    {
		struct struct_sous_imputation *nouvelle_sous_imputation;

		nouvelle_sous_imputation = malloc ( sizeof ( struct struct_sous_imputation ));

		nouvelle_sous_imputation -> no_sous_imputation = ++( imputation -> no_derniere_sous_imputation );
		nouvelle_sous_imputation -> nom_sous_imputation = g_strdup ( g_strstrip ( nom_sous_imputation ));
		nouvelle_sous_imputation -> nb_transactions = 0;
		nouvelle_sous_imputation -> balance = 0;

		imputation -> liste_sous_imputation = g_slist_append ( imputation -> liste_sous_imputation,
							     nouvelle_sous_imputation );

		mise_a_jour_combofix_imputation_necessaire =1;
		return ( nouvelle_sous_imputation );
	    }
	}
    }

    return NULL;
}
/* **************************************************************************************************** */





/* **************************************************************************************************** */
/* cette fonction renvoie l'adr de la imputation demandÃ©e en argument */
/* et NULL si pas trouvÃ©e */
/* **************************************************************************************************** */
struct struct_imputation *imputation_par_no ( gint no_imputation )
{
    if ( no_imputation )
    {
	GSList *liste_tmp;

	liste_tmp = g_slist_find_custom ( liste_struct_imputation,
					  GINT_TO_POINTER ( no_imputation ),
					  (GCompareFunc) recherche_imputation_par_no );

	if ( liste_tmp )
	    return ( liste_tmp -> data );
    }
    return NULL;
}
/* **************************************************************************************************** */


/* **************************************************************************************************** */
/* cette fonction renvoie l'adr de la sous imputation demandÃ©e */
/* ou NULL si la imputation ou la sous imputation n'existe pas */
/* **************************************************************************************************** */
struct struct_sous_imputation *sous_imputation_par_no ( gint no_imputation,
							gint no_sous_imputation )
{
    if ( no_imputation
	 &&
	 no_sous_imputation )
    {
	GSList *liste_tmp;

	liste_tmp = g_slist_find_custom ( liste_struct_imputation,
					  GINT_TO_POINTER ( no_imputation ),
					  (GCompareFunc) recherche_imputation_par_no );

	if ( liste_tmp )
	{
	    struct struct_imputation *imputation;

	    imputation = liste_tmp -> data;

	    liste_tmp = g_slist_find_custom ( imputation -> liste_sous_imputation,
					      GINT_TO_POINTER ( no_sous_imputation ),
					      (GCompareFunc) recherche_sous_imputation_par_no );

	    if ( liste_tmp )
		return ( liste_tmp -> data );
	}
    }
    return NULL;
}
/* **************************************************************************************************** */




/* **************************************************************************************************** */
/* cette fonction renvoie une chaine de la forme */
/* soit imputation : sous imputation */
/* soit imputation si no_sous_imputation=0 ou si la sous_imputation n'existe pas */
/* ou NULL si la imputation n'existe pas */
/* **************************************************************************************************** */
gchar *nom_imputation_par_no ( gint no_imputation,
			       gint no_sous_imputation )
{
    struct struct_imputation *imputation;
    struct struct_sous_imputation *sous_imputation;

    imputation = imputation_par_no ( no_imputation );
    sous_imputation = sous_imputation_par_no ( no_imputation,
				     no_sous_imputation );

    if ( sous_imputation )
	/* 	s'il y a une sous imputation, c'est qu'il y a une imputation... */
	return ( g_strconcat ( imputation -> nom_imputation,
			       " : ",
			       sous_imputation -> nom_sous_imputation,
			       NULL ));
    else
	if ( imputation )
	    return ( g_strdup ( imputation -> nom_imputation ));
	
    return NULL;
}
/* **************************************************************************************************** */


/* **************************************************************************************************** */
/* cette fonction renvoie une chaine contenant la sous imputation */
/* ou NULL si la imputation ou la sous imputation n'existe pas */
/* **************************************************************************************************** */
gchar *nom_sous_imputation_par_no ( gint no_imputation,
				    gint no_sous_imputation )
{
    struct struct_sous_imputation *sous_imputation;

    sous_imputation = sous_imputation_par_no ( no_imputation,
					       no_sous_imputation );

    if ( sous_imputation )
	    return ( g_strdup ( sous_imputation -> nom_sous_imputation ));
	
    return NULL;
}
/* **************************************************************************************************** */



/**
 *
 *
 */
void calcule_total_montant_budgetary_line ( void )
{
    GSList *list_tmp;

    reset_budgetary_line_counters();

    without_budgetary_line = calloc ( 1, sizeof ( struct struct_imputation ));
    without_budgetary_line -> no_imputation = 0;
    without_budgetary_line -> nom_imputation = _("No budgetary line");
    without_budgetary_line -> type_imputation = 0;
    without_budgetary_line -> no_derniere_sous_imputation = 0;

    list_tmp = gsb_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;
	GSList *liste_tmp;

	i = gsb_account_get_no_account ( list_tmp -> data );

	liste_tmp = gsb_account_get_transactions_list (i);
	while ( liste_tmp )
	{
	    gpointer operation;

	    operation = liste_tmp -> data;

	    if ( gsb_transaction_data_get_budgetary_number ( gsb_transaction_data_get_transaction_number (operation )))
	    {
		struct struct_imputation * budgetary_line = NULL;
		struct struct_sous_imputation * sub_budgetary_line = NULL;

		/* il y a une catégorie */
		budgetary_line = imputation_par_no ( gsb_transaction_data_get_budgetary_number ( gsb_transaction_data_get_transaction_number (operation )));

		/* on ajoute maintenant le montant à la sous ib si elle existe */
		if ( gsb_transaction_data_get_sub_budgetary_number ( gsb_transaction_data_get_transaction_number (operation )))
		    sub_budgetary_line = sous_imputation_par_no ( gsb_transaction_data_get_budgetary_number ( gsb_transaction_data_get_transaction_number (operation )), 
								  gsb_transaction_data_get_sub_budgetary_number ( gsb_transaction_data_get_transaction_number (operation )));

		add_transaction_to_budgetary_line ( operation, budgetary_line, 
						    sub_budgetary_line );
	    }
	    else if ( ! gsb_transaction_data_get_breakdown_of_transaction ( gsb_transaction_data_get_transaction_number (operation ))
		      && 
		      ! gsb_transaction_data_get_transaction_number_transfer ( gsb_transaction_data_get_transaction_number (operation )))
	    {
		add_transaction_to_budgetary_line ( operation, without_budgetary_line, 
						    NULL );
	    }

	    liste_tmp = liste_tmp -> next;
	}

	list_tmp = list_tmp -> next;
    }
}



/**
 *
 *
 */
void remove_transaction_from_budgetary_line ( gpointer  transaction,
					      struct struct_imputation * budgetary_line,
					      struct struct_sous_imputation * sub_budgetary_line )
{
    gdouble amount = 
	gsb_transaction_data_get_adjusted_amount ( gsb_transaction_data_get_transaction_number (transaction));

    if ( budgetary_line )
    {
	budgetary_line -> nb_transactions --;
	budgetary_line -> balance -= amount;
	if ( !budgetary_line -> nb_transactions ) /* Cope with float errors */
	    budgetary_line -> balance = 0.0;
    }
    else
    {
	without_budgetary_line -> nb_transactions --;
	without_budgetary_line -> balance -= amount;
	if ( !without_budgetary_line -> nb_transactions ) /* Cope with float errors */
	    without_budgetary_line -> balance = 0.0;
    }
   
    if ( sub_budgetary_line )
    {
	sub_budgetary_line -> nb_transactions --;
	sub_budgetary_line -> balance -= amount;
	if ( !sub_budgetary_line -> nb_transactions ) /* Cope with float errors */
	    sub_budgetary_line -> balance = 0.0;
    }
    else if ( budgetary_line )
    {
	budgetary_line -> nb_direct_transactions --;
	budgetary_line -> direct_balance -= amount;
    }
    else
    {
	without_budgetary_line -> nb_direct_transactions --;
	without_budgetary_line -> direct_balance -= amount;
	if ( !without_budgetary_line -> nb_direct_transactions ) /* Cope with float errors */
	    without_budgetary_line -> direct_balance = 0.0;
    }

}



/**
 *
 *
 */
void add_transaction_to_budgetary_line ( gpointer  transaction,
					 struct struct_imputation * budgetary_line,
					 struct struct_sous_imputation * sub_budgetary_line )
{
    gdouble amount = 
	gsb_transaction_data_get_adjusted_amount ( gsb_transaction_data_get_transaction_number (transaction));

    if ( budgetary_line )
    {
	budgetary_line -> nb_transactions ++;
	budgetary_line -> balance += amount;
    }
    else 
    {
	without_budgetary_line -> nb_transactions ++;
	without_budgetary_line -> balance ++;
    }
   
    if ( sub_budgetary_line )
    {
	sub_budgetary_line -> nb_transactions ++;
	sub_budgetary_line -> balance += amount;
    }
    else if ( budgetary_line )
    {
	budgetary_line -> nb_direct_transactions ++;
	budgetary_line -> direct_balance += amount;
    }
    else 
    {
	without_budgetary_line -> nb_direct_transactions ++;
	without_budgetary_line -> direct_balance ++;
    }
}



/**
 *
 *
 */
void reset_budgetary_line_counters ()
{
    GSList * tmp;

    free ( without_budgetary_line );

    tmp = liste_struct_imputation;
    while ( tmp )
    {
	struct struct_imputation * budgetary_line = tmp -> data;
	GSList * sub_budgetary_line_list;;

	budgetary_line -> balance = 0.0;
	budgetary_line -> nb_transactions = 0;
	budgetary_line -> direct_balance = 0;
	budgetary_line -> nb_direct_transactions = 0;

	sub_budgetary_line_list = budgetary_line -> liste_sous_imputation;
	while ( sub_budgetary_line_list )
	{
	    struct struct_sous_imputation * sub_budgetary_line = sub_budgetary_line_list -> data;

	    sub_budgetary_line -> balance = 0.0;
	    sub_budgetary_line -> nb_transactions = 0;

	    sub_budgetary_line_list = sub_budgetary_line_list -> next;
	}
	
	tmp = tmp -> next;
    }
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
