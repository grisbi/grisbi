/* ************************************************************************** */
/* work with the struct of amount comparison of the reports                   */
/*                                                                            */
/*                                  gsb_data_report_amount_comparison         */
/*                                                                            */
/*     Copyright (C)	2000-2007 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2007 Benjamin Drieu (bdrieu@april.org)	      */
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
 * \file gsb_data_report_amount_comparison.c
 * work with the amount comparison structure, no GUI here
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_data_report_amout_comparison.h"
#include "gsb_real.h"
/*END_INCLUDE*/

/** \struct
 * describe a report amount comparison
 * */

typedef struct
{
    /** @name general stuff */
    gint amount_comparison_number;
    gint report_number;				/**< the associated report */

    /** @name saved values of the structure */
    gint link_to_last_amount_comparison;	/* -1=first comparison, 0=and, 1=or, 2=except */
    gint first_comparison;			/* 0= =, 1= <, 2= <=, 3= >, 4= >=, 5= !=, 6= =0, 7= !=0, 8= >0, 9= <0 */
    gsb_real first_amount;
    gint link_first_to_second_part;		/* 0=and, 1=or, 2=except, 3=none */
    gint second_comparison;			/* 0= =, 1= <, 2= <=, 3= >, 4= >=, 5= != */
    gsb_real second_amount;

    /** @name dynamic values used for the gui */
    gpointer hbox_line;
    gpointer button_link;
    gpointer button_first_comparison;
    gpointer entry_first_amount;
    gpointer button_link_first_to_second_part;
    gpointer hbox_second_part;
    gpointer button_second_comparison;
    gpointer entry_second_amount;
} struct_amount_comparison;


/*START_STATIC*/
static void _gsb_data_report_amount_comparison_free ( struct_amount_comparison *amount_comparison );
static struct_amount_comparison *gsb_data_report_amount_comparison_get_struct_by_no ( gint amount_comparison_number );
static gint gsb_data_report_amount_comparison_max_number ( void );
/*END_STATIC*/

/*START_EXTERN*/
extern gsb_real null_real;
/*END_EXTERN*/



/** contains a g_slist of struct_amount_comparison */
static GSList *amount_comparison_list = NULL;

/** a pointers to the last amount comparison used (to increase the speed) */
static struct_amount_comparison *amount_comparison_buffer;



/**
 * set the amount comparison global variables to NULL, usually when we init all the global variables
 * 
 * \param none
 *
 * \return FALSE
 * */
gboolean gsb_data_report_amount_comparison_init_variables ( void )
{
    if ( amount_comparison_list)
    {
        GSList* tmp_list = amount_comparison_list;
        while ( tmp_list )
        {
	    struct_amount_comparison *amount_comparison;
	    amount_comparison = tmp_list -> data;
	    tmp_list = tmp_list -> next;
	    _gsb_data_report_amount_comparison_free ( amount_comparison );
        }
	g_slist_free ( amount_comparison_list );
    }
    amount_comparison_list = NULL;
    amount_comparison_buffer = NULL;

    return FALSE;
}




/**
 * return a pointer on the amount comparison structure which the number is in the parameter. 
 * that structure is stored in the buffer
 * 
 * \param amount_comparison_number
 * 
 * \return a pointer to the amount_comparison, NULL if not found
 * */
static struct_amount_comparison *gsb_data_report_amount_comparison_get_struct_by_no ( gint amount_comparison_number )
{
    GSList *amount_comparison_list_tmp;

    /* check first if the amount_comparison is in the buffer */

    if ( amount_comparison_buffer
	 &&
	 amount_comparison_buffer -> amount_comparison_number == amount_comparison_number )
	return amount_comparison_buffer;

    amount_comparison_list_tmp = amount_comparison_list;

    while ( amount_comparison_list_tmp )
    {
	struct_amount_comparison *amount_comparison;

	amount_comparison = amount_comparison_list_tmp -> data;

	if ( amount_comparison -> amount_comparison_number == amount_comparison_number )
	{
	    amount_comparison_buffer = amount_comparison;
	    return amount_comparison;
	}

	amount_comparison_list_tmp = amount_comparison_list_tmp -> next;
    }

    /* here, we didn't find any amount_comparison with that number */

    return 0;
}




