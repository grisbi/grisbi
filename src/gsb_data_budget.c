/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
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
 * \file gsb_budget_data.c
 * work with the budget structure, no GUI here
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_data_budget.h"
#include "meta_budgetary.h"
#include "imputation_budgetaire.h"
#include "gsb_data_form.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_transaction.h"
#include "gsb_form_widget.h"
#include "gsb_real.h"
#include "utils_str.h"
#include "structures.h"
#include "erreur.h"
/*END_INCLUDE*/


/**
 * \struct 
 * Describe a budget
 */
typedef struct
{
    /** @name budget content */
    guint budget_number;
    gchar *budget_name;
    gint budget_type;		/**< 0:credit / 1:debit  */

    GSList *sub_budget_list;

    /** @name gui budget list content (not saved) */
    gint budget_nb_transactions;
    gint budget_nb_direct_transactions;
    gsb_real budget_balance;
    gsb_real budget_direct_balance;
} struct_budget;


/**
 * \struct 
 * Describe a sub-budget
 */
typedef struct
{
    /** @name sub-budget content */
    guint sub_budget_number;
    gchar *sub_budget_name;

    guint mother_budget_number;

    /** @name gui sub-budget list content (not saved)*/
    gint sub_budget_nb_transactions;
    gsb_real sub_budget_balance;
} struct_sub_budget;


/*START_STATIC*/
static void _gsb_data_budget_free ( struct_budget* budget );
static void _gsb_data_sub_budget_free ( struct_sub_budget* sub_budget );
static GSList *gsb_data_budget_append_sub_budget_to_list ( GSList *budget_list,
                        GSList *sub_budget_list );
static gint gsb_data_budget_get_pointer_from_name_in_glist ( struct_budget *budget,
                        const gchar *name );
static gint gsb_data_budget_get_pointer_from_sub_name_in_glist ( struct_sub_budget *sub_budget,
                        const gchar *name );
static gpointer gsb_data_budget_get_structure_in_list ( gint no_budget,
                        GSList *list );
static gint gsb_data_budget_max_number ( void );
static gint gsb_data_budget_max_sub_budget_number ( gint budget_number );
static gint gsb_data_budget_new ( const gchar *name );
static gint gsb_data_budget_new_sub_budget ( gint budget_number,
                        const gchar *name );
static void gsb_data_budget_reset_counters ( void );
static gint gsb_data_sub_budget_compare ( struct_sub_budget * a, struct_sub_budget * b );
/*END_STATIC*/

/*START_EXTERN*/
extern gsb_real null_real;
/*END_EXTERN*/

/** contains the g_slist of struct_budget */
static GSList *budget_list = NULL;

/** a pointer to the last budget used (to increase the speed) */
static struct_budget *budget_buffer;
static struct_sub_budget *sub_budget_buffer;

/** a empty budget for the list of budgets
 * the number of the empty budget is 0 */
static struct_budget *empty_budget = NULL;


/**
 * set the budgets global variables to NULL, usually when we init all the global variables
 *
 * \param none
 *
 * \return FALSE
 * */
gboolean gsb_data_budget_init_variables ( void )
{
    if ( budget_list )
    {
        /* free memory used by the budget list */
        GSList *tmp_list = budget_list;
        while ( tmp_list )
        {
            struct_budget *budget;
            budget = tmp_list -> data;
            tmp_list = tmp_list -> next;
	    _gsb_data_budget_free ( budget );
        }
	g_slist_free (budget_list);
    }
    budget_list = NULL;

    /* recreate the empty budget */
    _gsb_data_budget_free ( empty_budget );
    empty_budget = g_malloc0 ( sizeof ( struct_budget ));
    empty_budget -> budget_name = g_strdup(_("No budget line"));

    budget_buffer = NULL;
    sub_budget_buffer = NULL;

    return FALSE;
}



/**
 * This internal function is used to free the memory used by a struct_budget structure
 */
static void _gsb_data_budget_free ( struct_budget* budget )
{
    if ( ! budget )
        return;
    /* free memory used by sub-bugdgets */
    if ( budget -> sub_budget_list)
    {
        GSList* sub_tmp_list = budget -> sub_budget_list;
        while ( sub_tmp_list )
        {
	    struct_sub_budget *sub_budget;
	    sub_budget = sub_tmp_list -> data;
	    sub_tmp_list = sub_tmp_list -> next;
	    _gsb_data_sub_budget_free ( sub_budget );
        }
	g_slist_free ( budget -> sub_budget_list );
    }
    if ( budget -> budget_name )
	g_free ( budget -> budget_name);
    g_free ( budget );
    if ( budget_buffer == budget )
	budget_buffer = NULL;
}

/**
 * This internal function is used to free the memory used by a struct_sub_budget structure
 */
static void _gsb_data_sub_budget_free ( struct_sub_budget* sub_budget )
{
    if ( ! sub_budget )
        return;
    if ( sub_budget -> sub_budget_name )
	g_free ( sub_budget -> sub_budget_name);
    g_free ( sub_budget );
    if ( sub_budget_buffer == sub_budget )
	sub_budget_buffer = NULL;
}


