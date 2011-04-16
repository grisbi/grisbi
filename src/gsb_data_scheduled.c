/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
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
 * \file gsb_scheduled_data.c
 * work with the scheduled structure, no GUI here
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"


/*START_INCLUDE*/
#include "gsb_data_scheduled.h"
#include "dialog.h"
#include "gsb_currency.h"
#include "gsb_data_currency.h"
#include "gsb_data_currency_link.h"
#include "gsb_real.h"
#include "utils_dates.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/


/**
 * \struct 
 * Describe a scheduled
 */
typedef struct
{
    /** @name general stuff */
    gint scheduled_number;
    gint account_number;
    gsb_real scheduled_amount;
    gint party_number;
    gchar *notes;
    gshort automatic_scheduled;			/**< 0=manual, 1=automatic (scheduled scheduled) */
    guint financial_year_number;

    /** @name dates of the scheduled */
    GDate *date;

    /** @name currency stuff */
    gint currency_number;

    /** @name category stuff */
    gint category_number;
    gint sub_category_number;
    gint budgetary_number;
    gint sub_budgetary_number;
    gint account_number_transfer;			/**< -1 for a scheduled neither categ, neither transfer, neither split */
    gint split_of_scheduled;			/**< 1 if it's a split of scheduled */
    gint mother_scheduled_number;			/**< for a split, the mother's scheduled number */

    /** @name method of payment */
    gint method_of_payment_number;
    gchar *method_of_payment_content;
    gint contra_method_of_payment_number;

    /** @name specific stuff for scheduled transactions */
    gint frequency;					/**<  0=once, 1=week, 2=month, 3=year, 4=perso */
    gint user_interval;					/**<  0=days, 1=monthes, 2=years */
    gint user_entry;
    GDate *limit_date;
} struct_scheduled;


/*START_STATIC*/
static void _gsb_data_scheduled_free ( struct_scheduled *scheduled );
static void gsb_data_scheduled_delete_all_scheduled ();
static gint gsb_data_scheduled_get_last_number (void);
static gint gsb_data_scheduled_get_last_white_number (void);
static struct_scheduled *gsb_data_scheduled_get_scheduled_by_no ( gint scheduled_number );
static gboolean gsb_data_scheduled_save_scheduled_pointer ( gpointer scheduled );
/*END_STATIC*/

/*START_EXTERN*/
extern gsb_real null_real;
/*END_EXTERN*/


/** the g_slist which contains all the scheduleds structures */
static GSList *scheduled_list = NULL;

/** the g_slist which contains all the white scheduleds structures
 * ie : 1 general white line
 * and 1 white line per split of scheduled */
static GSList *white_scheduled_list = NULL;

/** 2 pointers to the 2 last scheduled used (to increase the speed) */
static struct_scheduled *scheduled_buffer[2];

/** set the current buffer used */
static gint current_scheduled_buffer;

/**
 * Delete all scheduled and clear memory used by them
 */
void gsb_data_scheduled_delete_all_scheduled ()
{
    if ( scheduled_list )
    {
        GSList *tmp_list;
        tmp_list = scheduled_list;
        while (tmp_list)
        {
            struct_scheduled *scheduled;

            scheduled = tmp_list -> data;
            tmp_list = tmp_list -> next;
                _gsb_data_scheduled_free ( scheduled );
        } 
        g_slist_free ( scheduled_list );
        scheduled_list = NULL;
    }
}

/** set the scheduleds global variables to NULL, usually when we init all the global variables
 * \param none
 * \return FALSE
 * */
gboolean gsb_data_scheduled_init_variables ( void )
{
    gsb_data_scheduled_delete_all_scheduled ();
    return FALSE;
}


/** 
 * return a pointer to the g_slist of scheduleds structure
 * it's not a copy, so we must not free or change it
 * if we want to change something, use gsb_data_scheduled_copy_scheduled_list instead
 *
 * \param none
 * \return the slist of scheduleds structures
 * */
GSList *gsb_data_scheduled_get_scheduled_list ( void )
{
    return scheduled_list;
}


/**
 * find the last number of scheduled
 * 
 * \param none
 * 
 * \return the number
 * */
gint gsb_data_scheduled_get_last_number (void)
{
    gint last_number = 0;
    GSList *scheduled_list_tmp;

    scheduled_list_tmp = scheduled_list;

    while (scheduled_list_tmp)
    {
	struct_scheduled *scheduled;

	scheduled = scheduled_list_tmp -> data;
	if ( scheduled -> scheduled_number > last_number )
	    last_number = scheduled -> scheduled_number;

	scheduled_list_tmp = scheduled_list_tmp -> next;
    }
    return last_number;
}


