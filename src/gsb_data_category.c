/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2009-2010 Pierre Biava (grisbi@pierre.biava.name)                 */
/*          http://www.grisbi.org                                             */
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
 * \file gsb_data_category.c
 * work with the category structure, no GUI here
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_data_category.h"
#include "meta_categories.h"
#include "gsb_category.h"
#include "gsb_data_account.h"
#include "gsb_data_form.h"
#include "gsb_data_mix.h"
#include "gsb_data_transaction.h"
#include "gsb_form_widget.h"
#include "gsb_real.h"
#include "utils_str.h"
#include "erreur.h"
#include "structures.h"
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
    gint category_type;		/**< 0:credit / 1:debit / 2:special (transfert, split...) */

    GSList *sub_category_list;

    /** @name gui category list content (not saved) */
    gint category_nb_transactions;
    gint category_nb_direct_transactions;
    gsb_real category_balance;
    gsb_real category_direct_balance;
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
    gsb_real sub_category_balance;
} struct_sub_category;



/*START_STATIC*/
static void _gsb_data_category_free ( struct_category *category );
static void _gsb_data_sub_category_free ( struct_sub_category *sub_category );
static GSList *gsb_data_category_append_sub_category_to_list ( GSList *category_list,
							GSList *sub_category_list );
static gint gsb_data_category_get_pointer_from_name_in_glist ( struct_category *category,
							const gchar *name );
static gint gsb_data_category_get_pointer_from_sub_name_in_glist ( struct_sub_category *sub_category,
							    const gchar *name );
static gpointer gsb_data_category_get_structure_in_list ( gint no_category,
                        GSList *list );
static gint gsb_data_category_max_number ( void );
static gint gsb_data_category_max_sub_category_number ( gint category_number );
static gint gsb_data_category_new ( const gchar *name );
static gint gsb_data_category_new_sub_category ( gint category_number,
                        const gchar *name );
static void gsb_data_category_reset_counters ( void );
static gint gsb_data_sub_category_compare ( struct_sub_category * a, struct_sub_category * b );
/*END_STATIC*/

/*START_EXTERN*/
extern gsb_real null_real;
/*END_EXTERN*/


/** contains the g_slist of struct_category */
static GSList *category_list = NULL;

/** a pointer to the last category used (to increase the speed) */
static struct_category *category_buffer;
static struct_sub_category *sub_category_buffer;

/** a empty category for the list of categories
 * the number of the empty category is 0 */
static struct_category *empty_category = NULL;



/**
 * set the categories global variables to NULL, usually when we init all the global variables
 *
 * \param none
 *
 * \return FALSE
 * */
gboolean gsb_data_category_init_variables ( void )
{
    /* free the memory used by the category list */
    if ( category_list )
    {
	    GSList* cat_tmp_list = category_list;
	    while ( cat_tmp_list )
	    {
		struct_category *category;
		category = cat_tmp_list -> data;
		cat_tmp_list = cat_tmp_list -> next;
		_gsb_data_category_free ( category );
	    }
	    g_slist_free (category_list);
    }
    category_list = NULL;

    category_buffer = NULL;
    sub_category_buffer = NULL;

    /* recreate the empty category */
    /* set an empty name for that empty categ, else every transaction
     * without category will have that name */
    _gsb_data_category_free ( empty_category );
    empty_category = g_malloc0 ( sizeof ( struct_category ));
    empty_category -> category_name = g_strdup(_("No category"));

    return FALSE;
}



/**
 * find and return the structure of the category asked
 *
 * \param no_category number of category
 *
 * \return the adr of the struct of the category, empty_category or NULL if not exists
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
 * return the empty_category pointer
 *
 * \param
 *
 * \return a pointer to empty_category */
