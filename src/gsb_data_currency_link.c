/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2006 Cédric Auger (cedric@grisbi.org)            */
/*          http://www.grisbi.org                                             */
/*                      2009 Pierre Biava (grisbi@pierre.biava.name)          */
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
 * \file gsb_data_currency_link.c
 * work with the currency_link structure, no GUI here
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_data_currency_link.h"
#include "utils_dates.h"
#include "dialog.h"
#include "gsb_real.h"
/*END_INCLUDE*/


/**
 * \struct 
 * Describe a currency_link 
 */
typedef struct
{
    gint currency_link_number;

    gint first_currency;
    gint second_currency;
    gint fixed_link;  /* si = 1 le lien entre devises est fixe */
    gsb_real change_rate;
    GDate *modified_date;

    /* a link is invalid when :
     * - it's a comparison between 2 same currencies
     * - the same link exists before
     * it's set invalid while the configuration */
    gboolean invalid_link;

} struct_currency_link;

/*START_STATIC*/
static void _g_data_currency_link_free ( struct_currency_link *currency_link );
static gboolean gsb_data_currency_link_check_for_invalid ( gint currency_link_number );
static gpointer gsb_data_currency_link_get_structure ( gint currency_link_number );
static gint gsb_data_currency_link_max_number ( void );
/*END_STATIC*/

/*START_EXTERN*/
extern gsb_real null_real;
/*END_EXTERN*/

/** contains the g_slist of struct_currency_link */
static GSList *currency_link_list = NULL;

/** a pointer to the last currency_link used (to increase the speed) */
static struct_currency_link *currency_link_buffer;


/**
 * set the currency_links global variables to NULL, usually when we init all the global variables
 *
 * \param none
 *
 * \return FALSE
 * */
gboolean gsb_data_currency_link_init_variables ( void )
{
    if ( currency_link_list )
    {
        GSList* tmp_list = currency_link_list;
        while ( tmp_list )
        {
	    struct_currency_link *currency_link;
	    currency_link = tmp_list -> data;
	    tmp_list = tmp_list -> next;
            _g_data_currency_link_free ( currency_link );
        }
	g_slist_free ( currency_link_list );
    }
    currency_link_list = NULL;
    currency_link_buffer = NULL;
    return FALSE;
}


/**
 * find and return the structure of the currency_link asked
 *
 * \param currency_link_number number of currency_link
 *
 * \return the adr of the struct of the currency_link (NULL if doesn't exit)
 * */
gpointer gsb_data_currency_link_get_structure ( gint currency_link_number )
{
    GSList *tmp;

    if (!currency_link_number)
	return NULL;

    /* before checking all the currency_links, we check the buffer */

    if ( currency_link_buffer
	 &&
	 currency_link_buffer -> currency_link_number == currency_link_number )
	return currency_link_buffer;

    tmp = currency_link_list;

    while ( tmp )
    {
	struct_currency_link *currency_link;

	currency_link = tmp -> data;

	if ( currency_link -> currency_link_number == currency_link_number )
	{
	    currency_link_buffer = currency_link;
	    return currency_link;
	}
	tmp = tmp -> next;
    }
    return NULL;
}

/**
 * return the number of the currency_link given in param
 *
 * \param currency_link_ptr a pointer to the struct of the currency_link
 *
 * \return the number of the currency_link, 0 if problem
 * */
gint gsb_data_currency_link_get_no_currency_link ( gpointer currency_link_ptr )
{
    struct_currency_link *currency_link;

    if ( !currency_link_ptr )
	return 0;

    currency_link = currency_link_ptr;
    currency_link_buffer = currency_link;
    return currency_link -> currency_link_number;
}


/**
 * give the g_slist of currency_link structure
 * usefull when want to check all currency_links
 *
 * \param none
 *
 * \return the g_slist of currency_links structure
 * */
GSList *gsb_data_currency_link_get_currency_link_list ( void )
{
    return currency_link_list;
}



/**
 * find and return the last number of currency_link
 * 
 * \param none
 * 
 * \return last number of currency_link
 * */