/**
 * find the last number of the white lines
 * all the white lines have a number < 0, and it always exists at least
 * one line, which number -1 which is the general white line (without mother)
 * so we never return 0 to avoid -1 for a number of white split
 * 
 * \param none
 *
 * \return the number
 * */
gint gsb_data_scheduled_get_last_white_number (void)
{
    gint last_number = 0;
    GSList *scheduled_list_tmp;

    scheduled_list_tmp = white_scheduled_list;

    while (scheduled_list_tmp)
    {
	struct_scheduled *scheduled;

	scheduled = scheduled_list_tmp -> data;
	if ( scheduled -> scheduled_number < last_number )
	    last_number = scheduled -> scheduled_number;

	scheduled_list_tmp = scheduled_list_tmp -> next;
    }

    /* if the general white line has not been appened already, we
     * return -1 to keep that number for the general white line
     * (the number will be decreased after to numbered the new line) */

    if ( !last_number )
	last_number = -1;

    return last_number;
}

/**
 * get the number of the scheduled and save the pointer in the buffer
 * which will increase the speed later
 * 
 * \param scheduled a pointer to a scheduled
 * 
 * \return the number of the scheduled
 * */
gint gsb_data_scheduled_get_scheduled_number ( gpointer scheduled_pointer )
{
    struct_scheduled *scheduled;

    scheduled = scheduled_pointer;

    if ( !scheduled )
	return 0;

    /* if we want the scheduled number, usually it's to make other stuff after that
     * so we will save the adr of the scheduled to increase the speed after */

    gsb_data_scheduled_save_scheduled_pointer ( scheduled );

    return scheduled -> scheduled_number;
}


/**
 * save the pointer in a buffer to increase the speed later
 * 
 * \param scheduled the pointer to the scheduled
 * 
 * \return TRUE or FALSE if pb
 * */
gboolean gsb_data_scheduled_save_scheduled_pointer ( gpointer scheduled )
{
    /* check if the scheduled isn't already saved */

    if ( scheduled == scheduled_buffer[0]
	 ||
	 scheduled == scheduled_buffer[1] )
	return TRUE;

    current_scheduled_buffer = !current_scheduled_buffer;
    scheduled_buffer[current_scheduled_buffer] = scheduled;
    return TRUE;
}


/**
 * return the scheduled which the number is in the parameter. 
 * the new scheduled is stored in the buffer
 * 
 * \param scheduled_number
 * 
 * \return a pointer to the scheduled, NULL if not found
 * */
struct_scheduled *gsb_data_scheduled_get_scheduled_by_no ( gint scheduled_number )
{
    GSList *scheduled_list_tmp;

    /* check first if the scheduled is in the buffer */

    if ( scheduled_buffer[0]
	 &&
	 scheduled_buffer[0] -> scheduled_number == scheduled_number )
	return scheduled_buffer[0];

    if ( scheduled_buffer[1]
	 &&
	 scheduled_buffer[1] -> scheduled_number == scheduled_number )
	return scheduled_buffer[1];

    if ( scheduled_number < 0 )
	scheduled_list_tmp = white_scheduled_list;
    else
	scheduled_list_tmp = scheduled_list;

    while ( scheduled_list_tmp )
    {
	struct_scheduled *scheduled;

	scheduled = scheduled_list_tmp -> data;

	if ( scheduled -> scheduled_number == scheduled_number )
	{
	    gsb_data_scheduled_save_scheduled_pointer ( scheduled );
	    return scheduled;
	}

	scheduled_list_tmp = scheduled_list_tmp -> next;
    }

    /* here, we didn't find any scheduled with that number */

    return NULL;
}



/**
 * get the account_number
 * 
 * \param scheduled_number the number of the scheduled
 * 
 * \return the account of the scheduled or -1 if problem
 * */
gint gsb_data_scheduled_get_account_number ( gint scheduled_number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return -1;

    return scheduled -> account_number;
}


/**
 * set the account_number
 * if the scheduled has some children, they change too
 * 
 * \param scheduled_number
 * \param no_account
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_account_number ( gint scheduled_number,
						 gint no_account )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return FALSE;

    scheduled -> account_number = no_account;

    /* if the scheduled is a split, change all the children */
    if (scheduled -> split_of_scheduled)
    {
	GSList *tmp_list;
	GSList *save_tmp_list;

	tmp_list = gsb_data_scheduled_get_children (scheduled -> scheduled_number, FALSE);
	save_tmp_list = tmp_list;

	while (tmp_list)
	{
	    scheduled = tmp_list -> data;
	    scheduled -> account_number = no_account;
	    tmp_list = tmp_list -> next;
	}
	g_slist_free (save_tmp_list);
    }

    return TRUE;
}



/**
 * get the GDate of the scheduled 
 * 
 * \param scheduled_number the number of the scheduled
 * 
 * \return the GDate of the scheduled
 * */
