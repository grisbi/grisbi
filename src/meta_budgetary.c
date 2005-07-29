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
#include "utils_ib.h"
#include "utils_devises.h"
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
static gint budgetary_line_div_id ( gpointer budgetary_line );
static GSList * budgetary_line_div_list ( );
static gchar * budgetary_line_div_name ( gpointer div );
static gint budgetary_line_div_nb_transactions ( gpointer div );
static GSList * budgetary_line_div_sub_div_list ( gpointer div );
static gint budgetary_line_div_type ( gpointer div );
static gpointer budgetary_line_get_div_pointer ( int div_id );
static gpointer budgetary_line_get_div_pointer_from_name ( gchar * name, gboolean create );
static gpointer budgetary_line_get_sub_div_pointer ( int div_id, int sub_div_id );
static gpointer budgetary_line_get_sub_div_pointer_from_name ( int div_id, gchar * name, 
							gboolean create );
static gpointer budgetary_line_get_without_div_pointer ( );
static gboolean budgetary_line_remove_div ( int div_id );
static gboolean budgetary_line_remove_sub_div ( int div_id, int sub_div_id );
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
static gint budgetary_line_sub_div_id ( gpointer sub_budgetary_line );
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
extern GSList *liste_struct_imputation;
extern gint mise_a_jour_combofix_imputation_necessaire;
extern GtkTreeStore *model;
extern gint nb_enregistrements_imputations;
extern gint no_devise_totaux_ib;
/*END_EXTERN*/


static MetatreeInterface _budgetary_interface = {
    2,
    budgetary_line_tree_currency,
    N_("Budgetary line"),
    N_("No budgetary line"),
    N_("No sub-budgetary line"),
    budgetary_line_get_without_div_pointer,
    budgetary_line_get_div_pointer,
    budgetary_line_get_sub_div_pointer,
    budgetary_line_get_div_pointer_from_name,
    budgetary_line_get_sub_div_pointer_from_name,
    budgetary_line_div_nb_transactions,
    budgetary_line_sub_div_nb_transactions,
    budgetary_line_div_name,
    budgetary_line_sub_div_name,
    budgetary_line_div_balance,
    budgetary_line_sub_div_balance,
    budgetary_line_div_id,
    budgetary_line_sub_div_id,
    budgetary_line_div_list,
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
    budgetary_line_remove_div,
    budgetary_line_remove_sub_div,
    budgetary_line_add_transaction_to_div,
    budgetary_line_add_transaction_to_sub_div,
    budgetary_line_remove_transaction_from_div,
    budgetary_line_remove_transaction_from_sub_div,
};

MetatreeInterface * budgetary_interface = &_budgetary_interface;
struct struct_imputation * without_budgetary_line;


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
    return (gpointer) without_budgetary_line;
}



/**
 *
 *
 */
gpointer budgetary_line_get_div_pointer ( int div_id )
{
    return (gpointer) imputation_par_no ( div_id );
}



/**
 *
 *
 */
gpointer budgetary_line_get_sub_div_pointer ( int div_id, int sub_div_id )
{
    return sous_imputation_par_no ( div_id, sub_div_id );
}



/**
 *
 *
 */
gpointer budgetary_line_get_div_pointer_from_name ( gchar * name, gboolean create )
{
    return imputation_par_nom ( name, create, 0, 0 );
}



/**
 *
 *
 */
gpointer budgetary_line_get_sub_div_pointer_from_name ( int div_id, gchar * name, 
							gboolean create )
{
    return sous_imputation_par_nom ( budgetary_line_get_div_pointer (div_id), name, create );
}



/**
 *
 *
 */
gint budgetary_line_div_nb_transactions ( gpointer div )
{
    return ((struct struct_imputation *) div) -> nb_transactions;
}



/**
 *
 *
 */
gint budgetary_line_sub_div_nb_transactions ( gpointer div, gpointer sub_div )
{
    if ( sub_div )
    {
	return ((struct struct_sous_imputation *) sub_div) -> nb_transactions;
    }
    else if ( div )
    {
	return ((struct struct_imputation *) div) -> nb_direct_transactions;
    }
    return 0;
}



/**
 *
 *
 */
gchar * budgetary_line_div_name ( gpointer div )
{
    return ((struct struct_imputation *) div) -> nom_imputation;
}



/**
 *
 *
 */
gchar * budgetary_line_sub_div_name ( gpointer sub_div )
{
    if ( sub_div )
	return ((struct struct_sous_imputation *) sub_div) -> nom_sous_imputation;
    return "";
}



/**
 *
 *
 */
gdouble budgetary_line_div_balance ( gpointer div )
{
    return ((struct struct_imputation *) div) -> balance;
}



/**
 *
 *
 */
gdouble budgetary_line_sub_div_balance ( gpointer div, gpointer sub_div )
{
    if ( sub_div )
    {
	return ((struct struct_sous_imputation *) sub_div) -> balance;
    }
    else if ( div )
    {
	return ((struct struct_imputation *) div) -> direct_balance;
    }
    return 0;
}



/**
 *
 *
 */
gint budgetary_line_div_id ( gpointer budgetary_line )
{
    if ( budgetary_line )
	return ((struct struct_imputation *) budgetary_line) -> no_imputation;
    return 0;
}