gpointer gsb_data_category_get_empty_category ( void )
{
    return gsb_data_category_get_structure (0);
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

    /* check empty sub-categ */
    if (!no_sub_category)
	return NULL;

    /* before checking all the categories, we check the buffer */
    if ( sub_category_buffer
	 &&
	 sub_category_buffer -> mother_category_number == no_category
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

    /* if sub_category_ptr is NULL, we are on empty sub-category, the number is 0 */
    if ( !sub_category_ptr )
	return 0;

    sub_category = sub_category_ptr;
    sub_category_buffer = sub_category;
    return sub_category -> sub_category_number;
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
gint gsb_data_category_new ( const gchar *name )
{
    gint category_number;

    /* create the new category with a new number */

    category_number = gsb_data_category_new_with_number ( gsb_data_category_max_number () + 1 );

    /* append the name if necessary */

    if (name)
	gsb_data_category_set_name ( category_number,
				     name );
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

    category = g_malloc0 ( sizeof ( struct_category ));
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

    _gsb_data_category_free (category);

    return TRUE;
}

/**
 * This function is called to free the memory used by a category structure
 */
void _gsb_data_category_free ( struct_category *category )
{
    if ( ! category )
        return;
    /* free memory used by sub categories */
    if ( category -> sub_category_list )
    {
        GSList* sub_tmp_list = category -> sub_category_list;
        while ( sub_tmp_list )
        {
            struct_sub_category *sub_category;
            sub_category = sub_tmp_list -> data;
            sub_tmp_list = sub_tmp_list -> next;
            _gsb_data_sub_category_free ( sub_category );
        }
        g_slist_free ( category -> sub_category_list );
    }
    if ( category -> category_name )
        g_free ( category -> category_name );
    g_free ( category ); 
    if ( category_buffer == category )
	category_buffer = NULL;
}


/**
 * This function is called to free the memory used by a sub category structure
 */
void _gsb_data_sub_category_free ( struct_sub_category *sub_category )
{
    if ( ! sub_category )
        return;
    if ( sub_category -> sub_category_name )
        g_free ( sub_category -> sub_category_name );
    g_free ( sub_category );
    if ( sub_category_buffer == sub_category )
	sub_category_buffer = NULL;
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

    _gsb_data_sub_category_free (sub_category);

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
                        const gchar *name )
{
    gint sub_category_number;

    sub_category_number = gsb_data_category_new_sub_category_with_number ( gsb_data_category_max_sub_category_number (category_number) + 1,
									   category_number );

    /* append the name if necessary */

    if (name)
	gsb_data_category_set_sub_category_name ( category_number,
						  sub_category_number,
						  name );
    return sub_category_number;
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

    sub_category = g_malloc0 ( sizeof (struct_sub_category));
    sub_category -> sub_category_number = number;
    sub_category -> mother_category_number = category_number;

    category -> sub_category_list = g_slist_append ( category -> sub_category_list,
						     sub_category );

    sub_category_buffer = sub_category;

    return sub_category -> sub_category_number;
}


/**
 * get a string like 'category : sub-category', a transaction number
 * and fill the transaction with the category and sub-category
 * if the category or sub-category doesn't exist, create them
 *
 * \param transaction_number
 * \param string
 * \param is_transaction TRUE if it's for a transaction, FALSE for a scheduled transaction
 *
 * \return TRUE ok, FALSE if fail
 * */
gboolean gsb_data_category_fill_transaction_by_string ( gint transaction_number,
                        const gchar *string,
                        gboolean is_transaction )
{
    gchar **tab_char;
    gint category_number = 0;

    if (!string
	||
	!strlen (string))
    {
        /* no string so set no category */
        gsb_data_mix_set_category_number ( transaction_number,
                            0,
                            is_transaction );
        gsb_data_mix_set_sub_category_number ( transaction_number,
                            0,
                            is_transaction );
	    return TRUE;
    }

    tab_char = g_strsplit ( string, " : ", 2 );
    if (tab_char[0])
    {
        category_number = gsb_data_category_get_number_by_name ( tab_char[0],
                            TRUE,
                            gsb_data_mix_get_amount (
                            transaction_number, is_transaction).mantissa <0 );
	    gsb_data_mix_set_category_number ( transaction_number,
                            category_number,
                            is_transaction );
    }

    if ( tab_char[1] && category_number )
    {
        gsb_data_mix_set_sub_category_number ( transaction_number,
					        gsb_data_category_get_sub_category_number_by_name (
                            category_number, tab_char[1], TRUE ),
					        is_transaction );
    }
    else
        gsb_data_mix_set_sub_category_number ( transaction_number, 0, is_transaction );

    g_strfreev (tab_char);

    return TRUE;
}

/**
 * return the number of the category wich has the name in param
 * create it if necessary
 *
 * \param name the name of the category
 * \param create TRUE if we want to create it if it doen't exist
 * \param category_type the type of the category if we create it
 * 0:credit / 1:debit / 2:special (transfert, split...) *
 * \return the number of the category or 0 if problem
 * */
gint gsb_data_category_get_number_by_name ( const gchar *name,
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
	    gsb_data_category_set_type ( category_number, category_type );
	    gsb_category_update_combofix ( FALSE );
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
							 const gchar *name,
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
	{
	    sub_category_number = gsb_data_category_new_sub_category ( category_number,
								       name);
	    gsb_category_update_combofix ( FALSE );
	}
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
							const gchar *name )
{
    return ( my_strcasecmp ( category -> category_name, name ));
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
							    const gchar *name )
{
    if ( !sub_category -> sub_category_name
	 ||
	 !name )
	return 1;
    return ( my_strcasecmp ( sub_category -> sub_category_name, name ) );
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
 * 		the returned value need to be freed after use
 * */
gchar *gsb_data_category_get_name ( gint no_category,
				    gint no_sub_category,
				    const gchar *return_value_error )
{
    struct_category *category;
    gchar *return_value;

    category = gsb_data_category_get_structure ( no_category );

    if (!category || !no_category)
    {
        if ( return_value_error == NULL )
            return NULL;
        else
	        return my_strdup ( return_value_error );
    }

    return_value = my_strdup ( category -> category_name );

    if ( no_sub_category )
    {
        struct_sub_category *sub_category;

        sub_category = gsb_data_category_get_sub_category_structure ( no_category,
                                          no_sub_category );

        if ( sub_category )
        {
            return_value = g_strconcat ( return_value,
                         " : ",
                         sub_category -> sub_category_name,
                         NULL );
        }
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

    if ( !category )
        return FALSE;


    /* we free the last name */
    if ( category -> category_name )
        g_free ( category -> category_name );

    /* and copy the new one */
    if ( name )
    {
        GtkWidget *combofix;

        category -> category_name = my_strdup ( name );
        combofix = gsb_form_widget_get_widget ( TRANSACTION_FORM_PARTY );
        if ( combofix )
            gsb_category_update_combofix ( TRUE );
    }
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
 * \return a newly allocated string with the name of the category
 * 		or return_value_error to be freed too
 * */
gchar *gsb_data_category_get_sub_category_name ( gint no_category,
						 gint no_sub_category,
						 const gchar *return_value_error )
{
    struct_sub_category *sub_category;

    sub_category = gsb_data_category_get_sub_category_structure ( no_category,
								  no_sub_category );

    if (!sub_category)
	return (my_strdup (return_value_error));

    return my_strdup (sub_category -> sub_category_name);
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

    if ( !sub_category )
        return FALSE;

    /* we free the last name */

    if ( sub_category -> sub_category_name )
	g_free (sub_category -> sub_category_name);

    /* and copy the new one */
    if ( name )
    {
        GtkWidget *combofix;
        
        sub_category -> sub_category_name = my_strdup ( name );
        combofix = gsb_form_widget_get_widget ( TRANSACTION_FORM_PARTY );
        if ( combofix )
            gsb_category_update_combofix ( TRUE );
    }
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
 * \param set_split TRUE if we want to add the split to the specials category
 *
 * \return a g_slist of g_slist of gchar *
 * */
GSList *gsb_data_category_get_name_list ( gboolean set_debit,
					  gboolean set_credit,
					  gboolean set_special,
					  gboolean set_split )
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

	if (set_split)
	    special_list = g_slist_append ( special_list,
					    _("Split of transaction"));
	special_list = g_slist_append ( special_list,
					_("Transfer"));

	/* append the accounts name with a tab at the beginning */

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
 * append the sub-categories name with a tab at the beginning
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
 * 0:credit / 1:debit / 2:special (transfert, split...)
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
 * 0:credit / 1:debit / 2:special (transfert, split...)
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
gsb_real gsb_data_category_get_balance ( gint no_category )
{
    struct_category *category;

    category = gsb_data_category_get_structure ( no_category );

    if (!category)
	return null_real;

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
gsb_real gsb_data_category_get_sub_category_balance ( gint no_category,
						      gint no_sub_category )
{
    struct_sub_category *sub_category;

    sub_category = gsb_data_category_get_sub_category_structure ( no_category,
								  no_sub_category );

    if (!sub_category)
	return null_real;

    return sub_category -> sub_category_balance;
}

/**
 * return direct_balance of the category
 *
 * \param no_category the number of the category
 *
 * \return balance of the category or 0 if problem
 * */
gsb_real gsb_data_category_get_direct_balance ( gint no_category )
{
    struct_category *category;

    category = gsb_data_category_get_structure ( no_category );

    if (!category)
	return null_real;

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
	category -> category_balance = null_real;
	category -> category_nb_transactions = 0;
	category -> category_direct_balance = null_real;
	category -> category_nb_direct_transactions = 0;

	sub_list_tmp = category -> sub_category_list;

	while ( sub_list_tmp )
	{
	    struct_sub_category *sub_category;
	    
	    sub_category = sub_list_tmp -> data;

	    sub_category -> sub_category_nb_transactions = 0;
	    sub_category -> sub_category_balance = null_real;

	    sub_list_tmp = sub_list_tmp -> next;
	}
	list_tmp = list_tmp -> next;
    }
    
    /* reset the empty category */
    empty_category -> category_balance = null_real;
    empty_category -> category_nb_transactions = 0;
    empty_category -> category_direct_balance = null_real;
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

    devel_debug ( NULL );
    gsb_data_category_reset_counters ();

    if ( etat.add_archive_in_total_balance )
        list_tmp_transactions = gsb_data_transaction_get_complete_transactions_list ();
    else
        list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

    while ( list_tmp_transactions )
    {
    gint transaction_number_tmp;
    transaction_number_tmp = gsb_data_transaction_get_transaction_number (
                        list_tmp_transactions -> data );

    gsb_data_category_add_transaction_to_category ( transaction_number_tmp,
                        gsb_data_transaction_get_category_number ( transaction_number_tmp ),
                        gsb_data_transaction_get_sub_category_number (
                        transaction_number_tmp ) );

    list_tmp_transactions = list_tmp_transactions -> next;
    }
}



/**
 * Add the given transaction to a category in the counters if no
 * category is specified, add it to the blank category.
 *
 * \param transaction_number the transaction we want to work with
 *
 * \return
 * */
void gsb_data_category_add_transaction_to_category ( gint transaction_number,
						     gint category_id,
						     gint sub_category_id )
{
    struct_category *category;
    struct_sub_category *sub_category;

    /* if the transaction is a transfer or a split transaction, don't take it */
    if (gsb_data_transaction_get_split_of_transaction (transaction_number)
	||
	gsb_data_transaction_get_contra_transaction_number (transaction_number) > 0)
	return;

    category = gsb_data_category_get_structure ( category_id );
    sub_category = gsb_data_category_get_sub_category_structure ( category_id,
								  sub_category_id );

    /* should not happen, this is if the transaction has a categ wich doesn't exist
     * we show a debug warning and get without categ */
    if (!category)
    {
	gchar *tmpstr;
	tmpstr = g_strdup_printf ( _("The transaction %d has a category n°%d and sub-category n°%d but they don't exist."),
				   transaction_number, 
				   category_id,
				   sub_category_id );
	warning_debug (tmpstr);
	g_free (tmpstr);
	category = empty_category;
    }

    /* ok, now category is on the structure or on empty_category */
    if ( category )
    {
	category -> category_nb_transactions ++;
    if ( ! gsb_data_transaction_get_split_of_transaction ( transaction_number ) )
        category -> category_balance = gsb_real_add ( category -> category_balance,
                        gsb_data_transaction_get_adjusted_amount_for_currency (
                        transaction_number, category_tree_currency (), -1));
    }

    /* if we were on empty category, no sub-category */
    if (category == empty_category)
	return;

    if ( sub_category )
    {
	sub_category -> sub_category_nb_transactions ++;
    if ( ! gsb_data_transaction_get_split_of_transaction ( transaction_number ) )
        sub_category -> sub_category_balance = gsb_real_add (
                        sub_category -> sub_category_balance,
                        gsb_data_transaction_get_adjusted_amount_for_currency (
                        transaction_number, category_tree_currency (), -1));
    }
    else
    {
	category -> category_nb_direct_transactions ++;
    if ( ! gsb_data_transaction_get_split_of_transaction ( transaction_number ) )
        category -> category_direct_balance = gsb_real_add (
                        category -> category_direct_balance,
                        gsb_data_transaction_get_adjusted_amount_for_currency (
                        transaction_number, category_tree_currency (), -1));
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
	category -> category_balance = gsb_real_sub ( category -> category_balance,
						      gsb_data_transaction_get_adjusted_amount_for_currency ( transaction_number,
													      category_tree_currency (), -1));
	if ( !category -> category_nb_transactions ) /* Cope with float errors */
	    category -> category_balance = null_real;
    }

    if ( sub_category )
    {
	sub_category -> sub_category_nb_transactions --;
	sub_category -> sub_category_balance = gsb_real_sub ( sub_category -> sub_category_balance,
							      gsb_data_transaction_get_adjusted_amount_for_currency ( transaction_number,
														      category_tree_currency (), -1));
	if ( !sub_category -> sub_category_nb_transactions ) /* Cope with float errors */
	    sub_category -> sub_category_balance = null_real;
    }
    else
    {
	if ( category )
	{
	    category -> category_nb_direct_transactions --;
	    category -> category_direct_balance = gsb_real_sub ( category -> category_direct_balance,
								 gsb_data_transaction_get_adjusted_amount_for_currency ( transaction_number,
															 category_tree_currency (), -1));
	}
    }
}



/**
 * Find if two sub categories are the same
 *
 * \param a		First sub-category to compare.
 * \param b		Second sub-category to compare.	
 *
 * \return		Same as a <=> b.
 */
gint gsb_data_sub_category_compare ( struct_sub_category * a, struct_sub_category * b )
{
    if ( a != b && a -> sub_category_number == b -> sub_category_number )
    {
	return 0;
    }
    return 1;
}



/**
 * \brief Debug check to verify if some sub categories are doubled.
 * 
 * This is a bug caused in old version of Grisbi and this check has to
 * be there since we need to access to the structures directly without
 * resorting to numeric ids.
 *
 * \return	NULL if no error found.  A string describing any issue
 *		if any.
 */
gchar * gsb_debug_duplicate_categ_check ()
{
    GSList * tmp;
    gint num_duplicate = 0;
    gchar * output = "";

    tmp = category_list;
    while ( tmp )
    {
	struct_category * categ = tmp -> data;
	GSList * tmp_sous_categ = categ -> sub_category_list;

	while ( tmp_sous_categ )
	{
	    GSList * duplicate;
	    duplicate = g_slist_find_custom ( categ -> sub_category_list,
					      tmp_sous_categ -> data,
					      (GCompareFunc) gsb_data_sub_category_compare );
	    /* Second comparison is just there to find only one of them. */
	    if ( duplicate && duplicate > tmp_sous_categ )
	    {
	        gchar* tmpstr1 = output;
		gchar* tmpstr2 = g_strdup_printf ( 
				_("In <i>%s</i>, <i>%s</i> is a duplicate of <i>%s</i>.\n"),
				categ -> category_name,
				((struct_sub_category *) tmp_sous_categ -> data) -> sub_category_name,
				((struct_sub_category *) duplicate -> data) -> sub_category_name );
		output = g_strconcat ( tmpstr1,
				       tmpstr2,
				       NULL );
		g_free ( tmpstr1 );
		g_free ( tmpstr2 );
		num_duplicate ++;
	    }
	    tmp_sous_categ = tmp_sous_categ -> next;
	}
	
	tmp = tmp -> next;
    }

    if ( num_duplicate )
    {
	output [ strlen ( output ) - 1 ] = '\0';
	return output;
    }
    
    return NULL;
}



/**
 * Fix any duplicate in sub categories.  
 *
 * \return	TRUE on success.  FALSE otherwise.
 */
gboolean gsb_debug_duplicate_categ_fix ()
{
    GSList * tmp;

    tmp = category_list;
    while ( tmp )
    {
	struct_category * categ = tmp -> data;
	GSList * tmp_sous_categ = categ -> sub_category_list;

	while ( tmp_sous_categ )
	{
	    GSList * duplicate;
	    duplicate = g_slist_find_custom ( categ -> sub_category_list,
					      tmp_sous_categ -> data,
					      (GCompareFunc) gsb_data_sub_category_compare );
	    if ( duplicate )
	    {
		struct_sub_category * duplicate_categ = duplicate -> data;

		duplicate_categ -> sub_category_number = gsb_data_category_max_sub_category_number ( categ -> category_number ) + 1;
	    }
	    tmp_sous_categ = tmp_sous_categ -> next;
	}
	
	tmp = tmp -> next;
    }

    return TRUE;
}


/**
 * 
 * 
 *
 * \param 
 *
 * \return
 * */

gint gsb_data_category_test_create_category ( gint no_category,
                        const gchar *name,
                        gint category_type )
{
    GSList *list_tmp;
    gint category_number = 0;
    struct_category *category;

    list_tmp = g_slist_find_custom ( category_list,
                        name,
                        (GCompareFunc) gsb_data_category_get_pointer_from_name_in_glist );

    if ( list_tmp )
    {
        category = list_tmp -> data;
        return category -> category_number;
    }
    else
    {
        category = gsb_data_category_get_structure ( no_category );

        if ( !category )
        {
            category_number = gsb_data_category_new_with_number ( no_category );
            gsb_data_category_set_name ( category_number, name );
            gsb_data_category_set_type ( category_number, category_type );
        }
        else
        {
            category_number = gsb_data_category_new (name);
            gsb_data_category_set_type ( category_number, category_type );
            gsb_category_update_combofix ( FALSE );
        }
        return category_number;
    }
}


gboolean gsb_data_category_test_create_sub_category ( gint no_category,
                        gint no_sub_category,
                        const gchar *name )
{
    GSList *list_tmp;
    gint sub_category_number = 0;
    struct_category *category;
    struct_sub_category *sub_category;

    category = gsb_data_category_get_structure ( no_category );
    list_tmp = g_slist_find_custom ( category -> sub_category_list,
                        name,
                        (GCompareFunc) gsb_data_category_get_pointer_from_sub_name_in_glist );

    if ( list_tmp )
        return TRUE;
    else
    {
        sub_category = gsb_data_category_get_sub_category_structure ( no_category,
                        no_sub_category );
        if ( !sub_category )
        {
            sub_category_number = gsb_data_category_new_sub_category_with_number (
                        no_sub_category, no_category );
            gsb_data_category_set_sub_category_name ( no_category, no_sub_category, name );
        }
        else
        {
            sub_category_number = gsb_data_category_new_sub_category ( no_category, name );
            gsb_category_update_combofix ( FALSE );
        }
        return TRUE;
    }
    return FALSE;
}
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