GDate *gsb_data_scheduled_get_date ( gint scheduled_number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return NULL;

    return scheduled -> date;
}


/**
 * set the GDate of the scheduled
 * if the scheduled has some children, they change too
 * 
 * \param scheduled_number
 * \param no_account
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_date ( gint scheduled_number,
				       const GDate *date )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return FALSE;

    if (scheduled -> date)
	g_date_free (scheduled -> date);
    scheduled -> date = gsb_date_copy (date);

    /* if the scheduled is a split, change all the children */
    if (scheduled -> split_of_scheduled)
    {
	GSList *tmp_list;
	GSList *save_tmp_list;

	tmp_list = gsb_data_scheduled_get_children (scheduled -> scheduled_number, FALSE);
	save_tmp_list = tmp_list;

	while (tmp_list)
	{
	    scheduled = tmp_list -> data;

	    if (scheduled -> date)
		g_date_free (scheduled -> date);
	    scheduled -> date = gsb_date_copy (date);

	    tmp_list = tmp_list -> next;
	}
	g_slist_free (save_tmp_list);
    }

    return TRUE;
}



/**
 * get the amount of the scheduled without any currency change
 * (so just get the given amout)
 * 
 * \param scheduled_number the number of the scheduled
 * 
 * \return the amount of the scheduled
 * */
gsb_real gsb_data_scheduled_get_amount ( gint scheduled_number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return null_real;

    return scheduled -> scheduled_amount;
}


/**
 * set the amount of the scheduled
 * 
 * \param scheduled_number
 * \param amount
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_amount ( gint scheduled_number,
					 gsb_real amount )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return FALSE;

    scheduled -> scheduled_amount = amount;

    return TRUE;
}




/** 
 * get the currency_number 
 * 
 * \param scheduled_number the number of the scheduled
 * 
 * \return the currency number of the scheduled
 * */
gint gsb_data_scheduled_get_currency_number ( gint scheduled_number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return -1;

    return scheduled -> currency_number;
}


/**
 * set the currency_number
 * if the scheduled has some children, they change too
 * 
 * \param scheduled_number
 * \param no_currency
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_currency_number ( gint scheduled_number,
						  gint no_currency )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return FALSE;

    scheduled -> currency_number = no_currency;

    /* if the scheduled is a split, change all the children */
    if (scheduled -> split_of_scheduled)
    {
	GSList *tmp_list;
	GSList *save_tmp_list;

	tmp_list = gsb_data_scheduled_get_children (scheduled -> scheduled_number, FALSE);
	save_tmp_list = tmp_list;

	while (tmp_list)
	{
	    scheduled = tmp_list -> data;
	    scheduled -> currency_number = no_currency;
	    tmp_list = tmp_list -> next;
	}
	g_slist_free (save_tmp_list);
    }

    return TRUE;
}




/**
 * get the party_number 
 * 
 * \param scheduled_number the number of the scheduled
 * 
 * \return the currency number of the scheduled
 * */
gint gsb_data_scheduled_get_party_number ( gint scheduled_number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return -1;

    return scheduled -> party_number;
}


/**
 * set the party_number
 * if the scheduled has some children, they change too
 * 
 * \param scheduled_number
 * \param value
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_party_number ( gint scheduled_number,
					       gint no_party )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return FALSE;

    scheduled -> party_number = no_party;

    /* if the scheduled is a split, change all the children */
    if (scheduled -> split_of_scheduled)
    {
	GSList *tmp_list;
	GSList *save_tmp_list;

	tmp_list = gsb_data_scheduled_get_children (scheduled -> scheduled_number, FALSE);
	save_tmp_list = tmp_list;

	while (tmp_list)
	{
	    scheduled = tmp_list -> data;
	    scheduled -> party_number = no_party;
	    tmp_list = tmp_list -> next;
	}
	g_slist_free (save_tmp_list);
    }

    return TRUE;
}



/**
 * get the category_number 
 * 
 * \param scheduled_number the number of the scheduled
 * 
 * \return the category number of the scheduled
 * */
gint gsb_data_scheduled_get_category_number ( gint scheduled_number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return 0;

    return scheduled -> category_number;
}


/**
 * set the category_number
 * 
 * \param scheduled_number
 * \param value
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_category_number ( gint scheduled_number,
						  gint no_category )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return FALSE;

    scheduled -> category_number = no_category;

    return TRUE;
}


/**
 * get the sub_category_number 
 * 
 * \param scheduled_number the number of the scheduled
 * 
 * \return the sub_category number of the scheduled
 * */
gint gsb_data_scheduled_get_sub_category_number ( gint scheduled_number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return -1;

    return scheduled -> sub_category_number;
}


