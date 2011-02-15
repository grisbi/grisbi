/* ************************************************************************** */
/*                                                                            */
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
 * \file gsb_data_archive.c
 * work with the archive store structure, no GUI here
 * the archive store are used to show the archive in the list of transactions
 * as an archive by itself contains several accounts and no balance for each account
 * at the opening of grisbi, we create an intermediate list of structures wich contains
 * the link to the archive, but 1 structure per account, with the number of transactions
 * and the balance for each.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_data_archive_store.h"
#include "dialog.h"
#include "gsb_data_account.h"
#include "gsb_data_currency.h"
#include "gsb_data_transaction.h"
#include "gsb_real.h"
#include "transaction_list.h"
/*END_INCLUDE*/


/**
 * \struct 
 * Describe an archive store
 */
typedef struct
{
    gint archive_store_number;

    /* the corresponding archive (1 archive contains several archive store) */
    gint archive_number;

    /* account we are working on */
    gint account_number;

    /* balance of all the transactions of the archive for that account */
    gsb_real balance;

    /* number of transactions in the archive for that account */
    gint nb_transactions;
} struct_store_archive;


/*START_STATIC*/
static void _gsb_data_archive_store_free ( struct_store_archive *archive );
static struct_store_archive *gsb_data_archive_store_find_struct ( gint archive_number,
                        gint account_number );
static gint gsb_data_archive_store_max_number ( void );
static gint gsb_data_archive_store_new ( void );
/*END_STATIC*/

/*START_EXTERN*/
extern gsb_real null_real;
/*END_EXTERN*/

/** contains the g_slist of struct_store_archive */
static GSList *archive_store_list = NULL;

/** a pointer to the last archive_store used (to increase the speed) */
static struct_store_archive *archive_store_buffer;


/**
 * set the archives global variables to NULL,
 * usually when we init all the global variables
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_data_archive_store_init_variables ( void )
{
    if ( archive_store_list )
    {
        GSList* tmp_list = archive_store_list;
        while ( tmp_list )
        {
            struct_store_archive *archive;
            gint archive_number;

            archive = tmp_list -> data;
            archive_number = gsb_data_archive_store_get_archive_number (
                        archive -> archive_store_number );
            transaction_list_remove_archive ( archive_number );
            tmp_list = tmp_list -> next;
            _gsb_data_archive_store_free ( archive );
        }
        g_slist_free ( archive_store_list );
    }
    archive_store_list = NULL;
    archive_store_buffer = NULL;

    return FALSE;
}


/**
 * give the g_slist of archives structure
 * usefull when want to check all archives
 *
 * \param none
 *
 * \return the g_slist of archives structure
 * */
GSList *gsb_data_archive_store_get_archives_list ( void )
{
    return archive_store_list;
}

/**
 * return the number of the archives given in param
 *
 * \param archive_ptr a pointer to the struct of the archive
 *
 * \return the number of the archive, 0 if problem
 * */
gint gsb_data_archive_store_get_number ( gpointer archive_ptr )
{
    struct_store_archive *archive;

    if ( !archive_ptr )
    return 0;

    archive = archive_ptr;
    archive_store_buffer = archive;

    return archive -> archive_store_number;
}



/**
 * function called at the opening of grisbi
 * create all the archive store according to the archives in grisbi
 *
 * \param
 *
 * \return
 * */
