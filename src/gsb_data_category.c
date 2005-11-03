/* ************************************************************************** */
/* work with the struct of categories                                         */
/*                                                                            */
/*                                  data_account                              */
/*                                                                            */
/*     Copyright (C)	2000-2005 Cédric Auger (cedric@grisbi.org)	      */
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

/**
 * \file gsb_category_data.c
 * work with the category structure, no GUI here
 */


#include "include.h"

/*START_INCLUDE*/
#include "gsb_data_category.h"
#include "gsb_data_account.h"
#include "gsb_data_transaction.h"
#include "categories_onglet.h"
#include "traitement_variables.h"
#include "meta_categories.h"
#include "include.h"
/*END_INCLUDE*/

/**
 * \struct 
 * Describe a category
 */
typedef struct
{
    /** @name category content */
    guint category_number;
    gchar *category_name;
   gint category_type;		/**< 0:credit / 1:debit / 2:special (transfert, breakdown...) */

    GSList *sub_category_list;

    /** @name gui category list content (not saved) */
    gint category_nb_transactions;
    gint category_nb_direct_transactions;
    gdouble category_balance;
    gdouble category_direct_balance;
} struct_category;


/**
 * \struct 
 * Describe a sub-category
 */
typedef struct
{
    /** @name sub-category content */
    guint sub_category_number;
    gchar *sub_category_name;

    guint mother_category_number;

    /** @name gui sub-category list content (not saved)*/
    gint sub_category_nb_transactions;
    gdouble sub_category_balance;
} struct_sub_category;



/*START_STATIC*/
static GSList *gsb_data_category_append_sub_category_to_list ( GSList *category_list,
							GSList *sub_category_list );
static gint gsb_data_category_get_pointer_from_name_in_glist ( struct_category *category,
							gchar *name );
static gint gsb_data_category_get_pointer_from_sub_name_in_glist ( struct_sub_category *sub_category,
							    gchar *name );
static gpointer gsb_data_category_get_structure_in_list ( gint no_category,
						   GSList *list );
static gint gsb_data_category_max_number ( void );
static gint gsb_data_category_max_sub_category_number ( gint category_number );
static gint gsb_data_category_new ( gchar *name );
static gint gsb_data_category_new_sub_category ( gint category_number,
					  gchar *name );
static void gsb_data_category_reset_counters ( void );
/*END_STATIC*/

/*START_EXTERN*/
extern     gchar * buffer ;
extern gchar *categories_de_base_credit [] ;
extern gchar *categories_de_base_debit [] ;
/*END_EXTERN*/


/** contains the g_slist of struct_category */
static GSList *category_list;

/** a pointer to the last category used (to increase the speed) */
static struct_category *category_buffer;
static struct_sub_category *sub_category_buffer;

/** a empty category for the list of categories */
static struct_category *empty_category;



/**
 * set the categories global variables to NULL, usually when we init all the global variables
 *
 * \param none
 *
 * \return FALSE
 * */
gboolean gsb_data_category_init_variables ( void )
{
    if ( category_list )
	g_slist_free (category_list);

    if ( empty_category )
	free (empty_category);
    
    category_list = NULL;
    category_buffer = NULL;
    sub_category_buffer = NULL;

    /* create the empty category */

    empty_category = calloc ( 1,
			      sizeof ( struct_category ));
    empty_category -> category_name = _("No category line");

    return FALSE;
}



/**
 * find and return the structure of the category asked
 *
 * \param no_category number of category
 *
 * \return the adr of the struct of the category (NULL if doesn't exit)
 * */
gpointer gsb_data_category_get_structure ( gint no_category )
{
    if (!no_category)
	return empty_category;

    /* before checking all the categories, we check the category_buffer */

    if ( category_buffer
	 &&
	 category_buffer -> category_number == no_category )
	return category_buffer;

    return gsb_data_category_get_structure_in_list ( no_category,
						     category_list );
}



/**
 * find and return the structure of the category in the list given in param
 * don't use at this level the buffer because could be a bug for an imported list
 * so for normal list, use always gsb_data_category_get_structure
 *
 * \param no_category number of category
 * \param list the list of categories struct where we look for
 *
 * \return the adr of the struct of the category (NULL if doesn't exit)
 * */
