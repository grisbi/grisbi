/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cédric Auger (cedric@grisbi.org)	          */
/*			2003-2008 Benjamin Drieu (bdrieu@april.org)             	      */
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
 * \file gsb_data_payee_data.c
 * work with the payee structure, no GUI here
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_data_payee.h"
#include "gsb_data_form.h"
#include "gsb_data_report.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_transaction.h"
#include "gsb_form_widget.h"
#include "gtk_combofix.h"
#include "tiers_onglet.h"
#include "gsb_real.h"
#include "utils_str.h"
#include "meta_payee.h"
#include "structures.h"
#include "erreur.h"
/*END_INCLUDE*/


/**
 * \struct 
 * Describe a payee 
 */
typedef struct
{
    guint payee_number;
    gchar *payee_name;
    gchar *payee_description;
    gchar *payee_search_string;

    gint payee_nb_transactions;
    gsb_real payee_balance;
} struct_payee;


/*START_STATIC*/
static void _gsb_data_payee_free ( struct_payee* payee);
static GSList *gsb_data_payee_get_name_list ( void );
static gint gsb_data_payee_get_pointer_from_name_in_glist ( struct_payee *payee,
						     const gchar *name );
static gint gsb_data_payee_max_number ( void );
static void gsb_data_payee_reset_counters ( void );
/*END_STATIC*/

/*START_EXTERN*/
extern gsb_real null_real;
/*END_EXTERN*/

/** contains the g_slist of struct_payee */
static GSList *payee_list = NULL;

/** a pointer to the last payee used (to increase the speed) */
static struct_payee *payee_buffer = NULL;

/** a pointer to a "blank" payee structure, used in the list of payee
 * to group the transactions without payee */
static struct_payee *empty_payee = NULL;


/**
 * set the payees global variables to NULL, usually when we init all the global variables
 *
 * \param none
 *
 * \return FALSE
 * */
gboolean gsb_data_payee_init_variables ( void )
{
    /* free the memory used by the actual list */
    GSList *tmp_list;
    tmp_list = payee_list;
    while ( tmp_list )
    {
	struct_payee *payee;
	payee = tmp_list -> data;
	tmp_list = tmp_list -> next;
	_gsb_data_payee_free ( payee );
    }
    g_slist_free ( payee_list );
    payee_list = NULL;
    payee_buffer = NULL;

    /* create the blank payee */
    if (empty_payee)
    {
        g_free ( empty_payee -> payee_name );
	g_free ( empty_payee );
    }
    empty_payee = g_malloc0 (sizeof ( struct_payee ));
    empty_payee -> payee_name = g_strdup(_("No payee"));
    return FALSE;
}


/**
 * find and return the structure of the payee asked
 *
 * \param no_payee number of payee
 *
 * \return the adr of the struct of the payee (NULL if doesn't exit)
 * */
gpointer gsb_data_payee_get_structure ( gint no_payee )
{
    GSList *tmp;

    if (!no_payee)
	return empty_payee;

    /* before checking all the payees, we check the buffer */

    if ( payee_buffer
	 &&
	 payee_buffer -> payee_number == no_payee )
	return payee_buffer;

    tmp = payee_list;
    
    while ( tmp )
    {
	struct_payee *payee;

	payee = tmp -> data;

	if ( payee -> payee_number == no_payee )
	{
	    payee_buffer = payee;
	    return payee;
	}

	tmp = tmp -> next;
    }
    return NULL;
}

/**
 * return the empty_payee pointer
 *
 * \param
 *
 * \return a pointer to empty_category */
gpointer gsb_data_payee_get_empty_payee ( void )
{
    return gsb_data_payee_get_structure (0);
}


/**
 * give the g_slist of payees structure
 * usefull when want to check all payees
 *
 * \param none
 *
 * \return the g_slist of payees structure
 * */
GSList *gsb_data_payee_get_payees_list ( void )
{
    return payee_list;
}

/**
 * return the number of the payees given in param
 *
 * \param payee_ptr a pointer to the struct of the payee
 *
 * \return the number of the payee, 0 if problem
 * */