/**
 * find and return the structure of the budget asked
 *
 * \param no_budget number of budget
 *
 * \return the adr of the struct of the budget (empty_budget if doesn't exit)
 * */
gpointer gsb_data_budget_get_structure ( gint no_budget )
{
    if (!no_budget)
	return empty_budget;

    /* before checking all the budgets, we check the budget_buffer */

    if ( budget_buffer
	 &&
	 budget_buffer -> budget_number == no_budget )
	return budget_buffer;

    return gsb_data_budget_get_structure_in_list ( no_budget,
						   budget_list );
}


/**
 * return the empty_budget pointer
 *
 * \param
 *
 * \return a pointer to empty_budget */
gpointer gsb_data_budget_get_empty_budget ( void )
{
    return gsb_data_budget_get_structure (0);
}


/**
 * find and return the structure of the budget in the list given in param
 * don't use at this level the buffer because could be a bug for an imported list
 * so for normal list, use always gsb_data_budget_get_structure
 *
 * \param no_budget number of budget
 * \param list the list of budgets struct where we look for
 *
 * \return the adr of the struct of the budget (NULL if doesn't exit)
 * */
gpointer gsb_data_budget_get_structure_in_list ( gint no_budget,
                        GSList *list )
{
    GSList *tmp;

    if (!no_budget)
	return NULL;

    tmp = list;

    while ( tmp )
    {
	struct_budget *budget;

	budget = tmp -> data;

	if ( budget -> budget_number == no_budget )
	{
	    budget_buffer = budget;
	    return budget;
	}
	tmp = tmp -> next;
    }
    return NULL;
}



/**
 * find and return the structure of the sub-budget asked
 *
 * \param no_budget number of budget
 * \param no_sub_budget the number of the sub-budget
 *
 * \return the adr of the struct of the sub-budget (NULL if doesn't exit)
 * */
gpointer gsb_data_budget_get_sub_budget_structure ( gint no_budget,
                        gint no_sub_budget )
{
    GSList *tmp;
    struct_budget *budget;

    if (!no_budget
	||
	!no_sub_budget)
	return NULL;

    /* before checking all the budgets, we check the buffer */

    if ( sub_budget_buffer
	 &&
	 sub_budget_buffer -> mother_budget_number == no_budget
	 &&
	 sub_budget_buffer -> sub_budget_number == no_sub_budget )
	return sub_budget_buffer;

    budget = gsb_data_budget_get_structure ( no_budget );
    if ( ! budget )
    {
	return NULL;
    }

    tmp = budget -> sub_budget_list;

    while ( tmp )
    {
	struct_sub_budget *sub_budget;

	sub_budget = tmp -> data;

	if ( sub_budget -> sub_budget_number == no_sub_budget )
	{
	    sub_budget_buffer = sub_budget;
	    return sub_budget;
	}
	tmp = tmp -> next;
    }
    return NULL;
}


/**
 * give the g_slist of budgets structure
 * usefull when want to check all budgets
 *
 * \param none
 *
 * \return the g_slist of budgets structure
 * */
GSList *gsb_data_budget_get_budgets_list ( void )
{
    return budget_list;
}


/**
 * return the g_slist of the sub-budgets of the budget
 *
 * \param no_budget the number of the budget
 *
 * \return a g_slist of the struct of the sub-budgets or NULL if problem
 * */
GSList *gsb_data_budget_get_sub_budget_list ( gint no_budget )
{
    struct_budget *budget;

    budget = gsb_data_budget_get_structure ( no_budget );

    if (!budget)
	return 0;

    return budget -> sub_budget_list;
}



/**
 * return the number of the budgets given in param
 *
 * \param budget_ptr a pointer to the struct of the budget
 *
 * \return the number of the budget, 0 if problem
 * */
gint gsb_data_budget_get_no_budget ( gpointer budget_ptr )
{
    struct_budget *budget;

    if ( !budget_ptr )
	return 0;

    budget = budget_ptr;
    budget_buffer = budget;
    return budget -> budget_number;
}


/**
 * return the number of the sub-budget given in param
 *
 * \param sub_budget_ptr a pointer to the struct of the sub-budget
 *
 * \return the number of the budget, 0 if problem
 * */
gint gsb_data_budget_get_no_sub_budget ( gpointer sub_budget_ptr )
{
    struct_sub_budget *sub_budget;

    if ( !sub_budget_ptr )
	return 0;

    sub_budget = sub_budget_ptr;
    sub_budget_buffer = sub_budget;
    return sub_budget -> sub_budget_number;
}




/** find and return the last number of budget
 * \param none
 * \return last number of budget
 * */
gint gsb_data_budget_max_number ( void )
{
    GSList *tmp;
    gint number_tmp = 0;

    tmp = budget_list;

    while ( tmp )
    {
	struct_budget *budget;

	budget = tmp -> data;

	if ( budget -> budget_number > number_tmp )
	    number_tmp = budget -> budget_number;

	tmp = tmp -> next;
    }
    return number_tmp;
}


