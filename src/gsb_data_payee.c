/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cédric Auger (cedric@grisbi.org)	          */
/*			2003-2008 Benjamin Drieu (bdrieu@april.org)             	      */
/* 			https://www.grisbi.org				                              */
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
 **/

#include "config.h"

#include "include.h"
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "gsb_data_payee.h"
#include "grisbi_win.h"
#include "gsb_combo_box.h"
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
typedef struct _PayeeStruct	PayeeStruct;

struct _PayeeStruct
{
    gint 		payee_number;
    gchar *		payee_name;
    gchar *		payee_description;
    gchar *		payee_search_string;
	gint		ignore_case;
	gint		use_regex;

    gint		payee_nb_transactions;
    GsbReal		payee_balance;
};

/*START_STATIC*/
/** contains the g_slist of PayeeStruct */
static GSList *payee_list = NULL;

/** a pointer to the last payee used (to increase the speed) */
static PayeeStruct *payee_buffer = NULL;

/** a pointer to a "blank" payee structure, used in the list of payee
 * to group the transactions without payee */
static PayeeStruct *empty_payee = NULL;
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * This internal function is called to free the memory used by a PayeeStruct structure
 *
 * \param
 *
 * \return
 **/
static void _gsb_data_payee_free (PayeeStruct* payee)
{
    if (! payee)
        return;
    if (payee->payee_name)
        g_free (payee->payee_name);
    if (payee->payee_description)
        g_free (payee->payee_description);
    if (payee->payee_search_string)
        g_free (payee->payee_search_string);
    g_free (payee);
    if (payee_buffer == payee)
		payee_buffer = NULL;
}

/**
 * compare deux structures payees par le numéro de tiers
 *
 * \param	structure 1
 * \param	structure 1
 *
 * \return -1, 0 ou 1 en fonction du résultat de la comparaison.
 **/
static gint gsb_data_payee_get_pointer_in_gslist (PayeeStruct *payee1,
												  PayeeStruct *payee2)
{
    return (payee1->payee_number - payee2->payee_number);
}

/**
 * return a g_slist of names of all the payees
 * it's not a copy of the gchar...
 *
 * \param none
 *
 * \return a g_slist of gchar *
 **/
static GSList *gsb_data_payee_get_name_list (void)
{
    GSList *return_list;
    GSList *tmp_list;
	GrisbiWinEtat *w_etat;

	w_etat = grisbi_win_get_w_etat ();

	return_list = NULL;
	if (w_etat->metatree_unarchived_payees)
	{
		tmp_list = gsb_data_payee_get_unarchived_payees_list ();
	}
	else
	{
		tmp_list= payee_list;
	}
	while (tmp_list)
	{
		PayeeStruct *payee;

		payee = tmp_list->data;

		if (payee->payee_name)
			return_list = g_slist_append (return_list, payee->payee_name);

		tmp_list = tmp_list->next;
	}
	return_list = g_slist_sort (return_list, (GCompareFunc) my_strcasecmp);

	return return_list;
}

/**
 * used with g_slist_find_custom to find a payee in the g_list
 * by his name
 *
 * \param payee the struct of the current payee checked
 * \param name the name we are looking for
 *
 * \return 0 if it's the same name
 **/
static gint gsb_data_payee_get_pointer_from_name_in_glist (PayeeStruct *payee,
														   const gchar *name)
{
    return (my_strcasecmp (payee->payee_name, name));
}

/** find and return the last number of payee
 *
 * \param none
 *
 * \return last number of payee
 **/
static gint gsb_data_payee_max_number (void)
{
    GSList *tmp;
    gint number_tmp = 0;

    tmp = payee_list;
    while (tmp)
    {
		PayeeStruct *payee;

		payee = tmp->data;

		if (payee->payee_number > number_tmp)
			number_tmp = payee->payee_number;

		tmp = tmp->next;
    }
    return number_tmp;
}

/**
 * reset the counters of the payees
 *
 * \param
 *
 * \return
 **/