/** find and return the last number of amount_comparison
 * 
 * \param none
 *
 * \return last number of amount_comparison
 * */
gint gsb_data_report_amount_comparison_max_number ( void )
{
    GSList *tmp;
    gint number_tmp = 0;

    tmp = amount_comparison_list;

    while ( tmp )
    {
	struct_amount_comparison *amount_comparison;

	amount_comparison = tmp -> data;

	if ( amount_comparison -> amount_comparison_number > number_tmp )
	    number_tmp = amount_comparison -> amount_comparison_number;

	tmp = tmp -> next;
    }
    return number_tmp;
}



/**
 * create a new amount_comparison with a number, append it to the list
 * and return the number
 *
 * \param number the number we want to give to that amount_comparison, if 0, give a new number by itself
 *
 * \return the number of the new amount_comparison
 * */
gint gsb_data_report_amount_comparison_new ( gint number )
{
    struct_amount_comparison *amount_comparison;

    amount_comparison = g_malloc0 ( sizeof ( struct_amount_comparison ));

    if ( number )
	amount_comparison -> amount_comparison_number = number;
    else
	amount_comparison -> amount_comparison_number = gsb_data_report_amount_comparison_max_number () + 1;

    amount_comparison_list = g_slist_append ( amount_comparison_list,
					      amount_comparison );

    amount_comparison_buffer = amount_comparison;

    return amount_comparison -> amount_comparison_number;
}

/**
 * This function is called to free the memory used by a struct_amount_comparison structure
 */
static void _gsb_data_report_amount_comparison_free ( struct_amount_comparison *amount_comparison )
{
    if ( ! amount_comparison )
        return;
    g_free ( amount_comparison );
    if ( amount_comparison_buffer == amount_comparison )
	amount_comparison_buffer = NULL;
}

/**
 * remove a amount_comparison
 *
 * \param amount_comparison_number the amount_comparison we want to remove
 *
 * \return TRUE ok
 * */
gboolean gsb_data_report_amount_comparison_remove ( gint amount_comparison_number )
{
    struct_amount_comparison *amount_comparison;

    amount_comparison = gsb_data_report_amount_comparison_get_struct_by_no ( amount_comparison_number );

    if (!amount_comparison)
	return FALSE;

    amount_comparison_list = g_slist_remove ( amount_comparison_list, amount_comparison );
    _gsb_data_report_amount_comparison_free ( amount_comparison );

    return TRUE;
}


/**
 * dupplicate an amount_comparison
 * and return the number
 *
 * \param number the number we want to dup to that amount_comparison
 *
 * \return the number of the new amount_comparison
 * */
gint gsb_data_report_amount_comparison_dup ( gint last_amount_comparison_number )
{
    struct_amount_comparison *amount_comparison;
    struct_amount_comparison *last_amount_comparison;
    gint amount_comparison_number;
    
    last_amount_comparison = gsb_data_report_amount_comparison_get_struct_by_no (last_amount_comparison_number);

    if ( !last_amount_comparison )
	return 0;

    amount_comparison_number = gsb_data_report_amount_comparison_new (0);
    amount_comparison = gsb_data_report_amount_comparison_get_struct_by_no (amount_comparison_number);

    amount_comparison -> report_number = last_amount_comparison -> report_number;
    amount_comparison -> link_to_last_amount_comparison = last_amount_comparison -> link_to_last_amount_comparison;
    amount_comparison -> first_comparison = last_amount_comparison -> first_comparison;
    amount_comparison -> first_amount = last_amount_comparison -> first_amount;
    amount_comparison -> link_first_to_second_part = last_amount_comparison -> link_first_to_second_part;
    amount_comparison -> second_comparison = last_amount_comparison -> second_comparison;
    amount_comparison -> second_amount = last_amount_comparison -> second_amount;

    return amount_comparison_number;
}



