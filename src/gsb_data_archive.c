/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*     Copyright (C)    2000-2007 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2007 Benjamin Drieu (bdrieu@april.org)                       */
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
 * \file gsb_data_archive.c
 * work with the archive structure, no GUI here
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_data_archive.h"
#include "dialog.h"
#include "gsb_data_transaction.h"
#include "utils_dates.h"
#include "utils_str.h"
/*END_INCLUDE*/


/**
 * \struct 
 * Describe a archive 
 */
typedef struct
{
    guint archive_number;
    gchar *archive_name;

    /* for the archive created by date,
     * else the 2 values are NULL */
    GDate *beginning_date;
    GDate *end_date;

    /* for the archive created by financial year,
     * else the value is 0 */
    gint fyear_number;

    /* if created by report, this is the title of
     * the report */
    gchar *report_title;
} struct_archive;

/*START_STATIC*/
static void _gsb_data_archive_free ( struct_archive* archive);
static gpointer gsb_data_archive_get_structure ( gint archive_number );
static gint gsb_data_archive_max_number ( void );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/** contains the g_slist of struct_archive */
static GSList *archive_list = NULL;

/** a pointer to the last archive used (to increase the speed) */
static struct_archive *archive_buffer;


/**
 * set the archives global variables to NULL,
 * usually when we init all the global variables
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_data_archive_init_variables ( void )
{
    if ( archive_list )
    {
        GSList* tmp_list = archive_list;
        while ( tmp_list )
        {
	    struct_archive *archive;
	    archive = tmp_list -> data;
	    tmp_list = tmp_list -> next;
            _gsb_data_archive_free ( archive ); 
	}
        g_slist_free ( archive_list );
    }
    archive_list = NULL;
    archive_buffer = NULL;

    return FALSE;
}


/**
 * find and return the structure of the archive asked
 *
 * \param archive_number number of archive
 *
 * \return the adr of the struct of the archive (NULL if doesn't exit)
 * */
gpointer gsb_data_archive_get_structure ( gint archive_number )
{
    GSList *tmp;

    if (!archive_number)
	return NULL;

    /* before checking all the archives, we check the buffer */

    if ( archive_buffer
	 &&
	 archive_buffer -> archive_number == archive_number )
	return archive_buffer;

    tmp = archive_list;

    while ( tmp )
    {
	struct_archive *archive;

	archive = tmp -> data;

	if ( archive -> archive_number == archive_number )
	{
	    archive_buffer = archive;
	    return archive;
	}

	tmp = tmp -> next;
    }
    return NULL;
}


/**
 * give the g_slist of archives structure
 * usefull when want to check all archives
 *
 * \param none
 *
 * \return the g_slist of archives structure
 * */
GSList *gsb_data_archive_get_archives_list ( void )
{
    return archive_list;
}

/**
 * return the number of the archives given in param
 *
 * \param archive_ptr a pointer to the struct of the archive
 *
 * \return the number of the archive, 0 if problem
 * */
gint gsb_data_archive_get_no_archive ( gpointer archive_ptr )
{
    struct_archive *archive;

    if ( !archive_ptr )
	return 0;

    archive = archive_ptr;
    archive_buffer = archive;
    return archive -> archive_number;
}


/**
 * find and return the last number of archive
 * 
 * \param none
 * 
 * \return last number of archive
 * */
gint gsb_data_archive_max_number ( void )
{
    GSList *tmp;
    gint number_tmp = 0;

    tmp = archive_list;

    while ( tmp )
    {
	struct_archive *archive;

	archive = tmp -> data;

	if ( archive -> archive_number > number_tmp )
	    number_tmp = archive -> archive_number;

	tmp = tmp -> next;
    }
    return number_tmp;
}


/**
 * create a new archive, give him a number, append it to the list
 * and return the number
 *
 * \param name the name of the archive (can be freed after, it's a copy) or NULL
 *
 * \return the number of the new archive
 * */
gint gsb_data_archive_new ( const gchar *name )
{
    struct_archive *archive;

    archive = g_malloc0 ( sizeof ( struct_archive ));
    if (!archive)
    {
    dialogue_error_memory ();
    return 0;
    }
    archive -> archive_number = gsb_data_archive_max_number () + 1;

    if (name)
    archive -> archive_name = my_strdup (name);
    else 
    archive -> archive_name = NULL;

    archive_list = g_slist_append ( archive_list, archive );
    archive_buffer = archive;

    return archive -> archive_number;
}