gpointer gsb_data_category_get_structure_in_list ( gint no_category,
						   GSList *list )
{
    GSList *tmp;

    if (!no_category)
	return NULL;

    tmp = list;

    while ( tmp )
    {
	struct_category *category;

	category = tmp -> data;

	if ( category -> category_number == no_category )
	{
	    category_buffer = category;
	    return category;
	}
	tmp = tmp -> next;
    }
    return NULL;
}



/**
 * find and return the structure of the sub-category asked
 *
 * \param no_category number of category
 * \param no_sub_category the number of the sub-category
 *
 * \return the adr of the struct of the sub-category (NULL if doesn't exit)
 * */
gpointer gsb_data_category_get_sub_category_structure ( gint no_category,
							gint no_sub_category )
{
    GSList *tmp;
    struct_category *category;

    if (!no_category
	||
	!no_sub_category)
	return NULL;

    /* before checking all the categories, we check the buffer */

    if ( sub_category_buffer
	 &&
	 sub_category_buffer -> sub_category_number == no_sub_category )
	return sub_category_buffer;

    category = gsb_data_category_get_structure ( no_category );
    if ( ! category )
	return NULL;

    tmp = category -> sub_category_list;

    while ( tmp )
    {
	struct_sub_category *sub_category;

	sub_category = tmp -> data;

	if ( sub_category -> sub_category_number == no_sub_category )
	{
	    sub_category_buffer = sub_category;
	    return sub_category;
	}
	tmp = tmp -> next;
    }
    return NULL;
}


/**
 * give the g_slist of categories structure
 * usefull when want to check all categories
 *
 * \param none
 *
 * \return the g_slist of categories structure
 * */
GSList *gsb_data_category_get_categories_list ( void )
{
    return category_list;
}


/**
 * return the g_slist of the sub-categories of the category
 *
 * \param no_category the number of the category
 *
 * \return a g_slist of the struct of the sub-categories or NULL if problem
 * */
GSList *gsb_data_category_get_sub_category_list ( gint no_category )
{
    struct_category *category;

    category = gsb_data_category_get_structure ( no_category );

    if (!category)
	return 0;

    return category -> sub_category_list;
}



/**
 * return the number of the categories given in param
 *
 * \param category_ptr a pointer to the struct of the category
 *
 * \return the number of the category, 0 if problem
 * */
gint gsb_data_category_get_no_category ( gpointer category_ptr )
{
    struct_category *category;

    if ( !category_ptr )
	return 0;

    category = category_ptr;
    category_buffer = category;
    return category -> category_number;
}


/**
 * return the number of the sub-category given in param
 *
 * \param sub_category_ptr a pointer to the struct of the sub-category
 *
 * \return the number of the category, 0 if problem
 * */
gint gsb_data_category_get_no_sub_category ( gpointer sub_category_ptr )
{
    struct_sub_category *sub_category;

    if ( !sub_category_ptr )
	return 0;

    sub_category = sub_category_ptr;
    sub_category_buffer = sub_category;
    return sub_category -> sub_category_number;
}


/**
 * return the number of the category of the sub-category given in param
 *
 * \param sub_category_ptr a pointer to the struct of the sub-category
 *
 * \return the number of the category, 0 if problem
 * */
gint gsb_data_category_get_no_category_from_sub_category ( gpointer sub_category_ptr )
{
    struct_sub_category *sub_category;

    if ( !sub_category_ptr )
	return 0;

    sub_category = sub_category_ptr;
    sub_category_buffer = sub_category;
    return sub_category -> mother_category_number;
}



/** find and return the last number of category
 * \param none
 * \return last number of category
 * */
gint gsb_data_category_max_number ( void )
{
    GSList *tmp;
    gint number_tmp = 0;

    tmp = category_list;

    while ( tmp )
    {
	struct_category *category;

	category = tmp -> data;

	if ( category -> category_number > number_tmp )
	    number_tmp = category -> category_number;

	tmp = tmp -> next;
    }
    return number_tmp;
}


