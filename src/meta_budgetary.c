/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	     2004 Benjamin Drieu (bdrieu@april.org)	      */
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
#include "metatree.h"

/*START_INCLUDE*/
#include "meta_budgetary.h"
#include "utils_devises.h"
#include "gsb_data_budget.h"
#include "gsb_data_transaction.h"
#include "imputation_budgetaire.h"
#include "include.h"
#include "structures.h"
/*END_INCLUDE*/


/*START_STATIC*/
static gint budgetary_line_add_div ();
static gint budgetary_line_add_sub_div ( int div_id );
static gboolean budgetary_line_add_transaction_to_div ( gpointer  trans, 
						 int div_id );
static gboolean budgetary_line_add_transaction_to_sub_div ( gpointer  trans, 
						     int div_id, int sub_div_id );
static gdouble budgetary_line_div_balance ( gpointer div );
static gchar * budgetary_line_div_name ( gpointer div );
static gint budgetary_line_div_nb_transactions ( gpointer div );
static GSList * budgetary_line_div_sub_div_list ( gpointer div );
static gint budgetary_line_div_type ( gpointer div );
static gpointer budgetary_line_get_div_pointer_from_name ( gchar * name, gboolean create );
static gpointer budgetary_line_get_sub_div_pointer ( int div_id, int sub_div_id );
static gpointer budgetary_line_get_sub_div_pointer_from_name ( int div_id, gchar * name, 
							gboolean create );
static gpointer budgetary_line_get_without_div_pointer ( );
static gboolean budgetary_line_remove_transaction_from_div ( gpointer  trans, 
						      int div_id );
static gboolean budgetary_line_remove_transaction_from_sub_div ( gpointer  trans, 
							  int div_id, int sub_div_id );
static gint budgetary_line_scheduled_div_id ( struct operation_echeance * scheduled );
static void budgetary_line_scheduled_set_div_id ( struct operation_echeance * scheduled, 
					   int no_div );
static void budgetary_line_scheduled_set_sub_div_id ( struct operation_echeance * scheduled, 
					       int no_sub_div );
static gint budgetary_line_scheduled_sub_div_id ( struct operation_echeance * scheduled );
static gdouble budgetary_line_sub_div_balance ( gpointer div, gpointer sub_div );
static gchar * budgetary_line_sub_div_name ( gpointer sub_div );
static gint budgetary_line_sub_div_nb_transactions ( gpointer div, gpointer sub_div );
static gint budgetary_line_transaction_div_id ( gpointer  transaction );
static void budgetary_line_transaction_set_div_id ( gpointer  transaction, 
					     int no_div );
static void budgetary_line_transaction_set_sub_div_id ( gpointer  transaction, 
						 int no_sub_div );
static gint budgetary_line_transaction_sub_div_id ( gpointer  transaction );
static struct struct_devise * budgetary_line_tree_currency ( );
/*END_STATIC*/

/*START_EXTERN*/
extern GtkTreeStore *model;
extern gint no_devise_totaux_ib;
/*END_EXTERN*/


static MetatreeInterface _budgetary_interface = {
    2,
    budgetary_line_tree_currency,
    N_("Budgetary line"),
    N_("No budgetary line"),
    N_("No sub-budgetary line"),
    budgetary_line_get_without_div_pointer,
    gsb_data_budget_get_structure,
    budgetary_line_get_sub_div_pointer,
    budgetary_line_get_div_pointer_from_name,
    budgetary_line_get_sub_div_pointer_from_name,
    budgetary_line_div_nb_transactions,
    budgetary_line_sub_div_nb_transactions,
    budgetary_line_div_name,
    budgetary_line_sub_div_name,
    budgetary_line_div_balance,
    budgetary_line_sub_div_balance,
    gsb_data_budget_get_no_budget,
    gsb_data_budget_get_no_sub_budget,
    gsb_data_budget_get_budgets_list,
    budgetary_line_div_sub_div_list,
    budgetary_line_div_type,

    budgetary_line_transaction_div_id,
    budgetary_line_transaction_sub_div_id,
    budgetary_line_transaction_set_div_id,
    budgetary_line_transaction_set_sub_div_id,
    budgetary_line_scheduled_div_id,
    budgetary_line_scheduled_sub_div_id,
    budgetary_line_scheduled_set_div_id,
    budgetary_line_scheduled_set_sub_div_id,

    budgetary_line_add_div,
    budgetary_line_add_sub_div,
    gsb_data_budget_remove,
    gsb_data_budget_sub_budget_remove,
    budgetary_line_add_transaction_to_div,
    budgetary_line_add_transaction_to_sub_div,
    budgetary_line_remove_transaction_from_div,
    budgetary_line_remove_transaction_from_sub_div,
};

