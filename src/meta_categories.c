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

/*START_INCLUDE*/
#include "metatree.h"
#include "utils_categories.h"
#include "meta_categories.h"
#include "structures.h"
/*END_INCLUDE*/


/*START_STATIC*/
gpointer category_get_without_div_pointer ();
gpointer category_get_div_pointer (int);
gpointer category_get_sub_div_pointer (int, int);
gpointer category_get_div_pointer_from_name (gchar *,gboolean);
gpointer category_get_sub_div_pointer_from_name (int, gchar *,gboolean);
gint category_div_nb_transactions (gpointer);
gint category_sub_div_nb_transactions (gpointer,gpointer);
gchar * category_div_name (gpointer);
gchar * category_sub_div_name (gpointer);
gdouble category_div_balance (gpointer);
gdouble category_sub_div_balance (gpointer,gpointer);
gint category_div_id (gpointer);
gint category_sub_div_id (gpointer);
gint category_transaction_div_id (struct structure_operation *);
gint category_transaction_sub_div_id (struct structure_operation *);
void category_transaction_set_div_id (struct structure_operation *, int);
void category_transaction_set_sub_div_id (struct structure_operation *, int);
gint category_add_div ();
gint category_add_sub_div (int);
gboolean category_remove_div (int);
gboolean category_remove_sub_div (int, int);
gboolean category_add_transaction_to_div (struct structure_operation *, int);
gboolean category_add_transaction_to_sub_div (struct structure_operation *, int, int);
gboolean category_remove_transaction_from_div (struct structure_operation *, int);
gboolean category_remove_transaction_from_sub_div (struct structure_operation *, int, int);
GSList * category_div_list ( );
GSList * category_div_sub_div_list (gpointer);
gint category_div_type (gpointer);
gint category_scheduled_div_id (struct operation_echeance *);
gint category_scheduled_sub_div_id (struct operation_echeance *);
void category_scheduled_set_div_id (struct operation_echeance *, int);
void category_scheduled_set_sub_div_id (struct operation_echeance *, int);
struct struct_devise * category_tree_currency ( );
/*END_STATIC*/

/*START_EXTERN*/
extern GSList *liste_struct_categories;
extern gint nb_enregistrements_categories;
extern gint no_devise_totaux_categ;
/*END_EXTERN*/


static MetatreeInterface _category_interface = {
    2,
    category_tree_currency,
    N_("No category"),
    N_("No sub-category"),
    category_get_without_div_pointer,
    category_get_div_pointer,
    category_get_sub_div_pointer,
    category_get_div_pointer_from_name,
    category_get_sub_div_pointer_from_name,
    category_div_nb_transactions,
    category_sub_div_nb_transactions,
    category_div_name,
    category_sub_div_name,
    category_div_balance,
    category_sub_div_balance,
    category_div_id,
    category_sub_div_id,
    category_div_list,
    category_div_sub_div_list,
    category_div_type,

    category_transaction_div_id,
    category_transaction_sub_div_id,
    category_transaction_set_div_id,
    category_transaction_set_sub_div_id,
    category_scheduled_div_id,
    category_scheduled_sub_div_id,
    category_scheduled_set_div_id,
    category_scheduled_set_sub_div_id,

    category_add_div,
    category_add_sub_div,
    category_remove_div,
    category_remove_sub_div,
    category_add_transaction_to_div,
    category_add_transaction_to_sub_div,
    category_remove_transaction_from_div,
    category_remove_transaction_from_sub_div,
};

MetatreeInterface * category_interface = &_category_interface;
struct struct_categ * without_category;




/**
 *
 *
 */
struct struct_devise * category_tree_currency ( )
{
    return (struct struct_devise *) devise_par_no ( no_devise_totaux_categ );
}



/**
 *
 *
 */
gpointer category_get_without_div_pointer ( )
{
    return (gpointer) without_category;
}



/**
 *
 *
 */
gpointer category_get_div_pointer ( int div_id )
{
    return (gpointer) categ_par_no ( div_id );
}



