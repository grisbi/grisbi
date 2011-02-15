/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
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
 * \file gsb_data_fyear_data.c
 * work with the fyear structure, no GUI here
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_data_fyear.h"
#include "utils_dates.h"
#include "gsb_fyear.h"
#include "dialog.h"
#include "utils_str.h"
/*END_INCLUDE*/


/**
 * describe the invalid numbers
 * */
enum fyear_invalid {
    FYEAR_INVALID_DATE_ORDER = 1,
    FYEAR_INVALID_CROSS,
    FYEAR_INVALID_DATE,
};

/*START_STATIC*/
static void _gsb_data_fyear_free ( struct_fyear *fyear );
static gpointer gsb_data_fyear_get_structure ( gint fyear_number );
static gint gsb_data_fyear_max_number ( void );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/** contains the g_slist of struct_fyear */
static GSList *fyear_list = NULL;

/** a pointer to the last fyear used (to increase the speed) */
static struct_fyear *fyear_buffer;


/**
 * set the fyears global variables to NULL,
 * usually when we init all the global variables
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_data_fyear_init_variables ( void )
{
    if ( fyear_list )
    {
        GSList* tmp_list  = fyear_list;
        while ( tmp_list  )
        {
	    struct_fyear *fyear;
	    fyear = tmp_list  -> data;
	    tmp_list = tmp_list -> next;
	    _gsb_data_fyear_free ( fyear );
	}
        g_slist_free ( fyear_list );
    }
    fyear_list = NULL;
    fyear_buffer = NULL;
    return FALSE;
}


/**
 * find and return the structure of the fyear asked
 *
 * \param fyear_number number of fyear
 *
 * \return the adr of the struct of the fyear (NULL if doesn't exit)
 * */
gpointer gsb_data_fyear_get_structure ( gint fyear_number )
{
    GSList *tmp;

    if (!fyear_number)
	    return NULL;

    /* before checking all the fyears, we check the buffer */

    if ( fyear_buffer
	 &&
	 fyear_buffer -> fyear_number == fyear_number )
	    return fyear_buffer;

    tmp = fyear_list;

    while ( tmp )
    {
        struct_fyear *fyear;

	    fyear = tmp -> data;

	    if ( fyear -> fyear_number == fyear_number )
	    {
	        fyear_buffer = fyear;
	        return fyear;
	    }

	    tmp = tmp -> next;
    }
    return NULL;
}


/**
 * give the g_slist of fyears structure
 * usefull when want to check all fyears
 *
 * \param none
 *
 * \return the g_slist of fyears structure
 * */
GSList *gsb_data_fyear_get_fyears_list ( void )
{
    return fyear_list;
}

/**
 * return the number of the fyears given in param
 *
 * \param fyear_ptr a pointer to the struct of the fyear
 *
 * \return the number of the fyear, 0 if problem
 * */
gint gsb_data_fyear_get_no_fyear ( gpointer fyear_ptr )
{
    struct_fyear *fyear;
    
    if ( !fyear_ptr )
	return 0;
    
    fyear = fyear_ptr;
    fyear_buffer = fyear;
    return fyear -> fyear_number;
}


/**
 * find and return the last number of fyear
 * 
 * \param none
 * 
 * \return last number of fyear
 * */
gint gsb_data_fyear_max_number ( void )
{
    GSList *tmp;
    gint number_tmp = 0;

    tmp = fyear_list;
    
    while ( tmp )
    {
	struct_fyear *fyear;

	fyear = tmp -> data;

	if ( fyear -> fyear_number > number_tmp )
	    number_tmp = fyear -> fyear_number;

	tmp = tmp -> next;
    }
    return number_tmp;
}


/**
 * create a new fyear, give him a number, append it to the list
 * and return the number
 *
 * \param name the name of the fyear (can be freed after, it's a copy) or NULL
 *
 * \return the number of the new fyear
 * */
gint gsb_data_fyear_new ( const gchar *name )
{
    struct_fyear *fyear;

    fyear = g_malloc0 ( sizeof ( struct_fyear ));
    fyear -> fyear_number = gsb_data_fyear_max_number () + 1;

    if (name)
	fyear -> fyear_name = my_strdup (name);
    else 
	fyear -> fyear_name = NULL;

    fyear_list = g_slist_append ( fyear_list, fyear );
    fyear_buffer = fyear;

    return fyear -> fyear_number;
}


