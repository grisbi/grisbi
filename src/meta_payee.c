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
#include "meta_payee.h"
#include "utils_tiers.h"
#include "tiers_onglet.h"
#include "utils_devises.h"
#include "traitement_variables.h"
/*END_INCLUDE*/


/*START_STATIC*/
static gint payee_add_div ();
static gint payee_add_sub_div ( int div_id );
static gboolean payee_add_transaction_to_div ( struct structure_operation * trans, 
					int div_id );
static gboolean payee_add_transaction_to_sub_div ( struct structure_operation * trans, 
					    int div_id, int sub_div_id );
static gdouble payee_div_balance ( gpointer div );
static gint payee_div_id ( gpointer payee );
static GSList * payee_div_list ( );
static gchar * payee_div_name ( gpointer div );
static gint payee_div_nb_transactions ( gpointer div );
static GSList * payee_div_sub_div_list ( gpointer div );
static gint payee_div_type ( gpointer div );
static gpointer payee_get_div_pointer ( int div_id );
static gpointer payee_get_div_pointer_from_name ( gchar * name, gboolean create );
static gpointer payee_get_sub_div_pointer ( int div_id, int sub_div_id );
static gpointer payee_get_sub_div_pointer_from_name ( int div_id, gchar * name, 
					       gboolean create );
static gpointer payee_get_without_div_pointer ( );
static gboolean payee_remove_div ( int div_id );
static gboolean payee_remove_sub_div ( int div_id, int sub_div_id );
static gboolean payee_remove_transaction_from_div ( struct structure_operation * trans, 
					     int div_id );
static gboolean payee_remove_transaction_from_sub_div ( struct structure_operation * trans, 
						 int div_id, int sub_div_id );
static gint payee_scheduled_div_id ( struct operation_echeance * scheduled );
static void payee_scheduled_set_div_id ( struct operation_echeance * scheduled, 
				  int no_div );
static void payee_scheduled_set_sub_div_id ( struct operation_echeance * scheduled, 
				      int no_sub_div );
static gint payee_scheduled_sub_div_id ( struct operation_echeance * scheduled );
static gdouble payee_sub_div_balance ( gpointer div, gpointer sub_div );
static gint payee_sub_div_id ( gpointer sub_payee );
static gchar * payee_sub_div_name ( gpointer sub_div );
static gint payee_sub_div_nb_transactions ( gpointer div, gpointer sub_div );
static gint payee_transaction_div_id ( struct structure_operation * transaction );
static void payee_transaction_set_div_id ( struct structure_operation * transaction, 
				    int no_div );
static void payee_transaction_set_sub_div_id ( struct structure_operation * transaction, 
					int no_sub_div );
static gint payee_transaction_sub_div_id ( struct structure_operation * transaction );
static struct struct_devise * payee_tree_currency ( );
/*END_STATIC*/

/*START_EXTERN*/
extern GSList *liste_struct_tiers;
extern gint mise_a_jour_combofix_tiers_necessaire;
extern GtkTreeStore *model;
extern gint nb_enregistrements_tiers;
extern gint no_devise_totaux_tiers;
/*END_EXTERN*/


static MetatreeInterface _payee_interface = {
    1,
    payee_tree_currency,
    N_("No payee"),
    N_("No sub-payee"),
    payee_get_without_div_pointer,
    payee_get_div_pointer,
    payee_get_sub_div_pointer,
    payee_get_div_pointer_from_name,
    payee_get_sub_div_pointer_from_name,
    payee_div_nb_transactions,
    payee_sub_div_nb_transactions,
    payee_div_name,
    payee_sub_div_name,
    payee_div_balance,
    payee_sub_div_balance,
    payee_div_id,
    payee_sub_div_id,
    payee_div_list,
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
    payee_remove_div,
    payee_remove_sub_div,
    payee_add_transaction_to_div,
    payee_add_transaction_to_sub_div,
    payee_remove_transaction_from_div,
    payee_remove_transaction_from_sub_div,
};