/**
 * set the sub_category_number
 * 
 * \param scheduled_number
 * \param value
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_sub_category_number ( gint scheduled_number,
						      gint no_sub_category )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return FALSE;

    scheduled -> sub_category_number = no_sub_category;

    return TRUE;
}


/**
 * get if the scheduled is a split_of_scheduled
 * 
 * \param scheduled_number the number of the scheduled
 * 
 * \return TRUE if the scheduled is a split of scheduled
 * */
gint gsb_data_scheduled_get_split_of_scheduled ( gint scheduled_number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return -1;

    return scheduled -> split_of_scheduled;
}


/**
 * set if the scheduled is a split_of_scheduled
 * \param scheduled_number
 * \param is_split
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_split_of_scheduled ( gint scheduled_number,
							 gint is_split )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return FALSE;

    scheduled -> split_of_scheduled = is_split;

    return TRUE;
}


/**
 * get the notes
 * 
 * \param scheduled_number the number of the scheduled
 * 
 * \return the notes of the scheduled
 * */
gchar *gsb_data_scheduled_get_notes ( gint scheduled_number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return NULL;

    return scheduled -> notes;
}


/**
 * set the notes
 * the notes parameter will be copy before stored in memory
 * 
 * \param scheduled_number
 * \param notes a gchar with the new notes
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_notes ( gint scheduled_number,
					const gchar *notes )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return FALSE;

    if ( scheduled -> notes )
        g_free ( scheduled -> notes );

    if ( notes
	 &&
	 strlen (notes))
	scheduled -> notes = my_strdup (notes);
    else
	scheduled -> notes = NULL;

    return TRUE;
}



/**
 * get the method_of_payment_number
 * 
 * \param scheduled_number the number of the scheduled
 * 
 * \return the method_of_payment_number
 * */
gint gsb_data_scheduled_get_method_of_payment_number ( gint scheduled_number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return -1;

    return scheduled -> method_of_payment_number;
}


/**
 * set the method_of_payment_number
 * if the scheduled has some children, they change too
 * 
 * \param scheduled_number
 * \param 
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_method_of_payment_number ( gint scheduled_number,
							   gint number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return FALSE;

    scheduled -> method_of_payment_number = number;

    /* if the scheduled is a split, change all the children */
    if (scheduled -> split_of_scheduled)
    {
	GSList *tmp_list;
	GSList *save_tmp_list;

	tmp_list = gsb_data_scheduled_get_children (scheduled -> scheduled_number, FALSE);
	save_tmp_list = tmp_list;

	while (tmp_list)
	{
	    scheduled = tmp_list -> data;
	    scheduled -> method_of_payment_number = number;
	    tmp_list = tmp_list -> next;
	}
	g_slist_free (save_tmp_list);
    }

    return TRUE;
}


/**
 * get the method_of_payment_content
 * 
 * \param scheduled_number the number of the scheduled
 * 
 * \return the method_of_payment_content of the scheduled
 * */
gchar *gsb_data_scheduled_get_method_of_payment_content ( gint scheduled_number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return NULL;

    return scheduled -> method_of_payment_content;
}


/**
 * set the method_of_payment_content
 * dupplicate the parameter before storing it in the scheduled
 * 
 * \param scheduled_number
 * \param method_of_payment_content a gchar with the new method_of_payment_content
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_method_of_payment_content ( gint scheduled_number,
							    const gchar *method_of_payment_content )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return FALSE;

    if ( scheduled -> method_of_payment_content )
        g_free ( scheduled -> method_of_payment_content );

    if ( method_of_payment_content
	 &&
	 strlen (method_of_payment_content))
	scheduled -> method_of_payment_content = my_strdup (method_of_payment_content);
    else
	scheduled -> method_of_payment_content = NULL;

    return TRUE;
}




/**
 * get the automatic_scheduled
 * 
 * \param scheduled_number the number of the scheduled
 * 
 * \return 1 if the scheduled was taken automaticly
 * */
gint gsb_data_scheduled_get_automatic_scheduled ( gint scheduled_number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return -1;

    return scheduled -> automatic_scheduled;
}


/**
 * set the automatic_scheduled
 * 
 * \param scheduled_number
 * \param  automatic_scheduled
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_automatic_scheduled ( gint scheduled_number,
						      gint automatic_scheduled )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return FALSE;

    scheduled -> automatic_scheduled = automatic_scheduled;

    return TRUE;
}



/**
 * get the financial_year_number
 * 
 * \param scheduled_number the number of the scheduled
 * 
 * \return the financial_year_number
 * */
gint gsb_data_scheduled_get_financial_year_number ( gint scheduled_number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return -1;

    return scheduled -> financial_year_number;
}