/**
 * This internal function is called to free the memory used by a struct_fyear structure
 */
static void _gsb_data_fyear_free ( struct_fyear *fyear )
{
    if ( ! fyear )
	return;
    if ( fyear -> fyear_name )
	g_free ( fyear -> fyear_name );
    if ( fyear -> beginning_date )
	g_date_free ( fyear -> beginning_date );
    if ( fyear -> end_date )
	g_date_free ( fyear -> end_date );
    g_free ( fyear );
    if ( fyear_buffer == fyear )
	fyear_buffer = NULL;
}

/**
 * remove a fyear
 * set all the fyears of transaction which are this one to 0
 *
 * \param fyear_number the fyear we want to remove
 *
 * \return TRUE ok
 * */
gboolean gsb_data_fyear_remove ( gint fyear_number )
{
    struct_fyear *fyear;

    fyear = gsb_data_fyear_get_structure ( fyear_number );

    if (!fyear)
	return FALSE;
    
    fyear_list = g_slist_remove ( fyear_list,
				  fyear );
    
    _gsb_data_fyear_free ( fyear );

    return TRUE;
}


/**
 * set a new number for the fyear
 * normally used only while loading the file because
 * the number are given automaticly
 *
 * \param fyear_number the number of the fyear
 * \param new_no_fyear the new number of the fyear
 *
 * \return the new number or 0 if the fyear doen't exist
 * */
gint gsb_data_fyear_set_new_number ( gint fyear_number,
                        gint new_no_fyear )
{
    struct_fyear *fyear;

    fyear = gsb_data_fyear_get_structure ( fyear_number );

    if (!fyear)
	return 0;

    fyear -> fyear_number = new_no_fyear;
    return new_no_fyear;
}


/**
 * return the name of the fyear
 *
 * \param fyear_number the number of the fyear
 *
 * \return the name of the fyear or NULL if fail
 * */
const gchar *gsb_data_fyear_get_name ( gint fyear_number )
{
    struct_fyear *fyear;

    fyear = gsb_data_fyear_get_structure ( fyear_number );

    if (!fyear)
        return NULL;

    return fyear -> fyear_name;
}


/**
 * set the name of the fyear
 * the value is dupplicate in memory
 *
 * \param fyear_number the number of the fyear
 * \param name the name of the fyear
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_fyear_set_name ( gint fyear_number,
				   const gchar *name )
{
    struct_fyear *fyear;

    fyear = gsb_data_fyear_get_structure ( fyear_number );

    if (!fyear)
	return FALSE;

    /* we free the last name */
    if ( fyear -> fyear_name )
	g_free (fyear -> fyear_name);
    
    /* and copy the new one */
    fyear -> fyear_name = my_strdup (name);

   /* update list fyear in form */
    if (fyear -> showed_in_form)
	gsb_fyear_update_fyear_list ();

    return TRUE;
}



/**
 * return the beginning date of the fyear
 *
 * \param fyear_number the number of the fyear
 *
 * \return the beginning date of the fyear or NULL if fail
 * */
GDate *gsb_data_fyear_get_beginning_date ( gint fyear_number )
{
    struct_fyear *fyear;

    fyear = gsb_data_fyear_get_structure ( fyear_number );

    if (!fyear)
	return NULL;

    return fyear -> beginning_date;
}


/**
 * set the beginning date of the fyear
 * the value is dupplicate in memory
 *
 * \param fyear_number the number of the fyear
 * \param date the beginning date of the fyear
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_fyear_set_beginning_date ( gint fyear_number,
					    GDate *date )
{
    struct_fyear *fyear;

    fyear = gsb_data_fyear_get_structure ( fyear_number );

    if (!fyear)
	return FALSE;

    /* we free the last date */
    if ( fyear -> beginning_date )
	g_date_free (fyear -> beginning_date);
    
    /* and copy the new one */
    fyear -> beginning_date = gsb_date_copy (date);

    return TRUE;
}


