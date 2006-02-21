/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2004-2006 Benjamin Drieu (bdrieu@april.org)	      */
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
#include "meta_categories.h"
#include "gsb_data_category.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_transaction.h"
#include "categories_onglet.h"
#include "utils_str.h"
#include "include.h"
/*END_INCLUDE*/


/*START_STATIC*/
static gint category_add_div ();
static gint category_add_sub_div ( int div_id );
static gboolean category_add_transaction_to_div ( gpointer  trans, 
					   int div_id );
static gboolean category_add_transaction_to_sub_div ( gpointer  trans, 
					       int div_id, int sub_div_id );
static gsb_real category_div_balance ( gpointer div );
static gchar * category_div_name ( gpointer div );
static gint category_div_nb_transactions ( gpointer div );
static GSList * category_div_sub_div_list ( gpointer div );
static gint category_div_type ( gpointer div );
static gpointer category_get_div_pointer_from_name ( gchar * name, gboolean create );
static gpointer category_get_sub_div_pointer ( int div_id, int sub_div_id );
static gpointer category_get_sub_div_pointer_from_name ( int div_id, gchar * name, gboolean create );
static gpointer category_get_without_div_pointer ( );
static gboolean category_remove_transaction_from_div ( gpointer  trans, 
						int div_id );
static gboolean category_remove_transaction_from_sub_div ( gpointer  trans, 
						    int div_id, int sub_div_id );
static gint category_scheduled_div_id ( gint scheduled_number );
static void category_scheduled_set_div_id ( gint scheduled_number,
				     int no_div );
static void category_scheduled_set_sub_div_id ( gint scheduled_number,
					 int no_sub_div );
static gint category_scheduled_sub_div_id ( gint scheduled_number);
static gsb_real category_sub_div_balance ( gpointer div, gpointer sub_div );
static gchar * category_sub_div_name ( gpointer sub_div );
static gint category_sub_div_nb_transactions ( gpointer div, gpointer sub_div );
static gint category_transaction_div_id ( gpointer  transaction );
static void category_transaction_set_div_id ( gpointer  transaction, 
				       int no_div );
static void category_transaction_set_sub_div_id ( gpointer  transaction, 
					   int no_sub_div );
static gint category_transaction_sub_div_id ( gpointer  transaction );
/*END_STATIC*/

/*START_EXTERN*/
extern int no_devise_totaux_categ;
extern gsb_real null_real ;
/*END_EXTERN*/

