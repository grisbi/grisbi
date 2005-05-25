/* ************************************************************************** */
/*                                  utils_operations.c                    */
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
#include "utils_operations.h"
#include "utils_ib.h"
#include "utils_categories.h"
#include "utils_tiers.h"
#include "gsb_transaction_data.h"
#include "search_glist.h"
#include "metatree.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void update_transaction_in_budgetary_line_tree ( gpointer  transaction );
static void update_transaction_in_categ_tree ( gpointer  transaction );
static void update_transaction_in_payee_tree ( gpointer  transaction );
/*END_STATIC*/


/*START_EXTERN*/
extern MetatreeInterface * budgetary_interface ;
extern GtkTreeStore *budgetary_line_tree_model;
extern GtkTreeStore * categ_tree_model;
extern MetatreeInterface * category_interface ;
extern MetatreeInterface * payee_interface ;
extern GtkTreeStore *payee_tree_model;
/*END_EXTERN*/





/*****************************************************************************************************/
/* renvoie l'adr de l'opÃ© demandÃ©e par son no de cheque */
/* ou NULL si pas trouvÃ©e */
/*****************************************************************************************************/
gpointer operation_par_cheque ( gint no_cheque,
				gint no_compte )
{
    GSList *liste_tmp;

    liste_tmp = g_slist_find_custom ( gsb_transaction_data_get_transactions_list (),
				      GINT_TO_POINTER ( no_cheque ),
				      (GCompareFunc) recherche_operation_par_cheque );

    if ( liste_tmp )
	return ( liste_tmp -> data );

    return NULL;
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* renvoie l'adr de l'opÃ© demandÃ©e par son no id*/
/* ou NULL si pas trouvÃ©e */
/*****************************************************************************************************/
gpointer operation_par_id ( gchar *no_id,
					       gint no_compte )
{
    GSList *liste_tmp;

    liste_tmp = g_slist_find_custom ( gsb_transaction_data_get_transactions_list (),
				      g_strstrip ( no_id ),
				      (GCompareFunc) recherche_operation_par_id );

    if ( liste_tmp )
	return ( liste_tmp -> data );

    return NULL;
}



/**
 *
 *
 */
void update_transaction_in_categ_tree ( gpointer  transaction )
{
    /* FIXME: Kludgeish, we should maintain a state. */
    calcule_total_montant_categ();
    update_transaction_in_tree ( category_interface, GTK_TREE_MODEL (categ_tree_model), transaction );
}



/**
 *
 *
 */
void update_transaction_in_budgetary_line_tree ( gpointer  transaction )
{
    /* FIXME: Kludgeish, we should maintain a state. */
    calcule_total_montant_budgetary_line();
    update_transaction_in_tree ( budgetary_interface, GTK_TREE_MODEL (budgetary_line_tree_model), 
				 transaction );
}



/**
 *
 *
 */
void update_transaction_in_payee_tree ( gpointer  transaction )
{
    /* FIXME: Kludgeish, we should maintain a state. */
    calcule_total_montant_payee();
    update_transaction_in_tree ( payee_interface, GTK_TREE_MODEL (payee_tree_model), transaction );
}



/**
 *
 *
 */
void update_transaction_in_trees ( gpointer  transaction )
{
    if ( DEBUG )
	printf ( "update_transaction_in_trees\n" );
    update_transaction_in_categ_tree ( transaction );
    update_transaction_in_payee_tree ( transaction );
    update_transaction_in_budgetary_line_tree ( transaction );
}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
