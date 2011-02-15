/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cédric Auger (cedric@grisbi.org)	          */
/*			2003-2008 Benjamin Drieu (bdrieu@april.org)	                      */
/* 			http://www.grisbi.org				                              */
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
 * \file gsb_data_reconciliation.c
 * work with the reconciliation structure, no GUI here
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_data_reconcile.h"
#include "dialog.h"
#include "gsb_data_transaction.h"
#include "utils_dates.h"
#include "utils_str.h"
#include "gsb_real.h"
/*END_INCLUDE*/

/** \struct
 * describe an reconciliation operation
 * */
typedef struct
{
    gint reconcile_number;
    gchar *reconcile_name;
    gint account_number;

    GDate *reconcile_init_date;
    GDate *reconcile_final_date;

    gsb_real reconcile_init_balance;
    gsb_real reconcile_final_balance;

} struct_reconcile;


/*START_STATIC*/
static void _gsb_data_reconcile_free ( struct_reconcile *reconcile );
static gint gsb_data_reconcile_cmp_int (struct_reconcile *reconcile_1,
                        struct_reconcile *reconcile_2);
static gpointer gsb_data_reconcile_get_structure ( gint reconcile_number );
/*END_STATIC*/

/*START_EXTERN*/
extern gsb_real null_real;
/*END_EXTERN*/


/** contains a g_list of struct_reconcile */
static GList *reconcile_list;

/** a pointer to the last reconcile used (to increase the speed) */
static struct_reconcile *reconcile_buffer;


/**
 * set the reconciles global variables to NULL, usually when we init all the global variables
 *
 * \param none
 *
 * \return FALSE
 * */
gboolean gsb_data_reconcile_init_variables ( void )
{
    if ( reconcile_list )
    {
        GList* tmp_list = reconcile_list;
        while ( tmp_list )
        {
	    struct_reconcile *reconcile;
	    reconcile = tmp_list -> data;
	    tmp_list = tmp_list -> next;
	    _gsb_data_reconcile_free ( reconcile );
        }
	g_list_free ( reconcile_list );
    }
    reconcile_list = NULL;
    reconcile_buffer = NULL;
    return FALSE;
}


/**
 * find and return the structure of the reconcile asked
 *
 * \param reconcile_number number of reconcile
 *
 * \return the adr of the struct of the reconcile (NULL if doesn't exit)
 * */
gpointer gsb_data_reconcile_get_structure ( gint reconcile_number )
{
    GList *tmp;

    if (!reconcile_number)
	return NULL;

    /* before checking all the reconciles, we check the buffer */
    if ( reconcile_buffer
	 &&
	 reconcile_buffer -> reconcile_number == reconcile_number )
	return reconcile_buffer;

    tmp = reconcile_list;

    while ( tmp )
    {
	struct_reconcile *reconcile;

	reconcile = tmp -> data;

	if ( reconcile -> reconcile_number == reconcile_number )
	{
	    reconcile_buffer = reconcile;
	    return reconcile;
	}
	tmp = tmp -> next;
    }
    return NULL;
}

/**
 * return the number of the reconcile given in param
 *
 * \param reconcile_ptr a pointer to the struct of the reconcile
 *
 * \return the number of the reconcile, 0 if problem
 * */
gint gsb_data_reconcile_get_no_reconcile ( gpointer reconcile_ptr )
{
    struct_reconcile *reconcile;

    if ( !reconcile_ptr )
	return 0;

    reconcile = reconcile_ptr;
    reconcile_buffer = reconcile;
    return reconcile -> reconcile_number;
}


/**
 * give the g_list of reconcile structure
 *
 * \param none
 *
 * \return the g_list of reconciles structure
 * */
GList *gsb_data_reconcile_get_reconcile_list ( void )
{
    return reconcile_list;
}



/**
 * find and return the last number of reconcile
 * 
 * \param none
 * 
 * \return last number of reconcile
 * */
gint gsb_data_reconcile_max_number ( void )
{
    GList *tmp;
    gint number_tmp = 0;

    tmp = reconcile_list;

    while ( tmp )
    {
	struct_reconcile *reconcile;

	reconcile = tmp -> data;

	if ( reconcile -> reconcile_number > number_tmp )
	    number_tmp = reconcile -> reconcile_number;

	tmp = tmp -> next;
    }
    return number_tmp;
}


/**
 * create a new reconcile, give it a number, append it to the list
 * and return the number
 *
 * \param name the name of the reconcile (can be freed after, it's a copy) or NULL
 *
 * \return the number of the new reconcile or 0 if memory problem (a message will be showed)
 * */