/**
 * find and return the last number of the sub-budgets
 *
 * \param 
 * 
 * \return last number of the sub-budgets
 * */
gint gsb_data_budget_max_sub_budget_number ( gint budget_number )
{
    struct_budget *budget;
    GSList *tmp;
    gint number_tmp = 0;

    budget = gsb_data_budget_get_structure ( budget_number );

    tmp = budget -> sub_budget_list;

    while ( tmp )
    {
	struct_sub_budget *sub_budget;

	sub_budget = tmp -> data;

	if ( sub_budget -> sub_budget_number > number_tmp )
	    number_tmp = sub_budget -> sub_budget_number;

	tmp = tmp -> next;
    }
    return number_tmp;
}



/**
 * create a new budget, give it a number, append it to the list
 * and return the number
 * update combofix and mark file as modified
 *
 * \param name the name of the budget (can be freed after, it's a copy) or NULL
 *
 * \return the number of the new budget
 * */
gint gsb_data_budget_new ( const gchar *name )
{
    gint budget_number;

    /* create the new budget with a new number */

    budget_number = gsb_data_budget_new_with_number ( gsb_data_budget_max_number () + 1 );

    /* append the name if necessary */

    if (name)
	gsb_data_budget_set_name ( budget_number,
				   name );
    return budget_number;
}


/**
 * create a new budget with a number, append it to the list
 * and return the number
 * 
 *
 * \param number the number we want to give to that budget
 * \param import_list a g_slist with the imported budgets if we are importing them,
 * NULL else and the budget will be hapened to the normal budgets list
 *
 * \return the number of the new budget
 * */
gint gsb_data_budget_new_with_number ( gint number )
{
    struct_budget *budget;

    budget = g_malloc0 ( sizeof ( struct_budget ));
    budget -> budget_number = number;

    budget_list = g_slist_append ( budget_list,
				   budget );

    budget_buffer = budget;

    return budget -> budget_number;
}



/**
 * remove a budget
 * set all the budgets of transaction which are this one to 0
 * update combofix and mark file as modified
 *
 * \param no_budget the budget we want to remove
 *
 * \return TRUE ok
 * */
gboolean gsb_data_budget_remove ( gint no_budget )
{
    struct_budget *budget;

    budget = gsb_data_budget_get_structure ( no_budget );

    if (!budget)
	return FALSE;

    budget_list = g_slist_remove ( budget_list,
				   budget );

    _gsb_data_budget_free (budget);

    return TRUE;
}


/**
 * remove a sub-budget from a budget
 * set all the budgets of transaction which are this one to 0
 * update combofix and mark file as modified
 *
 * \param no_budget the budget we want to remove
 *
 * \return TRUE ok
 * */
gboolean gsb_data_budget_sub_budget_remove ( gint no_budget,
                        gint no_sub_budget )
{
    struct_budget *budget;
    struct_sub_budget *sub_budget;

    budget = gsb_data_budget_get_structure ( no_budget );
    sub_budget = gsb_data_budget_get_sub_budget_structure ( no_budget,
							    no_sub_budget );

    if (!budget
	||
	!sub_budget)
	return FALSE;

    budget -> sub_budget_list = g_slist_remove ( budget -> sub_budget_list,
						 sub_budget );

    _gsb_data_sub_budget_free (sub_budget);

    return TRUE;
}


/**
 * create a new sub-budget, append it to the list
 * and return the new number
 *
 * \param budget_number the number of the mother
 * \param name the name of the sub-budget
 *
 * \return the number of the new sub-budget or 0 if problem
 * */
gint gsb_data_budget_new_sub_budget ( gint budget_number,
                        const gchar *name )
{
    gint sub_budget_number;

    sub_budget_number = gsb_data_budget_new_sub_budget_with_number ( gsb_data_budget_max_sub_budget_number (budget_number) + 1,
								     budget_number );

    /* append the name if necessary */

    if (name)
	gsb_data_budget_set_sub_budget_name ( budget_number,
					      sub_budget_number,
					      name );

    return sub_budget_number;
}


/**
 * create a new sub-budget with a number, append it to the list
 * and return the number
 *
 * \param number the number we want to give to that sub-budget
 * \param budget_number the number of the mother
 *
 * \return the number of the new sub-budget or 0 if problem
 * */
gint gsb_data_budget_new_sub_budget_with_number ( gint number,
                        gint budget_number)
{
    struct_budget *budget;
    struct_sub_budget *sub_budget;

    budget = gsb_data_budget_get_structure ( budget_number );

    if (!budget)
	return 0;

    sub_budget = g_malloc0 ( sizeof (struct_sub_budget));
    sub_budget -> sub_budget_number = number;
    sub_budget -> mother_budget_number = budget_number;

    budget -> sub_budget_list = g_slist_append ( budget -> sub_budget_list,
						 sub_budget );

    sub_budget_buffer = sub_budget;

    return sub_budget -> sub_budget_number;
}