void gsb_data_archive_store_create_list ( void )
{
    GSList *tmp_list;

    tmp_list = gsb_data_transaction_get_complete_transactions_list ();
    while (tmp_list)
    {
    gint transaction_number;
    gint archive_number;

    transaction_number = gsb_data_transaction_get_transaction_number (tmp_list -> data);

    archive_number = gsb_data_transaction_get_archive_number (transaction_number);

    if (archive_number)
    {
        struct_store_archive *archive_store;
        gint floating_point;
        gint account_number;

        account_number = gsb_data_transaction_get_account_number (transaction_number);
        floating_point = gsb_data_currency_get_floating_point (
                            gsb_data_account_get_currency (account_number) );
        archive_store = gsb_data_archive_store_find_struct ( archive_number, account_number);
        if (archive_store)
        {
            /* there is already a struct_store_archive for the same archive and the same account,
             * we increase the balance except for operations */
            if ( !gsb_data_transaction_get_mother_transaction_number ( transaction_number ) )
                archive_store -> balance = gsb_real_add ( archive_store -> balance,
                                    gsb_data_transaction_get_adjusted_amount (
                                    transaction_number,
                                    floating_point));
            archive_store -> nb_transactions++;
        }
        else
        {
            /* there is no struct_store_archive for that transaction, we make a new one 
             * with the balance of the transaction as balance */
            gint archive_store_number;

            archive_store_number = gsb_data_archive_store_new ();
            archive_store = gsb_data_archive_store_get_structure (archive_store_number);

            archive_store -> archive_number = archive_number;
            archive_store -> account_number = account_number;
            archive_store -> balance = gsb_data_transaction_get_adjusted_amount (
                            transaction_number, floating_point);
            if ( ! gsb_data_transaction_get_mother_transaction_number ( transaction_number ) )
                archive_store -> nb_transactions = 1;
        }
    }
    tmp_list = tmp_list -> next;
    }
}

/**
 * This internal function is called to free the memory used by a struct_store_archive structure
 */
static void _gsb_data_archive_store_free ( struct_store_archive *archive )
{
    if ( ! archive )
        return;
    g_free ( archive );
    if ( archive_store_buffer == archive )
    archive_store_buffer = NULL;
}

/**
 * remove an archive store
 *
 * \param archive_store_number the archive store we want to remove
 *
 * \return TRUE ok
 * */
gboolean gsb_data_archive_store_remove ( gint archive_store_number )
{
    struct_store_archive *archive;

    archive = gsb_data_archive_store_get_structure ( archive_store_number );

    if (!archive)
    return FALSE;

    archive_store_list = g_slist_remove ( archive_store_list,
                      archive );

    _gsb_data_archive_store_free ( archive );
    return TRUE;
}

/**
 * remove all the archives stores corresponding to the archive
 *
 * \param archive_number the archive we want remove the corresponding archives stores
 *
 * \return TRUE ok
 * */
gboolean gsb_data_archive_store_remove_by_archive ( gint archive_number )
{
    GSList *tmp_list;

    tmp_list = archive_store_list;
    while (tmp_list)
    {
    struct_store_archive *archive;

    archive = tmp_list -> data;
    tmp_list = tmp_list -> next;
    if (archive -> archive_number == archive_number)
    {
        archive_store_list = g_slist_remove ( archive_store_list,
                          archive );
        _gsb_data_archive_store_free ( archive );
    }
    }
    return TRUE;
}




/**
 * return the number of the corresponding archive
 *
 * \param archive_store_number the number of the archive store
 *
 * \return the  of the archive or 0 if fail
 * */
gint gsb_data_archive_store_get_archive_number ( gint archive_store_number )
{
    struct_store_archive *archive;

    archive = gsb_data_archive_store_get_structure ( archive_store_number );

    if (!archive)
    return 0;

    return archive -> archive_number;
}

/**
 * return the account_number of the archive store
 *
 * \param archive_store_number the number of the archive store
 *
 * \return the  of the archive or 0 if fail
 * */
gint gsb_data_archive_store_get_account_number ( gint archive_store_number )
{
    struct_store_archive *archive;

    archive = gsb_data_archive_store_get_structure ( archive_store_number );

    if (!archive)
    return 0;

    return archive -> account_number;
}

/**
 * return the balance of the archive store
 * ie the balance of the archive for the account in the archive store
 *
 * \param archive_store_number the number of the archive store
 *
 * \return the  of the archive or 0 if fail
 * */