gint gsb_data_payee_get_no_payee ( gpointer payee_ptr )
{
    struct_payee *payee;
    
    if ( !payee_ptr )
	return 0;
    
    payee = payee_ptr;
    payee_buffer = payee;
    return payee -> payee_number;
}


/** find and return the last number of payee
 * \param none
 * \return last number of payee
 * */
gint gsb_data_payee_max_number ( void )
{
    GSList *tmp;
    gint number_tmp = 0;

    tmp = payee_list;
    
    while ( tmp )
    {
	struct_payee *payee;

	payee = tmp -> data;

	if ( payee -> payee_number > number_tmp )
	    number_tmp = payee -> payee_number;

	tmp = tmp -> next;
    }
    return number_tmp;
}


/**
 * create a new payee, give him a number, append it to the list
 * and return the number
 *
 * \param name the name of the payee (can be freed after, it's a copy) or NULL
 *
 * \return the number of the new payee
 * */
gint gsb_data_payee_new ( const gchar *name )
{
    struct_payee *payee;

    payee = g_malloc0 ( sizeof ( struct_payee ) );
    payee -> payee_number = gsb_data_payee_max_number () + 1;

    if ( name )
    {
        GtkWidget *combofix;

        payee -> payee_name = my_strdup ( name );
        combofix = gsb_form_widget_get_widget ( TRANSACTION_FORM_PARTY );
    
        if ( combofix && name )
            gtk_combofix_append_text ( GTK_COMBOFIX ( combofix ), name );
    }
    else 
        payee -> payee_name = NULL;

    payee_list = g_slist_append ( payee_list, payee );

    return payee -> payee_number;
}


/**
 * This internal function is called to free the memory used by a struct_payee structure 
 */ 
static void _gsb_data_payee_free ( struct_payee* payee)
{
    if ( ! payee )
        return;
    if ( payee -> payee_name)
        g_free ( payee -> payee_name);
    if ( payee -> payee_description)
        g_free ( payee -> payee_description);
    if ( payee -> payee_search_string)
        g_free ( payee -> payee_search_string);
    g_free ( payee );
    if ( payee_buffer == payee )
	payee_buffer = NULL;
}

/**
 * remove a payee
 * set all the payees of transaction which are this one to 0
 * update combofix and mark file as modified
 *
 * \param no_payee the payee we want to remove
 *
 * \return TRUE ok
 * */
gboolean gsb_data_payee_remove ( gint no_payee )
{
    struct_payee *payee;
    GtkWidget *combofix;

    payee = gsb_data_payee_get_structure ( no_payee );

    if (!payee)
        return FALSE;

    combofix = gsb_form_widget_get_widget ( TRANSACTION_FORM_PARTY );
    if ( combofix )
        gtk_combofix_remove_text ( GTK_COMBOFIX ( combofix ), payee -> payee_name );

    payee_list = g_slist_remove ( payee_list,
				  payee );
    _gsb_data_payee_free (payee);

    return TRUE;
}


/**
 * set a new number for the payee
 * normally used only while loading the file because
 * the number are given automaticly
 *
 * \param no_payee the number of the payee
 * \param new_no_payee the new number of the payee
 *
 * \return the new number or 0 if the payee doen't exist
 * */
gint gsb_data_payee_set_new_number ( gint no_payee,
				     gint new_no_payee )
{
    struct_payee *payee;

    payee = gsb_data_payee_get_structure ( no_payee );

    if (!payee)
	return 0;

    payee -> payee_number = new_no_payee;
    return new_no_payee;
}


/**
 * return the number of the payee wich has the name in param
 * create it if necessary
 *
 * \param name the name of the payee
 * \param create TRUE if we want to create it if it doen't exist
 *
 * \return the number of the payee or 0 if problem
 * */
gint gsb_data_payee_get_number_by_name ( const gchar *name,
                        gboolean create )
{
    GSList *list_tmp;
    gint payee_number = 0;

    list_tmp = g_slist_find_custom ( payee_list,
                        name,
                        (GCompareFunc) gsb_data_payee_get_pointer_from_name_in_glist );
    
    if ( list_tmp )
    {
        struct_payee *payee;
        
        payee = list_tmp -> data;
        payee_number = payee -> payee_number;
    }
    else
    {
        if (create)
        {
            payee_number = gsb_data_payee_new ( name );
        }
    }
    return payee_number;
}


