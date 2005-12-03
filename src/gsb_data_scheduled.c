/* ************************************************************************** */
/* work with the struct of accounts                                           */
/*                                                                            */
/*                                  gsb_data_scheduled.c                      */
/*                                                                            */
/*     Copyright (C)	2000-2006 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2006 Benjamin Drieu (bdrieu@april.org)	      */
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
 * \file gsb_scheduled_data.c
 * work with the scheduled structure, no GUI here
 */


#include "include.h"


/*START_INCLUDE*/
#include "gsb_data_scheduled.h"
#include "dialog.h"
#include "include.h"
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
    gdouble scheduled_amount;
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
    gint account_number_transfer;			/**< -1 for a scheduled neither categ, neither transfer, neither breakdown */
    gint breakdown_of_scheduled;			/**< 1 if it's a breakdown of scheduled */
    gint mother_scheduled_number;			/**< for a breakdown, the mother's scheduled number */

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
static gint gsb_data_scheduled_get_last_number (void);
static gint gsb_data_scheduled_get_last_white_number (void);
static struct_scheduled *gsb_data_scheduled_get_scheduled_by_no ( gint no_scheduled );
static gboolean gsb_data_scheduled_save_scheduled_pointer ( gpointer scheduled );
/*END_STATIC*/

/*START_EXTERN*/
extern     gchar * buffer ;
/*END_EXTERN*/


/** the g_slist which contains all the scheduleds structures */
static GSList *scheduled_list = NULL;

/** the g_slist which contains all the white scheduleds structures
 * ie : 1 general white line
 * and 1 white line per breakdown of scheduled */
static GSList *white_scheduled_list;

/** 2 pointers to the 2 last scheduled used (to increase the speed) */
static struct_scheduled *scheduled_buffer[2];

/** set the current buffer used */
static gint current_scheduled_buffer;

/** set the scheduleds global variables to NULL, usually when we init all the global variables
 * \param none
 * \return FALSE
 * */
gboolean gsb_data_scheduled_init_variables ( void )
{
    scheduled_buffer[0] = NULL;
    scheduled_buffer[1] = NULL;
    current_scheduled_buffer = 0;
    scheduled_list = NULL;

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
 * so we never return 0 to avoid -1 for a number of white breakdown
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
 * \param no_scheduled
 * 
 * \return a pointer to the scheduled, NULL if not found
 * */
struct_scheduled *gsb_data_scheduled_get_scheduled_by_no ( gint no_scheduled )
{
    GSList *scheduled_list_tmp;

    /* check first if the scheduled is in the buffer */

    if ( scheduled_buffer[0]
	 &&
	 scheduled_buffer[0] -> scheduled_number == no_scheduled )
	return scheduled_buffer[0];

    if ( scheduled_buffer[1]
	 &&
	 scheduled_buffer[1] -> scheduled_number == no_scheduled )
	return scheduled_buffer[1];

    if ( no_scheduled < 0 )
	scheduled_list_tmp = white_scheduled_list;
    else
	scheduled_list_tmp = scheduled_list;

    while ( scheduled_list_tmp )
    {
	struct_scheduled *scheduled;

	scheduled = scheduled_list_tmp -> data;

	if ( scheduled -> scheduled_number == no_scheduled )
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
 * \param no_scheduled the number of the scheduled
 * 
 * \return the account of the scheduled or -1 if problem
 * */
gint gsb_data_scheduled_get_account_number ( gint no_scheduled )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return -1;

    return scheduled -> account_number;
}


/**
 * set the account_number
 * 
 * \param no_scheduled
 * \param no_account
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_account_number ( gint no_scheduled,
						 gint no_account )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return FALSE;

    scheduled -> account_number = no_account;

    return TRUE;
}



/**
 * get the GDate of the scheduled 
 * 
 * \param no_scheduled the number of the scheduled
 * 
 * \return the GDate of the scheduled
 * */
GDate *gsb_data_scheduled_get_date ( gint no_scheduled )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return NULL;

    return scheduled -> date;
}


/** set the GDate of the scheduled
 * \param no_scheduled
 * \param no_account
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_date ( gint no_scheduled,
				       GDate *date )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return FALSE;

    scheduled -> date = date;

    return TRUE;
}



/**
 * get the amount of the scheduled without any currency change
 * (so just get the given amout)
 * 
 * \param no_scheduled the number of the scheduled
 * 
 * \return the amount of the scheduled
 * */
gdouble gsb_data_scheduled_get_amount ( gint no_scheduled )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return 0;

    return scheduled -> scheduled_amount;
}