/**
 * return the number of the budget wich has the name in param
 * create it if necessary
 *
 * \param name the name of the budget
 * \param create TRUE if we want to create it if it doen't exist
 * \param budget_type the type of the budget if we create it
 *
 * \return the number of the budget or FALSE if problem
 * */
gint gsb_data_budget_get_number_by_name ( const gchar *name,
                        gboolean create,
                        gint budget_type )
{
    GSList *list_tmp;
    gint budget_number = 0;

    if (!name)
	return FALSE;

    if (!strlen (name))
	return FALSE;

    list_tmp = g_slist_find_custom ( budget_list,
				     name,
				     (GCompareFunc) gsb_data_budget_get_pointer_from_name_in_glist );

    if ( list_tmp )
    {
	struct_budget *budget;

	budget = list_tmp -> data;
	budget_number = budget -> budget_number;
    }
    else
    {
	if (create)
	{
	    budget_number = gsb_data_budget_new (name);
	    gsb_data_budget_set_type ( budget_number, budget_type );
	    gsb_budget_update_combofix ( FALSE );
	}
    }
    return budget_number;
}



/**
 * return the number of the sub-budget wich has the name in param
 * create it if necessary
 *
 * \param budget_number the number of the budget
 * \param name the name of the sub-budget
 * \param create TRUE if we want to create it if it doen't exist
 *
 * \return the number of the sub-budget or FALSE if problem
 * */
gint gsb_data_budget_get_sub_budget_number_by_name ( gint budget_number,
                        const gchar *name,
                        gboolean create )
{
    GSList *list_tmp;
    struct_budget *budget;
    gint sub_budget_number = 0;

    if (!name || !strlen (name))
	return FALSE;

    budget = gsb_data_budget_get_structure ( budget_number );

    if (!budget)
	return FALSE;

    list_tmp = g_slist_find_custom ( budget -> sub_budget_list,
				     name,
				     (GCompareFunc) gsb_data_budget_get_pointer_from_sub_name_in_glist );

    if ( list_tmp )
    {
        struct_sub_budget *sub_budget;

        sub_budget = list_tmp -> data;
        sub_budget_number = sub_budget -> sub_budget_number;
    }
    else
    {
        if ( create )
        {
            sub_budget_number = gsb_data_budget_new_sub_budget ( budget_number, name );
            gsb_budget_update_combofix ( FALSE );
        }
    }
    return sub_budget_number;
}




/**
 * used with g_slist_find_custom to find a budget in the g_list
 * by his name
 *
 * \param budget the struct of the current budget checked
 * \param name the name we are looking for
 *
 * \return 0 if it's the same name
 * */
gint gsb_data_budget_get_pointer_from_name_in_glist ( struct_budget *budget,
                        const gchar *name )
{
    return ( my_strcasecmp ( budget -> budget_name, name ) );
}


/**
 * used with g_slist_find_custom to find a sub-budget in the g_list
 * by his name
 *
 * \param sub_budget the struct of the current sub_budget checked
 * \param name the name we are looking for
 *
 * \return 0 if it's the same name
 * */
gint gsb_data_budget_get_pointer_from_sub_name_in_glist ( struct_sub_budget *sub_budget,
                        const gchar *name )
{
    return ( my_strcasecmp ( sub_budget -> sub_budget_name, name ));
}


/**
 * return the name of the budget
 * and the full name (ie budget : sub-budget if no_sub_budget is given)
 *
 * \param no_budget the number of the budget
 * \param no_sub_budget if we want the full name of the budget
 * \param return_value_error if problem, the value we return
 *
 * \return the name of the budget (in a newly allocated string), budget : sub-budget or NULL/No budget if problem
 * 	the returned value need to be freed
 * */
gchar *gsb_data_budget_get_name ( gint no_budget,
                    gint no_sub_budget,
                    const gchar *return_value_error )
{
    struct_budget *budget;
    gchar *return_value;

    budget = gsb_data_budget_get_structure ( no_budget );

    if (!budget || !no_budget)
	return my_strdup (return_value_error);

    return_value = my_strdup ( budget -> budget_name );

    if ( no_sub_budget )
    {
	struct_sub_budget *sub_budget;

	sub_budget = gsb_data_budget_get_sub_budget_structure ( no_budget,
								no_sub_budget );

	if (sub_budget)
	{
	    return_value = g_strconcat ( return_value,
					 " : ",
					 sub_budget -> sub_budget_name,
					 NULL );
	}
    }
    return return_value;
}


/**
 * set the name of the budget
 * the value is dupplicate in memory
 *
 * \param no_budget the number of the budget
 * \param name the name of the budget
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_budget_set_name ( gint no_budget,
                    const gchar *name )
{
    struct_budget *budget;

    budget = gsb_data_budget_get_structure ( no_budget );

    if (!budget)
        return FALSE;

    /* we free the last name */

    if ( budget -> budget_name )
        g_free (budget -> budget_name);

    /* and copy the new one */
    if ( name )
    {
        GtkWidget *combofix;

        budget -> budget_name = my_strdup (name);
        combofix = gsb_form_widget_get_widget ( TRANSACTION_FORM_BUDGET );
        if ( combofix )
            gsb_budget_update_combofix ( TRUE );
    }
    else
        budget -> budget_name = NULL;

    return TRUE;
}