/**
 * used with g_slist_find_custom to find a payee in the g_list
 * by his name
 *
 * \param payee the struct of the current payee checked
 * \param name the name we are looking for
 *
 * \return 0 if it's the same name
 * */
gint gsb_data_payee_get_pointer_from_name_in_glist ( struct_payee *payee,
						     const gchar *name )
{
    return ( my_strcasecmp ( payee -> payee_name, name ) );
}

/**
 * return the name of the payee
 *
 * \param no_payee the number of the payee
 * \param can_return_null if problem, return NULL if TRUE or "No payee" if FALSE
 *
 * \return the name of the payee or NULL/No payee if problem
 * */
const gchar *gsb_data_payee_get_name ( gint no_payee,
				       gboolean can_return_null)
{
    struct_payee *payee;

    payee = gsb_data_payee_get_structure ( no_payee );

    if (!payee || !no_payee)
    {
	if (can_return_null)
	    return NULL;
	else
	    return (_("No payee defined"));
    }

    return payee -> payee_name;
}


/**
 * set the name of the payee
 * the value is dupplicate in memory
 *
 * \param no_payee the number of the payee
 * \param name the name of the payee
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_payee_set_name ( gint no_payee,
				   const gchar *name )
{
    struct_payee *payee;
    GtkWidget *combofix;

    payee = gsb_data_payee_get_structure ( no_payee );

    if (!payee)
        return FALSE;

    combofix = gsb_form_widget_get_widget ( TRANSACTION_FORM_PARTY );

    /* we free the last name */
    if ( payee -> payee_name )
    {
        if ( combofix )
            gtk_combofix_remove_text ( GTK_COMBOFIX ( combofix ), payee -> payee_name );
	    g_free ( payee -> payee_name );
    }
    
    /* and copy the new one or set NULL */
    payee -> payee_name = my_strdup ( name );

    if ( combofix && name && strlen ( name ) )
        gtk_combofix_append_text ( GTK_COMBOFIX ( combofix ), name );

    return TRUE;
}

/**
 * return a g_slist of names of all the payees
 * it's not a copy of the gchar...
 *
 * \param none
 *
 * \return a g_slist of gchar *
 * */
GSList *gsb_data_payee_get_name_list ( void )
{
    GSList *return_list;
    GSList *tmp_list;

    return_list = NULL;
    tmp_list= payee_list;

    while ( tmp_list )
    {
        struct_payee *payee;

        payee = tmp_list -> data;

        if ( payee -> payee_name )
			return_list = g_slist_append ( return_list, payee -> payee_name );

        tmp_list = tmp_list -> next;
    }
    return return_list;
}

/**
 * return a g_slist of names of all the payees and
 * the name of the reports which have to be with the payees
 * it's not a copy of the gchar...
 *
 * \param none
 *
 * \return a g_slist of gchar *
 * */
GSList *gsb_data_payee_get_name_and_report_list ( void )
{
    GSList *return_list;
    GSList *tmp_list;
    GSList *pointer;


    /* for the transactions list, it's a complex type of list, so a g_slist
     * which contains some g_slist of names of payees, one of the 2 g_slist
     * is the selected reports names */

    tmp_list= gsb_data_payee_get_name_list ();
    return_list = NULL;
    return_list = g_slist_append ( return_list,
				   tmp_list );

    /* we append the selected reports */

    tmp_list = NULL;
    pointer = gsb_data_report_get_report_list ();

    while ( pointer )
    {
	gint report_number;

	report_number = gsb_data_report_get_report_number (pointer -> data);

	if ( gsb_data_report_get_append_in_payee (report_number))
	{
	    if ( tmp_list )
		tmp_list = g_slist_append ( tmp_list,
					    g_strconcat ( "\t",
							  my_strdup (gsb_data_report_get_report_name(report_number)),
							  NULL ));
	    else
	    {
		tmp_list = g_slist_append ( tmp_list,
					    g_strdup(_("Report")));
		tmp_list = g_slist_append ( tmp_list,
					    g_strconcat ( "\t",
							  my_strdup (gsb_data_report_get_report_name(report_number)),
							  NULL ));
	    }
	}
	pointer = pointer -> next;
    }

    if (tmp_list)
	return_list = g_slist_append ( return_list,
				       tmp_list );

    return return_list;
}