/**
 * return the end date of the fyear
 *
 * \param fyear_number the number of the fyear
 *
 * \return the end date of the fyear or NULL if fail
 * */
GDate *gsb_data_fyear_get_end_date ( gint fyear_number )
{
    struct_fyear *fyear;

    fyear = gsb_data_fyear_get_structure ( fyear_number );

    if (!fyear)
	return NULL;

    return fyear -> end_date;
}


/**
 * set the end date of the fyear
 * the value is dupplicate in memory
 *
 * \param fyear_number the number of the fyear
 * \param date the end date of the fyear
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_fyear_set_end_date ( gint fyear_number,
				       GDate *date )
{
    struct_fyear *fyear;

    fyear = gsb_data_fyear_get_structure ( fyear_number );

    if (!fyear)
	return FALSE;

    /* we free the last date */
    if ( fyear -> end_date )
	g_date_free (fyear -> end_date);

    /* and copy the new one */
    fyear -> end_date = gsb_date_copy (date);

    return TRUE;
}



/**
 * return the showed_in_form of the fyear
 *
 * \param fyear_number the number of the fyear
 *
 * \return the showed_in_form of the fyear or NULL if fail
 * */
gboolean gsb_data_fyear_get_form_show ( gint fyear_number )
{
    struct_fyear *fyear;

    fyear = gsb_data_fyear_get_structure ( fyear_number );

    if (!fyear)
	return FALSE;

    return fyear -> showed_in_form;
}


/**
 * set the showed_in_form of the fyear
 *
 * \param fyear_number the number of the fyear
 * \param showed_in_form the showed_in_form of the fyear
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_fyear_set_form_show ( gint fyear_number,
					gboolean showed_in_form )
{
    struct_fyear *fyear;

    fyear = gsb_data_fyear_get_structure ( fyear_number );

    if (!fyear)
	return FALSE;

    /* and copy the new one */
    fyear -> showed_in_form = showed_in_form;

    /* update list fyear in form */
    gsb_fyear_update_fyear_list ();

    return TRUE;
}


/**
 * return the invalid_fyear of the fyear
 * the flag invalid is set automatickly by gsb_data_fyear_check_for_invalid
 *
 * \param fyear_number the number of the fyear
 *
 * \return TRUE if the fyear is invalid 
 * */
gint gsb_data_fyear_get_invalid ( gint fyear_number )
{
    struct_fyear *fyear;

    fyear = gsb_data_fyear_get_structure ( fyear_number );

    if (!fyear)
	return FALSE;

    return fyear -> invalid_fyear;
}

/**
 * return the message error because of the invalid fyear
 *
 * \param fyear_number the number of the fyear
 *
 * \return a const gchar formatted with markup : error the message or NULL if not exist
 * */
const gchar *gsb_data_fyear_get_invalid_message ( gint fyear_number )
{
    struct_fyear *fyear;
    gchar *string = NULL;

    fyear = gsb_data_fyear_get_structure ( fyear_number );

    if ( !fyear || !fyear -> invalid_fyear )
        return NULL;

    switch ( fyear -> invalid_fyear )
    {
    case FYEAR_INVALID_DATE_ORDER:
        string = make_red ( _("Warning : the dates are not in good order.") );
        break;
    case FYEAR_INVALID_CROSS:
        string = make_red ( _("Warning : that financial year cross with another one.") );
        break;
    case FYEAR_INVALID_DATE:
        string = make_red ( _("Warning : Invalid date.") );
        break;
    }

    return string;
}

/**
 * check all the fyear if the are invalid and set their flag
 * normaly called to each change of a fyear
 * an invalid fyear will not be used by grisbi and showed as invalid in the configuration
 *
 * \param fyear_number the number of the fyear to check
 *
 * \return TRUE  if invalid, FALSE if not
 * */
void gsb_data_fyear_check_all_for_invalid ( void )
{
    GSList *tmp_list;

    tmp_list = fyear_list;

    while (tmp_list)
    {
	fyear_buffer = tmp_list -> data;
	gsb_data_fyear_check_for_invalid ( fyear_buffer -> fyear_number);
	
	tmp_list = tmp_list -> next;
    }
}