/**
 * set the amount of the scheduled
 * 
 * \param no_scheduled
 * \param amount
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_amount ( gint no_scheduled,
					 gdouble amount )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return FALSE;

    scheduled -> scheduled_amount = amount;

    return TRUE;
}




/** 
 * get the currency_number 
 * 
 * \param no_scheduled the number of the scheduled
 * 
 * \return the currency number of the scheduled
 * */
gint gsb_data_scheduled_get_currency_number ( gint no_scheduled )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return -1;

    return scheduled -> currency_number;
}


/**
 * set the currency_number
 * 
 * \param no_scheduled
 * \param no_currency
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_currency_number ( gint no_scheduled,
						  gint no_currency )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return FALSE;

    scheduled -> currency_number = no_currency;

    return TRUE;
}




/**
 * get the party_number 
 * 
 * \param no_scheduled the number of the scheduled
 * 
 * \return the currency number of the scheduled
 * */
gint gsb_data_scheduled_get_party_number ( gint no_scheduled )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return -1;

    return scheduled -> party_number;
}


/**
 * set the party_number
 * \param no_scheduled
 * \param value
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_party_number ( gint no_scheduled,
					       gint no_party )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return FALSE;

    scheduled -> party_number = no_party;

    return TRUE;
}



/**
 * get the category_number 
 * 
 * \param no_scheduled the number of the scheduled
 * 
 * \return the category number of the scheduled
 * */
gint gsb_data_scheduled_get_category_number ( gint no_scheduled )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return -1;

    return scheduled -> category_number;
}


/**
 * set the category_number
 * 
 * \param no_scheduled
 * \param value
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_category_number ( gint no_scheduled,
						  gint no_category )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return FALSE;

    scheduled -> category_number = no_category;

    return TRUE;
}


/**
 * get the sub_category_number 
 * 
 * \param no_scheduled the number of the scheduled
 * 
 * \return the sub_category number of the scheduled
 * */
gint gsb_data_scheduled_get_sub_category_number ( gint no_scheduled )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return -1;

    return scheduled -> sub_category_number;
}


/**
 * set the sub_category_number
 * 
 * \param no_scheduled
 * \param value
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_sub_category_number ( gint no_scheduled,
						      gint no_sub_category )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return FALSE;

    scheduled -> sub_category_number = no_sub_category;

    return TRUE;
}


/**
 * get if the scheduled is a breakdown_of_scheduled
 * 
 * \param no_scheduled the number of the scheduled
 * 
 * \return TRUE if the scheduled is a breakdown of scheduled
 * */
gint gsb_data_scheduled_get_breakdown_of_scheduled ( gint no_scheduled )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return -1;

    return scheduled -> breakdown_of_scheduled;
}


/**
 * set if the scheduled is a breakdown_of_scheduled
 * \param no_scheduled
 * \param is_breakdown
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_breakdown_of_scheduled ( gint no_scheduled,
							 gint is_breakdown )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return FALSE;

    scheduled -> breakdown_of_scheduled = is_breakdown;

    return TRUE;
}


/**
 * get the notes
 * 
 * \param no_scheduled the number of the scheduled
 * 
 * \return the notes of the scheduled
 * */
gchar *gsb_data_scheduled_get_notes ( gint no_scheduled )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return NULL;

    return scheduled -> notes;
}


/**
 * set the notes
 * the notes parameter will be copy before stored in memory
 * 
 * \param no_scheduled
 * \param notes a gchar with the new notes
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_notes ( gint no_scheduled,
					const gchar *notes )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return FALSE;

    if ( notes
	 &&
	 strlen (notes))
	scheduled -> notes = g_strdup (notes);
    else
	scheduled -> notes = NULL;

    return TRUE;
}



/**
 * get the method_of_payment_number
 * 
 * \param no_scheduled the number of the scheduled
 * 
 * \return the method_of_payment_number
 * */
gint gsb_data_scheduled_get_method_of_payment_number ( gint no_scheduled )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return -1;

    return scheduled -> method_of_payment_number;
}


/**
 * set the method_of_payment_number
 * 
 * \param no_scheduled
 * \param 
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_method_of_payment_number ( gint no_scheduled,
							   gint number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return FALSE;

    scheduled -> method_of_payment_number = number;

    return TRUE;
}


/**
 * get the method_of_payment_content
 * 
 * \param no_scheduled the number of the scheduled
 * 
 * \return the method_of_payment_content of the scheduled
 * */