static void gsb_data_payee_reset_counters (void)
{
    GSList *list_tmp;

    list_tmp = payee_list;
    while (list_tmp)
    {
		PayeeStruct *payee;

		payee = list_tmp->data;
		payee->payee_balance = null_real;
		payee->payee_nb_transactions = 0;

		list_tmp = list_tmp->next;
    }

    /* reset the blank payee counters */

    empty_payee->payee_balance = null_real;
    empty_payee->payee_nb_transactions = 0;
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 * set the payees global variables to NULL, usually when we init all the global variables
 *
 * \param none
 *
 * \return FALSE
 **/
gboolean gsb_data_payee_init_variables (gboolean cleanup)
{
    /* free the memory used by the actual list */
    GSList *tmp_list;

	tmp_list = payee_list;
    while (tmp_list)
    {
		PayeeStruct *payee;
		payee = tmp_list->data;
		tmp_list = tmp_list->next;
		_gsb_data_payee_free (payee);
    }
    g_slist_free (payee_list);
	if (cleanup)
	{
		payee_list = NULL;
		payee_buffer = NULL;

		/* create the blank payee */
		if (empty_payee)
		{
			g_free (empty_payee->payee_name);
			g_free (empty_payee);
		}
		empty_payee = g_malloc0 (sizeof (PayeeStruct));
		empty_payee->payee_name = g_strdup(_("No payee"));
	}

	return FALSE;
}

/**
 * find and return the structure of the payee asked
 *
 * \param no_payee number of payee
 *
 * \return the adr of the struct of the payee (NULL if doesn't exit)
 **/
gpointer gsb_data_payee_get_structure (gint no_payee)
{
    GSList *tmp;

    if (!no_payee)
		return empty_payee;

    /* before checking all the payees, we check the buffer */
    if (payee_buffer && payee_buffer->payee_number == no_payee)
		return payee_buffer;

    tmp = payee_list;
    while (tmp)
    {
		PayeeStruct *payee;

		payee = tmp->data;
		if (payee->payee_number == no_payee)
		{
			payee_buffer = payee;
			return payee;
		}

		tmp = tmp->next;
    }
    return NULL;
}

/**
 * return the empty_payee pointer
 *
 * \param
 *
 * \return a pointer to empty_category */
gpointer gsb_data_payee_get_empty_payee (void)
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
 **/
GSList *gsb_data_payee_get_payees_list (void)
{
    return payee_list;
}

/**
 * return the number of the payees given in param
 *
 * \param payee_ptr a pointer to the struct of the payee
 *
 * \return the number of the payee, 0 if problem
 **/
gint gsb_data_payee_get_no_payee (gpointer payee_ptr)
{
    PayeeStruct *payee;

    if (!payee_ptr)
		return 0;

    payee = payee_ptr;
    payee_buffer = payee;
    return payee->payee_number;
}



/**
 * create a new payee, give him a number, append it to the list
 * and return the number
 *
 * \param name the name of the payee (can be freed after, it's a copy) or NULL
 *
 * \return the number of the new payee
 **/
gint gsb_data_payee_new (const gchar *name)
{
    PayeeStruct *payee;

    payee = g_malloc0 (sizeof (PayeeStruct));
    payee->payee_number = gsb_data_payee_max_number () + 1;

    if (name)
    {
        GtkWidget *combofix;

        payee->payee_name = my_strdup (name);
        combofix = gsb_form_widget_get_widget (TRANSACTION_FORM_PARTY);

        if (combofix && name)
            gtk_combofix_append_text (GTK_COMBOFIX (combofix), name);
    }
    else
        payee->payee_name = NULL;

    payee_list = g_slist_append (payee_list, payee);

    return payee->payee_number;
}

/**
 * remove a payee
 * set all the payees of transaction which are this one to 0
 * update combofix and mark file as modified
 *
 * \param no_payee the payee we want to remove
 *
 * \return TRUE ok
 **/
gboolean gsb_data_payee_remove (gint no_payee)
{
    PayeeStruct *payee;
    GtkWidget *combofix;

    payee = gsb_data_payee_get_structure (no_payee);

    if (!payee)
        return FALSE;

    combofix = gsb_form_widget_get_widget (TRANSACTION_FORM_PARTY);
    if (combofix)
        gtk_combofix_remove_text (GTK_COMBOFIX (combofix), payee->payee_name);

    payee_list = g_slist_remove (payee_list, payee);
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
 **/
gint gsb_data_payee_set_new_number (gint no_payee,
									gint new_no_payee)
{
    PayeeStruct *payee;

    payee = gsb_data_payee_get_structure (no_payee);

    if (!payee)
		return 0;

    payee->payee_number = new_no_payee;
    return new_no_payee;
}

/**
 * return the number of the payee which has the name in param
 * create it if necessary
 *
 * \param name the name of the payee
 * \param create TRUE if we want to create it if it doen't exist
 *
 * \return the number of the payee or 0 if problem
 **/
gint gsb_data_payee_get_number_by_name (const gchar *name,
										gboolean create)
{
    GSList *list_tmp;
    gint payee_number = 0;

    list_tmp = g_slist_find_custom (payee_list,
									name,
									(GCompareFunc) gsb_data_payee_get_pointer_from_name_in_glist);
    if (list_tmp)
    {
        PayeeStruct *payee;

        payee = list_tmp->data;
        payee_number = payee->payee_number;
    }
    else
    {
        if (create)
        {
            payee_number = gsb_data_payee_new (name);
        }
    }
    return payee_number;
}

/**
 * return the name of the payee
 *
 * \param no_payee the number of the payee
 * \param can_return_null if problem, return NULL if TRUE or "No payee" if FALSE
 *
 * \return the name of the payee or NULL/No payee if problem
 **/
const gchar *gsb_data_payee_get_name (gint no_payee,
									  gboolean can_return_null)
{
    PayeeStruct *payee;

    payee = gsb_data_payee_get_structure (no_payee);

    if (!payee || !no_payee)
    {
		if (can_return_null)
			return NULL;
		else
			return (_("No payee defined"));
    }

    return payee->payee_name;
}

/**
 * set the name of the payee
 * the value is dupplicate in memory
 *
 * \param no_payee the number of the payee
 * \param name the name of the payee
 *
 * \return TRUE if ok or FALSE if problem
 **/
gboolean gsb_data_payee_set_name (gint no_payee,
								  const gchar *name)
{
    PayeeStruct *payee;
    GtkWidget *combofix;

    payee = gsb_data_payee_get_structure (no_payee);

    if (!payee)
        return FALSE;

    combofix = gsb_form_widget_get_widget (TRANSACTION_FORM_PARTY);

    /* we free the last name */
    if (payee->payee_name)
    {
		if (combofix)
			gtk_combofix_remove_text (GTK_COMBOFIX (combofix), payee->payee_name);
		g_free (payee->payee_name);
    }

    /* and copy the new one or set NULL */
    payee->payee_name = my_strdup (name);

    if (combofix && name && strlen (name))
        gtk_combofix_append_text (GTK_COMBOFIX (combofix), name);

    return TRUE;
}

/**
 * compare the name of the payee
 *
 * \param payee_ptr_a a pointer a pointer to the struct of the payee a
 * \param payee_ptr_b a pointer a pointer to the struct of the payee b
 *
 * \return a > b or NULL if problem
 **/
gboolean gsb_data_payee_compare_payees_by_name (gpointer payee_ptr_a,
                                                gpointer payee_ptr_b)
{
    PayeeStruct *payee_a;
    PayeeStruct *payee_b;
    gchar *name_a;
    gchar *name_b;

	if (!payee_ptr_a || !payee_ptr_b)
		return 0;

    payee_a = payee_ptr_a;
    payee_b = payee_ptr_b;
    name_a = payee_a->payee_name;
    name_b = payee_b->payee_name;
    return g_utf8_collate (name_a, name_b);
}

/**
 * return a g_slist of names of all the payees and
 * the name of the reports which have to be with the payees
 * it's not a copy of the gchar...
 *
 * \param none
 *
 * \return a g_slist of gchar *
 **/
GSList *gsb_data_payee_get_name_and_report_list (void)
{
    GSList *return_list = NULL;
    GSList *tmp_list;
    GSList *pointer;

    /* for the transactions list, it's a complex type of list, so a g_slist
     * which contains some g_slist of names of payees, one of the 2 g_slist
     * is the selected reports names */

    tmp_list= gsb_data_payee_get_name_list ();
    return_list = g_slist_append (return_list, tmp_list);

    /* we append the selected reports */
    tmp_list = NULL;
    pointer = gsb_data_report_get_report_list ();
    while (pointer)
    {
		gint report_number;

		report_number = gsb_data_report_get_report_number (pointer->data);

		if (gsb_data_report_get_append_in_payee (report_number))
		{
			gchar *tmp_str;

			tmp_str = my_strdup (gsb_data_report_get_report_name (report_number));
			if (tmp_list)
				tmp_list = g_slist_append (tmp_list, g_strconcat ("\t", tmp_str, NULL));
			else
			{
				tmp_list = g_slist_append (tmp_list, g_strdup (_("Report")));
				tmp_list = g_slist_append (tmp_list, g_strconcat ("\t", tmp_str, NULL));
			}
			g_free (tmp_str);
		}
		pointer = pointer->next;
    }

    if (tmp_list)
        return_list = g_slist_append (return_list, tmp_list);

    return return_list;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void gsb_data_payee_free_name_and_report_list (GSList *liste)
{
    if (g_slist_length (liste) == 2)
    {
        GSList *tmp_list;

        tmp_list = g_slist_nth_data (liste, 1);
        g_slist_free_full (tmp_list, (GDestroyNotify) g_free);
    }
    g_slist_free (liste->data);
    g_slist_free (liste);
}

/**
 * return the description of the payee
 *
 * \param no_payee the number of the payee
 *
 * \return the description of the payee or NULL if problem
 **/
const gchar *gsb_data_payee_get_description (gint no_payee)
{
    PayeeStruct *payee;

    payee = gsb_data_payee_get_structure (no_payee);

    if (!payee)
		return NULL;

    return payee->payee_description;
}

/**
 * set the description of the payee
 * the value is dupplicate in memory
 *
 * \param no_payee the number of the payee
 * \param description the description of the payee
 *
 * \return TRUE if ok or FALSE if problem
 **/
gboolean gsb_data_payee_set_description (gint no_payee,
										 const gchar *description)
{
    PayeeStruct *payee;

    payee = gsb_data_payee_get_structure (no_payee);

    if (!payee)
		return FALSE;

    /* we free the last name */
    if (payee->payee_description)
		g_free (payee->payee_description);

    /* and copy the new one */
    if (description)
		payee->payee_description = my_strdup (description);
    else
		payee->payee_description = NULL;

    return TRUE;
}

/**
 * return nb_transactions of the payee
 *
 * \param no_payee the number of the payee
 *
 * \return nb_transactions of the payee or 0 if problem
 **/
gint gsb_data_payee_get_nb_transactions (gint no_payee)
{
    PayeeStruct *payee;

    payee = gsb_data_payee_get_structure (no_payee);

    if (!payee)
		return 0;

    return payee->payee_nb_transactions;
}

/**
 * return balance of the payee
 *
 * \param no_payee the number of the payee
 *
 * \return balance of the payee or 0 if problem
 **/
GsbReal gsb_data_payee_get_balance (gint no_payee)
{
    PayeeStruct *payee;

    payee = gsb_data_payee_get_structure (no_payee);

    if (!payee)
		return null_real;

    return payee->payee_balance;
}

/**
 * update the counters of the payees
 *
 * \param
 *
 * \return
 **/
void gsb_data_payee_update_counters (void)
{
    GSList *list_tmp_transactions;
	GrisbiWinEtat *w_etat;

	w_etat = grisbi_win_get_w_etat ();
	gsb_data_payee_reset_counters ();

    if (w_etat->metatree_add_archive_in_totals)
        list_tmp_transactions = gsb_data_transaction_get_complete_transactions_list ();
    else
        list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

    while (list_tmp_transactions)
    {
		gint transaction_number_tmp;

		transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions->data);
		gsb_data_payee_add_transaction_to_payee (transaction_number_tmp);

		list_tmp_transactions = list_tmp_transactions->next;
    }
}

/**
 * add the given transaction to its payee in the counters
 * if the transaction has no payee, add it to the blank payee
 *
 * \param transaction_number the transaction we want to work with
 *
 * \return
 **/
void gsb_data_payee_add_transaction_to_payee (gint transaction_number)
{
    PayeeStruct *payee;
	gint contra_number;

	/* if the transaction is a split transaction or a contra transaction don't take it */
	if (gsb_data_transaction_get_mother_transaction_number (transaction_number))
	{
		return;
	}
	else if ((contra_number = gsb_data_transaction_get_contra_transaction_number (transaction_number)) > 0)
	{
		gint tmp_number;

		tmp_number = gsb_data_transaction_get_contra_transaction_number (contra_number);
		if (tmp_number > contra_number)
			return;
	}

	/* if no payee in that transaction and it's neither a split transaction, we work with empty_payee */
    payee = gsb_data_payee_get_structure (gsb_data_transaction_get_payee_number (transaction_number));

    /* should not happen, this is if the transaction has a payee which doesn't exists
     * we show a debug warning and get without payee */
    if (!payee)
    {
        gchar *tmpstr;

        tmpstr = g_strdup_printf ("The transaction %d has a payee %d but it doesn't exist.",
								  transaction_number,
								  gsb_data_transaction_get_payee_number (transaction_number));
        warning_debug (tmpstr);
        g_free (tmpstr);
        payee = empty_payee;
    }

    payee->payee_nb_transactions ++;
	payee->payee_balance = gsb_real_add (payee->payee_balance,
										 gsb_data_transaction_get_adjusted_amount_for_currency
										 (transaction_number, payee_tree_currency (),
										  -1));
}

/**
 * remove the given transaction to its payee in the counters
 * if the transaction has no payee, remove it to the blank payee
 *
 * \param transaction_number the transaction we want to work with
 *
 * \return
 **/
void gsb_data_payee_remove_transaction_from_payee (gint transaction_number)
{
    PayeeStruct *payee;

    payee = gsb_data_payee_get_structure (gsb_data_transaction_get_payee_number (transaction_number));

    /* if no payee in that transaction, and it's neither a split, neither a transfer,
     * we work with empty_payee */

    if (!payee
		&& !gsb_data_transaction_get_split_of_transaction (transaction_number)
		&& gsb_data_transaction_get_contra_transaction_number (transaction_number) == 0)
	{
        payee = empty_payee;
	}

    if (payee)
    {
        payee->payee_nb_transactions --;
        payee->payee_balance = gsb_real_sub (payee->payee_balance,
											 gsb_data_transaction_get_adjusted_amount_for_currency (transaction_number,
																									payee_tree_currency (),
																									-1));

        if (!payee->payee_nb_transactions) /* Cope with float errors */
            payee->payee_balance = null_real;
    }
}

/**
 * remove all the payees which are not used
 *
 * \param button	the toolbar button
 * \param null
 *
 * \return the number of payees removed
 **/
gint gsb_data_payee_remove_unused (void)
{
    GSList *tmp_list;
    GSList *used = NULL;
    gint nb_removed = 0;

	/* first we create a list of used categories */
    tmp_list = gsb_data_transaction_get_complete_transactions_list ();
    while (tmp_list)
    {
		TransactionStruct *transaction;

		transaction = tmp_list->data;
        if (!g_slist_find (used, GINT_TO_POINTER (transaction->party_number)))
        {
            used = g_slist_append (used, GINT_TO_POINTER (transaction->party_number));
        }
        tmp_list = tmp_list->next;
    }

    /* it also scans the list of sheduled transactions. fix bug 538 */
    tmp_list = gsb_data_scheduled_get_scheduled_list ();
    while (tmp_list)
    {
		ScheduledStruct *scheduled;

		scheduled = tmp_list->data;
        if (!g_slist_find (used, GINT_TO_POINTER (scheduled->party_number)))
        {
            used = g_slist_append (used, GINT_TO_POINTER (scheduled->party_number));
        }
        tmp_list = tmp_list->next;
    }

    /* now check each payee to know if it is used */
    tmp_list = gsb_data_payee_get_payees_list ();
    while (tmp_list)
    {
        PayeeStruct *payee = tmp_list->data;

        tmp_list = tmp_list->next;
        if (!used || !g_slist_find (used, GINT_TO_POINTER (payee->payee_number)))
        {
            /* payee not used */
            payee_buffer = payee;	/* increase speed */
            gsb_data_payee_remove (payee->payee_number);
            nb_removed++;
        }
    }

	return nb_removed;
}

/**
 *
 *
 * \param
 *
 * \return search string or an empty string if search string is NULL
 **/
const gchar *gsb_data_payee_get_search_string (gint no_payee)
{
    PayeeStruct *payee;

    payee = gsb_data_payee_get_structure (no_payee);

    if (!payee)
        return "";

    if (payee->payee_search_string && strlen (payee->payee_search_string))
        return payee->payee_search_string;
    else
        return "";
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gboolean gsb_data_payee_set_search_string (gint no_payee, const gchar *search_string)
{
    PayeeStruct *payee;

    payee = gsb_data_payee_get_structure (no_payee);

    if (!payee)
        return FALSE;

    /* we free the last name */
    if (payee->payee_search_string)
        g_free (payee->payee_search_string);

    /* and copy the new one */
    if (search_string)
        payee->payee_search_string = my_strdup (search_string);
    else
        payee->payee_search_string = NULL;

    return TRUE;
}

/**
 * renvoie le nombre de tiers non utilisé (fonction à revoir)
 *
 * \param
 *
 * \return nb_unused
 **/
gint gsb_data_payee_get_unused_payees (void)
{
    GSList *tmp_list;
    GSList *used = NULL;
    gint nb_unused = 0;
	GrisbiWinEtat *w_etat;

	w_etat = grisbi_win_get_w_etat ();

    /* on tient compte de toutes les opérations : méthode courte*/
    if (w_etat->metatree_add_archive_in_totals)
    {
        /* it scans the list of sheduled transactions. fix bug 538 */
        tmp_list = gsb_data_scheduled_get_scheduled_list ();
        while (tmp_list)
        {
            gint payee_number;

            payee_number = gsb_data_scheduled_get_payee_number (
                                gsb_data_scheduled_get_scheduled_number (
                                tmp_list->data));
            if (!g_slist_find (used, GINT_TO_POINTER (payee_number)))
            {
                used = g_slist_append (used, GINT_TO_POINTER (payee_number));
            }
            tmp_list = tmp_list->next;
        }

        /* now check each payee to know if it is used */
        tmp_list = gsb_data_payee_get_payees_list ();
        while (tmp_list)
        {
            PayeeStruct *payee = tmp_list->data;

            tmp_list = tmp_list->next;

            if (payee->payee_nb_transactions == 0
             &&
             (!used
              ||
              !g_slist_find (used, GINT_TO_POINTER (payee->payee_number))))
            {
                /* payee not used */
                nb_unused++;
            }
        }

        return nb_unused;
    }

    /* méthode longue */
    /* first we create a list of used payees */
    tmp_list = gsb_data_transaction_get_complete_transactions_list ();
    while (tmp_list)
    {
        gint payee_number;

        payee_number = gsb_data_transaction_get_payee_number (
                    gsb_data_transaction_get_transaction_number (tmp_list->data));
        if (!g_slist_find (used, GINT_TO_POINTER (payee_number)))
        {
            used = g_slist_append (used, GINT_TO_POINTER (payee_number));
        }
        tmp_list = tmp_list->next;
    }

    /* it also scans the list of sheduled transactions. fix bug 538 */
    tmp_list = gsb_data_scheduled_get_scheduled_list ();
    while (tmp_list)
    {
        gint payee_number;

        payee_number = gsb_data_scheduled_get_payee_number (
                        gsb_data_scheduled_get_scheduled_number (
                        tmp_list->data));
        if (!g_slist_find (used, GINT_TO_POINTER (payee_number)))
        {
            used = g_slist_append (used, GINT_TO_POINTER (payee_number));
        }
        tmp_list = tmp_list->next;
    }

    /* now check each payee to know if it is used */
    tmp_list = gsb_data_payee_get_payees_list ();
    while (tmp_list)
    {
        PayeeStruct *payee = tmp_list->data;

        tmp_list = tmp_list->next;
        if (!used || !g_slist_find (used, GINT_TO_POINTER (payee->payee_number)))
        {
            /* payee not used */
            nb_unused++;
        }
    }
    return nb_unused;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gint gsb_data_payee_get_ignore_case (gint no_payee)
{
    PayeeStruct *payee;

    payee = gsb_data_payee_get_structure (no_payee);

    if (!payee)
		return 0;

    return payee->ignore_case;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gboolean gsb_data_payee_set_ignore_case (gint no_payee,
										 gint ignore_case)
{
    PayeeStruct *payee;

    payee = gsb_data_payee_get_structure (no_payee);

    if (!payee)
		return FALSE;

    payee->ignore_case = ignore_case;

	return TRUE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gint gsb_data_payee_get_use_regex (gint no_payee)
{
    PayeeStruct *payee;

    payee = gsb_data_payee_get_structure (no_payee);

    if (!payee)
		return 0;

    return payee->use_regex;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gboolean gsb_data_payee_set_use_regex (gint no_payee,
									   gint use_regex)
{
    PayeeStruct *payee;

    payee = gsb_data_payee_get_structure (no_payee);

    if (!payee)
		return FALSE;

    payee->use_regex = use_regex;

	return TRUE;
}

/**
 * retourne la liste des tiers utilisés par les transactions.
 * Limite le nombre de tiers dans les gtkcombofix pour accelérer
 * l'affichage.
 *
 * \param
 *
 * \return
 **/
GSList *gsb_data_payee_get_unarchived_payees_list (void)
{
	GSList *tmp_list;
	GSList *payees_list = NULL;
	GSList *transactions_list;

	transactions_list = gsb_data_transaction_get_transactions_list ();
	while (transactions_list)
	{
		gint transaction_number;
		gint payee_number;
		PayeeStruct *payee;

		transaction_number = gsb_data_transaction_get_transaction_number (transactions_list->data);
		payee_number = gsb_data_transaction_get_payee_number (transaction_number);
		payee = gsb_data_payee_get_structure (payee_number);

		if (payee)
		{
			tmp_list = g_slist_find_custom (payees_list,
											payee,
											(GCompareFunc) gsb_data_payee_get_pointer_in_gslist);
			if (!tmp_list)
			{
				payees_list = g_slist_append (payees_list, payee);
			}
		}
		transactions_list = transactions_list->next;
	}
	return payees_list;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
GSList *gsb_data_payee_get_search_payee_list (const gchar *text,
											  gboolean ignore_case)
{
	devel_debug (text);
	GSList *list = NULL;
	GSList *pointer;
	pointer = gsb_data_payee_get_payees_list ();
	while (pointer)
	{
		const gchar *name;
		gint payee_number;
		PayeeStruct *payee;

		payee = pointer->data;
		name = payee->payee_name;
		payee_number = payee->payee_number;
		if (ignore_case)
		{
			if (name && utils_str_my_case_strstr (name, text))
			{
				list = g_slist_append (list, GINT_TO_POINTER (payee_number));
			}
		}
		else
		{
			if (name && g_strstr_len (name, -1, text))
			{
				list = g_slist_append (list, GINT_TO_POINTER (payee_number));
			}
		}
		pointer = pointer->next;
	}

	return list;
}
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