gint gsb_data_currency_link_max_number ( void )
{
    GSList *tmp;
    gint number_tmp = 0;

    tmp = currency_link_list;

    while ( tmp )
    {
	struct_currency_link *currency_link;

	currency_link = tmp -> data;

	if ( currency_link -> currency_link_number > number_tmp )
	    number_tmp = currency_link -> currency_link_number;

	tmp = tmp -> next;
    }
    return number_tmp;
}


/**
 * create a new currency_link, give him a number, append it to the list
 * and return the number
 *
 * \param currency_link_number the number of the new currency link or 0 for automatic
 * 		(should be always automatic, except while loading a file)
 *
 * \return the number of the new currency_link
 * */
gint gsb_data_currency_link_new ( gint currency_link_number )
{
    struct_currency_link *currency_link;

    currency_link = g_malloc0 ( sizeof ( struct_currency_link ));

    if (currency_link_number)
	currency_link -> currency_link_number = currency_link_number;
    else
	currency_link -> currency_link_number = gsb_data_currency_link_max_number () + 1;

    currency_link -> modified_date = gdate_today ( );

    currency_link_list = g_slist_append ( currency_link_list, currency_link );

    return currency_link -> currency_link_number;
}


/**
 * This internal function is called to free the memory used by a struct_currency_link structure.
 */
static void _g_data_currency_link_free ( struct_currency_link *currency_link )
{
    if ( ! currency_link )
        return ;
    if ( currency_link -> modified_date )
        g_date_free ( currency_link -> modified_date );
    g_free (currency_link);
    if ( currency_link_buffer == currency_link )
	currency_link_buffer = NULL;
}

/**
 * remove a currency_link
 * set all the currency_links of transaction which are this one to 0
 * update combofix and mark file as modified
 *
 * \param currency_link_number the currency_link we want to remove
 *
 * \return TRUE ok
 * */
gboolean gsb_data_currency_link_remove ( gint currency_link_number )
{
    struct_currency_link *currency_link;

    currency_link = gsb_data_currency_link_get_structure ( currency_link_number );

    if (!currency_link)
	return FALSE;

    currency_link_list = g_slist_remove ( currency_link_list,
					  currency_link );

    _g_data_currency_link_free ( currency_link );

    return TRUE;
}


/**
 * set a new number for the currency_link
 * normally used only while loading the file because
 * the number are given automaticly
 *
 * \param currency_link_number the number of the currency_link
 * \param new_no_currency_link the new number of the currency_link
 *
 * \return the new number or 0 if the currency_link doen't exist
 * */
gint gsb_data_currency_link_set_new_number ( gint currency_link_number,
                        gint new_no_currency_link )
{
    struct_currency_link *currency_link;

    currency_link = gsb_data_currency_link_get_structure ( currency_link_number );

    if (!currency_link)
	return 0;

    currency_link -> currency_link_number = new_no_currency_link;
    return new_no_currency_link;
}



/**
 * return the first_currency of the currency_link
 *
 * \param currency_link_number the number of the currency_link
 *
 * \return the first_currency of the currency_link or 0 if fail
 * */
gint gsb_data_currency_link_get_first_currency ( gint currency_link_number )
{
    struct_currency_link *currency_link;

    currency_link = gsb_data_currency_link_get_structure ( currency_link_number );

    if (!currency_link)
	return 0;

    return currency_link -> first_currency;
}


/**
 * set the first_currency of the currency_link
 * check and fill the invalid flag
 *
 * \param currency_link_number the number of the currency_link
 * \param first_currency the first_currency of the currency_link
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_currency_link_set_first_currency ( gint currency_link_number,
                        gint first_currency )
{
    struct_currency_link *currency_link;

    currency_link = gsb_data_currency_link_get_structure ( currency_link_number );

    if (!currency_link)
	return FALSE;

    currency_link -> first_currency = first_currency;
    gsb_data_currency_link_check_for_invalid (currency_link_number);

    return TRUE;
}


/**
 * return the second_currency of the currency_link
 *
 * \param currency_link_number the number of the currency_link
 *
 * \return the second_currency of the currency_link or 0 if fail
 * */