/** 
 * set the report_number
 * 
 * \param amount_comparison_number number of the amount_comparison
 * \param report_number
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_amount_comparison_set_report_number ( gint amount_comparison_number,
							       gint report_number)
{
    struct_amount_comparison *amount_comparison;

    amount_comparison = gsb_data_report_amount_comparison_get_struct_by_no (amount_comparison_number);

    if ( !amount_comparison )
	return FALSE;

    amount_comparison -> report_number = report_number;

    return TRUE;
}

/**
 * get the link_to_last_amount_comparison
 * 
 * \param amount_comparison_number the number of the amount_comparison
 *
 * \return the  of the amount_comparison, -1 if problem
 * */
gint gsb_data_report_amount_comparison_get_link_to_last_amount_comparison ( gint amount_comparison_number )
{
    struct_amount_comparison *amount_comparison;

    amount_comparison = gsb_data_report_amount_comparison_get_struct_by_no (amount_comparison_number);

    if ( !amount_comparison )
	return 0;

    return amount_comparison -> link_to_last_amount_comparison;
}

/** 
 * set the link_to_last_amount_comparison
 * 
 * \param amount_comparison_number number of the amount_comparison
 * \param link_to_last_amount_comparison
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_amount_comparison_set_link_to_last_amount_comparison ( gint amount_comparison_number,
										gint link_to_last_amount_comparison)
{
    struct_amount_comparison *amount_comparison;

    amount_comparison = gsb_data_report_amount_comparison_get_struct_by_no (amount_comparison_number);

    if ( !amount_comparison )
	return FALSE;

    amount_comparison -> link_to_last_amount_comparison = link_to_last_amount_comparison;

    return TRUE;
}

/**
 * get the first_comparison
 * 
 * \param amount_comparison_number the number of the amount_comparison
 *
 * \return the  of the amount_comparison, -1 if problem
 * */
gint gsb_data_report_amount_comparison_get_first_comparison ( gint amount_comparison_number )
{
    struct_amount_comparison *amount_comparison;

    amount_comparison = gsb_data_report_amount_comparison_get_struct_by_no (amount_comparison_number);

    if ( !amount_comparison )
	return 0;

    return amount_comparison -> first_comparison;
}

/** 
 * set the first_comparison
 * 
 * \param amount_comparison_number number of the amount_comparison
 * \param first_comparison
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_amount_comparison_set_first_comparison ( gint amount_comparison_number,
								  gint first_comparison)
{
    struct_amount_comparison *amount_comparison;

    amount_comparison = gsb_data_report_amount_comparison_get_struct_by_no (amount_comparison_number);

    if ( !amount_comparison )
	return FALSE;

    amount_comparison -> first_comparison = first_comparison;

    return TRUE;
}


/**
 * get the first_amount
 * 
 * \param amount_comparison_number the number of the amount_comparison
 *
 * \return the  of the amount_comparison, -1 if problem
 * */
gsb_real gsb_data_report_amount_comparison_get_first_amount ( gint amount_comparison_number )
{
    struct_amount_comparison *amount_comparison;

    amount_comparison = gsb_data_report_amount_comparison_get_struct_by_no (amount_comparison_number);

    if ( !amount_comparison )
	return null_real;

    return amount_comparison -> first_amount;
}

/** 
 * set the first_amount
 * 
 * \param amount_comparison_number number of the amount_comparison
 * \param first_amount
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_amount_comparison_set_first_amount ( gint amount_comparison_number,
							      gsb_real first_amount)
{
    struct_amount_comparison *amount_comparison;

    amount_comparison = gsb_data_report_amount_comparison_get_struct_by_no (amount_comparison_number);

    if ( !amount_comparison )
	return FALSE;

    amount_comparison -> first_amount = first_amount;

    return TRUE;
}


/**
 * get the link_first_to_second_part
 * 
 * \param amount_comparison_number the number of the amount_comparison
 *
 * \return the  of the amount_comparison, -1 if problem
 * */