gchar *gsb_data_scheduled_get_method_of_payment_content ( gint no_scheduled )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return NULL;

    return scheduled -> method_of_payment_content;
}


/**
 * set the method_of_payment_content
 * dupplicate the parameter before storing it in the scheduled
 * 
 * \param no_scheduled
 * \param method_of_payment_content a gchar with the new method_of_payment_content
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_method_of_payment_content ( gint no_scheduled,
							    const gchar *method_of_payment_content )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return FALSE;

    if ( method_of_payment_content
	 &&
	 strlen (method_of_payment_content))
	scheduled -> method_of_payment_content = g_strdup (method_of_payment_content);
    else
	scheduled -> method_of_payment_content = NULL;

    return TRUE;
}




/**
 * get the automatic_scheduled
 * 
 * \param no_scheduled the number of the scheduled
 * 
 * \return 1 if the scheduled was taken automaticly
 * */
gint gsb_data_scheduled_get_automatic_scheduled ( gint no_scheduled )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return -1;

    return scheduled -> automatic_scheduled;
}


/**
 * set the automatic_scheduled
 * 
 * \param no_scheduled
 * \param  automatic_scheduled
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_automatic_scheduled ( gint no_scheduled,
						      gint automatic_scheduled )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return FALSE;

    scheduled -> automatic_scheduled = automatic_scheduled;

    return TRUE;
}



/**
 * get the financial_year_number
 * 
 * \param no_scheduled the number of the scheduled
 * 
 * \return the financial_year_number
 * */
gint gsb_data_scheduled_get_financial_year_number ( gint no_scheduled )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return -1;

    return scheduled -> financial_year_number;
}


/**
 * set the financial_year_number
 * 
 * \param no_scheduled
 * \param  financial_year_number
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_financial_year_number ( gint no_scheduled,
							gint financial_year_number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return FALSE;

    scheduled -> financial_year_number = financial_year_number;

    return TRUE;
}



/**
 * get the budgetary_number
 * 
 * \param no_scheduled the number of the scheduled
 * 
 * \return the budgetary_number of the scheduled
 * */
gint gsb_data_scheduled_get_budgetary_number ( gint no_scheduled )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return -1;

    return scheduled -> budgetary_number;
}


/**
 * set the budgetary_number
 * 
 * \param no_scheduled
 * \param budgetary_number
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_budgetary_number ( gint no_scheduled,
						   gint budgetary_number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return FALSE;

    scheduled -> budgetary_number = budgetary_number;

    return TRUE;
}


/**
 * get the  sub_budgetary_number
 * 
 * \param no_scheduled the number of the scheduled
 * 
 * \return the sub_budgetary_number number of the scheduled
 * */
gint gsb_data_scheduled_get_sub_budgetary_number ( gint no_scheduled )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return -1;

    return scheduled -> sub_budgetary_number;
}


/**
 * set the sub_budgetary_number
 * 
 * \param no_scheduled
 * \param sub_budgetary_number
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_sub_budgetary_number ( gint no_scheduled,
						       gint sub_budgetary_number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return FALSE;

    scheduled -> sub_budgetary_number = sub_budgetary_number;

    return TRUE;
}



/** 
 * get the  account_number_transfer
 * \param no_scheduled the number of the scheduled
 * 
 * \return the account_number_transfer number of the scheduled
 * */
gint gsb_data_scheduled_get_account_number_transfer ( gint no_scheduled )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return -1;

    return scheduled -> account_number_transfer;
}


/**
 * set the account_number_transfer
 * 
 * \param no_scheduled
 * \param account_number_transfer
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_account_number_transfer ( gint no_scheduled,
							  gint account_number_transfer )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return FALSE;

    scheduled -> account_number_transfer = account_number_transfer;

    return TRUE;
}



/**
 * get the  mother_scheduled_number
 * 
 * \param no_scheduled the number of the scheduled
 * 
 * \return the mother_scheduled_number of the scheduled or 0 if the scheduled doen't exist
 * */
gint gsb_data_scheduled_get_mother_scheduled_number ( gint no_scheduled )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no (no_scheduled);

    if ( !scheduled )
	return 0;

    return scheduled -> mother_scheduled_number;
}


/**
 * set the mother_scheduled_number
 * 
 * \param no_scheduled
 * \param mother_scheduled_number
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_mother_scheduled_number ( gint no_scheduled,
							  gint mother_scheduled_number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return FALSE;

    scheduled -> mother_scheduled_number = mother_scheduled_number;

    return TRUE;
}



/**
 * get the contra_method_of_payment_number
 * 
 * \param no_scheduled the number of the scheduled
 * 
 * \return the contra_method_of_payment_number
 * */