/**
 * find and return the last number of the sub-categories
 *
 * \param 
 * 
 * \return last number of the sub-categories
 * */
gint gsb_data_category_max_sub_category_number ( gint category_number )
{
    struct_category *category;
    GSList *tmp;
    gint number_tmp = 0;

    category = gsb_data_category_get_structure ( category_number );

    tmp = category -> sub_category_list;

    while ( tmp )
    {
	struct_sub_category *sub_category;

	sub_category = tmp -> data;

	if ( sub_category -> sub_category_number > number_tmp )
	    number_tmp = sub_category -> sub_category_number;

	tmp = tmp -> next;
    }
    return number_tmp;
}



/**
 * create a new category, give it a number, append it to the list
 * and return the number
 * update combofix and mark file as modified
 *
 * \param name the name of the category (can be freed after, it's a copy) or NULL
 *
 * \return the number of the new category
 * */
gint gsb_data_category_new ( gchar *name )
{
    gint category_number;

    /* create the new category with a new number */

    category_number = gsb_data_category_new_with_number ( gsb_data_category_max_number () + 1 );

    /* append the name if necessary */

    if (name)
	gsb_data_category_set_name ( category_number,
				     name );

    mise_a_jour_combofix_categ ();
    modification_fichier(TRUE);

    return category_number;
}


/**
 * create a new category with a number, append it to the list
 * and return the number
 * 
 *
 * \param number the number we want to give to that category
 *
 * \return the number of the new category
 * */
gint gsb_data_category_new_with_number ( gint number )
{
    struct_category *category;

    category = calloc ( 1,
			sizeof ( struct_category ));
    category -> category_number = number;

    category_list = g_slist_append ( category_list,
				     category );

    category_buffer = category;

    return category -> category_number;
}



/**
 * remove a category
 * set all the categories of transaction which are this one to 0
 * update combofix and mark file as modified
 *
 * \param no_category the category we want to remove
 *
 * \return TRUE ok
 * */
gboolean gsb_data_category_remove ( gint no_category )
{
    struct_category *category;

    category = gsb_data_category_get_structure ( no_category );

    if (!category)
	return FALSE;

    category_list = g_slist_remove ( category_list,
				     category );

    mise_a_jour_combofix_categ ();
    modification_fichier(TRUE);
    return TRUE;
}


/**
 * remove a sub-category from a category
 * set all the categories of transaction which are this one to 0
 * update combofix and mark file as modified
 *
 * \param no_category the category we want to remove
 *
 * \return TRUE ok
 * */
gboolean gsb_data_category_sub_category_remove ( gint no_category,
						 gint no_sub_category )
{
    struct_category *category;
    struct_sub_category *sub_category;

    category = gsb_data_category_get_structure ( no_category );
    sub_category = gsb_data_category_get_sub_category_structure ( no_category,
								  no_sub_category );

    if (!category
	||
	!sub_category)
	return FALSE;

    category -> sub_category_list = g_slist_remove ( category -> sub_category_list,
						     sub_category );

    mise_a_jour_combofix_categ ();
    modification_fichier(TRUE);
    return TRUE;
}


/**
 * create a new sub-category, append it to the list
 * and return the new number
 *
 * \param category_number the number of the mother
 * \param name the name of the sub-category
 *
 * \return the number of the new sub-category or 0 if problem
 * */
gint gsb_data_category_new_sub_category ( gint category_number,
					  gchar *name )
{
    gint sub_category_number;

    sub_category_number = gsb_data_category_new_sub_category_with_number ( gsb_data_category_max_sub_category_number (category_number) + 1,
									   category_number );

    /* append the name if necessary */

    if (name)
	gsb_data_category_set_sub_category_name ( category_number,
						  sub_category_number,
						  name );

    mise_a_jour_combofix_categ ();
    modification_fichier(TRUE);

    return category_number;
}


/**
 * create a new sub-category with a number, append it to the list
 * and return the number
 *
 * \param number the number we want to give to that sub-category
 * \param category_number the number of the mother
 *
 * \return the number of the new sub-category or 0 if problem
 * */
