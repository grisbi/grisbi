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
gboolean category_remove_div (int);
gboolean category_remove_sub_div (int, int);
gboolean category_add_transaction_to_div (struct structure_operation *, int);
gboolean category_add_transaction_to_sub_div (struct structure_operation *, int, int);
gboolean category_remove_transaction_from_div (struct structure_operation *, int);
gboolean category_remove_transaction_from_sub_div (struct structure_operation *, int, int);
/*END_STATIC*/


static MetatreeInterface _category_interface = {
  N_("No category"),
  N_("No sub-category"),
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
  category_transaction_div_id,
  category_transaction_sub_div_id,
  category_transaction_set_div_id,
  category_transaction_set_sub_div_id,
  category_remove_div,
  category_remove_sub_div,
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
void category_transaction_set_div_id ( struct structure_operation * transaction, int no_div )
{
  if ( transaction )
    transaction -> categorie = no_div;
}



/**
 *
 *
 */
void category_transaction_set_sub_div_id ( struct structure_operation * transaction, int no_sub_div )
{
  if ( transaction )
    transaction -> sous_categorie = no_sub_div;
}



/**
 *
 *
 */
gboolean category_remove_div ( int div_id )
{
}



/**
 *
 *
 */
gboolean category_remove_sub_div ( int div_id, int sub_div_id )
{
}



/**
 *
 *
 */
gboolean category_add_transaction_to_div ( struct structure_operation * trans, int div_id )
{
}



/**
 *
 *
 */
gboolean category_add_transaction_to_sub_div ( struct structure_operation * trans, 
					       int div_id, int sub_div_id )
{
}



/**
 *
 *
 */
gboolean category_remove_transaction_from_div ( struct structure_operation * trans, int div_id )
{
}



/**
 *
 *
 */
gboolean category_remove_transaction_from_sub_div ( struct structure_operation * trans, 
						    int div_id, int sub_div_id )
{
}