gint gsb_data_scheduled_get_contra_method_of_payment_number ( gint no_scheduled )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return -1;

    return scheduled -> contra_method_of_payment_number;
}


/**
 * set the contra_method_of_payment_number
 * 
 * \param no_scheduled
 * \param 
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_contra_method_of_payment_number ( gint no_scheduled,
								  gint number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return FALSE;

    scheduled -> contra_method_of_payment_number = number;

    return TRUE;
}


/**
 * get the frequency
 * 
 * \param no_scheduled the number of the scheduled
 * 
 * \return the frequency
 * */
gint gsb_data_scheduled_get_frequency ( gint no_scheduled )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return -1;

    return scheduled -> frequency;
}


/**
 * set the frequency
 * 
 * \param no_scheduled
 * \param 
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_frequency ( gint no_scheduled,
					    gint number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return FALSE;

    scheduled -> frequency = number;

    return TRUE;
}


/**
 * get the user_interval
 * 
 * \param no_scheduled the number of the scheduled
 * 
 * \return the user_interval
 * */
gint gsb_data_scheduled_get_user_interval ( gint no_scheduled )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return -1;

    return scheduled -> user_interval;
}


/**
 * set the user_interval
 * 
 * \param no_scheduled
 * \param 
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_user_interval ( gint no_scheduled,
						gint number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return FALSE;

    scheduled -> user_interval = number;

    return TRUE;
}


/**
 * get the user_entry
 * 
 * \param no_scheduled the number of the scheduled
 * 
 * \return the user_entry
 * */
gint gsb_data_scheduled_get_user_entry ( gint no_scheduled )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return -1;

    return scheduled -> user_entry;
}


/**
 * set the user_entry
 * 
 * \param no_scheduled
 * \param 
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_user_entry ( gint no_scheduled,
					     gint number )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return FALSE;

    scheduled -> user_entry = number;

    return TRUE;
}



/**
 * get the limit_GDate of the scheduled 
 * 
 * \param no_scheduled the number of the scheduled
 * 
 * \return the limit_GDate of the scheduled
 * */
GDate *gsb_data_scheduled_get_limit_date ( gint no_scheduled )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return NULL;

    return scheduled -> limit_date;
}


/** set the limit_GDate of the scheduled
 * 
 * \param no_scheduled
 * \param no_account
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_scheduled_set_limit_date ( gint no_scheduled,
					     GDate *date )
{
    struct_scheduled *scheduled;

    scheduled = gsb_data_scheduled_get_scheduled_by_no ( no_scheduled);

    if ( !scheduled )
	return FALSE;

    scheduled -> limit_date = date;

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

    scheduled = calloc ( 1,
			 sizeof ( struct_scheduled ));

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
 * if there is a mother scheduled, it's a breakdown and we increment in the negatives values
 * the number of that line
 * without mother scheduled, it's the general white line, the number is -1
 *
 * if it's a child breakdown, the account is set as for its mother,
 * if it's the last white line, the account is set to -1
 * that scheduled is appended to the white scheduleds list
 * 
 * \param mother_scheduled_number the number of the mother's scheduled if it's a breakdown child ; 0 if not
 *
 * \return the number of the white line
 *
 * */
gint gsb_data_scheduled_new_white_line ( gint mother_scheduled_number)
{
    struct_scheduled *scheduled;

    scheduled = calloc ( 1,
			 sizeof ( struct_scheduled ));

    if ( !scheduled )
    {
	dialogue_error ( _("Cannot allocate memory, bad things will happen soon") );
	/* to avoid more error, return the number of the general white line... */
	return -1;
    }

    /* we fill some things for the child breakdown to help to sort the list */

    if ( mother_scheduled_number )
    {
	scheduled -> scheduled_number = gsb_data_scheduled_get_last_white_number () - 1;
	scheduled -> mother_scheduled_number = mother_scheduled_number;
    }
    else
	scheduled -> scheduled_number = -1;

    white_scheduled_list = g_slist_append ( white_scheduled_list,
					    scheduled );

    gsb_data_scheduled_save_scheduled_pointer (scheduled);

    return scheduled -> scheduled_number;
}



/**
 * remove the scheduled from the scheduled's list
 * free the scheduled
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

    scheduled_list = g_slist_remove ( scheduled_list,
				      scheduled );
    g_free (scheduled);
    return TRUE;
}