gint gsb_data_currency_link_get_second_currency ( gint currency_link_number )
{
    struct_currency_link *currency_link;

    currency_link = gsb_data_currency_link_get_structure ( currency_link_number );

    if (!currency_link)
	return 0;

    return currency_link -> second_currency;
}


/**
 * set the second_currency of the currency_link
 * check and fill the invalid flag
 *
 * \param currency_link_number the number of the currency_link
 * \param second_currency the second_currency of the currency_link
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_currency_link_set_second_currency ( gint currency_link_number,
                        gint second_currency )
{
    struct_currency_link *currency_link;

    currency_link = gsb_data_currency_link_get_structure ( currency_link_number );

    if (!currency_link)
	return FALSE;

    currency_link -> second_currency = second_currency;
    gsb_data_currency_link_check_for_invalid (currency_link_number);

    return TRUE;
}


/**
 * return the change_rate of the currency_link *
 * \param currency_link_number the number of the currency_link
 *
 * \return the change_rate of the currency_link or 0 if problem
 * */
gsb_real gsb_data_currency_link_get_change_rate ( gint currency_link_number )
{
    struct_currency_link *currency_link;

    currency_link = gsb_data_currency_link_get_structure ( currency_link_number );

    if (!currency_link)
	return null_real;

    return currency_link -> change_rate;
}


/**
 * set the change_rate of the currency_link
 *
 * \param currency_link_number the number of the currency_link
 * \param  the  of the currency_link
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_currency_link_set_change_rate ( gint currency_link_number,
                        gsb_real change_rate )
{
    struct_currency_link *currency_link;

    currency_link = gsb_data_currency_link_get_structure ( currency_link_number );

    if (!currency_link)
	return FALSE;

    currency_link -> change_rate = change_rate;

    return TRUE;
}



/**
 * return the invalid_link of the currency_link
 * the flag invalid is set automatickly by gsb_data_currency_link_check_for_invalid
 * when change 1 of the currency
 *
 * \param currency_link_number the number of the currency_link
 *
 * \return TRUE if the link is invalid 
 * */
gint gsb_data_currency_link_get_invalid_link ( gint currency_link_number )
{
    struct_currency_link *currency_link;

    currency_link = gsb_data_currency_link_get_structure ( currency_link_number );

    if (!currency_link)
	return 0;

    return currency_link -> invalid_link;
}



/**
 * return the message error because of the invalid link
 *
 * \param currency_link_number the number of the currency_link
 *
 * \return a const gchar formatted with markup : error the message 
 * */
const gchar *gsb_data_currency_link_get_invalid_message ( gint currency_link_number )
{
    struct_currency_link *currency_link;

    currency_link = gsb_data_currency_link_get_structure ( currency_link_number );

    if (!currency_link)
        return 0;

    if (!currency_link -> invalid_link)
        return NULL;

    if ( currency_link -> first_currency == currency_link -> second_currency)
        return make_red ( _("Warning : the two currencies of the link are identicals.") );

    return make_red ( _("Warning : that link is already defined.") );
}


/**
 * check if the link is invalid and set the flag
 * a link is invalid if :
 * - the 2 currencies are the same
 * - another similar link exists already
 * an invalid link will not be used by grisbi and showed as invalid in the configuration
 *
 * \param currency_link_number the number of the currency_link to check
 *
 * \return TRUE  if invalid, FALSE if not
 * */