gint gsb_data_report_amount_comparison_get_link_first_to_second_part ( gint amount_comparison_number )
{
    struct_amount_comparison *amount_comparison;

    amount_comparison = gsb_data_report_amount_comparison_get_struct_by_no (amount_comparison_number);

    if ( !amount_comparison )
	return 0;

    return amount_comparison -> link_first_to_second_part;
}

/** 
 * set the link_first_to_second_part
 * 
 * \param amount_comparison_number number of the amount_comparison
 * \param link_first_to_second_part
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_amount_comparison_set_link_first_to_second_part ( gint amount_comparison_number,
									   gint link_first_to_second_part)
{
    struct_amount_comparison *amount_comparison;

    amount_comparison = gsb_data_report_amount_comparison_get_struct_by_no (amount_comparison_number);

    if ( !amount_comparison )
	return FALSE;

    amount_comparison -> link_first_to_second_part = link_first_to_second_part;

    return TRUE;
}


/**
 * get the second_comparison
 * 
 * \param amount_comparison_number the number of the amount_comparison
 *
 * \return the  of the amount_comparison, -1 if problem
 * */
gint gsb_data_report_amount_comparison_get_second_comparison ( gint amount_comparison_number )
{
    struct_amount_comparison *amount_comparison;

    amount_comparison = gsb_data_report_amount_comparison_get_struct_by_no (amount_comparison_number);

    if ( !amount_comparison )
	return 0;

    return amount_comparison -> second_comparison;
}

/** 
 * set the second_comparison
 * 
 * \param amount_comparison_number number of the amount_comparison
 * \param second_comparison
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_amount_comparison_set_second_comparison ( gint amount_comparison_number,
								   gint second_comparison)
{
    struct_amount_comparison *amount_comparison;

    amount_comparison = gsb_data_report_amount_comparison_get_struct_by_no (amount_comparison_number);

    if ( !amount_comparison )
	return FALSE;

    amount_comparison -> second_comparison = second_comparison;

    return TRUE;
}


/**
 * get the second_amount
 * 
 * \param amount_comparison_number the number of the amount_comparison
 *
 * \return the  of the amount_comparison, -1 if problem
 * */
gsb_real gsb_data_report_amount_comparison_get_second_amount ( gint amount_comparison_number )
{
    struct_amount_comparison *amount_comparison;

    amount_comparison = gsb_data_report_amount_comparison_get_struct_by_no (amount_comparison_number);

    if ( !amount_comparison )
	return null_real;

    return amount_comparison -> second_amount;
}

/** 
 * set the second_amount
 * 
 * \param amount_comparison_number number of the amount_comparison
 * \param second_amount
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_amount_comparison_set_second_amount ( gint amount_comparison_number,
							       gsb_real second_amount)
{
    struct_amount_comparison *amount_comparison;

    amount_comparison = gsb_data_report_amount_comparison_get_struct_by_no (amount_comparison_number);

    if ( !amount_comparison )
	return FALSE;

    amount_comparison -> second_amount = second_amount;

    return TRUE;
}


/**
 * get the hbox_line
 * 
 * \param amount_comparison_number the number of the amount_comparison
 *
 * \return the  of the amount_comparison, -1 if problem
 * */
gpointer gsb_data_report_amount_comparison_get_hbox_line ( gint amount_comparison_number )
{
    struct_amount_comparison *amount_comparison;

    amount_comparison = gsb_data_report_amount_comparison_get_struct_by_no (amount_comparison_number);

    if ( !amount_comparison )
	return 0;

    return amount_comparison -> hbox_line;
}

