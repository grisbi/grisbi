/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	     2008 Benjamin Drieu (bdrieu@april.org)	      */
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
#include "meta_payee.h"
#include "gsb_data_payee.h"
#include "gsb_data_transaction.h"
#include "utils_str.h"
#include "tiers_onglet.h"
#include "gsb_real.h"
/*END_INCLUDE*/


/*START_STATIC*/
static gint payee_add_div ();
static gint payee_add_sub_div ( int div_id );
static gboolean payee_add_transaction_to_div ( gint transaction_number, 
					int div_id );
static void payee_add_transaction_to_sub_div ( gint transaction_number, 
					int div_id, int sub_div_id );
static gchar *payee_div_name ( gint div );
static GSList * payee_div_sub_div_list ( gint div );
static gint payee_div_type ( gint div );
static gint payee_get_sub_div_pointer_from_name ( int div_id, const gchar * name, 
					   gboolean create );
static gint payee_get_without_div_pointer ( );
static gboolean payee_remove_sub_div ( int div_id, int sub_div_id );
static gint payee_scheduled_div_id ( gint scheduled_number );
static gboolean payee_scheduled_set_div_id ( gint scheduled_number, 
				      int no_div );
static gboolean payee_scheduled_set_sub_div_id ( gint scheduled_number,
					  int no_sub_div );
static gint payee_scheduled_sub_div_id ( gint scheduled_number );
static gsb_real payee_sub_div_balance ( gint div, gint sub_div );
static gint payee_sub_div_id ( gpointer sub_payee );
static gchar * payee_sub_div_name ( gint div, gint sub_div );
static gint payee_sub_div_nb_transactions ( gint div, gint sub_div );
static gboolean payee_transaction_set_sub_div_id ( gint transaction_number, 
					    int no_sub_div );
static gint payee_transaction_sub_div_id ( gint transaction_number );
/*END_STATIC*/

/*START_EXTERN*/
extern gint no_devise_totaux_tiers;
extern gsb_real null_real;
/*END_EXTERN*/


static MetatreeInterface _payee_interface = {
    1,
    0,
    payee_tree_currency,
    N_("Payee"),
    N_("payee"),
    N_("sub-payee"),
    N_("No payee"),
    N_("No sub-payee"),
    payee_get_without_div_pointer,
    gsb_data_payee_get_number_by_name,
    payee_get_sub_div_pointer_from_name,
    gsb_data_payee_get_nb_transactions,
    payee_sub_div_nb_transactions,
    payee_div_name,
    payee_sub_div_name,
    gsb_data_payee_get_balance,
    payee_sub_div_balance,
    gsb_data_payee_get_no_payee,
    payee_sub_div_id,
    gsb_data_payee_get_payees_list,
    payee_div_sub_div_list,
    payee_div_type,

    gsb_data_transaction_get_party_number,
    payee_transaction_sub_div_id,
    gsb_data_transaction_set_party_number,
    payee_transaction_set_sub_div_id,
    payee_scheduled_div_id,
    payee_scheduled_sub_div_id,
    payee_scheduled_set_div_id,
    payee_scheduled_set_sub_div_id,

    payee_add_div,
    payee_add_sub_div,
    gsb_data_payee_remove,
    payee_remove_sub_div,
    payee_add_transaction_to_div,
    payee_add_transaction_to_sub_div,
    gsb_data_payee_remove_transaction_from_payee,
    gsb_data_payee_remove_transaction_from_payee,
    payee_hold_position_set_path,
    payee_hold_position_set_expand,
};

MetatreeInterface * payee_interface = &_payee_interface;


/**
 *
 *
 */
gint payee_tree_currency ( )
{
    return no_devise_totaux_tiers;
}


/**
 *
 *
 */
gint payee_get_without_div_pointer ( )
{
    return 0;
}


/**
 *
 *
 */
gint payee_get_sub_div_pointer_from_name ( int div_id, const gchar * name, 
					   gboolean create )
{
    return 0;
}



/**
 *
 *
 */
gint payee_sub_div_nb_transactions ( gint div, gint sub_div )
{
    return 0;
}



/**
 *
 *
 */
gchar *payee_div_name ( gint div )
{
    const gchar * payee_name = gsb_data_payee_get_name ( div, TRUE );
    if ( ! payee_name )
	payee_name = _(payee_interface -> no_div_label);

    return my_strdup ( payee_name );
}



/**
 *
 *
 */
gchar * payee_sub_div_name ( gint div, gint sub_div )
{
    return my_strdup ("");
}




/**
 *
 *
 */
gsb_real payee_sub_div_balance ( gint div, gint sub_div )
{
    return null_real;
}





/**
 *
 *
 */
gint payee_sub_div_id ( gpointer sub_payee )
{
    return 0;
}





/**
 *
 *
 */
GSList * payee_div_sub_div_list ( gint div )
{
    return NULL;
}



/**
 *
 * must return -1 because 0 or 1 or 2 are for credit/debit
 */
gint payee_div_type ( gint div )
{
    return -1;
}




/**
 *
 *
 */
gint payee_transaction_sub_div_id ( gint transaction_number )
{
    return 0;
}



/**
 *
 *
 */
gboolean payee_transaction_set_sub_div_id ( gint transaction_number, 
					    int no_sub_div )
{
    return FALSE;
}



/**
 *
 * \return -1 if no type is supported in backend model, type otherwise.
 */
gint payee_scheduled_div_id ( gint scheduled_number )
{
    return -1;
}



/**
 *
 *
 */
gint payee_scheduled_sub_div_id ( gint scheduled_number )
{
    return -1;
}



/**
 *
 *
 */
gboolean payee_scheduled_set_div_id ( gint scheduled_number, 
				      int no_div )
{
    return FALSE;
}



/**
 *
 *
 */
gboolean payee_scheduled_set_sub_div_id ( gint scheduled_number,
					  int no_sub_div )
{
    return FALSE;
}



/**
 * create a new payee with an automatic name
 * called by the button in the payee list
 *
 * \param 
 *
 * \return the new number of the payee
 *
 */
gint payee_add_div ()
{
    gint payee_number;
    gchar *name;
    int i = 1;

    /** Find a unique name for payee */
    name =  my_strdup ( _("New payee") );

    while ( gsb_data_payee_get_number_by_name ( name,
						FALSE ))
    {
	i++;
	g_free (name);
	name = g_strdup_printf ( _("New payee #%d"), i ); 
    }

    payee_number = gsb_data_payee_new (name);

    g_free (name);

    return payee_number;
}



/**
 *
 *
 */
gint payee_add_sub_div ( int div_id )
{
    return 0;
}




/**
 *
 *
 */
gboolean payee_remove_sub_div ( int div_id, int sub_div_id )
{
    return FALSE;
}



/**
 *
 *
 */
gboolean payee_add_transaction_to_div ( gint transaction_number, 
					int div_id )
{
    gsb_data_payee_add_transaction_to_payee (transaction_number);
    return TRUE;
}



/**
 *
 *
 */
void payee_add_transaction_to_sub_div ( gint transaction_number, 
					int div_id, int sub_div_id )
{
    return;
}