gint gsb_data_reconcile_new ( const gchar *name )
{
    struct_reconcile *reconcile;

    reconcile = g_malloc0 ( sizeof ( struct_reconcile ));
    if (!reconcile)
    {
	dialogue_error_memory ();
	return 0;
    }

    reconcile -> reconcile_number = gsb_data_reconcile_max_number () + 1;

    if (name)
	reconcile -> reconcile_name = my_strdup (name);

    reconcile_list = g_list_append ( reconcile_list, reconcile );

    return reconcile -> reconcile_number;
}

/**
 * This function is called to free the memory used by a struct_reconcile structure
 */
static void _gsb_data_reconcile_free ( struct_reconcile *reconcile )
{
    if ( ! reconcile )
        return;
    if ( reconcile -> reconcile_name )
        g_free ( reconcile -> reconcile_name );
    if ( reconcile -> reconcile_init_date)
        g_date_free ( reconcile -> reconcile_init_date );
    if ( reconcile -> reconcile_final_date)
        g_date_free ( reconcile -> reconcile_final_date );
    g_free ( reconcile );
    if ( reconcile_buffer == reconcile )
	reconcile_buffer = NULL;
}

/**
 * remove a reconcile
 * all the transactions marked by that reconcile will be marked P
 * and lose the link to that reconcile
 *
 * \param reconcile_number the reconcile we want to remove
 *
 * \return TRUE ok
 * */
gboolean gsb_data_reconcile_remove ( gint reconcile_number )
{
    struct_reconcile *reconcile;
    GSList *list_tmp;

    reconcile = gsb_data_reconcile_get_structure ( reconcile_number );

    if (!reconcile)
	return FALSE;

    reconcile_list = g_list_remove ( reconcile_list,
				      reconcile );
    _gsb_data_reconcile_free ( reconcile );

    /* remove that reconcile of the transactions */
    list_tmp = gsb_data_transaction_get_complete_transactions_list ();

    while (list_tmp)
    {
	gint transaction_number = gsb_data_transaction_get_transaction_number (list_tmp -> data);

	if ( gsb_data_transaction_get_reconcile_number (transaction_number) == reconcile_number )
	{
	    gsb_data_transaction_set_reconcile_number ( transaction_number, 0 );
	    gsb_data_transaction_set_marked_transaction ( transaction_number,
							  OPERATION_POINTEE );
	}
	list_tmp = list_tmp -> next;
    }

    return TRUE;
}


/**
 * set a new number for the reconcile
 * normally used only while loading the file because
 * the number are given automaticly
 *
 * \param reconcile_number the number of the reconcile
 * \param new_no_reconcile the new number of the reconcile
 *
 * \return the new number or 0 if the reconcile doen't exist
 * */
gint gsb_data_reconcile_set_new_number ( gint reconcile_number,
                        gint new_no_reconcile )
{
    struct_reconcile *reconcile;

    reconcile = gsb_data_reconcile_get_structure (reconcile_number);

    if (!reconcile)
	return 0;

    reconcile -> reconcile_number = new_no_reconcile;
    return new_no_reconcile;
}


/**
 * return the name of the reconcile
 *
 * \param reconcile_number the number of the reconcile
 *
 * \return the name of the reconcile or NULL if problem
 * */
const gchar *gsb_data_reconcile_get_name ( gint reconcile_number )
{
    struct_reconcile *reconcile;

    reconcile = gsb_data_reconcile_get_structure ( reconcile_number );

    if (!reconcile)
	return NULL;

    return reconcile -> reconcile_name;
}


/**
 * set the name of the reconcile
 * the value is dupplicate in memory (so parameter can be freed after)
 *
 * \param reconcile_number the number of the reconcile
 * \param name the name of the reconcile
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_reconcile_set_name ( gint reconcile_number,
                        const gchar *name )
{
    struct_reconcile *reconcile;

    reconcile = gsb_data_reconcile_get_structure ( reconcile_number );

    if (!reconcile)
	return FALSE;

    /* we free the last name */
    if ( reconcile -> reconcile_name )
	g_free (reconcile -> reconcile_name);

    /* and copy the new one */
    reconcile -> reconcile_name = my_strdup (name);

    return TRUE;
}



/**
 * return the number of the reconcile wich has the name in param
 * create it if necessary
 *
 * \param name the name of the reconcile
 *
 * \return the number of the reconcile or 0 if doesn't exist
 * */