/**
 * return the name of the sub-budget
 *
 * \param no_budget the number of the budget
 * \param no_sub_budget the number of the sub-budget
 * \param return_value_error if problem, return that value
 *
 * \return a newly allocated string with the name of the budget 
 * 		or return_value_error to be freed too
 * */
gchar *gsb_data_budget_get_sub_budget_name ( gint no_budget,
                        gint no_sub_budget,
                        const gchar *return_value_error )
{
    struct_sub_budget *sub_budget;

    sub_budget = gsb_data_budget_get_sub_budget_structure ( no_budget,
							    no_sub_budget );

    if (!sub_budget)
	return (my_strdup (return_value_error));

    return my_strdup (sub_budget -> sub_budget_name);
}


/**
 * set the name of the sub-budget
 * the value is dupplicate in memory
 *
 * \param no_budget the number of the budget
 * \param no_sub_budget the number of the sub-budget
 * \param name the name of the sub-budget
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_budget_set_sub_budget_name ( gint no_budget,
                        gint no_sub_budget,
                        const gchar *name )
{
    struct_sub_budget *sub_budget;

    sub_budget = gsb_data_budget_get_sub_budget_structure ( no_budget,
							    no_sub_budget );

    if (!sub_budget)
	return FALSE;

    /* we free the last name */

    if ( sub_budget -> sub_budget_name )
	g_free (sub_budget -> sub_budget_name);

    /* and copy the new one */
    if ( name )
	sub_budget -> sub_budget_name = my_strdup (name);
    else
	sub_budget -> sub_budget_name = NULL;
    return TRUE;
}


/**
 * return a g_slist of g_slist of names of the budgets
 *
 * \param set_debit TRUE if we want to have the debits
 * \param set_credit TRUE if we want to have the credits
 *
 * \return a g_slist of g_slist of gchar *
 * */