/**
 * set the financial_year_number
 * 
 * \param scheduled_number
 * \param  financial_year_number
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_financial_year_number ( gint scheduled_number,
							gint financial_year_number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return FALSE;

    scheduled -> financial_year_number = financial_year_number;

    return TRUE;
}



/**
 * get the budgetary_number
 * 
 * \param scheduled_number the number of the scheduled
 * 
 * \return the budgetary_number of the scheduled
 * */
gint gsb_data_scheduled_get_budgetary_number ( gint scheduled_number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return -1;

    return scheduled -> budgetary_number;
}


/**
 * set the budgetary_number
 * 
 * \param scheduled_number
 * \param budgetary_number
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_budgetary_number ( gint scheduled_number,
						   gint budgetary_number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return FALSE;

    scheduled -> budgetary_number = budgetary_number;

    return TRUE;
}


/**
 * get the  sub_budgetary_number
 * 
 * \param scheduled_number the number of the scheduled
 * 
 * \return the sub_budgetary_number number of the scheduled
 * */
gint gsb_data_scheduled_get_sub_budgetary_number ( gint scheduled_number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return -1;

    return scheduled -> sub_budgetary_number;
}


/**
 * set the sub_budgetary_number
 * 
 * \param scheduled_number
 * \param sub_budgetary_number
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_sub_budgetary_number ( gint scheduled_number,
						       gint sub_budgetary_number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return FALSE;

    scheduled -> sub_budgetary_number = sub_budgetary_number;

    return TRUE;
}


/**
 * this function check if the scheduled transaction is a transfer or
 * not, to avoid many lines in the source code
 *
 * \param scheduled_number
 *
 * \return TRUE : is a transfer, FALSE : is not a transfer
 * */
gboolean gsb_data_scheduled_is_transfer ( gint scheduled_number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return FALSE;

    /* it's more complex than for a transaction, because no transaction number to transfer,
     * we have only account_number_transfer, and it can be 0...
     * first, need to check if it's -1 => neither transfer, neither categ */

    if (scheduled -> account_number_transfer == -1)
	return FALSE;

    /* next, if account_number_transfer is > 0, it's a transfer */
    if (scheduled -> account_number_transfer > 0)
	return TRUE;
    
    /* ok, now we have an account_number_transfer at 0, so it can be
     * a normal scheduled transactions (with categs), or split */
    if (scheduled -> category_number
	||
	scheduled -> split_of_scheduled)
	return FALSE;
    return TRUE;
}



/** 
 * get the  account_number_transfer
 * \param scheduled_number the number of the scheduled
 * 
 * \return the account_number_transfer number of the scheduled
 * */
gint gsb_data_scheduled_get_account_number_transfer ( gint scheduled_number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return -1;

    return scheduled -> account_number_transfer;
}


/**
 * set the account_number_transfer
 * 
 * \param scheduled_number
 * \param account_number_transfer
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_account_number_transfer ( gint scheduled_number,
							  gint account_number_transfer )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return FALSE;

    scheduled -> account_number_transfer = account_number_transfer;

    return TRUE;
}



/**
 * get the  mother_scheduled_number
 * 
 * \param scheduled_number the number of the scheduled
 * 
 * \return the mother_scheduled_number of the scheduled or 0 if the scheduled doen't exist
 * */
gint gsb_data_scheduled_get_mother_scheduled_number ( gint scheduled_number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no (scheduled_number);

    if ( !scheduled )
	return 0;

    return scheduled -> mother_scheduled_number;
}


/**
 * set the mother_scheduled_number
 * 
 * \param scheduled_number
 * \param mother_scheduled_number
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_mother_scheduled_number ( gint scheduled_number,
							  gint mother_scheduled_number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return FALSE;

    scheduled -> mother_scheduled_number = mother_scheduled_number;

    return TRUE;
}



/**
 * get the contra_method_of_payment_number
 * 
 * \param scheduled_number the number of the scheduled
 * 
 * \return the contra_method_of_payment_number
 * */
gint gsb_data_scheduled_get_contra_method_of_payment_number ( gint scheduled_number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return -1;

    return scheduled -> contra_method_of_payment_number;
}


/**
 * set the contra_method_of_payment_number
 * 
 * \param scheduled_number
 * \param 
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_contra_method_of_payment_number ( gint scheduled_number,
								  gint number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return FALSE;

    scheduled -> contra_method_of_payment_number = number;

    return TRUE;
}


/**
 * get the frequency
 * 
 * \param scheduled_number the number of the scheduled
 * 
 * \return the frequency
 * */
gint gsb_data_scheduled_get_frequency ( gint scheduled_number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return -1;

    return scheduled -> frequency;
}


/**
 * set the frequency
 * 
 * \param scheduled_number
 * \param 
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_frequency ( gint scheduled_number,
					    gint number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return FALSE;

    scheduled -> frequency = number;

    return TRUE;
}


/**
 * get the user_interval
 * 
 * \param scheduled_number the number of the scheduled
 * 
 * \return the user_interval
 * */
