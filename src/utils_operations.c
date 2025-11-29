/* ************************************************************************** */
/*                                  utils_operations.c                    */
/*                                                                            */
/*     Copyright (C)	2000-2003 CÃ©dric Auger (cedric@grisbi.org)	      */
/*			2003-2004 Benjamin Drieu (bdrieu@april.org)	      */
/*			2003-2004 Alain Portal (aportal@univ-montp2.fr)	      */
/* 			https://www.grisbi.org				      */
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
/*  along with this program; if not, see <https://www.gnu.org/licenses/>.     */
/*                                                                            */
/* ************************************************************************** */

#include "config.h"

#include "include.h"


/*START_INCLUDE*/
#include "utils_operations.h"
#include "categories_onglet.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_payee.h"
#include "imputation_budgetaire.h"
#include "meta_budgetary.h"
#include "meta_categories.h"
#include "meta_payee.h"
#include "metatree.h"
#include "tiers_onglet.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void delete_transaction_in_budgetary_line_tree ( gint transaction_number );
static void delete_transaction_in_categ_tree ( gint transaction_number );
static void delete_transaction_in_payee_tree ( gint transaction_number );
static void update_transaction_in_budgetary_line_tree ( gint transaction_number );
static void update_transaction_in_categ_tree ( gint transaction_number );
static void update_transaction_in_payee_tree ( gint transaction_number );
/*END_STATIC*/


/*START_EXTERN*/
/*END_EXTERN*/


/**
 * update the given transaction in the metatrees
 *
 * \param transaction_number	the transaction to update
 *
 * \return
 */
void update_transaction_in_trees ( gint transaction_number )
{
    devel_debug_int (transaction_number);

    update_transaction_in_categ_tree ( transaction_number );
    update_transaction_in_payee_tree ( transaction_number );
    update_transaction_in_budgetary_line_tree ( transaction_number );
}

/**
 * delete the given transaction in the metatrees
 *
 * \param transaction_number	the transaction to delete
 *
 * \return
 */
void delete_transaction_in_trees ( gint transaction_number )
{
    devel_debug_int (transaction_number);

    if ( transaction_number < 0 )
        return;

    delete_transaction_in_categ_tree ( transaction_number );
    delete_transaction_in_payee_tree ( transaction_number );
    delete_transaction_in_budgetary_line_tree ( transaction_number );
}


/**
 *
 *
 */
void update_transaction_in_categ_tree ( gint transaction_number )
{
    MetatreeInterface *category_interface;

    category_interface = category_get_metatree_interface ( );
    /* FIXME: Kludgeish, we should maintain a state. */
    gsb_data_category_update_counters ( );
    update_transaction_in_tree ( category_interface,
                                 GTK_TREE_MODEL ( categories_get_tree_store ( ) ),
                                 transaction_number );
}



/**
 *
 *
 */
void update_transaction_in_budgetary_line_tree ( gint transaction_number )
{
    MetatreeInterface *budgetary_interface;

    budgetary_interface = budgetary_line_get_metatree_interface ( );
    /* FIXME: Kludgeish, we should maintain a state. */
    gsb_data_budget_update_counters ( );
    update_transaction_in_tree ( budgetary_interface,
                        GTK_TREE_MODEL ( budgetary_lines_get_tree_store ( ) ),
                        transaction_number );
}



/**
 *
 *
 */
void update_transaction_in_payee_tree ( gint transaction_number )
{
    MetatreeInterface *payee_interface;

    payee_interface = payee_get_metatree_interface ( );
    /* FIXME: Kludgeish, we should maintain a state. */
    gsb_data_payee_update_counters ();
    update_transaction_in_tree ( payee_interface,
                        GTK_TREE_MODEL ( payees_get_tree_store ( ) ),
                        transaction_number );
}

/**
 *
 *
 */
void delete_transaction_in_categ_tree ( gint transaction_number )
{
    MetatreeInterface *category_interface;

    category_interface = category_get_metatree_interface ( );
    gsb_data_category_remove_transaction_from_category (transaction_number);
    metatree_remove_transaction ( GTK_TREE_VIEW ( categories_get_tree_view ( ) ),
                                  category_interface,
                                  transaction_number,
                                  FALSE);
}



/**
 *
 *
 */
void delete_transaction_in_budgetary_line_tree ( gint transaction_number )
{
    MetatreeInterface *budgetary_interface;

    budgetary_interface = budgetary_line_get_metatree_interface ( );
    gsb_data_budget_remove_transaction_from_budget (transaction_number);
    metatree_remove_transaction ( GTK_TREE_VIEW ( budgetary_lines_get_tree_view ( ) ),
                        budgetary_interface,
                        transaction_number,
                        FALSE );
}



/**
 *
 *
 */
void delete_transaction_in_payee_tree ( gint transaction_number )
{
    MetatreeInterface *payee_interface;

    payee_interface = payee_get_metatree_interface ( );
    gsb_data_payee_remove_transaction_from_payee (transaction_number);
    metatree_remove_transaction ( GTK_TREE_VIEW ( payees_get_tree_view ( ) ),
                        payee_interface,
                        transaction_number,
                        FALSE );
}




/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