/**
 *
 *
 */
gpointer category_get_sub_div_pointer ( int div_id, int sub_div_id )
{
    return (gpointer) sous_categ_par_no ( div_id, sub_div_id );
}



/**
 *
 *
 */
gpointer category_get_div_pointer_from_name ( gchar * name, gboolean create )
{
    return categ_par_nom ( name, create, 0, 0 );
}



/**
 *
 *
 */
gpointer category_get_sub_div_pointer_from_name ( int div_id, gchar * name, gboolean create )
{
    return sous_categ_par_nom ( category_get_div_pointer (div_id), name, create );
}



/**
 *
 *
 */
gint category_div_nb_transactions ( gpointer div )
{
    return ((struct struct_categ *) div) -> nb_transactions;
}



/**
 *
 *
 */
gint category_sub_div_nb_transactions ( gpointer div, gpointer sub_div )
{
    if ( sub_div )
    {
	return ((struct struct_sous_categ *) sub_div) -> nb_transactions;
    }
    else if ( div )
    {
	return ((struct struct_categ *) div) -> nb_direct_transactions;
    }
    return 0;
}



/**
 *
 *
 */
gchar * category_div_name ( gpointer div )
{
    return ((struct struct_categ *) div) -> nom_categ;
}



/**
 *
 *
 */
gchar * category_sub_div_name ( gpointer sub_div )
{
    if ( sub_div )
	return ((struct struct_sous_categ *) sub_div) -> nom_sous_categ;
    return "";
}



/**
 *
 *
 */
gdouble category_div_balance ( gpointer div )
{
    return ((struct struct_categ *) div) -> balance;
}



/**
 *
 *
 */
gdouble category_sub_div_balance ( gpointer div, gpointer sub_div )
{
    if ( sub_div )
    {
	return ((struct struct_sous_categ *) sub_div) -> balance;
    }
    else if ( div )
    {
	return ((struct struct_categ *) div) -> direct_balance;
    }
    return 0;
}



/**
 *
 *
 */
gint category_div_id ( gpointer category )
{
    if ( category )
	return ((struct struct_categ *) category) -> no_categ;
    return 0;
}



/**
 *
 *
 */
gint category_sub_div_id ( gpointer sub_category )
{
    if ( sub_category )
	return ((struct struct_sous_categ *) sub_category) -> no_sous_categ;
    return 0;
}



/**
 *
 *
 */
GSList * category_div_list ( )
{
    return liste_struct_categories;
}



/**
 *
 *
 */
GSList * category_div_sub_div_list ( gpointer div )
{
    if ( div )
	return ((struct struct_categ *) div) -> liste_sous_categ;
    return NULL;
}



/**
 *
 *
 */
gint category_div_type ( gpointer div )
{
    if ( div )
	return ((struct struct_categ *) div) -> type_categ;
    return 0;
}



/**
 *
 * \return -1 if no type is supported in backend model, type otherwise.
 */
gint category_transaction_div_id ( struct structure_operation * transaction )
{
    if ( transaction )
	return transaction -> categorie;
    return 0;
}



/**
 *
 *
 */
gint category_transaction_sub_div_id ( struct structure_operation * transaction )
{
    if ( transaction )
	return transaction -> sous_categorie;
    return 0;
}



/**
 *
 *
 */
void category_transaction_set_div_id ( struct structure_operation * transaction, 
				       int no_div )
{
    if ( transaction )
	transaction -> categorie = no_div;
}



/**
 *
 *
 */
void category_transaction_set_sub_div_id ( struct structure_operation * transaction, 
					   int no_sub_div )
{
    if ( transaction )
	transaction -> sous_categorie = no_sub_div;
}



/**
 *
 * \return -1 if no type is supported in backend model, type otherwise.
 */
gint category_scheduled_div_id ( struct operation_echeance * scheduled )
{
    if ( scheduled )
	return scheduled -> categorie;
    return 0;
}



/**
 *
 *
 */
