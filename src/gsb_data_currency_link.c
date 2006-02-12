
/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2006 Cédric Auger (cedric@grisbi.org)	      */
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
 * \file gsb_data_currency_link.c
 * work with the currency_link structure, no GUI here
 */


#include "include.h"

/*START_INCLUDE*/
#include "gsb_data_currency_link.h"
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
    gdouble change_rate;
} struct_currency_link;

/*START_STATIC*/
static gdouble gsb_data_currency_link_get_change_rate ( gint currency_link_number );
static GSList *gsb_data_currency_link_get_currency_link_list ( void );
static gint gsb_data_currency_link_get_first_currency ( gint currency_link_number );
static gint gsb_data_currency_link_get_no_currency_link ( gpointer currency_link_ptr );
static gint gsb_data_currency_link_get_second_currency ( gint currency_link_number );
static gpointer gsb_data_currency_link_get_structure ( gint currency_link_number );
static gint gsb_data_currency_link_max_number ( void );
static gint gsb_data_currency_link_new ( gint currency_link_number );
static gboolean gsb_data_currency_link_remove ( gint currency_link_number );
static gboolean gsb_data_currency_link_set_ ( gint currency_link_number,
				       gint second_currency );
static gboolean gsb_data_currency_link_set_change_rate ( gint currency_link_number,
						  gdouble change_rate);
static gboolean gsb_data_currency_link_set_first_currency ( gint currency_link_number,
						     gint first_currency );
static gint gsb_data_currency_link_set_new_number ( gint currency_link_number,
					     gint new_no_currency_link );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/** contains the g_slist of struct_currency_link */
static GSList *currency_link_list;

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

    currency_link = calloc ( 1, sizeof ( struct_currency_link ));

    if (currency_link_number)
	currency_link -> currency_link_number = currency_link_number;
    else
	currency_link -> currency_link_number = gsb_data_currency_link_max_number () + 1;

    currency_link_list = g_slist_append ( currency_link_list, currency_link );

    return currency_link -> currency_link_number;
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

    /* remove the currency_link from the buffers */
    if ( currency_link_buffer == currency_link )
	currency_link_buffer = NULL;
    g_free (currency_link);

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
 *
 * \param currency_link_number the number of the currency_link
 * \param second_currency the second_currency of the currency_link
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_currency_link_set_ ( gint currency_link_number,
				       gint second_currency )
{
    struct_currency_link *currency_link;

    currency_link = gsb_data_currency_link_get_structure ( currency_link_number );

    if (!currency_link)
	return FALSE;

    currency_link -> second_currency = second_currency;

    return TRUE;
}


/**
 * return the change_rate of the currency_link
 *
 * \param currency_link_number the number of the currency_link
 *
 * \return the change_rate of the currency_link or 0 if problem
 * */
gdouble gsb_data_currency_link_get_change_rate ( gint currency_link_number )
{
    struct_currency_link *currency_link;

    currency_link = gsb_data_currency_link_get_structure ( currency_link_number );

    if (!currency_link)
	return 0.0;

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
						  gdouble change_rate)
{
    struct_currency_link *currency_link;

    currency_link = gsb_data_currency_link_get_structure ( currency_link_number );

    if (!currency_link)
	return FALSE;

    currency_link -> change_rate = change_rate;

    return TRUE;
}