gint gsb_data_category_new_sub_category_with_number ( gint number,
						      gint category_number)
{
    struct_category *category;
    struct_sub_category *sub_category;

    category = gsb_data_category_get_structure ( category_number );

    if (!category)
	return 0;

    sub_category = calloc ( 1,
			    sizeof (struct_sub_category));
    sub_category -> sub_category_number = number;
    sub_category -> mother_category_number = category_number;

    category -> sub_category_list = g_slist_append ( category -> sub_category_list,
						     sub_category );

    return sub_category -> sub_category_number;
}



/**
 * return the number of the category wich has the name in param
 * create it if necessary
 *
 * \param name the name of the category
 * \param create TRUE if we want to create it if it doen't exist
 * \param category_type the type of the category if we create it
 *
 * \return the number of the category or 0 if problem
 * */
gint gsb_data_category_get_number_by_name ( gchar *name,
					    gboolean create,
					    gint category_type )
{
    GSList *list_tmp;
    gint category_number = 0;

    list_tmp = g_slist_find_custom ( category_list,
				     name,
				     (GCompareFunc) gsb_data_category_get_pointer_from_name_in_glist );

    if ( list_tmp )
    {
	struct_category *category;

	category = list_tmp -> data;
	category_number = category -> category_number;
    }
    else
    {
	if (create)
	{
	    category_number = gsb_data_category_new (name);
	    gsb_data_category_set_type ( category_number,
					 category_type );
	}
    }
    return category_number;
}



/**
 * return the number of the sub-category wich has the name in param
 * create it if necessary
 *
 * \param category_number the number of the category
 * \param name the name of the sub-category
 * \param create TRUE if we want to create it if it doen't exist
 *
 * \return the number of the sub-category or 0 if problem
 * */
gint gsb_data_category_get_sub_category_number_by_name ( gint category_number,
							 gchar *name,
							 gboolean create )
{
    GSList *list_tmp;
    struct_category *category;
    gint sub_category_number = 0;

    category = gsb_data_category_get_structure ( category_number );

    if (!category)
	return 0;

    list_tmp = g_slist_find_custom ( category -> sub_category_list,
				     name,
				     (GCompareFunc) gsb_data_category_get_pointer_from_sub_name_in_glist );

    if ( list_tmp )
    {
	struct_sub_category *sub_category;

	sub_category = list_tmp -> data;
	sub_category_number = sub_category -> sub_category_number;
    }
    else
    {
	if (create)
	    sub_category_number = gsb_data_category_new_sub_category ( category_number,
								       name);
    }
    return sub_category_number;
}




/**
 * used with g_slist_find_custom to find a category in the g_list
 * by his name
 *
 * \param category the struct of the current category checked
 * \param name the name we are looking for
 *
 * \return 0 if it's the same name
 * */
gint gsb_data_category_get_pointer_from_name_in_glist ( struct_category *category,
							gchar *name )
{
    return ( g_strcasecmp ( category -> category_name,
			    name ));
}


/**
 * used with g_slist_find_custom to find a sub-category in the g_list
 * by his name
 *
 * \param sub_category the struct of the current sub_category checked
 * \param name the name we are looking for
 *
 * \return 0 if it's the same name
 * */
gint gsb_data_category_get_pointer_from_sub_name_in_glist ( struct_sub_category *sub_category,
							    gchar *name )
{
    if ( ! sub_category -> sub_category_name )
    {
	return 1;
    }
    return ( g_strcasecmp ( sub_category -> sub_category_name,
			    name ));
}


/**
 * return the name of the category
 * and the full name (ie category : sub-category if no_sub_category is given)
 *
 * \param no_category the number of the category
 * \param no_sub_category if we want the full name of the category
 * \param return_value_error if problem, the value we return
 *
 * \return the name of the category, category : sub-category or NULL/No category if problem
 * */