GSList *gsb_data_budget_get_name_list ( gboolean set_debit,
                        gboolean set_credit )
{
    GSList *return_list;
    GSList *tmp_list;
    GSList *debit_list = NULL;
    GSList *credit_list = NULL;

    return_list = NULL;

    /* fill debit_list and/or credit_list and them sub-budgets */
    tmp_list = budget_list;

    while ( tmp_list )
    {
	struct_budget *budget;

	budget = tmp_list -> data;

	if ( budget -> budget_type )
	{
	    if ( set_debit )
	    {
		debit_list = g_slist_append ( debit_list,
					      budget -> budget_name);
		debit_list = gsb_data_budget_append_sub_budget_to_list ( debit_list,
									 budget -> sub_budget_list);
	    }
	}
	else
	{
	    if ( set_credit )
	    {
		credit_list = g_slist_append ( credit_list,
					       budget -> budget_name);
		credit_list = gsb_data_budget_append_sub_budget_to_list ( credit_list,
									  budget -> sub_budget_list);
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

    return return_list;
}



/**
 * append the sub-budgets name with a tab at the beginning
 * to the list of budgets given in param
 *
 * \param budget_list a g_slist of budgets names
 * \param sub_budget_list a g_slist which contains the sub budgets to append
 *
 * \return the new budget_list (normally shouldn't changed
 * */
GSList *gsb_data_budget_append_sub_budget_to_list ( GSList *budget_list,
                        GSList *sub_budget_list )
{
    GSList *tmp_list;

    if (!sub_budget_list)
	return budget_list;

    tmp_list = sub_budget_list;

    while (tmp_list)
    {
	struct_sub_budget *sub_budget;

	sub_budget = tmp_list -> data;

	budget_list = g_slist_append ( budget_list,
				       g_strconcat ( "\t",
						     sub_budget -> sub_budget_name,
						     NULL ));
	tmp_list = tmp_list -> next;
    }
    return budget_list;
}




/**
 * return the type of the budget
 * 0:credit / 1:debit / 2:special (transfert, split...)
 *
 * \param no_budget the number of the budget
 * \param can_return_null if problem, return NULL if TRUE or "No budget" if FALSE
 *
 * \return the name of the budget or NULL/No budget if problem
 * */
gint gsb_data_budget_get_type ( gint no_budget )
{
    struct_budget *budget;

    budget = gsb_data_budget_get_structure ( no_budget );

    if (!budget)
	return 0;

    return budget -> budget_type;
}


/**
 * set the type of the budget
 * 0:credit / 1:debit / 2:special (transfert, split...)
 *
 * \param no_budget the number of the budget
 * \param name the name of the budget
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_budget_set_type ( gint no_budget,
                        gint budget_type )
{
    struct_budget *budget;

    budget = gsb_data_budget_get_structure ( no_budget );

    if (!budget)
	return FALSE;

    budget -> budget_type = budget_type;
    return TRUE;
}


/**
 * return nb_transactions of the budget
 *
 * \param no_budget the number of the budget
 *
 * \return nb_transactions of the budget or 0 if problem
 * */
gint gsb_data_budget_get_nb_transactions ( gint no_budget )
{
    struct_budget *budget;

    budget = gsb_data_budget_get_structure ( no_budget );

    if (!budget)
	return 0;

    return budget -> budget_nb_transactions;
}



/**
 * return nb_transactions of the sub-budget
 *
 * \param no_budget the number of the budget
 * \param no_sub_budget the number of the sub-budget
 *
 * \return nb_transactions of the sub-budget or 0 if problem
 * */
gint gsb_data_budget_get_sub_budget_nb_transactions ( gint no_budget,
                        gint no_sub_budget )
{
    struct_sub_budget *sub_budget;

    sub_budget = gsb_data_budget_get_sub_budget_structure ( no_budget,
							    no_sub_budget );

    if (!sub_budget)
	return 0;

    return sub_budget -> sub_budget_nb_transactions;
}



/**
 * return nb_direct_transactions of the budget
 *
 * \param no_budget the number of the budget
 *
 * \return nb_direct_transactions of the budget or 0 if problem
 * */
gint gsb_data_budget_get_nb_direct_transactions ( gint no_budget )
{
    struct_budget *budget;

    budget = gsb_data_budget_get_structure ( no_budget );

    if (!budget)
	return 0;

    return budget -> budget_nb_direct_transactions;
}




/**
 * return balance of the budget
 *
 * \param no_budget the number of the budget
 *
 * \return balance of the budget or 0 if problem
 * */
gsb_real gsb_data_budget_get_balance ( gint no_budget )
{
    struct_budget *budget;

    budget = gsb_data_budget_get_structure ( no_budget );

    if (!budget)
	return null_real;

    return budget -> budget_balance;
}


/**
 * return balance of the sub-budget
 *
 * \param no_budget the number of the budget
 * \param no_sub_budget the number of the sub-budget
 *
 * \return balance of the sub-budget or 0 if problem
 * */
gsb_real gsb_data_budget_get_sub_budget_balance ( gint no_budget,
                        gint no_sub_budget )
{
    struct_sub_budget *sub_budget;

    sub_budget = gsb_data_budget_get_sub_budget_structure ( no_budget,
							    no_sub_budget );

    if (!sub_budget)
	return null_real;

    return sub_budget -> sub_budget_balance;
}

/**
 * return direct_balance of the budget
 *
 * \param no_budget the number of the budget
 *
 * \return balance of the budget or 0 if problem
 * */
gsb_real gsb_data_budget_get_direct_balance ( gint no_budget )
{
    struct_budget *budget;

    budget = gsb_data_budget_get_structure ( no_budget );

    if (!budget)
	return null_real;

    return budget -> budget_direct_balance;
}



/**
 * reset the counters of the budgets and sub-budgets
 *
 * \param 
 *
 * \return 
 * */
void gsb_data_budget_reset_counters ( void )
{
    GSList *list_tmp;

    list_tmp = budget_list;

    while ( list_tmp )
    {
	struct_budget *budget;
	GSList *sub_list_tmp;

	budget = list_tmp -> data;
	budget -> budget_balance = null_real;
	budget -> budget_nb_transactions = 0;
	budget -> budget_direct_balance = null_real;
	budget -> budget_nb_direct_transactions = 0;

	sub_list_tmp = budget -> sub_budget_list;

	while ( sub_list_tmp )
	{
	    struct_sub_budget *sub_budget;

	    sub_budget = sub_list_tmp -> data;

	    sub_budget -> sub_budget_nb_transactions = 0;
	    sub_budget -> sub_budget_balance = null_real;

	    sub_list_tmp = sub_list_tmp -> next;
	}
	list_tmp = list_tmp -> next;
    }

    /* reset the empty budget */
    empty_budget -> budget_balance = null_real;
    empty_budget -> budget_nb_transactions = 0;
    empty_budget -> budget_direct_balance = null_real;
    empty_budget -> budget_nb_direct_transactions = 0;
}

/**
 * update the counters of the budgets
 *
 * \param
 *
 * \return
 * */
void gsb_data_budget_update_counters ( void )
{
    GSList *list_tmp_transactions;

    gsb_data_budget_reset_counters ();

    if ( etat.add_archive_in_total_balance )
        list_tmp_transactions = gsb_data_transaction_get_complete_transactions_list ();
    else
        list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

    while ( list_tmp_transactions )
    {
	gint transaction_number_tmp = gsb_data_transaction_get_transaction_number ( list_tmp_transactions -> data);

	gsb_data_budget_add_transaction_to_budget ( transaction_number_tmp,
						    gsb_data_transaction_get_budgetary_number ( transaction_number_tmp ),
						    gsb_data_transaction_get_sub_budgetary_number ( transaction_number_tmp ) );

	list_tmp_transactions = list_tmp_transactions -> next;
    }
}


/**
 * Add the given transaction to a budget in the counters if no budget
 * is specified, add it to the blank budget.
 *
 * \param transaction_number	Transaction we want to work with.
 * \param budget_id		Budget to add transaction into total.
 * \param sub_budget_id		Sub-budget to add transaction into total.
 */
void gsb_data_budget_add_transaction_to_budget ( gint transaction_number,
                        gint budget_id,
                        gint sub_budget_id )
{
    struct_budget *budget;
    struct_sub_budget *sub_budget;

    budget = gsb_data_budget_get_structure (  budget_id );
    sub_budget = gsb_data_budget_get_sub_budget_structure ( budget_id ,
							    sub_budget_id );

    /* should not happen, this is if the transaction has a budget wich doesn't exist
     * we show a debug warning and get without budget */
    if (!budget)
    {
	gchar *tmpstr;
	tmpstr = g_strdup_printf ( _("The transaction %d has a budget n°%d and sub-budget n°%d but they don't exist."),
				   transaction_number, 
				   budget_id,
				   sub_budget_id );
	warning_debug (tmpstr);
	g_free (tmpstr);
	budget = empty_budget;
    }

    /* now budget is on the budget structure or on empty_budget */
    if ( budget )
    {
	budget -> budget_nb_transactions ++;
    if ( ! gsb_data_transaction_get_split_of_transaction ( transaction_number ) )
        budget -> budget_balance = gsb_real_add ( budget -> budget_balance,
                        gsb_data_transaction_get_adjusted_amount_for_currency (
                        transaction_number, budgetary_line_tree_currency (), -1));
    }

    /* if we are on empty_budget, no sub-budget */
    if (budget == empty_budget)
	return;

    if ( sub_budget )
    {
	sub_budget -> sub_budget_nb_transactions ++;
    if ( ! gsb_data_transaction_get_split_of_transaction ( transaction_number ) )
        sub_budget -> sub_budget_balance = gsb_real_add (
                        sub_budget -> sub_budget_balance,
                        gsb_data_transaction_get_adjusted_amount_for_currency (
                        transaction_number, budgetary_line_tree_currency (), -1));
    }
    else
    {
	budget -> budget_nb_direct_transactions ++;
    if ( ! gsb_data_transaction_get_split_of_transaction ( transaction_number ) )
        budget -> budget_direct_balance = gsb_real_add (
                            budget -> budget_direct_balance,
                            gsb_data_transaction_get_adjusted_amount_for_currency (
                            transaction_number, budgetary_line_tree_currency (), -1));
    }
}

/**
 * remove the given transaction to its budget in the counters
 * if the transaction has no budget, remove it to the blank budget
 *
 * \param transaction_number the transaction we want to work with
 *
 * \return
 * */
void gsb_data_budget_remove_transaction_from_budget ( gint transaction_number )
{
    struct_budget *budget;
    struct_sub_budget *sub_budget;

    budget = gsb_data_budget_get_structure ( gsb_data_transaction_get_budgetary_number (transaction_number));
    sub_budget = gsb_data_budget_get_sub_budget_structure ( gsb_data_transaction_get_budgetary_number (transaction_number),
							    gsb_data_transaction_get_sub_budgetary_number (transaction_number));

    if ( budget )
    {
	budget -> budget_nb_transactions --;
	budget -> budget_balance = gsb_real_sub ( budget -> budget_balance,
						  gsb_data_transaction_get_adjusted_amount_for_currency ( transaction_number,
													  budgetary_line_tree_currency (), -1));
	if ( !budget -> budget_nb_transactions ) /* Cope with float errors */
	    budget -> budget_balance = null_real;
    }

    if ( sub_budget )
    {
	sub_budget -> sub_budget_nb_transactions --;
	sub_budget -> sub_budget_balance = gsb_real_sub ( sub_budget -> sub_budget_balance,
							  gsb_data_transaction_get_adjusted_amount_for_currency ( transaction_number,
														  budgetary_line_tree_currency (), -1));
	if ( !sub_budget -> sub_budget_nb_transactions ) /* Cope with float errors */
	    sub_budget -> sub_budget_balance = null_real;
    }
    else
    {
	if ( budget )
	{
	    budget -> budget_nb_direct_transactions --;
	    budget -> budget_direct_balance = gsb_real_sub ( budget -> budget_direct_balance,
							     gsb_data_transaction_get_adjusted_amount_for_currency ( transaction_number,
														     budgetary_line_tree_currency (), -1));
	}
    }
}




/**
 * Find if two sub budgets are the same
 *
 * \param a		First sub-budget to compare.
 * \param b		Second sub-budget to compare.	
 *
 * \return		Same as a <=> b.
 */
gint gsb_data_sub_budget_compare ( struct_sub_budget * a, struct_sub_budget * b )
{
    if ( a != b && a -> sub_budget_number == b -> sub_budget_number )
    {
	return 0;
    }
    return 1;
}



/**
 * \brief Debug check to verify if some sub budgets are doubled.
 * 
 * This is a bug caused in old version of Grisbi and this check has to
 * be there since we need to access to the structures directly without
 * resorting to numeric ids.
 *
 * \return	NULL if no error found.  A string describing any issue
 *		if any.
 */
gchar * gsb_debug_duplicate_budget_check ()
{
    GSList * tmp;
    gint num_duplicate = 0;
    gchar * output = "";

    tmp = budget_list;
    while ( tmp )
    {
	struct_budget * budget = tmp -> data;
	GSList * tmp_sous_budget = budget -> sub_budget_list;

	while ( tmp_sous_budget )
	{
	    GSList * duplicate;
	    duplicate = g_slist_find_custom ( budget -> sub_budget_list,
					      tmp_sous_budget -> data,
					      (GCompareFunc) gsb_data_sub_budget_compare );
	    /* Second comparison is just there to find only one of them. */
	    if ( duplicate && duplicate > tmp_sous_budget )
	    {
	        gchar* tmpstr1 = output;
		gchar* tmpstr2 = g_strdup_printf ( 
		                        _("In <i>%s</i>, <i>%s</i> is a duplicate of <i>%s</i>.\n"),
					budget -> budget_name,
					((struct_sub_budget *) tmp_sous_budget -> data) -> sub_budget_name,
					((struct_sub_budget *) duplicate -> data) -> sub_budget_name );
		output = g_strconcat ( tmpstr1,
				       tmpstr2,
				       NULL );
		g_free ( tmpstr1 );
		g_free ( tmpstr2 );
		num_duplicate ++;
	    }
	    tmp_sous_budget = tmp_sous_budget -> next;
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
 * Fix any duplicate in sub budgets.  
 *
 * \return	TRUE on success.  FALSE otherwise.
 */
gboolean gsb_debug_duplicate_budget_fix ()
{
    GSList * tmp;

    tmp = budget_list;
    while ( tmp )
    {
	struct_budget * budget = tmp -> data;
	GSList * tmp_sous_budget = budget -> sub_budget_list;

	while ( tmp_sous_budget )
	{
	    GSList * duplicate;
	    duplicate = g_slist_find_custom ( budget -> sub_budget_list,
					      tmp_sous_budget -> data,
					      (GCompareFunc) gsb_data_sub_budget_compare );
	    if ( duplicate )
	    {
		struct_sub_budget * duplicate_budget = duplicate -> data;

		duplicate_budget -> sub_budget_number = gsb_data_budget_max_sub_budget_number ( budget -> budget_number ) + 1;
	    }
	    tmp_sous_budget = tmp_sous_budget -> next;
	}
	
	tmp = tmp -> next;
    }

    return TRUE;
}


/**
 * fill the budget of the transaction from the string given in param
 * create the budget if necessary
 * if string is NULL, free the budget of the transaction
 *
 * \param transaction_number
 * \param string a string like "budget : sub_budget"
 * \param is_transaction TRUE if it's for a transaction, FALSE for a scheduled transaction
 *
 * \return
 * */
void gsb_data_budget_set_budget_from_string ( gint transaction_number,
                        const gchar *string,
                        gboolean is_transaction )
{
    gchar **tab_char;
    gint budget_number;

    /* the simpliest is to split in 2 parts, transaction and scheduled,
     * but the 2 parts are exactly the same, exept the call to the functions */
    if (is_transaction)
    {
        if (!string || strlen ( string ) == 0 )
        {
            gsb_data_transaction_set_budgetary_number ( transaction_number, 0 );
            gsb_data_transaction_set_sub_budgetary_number ( transaction_number, 0 );
            return;
        }

        tab_char = g_strsplit ( string, " : ", 2 );

        /* we don't mind if tab_char exists and others, all the checks will be done in ...get_number_by_name */
        budget_number = gsb_data_budget_get_number_by_name ( g_strstrip ( tab_char[0] ),
                                TRUE,
                                gsb_data_transaction_get_amount ( transaction_number ).mantissa < 0 );
        gsb_data_transaction_set_budgetary_number ( transaction_number, budget_number );

        if ( tab_char[1] )
            gsb_data_transaction_set_sub_budgetary_number ( transaction_number,
                                gsb_data_budget_get_sub_budget_number_by_name ( budget_number,
                                                        g_strstrip (tab_char[1]),
                                                        TRUE ));
    }
    else
    {
        if (!string)
        {
            gsb_data_scheduled_set_budgetary_number ( transaction_number, 0 );
            gsb_data_scheduled_set_sub_budgetary_number ( transaction_number, 0 );
            return;
        }

        tab_char = g_strsplit ( string,
                    " : ",
                    2 );

        /* we don't mind if tab_char exists and others, all the checks will be done in ...get_number_by_name */
        budget_number = gsb_data_budget_get_number_by_name ( tab_char[0],
                                     TRUE,
                                     gsb_data_scheduled_get_amount (transaction_number).mantissa <0 );
        gsb_data_scheduled_set_budgetary_number ( transaction_number,
                                budget_number );
        if ( tab_char[1] )
            gsb_data_scheduled_set_sub_budgetary_number ( transaction_number,
                                  gsb_data_budget_get_sub_budget_number_by_name ( budget_number,
                                                          tab_char[1],
                                                          TRUE ));
    }
    g_strfreev (tab_char);
}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