gboolean gsb_data_currency_link_check_for_invalid ( gint currency_link_number )
{
    struct_currency_link *currency_link;
    GSList *tmp_list;

    currency_link = gsb_data_currency_link_get_structure ( currency_link_number );

    if (!currency_link)
	return FALSE;

    /* first check : if the 2 currencies are identical */

    if ( currency_link -> first_currency == currency_link -> second_currency)
    {
	currency_link -> invalid_link = TRUE;
	return TRUE;
    }

    /* second check : if that link exists already */

    tmp_list = currency_link_list;

    while (tmp_list)
    {
	struct_currency_link *tmp_currency_link;

	tmp_currency_link = tmp_list -> data;

	if ( tmp_currency_link -> currency_link_number != currency_link -> currency_link_number
	     &&
	     (( tmp_currency_link -> first_currency == currency_link -> first_currency
		&&
		tmp_currency_link -> second_currency == currency_link -> second_currency )
	      ||
	      ( tmp_currency_link -> first_currency == currency_link -> second_currency
		&&
		tmp_currency_link -> second_currency == currency_link -> first_currency )))
	{
	    currency_link -> invalid_link = TRUE;
	    return TRUE;
	}

	tmp_list = tmp_list -> next;
    }

    /* it's ok, the link is valid */
    currency_link -> invalid_link = FALSE;

    return FALSE;
}


/**
 * look for a link between the 2 currencies given in the param
 *
 * \param currency_1 the number of the first currency
 * \param currency_2 the number of the second currency
 *
 * \return the number of the link, 0 if not found, -1 if they are the same currencies
 * */
gint gsb_data_currency_link_search ( gint currency_1,
                        gint currency_2 )
{
    GSList *tmp_list;

    if (!currency_1
	||
	!currency_2 )
	return 0;

    if ( currency_1 == currency_2 )
	return -1;

    tmp_list = currency_link_list;

    while (tmp_list)
    {
	struct_currency_link *tmp_currency_link;

	tmp_currency_link = tmp_list -> data;

	if ( !tmp_currency_link -> invalid_link
	     &&
	     (( tmp_currency_link -> first_currency == currency_1
		&&
		tmp_currency_link -> second_currency == currency_2 )
	      ||
	      ( tmp_currency_link -> first_currency == currency_2
		&&
		tmp_currency_link -> second_currency == currency_1 )))
	    return tmp_currency_link -> currency_link_number;

	tmp_list = tmp_list -> next;
    }
    return 0;
}


/**
 * get the GDate of the currency link 
 * 
 * \param currency_link_number
 * 
 * \return the GDate of the currency link
 * */
GDate *gsb_data_currency_link_get_modified_date ( gint currency_link_number )
{
    struct_currency_link *currency_link;

    currency_link = gsb_data_currency_link_get_structure ( currency_link_number );

    if (!currency_link)
        return NULL;

    return currency_link -> modified_date;
}


/**
 * set the GDate of the currency link
 * 
 * \param currency_link_number
 * \param date
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_currency_link_set_modified_date ( gint currency_link_number,
                        GDate *date )
{
    struct_currency_link *currency_link;

    currency_link = gsb_data_currency_link_get_structure ( currency_link_number );

    if ( !currency_link )
        return FALSE;

    if ( !date )
        return FALSE;

    if ( currency_link -> modified_date )
        g_date_free ( currency_link -> modified_date );

    currency_link -> modified_date = gsb_date_copy ( date );

    return TRUE;
}


/**
 * return the change_link_currency flag
 *
 * \param currency_link_number the number of the currency_link
 *
 * \return TRUE if ok or FALSE if no ok 
 * */
gboolean gsb_data_currency_link_get_fixed_link ( gint currency_link_number )
{
    struct_currency_link *currency_link;

    currency_link = gsb_data_currency_link_get_structure ( currency_link_number );

    if ( !currency_link )
	    return FALSE;

    return currency_link -> fixed_link;
}


/**
 * set the change_link_currency flag
 *
 * \param currency_link_number the number of the currency_link
 * \param  the change_link_currency flag for the currency_link
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_currency_link_set_fixed_link ( gint currency_link_number,
                        gboolean fixed_link )
{
    struct_currency_link *currency_link;

    currency_link = gsb_data_currency_link_get_structure ( currency_link_number );

    if ( !currency_link )
	    return FALSE;

    currency_link -> fixed_link = fixed_link;

    return TRUE;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