gchar *gsb_data_category_get_name ( gint no_category,
				    gint no_sub_category,
				    gchar *return_value_error )
{
    struct_category *category;
    gchar *return_value;

    category = gsb_data_category_get_structure ( no_category );

    if (!category)
	return return_value_error;

    return_value = category -> category_name;

    if ( no_sub_category )
    {
	struct_sub_category *sub_category;

	sub_category = gsb_data_category_get_sub_category_structure ( no_category,
								      no_sub_category );

	if (sub_category)
	    return_value = g_strconcat ( return_value,
					 " : ",
					 sub_category -> sub_category_name,
					 NULL );
    }
    return return_value;
}


/**
 * set the name of the category
 * the value is dupplicate in memory
 *
 * \param no_category the number of the category
 * \param name the name of the category
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_category_set_name ( gint no_category,
				      const gchar *name )
{
    struct_category *category;

    category = gsb_data_category_get_structure ( no_category );

    if (!category)
	return FALSE;

    /* we free the last name */

    if ( category -> category_name )
	free (category -> category_name);

    /* and copy the new one */
    if ( name )
	category -> category_name = g_strdup (name);
    else
	category -> category_name = NULL;
    return TRUE;
}

/**
 * return the name of the sub-category
 *
 * \param no_category the number of the category
 * \param no_sub_category the number of the sub-category
 * \param return_value_error if problem, return that value
 *
 * \return the name of the category or NULL/No sub-category if problem
 * */
gchar *gsb_data_category_get_sub_category_name ( gint no_category,
						 gint no_sub_category,
						 gchar *return_value_error )
{
    struct_sub_category *sub_category;

    sub_category = gsb_data_category_get_sub_category_structure ( no_category,
								  no_sub_category );

    if (!sub_category)
	return (return_value_error);

    return sub_category -> sub_category_name;
}


/**
 * set the name of the sub-category
 * the value is dupplicate in memory
 *
 * \param no_category the number of the category
 * \param no_sub_category the number of the sub-category
 * \param name the name of the sub-category
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_category_set_sub_category_name ( gint no_category,
						   gint no_sub_category,
						   const gchar *name )
{
    struct_sub_category *sub_category;

    sub_category = gsb_data_category_get_sub_category_structure ( no_category,
								  no_sub_category );

    if (!sub_category)
	return FALSE;

    /* we free the last name */

    if ( sub_category -> sub_category_name )
	free (sub_category -> sub_category_name);

    /* and copy the new one */
    if ( name )
	sub_category -> sub_category_name = g_strdup (name);
    else
	sub_category -> sub_category_name = NULL;

    return TRUE;
}


/**
 * return a g_slist of g_slist of names of the categories
 *
 * \param set_debit TRUE if we want to have the debits
 * \param set_credit TRUE if we want to have the credits
 * \param set_special TRUE if we want to have the specials
 * \param set_breakdown TRUE if we want to add the breakdown to the specials category
 *
 * \return a g_slist of g_slist of gchar *
 * */
GSList *gsb_data_category_get_name_list ( gboolean set_debit,
					  gboolean set_credit,
					  gboolean set_special,
					  gboolean set_breakdown )
{
    GSList *return_list;
    GSList *tmp_list;
    GSList *debit_list = NULL;
    GSList *credit_list = NULL;

    return_list = NULL;

    /* fill debit_list and/or credit_list and them sub-categories */

    tmp_list = category_list;

    while ( tmp_list )
    {
	struct_category *category;

	category = tmp_list -> data;

	if ( category -> category_type )
	{
	    if ( set_debit )
	    {
		debit_list = g_slist_append ( debit_list,
					      category -> category_name);
		debit_list = gsb_data_category_append_sub_category_to_list ( debit_list,
									     category -> sub_category_list);
	    }
	}
	else
	{
	    if ( set_credit )
	    {
		credit_list = g_slist_append ( credit_list,
					       category -> category_name);
		credit_list = gsb_data_category_append_sub_category_to_list ( credit_list,
									      category -> sub_category_list);
	    }
	}
	tmp_list = tmp_list -> next;
    }

    /* append what we need to return_list */

    if ( set_debit )
	return_list = g_slist_append ( return_list,
				       debit_list );
    if ( set_credit )
	return_list = g_slist_append ( return_list,
				       credit_list );

    /* append the specials list if needed */

    if (set_special)
    {
	GSList *special_list = NULL;

	if (set_breakdown)
	    special_list = g_slist_append ( special_list,
					    _("Breakdown of transaction"));
	special_list = g_slist_append ( special_list,
					_("Transfer"));

	/* append the accounts name with a tab at the begining */

	tmp_list= gsb_data_account_get_list_accounts ();

	while ( tmp_list )
	{
	    gint i;

	    i = gsb_data_account_get_no_account ( tmp_list -> data );
	    if ( ! gsb_data_account_get_closed_account (i) )
		special_list = g_slist_append ( special_list,
						g_strconcat ( "\t",
							      gsb_data_account_get_name (i),
							      NULL ));
	    tmp_list = tmp_list -> next;
	}
	return_list = g_slist_append ( return_list,
				       special_list );
    }
    return return_list;
}