/**
 * return the description of the payee
 *
 * \param no_payee the number of the payee
 *
 * \return the description of the payee or NULL if problem
 * */
const gchar *gsb_data_payee_get_description ( gint no_payee )
{
    struct_payee *payee;

    payee = gsb_data_payee_get_structure ( no_payee );

    if (!payee)
	return NULL;

    return payee -> payee_description;
}


/**
 * set the description of the payee
 * the value is dupplicate in memory
 *
 * \param no_payee the number of the payee
 * \param description the description of the payee
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_payee_set_description ( gint no_payee,
					  const gchar *description )
{
    struct_payee *payee;

    payee = gsb_data_payee_get_structure ( no_payee );

    if (!payee)
	return FALSE;

    /* we free the last name */
    if ( payee -> payee_description )
	g_free (payee -> payee_description);
    
    /* and copy the new one */
    if (description)
	payee -> payee_description = my_strdup (description);
    else
	payee -> payee_description = NULL;

    return TRUE;
}




/**
 * return nb_transactions of the payee
 *
 * \param no_payee the number of the payee
 *
 * \return nb_transactions of the payee or 0 if problem
 * */
gint gsb_data_payee_get_nb_transactions ( gint no_payee )
{
    struct_payee *payee;

    payee = gsb_data_payee_get_structure ( no_payee );

    if (!payee)
	return 0;

    return payee -> payee_nb_transactions;
}



/**
 * return balance of the payee
 *
 * \param no_payee the number of the payee
 *
 * \return balance of the payee or 0 if problem
 * */
gsb_real gsb_data_payee_get_balance ( gint no_payee )
{
    struct_payee *payee;

    payee = gsb_data_payee_get_structure ( no_payee );

    if (!payee)
	return null_real;

    return payee -> payee_balance;
}


/**
 * reset the counters of the payees
 *
 * \param 
 *
 * \return 
 * */
void gsb_data_payee_reset_counters ( void )
{
    GSList *list_tmp;

    list_tmp = payee_list;

    while ( list_tmp )
    {
	struct_payee *payee;

	payee = list_tmp -> data;
	payee -> payee_balance = null_real;
	payee -> payee_nb_transactions = 0;

	list_tmp = list_tmp -> next;
    }
    
    /* reset the blank payee counters */

    empty_payee -> payee_balance = null_real;
    empty_payee -> payee_nb_transactions = 0;
}

/**
 * update the counters of the payees
 *
 * \param
 *
 * \return
 * */
void gsb_data_payee_update_counters ( void )
{
    GSList *list_tmp_transactions;

    gsb_data_payee_reset_counters ();

    if ( etat.add_archive_in_total_balance )
        list_tmp_transactions = gsb_data_transaction_get_complete_transactions_list ();
    else
        list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

    while ( list_tmp_transactions )
    {
	gint transaction_number_tmp;
	transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);
	
	gsb_data_payee_add_transaction_to_payee ( transaction_number_tmp );

	list_tmp_transactions = list_tmp_transactions -> next;
    }
}


/**
 * add the given transaction to its payee in the counters
 * if the transaction has no payee, add it to the blank payee
 *
 * \param transaction_number the transaction we want to work with
 *
 * \return
 * */
void gsb_data_payee_add_transaction_to_payee ( gint transaction_number )
{
    struct_payee *payee;

    payee = gsb_data_payee_get_structure ( gsb_data_transaction_get_party_number (transaction_number));

    /* if no payee in that transaction, and it's neither a split, neither a transfer,
     * we work with empty_payee */

    /* should not happen, this is if the transaction has a payee wich doesn't exists
     * we show a debug warning and get without payee */
    if (!payee)
    {
	gchar *tmpstr;
	tmpstr = g_strdup_printf ( _("The transaction %d has a payee n°%d but it doesn't exist."),
				   transaction_number, 
				   gsb_data_transaction_get_party_number (transaction_number));
	warning_debug (tmpstr);
	g_free (tmpstr);
	payee = empty_payee;
    }

    payee -> payee_nb_transactions ++;
    if ( ! gsb_data_transaction_get_split_of_transaction ( transaction_number ) )
        payee -> payee_balance = gsb_real_add ( payee -> payee_balance,
					    gsb_data_transaction_get_adjusted_amount_for_currency 
                        ( transaction_number, payee_tree_currency (), -1));
}


