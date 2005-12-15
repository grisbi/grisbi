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
#include "meta_payee.h"
#include "utils_devises.h"
#include "gsb_data_payee.h"
#include "gsb_data_transaction.h"
#include "utils_str.h"
#include "include.h"
#include "structures.h"
/*END_INCLUDE*/


/*START_STATIC*/
static gint payee_add_div ();
static gint payee_add_sub_div ( int div_id );
static gboolean payee_add_transaction_to_div ( gpointer  trans, 
					int div_id );
static gboolean payee_add_transaction_to_sub_div ( gpointer  trans, 
					    int div_id, int sub_div_id );
static gdouble payee_div_balance ( gpointer div );
static gchar *payee_div_name ( gpointer div );
static gint payee_div_nb_transactions ( gpointer div );
static GSList * payee_div_sub_div_list ( gpointer div );
static gint payee_div_type ( gpointer div );
static gpointer payee_get_div_pointer_from_name ( gchar * name, gboolean create );
static gpointer payee_get_sub_div_pointer ( int div_id, int sub_div_id );
static gpointer payee_get_sub_div_pointer_from_name ( int div_id, gchar * name, 
					       gboolean create );
static gboolean payee_remove_sub_div ( int div_id, int sub_div_id );
static gboolean payee_remove_transaction_from_div ( gpointer  trans, 
					     int div_id );
static gboolean payee_remove_transaction_from_sub_div ( gpointer  trans, 
						 int div_id, int sub_div_id );
static gint payee_scheduled_div_id ( gint scheduled_number );
static void payee_scheduled_set_div_id ( gint scheduled_number, 
				  int no_div );
static void payee_scheduled_set_sub_div_id ( gint scheduled_number,
				      int no_sub_div );
static gint payee_scheduled_sub_div_id ( gint scheduled_number );
static gdouble payee_sub_div_balance ( gpointer div, gpointer sub_div );
static gint payee_sub_div_id ( gpointer sub_payee );
static gchar * payee_sub_div_name ( gpointer sub_div );
static gint payee_sub_div_nb_transactions ( gpointer div, gpointer sub_div );
static gint payee_transaction_div_id ( gpointer  transaction );
static void payee_transaction_set_div_id ( gpointer  transaction, 
				    int no_div );
static void payee_transaction_set_sub_div_id ( gpointer  transaction, 
					int no_sub_div );
static gint payee_transaction_sub_div_id ( gpointer  transaction );
/*END_STATIC*/

/*START_EXTERN*/
extern GtkTreeStore *model;
extern gint no_devise_totaux_tiers;
/*END_EXTERN*/


static MetatreeInterface _payee_interface = {
    1,
    payee_tree_currency,
    N_("Payee"),
    N_("No payee"),
    N_("No sub-payee"),
    gsb_data_payee_get_empty_payee,
    gsb_data_payee_get_structure,
    payee_get_sub_div_pointer,
    payee_get_div_pointer_from_name,
    payee_get_sub_div_pointer_from_name,
    payee_div_nb_transactions,
    payee_sub_div_nb_transactions,
    payee_div_name,
    payee_sub_div_name,
    payee_div_balance,
    payee_sub_div_balance,
    gsb_data_payee_get_no_payee,
    payee_sub_div_id,
    gsb_data_payee_get_payees_list,
    payee_div_sub_div_list,
    payee_div_type,

    payee_transaction_div_id,
    payee_transaction_sub_div_id,
    payee_transaction_set_div_id,
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
    payee_remove_transaction_from_div,
    payee_remove_transaction_from_sub_div,
};

MetatreeInterface * payee_interface = &_payee_interface;


/**
 *
 *
 */
struct struct_devise * payee_tree_currency ( )
{
    return (struct struct_devise *) devise_par_no ( no_devise_totaux_tiers );
}





/**
 *
 *
 */
gpointer payee_get_sub_div_pointer ( int div_id, int sub_div_id )
{
    return NULL;
}



/**
 *
 *
 */
gpointer payee_get_div_pointer_from_name ( gchar * name, gboolean create )
{
    return gsb_data_payee_get_structure ( gsb_data_payee_get_number_by_name ( name,
								    FALSE ));
}



/**
 *
 *
 */
gpointer payee_get_sub_div_pointer_from_name ( int div_id, gchar * name, 
					       gboolean create )
{
    return NULL;
}



/**
 *
 *
 */
gint payee_div_nb_transactions ( gpointer div )
{
    return gsb_data_payee_get_nb_transactions ( gsb_data_payee_get_no_payee (div));
}



/**
 *
 *
 */
gint payee_sub_div_nb_transactions ( gpointer div, gpointer sub_div )
{
    return 0;
}



/**
 *
 *
 */
gchar *payee_div_name ( gpointer div )
{
    /* FIXME : later we should use the list with the number of payee, and not a pointer */

    return gsb_data_payee_get_name( gsb_data_payee_get_no_payee(div),
			       TRUE);
}



/**
 *
 *
 */
gchar * payee_sub_div_name ( gpointer sub_div )
{
    return "";
}



/**
 *
 *
 */
gdouble payee_div_balance ( gpointer div )
{
    return gsb_data_payee_get_balance ( gsb_data_payee_get_no_payee (div));
}



/**
 *
 *
 */
gdouble payee_sub_div_balance ( gpointer div, gpointer sub_div )
{
    return 0;
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
GSList * payee_div_sub_div_list ( gpointer div )
{
    return NULL;
}



/**
 *
 *
 */
gint payee_div_type ( gpointer div )
{
    return -1;
}



/**
 *
 * \return -1 if no type is supported in backend model, type otherwise.
 */
gint payee_transaction_div_id ( gpointer  transaction )
{
    return gsb_data_transaction_get_party_number ( gsb_data_transaction_get_transaction_number (transaction ));
}



/**
 *
 *
 */
gint payee_transaction_sub_div_id ( gpointer  transaction )
{
    return -1;
}



/**
 *
 *
 */
void payee_transaction_set_div_id ( gpointer  transaction, 
				    int no_div )
{
    gsb_data_transaction_set_party_number ( gsb_data_transaction_get_transaction_number (transaction),
					    no_div );
}



/**
 *
 *
 */
void payee_transaction_set_sub_div_id ( gpointer  transaction, 
					int no_sub_div )
{

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
void payee_scheduled_set_div_id ( gint scheduled_number, 
				  int no_div )
{

}



/**
 *
 *
 */
void payee_scheduled_set_sub_div_id ( gint scheduled_number,
				      int no_sub_div )
{

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
    return -1;
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
gboolean payee_add_transaction_to_div ( gpointer  trans, 
					int div_id )
{
    gsb_data_payee_add_transaction_to_payee ( gsb_data_transaction_get_transaction_number (trans));
    return TRUE;
}



/**
 *
 *
 */
gboolean payee_add_transaction_to_sub_div ( gpointer  trans, 
					    int div_id, int sub_div_id )
{
    return TRUE;
}



/**
 *
 *
 */
gboolean payee_remove_transaction_from_div ( gpointer  trans, 
					     int div_id )
{
    gsb_data_payee_remove_transaction_from_payee ( gsb_data_transaction_get_transaction_number (trans));
    return TRUE;
}



/**
 *
 *
 */
gboolean payee_remove_transaction_from_sub_div ( gpointer  trans, 
						 int div_id, int sub_div_id )
{
    gsb_data_payee_remove_transaction_from_payee ( gsb_data_transaction_get_transaction_number (trans));
    return TRUE;
}