/**
 * append the sub-categories name with a tab at the begining
 * to the list of categories given in param
 *
 * \param category_list a g_slist of categories names
 * \param sub_category_list a g_slist which contains the sub categories to append
 *
 * \return the new category_list (normally shouldn't changed
 * */
GSList *gsb_data_category_append_sub_category_to_list ( GSList *category_list,
							GSList *sub_category_list )
{
    GSList *tmp_list;

    if (!sub_category_list)
	return category_list;

    tmp_list = sub_category_list;

    while (tmp_list)
    {
	struct_sub_category *sub_category;

	sub_category = tmp_list -> data;

	category_list = g_slist_append ( category_list,
					 g_strconcat ( "\t",
						       sub_category -> sub_category_name,
						       NULL ));
	tmp_list = tmp_list -> next;
    }
    return category_list;
}




/**
 * return the type of the category
 * 0:credit / 1:debit / 2:special (transfert, breakdown...)
 *
 * \param no_category the number of the category
 * \param can_return_null if problem, return NULL if TRUE or "No category" if FALSE
 *
 * \return the name of the category or NULL/No category if problem
 * */
gint gsb_data_category_get_type ( gint no_category )
{
    struct_category *category;

    category = gsb_data_category_get_structure ( no_category );

    if (!category)
	return 0;

    return category -> category_type;
}


/**
 * set the type of the category
 * 0:credit / 1:debit / 2:special (transfert, breakdown...)
 *
 * \param no_category the number of the category
 * \param name the name of the category
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_category_set_type ( gint no_category,
				      gint category_type )
{
    struct_category *category;

    category = gsb_data_category_get_structure ( no_category );

    if (!category)
	return FALSE;

    category -> category_type = category_type;
    return TRUE;
}


/**
 * return nb_transactions of the category
 *
 * \param no_category the number of the category
 *
 * \return nb_transactions of the category or 0 if problem
 * */
gint gsb_data_category_get_nb_transactions ( gint no_category )
{
    struct_category *category;

    category = gsb_data_category_get_structure ( no_category );

    if (!category)
	return 0;

    return category -> category_nb_transactions;
}



/**
 * return nb_transactions of the sub-category
 *
 * \param no_category the number of the category
 * \param no_sub_category the number of the sub-category
 *
 * \return nb_transactions of the sub-category or 0 if problem
 * */
gint gsb_data_category_get_sub_category_nb_transactions ( gint no_category,
							  gint no_sub_category )
{
    struct_sub_category *sub_category;

    sub_category = gsb_data_category_get_sub_category_structure ( no_category,
								  no_sub_category );

    if (!sub_category)
	return 0;

    return sub_category -> sub_category_nb_transactions;
}



/**
 * return nb_direct_transactions of the category
 *
 * \param no_category the number of the category
 *
 * \return nb_direct_transactions of the category or 0 if problem
 * */
gint gsb_data_category_get_nb_direct_transactions ( gint no_category )
{
    struct_category *category;

    category = gsb_data_category_get_structure ( no_category );

    if (!category)
	return 0;

    return category -> category_nb_direct_transactions;
}




/**
 * return balance of the category
 *
 * \param no_category the number of the category
 *
 * \return balance of the category or 0 if problem
 * */
gdouble gsb_data_category_get_balance ( gint no_category )
{
    struct_category *category;

    category = gsb_data_category_get_structure ( no_category );

    if (!category)
	return 0;

    return category -> category_balance;
}