MetatreeInterface * payee_interface = &_payee_interface;
struct struct_tiers * without_payee;




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
gpointer payee_get_without_div_pointer ( )
{
    return (gpointer) without_payee;
}



/**
 *
 *
 */
gpointer payee_get_div_pointer ( int div_id )
{
    return (gpointer) tiers_par_no ( div_id );
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
    return tiers_par_nom ( name, 0 );
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
    return ((struct struct_tiers *) div) -> nb_transactions;
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
gchar * payee_div_name ( gpointer div )
{
    return ((struct struct_tiers *) div) -> nom_tiers;
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
    return ((struct struct_tiers *) div) -> balance;
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
gint payee_div_id ( gpointer payee )
{
    if ( payee )
	return ((struct struct_tiers *) payee) -> no_tiers;
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
GSList * payee_div_list ( )
{
    return liste_struct_tiers;
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
gint payee_transaction_div_id ( struct structure_operation * transaction )
{
    return transaction -> tiers;
}



/**
 *
 *
 */
gint payee_transaction_sub_div_id ( struct structure_operation * transaction )
{
    return -1;
}



/**
 *
 *
 */
void payee_transaction_set_div_id ( struct structure_operation * transaction, 
				    int no_div )
{
    transaction -> tiers = no_div;
}



/**
 *
 *
 */
void payee_transaction_set_sub_div_id ( struct structure_operation * transaction, 
					int no_sub_div )
{

}



/**
 *
 * \return -1 if no type is supported in backend model, type otherwise.
 */
gint payee_scheduled_div_id ( struct operation_echeance * scheduled )
{
    return -1;
}



/**
 *
 *
 */
gint payee_scheduled_sub_div_id ( struct operation_echeance * scheduled )
{
    return -1;
}



/**
 *
 *
 */
void payee_scheduled_set_div_id ( struct operation_echeance * scheduled, 
				  int no_div )
{

}



/**
 *
 *
 */
void payee_scheduled_set_sub_div_id ( struct operation_echeance * scheduled, 
				      int no_sub_div )
{

}



/**
 *
 *
 */
gint payee_add_div ()
{
    struct struct_tiers * new_payee;
    gchar * name;
    int i = 1;

    /** Find a unique name for payee */
    name =  g_strdup ( _("New payee") );

    while ( tiers_par_nom ( name, 0 ) )
    {
	i++;
	name = g_strdup_printf ( _("New payee #%d"), i ); 
    }

    new_payee = ajoute_nouveau_tiers ( name );

    return new_payee -> no_tiers;
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
gboolean payee_remove_div ( int div_id )
{
    liste_struct_tiers = g_slist_remove ( liste_struct_tiers,
					   payee_get_div_pointer ( div_id ) );

    /** Then, update various counters and stuff. */
    nb_enregistrements_tiers--;
    mise_a_jour_combofix_tiers_necessaire = 1;
    mise_a_jour_combofix_tiers ();
    modification_fichier(TRUE);

    return TRUE;
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
gboolean payee_add_transaction_to_div ( struct structure_operation * trans, 
					int div_id )
{
    add_transaction_to_payee ( trans, payee_get_div_pointer ( div_id ) );
    return TRUE;
}



/**
 *
 *
 */
gboolean payee_add_transaction_to_sub_div ( struct structure_operation * trans, 
					    int div_id, int sub_div_id )
{
    return TRUE;
}



/**
 *
 *
 */
gboolean payee_remove_transaction_from_div ( struct structure_operation * trans, 
					     int div_id )
{
    remove_transaction_from_payee ( trans, payee_get_div_pointer (div_id) );
    return TRUE;
}



/**
 *
 *
 */
gboolean payee_remove_transaction_from_sub_div ( struct structure_operation * trans, 
						 int div_id, int sub_div_id )
{
    remove_transaction_from_payee ( trans, payee_get_div_pointer (div_id) );
    return TRUE;
}