/** 
 * set the hbox_line
 * 
 * \param amount_comparison_number number of the amount_comparison
 * \param hbox_line
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_amount_comparison_set_hbox_line ( gint amount_comparison_number,
							   gpointer hbox_line)
{
    struct_amount_comparison *amount_comparison;

    amount_comparison = gsb_data_report_amount_comparison_get_struct_by_no (amount_comparison_number);

    if ( !amount_comparison )
	return FALSE;

    amount_comparison -> hbox_line = hbox_line;

    return TRUE;
}

/**
 * get the button_link
 * 
 * \param amount_comparison_number the number of the amount_comparison
 *
 * \return the  of the amount_comparison, -1 if problem
 * */
gpointer gsb_data_report_amount_comparison_get_button_link ( gint amount_comparison_number )
{
    struct_amount_comparison *amount_comparison;

    amount_comparison = gsb_data_report_amount_comparison_get_struct_by_no (amount_comparison_number);

    if ( !amount_comparison )
	return 0;

    return amount_comparison -> button_link;
}

/** 
 * set the button_link
 * 
 * \param amount_comparison_number number of the amount_comparison
 * \param button_link
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_amount_comparison_set_button_link ( gint amount_comparison_number,
							     gpointer button_link)
{
    struct_amount_comparison *amount_comparison;

    amount_comparison = gsb_data_report_amount_comparison_get_struct_by_no (amount_comparison_number);

    if ( !amount_comparison )
	return FALSE;

    amount_comparison -> button_link = button_link;

    return TRUE;
}

/**
 * get the button_first_comparison
 * 
 * \param amount_comparison_number the number of the amount_comparison
 *
 * \return the  of the amount_comparison, -1 if problem
 * */
gpointer gsb_data_report_amount_comparison_get_button_first_comparison ( gint amount_comparison_number )
{
    struct_amount_comparison *amount_comparison;

    amount_comparison = gsb_data_report_amount_comparison_get_struct_by_no (amount_comparison_number);

    if ( !amount_comparison )
	return 0;

    return amount_comparison -> button_first_comparison;
}

/** 
 * set the button_first_comparison
 * 
 * \param amount_comparison_number number of the amount_comparison
 * \param button_first_comparison
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_amount_comparison_set_button_first_comparison ( gint amount_comparison_number,
									 gpointer button_first_comparison)
{
    struct_amount_comparison *amount_comparison;

    amount_comparison = gsb_data_report_amount_comparison_get_struct_by_no (amount_comparison_number);

    if ( !amount_comparison )
	return FALSE;

    amount_comparison -> button_first_comparison = button_first_comparison;

    return TRUE;
}

/**
 * get the entry_first_amount
 * 
 * \param amount_comparison_number the number of the amount_comparison
 *
 * \return the  of the amount_comparison, -1 if problem
 * */
gpointer gsb_data_report_amount_comparison_get_entry_first_amount ( gint amount_comparison_number )
{
    struct_amount_comparison *amount_comparison;

    amount_comparison = gsb_data_report_amount_comparison_get_struct_by_no (amount_comparison_number);

    if ( !amount_comparison )
	return 0;

    return amount_comparison -> entry_first_amount;
}

/** 
 * set the entry_first_amount
 * 
 * \param amount_comparison_number number of the amount_comparison
 * \param entry_first_amount
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_amount_comparison_set_entry_first_amount ( gint amount_comparison_number,
								    gpointer entry_first_amount)
{
    struct_amount_comparison *amount_comparison;

    amount_comparison = gsb_data_report_amount_comparison_get_struct_by_no (amount_comparison_number);

    if ( !amount_comparison )
	return FALSE;

    amount_comparison -> entry_first_amount = entry_first_amount;

    return TRUE;
}

/**
 * get the button_link_first_to_second_part
 * 
 * \param amount_comparison_number the number of the amount_comparison
 *
 * \return the  of the amount_comparison, -1 if problem
 * */
gpointer gsb_data_report_amount_comparison_get_button_link_first_to_second_part ( gint amount_comparison_number )
{
    struct_amount_comparison *amount_comparison;

    amount_comparison = gsb_data_report_amount_comparison_get_struct_by_no (amount_comparison_number);

    if ( !amount_comparison )
	return 0;

    return amount_comparison -> button_link_first_to_second_part;
}

