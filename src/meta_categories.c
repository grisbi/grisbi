/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2004-2008 Benjamin Drieu (bdrieu@april.org)	      */
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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"
#include "metatree.h"

/*START_INCLUDE*/
#include "meta_categories.h"
#include "categories_onglet.h"
#include "gsb_category.h"
#include "gsb_data_category.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_transaction.h"
#include "utils_str.h"
#include "gsb_real.h"
/*END_INCLUDE*/


/*START_STATIC*/
static gint category_add_div ();
static gint category_add_sub_div ( int div_id );
static gboolean category_add_transaction_to_div ( gint transaction_number, 
					   int div_id );
static gchar *category_div_name ( gint div );
static gint category_get_div_pointer_from_name ( const gchar * name, gboolean create );
static gint category_get_without_div_pointer ( );
static gboolean category_scheduled_set_div_id ( gint scheduled_number,
					 int no_div );
static gsb_real category_sub_div_balance ( gint div, gint sub_div );
static gchar *category_sub_div_name ( gint div, gint sub_div );
static gint category_sub_div_nb_transactions ( gint div, gint sub_div );
static gint category_transaction_div_id ( gint transaction_number );
/*END_STATIC*/

/*START_EXTERN*/
extern gint no_devise_totaux_categ;
extern gsb_real null_real;
/*END_EXTERN*/

static MetatreeInterface _category_interface = {
    2,
    1,
    category_tree_currency,
    N_("Category"),
    N_("category"),
    N_("sub-category"),
    N_("No category"),
    N_("No sub-category"),
    category_get_without_div_pointer,
    category_get_div_pointer_from_name,
    gsb_data_category_get_sub_category_number_by_name,
    gsb_data_category_get_nb_transactions,
    category_sub_div_nb_transactions,
    category_div_name,
    category_sub_div_name,
    gsb_data_category_get_balance,
    category_sub_div_balance,
    gsb_data_category_get_no_category,
    gsb_data_category_get_no_sub_category,
    gsb_data_category_get_categories_list,
    gsb_data_category_get_sub_category_list,
    gsb_data_category_get_type,

    category_transaction_div_id,
    gsb_data_transaction_get_sub_category_number,
    gsb_data_transaction_set_category_number,
    gsb_data_transaction_set_sub_category_number,
    gsb_data_scheduled_get_category_number,
    gsb_data_scheduled_get_sub_category_number,
    category_scheduled_set_div_id,
    gsb_data_scheduled_set_sub_category_number,

    category_add_div,
    category_add_sub_div,
    gsb_data_category_remove,
    gsb_data_category_sub_category_remove,
    category_add_transaction_to_div,
    gsb_data_category_add_transaction_to_category,
    gsb_data_category_remove_transaction_from_category,
    gsb_data_category_remove_transaction_from_category,
    category_hold_position_set_path,
    category_hold_position_set_expand,
};

MetatreeInterface * category_interface = &_category_interface;




/**
 *
 *
 */
gint category_tree_currency ( )
{
    return no_devise_totaux_categ;
}



/**
 *
 *
 */
gint category_get_without_div_pointer ( )
{
    return 0;
}





/**
 *
 *
 */
gint category_get_div_pointer_from_name ( const gchar * name, gboolean create )
{
    return (gsb_data_category_get_number_by_name ( name, create, 0 ));
}




/**
 *
 *
 */
gint category_sub_div_nb_transactions ( gint div, gint sub_div )
{
    if ( sub_div )
    {
	return gsb_data_category_get_sub_category_nb_transactions ( div,
								    sub_div );
    }
    else if ( div )
    {
	return gsb_data_category_get_nb_direct_transactions (div);
    }
    return 0;
}



/**
 * return the category name
 *
 * \param div category number
 *
 * \retun a string, need to be freed
 */
gchar *category_div_name ( gint div )
{
    return gsb_data_category_get_name ( div,
					0,
					_(category_interface -> no_div_label ));
}



/**
 *
 *
 */
gchar *category_sub_div_name ( gint div, gint sub_div )
{
    return gsb_data_category_get_sub_category_name ( div,
						     sub_div,
						     _(category_interface -> no_sub_div_label  ));
}




/**
 *
 *
 */
gsb_real category_sub_div_balance ( gint div, gint sub_div )
{
    if ( sub_div )
    {
	return gsb_data_category_get_sub_category_balance ( div,
							    sub_div );
    }
    else if ( div )
    {
	return gsb_data_category_get_direct_balance (div);
    }
    return null_real;
}






/**
 * Get category number for transaction.
 *
 * @param transaction	Transaction to get category number from.
 *
 * @return	Transaction category number.  0 if no transaction is
 *		demanded.  -1 if transaction is a transfert or a
 *		split of transaction to avoid transaction being
 *		considered as a "No category" transaction.
 */
gint category_transaction_div_id ( gint transaction_number )
{
    if (transaction_number)
    {
	if ( gsb_data_transaction_get_contra_transaction_number (transaction_number) > 0
	     || 
	     gsb_data_transaction_get_split_of_transaction (transaction_number))
	    return -1;
	else
	    return gsb_data_transaction_get_category_number (transaction_number);
    }
    return 0;
}



/**
 *
 *
 */
gboolean category_scheduled_set_div_id ( gint scheduled_number,
					 int no_div )
{
    if ( scheduled_number )
    {
	gsb_data_scheduled_set_category_number ( scheduled_number,
						 no_div);
	if ( !gsb_data_scheduled_get_category_number (scheduled_number))
	    gsb_data_scheduled_set_account_number_transfer ( scheduled_number,
							     0 );
    }
    /* TODO dOm : add the return instruction.
     * Is not better for this function to return void ? */ 
    return TRUE;
}


/**
 *
 *
 */
gint category_add_div ( void )
{
    gint new_category_number;
    gchar * name;
    int i = 1;

    /** Find a unique name for category */
    name =  my_strdup (_("New category"));

    while ( gsb_data_category_get_number_by_name ( name, FALSE, 0 ))
    {
	g_free (name);
	i++;
	name = g_strdup_printf ( _("New category #%d"), i ); 
    }

    new_category_number = gsb_data_category_get_number_by_name ( name, TRUE, 0 );

    return new_category_number;
}



/**
 *
 *
 */
gint category_add_sub_div ( int div_id )
{
    gint new_sub_category_number;
    gchar * name;
    int i = 1;

    if ( !div_id )
	return 0;

    /** Find a unique name for category */
    name =  my_strdup (_("New sub-category"));

    while ( gsb_data_category_get_sub_category_number_by_name ( div_id, name, FALSE ))
    {
	g_free (name);
	i++;
	name = g_strdup_printf ( _("New sub-category #%d"), i ); 
    }

    new_sub_category_number = gsb_data_category_get_sub_category_number_by_name ( div_id, name, TRUE );

    return new_sub_category_number;
}






/**
 *
 *
 */
gboolean category_add_transaction_to_div ( gint transaction_number, 
					   int div_id )
{
    gsb_data_category_add_transaction_to_category ( transaction_number,
						    div_id, 0 );
    return TRUE;
}