/**
 * return balance of the sub-category
 *
 * \param no_category the number of the category
 * \param no_sub_category the number of the sub-category
 *
 * \return balance of the sub-category or 0 if problem
 * */
gdouble gsb_data_category_get_sub_category_balance ( gint no_category,
						     gint no_sub_category )
{
    struct_sub_category *sub_category;

    sub_category = gsb_data_category_get_sub_category_structure ( no_category,
								  no_sub_category );

    if (!sub_category)
	return 0;

    return sub_category -> sub_category_balance;
}

/**
 * return direct_balance of the category
 *
 * \param no_category the number of the category
 *
 * \return balance of the category or 0 if problem
 * */
gdouble gsb_data_category_get_direct_balance ( gint no_category )
{
    struct_category *category;

    category = gsb_data_category_get_structure ( no_category );

    if (!category)
	return 0;

    return category -> category_direct_balance;
}



/**
 * reset the counters of the categories and sub-categories
 *
 * \param 
 *
 * \return 
 * */
void gsb_data_category_reset_counters ( void )
{
    GSList *list_tmp;

    list_tmp = category_list;

    while ( list_tmp )
    {
	struct_category *category;
	GSList *sub_list_tmp;

	category = list_tmp -> data;
	category -> category_balance = 0.0;
	category -> category_nb_transactions = 0;

	sub_list_tmp = category -> sub_category_list;

	while ( sub_list_tmp )
	{
	    struct_sub_category *sub_category;
	    
	    sub_category = sub_list_tmp -> data;

	    sub_category -> sub_category_nb_transactions = 0;
	    sub_category -> sub_category_balance = 0.0;

	    sub_list_tmp = sub_list_tmp -> next;
	}
	list_tmp = list_tmp -> next;
    }
    
    /* reset the empty category */
    empty_category -> category_balance = 0.0;
    empty_category -> category_nb_transactions = 0;
    empty_category -> category_direct_balance = 0.0;
    empty_category -> category_nb_direct_transactions = 0;
}

/**
 * update the counters of the categories
 *
 * \param
 *
 * \return
 * */
void gsb_data_category_update_counters ( void )
{
    GSList *list_tmp_transactions;

    gsb_data_category_reset_counters ();

    list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

    while ( list_tmp_transactions )
    {
	gint transaction_number_tmp;
	transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	gsb_data_category_add_transaction_to_category ( transaction_number_tmp );

	list_tmp_transactions = list_tmp_transactions -> next;
    }
}


/**
 * add the given transaction to its category in the counters
 * if the transaction has no category, add it to the blank category
 *
 * \param transaction_number the transaction we want to work with
 *
 * \return
 * */
void gsb_data_category_add_transaction_to_category ( gint transaction_number )
{
    struct_category *category;
    struct_sub_category *sub_category;

    category = gsb_data_category_get_structure ( gsb_data_transaction_get_category_number (transaction_number));
    sub_category = gsb_data_category_get_sub_category_structure ( gsb_data_transaction_get_category_number (transaction_number),
								  gsb_data_transaction_get_sub_category_number (transaction_number));

    if ( category )
    {
	category -> category_nb_transactions ++;
	category -> category_balance += gsb_data_transaction_get_adjusted_amount_for_currency ( transaction_number, category_tree_currency () -> no_devise );
    }
    else
    {
	empty_category -> category_nb_transactions ++;
	empty_category -> category_balance += gsb_data_transaction_get_adjusted_amount_for_currency ( transaction_number, category_tree_currency () -> no_devise );
    }

    if ( sub_category )
    {
	sub_category -> sub_category_nb_transactions ++;
	sub_category -> sub_category_balance += gsb_data_transaction_get_adjusted_amount_for_currency ( transaction_number, category_tree_currency () -> no_devise);


gsb_data_transaction_get_adjusted_amount (transaction_number);
    }
    else
    {
	if ( category )
	{
	    category -> category_nb_direct_transactions ++;
	    category -> category_direct_balance += gsb_data_transaction_get_adjusted_amount_for_currency ( transaction_number, category_tree_currency () -> no_devise );
	}
    }
}