gint gsb_data_reconcile_get_number_by_name ( const gchar *name )
{
    GList *list_tmp;

    if ( !name || strlen ( name ) == 0 )
	    return FALSE;

    list_tmp = reconcile_list;

    while (list_tmp)
    {
        struct_reconcile *reconcile;

        reconcile = list_tmp -> data;
        if ( reconcile -> reconcile_name
         && strlen ( reconcile -> reconcile_name ) == 0
         && strcmp ( reconcile -> reconcile_name, name ) == 0 )
            return (reconcile -> reconcile_number);

        list_tmp = list_tmp -> next;
    }

    return FALSE;
}


/**
 * return the account number of the reconcile
 *
 * \param account number the number of the reconcile
 *
 * \return the account number of the reconcile or -1 if problem
 * */
gint gsb_data_reconcile_get_account ( gint reconcile_number )
{
    struct_reconcile *reconcile;

    reconcile = gsb_data_reconcile_get_structure ( reconcile_number );

    if (!reconcile)
	return -1;

    return reconcile -> account_number;
}


/**
 * set the account number of the reconcile
 *
 * \param reconcile_number the number of the reconcile
 * \param account_number the account number of the reconcile
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_reconcile_set_account ( gint reconcile_number,
                        gint account_number )
{
    struct_reconcile *reconcile;

    reconcile = gsb_data_reconcile_get_structure ( reconcile_number );

    if (!reconcile)
	return FALSE;

    /* and copy the new one */
    reconcile -> account_number = account_number;

    return TRUE;
}


/**
 * return the init_date of the reconcile
 *
 * \param reconcile_number the number of the reconcile
 *
 * \return the init_date of the reconcile or NULL if problem
 * */
const GDate *gsb_data_reconcile_get_init_date ( gint reconcile_number )
{
    struct_reconcile *reconcile;

    reconcile = gsb_data_reconcile_get_structure ( reconcile_number );

    if (!reconcile)
	return NULL;

    return reconcile -> reconcile_init_date;
}


/**
 * set the init_date of the reconcile
 * the value is dupplicate in memory (so parameter can be freed after)
 *
 * \param reconcile_number the number of the reconcile
 * \param date the init_date of the reconcile
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_reconcile_set_init_date ( gint reconcile_number,
					    const GDate *date )
{
    struct_reconcile *reconcile;

    reconcile = gsb_data_reconcile_get_structure ( reconcile_number );

    if (!reconcile)
	return FALSE;

    /* we free the last date */
    if ( reconcile -> reconcile_init_date )
	g_date_free (reconcile -> reconcile_init_date);

    /* and copy the new one */
    reconcile -> reconcile_init_date = gsb_date_copy (date);

    return TRUE;
}


/**
 * return the final_date of the reconcile
 *
 * \param reconcile_number the number of the reconcile
 *
 * \return the final_date of the reconcile or NULL if problem
 * */
const GDate *gsb_data_reconcile_get_final_date ( gint reconcile_number )
{
    struct_reconcile *reconcile;

    reconcile = gsb_data_reconcile_get_structure ( reconcile_number );

    if (!reconcile)
	return NULL;

    return reconcile -> reconcile_final_date;
}


/**
 * set the final_date of the reconcile
 * the value is dupplicate in memory (so parameter can be freed after)
 *
 * \param reconcile_number the number of the reconcile
 * \param date the final_date of the reconcile
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_reconcile_set_final_date ( gint reconcile_number,
                        const GDate *date )
{
    struct_reconcile *reconcile;

    reconcile = gsb_data_reconcile_get_structure ( reconcile_number );

    if (!reconcile)
	return FALSE;

    /* we free the last date */
    if ( reconcile -> reconcile_final_date )
        g_date_free (reconcile -> reconcile_final_date);

    /* and copy the new one */
    reconcile -> reconcile_final_date = gsb_date_copy (date);

    /* retrie la liste */
    reconcile_list = g_list_sort ( reconcile_list, (GCompareFunc) gsb_data_reconcile_cmp_int );

    return TRUE;
}


/**
 * return the init_amount of the reconcile
 *
 * \param reconcile_number the number of the reconcile
 *
 * \return the init_amount of the reconcile or null_real if problem
 * */
gsb_real gsb_data_reconcile_get_init_balance ( gint reconcile_number )
{
    struct_reconcile *reconcile;

    reconcile = gsb_data_reconcile_get_structure ( reconcile_number );

    if (!reconcile)
	return null_real;

    return reconcile -> reconcile_init_balance;
}