/**
 *
 *
 */
gint budgetary_line_sub_div_id ( gpointer sub_budgetary_line )
{
    if ( sub_budgetary_line )
	return ((struct struct_sous_imputation *) sub_budgetary_line) -> no_sous_imputation;
    return 0;
}



/**
 *
 *
 */
GSList * budgetary_line_div_list ( )
{
    return liste_struct_imputation;
}



/**
 *
 *
 */
GSList * budgetary_line_div_sub_div_list ( gpointer div )
{
    if ( div )
	return ((struct struct_imputation *) div) -> liste_sous_imputation;
    return NULL;
}



/**
 *
 *
 */
gint budgetary_line_div_type ( gpointer div )
{
    if ( div )
	return ((struct struct_imputation *) div) -> type_imputation;
    return 0;
}



/**
 *
 * \return -1 if no type is supported in backend model, type otherwise.
 */
gint budgetary_line_transaction_div_id ( gpointer  transaction )
{
    if ( transaction )
	return gsb_data_transaction_get_budgetary_number ( gsb_data_transaction_get_transaction_number (transaction ));
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
    struct struct_imputation * new_budgetary_line;
    gchar * name;
    int i = 1;

    /** Find a unique name for budgetary_line */
    name =  g_strdup ( _("New budgetary line") );

    while ( imputation_par_nom ( name, 0, 0, 0 ) )
    {
	i++;
	name = g_strdup_printf ( _("New budgetary line #%d"), i ); 
    }

    new_budgetary_line = imputation_par_nom ( name, 1, 0, 0 );
    mise_a_jour_combofix_imputation_necessaire = 1;
    mise_a_jour_combofix_imputation();

    return new_budgetary_line -> no_imputation;
}



/**
 *
 *
 */
gint budgetary_line_add_sub_div ( int div_id )
{
    struct struct_imputation * parent_budgetary_line;
    struct struct_sous_imputation * new_sub_budgetary_line;
    gchar * name;
    int i = 1;

    parent_budgetary_line = budgetary_line_get_div_pointer ( div_id );

    /** Find a unique name for budgetary_line */
    name =  g_strdup ( _("New sub-budgetary line") );

    while ( sous_imputation_par_nom ( parent_budgetary_line, name, 0 ) )
    {
	i++;
	name = g_strdup_printf ( _("New sub-budgetary line #%d"), i ); 
    }

    new_sub_budgetary_line = sous_imputation_par_nom ( parent_budgetary_line, name, 1 );

    mise_a_jour_combofix_imputation_necessaire = 1;
    mise_a_jour_combofix_imputation();

    return new_sub_budgetary_line -> no_sous_imputation;
}



/**
 *
 *
 */
gboolean budgetary_line_remove_div ( int div_id )
{
    liste_struct_imputation = g_slist_remove ( liste_struct_imputation,
					       budgetary_line_get_div_pointer ( div_id ) );

    /** Then, update various counters and stuff. */
    nb_enregistrements_imputations--;
    mise_a_jour_combofix_imputation_necessaire = 1;
    mise_a_jour_combofix_imputation  ();

    return TRUE;
}



/**
 *
 *
 */
gboolean budgetary_line_remove_sub_div ( int div_id, int sub_div_id )
{
    struct struct_imputation * budgetary_line;
    struct struct_sous_imputation * sub_budgetary_line;

    budgetary_line = budgetary_line_get_div_pointer ( div_id );
    if ( budgetary_line )
    {
	sub_budgetary_line = budgetary_line_get_sub_div_pointer ( div_id, sub_div_id );
	budgetary_line -> liste_sous_imputation = g_slist_remove ( budgetary_line -> liste_sous_imputation,
								   sub_budgetary_line );
	mise_a_jour_combofix_imputation_necessaire = 1;
	mise_a_jour_combofix_imputation ();
	return TRUE;
    }
    return FALSE;
}



/**
 *
 *
 */
gboolean budgetary_line_add_transaction_to_div ( gpointer  trans, 
						 int div_id )
{
    add_transaction_to_budgetary_line ( trans, budgetary_line_get_div_pointer ( div_id ), 
					NULL );
    return TRUE;
}



/**
 *
 *
 */
gboolean budgetary_line_add_transaction_to_sub_div ( gpointer  trans, 
						     int div_id, int sub_div_id )
{
    add_transaction_to_budgetary_line ( trans, budgetary_line_get_div_pointer ( div_id ), 
					budgetary_line_get_sub_div_pointer ( div_id, 
									     sub_div_id ) );
    return TRUE;
}



/**
 *
 *
 */
gboolean budgetary_line_remove_transaction_from_div ( gpointer  trans, 
						      int div_id )
{
    remove_transaction_from_budgetary_line ( trans, 
					     budgetary_line_get_div_pointer (div_id), NULL );
    return TRUE;
}



/**
 *
 *
 */
gboolean budgetary_line_remove_transaction_from_sub_div ( gpointer  trans, 
							  int div_id, int sub_div_id )
{
    remove_transaction_from_budgetary_line ( trans, 
					     budgetary_line_get_div_pointer (div_id), 
					     budgetary_line_get_sub_div_pointer (div_id, sub_div_id) );
    return TRUE;
}