MetatreeInterface * budgetary_interface = &_budgetary_interface;


/**
 *
 *
 */
struct struct_devise * budgetary_line_tree_currency ( )
{
    return (struct struct_devise *) devise_par_no ( no_devise_totaux_ib );
}



/**
 *
 *
 */
gpointer budgetary_line_get_without_div_pointer ( )
{
    return NULL;
}



/**
 *
 *
 */
gpointer budgetary_line_get_sub_div_pointer ( int div_id, int sub_div_id )
{
    return gsb_data_budget_get_sub_budget_structure ( div_id, sub_div_id );
}



/**
 *
 *
 */
gpointer budgetary_line_get_div_pointer_from_name ( gchar * name, gboolean create )
{
    return gsb_data_budget_get_structure (gsb_data_budget_get_number_by_name ( name, create, 0 ));
}



/**
 *
 *
 */
gpointer budgetary_line_get_sub_div_pointer_from_name ( int div_id, gchar * name, 
							gboolean create )
{
    return gsb_data_budget_get_sub_budget_structure ( div_id,
							  gsb_data_budget_get_sub_budget_number_by_name( div_id, name, create));
}



/**
 *
 *
 */
gint budgetary_line_div_nb_transactions ( gpointer div )
{
    gint budget_number;

    budget_number = gsb_data_budget_get_no_budget (div);

    return gsb_data_budget_get_nb_transactions ( budget_number );
}



/**
 *
 *
 */
gint budgetary_line_sub_div_nb_transactions ( gpointer div, gpointer sub_div )
{
    gint budget_number;
    gint sub_budget_number;

    budget_number = gsb_data_budget_get_no_budget (div);
    sub_budget_number = gsb_data_budget_get_no_sub_budget (sub_div);

    if ( sub_div )
    {
	return gsb_data_budget_get_sub_budget_nb_transactions ( budget_number,
								    sub_budget_number );
    }
    else if ( div )
    {
	return gsb_data_budget_get_nb_direct_transactions (budget_number);
    }
    return 0;
}



/**
 *
 *
 */
gchar * budgetary_line_div_name ( gpointer div )
{
    gint budget_number;

    budget_number = gsb_data_budget_get_no_budget (div);

    return gsb_data_budget_get_name ( budget_number,
					0,
					"" );
}



/**
 *
 *
 */
gchar * budgetary_line_sub_div_name ( gpointer sub_div )
{
    gint budget_number;
    gint sub_budget_number;

    budget_number = gsb_data_budget_get_no_budget_from_sub_budget (sub_div);
    sub_budget_number = gsb_data_budget_get_no_sub_budget (sub_div);

    return gsb_data_budget_get_sub_budget_name ( budget_number,
						     sub_budget_number,
						     "" );
}



/**
 *
 *
 */
gdouble budgetary_line_div_balance ( gpointer div )
{
    gint budget_number;

    budget_number = gsb_data_budget_get_no_budget (div);

    return gsb_data_budget_get_balance ( budget_number );
}



/**
 *
 *
 */
gdouble budgetary_line_sub_div_balance ( gpointer div, gpointer sub_div )
{
    gint budget_number;
    gint sub_budget_number;

    budget_number = gsb_data_budget_get_no_budget (div);
    sub_budget_number = gsb_data_budget_get_no_sub_budget (sub_div);

    if ( sub_div )
    {
	return gsb_data_budget_get_sub_budget_balance ( budget_number,
							    sub_budget_number );
    }
    else if ( div )
    {
	return gsb_data_budget_get_direct_balance ( budget_number );
    }
    return 0;
}






/**
 *
 *
 */
GSList * budgetary_line_div_sub_div_list ( gpointer div )
{
    gint budget_number;

    budget_number = gsb_data_budget_get_no_budget (div);

    return gsb_data_budget_get_sub_budget_list (budget_number);
}



/**
 *
 *
 */
gint budgetary_line_div_type ( gpointer div )
{
    gint budget_number;

    budget_number = gsb_data_budget_get_no_budget (div);

    return gsb_data_budget_get_type (budget_number);
}



/**
 *
 * \return -1 if no type is supported in backend model, type otherwise.
 */