gint gsb_data_scheduled_get_user_interval ( gint scheduled_number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return -1;

    return scheduled -> user_interval;
}


/**
 * set the user_interval
 * 
 * \param scheduled_number
 * \param 
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_user_interval ( gint scheduled_number,
						gint number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return FALSE;

    scheduled -> user_interval = number;

    return TRUE;
}


/**
 * get the user_entry
 * 
 * \param scheduled_number the number of the scheduled
 * 
 * \return the user_entry
 * */
gint gsb_data_scheduled_get_user_entry ( gint scheduled_number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return -1;

    return scheduled -> user_entry;
}


/**
 * set the user_entry
 * 
 * \param scheduled_number
 * \param 
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_user_entry ( gint scheduled_number,
					     gint number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return FALSE;

    scheduled -> user_entry = number;

    return TRUE;
}



/**
 * get the limit_GDate of the scheduled 
 * 
 * \param scheduled_number the number of the scheduled
 * 
 * \return the limit_GDate of the scheduled
 * */
GDate *gsb_data_scheduled_get_limit_date ( gint scheduled_number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return NULL;

    return scheduled -> limit_date;
}


/** set the limit_GDate of the scheduled
 * 
 * \param scheduled_number
 * \param no_account
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_limit_date ( gint scheduled_number,
					     const GDate *date )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return FALSE;

    if (scheduled -> limit_date)
	g_date_free (scheduled -> limit_date);

    scheduled -> limit_date = gsb_date_copy (date);

    return TRUE;
}




/**
 * create a new scheduled and append it to the list in the right account
 * set the scheduled number given in param (if no number, give the last number + 1)
 * set the number of the account, the number of the scheduled and the currency number
 * which is by default the currency of the account
 * 
 * \param scheduled_number the number of the scheduled
 * 
 * \return the number of the new scheduled, 0 if problem
 * */
gint gsb_data_scheduled_new_scheduled_with_number ( gint scheduled_number )
{
    struct_scheduled *scheduled;

    scheduled = g_malloc0 ( sizeof ( struct_scheduled ));

    if ( !scheduled )
    {
	dialogue_error_memory ();
	return FALSE;
    }

    if ( !scheduled_number )
	scheduled_number = gsb_data_scheduled_get_last_number () + 1;

    scheduled -> scheduled_number = scheduled_number;

    scheduled_list = g_slist_append ( scheduled_list,
				      scheduled );

    gsb_data_scheduled_save_scheduled_pointer (scheduled);

    return scheduled -> scheduled_number;
}


/**
 * create a new scheduled with gsb_data_scheduled_new_scheduled_with_number
 * but set automatickly the last number
 * 
 * \param 
 * 
 * \return the number of the new scheduled
 * */
gint gsb_data_scheduled_new_scheduled ( void )
{
    return gsb_data_scheduled_new_scheduled_with_number ( gsb_data_scheduled_get_last_number () + 1);
}


/** 
 * create a new white line
 * if there is a mother scheduled, it's a split and we increment in the negatives values
 * the number of that line
 * without mother scheduled, it's the general white line, the number is -1
 *
 * if it's a child split, the account is set as for its mother,
 * if it's the last white line, the account is set to -1
 * that scheduled is appended to the white scheduleds list
 * 
 * \param mother_scheduled_number the number of the mother's scheduled if it's a split child ; 0 if not
 *
 * \return the number of the white line
 *
 * */
gint gsb_data_scheduled_new_white_line ( gint mother_scheduled_number)
{
    struct_scheduled *scheduled;

    scheduled = g_malloc0 ( sizeof ( struct_scheduled ));

    if ( !scheduled )
    {
	dialogue_error ( _("Cannot allocate memory, bad things will happen soon") );
	/* to avoid more error, return the number of the general white line... */
	return -1;
    }

    /* we fill some things for the child split to help to sort the list */

    if ( mother_scheduled_number )
    {
	scheduled -> scheduled_number = gsb_data_scheduled_get_last_white_number () - 1;
	scheduled -> mother_scheduled_number = mother_scheduled_number;
	scheduled -> account_number = gsb_data_scheduled_get_account_number (mother_scheduled_number);
    }
    else
    {
	scheduled -> scheduled_number = -1;
	scheduled -> account_number = -1;
    }

    white_scheduled_list = g_slist_append ( white_scheduled_list,
					    scheduled );

    gsb_data_scheduled_save_scheduled_pointer (scheduled);

    return scheduled -> scheduled_number;
}


/**
 * This internal function is called to free memory used by a struct_scheduled structure.
 */