/** 
 * set the button_link_first_to_second_part
 * 
 * \param amount_comparison_number number of the amount_comparison
 * \param button_link_first_to_second_part
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_amount_comparison_set_button_link_first_to_second_part ( gint amount_comparison_number,
										  gpointer button_link_first_to_second_part)
{
    struct_amount_comparison *amount_comparison;

    amount_comparison = gsb_data_report_amount_comparison_get_struct_by_no (amount_comparison_number);

    if ( !amount_comparison )
	return FALSE;

    amount_comparison -> button_link_first_to_second_part = button_link_first_to_second_part;

    return TRUE;
}

/**
 * get the hbox_second_part
 * 
 * \param amount_comparison_number the number of the amount_comparison
 *
 * \return the  of the amount_comparison, -1 if problem
 * */
gpointer gsb_data_report_amount_comparison_get_hbox_second_part ( gint amount_comparison_number )
{
    struct_amount_comparison *amount_comparison;

    amount_comparison = gsb_data_report_amount_comparison_get_struct_by_no (amount_comparison_number);

    if ( !amount_comparison )
	return 0;

    return amount_comparison -> hbox_second_part;
}

/** 
 * set the hbox_second_part
 * 
 * \param amount_comparison_number number of the amount_comparison
 * \param hbox_second_part
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_amount_comparison_set_hbox_second_part ( gint amount_comparison_number,
								  gpointer hbox_second_part)
{
    struct_amount_comparison *amount_comparison;

    amount_comparison = gsb_data_report_amount_comparison_get_struct_by_no (amount_comparison_number);

    if ( !amount_comparison )
	return FALSE;

    amount_comparison -> hbox_second_part = hbox_second_part;

    return TRUE;
}

/**
 * get the button_second_comparison
 * 
 * \param amount_comparison_number the number of the amount_comparison
 *
 * \return the  of the amount_comparison, -1 if problem
 * */
gpointer gsb_data_report_amount_comparison_get_button_second_comparison ( gint amount_comparison_number )
{
    struct_amount_comparison *amount_comparison;

    amount_comparison = gsb_data_report_amount_comparison_get_struct_by_no (amount_comparison_number);

    if ( !amount_comparison )
	return 0;

    return amount_comparison -> button_second_comparison;
}

/** 
 * set the button_second_comparison
 * 
 * \param amount_comparison_number number of the amount_comparison
 * \param button_second_comparison
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_amount_comparison_set_button_second_comparison ( gint amount_comparison_number,
									  gpointer button_second_comparison)
{
    struct_amount_comparison *amount_comparison;

    amount_comparison = gsb_data_report_amount_comparison_get_struct_by_no (amount_comparison_number);

    if ( !amount_comparison )
	return FALSE;

    amount_comparison -> button_second_comparison = button_second_comparison;

    return TRUE;
}

/**
 * get the entry_second_amount
 * 
 * \param amount_comparison_number the number of the amount_comparison
 *
 * \return the  of the amount_comparison, -1 if problem
 * */
gpointer gsb_data_report_amount_comparison_get_entry_second_amount ( gint amount_comparison_number )
{
    struct_amount_comparison *amount_comparison;

    amount_comparison = gsb_data_report_amount_comparison_get_struct_by_no (amount_comparison_number);

    if ( !amount_comparison )
	return 0;

    return amount_comparison -> entry_second_amount;
}

/** 
 * set the entry_second_amount
 * 
 * \param amount_comparison_number number of the amount_comparison
 * \param entry_second_amount
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_amount_comparison_set_entry_second_amount ( gint amount_comparison_number,
								     gpointer entry_second_amount)
{
    struct_amount_comparison *amount_comparison;

    amount_comparison = gsb_data_report_amount_comparison_get_struct_by_no (amount_comparison_number);

    if ( !amount_comparison )
	return FALSE;

    amount_comparison -> entry_second_amount = entry_second_amount;

    return TRUE;
}