/**
 * remove the given transaction to its payee in the counters
 * if the transaction has no payee, remove it to the blank payee
 *
 * \param transaction_number the transaction we want to work with
 *
 * \return
 * */
void gsb_data_payee_remove_transaction_from_payee ( gint transaction_number )
{
    struct_payee *payee;

    payee = gsb_data_payee_get_structure ( gsb_data_transaction_get_party_number (transaction_number));

    /* if no payee in that transaction, and it's neither a split, neither a transfer,
     * we work with empty_payee */

    if (!payee
            &&
            !gsb_data_transaction_get_split_of_transaction (transaction_number)
            && 
            gsb_data_transaction_get_contra_transaction_number (transaction_number) == 0)
        payee = empty_payee;

    if (payee)
    {
        payee -> payee_nb_transactions --;
        payee -> payee_balance = gsb_real_sub ( payee -> payee_balance,
                gsb_data_transaction_get_adjusted_amount_for_currency ( transaction_number,
                    payee_tree_currency (), -1));

        if ( !payee -> payee_nb_transactions ) /* Cope with float errors */
            payee -> payee_balance = null_real;
    }
}


/**
 * remove all the payees wich are not used
 *
 * \param button	the toolbar button
 * \param null
 *
 * \return the number of payees removed
 * */
gint gsb_data_payee_remove_unused ( void )
{
    GSList *tmp_list;
    GSList *used = NULL;
    gint nb_removed = 0;

    /* first we create a list of used categories */
    tmp_list = gsb_data_transaction_get_complete_transactions_list ();
    while (tmp_list)
    {
        gint payee_number;

        payee_number = gsb_data_transaction_get_party_number (
                    gsb_data_transaction_get_transaction_number (tmp_list -> data));
        if (!g_slist_find (used, GINT_TO_POINTER (payee_number)))
        {
            used = g_slist_append ( used, GINT_TO_POINTER (payee_number));
        }
        tmp_list = tmp_list -> next;
    }

    /* it also scans the list of sheduled transactions. fix bug 538 */
    tmp_list = gsb_data_scheduled_get_scheduled_list ();
    while (tmp_list)
    {
        gint payee_number;

        payee_number = gsb_data_scheduled_get_party_number (
                        gsb_data_scheduled_get_scheduled_number (
                        tmp_list -> data));
        if (!g_slist_find (used, GINT_TO_POINTER (payee_number)))
        {
            used = g_slist_append ( used, GINT_TO_POINTER (payee_number));
        }
        tmp_list = tmp_list -> next;
    }

    /* now check each payee to know if it is used */
    tmp_list = gsb_data_payee_get_payees_list ();
    while (tmp_list)
    {
        struct_payee *payee = tmp_list -> data;

        tmp_list = tmp_list -> next;
        if ( !used || !g_slist_find (used, GINT_TO_POINTER (payee -> payee_number)))
        {
            /* payee not used */
            payee_buffer = payee;	/* increase speed */
            gsb_data_payee_remove (payee -> payee_number);
            nb_removed++;
        }
    }
    return nb_removed;
}


const gchar *gsb_data_payee_get_search_string ( gint no_payee )
{
    struct_payee *payee;

    payee = gsb_data_payee_get_structure ( no_payee );

    if (!payee)
        return "";

    if ( payee -> payee_search_string && strlen (payee -> payee_search_string) )
        return payee -> payee_search_string;
    else
        return "";
}

gboolean gsb_data_payee_set_search_string ( gint no_payee, const gchar *search_string )
{
    struct_payee *payee;

    payee = gsb_data_payee_get_structure ( no_payee );

    if (!payee)
        return FALSE;

    /* we free the last name */
    if ( payee -> payee_search_string )
        g_free (payee -> payee_search_string);
    
    /* and copy the new one */
    if (search_string)
        payee -> payee_search_string = my_strdup (search_string);
    else
        payee -> payee_search_string = NULL;

    return TRUE;
}