gsb_real gsb_data_archive_store_get_balance ( gint archive_store_number )
{
    struct_store_archive *archive;

    archive = gsb_data_archive_store_get_structure ( archive_store_number );

    if (!archive)
    return null_real;

    return archive -> balance;
}

/**
 * return the number of transactions of the archive store
 *
 * \param archive_store_number the number of the archive store
 *
 * \return the  of the archive or 0 if fail
 * */
gint gsb_data_archive_store_get_transactions_number ( gint archive_store_number )
{
    struct_store_archive *archive;

    archive = gsb_data_archive_store_get_structure ( archive_store_number );

    if (!archive)
    return 0;

    return archive -> nb_transactions;
}



/**
 * find and return the structure of the archive asked
 *
 * \param archive_store_number number of archive
 *
 * \return the adr of the struct of the archive (NULL if doesn't exit)
 * */
gpointer gsb_data_archive_store_get_structure ( gint archive_store_number )
{
    GSList *tmp;

    if (!archive_store_number)
    return NULL;

    /* before checking all the archives, we check the buffer */
    if ( archive_store_buffer
     &&
     archive_store_buffer -> archive_store_number == archive_store_number )
    return archive_store_buffer;

    tmp = archive_store_list;

    while ( tmp )
    {
    struct_store_archive *archive;

    archive = tmp -> data;

    if ( archive -> archive_store_number == archive_store_number )
    {
        archive_store_buffer = archive;
        return archive;
    }

    tmp = tmp -> next;
    }
    return NULL;
}

/**
 * find and return the last number of archive
 * 
 * \param none
 * 
 * \return last number of archive
 * */
static gint gsb_data_archive_store_max_number ( void )
{
    GSList *tmp;
    gint number_tmp = 0;

    tmp = archive_store_list;

    while ( tmp )
    {
    struct_store_archive *archive;

    archive = tmp -> data;

    if ( archive -> archive_store_number > number_tmp )
        number_tmp = archive -> archive_store_number;

    tmp = tmp -> next;
    }
    return number_tmp;
}

/**
 * find the archive  store corresponding to the archive number and account number
 * given in param
 *
 * \param archive_number the wanted archive number
 * \param account_number the wanted account number
 *
 * \return a pointer to the found struct_store_archive or NULL
 * */
static struct_store_archive *gsb_data_archive_store_find_struct ( gint archive_number,
                        gint account_number )
{
    GSList *tmp_list;

    tmp_list = archive_store_list;
    while (tmp_list)
    {
    struct_store_archive *archive;

    archive = tmp_list -> data;

    if (archive -> archive_number == archive_number
        &&
        archive -> account_number == account_number )
        return archive;

    tmp_list = tmp_list -> next;
    }
    return NULL;
}

/**
 * create a new archive store, give it a number, append it to the list
 * and return the number
 *
 * \param
 *
 * \return the number of the new archive store
 * */
static gint gsb_data_archive_store_new ( void )
{
    struct_store_archive *archive;

    archive = g_malloc0 ( sizeof ( struct_store_archive ));
    if (!archive)
    {
    dialogue_error_memory ();
    return 0;
    }
    archive -> archive_store_number = gsb_data_archive_store_max_number () + 1;

    archive_store_list = g_slist_append ( archive_store_list, archive );
    archive_store_buffer = archive;

    return archive -> archive_store_number;
}


gsb_real gsb_data_archive_store_get_archives_balance ( gint account_number )
{
    GSList *tmp_list;
    gsb_real balance = null_real;

    tmp_list = gsb_data_archive_store_get_archives_list ( );

    while (tmp_list)
    {
        struct_store_archive *archive_store;

        archive_store = tmp_list -> data;

        if ( archive_store -> account_number == account_number )
            balance = gsb_real_add ( balance, archive_store -> balance );

        tmp_list = tmp_list -> next;
    }

    return balance;
}
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