/**
 * set the init_amount of the reconcile
 *
 * \param reconcile_number the number of the reconcile
 * \param amount the init_amount of the reconcile
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_reconcile_set_init_balance ( gint reconcile_number,
					       gsb_real amount )
{
    struct_reconcile *reconcile;

    reconcile = gsb_data_reconcile_get_structure ( reconcile_number );

    if (!reconcile)
	return FALSE;

    reconcile -> reconcile_init_balance = amount;

    return TRUE;
}


/**
 * return the final_amount of the reconcile
 *
 * \param reconcile_number the number of the reconcile
 *
 * \return the final balance of the reconcile or null_real if problem
 * */
gsb_real gsb_data_reconcile_get_final_balance ( gint reconcile_number )
{
    struct_reconcile *reconcile;

    reconcile = gsb_data_reconcile_get_structure ( reconcile_number );

    if (!reconcile)
	return null_real;

    return reconcile -> reconcile_final_balance;
}


/**
 * set the final_amount of the reconcile
 *
 * \param reconcile_number the number of the reconcile
 * \param amount the final_amount of the reconcile
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_reconcile_set_final_balance ( gint reconcile_number,
						gsb_real amount )
{
    struct_reconcile *reconcile;

    reconcile = gsb_data_reconcile_get_structure ( reconcile_number );

    if (!reconcile)
	return FALSE;

    reconcile -> reconcile_final_balance = amount;

    return TRUE;
}


/**
 * find the last reconcile number for the given account
 *
 * \param account_number
 *
 * \return the last reconcile number for that account, 0 if none found
 * */
gint gsb_data_reconcile_get_account_last_number ( gint account_number )
{
    GList *tmp_list;

    tmp_list = g_list_last ( gsb_data_reconcile_get_sort_reconcile_list (
                                        account_number) );
    if ( tmp_list )
        return GPOINTER_TO_INT ( tmp_list -> data );
    else
        return 0;
}


/**
 * try to find a reconcile wich contains the date given in param
 * for the given account
 *
 * \param date
 * \param account_number
 *
 * \return the number of the found reconcile or 0 if not found
 * */
gint gsb_data_reconcile_get_number_by_date ( const GDate *date,
                        gint account_number )
{
    GList *tmp_list;

    if (!date
	||
	!g_date_valid (date))
	return 0;

    /* check all the reconciles */
    tmp_list = reconcile_list;
    while (tmp_list)
    {
	struct_reconcile *reconcile;

	reconcile = tmp_list -> data;

	if (reconcile -> account_number == account_number
	    &&
	    g_date_compare ( reconcile -> reconcile_init_date,
			     date ) <= 0
	    &&
	    g_date_compare ( date,
			     reconcile -> reconcile_final_date ) <= 0 )
	    return reconcile -> reconcile_number;
	tmp_list = tmp_list -> next;
    }
    return 0;
}


/**
 * renvoie la liste des rapprochements triée par date pour un compte donné
 *
 * \param account_number
 *
 * \return the reconcile list for that account
 * */
GList *gsb_data_reconcile_get_sort_reconcile_list ( gint account_number )
{
    GList *tmp_list;
    GList *rec_list = NULL;
    GList *new_list = NULL;
    struct_reconcile *reconcile;

    /* first we localize the GList struct of that reconcile */
    tmp_list = reconcile_list;
    while (tmp_list)
    {
        reconcile = tmp_list -> data;

        if (reconcile -> account_number == account_number)
        {
            rec_list = g_list_insert_sorted ( rec_list, reconcile, 
                                             (GCompareFunc) gsb_data_reconcile_cmp_int );
        }
	   
        tmp_list = tmp_list -> next;
    }

    tmp_list = rec_list;
    while (tmp_list)
    {
        reconcile = tmp_list -> data;
        
        new_list = g_list_append ( new_list, GINT_TO_POINTER (
                                    reconcile -> reconcile_number) );
        tmp_list = tmp_list -> next;
    }
    return new_list;
}


/**
 * compare deux rapprochements par date
 *
 * \param reconcile_1, reconcile_2
 *
 * \return 0 -1 1 comme strcmp
 * */
gint gsb_data_reconcile_cmp_int (struct_reconcile *reconcile_1,
                        struct_reconcile *reconcile_2)
{
    gint result;

    if ( ! reconcile_1 -> reconcile_final_date )
        return -1;
    else if ( ! reconcile_2 -> reconcile_final_date )
        return 1;
    else
        result = g_date_compare ( reconcile_1 -> reconcile_final_date,
                           reconcile_2 -> reconcile_final_date );
    if (result == 0 )
        return reconcile_1 -> reconcile_number - reconcile_2 -> reconcile_number;
    else
        return result;
}
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