static MetatreeInterface _category_interface = {
    2,
    category_tree_currency,
    N_("Category"),
    N_("No category"),
    N_("No sub-category"),
    category_get_without_div_pointer,
    gsb_data_category_get_structure,
    category_get_sub_div_pointer,
    category_get_div_pointer_from_name,
    category_get_sub_div_pointer_from_name,
    category_div_nb_transactions,
    category_sub_div_nb_transactions,
    category_div_name,
    category_sub_div_name,
    category_div_balance,
    category_sub_div_balance,
    gsb_data_category_get_no_category,
    gsb_data_category_get_no_sub_category,
    gsb_data_category_get_categories_list,
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
    gsb_data_category_remove,
    gsb_data_category_sub_category_remove,
    category_add_transaction_to_div,
    category_add_transaction_to_sub_div,
    category_remove_transaction_from_div,
    category_remove_transaction_from_sub_div,
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
gpointer category_get_without_div_pointer ( )
{
    return NULL;
}




/**
 *
 *
 */
gpointer category_get_sub_div_pointer ( int div_id, int sub_div_id )
{
    return gsb_data_category_get_sub_category_structure ( div_id, sub_div_id );
}



/**
 *
 *
 */
gpointer category_get_div_pointer_from_name ( gchar * name, gboolean create )
{
    return gsb_data_category_get_structure (gsb_data_category_get_number_by_name ( name, create, 0 ));
}



/**
 *
 *
 */
gpointer category_get_sub_div_pointer_from_name ( int div_id, gchar * name, gboolean create )
{
    return gsb_data_category_get_sub_category_structure ( div_id,
							  gsb_data_category_get_sub_category_number_by_name( div_id, name, create));
}




/**
 *
 *
 */
gint category_div_nb_transactions ( gpointer div )
{
    gint category_number;

    category_number = gsb_data_category_get_no_category (div);

    return gsb_data_category_get_nb_transactions ( category_number );
}



/**
 *
 *
 */
gint category_sub_div_nb_transactions ( gpointer div, gpointer sub_div )
{
    gint category_number;
    gint sub_category_number;

    category_number = gsb_data_category_get_no_category (div);
    sub_category_number = gsb_data_category_get_no_sub_category (sub_div);

    if ( sub_div )
    {
	return gsb_data_category_get_sub_category_nb_transactions ( category_number,
								    sub_category_number );
    }
    else if ( div )
    {
	return gsb_data_category_get_nb_direct_transactions (category_number);
    }
    return 0;
}



/**
 *
 *
 */
gchar * category_div_name ( gpointer div )
{
    gint category_number;

    category_number = gsb_data_category_get_no_category (div);

    return gsb_data_category_get_name ( category_number,
					0,
					"" );
}



/**
 *
 *
 */
gchar * category_sub_div_name ( gpointer sub_div )
{
    gint category_number;
    gint sub_category_number;

    if ( ! sub_div )
	return NULL;

    category_number = gsb_data_category_get_no_category_from_sub_category (sub_div);
    sub_category_number = gsb_data_category_get_no_sub_category (sub_div);

    return gsb_data_category_get_sub_category_name ( category_number,
						     sub_category_number,
						     "" );
}



/**
 *
 *
 */
gsb_real category_div_balance ( gpointer div )
{
    gint category_number;

    category_number = gsb_data_category_get_no_category (div);

    return gsb_data_category_get_balance ( category_number );
}



/**
 *
 *
 */
gsb_real category_sub_div_balance ( gpointer div, gpointer sub_div )
{
    gint category_number;
    gint sub_category_number;

    category_number = gsb_data_category_get_no_category (div);
    sub_category_number = gsb_data_category_get_no_sub_category (sub_div);

    if ( sub_div )
    {
	return gsb_data_category_get_sub_category_balance ( category_number,
							    sub_category_number );
    }
    else if ( div )
    {
	return gsb_data_category_get_direct_balance ( category_number );
    }
    return null_real;
}





/**
 *
 *
 */
GSList * category_div_sub_div_list ( gpointer div )
{
    gint category_number;

    category_number = gsb_data_category_get_no_category (div);

    return gsb_data_category_get_sub_category_list (category_number);
}



/**
 *
 *
 */
gint category_div_type ( gpointer div )
{
    gint category_number;

    category_number = gsb_data_category_get_no_category (div);

    return gsb_data_category_get_type (category_number);
}



/**
 * Get category number for transaction.
 *
 * @param transaction	Transaction to get category number from.
 *
 * @return	Transaction category number.  0 if no transaction is
 *		demanded.  -1 if transaction is a transfert or a
 *		breakdown of transaction to avoid transaction being
 *		considered as a "No category" transaction.
 */
gint category_transaction_div_id ( gpointer  transaction )
{
    if ( transaction )
    {
	if ( gsb_data_transaction_get_transaction_number_transfer ( gsb_data_transaction_get_transaction_number (transaction ))
	     || 
	     gsb_data_transaction_get_breakdown_of_transaction ( gsb_data_transaction_get_transaction_number (transaction )))
	    return -1;
	else
	    return gsb_data_transaction_get_category_number ( gsb_data_transaction_get_transaction_number (transaction ));
    }
    return 0;
}



/**
 *
 *
 */
gint category_transaction_sub_div_id ( gpointer  transaction )
{
    if ( transaction )
	return gsb_data_transaction_get_sub_category_number ( gsb_data_transaction_get_transaction_number (transaction ));
    return 0;
}



/**
 *
 *
 */
void category_transaction_set_div_id ( gpointer  transaction, 
				       int no_div )
{
    if ( transaction )
	gsb_data_transaction_set_category_number ( gsb_data_transaction_get_transaction_number (transaction ),
						   no_div );
}



/**
 *
 *
 */
void category_transaction_set_sub_div_id ( gpointer  transaction, 
					   int no_sub_div )
{
    if ( transaction )
	gsb_data_transaction_set_sub_category_number ( gsb_data_transaction_get_transaction_number (transaction ),
						       no_sub_div );
}



/**
 *
 * \return -1 if no type is supported in backend model, type otherwise.
 */
gint category_scheduled_div_id ( gint scheduled_number )
{
    if ( scheduled_number )
	return gsb_data_scheduled_get_category_number (scheduled_number);
    return 0;
}



/**
 *
 *
 */
gint category_scheduled_sub_div_id ( gint scheduled_number)
{
    if ( scheduled_number )
	return gsb_data_scheduled_get_sub_category_number (scheduled_number);
    return 0;
}



/**
 *
 *
 */
void category_scheduled_set_div_id ( gint scheduled_number,
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
}



/**
 *
 *
 */
void category_scheduled_set_sub_div_id ( gint scheduled_number,
					 int no_sub_div )
{
    if ( scheduled_number )
	gsb_data_scheduled_set_sub_category_number ( scheduled_number,
						     no_sub_div);
}



/**
 *
 *
 */
gint category_add_div ()
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
    mise_a_jour_combofix_categ();

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
	return -1;

    /** Find a unique name for category */
    name =  my_strdup (_("New sub-category"));

    while ( gsb_data_category_get_sub_category_number_by_name ( div_id, name, FALSE ))
    {
	g_free (name);
	i++;
	name = g_strdup_printf ( _("New sub-category #%d"), i ); 
    }

    new_sub_category_number = gsb_data_category_get_sub_category_number_by_name ( div_id, name, TRUE );

    mise_a_jour_combofix_categ();

    return new_sub_category_number;
}






/**
 *
 *
 */
gboolean category_add_transaction_to_div ( gpointer  trans, 
					   int div_id )
{
    gsb_data_category_add_transaction_to_category ( gsb_data_transaction_get_transaction_number (trans),
						    div_id, 0 );
    return TRUE;
}



/**
 *
 *
 */
gboolean category_add_transaction_to_sub_div ( gpointer  trans, 
					       int div_id, int sub_div_id )
{
    gsb_data_category_add_transaction_to_category ( gsb_data_transaction_get_transaction_number (trans),
						    div_id, sub_div_id );
    return TRUE;
}



/**
 *
 *
 */
gboolean category_remove_transaction_from_div ( gpointer  trans, 
						int div_id )
{
    gsb_data_category_remove_transaction_from_category ( gsb_data_transaction_get_transaction_number (trans));
    return TRUE;
}



/**
 *
 *
 */
gboolean category_remove_transaction_from_sub_div ( gpointer  trans, 
						    int div_id, int sub_div_id )
{
    gsb_data_category_remove_transaction_from_category ( gsb_data_transaction_get_transaction_number (trans));
    return TRUE;
}