/**
 * check if the fyear is invalid and set the flag
 * a fyear is invalid
 * an invalid fyear will not be used by grisbi and showed as invalid in the configuration
 *
 * \param fyear_number the number of the fyear to check
 *
 * \return TRUE  if invalid, FALSE if not
 * */
gboolean gsb_data_fyear_check_for_invalid ( gint fyear_number )
{
    struct_fyear *fyear;

    fyear = gsb_data_fyear_get_structure ( fyear_number );

    if (!fyear)
	return FALSE;

    /* check if there is some good date */
    if (!fyear -> beginning_date
	||
	!fyear -> end_date)
    {
	fyear -> invalid_fyear = FYEAR_INVALID_DATE;
	return TRUE;
    }

    /* first : we check that the first date is above the second */
    if ( g_date_compare (fyear -> beginning_date,
			 fyear -> end_date) > 0)
    {
	fyear -> invalid_fyear = FYEAR_INVALID_DATE_ORDER;
	return TRUE;
    }

    /* second : we check if there is not a cross with another fyear */
    if ( (gsb_data_fyear_get_from_date (fyear -> beginning_date) != fyear_number)
	 ||
	 (gsb_data_fyear_get_from_date (fyear -> end_date) != fyear_number))
    {
	fyear -> invalid_fyear = FYEAR_INVALID_CROSS;
	return TRUE;
    }

    /* it's ok, the fyear is valid */
    fyear -> invalid_fyear = FALSE;

    return FALSE;
}



/**
 * get the financial year corresponding to the given date
 * if there is more than 1 financial year corresponding to that date, return -1
 *
 * \param date
 *
 * \return the number of financial year, 0 if none on that date, -1 if more than 1 on that date
 * */
gint gsb_data_fyear_get_from_date ( const GDate *date )
{
    GSList *tmp_list;
    gint return_value = 0;

    if (!date)
	return 0;

    tmp_list = fyear_list;
    while (tmp_list)
    {
	struct_fyear *fyear;

	fyear = tmp_list -> data;

	/* check the fyear only if the dates are valid */
	if (fyear -> beginning_date && fyear -> end_date)
	{
	    if ( g_date_compare ( date, fyear -> beginning_date) >= 0
		 &&
		 g_date_compare ( date, fyear -> end_date) <= 0 )
	    {
		if (return_value)
		    return_value = -1;
		else
		    return_value = fyear -> fyear_number;
	    }
	}
	tmp_list = tmp_list -> next;
    }
    return return_value;
}


/**
 * compare 2 financial years
 *
 * \param fyear_number_1
 * \param fyear_number_2
 *
 * \return -1 if fyear 1 is before 2 ; +1 if fyear 1 is after 2 ; 0 if problem
 * */
gint gsb_data_fyear_compare ( gint fyear_number_1, gint fyear_number_2 )
{
    struct_fyear *fyear_1;
    struct_fyear *fyear_2;

    fyear_1 = gsb_data_fyear_get_structure (fyear_number_1);
    fyear_2 = gsb_data_fyear_get_structure (fyear_number_2);

    return gsb_data_fyear_compare_from_struct ( fyear_1, fyear_2 );

 }


/**
 * compare 2 financial years bye struct_fyear *fyear
 *
 * \param struct_fyear *fyear_1
 * \param struct_fyear *fyear_2
 *
 * \return -1 if fyear 2 is before 1 ; +1 if fyear 2 is after 1 ; 0 if problem
 * */
gint gsb_data_fyear_compare_from_struct ( struct_fyear *fyear_1,
                        struct_fyear *fyear_2 )
{
    if ( !fyear_1 || !fyear_2 )
        return 0;

    if ( ! fyear_1 -> beginning_date )
        return -1;
    if ( ! fyear_2 -> beginning_date )
        return -1;

    if (g_date_compare (fyear_1 -> beginning_date, fyear_2 -> end_date) >= 0)
        return -1;
    if (g_date_compare (fyear_2 -> beginning_date, fyear_1 -> end_date) >= 0)
        return 1;
    return 0;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
