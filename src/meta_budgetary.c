/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2004-2006 Benjamin Drieu (bdrieu@april.org)	      */
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
#include <glib/gi18n.h>


/*START_INCLUDE*/
#include "meta_budgetary.h"
#include "grisbi_win.h"
#include "gsb_data_budget.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_transaction.h"
#include "gsb_real.h"
#include "imputation_budgetaire.h"
#include "structures.h"
#include "utils_str.h"
/*END_INCLUDE*/


/*START_STATIC*/
static gint budgetary_line_add_div ( void );
static gint budgetary_line_add_sub_div ( int div_id );
static gboolean budgetary_line_add_transaction_to_div ( gint transaction_number,
						 int div_id );
static gchar *budgetary_line_div_name ( gint div );
static gint budgetary_line_get_without_div_pointer ( void );
static GsbReal budgetary_line_sub_div_balance ( gint div, gint sub_div );
static gchar *budgetary_line_sub_div_name ( gint div, gint sub_div );
static gint budgetary_line_sub_div_nb_transactions ( gint div, gint sub_div );
static gint budgetary_lint_get_number_by_name ( const gchar *name, gboolean create );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * return budgetary number
 *
 * \param	transaction_number
 *
 * \return -1 if split or contra transfert transaction other budgetary number
 **/
static gint budgetary_line_transaction_div_id (gint transaction_number)
{
	if (transaction_number)
	{
		if ( gsb_data_transaction_get_contra_transaction_number (transaction_number) > 0
			 ||
			 gsb_data_transaction_get_split_of_transaction (transaction_number))
			return -1;
		else
			return gsb_data_transaction_get_budgetary_number (transaction_number);
	}
	return 0;
}

static MetatreeInterface _budgetary_interface = {
    2,
    METATREE_BUDGET,
    budgetary_line_tree_currency,
    N_("Budgetary line"),
    N_("budgetary line"),
    N_("sub-budgetary line"),
    N_("No budgetary line"),
    N_("No sub-budgetary line"),
    budgetary_line_get_without_div_pointer,
    budgetary_lint_get_number_by_name,
    gsb_data_budget_get_sub_budget_number_by_name,
    gsb_data_budget_get_nb_transactions,
    budgetary_line_sub_div_nb_transactions,
    budgetary_line_div_name,
    budgetary_line_sub_div_name,
    gsb_data_budget_get_balance,
    budgetary_line_sub_div_balance,
    gsb_data_budget_get_no_budget,
    gsb_data_budget_get_no_sub_budget,
    gsb_data_budget_get_budgets_list,
    gsb_data_budget_get_sub_budget_list,
    gsb_data_budget_get_type,

    budgetary_line_transaction_div_id,
    gsb_data_transaction_get_sub_budgetary_number,
    gsb_data_transaction_set_budgetary_number,
    gsb_data_transaction_set_sub_budgetary_number,
    gsb_data_scheduled_get_budgetary_number,
    gsb_data_scheduled_get_sub_budgetary_number,
    gsb_data_scheduled_set_budgetary_number,
    gsb_data_scheduled_set_sub_budgetary_number,

    budgetary_line_add_div,
    budgetary_line_add_sub_div,
    gsb_data_budget_remove,
    gsb_data_budget_sub_budget_remove,
    budgetary_line_add_transaction_to_div,
    gsb_data_budget_add_transaction_to_budget,
    gsb_data_budget_remove_transaction_from_budget,
    gsb_data_budget_remove_transaction_from_budget,
    budgetary_hold_position_set_path,
    budgetary_hold_position_set_expand,
};

static MetatreeInterface *budgetary_interface = &_budgetary_interface;

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 *
 *
 */
gint budgetary_line_tree_currency ( void )
{
	GrisbiWinEtat *w_etat;

	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();

	return w_etat->no_devise_totaux_ib;
}



/**
 *
 *
 */
gint budgetary_line_get_without_div_pointer ( void )
{
    return 0;
}


/**
 *
 *
 */
gint budgetary_lint_get_number_by_name ( const gchar *name, gboolean create )
{
    return gsb_data_budget_get_number_by_name (name, create, 0 );
}


/**
 *
 *
 */
gint budgetary_line_sub_div_nb_transactions ( gint div, gint sub_div )
{
    if ( sub_div )
    {
	return gsb_data_budget_get_sub_budget_nb_transactions ( div,
								sub_div );
    }
    else if ( div )
    {
	return gsb_data_budget_get_nb_direct_transactions (div);
    }
    return 0;
}



/**
 *
 *
 */
gchar * budgetary_line_div_name ( gint div )
{
    return gsb_data_budget_get_name ( div,
				      0,
				      _(budgetary_interface -> no_div_label ));
}



/**
 *
 *
 */
gchar * budgetary_line_sub_div_name ( gint div, gint sub_div )
{
    return gsb_data_budget_get_sub_budget_name ( div,
						 sub_div,
						 _(budgetary_interface -> no_sub_div_label ));
}





/**
 *
 *
 */
GsbReal budgetary_line_sub_div_balance ( gint div, gint sub_div )
{
    if ( sub_div )
    {
	return gsb_data_budget_get_sub_budget_balance ( div,
							sub_div );
    }
    else if ( div )
    {
	return gsb_data_budget_get_direct_balance (div);
    }
    return null_real;
}




/**
 *
 *
 */
gint budgetary_line_add_div ( void )
{
    gint new_budget_number;
    gchar * name;
    int i = 1;

    /** Find a unique name for budget */
    name =  my_strdup (_("New budget"));

    while ( gsb_data_budget_get_number_by_name ( name, FALSE, 0 ))
    {
	g_free (name);
	i++;
	name = g_strdup_printf ( _("New budget #%d"), i );
    }

    new_budget_number = gsb_data_budget_get_number_by_name ( name, TRUE, 0 );

    return new_budget_number;
}



/**
 *
 *
 */
gint budgetary_line_add_sub_div ( int div_id )
{
    gint new_sub_budget_number;
    gchar * name;
    int i = 1;

    if ( !div_id )
	return 0;

    /** Find a unique name for budget */
    name =  my_strdup (_("New sub-budget"));

    while ( gsb_data_budget_get_sub_budget_number_by_name ( div_id, name, FALSE ))
    {
	g_free (name);
	i++;
	name = g_strdup_printf ( _("New sub-budget #%d"), i );
    }

    new_sub_budget_number = gsb_data_budget_get_sub_budget_number_by_name ( div_id, name, TRUE );

    return new_sub_budget_number;
}




/**
 *
 *
 *
 */
gboolean budgetary_line_add_transaction_to_div ( gint transaction_number,
						 int div_id )
{
    gsb_data_budget_add_transaction_to_budget ( transaction_number,
						div_id, 0 );
    return TRUE;
}


/**
 *
 *
 *
 */
MetatreeInterface *budgetary_line_get_metatree_interface ( void )
{
    return budgetary_interface;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