static void _gsb_data_scheduled_free ( struct_scheduled *scheduled )
{
    if ( ! scheduled )
        return;
    if ( scheduled -> notes )
	g_free ( scheduled -> notes );
    if ( scheduled -> date )
	g_date_free ( scheduled -> date );
    if ( scheduled -> limit_date )
	g_date_free ( scheduled -> limit_date );
    if ( scheduled -> method_of_payment_content )
	g_free ( scheduled -> method_of_payment_content );
    g_free ( scheduled );
    scheduled_buffer[0] = NULL;
    scheduled_buffer[1] = NULL;
    current_scheduled_buffer = 0;
}

/**
 * remove the scheduled from the scheduled's list
 * free the scheduled, if there's some children,
 * remove also the children
 * 
 * \param scheduled_number
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_remove_scheduled ( gint scheduled_number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
	return FALSE;

    if ( scheduled -> split_of_scheduled )
    {
	GSList *list_tmp;

	list_tmp = gsb_data_scheduled_get_children (scheduled_number, FALSE);

	while ( list_tmp )
	{
	    struct_scheduled *scheduled_child;

	    scheduled_child = list_tmp -> data;

	    if ( scheduled_child )
	    {
		scheduled_list = g_slist_remove ( scheduled_list,
						  scheduled_child );
		_gsb_data_scheduled_free ( scheduled_child );
	    }
	    list_tmp = list_tmp -> next;
	}
    }

    scheduled_list = g_slist_remove ( scheduled_list,
				      scheduled );

    _gsb_data_scheduled_free ( scheduled );
    return TRUE;
}


/**
 * find the children of the split given in param and
 * return their numbers or their adress in a GSList
 * the list sould be freed
 *
 * \param scheduled_number a split of scheduled transaction
 * \param return_number TRUE if we want a list of number, FALSE if we want a list of struct adr
 *
 * \return a GSList of the numbers/adress of the children, NULL if no child
 * */
GSList *gsb_data_scheduled_get_children ( gint scheduled_number,
					  gboolean return_number )
{
    struct_scheduled *scheduled;
    GSList *children_list = NULL;
    GSList *tmp_list;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled
	 ||
	 !scheduled -> split_of_scheduled)
	return NULL;

    /* get the normal children */
    tmp_list = scheduled_list;
    while ( tmp_list )
    {
	struct_scheduled *tmp_scheduled;

	tmp_scheduled = tmp_list -> data;

	if ( tmp_scheduled -> mother_scheduled_number == scheduled_number )
	{
	    if (return_number)
		children_list = g_slist_append ( children_list,
						 GINT_TO_POINTER ( tmp_scheduled -> scheduled_number ));
	    else
		children_list = g_slist_append ( children_list,
						 tmp_scheduled);
	}
	tmp_list = tmp_list -> next;
    }

    /* get the white line too */
    tmp_list = white_scheduled_list;
    while ( tmp_list )
    {
	struct_scheduled *tmp_scheduled;

	tmp_scheduled = tmp_list -> data;

	if ( tmp_scheduled -> mother_scheduled_number == scheduled_number )
	{
	    if (return_number)
		children_list = g_slist_append ( children_list,
						 GINT_TO_POINTER ( tmp_scheduled -> scheduled_number ));
	    else
		children_list = g_slist_append ( children_list,
						 tmp_scheduled);
	}
	tmp_list = tmp_list -> next;
    }

    return children_list;
}


/**
 * find the white line corresponding to the scheduled transaction
 * given in param and return the number
 * if the scheduled is not a split, return -1, the general white line
 *
 * \param scheduled_number a split scheduled number
 *
 * \return the number of the white line of the split or -1
 * */
gint gsb_data_scheduled_get_white_line ( gint scheduled_number)
{
    struct_scheduled *scheduled;
    GSList *tmp_list;

    scheduled = gsb_data_scheduled_get_scheduled_by_no (scheduled_number);

    if (!scheduled
	 ||
	 !scheduled -> split_of_scheduled)
       return -1;

    tmp_list = white_scheduled_list;

    while ( tmp_list )
    {
	struct_scheduled *tmp_scheduled;

	tmp_scheduled = tmp_list -> data;

	if ( tmp_scheduled -> mother_scheduled_number == scheduled_number )
	    return tmp_scheduled -> scheduled_number;

	tmp_list = tmp_list -> next;
    }
    return -1;
}


/**
 * get floating point of the currency of the scheduled given
 *
 * \param scheduled_number the number of the scheduled
 *
 * \return the floating_point of currency number of the scheduled
 * */
gint gsb_data_scheduled_get_currency_floating_point ( gint scheduled_number )
{
    struct_scheduled *scheduled;
    gint floating_point;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( !scheduled )
        return -1;
    else
    {
        floating_point = gsb_data_currency_get_floating_point ( scheduled -> currency_number );
        return floating_point;
    }
}