/**
 * remove the given transaction to its category in the counters
 * if the transaction has no category, remove it to the blank category
 *
 * \param transaction_number the transaction we want to work with
 *
 * \return
 * */
void gsb_data_category_remove_transaction_from_category ( gint transaction_number )
{
    struct_category *category;
    struct_sub_category *sub_category;

    category = gsb_data_category_get_structure ( gsb_data_transaction_get_category_number (transaction_number));
    sub_category = gsb_data_category_get_sub_category_structure ( gsb_data_transaction_get_category_number (transaction_number),
								  gsb_data_transaction_get_sub_category_number (transaction_number));

    if ( category )
    {
	category -> category_nb_transactions --;
	category -> category_balance -= gsb_data_transaction_get_adjusted_amount_for_currency ( transaction_number, category_tree_currency () -> no_devise );
	if ( !category -> category_nb_transactions ) /* Cope with float errors */
	    category -> category_balance = 0.0;
    }

    if ( sub_category )
    {
	sub_category -> sub_category_nb_transactions --;
	sub_category -> sub_category_balance -= gsb_data_transaction_get_adjusted_amount_for_currency ( transaction_number, category_tree_currency () -> no_devise );
	if ( !sub_category -> sub_category_nb_transactions ) /* Cope with float errors */
	    sub_category -> sub_category_balance = 0.0;
    }
    else
    {
	if ( category )
	{
	    category -> category_nb_direct_transactions --;
	    category -> category_direct_balance -= gsb_data_transaction_get_adjusted_amount_for_currency ( transaction_number, category_tree_currency () -> no_devise );
	}
    }
}



/**
 * create the default list of categories
 * fill category_list with the default categories
 *
 * \param
 *
 * \return
 * */
void gsb_data_category_create_default_category_list ( void )
{
    GSList *list_tmp;
    gint i;
    
    /* FIXME : should ask here for different kind of categories,
     * or leave them blank...
     * */

    /** In fact, we merge the category list with nothing, ending in
     * creating the base categories. */

    list_tmp = NULL;
    i = 0;

    while ( categories_de_base_debit[i] )
    {
	gint categ = gsb_data_category_new ( categories_de_base_debit[i] );
	gsb_data_category_set_type ( categ, 1 );
	list_tmp = g_slist_append ( list_tmp, categories_de_base_credit[i] );
	i++;
    }
    gsb_data_category_merge_category_list (list_tmp);
    free (list_tmp);

    list_tmp = NULL;
    i = 0;

    while ( categories_de_base_credit[i] )
    {
	gint categ = gsb_data_category_new ( categories_de_base_debit[i] );
	gsb_data_category_set_type ( categ, 1 );
	list_tmp = g_slist_append ( list_tmp, categories_de_base_credit[i] );
	i++;
    }
    gsb_data_category_merge_category_list (list_tmp);
    free (list_tmp);
}



/**
 * merge the given category list with the current category list
 *
 * \param list_to_merge
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_category_merge_category_list ( GSList *list_to_merge )
{
    GSList *list_tmp;

    list_tmp = list_to_merge;

    while ( list_tmp )
    {
	gint category_number;
	struct_category *new_category;

	new_category = list_tmp -> data;

	/* we try to find the new category in the currents categories
	 * if don't, it creates it */

	category_number = gsb_data_category_get_number_by_name ( new_category -> category_name,
								 TRUE,
								 new_category -> category_type );

	/* we check category_number but normally it will always != 0 */

	if ( category_number )
	{
	    GSList *sub_list_tmp;

	    sub_list_tmp = new_category -> sub_category_list;

	    while ( sub_list_tmp )
	    {
		struct_sub_category *new_sub_category;

		new_sub_category = sub_list_tmp -> data;

		gsb_data_category_get_sub_category_number_by_name ( category_number,
								    new_sub_category -> sub_category_name,
								    TRUE );
		sub_list_tmp = sub_list_tmp -> next;
	    }
	    list_tmp = list_tmp -> next;
	}
    }
    return TRUE;
}