/**
 * This internal function is called to free the memory used by an struct_archive structure
 */
static void _gsb_data_archive_free ( struct_archive* archive)
{
    if ( ! archive )
        return;
    if ( archive -> archive_name )
	g_free ( archive -> archive_name );
    if ( archive -> beginning_date )
	g_date_free ( archive -> beginning_date );
    if ( archive -> end_date )
	g_date_free ( archive -> end_date );
    if ( archive -> report_title )
	g_free ( archive -> report_title );
    g_free ( archive );
    if ( archive_buffer == archive )
	archive_buffer = NULL;
}

/**
 * remove an archive
 * remove too the archive from the transactions linked to it
 *
 * \param archive_number the archive we want to remove
 *
 * \return TRUE ok
 * */
gboolean gsb_data_archive_remove ( gint archive_number )
{
    struct_archive *archive;
    GSList *tmp_list;

    archive = gsb_data_archive_get_structure ( archive_number );

    if (!archive)
	return FALSE;

    /* remove the archive from the transactions */
    tmp_list = gsb_data_transaction_get_complete_transactions_list ();
    while (tmp_list)
    {
	gint transaction_number;

	transaction_number = gsb_data_transaction_get_transaction_number (tmp_list -> data);
	if (gsb_data_transaction_get_archive_number (transaction_number) == archive -> archive_number)
	    gsb_data_transaction_set_archive_number ( transaction_number,
						      0 );
	tmp_list = tmp_list -> next;
    }

    /* remove the archive from the list */
    archive_list = g_slist_remove ( archive_list,
				    archive );

    _gsb_data_archive_free (archive);

    return TRUE;
}


/**
 * set a new number for the archive
 * normally used only while loading the file because
 * the number are given automaticly
 *
 * \param archive_number the number of the archive
 * \param new_no_archive the new number of the archive
 *
 * \return the new number or 0 if the archive doen't exist
 * */
gint gsb_data_archive_set_new_number ( gint archive_number,
                        gint new_no_archive )
{
    struct_archive *archive;

    archive = gsb_data_archive_get_structure ( archive_number );

    if (!archive)
    return 0;

    archive -> archive_number = new_no_archive;
    return new_no_archive;
}


/**
 * return the name of the archive
 *
 * \param archive_number the number of the archive
 *
 * \return the name of the archive or NULL if fail
 * */
const gchar *gsb_data_archive_get_name ( gint archive_number )
{
    struct_archive *archive;

    archive = gsb_data_archive_get_structure ( archive_number );

    if (!archive)
	return NULL;

    return archive -> archive_name;
}


/**
 * set the name of the archive
 * the value is dupplicate in memory
 *
 * \param archive_number the number of the archive
 * \param name the name of the archive
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_archive_set_name ( gint archive_number,
                        const gchar *name )
{
    struct_archive *archive;

    archive = gsb_data_archive_get_structure ( archive_number );

    if (!archive)
	return FALSE;

    /* we free the last name */
    if ( archive -> archive_name )
	g_free (archive -> archive_name);

    /* and copy the new one */
    archive -> archive_name = my_strdup (name);

    return TRUE;
}



/**
 * return the beginning date of the archive
 *
 * \param archive_number the number of the archive
 *
 * \return the beginning date of the archive or NULL if fail
 * */
GDate *gsb_data_archive_get_beginning_date ( gint archive_number )
{
    struct_archive *archive;

    archive = gsb_data_archive_get_structure ( archive_number );

    if (!archive)
	return NULL;

    return archive -> beginning_date;
}


/**
 * set the beginning date of the archive
 * the value is dupplicate in memory
 *
 * \param archive_number the number of the archive
 * \param date the beginning date of the archive
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_archive_set_beginning_date ( gint archive_number,
                        const GDate *date )
{
    struct_archive *archive;

    archive = gsb_data_archive_get_structure ( archive_number );

    if (!archive)
	return FALSE;

    /* we free the last date */
    if ( archive -> beginning_date )
	g_date_free (archive -> beginning_date);

    /* and copy the new one */
    archive -> beginning_date = gsb_date_copy (date);

    return TRUE;
}