/**
 * get the amount of the scheduled, modified to be ok with the currency
 * given in param 
 * 
 * \param scheduled_number 		the number of the scheduled
 * \param return_currency_number 	the currency we want to adjust the transaction's amount
 * \param return_exponent 		the exponent we want to have for the returned number, or -1 for the exponent of the returned currency
 * 
 * \return the amount of the transaction
 * */
gsb_real gsb_data_scheduled_get_adjusted_amount_for_currency ( gint scheduled_number,
                        gint return_currency_number,
                        gint return_exponent )
{
    struct_scheduled *scheduled;
    gsb_real amount = null_real;
    gint link_number;

    if (return_exponent == -1)
        return_exponent = gsb_data_currency_get_floating_point ( return_currency_number );

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( scheduled_number);

    if ( ! (scheduled && return_currency_number ) )
        return gsb_real_adjust_exponent  ( null_real, return_exponent );

    /* if the transaction currency is the same of the account's one,
     * we just return the transaction's amount */
    if ( scheduled -> currency_number == return_currency_number )
        return gsb_real_adjust_exponent  ( scheduled -> scheduled_amount,
					   return_exponent );

    /* now we can adjust the amount */
    if ( ( link_number = gsb_data_currency_link_search ( scheduled -> currency_number,
							return_currency_number ) ) )
    {
	/* there is a hard link between the transaction currency and the return currency */
        if ( gsb_data_currency_link_get_first_currency ( link_number ) == scheduled -> currency_number )
            amount = gsb_real_mul ( scheduled -> scheduled_amount,
                        gsb_data_currency_link_get_change_rate ( link_number ) );
        else
            amount = gsb_real_div ( scheduled -> scheduled_amount,
                        gsb_data_currency_link_get_change_rate (link_number));

    }
    else if ( return_currency_number > 0 && scheduled -> currency_number > 0 )
    {
        gchar *tmp_str;

        tmp_str = g_strdup ( _("Error: is missing one or more links between currencies.\n"
                        "You need to fix it and start over.") );
        dialogue_error ( tmp_str );

        g_free ( tmp_str );
    }

    return gsb_real_adjust_exponent  ( amount, return_exponent );
}


/**
 * copy the content of a scheduled transaction into the second one
 * the 2 scheduled transactions must exist before
 * only the scheduled_number will be modified in the target transaction
 *
 * \param source_scheduled_number the transaction we want to copy
 * \param target_scheduled_number the transaction we want to fill with the content of the first one
 *
 * \return TRUE if ok, FALSE else
 * */
gboolean gsb_data_scheduled_copy_scheduled ( gint source_scheduled_number,
                        gint target_scheduled_number )
{
    struct_scheduled *source_scheduled;
    struct_scheduled *target_scheduled;

    source_scheduled = gsb_data_scheduled_get_scheduled_by_no ( source_scheduled_number );
    target_scheduled = gsb_data_scheduled_get_scheduled_by_no ( target_scheduled_number );

    if ( !source_scheduled || !target_scheduled )
        return FALSE;

    memcpy ( target_scheduled, source_scheduled, sizeof ( struct_scheduled ) );

    target_scheduled -> scheduled_number = target_scheduled_number;

    /* make a new copy of all the pointers */
    if ( target_scheduled -> notes)
        target_scheduled -> notes = my_strdup ( source_scheduled -> notes );

    if ( target_scheduled -> date)
        target_scheduled -> date = gsb_date_copy ( source_scheduled -> date );

    if ( target_scheduled -> limit_date)
        target_scheduled -> limit_date = gsb_date_copy ( source_scheduled -> limit_date );

    if ( target_scheduled -> method_of_payment_content)
        target_scheduled -> method_of_payment_content = my_strdup (
                        source_scheduled -> method_of_payment_content );
    return TRUE;
}


/**
 * retourne TRUE si la variance de l'opération ventilée == 0
 *
 *
 *
 * */
gboolean gsb_data_scheduled_get_variance ( gint mother_scheduled_number )
{
    GSList *tmp_list;
    gsb_real total_split = null_real;
    gsb_real variance;

    tmp_list = scheduled_list;
    while ( tmp_list )
    {
        struct_scheduled *tmp_scheduled;

        tmp_scheduled = tmp_list -> data;

        if ( tmp_scheduled -> mother_scheduled_number == mother_scheduled_number )
        {
            total_split = gsb_real_add ( total_split,
                        gsb_data_scheduled_get_amount ( tmp_scheduled -> scheduled_number ) );
        }
        tmp_list = tmp_list -> next;
    }

    variance = gsb_real_sub ( gsb_data_scheduled_get_amount ( mother_scheduled_number ), total_split );

    if ( variance.mantissa == 0 )
        return TRUE;
    else
        return FALSE;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