gint category_scheduled_sub_div_id ( struct operation_echeance * scheduled )
{
    if ( scheduled )
	return scheduled -> sous_categorie;
    return 0;
}



/**
 *
 *
 */
void category_scheduled_set_div_id ( struct operation_echeance * scheduled, 
				     int no_div )
{
    if ( scheduled )
    {
	scheduled -> categorie = no_div;
	if ( !scheduled -> categorie )
	    scheduled -> compte_virement = 0;
    }
}



/**
 *
 *
 */
void category_scheduled_set_sub_div_id ( struct operation_echeance * scheduled, 
					 int no_sub_div )
{
    if ( scheduled )
	scheduled -> sous_categorie = no_sub_div;
}



/**
 *
 *
 */
gint category_add_div ()
{
    struct struct_categ * new_category;
    gchar * name;
    int i = 1;

    /** Find a unique name for category */
    name =  _("New category");

    while ( categ_par_nom ( name, 0, 0, 0 ) )
    {
	i++;
	name = g_strdup_printf ( _("New category #%d"), i ); 
    }

    new_category = categ_par_nom ( name, 1, 0, 0 );
    mise_a_jour_combofix_categ();

    return new_category -> no_categ;
}



/**
 *
 *
 */
gint category_add_sub_div ( int div_id )
{
    struct struct_categ * parent_category;
    struct struct_sous_categ * new_sub_category;
    gchar * name;
    int i = 1;

    parent_category = category_get_div_pointer ( div_id );
    if ( ! parent_category )
	return -1;

    /** Find a unique name for category */
    name =  _("New sub-category");

    while ( sous_categ_par_nom ( parent_category, name, 0 ) )
    {
	i++;
	name = g_strdup_printf ( _("New sub-category #%d"), i ); 
    }

    new_sub_category = sous_categ_par_nom ( parent_category, name, 1 );

    mise_a_jour_combofix_categ();
/*     modif_categ = 0; */

    return new_sub_category -> no_sous_categ;
}



/**
 *
 *
 */
gboolean category_remove_div ( int div_id )
{
    liste_struct_categories = g_slist_remove ( liste_struct_categories,
					       category_get_div_pointer ( div_id ) );

    /** Then, update various counters and stuff. */
    nb_enregistrements_categories--;
    mise_a_jour_combofix_categ  ();

    return TRUE;
}



/**
 *
 *
 */
gboolean category_remove_sub_div ( int div_id, int sub_div_id )
{
    struct struct_categ * categ;
    struct struct_sous_categ * sous_categ;

    categ = category_get_div_pointer ( div_id );
    if ( categ )
    {
	sous_categ = category_get_sub_div_pointer ( div_id, sub_div_id );
	categ -> liste_sous_categ = g_slist_remove ( categ -> liste_sous_categ,
						     sous_categ );
	mise_a_jour_combofix_categ ();
	return TRUE;
    }
    return FALSE;
}



/**
 *
 *
 */
gboolean category_add_transaction_to_div ( struct structure_operation * trans, 
					   int div_id )
{
    add_transaction_to_category ( trans, category_get_div_pointer ( div_id ), NULL );
    return TRUE;
}



/**
 *
 *
 */
gboolean category_add_transaction_to_sub_div ( struct structure_operation * trans, 
					       int div_id, int sub_div_id )
{
    add_transaction_to_category ( trans, category_get_div_pointer ( div_id ), 
				  category_get_sub_div_pointer ( div_id, sub_div_id ) );
    return TRUE;
}



/**
 *
 *
 */
gboolean category_remove_transaction_from_div ( struct structure_operation * trans, 
						int div_id )
{
    remove_transaction_from_category ( trans, 
				       category_get_div_pointer (div_id), NULL );
    return TRUE;
}



/**
 *
 *
 */
gboolean category_remove_transaction_from_sub_div ( struct structure_operation * trans, 
						    int div_id, int sub_div_id )
{
    remove_transaction_from_category ( trans, 
				       category_get_div_pointer (div_id), 
				       category_get_sub_div_pointer (div_id, sub_div_id) );
    return TRUE;
}