gint budgetary_line_transaction_div_id ( gpointer  transaction )
{
    if ( transaction )
    {
	if ( gsb_data_transaction_get_transaction_number_transfer ( gsb_data_transaction_get_transaction_number (transaction ))
	     || 
	     gsb_data_transaction_get_breakdown_of_transaction ( gsb_data_transaction_get_transaction_number (transaction )))
	    return -1;
	else
	    return gsb_data_transaction_get_budgetary_number ( gsb_data_transaction_get_transaction_number (transaction ));
    }
    return 0;
}



/**
 *
 *
 */
gint budgetary_line_transaction_sub_div_id ( gpointer  transaction )
{
   if ( transaction )
	return gsb_data_transaction_get_sub_budgetary_number ( gsb_data_transaction_get_transaction_number (transaction ));
    return 0;
}



/**
 *
 *
 */
void budgetary_line_transaction_set_div_id ( gpointer  transaction, 
					     int no_div )
{
    if ( transaction )
	gsb_data_transaction_set_budgetary_number ( gsb_data_transaction_get_transaction_number ( transaction ),
						    no_div);
}



/**
 *
 *
 */
void budgetary_line_transaction_set_sub_div_id ( gpointer  transaction, 
						 int no_sub_div )
{
    if ( transaction )
	gsb_data_transaction_set_sub_budgetary_number ( gsb_data_transaction_get_transaction_number ( transaction ),
							no_sub_div);
}



/**
 *
 * \return -1 if no type is supported in backend model, type otherwise.
 */
gint budgetary_line_scheduled_div_id ( struct operation_echeance * scheduled )
{
    if ( scheduled )
	return scheduled -> imputation;
    return 0;
}



/**
 *
 *
 */
gint budgetary_line_scheduled_sub_div_id ( struct operation_echeance * scheduled )
{
    if ( scheduled )
	return scheduled -> sous_imputation;
    return 0;
}



/**
 *
 *
 */
void budgetary_line_scheduled_set_div_id ( struct operation_echeance * scheduled, 
					   int no_div )
{
    if ( scheduled )
    {
	scheduled -> imputation = no_div;
	if ( !scheduled -> imputation )
	    scheduled -> compte_virement = 0;
    }
}



/**
 *
 *
 */
void budgetary_line_scheduled_set_sub_div_id ( struct operation_echeance * scheduled, 
					       int no_sub_div )
{
    if ( scheduled )
	scheduled -> sous_imputation = no_sub_div;
}



/**
 *
 *
 */
gint budgetary_line_add_div ()
{
    gint new_budget_number;
    gchar * name;
    int i = 1;

    /** Find a unique name for budget */
    name =  g_strdup (_("New budget"));

    while ( gsb_data_budget_get_number_by_name ( name, FALSE, 0 ))
    {
	g_free (name);
	i++;
	name = g_strdup_printf ( _("New budget #%d"), i ); 
    }

    new_budget_number = gsb_data_budget_get_number_by_name ( name, TRUE, 0 );
    mise_a_jour_combofix_imputation();

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
	return -1;

    /** Find a unique name for budget */
    name =  g_strdup (_("New sub-budget"));

    while ( gsb_data_budget_get_sub_budget_number_by_name ( div_id, name, FALSE ))
    {
	g_free (name);
	i++;
	name = g_strdup_printf ( _("New sub-budget #%d"), i ); 
    }

    new_sub_budget_number = gsb_data_budget_get_sub_budget_number_by_name ( div_id, name, TRUE );

    mise_a_jour_combofix_imputation();

    return new_sub_budget_number;
}




/**
 *
 *
 */
gboolean budgetary_line_add_transaction_to_div ( gpointer  trans, 
						 int div_id )
{
    gsb_data_budget_add_transaction_to_budget ( gsb_data_transaction_get_transaction_number (trans));
    return TRUE;
}



/**
 *
 *
 */
gboolean budgetary_line_add_transaction_to_sub_div ( gpointer  trans, 
						     int div_id, int sub_div_id )
{
    gsb_data_budget_add_transaction_to_budget ( gsb_data_transaction_get_transaction_number (trans));
    return TRUE;
}



/**
 *
 *
 */
gboolean budgetary_line_remove_transaction_from_div ( gpointer  trans, 
						      int div_id )
{
    gsb_data_budget_remove_transaction_from_budget ( gsb_data_transaction_get_transaction_number (trans));
    return TRUE;
}



/**
 *
 *
 */
gboolean budgetary_line_remove_transaction_from_sub_div ( gpointer  trans, 
							  int div_id, int sub_div_id )
{
    gsb_data_budget_remove_transaction_from_budget ( gsb_data_transaction_get_transaction_number (trans));
    return TRUE;
}