/**
 * return the end date of the archive
 *
 * \param archive_number the number of the archive
 *
 * \return the end date of the archive or NULL if fail
 * */
GDate *gsb_data_archive_get_end_date ( gint archive_number )
{
    struct_archive *archive;

    archive = gsb_data_archive_get_structure ( archive_number );

    if (!archive)
	return NULL;

    return archive -> end_date;
}


/**
 * set the end date of the archive
 * the value is dupplicate in memory
 *
 * \param archive_number the number of the archive
 * \param date the end date of the archive
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_archive_set_end_date ( gint archive_number,
                        const GDate *date )
{
    struct_archive *archive;

    archive = gsb_data_archive_get_structure ( archive_number );

    if (!archive)
	return FALSE;

    /* we free the last date */
    if ( archive -> end_date )
	g_date_free (archive -> end_date);

    /* and copy the new one */
    archive -> end_date = gsb_date_copy (date);

    return TRUE;
}

/**
 * return the fyear of the archive
 *
 * \param archive_number the number of the archive
 *
 * \return the fyear of the archive or 0 if fail
 * */
gint gsb_data_archive_get_fyear ( gint archive_number )
{
    struct_archive *archive;

    archive = gsb_data_archive_get_structure ( archive_number );

    if (!archive)
	return 0;

    return archive -> fyear_number;
}


/**
 * set the fyear of the archive
 *
 * \param archive_number the number of the archive
 * \param fyear_number the fyear of the archive
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_archive_set_fyear ( gint archive_number,
                        gint fyear_number )
{
    struct_archive *archive;

    archive = gsb_data_archive_get_structure ( archive_number );

    if (!archive)
	return FALSE;

    archive -> fyear_number = fyear_number;

    return TRUE;
}


/**
 * return the report creation of the archive
 *
 * \param archive_number the number of the archive
 *
 * \return the report_title or NULL
 * */
const gchar *gsb_data_archive_get_report_title ( gint archive_number )
{
    struct_archive *archive;

    archive = gsb_data_archive_get_structure ( archive_number );

    if (!archive)
	return NULL;

    return archive -> report_title;
}


/**
 * set the report_title value,
 *
 * \param archive_number the number of the archive
 * \param report_title the title to set if created by a report
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_archive_set_report_title ( gint archive_number,
                        const gchar *report_title )
{
    struct_archive *archive;

    archive = gsb_data_archive_get_structure ( archive_number );

    if (!archive)
	return FALSE;

    if (archive -> report_title)
	g_free (archive -> report_title);

    archive -> report_title = my_strdup (report_title);

    return TRUE;
}



/**
 * get the archive corresponding to the given date
 * if there is more than 1 archive corresponding to that date, return -1
 *
 * \param date
 *
 * \return the number of archive, 0 if none on that date, -1 if more than 1 on that date
 * */
gint gsb_data_archive_get_from_date ( const GDate *date )
{
    GSList *tmp_list;
    gint return_value = 0;

    if (!date)
	return 0;

    tmp_list = archive_list;
    while (tmp_list)
    {
	struct_archive *archive;

	archive = tmp_list -> data;

	/* check the archive only if the dates are valid */
	if (archive -> beginning_date && archive -> end_date)
	{
	    if ( g_date_compare ( date, archive -> beginning_date) >= 0
		 &&
		 g_date_compare ( date, archive -> end_date) <= 0 )
	    {
		if (return_value)
		    return_value = -1;
		else
		    return_value = archive -> archive_number;
	    }
	}
	tmp_list = tmp_list -> next;
    }
    return return_value;
}




/**
 * get the archive corresponding to the given financial year
 *
 * \param fyear_number
 *
 * \return the number of archive, 0 if none
 * */
gint gsb_data_archive_get_from_fyear ( gint fyear_number )
{
    GSList *tmp_list;

    tmp_list = archive_list;
    while (tmp_list)
    {
	struct_archive *archive;

	archive = tmp_list -> data;

	if (archive -> fyear_number == fyear_number)
	    return archive -> archive_number;

	tmp_list = tmp_list -> next;
    }
    return 0;
}



