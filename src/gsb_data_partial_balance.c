/* ************************************************************************** */
/*                                                                            */
/*     copyright (c)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2004-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*                      2008-2009 Pierre Biava (grisbi@pierre.biava.name)     */
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
 * \file data_partial_balance.c
 */

#include "include.h"

/*START_INCLUDE*/
#include "gsb_data_partial_balance.h"
#include "./dialog.h"
#include "./gsb_data_account.h"
#include "./gsb_real.h"
#include "./utils_str.h"
#include "./gsb_real.h"
#include "./gsb_data_account.h"
/*END_INCLUDE*/

/** \struct
 * contenant les éléments d'un solde partiel
 * */
typedef struct
{
    gint partial_balance_number;
    gchar *balance_name;
    gchar *liste_cptes;
    kind_account kind;
    gint currency;
} struct_partial_balance;


/*START_STATIC*/
static  void _gsb_data_partial_balance_free ( struct_partial_balance *partial_balance);
static gint gsb_data_partial_balance_cmp_func ( struct_partial_balance *partial_balance_1,
                        struct_partial_balance *partial_balance_2 );
static gpointer gsb_data_partial_balance_get_structure ( gint partial_balance_number );
static gboolean gsb_data_partial_balance_remove ( gint partial_balance_number );
/*END_STATIC*/

/*START_EXTERN*/
extern gsb_real null_real;
/*END_EXTERN*/

/** contains the g_slist of struct_partial_balance */
static GSList *partial_balance_list = NULL;

/** a pointer to the last partial_balance used (to increase the speed) */
static struct_partial_balance *partial_balance_buffer;


/**
 * set the partial_balance global variables to NULL,
 * usually when we init all the global variables
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_data_partial_balance_init_variables ( void )
{
    if ( partial_balance_list )
    {
        GSList* tmp_list = partial_balance_list;
        while ( tmp_list )
        {
            struct_partial_balance *partial_balance;
            partial_balance = tmp_list -> data;
            tmp_list = tmp_list -> next;
            _gsb_data_partial_balance_free ( partial_balance ); 
        }
        g_slist_free ( partial_balance_list );
    }
    partial_balance_list = NULL;
    partial_balance_buffer = NULL;

    return FALSE;
}


/**
 * find and return the structure of the partial_balance asked
 *
 * \param partial_balance_number number of partial_balance
 *
 * \return the adr of the struct of the partial_balance (NULL if doesn't exit)
 * */
gpointer gsb_data_partial_balance_get_structure ( gint partial_balance_number )
{
    GSList *tmp;

    if ( !partial_balance_number )
        return NULL;

    /* before checking all the import rule, we check the buffer */
    if ( partial_balance_buffer
     &&
     partial_balance_buffer -> partial_balance_number == partial_balance_number )
        return partial_balance_buffer;

    tmp = partial_balance_list;

    while ( tmp )
    {
        struct_partial_balance *partial_balance;

        partial_balance = tmp -> data;

        if ( partial_balance -> partial_balance_number == partial_balance_number )
        {
            partial_balance_buffer = partial_balance;
            return partial_balance;
        }

        tmp = tmp -> next;
    }
    return NULL;
}


/**
 * give the g_slist of partial_balance structure
 * usefull when want to check all partial_balance
 *
 * \param none
 *
 * \return the g_slist of partial_balance structure
 * */
GSList *gsb_data_partial_balance_get_list ( void )
{
    return partial_balance_list;
}


/**
 * return the number of the partial_balance given in param
 *
 * \param balance_ptr a pointer to the struct of the partial_balance
 *
 * \return the number of the partial_balance, 0 if problem
 * */
gint gsb_data_partial_balance_get_number ( gpointer balance_ptr )
{
    struct_partial_balance *partial_balance;

    if ( !balance_ptr )
        return 0;

    partial_balance = balance_ptr;
    partial_balance_buffer = partial_balance;
    return partial_balance -> partial_balance_number;
}


/**
 * create a new partial_balance, give him a number, append it to the list
 * and return the number
 *
 * \param name the name of the partial_balance (can be freed after, it's a copy) or NULL
 *
 * \return the number of the new partial_balance
 * */
gint gsb_data_partial_balance_new ( const gchar *name )
{
    struct_partial_balance *partial_balance;

    partial_balance = g_malloc0 ( sizeof ( struct_partial_balance ));
    if ( ! partial_balance )
    {
        dialogue_error_memory ( );
        return 0;
    }
    partial_balance -> partial_balance_number = g_slist_length ( partial_balance_list ) + 1;

    if ( name )
        partial_balance -> balance_name = my_strdup ( name );
    else 
        partial_balance -> balance_name = NULL;

    partial_balance_list = g_slist_insert_sorted ( partial_balance_list,
                        partial_balance,
                        (GCompareFunc) gsb_data_partial_balance_cmp_func );
    partial_balance_buffer = partial_balance;

    return partial_balance -> partial_balance_number;
}


/**
 * This internal function is called to free the memory used by an 
 * struct_partial_balance structure
 */
static void _gsb_data_partial_balance_free ( struct_partial_balance *partial_balance)
{
    if ( ! partial_balance )
        return;
    if ( partial_balance -> balance_name )
        g_free ( partial_balance -> balance_name );
    g_free ( partial_balance );
    if ( partial_balance_buffer == partial_balance )
        partial_balance_buffer = NULL;
}


/**
 * remove a partial_balance
 *
 * \param partial_balance_number the partial_balance we want to remove
 *
 * \return TRUE ok
 * */
gboolean gsb_data_partial_balance_remove ( gint partial_balance_number )
{
    struct_partial_balance *partial_balance;

    partial_balance = gsb_data_partial_balance_get_structure ( partial_balance_number );

    if ( !partial_balance )
        return FALSE;
    
    /* remove the partial_balance from the list */
    partial_balance_list = g_slist_remove ( partial_balance_list, partial_balance );

    _gsb_data_partial_balance_free ( partial_balance );

    return TRUE;
}


/**
 * set a new number for the partial_balance
 * normally used only while loading the file because
 * the number are given automaticly
 *
 * \param partial_balance_number the number of the partial_balance
 * \param new_no_partial_balance the new number of the partial_balance
 *
 * \return the new number or 0 if the partial_balance doen't exist
 * */
gint gsb_data_partial_balance_set_new_number ( gint partial_balance_number,
                        gint new_no_partial_balance )
{
    struct_partial_balance *partial_balance;

    partial_balance = gsb_data_partial_balance_get_structure ( partial_balance_number );

    if ( !partial_balance )
        return 0;

    partial_balance -> partial_balance_number = new_no_partial_balance;
    return new_no_partial_balance;
}


/**
 * return the liste_cptes of the partial_balance
 *
 * \param partial_balance_number the number of the partial_balance
 *
 * \return the liste_cptes of the partial_balance or NULL if fail
 * */
const gchar *gsb_data_partial_balance_get_liste_cptes ( gint partial_balance_number )
{
    struct_partial_balance *partial_balance;

    partial_balance = gsb_data_partial_balance_get_structure ( partial_balance_number );

    if ( !partial_balance )
        return NULL;

    return partial_balance -> liste_cptes;
}


/**
 * set the liste_cptes of the partial_balance
 * the value is dupplicate in memory
 *
 * \param partial_balance_number the number of the partial_balance
 * \param liste_cptes of the partial_balance
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_partial_balance_set_liste_cptes ( gint partial_balance_number,
                        const gchar *liste_cptes )
{
    struct_partial_balance *partial_balance;

    partial_balance = gsb_data_partial_balance_get_structure ( partial_balance_number );

    if ( !partial_balance )
        return FALSE;

    /* we free the last name */
    if ( partial_balance -> liste_cptes )
        g_free (partial_balance -> liste_cptes);

    /* and copy the new one */
    partial_balance -> liste_cptes = my_strdup ( liste_cptes );

    return TRUE;
}


/**
 * return the name of the partial_balance
 *
 * \param partial_balance_number the number of the partial_balance
 *
 * \return the name of the partial_balance or NULL if fail
 * */
const gchar *gsb_data_partial_balance_get_name ( gint partial_balance_number )
{
    struct_partial_balance *partial_balance;

    partial_balance = gsb_data_partial_balance_get_structure ( partial_balance_number );

    if ( !partial_balance )
        return NULL;

    return partial_balance -> balance_name;
}


/**
 * set the name of the import_rule
 * the value is dupplicate in memory
 *
 * \param import_rule_number the number of the import_rule
 * \param name the name of the import_rule
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_partial_balance_set_name ( gint partial_balance_number,
                        const gchar *name )
{
    struct_partial_balance *partial_balance;

    partial_balance = gsb_data_partial_balance_get_structure ( partial_balance_number );

    if ( !partial_balance )
        return FALSE;

    /* we free the last name */
    if ( partial_balance -> balance_name )
        g_free (partial_balance -> balance_name);

    /* and copy the new one */
    partial_balance -> balance_name = my_strdup ( name );

    return TRUE;
}


/** get the kind of the partial_balance
 * \param partial_balance_number no of the partial_balance
 * \return partial_balance type or 0 if the partial_balance doesn't exist
 * */
kind_account gsb_data_partial_balance_get_kind ( gint partial_balance_number )
{
    struct_partial_balance *partial_balance;

    partial_balance = gsb_data_partial_balance_get_structure ( partial_balance_number );

    if ( !partial_balance )
        return 0;

    return partial_balance -> kind;
}


/** set the kind of the partial_balance
 * \param partial_balance_number no of the partial_balance
 * \param kind type to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_partial_balance_set_kind ( gint partial_balance_number,
                        kind_account kind )
{
    struct_partial_balance *partial_balance;

    partial_balance = gsb_data_partial_balance_get_structure ( partial_balance_number );

    if ( !partial_balance )
        return FALSE;

    partial_balance -> kind = kind;

    return TRUE;
}


/** get the currency of the partial_balance
 * \param partial_balance_number no of the partial_balance
 * \return partial_balance currency or 0 if the partial_balance doesn't exist
 * */
gint gsb_data_partial_balance_get_currency ( gint partial_balance_number )
{
    struct_partial_balance *partial_balance;

    partial_balance = gsb_data_partial_balance_get_structure ( partial_balance_number );

    if ( !partial_balance )
        return 0;

    return partial_balance -> currency;
}


/** set the currency of the partial_balance
 * \param partial_balance_number no of the partial_balance
 * \param partial_balance_kind type to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_partial_balance_set_currency ( gint partial_balance_number,
                        gint currency )
{
    struct_partial_balance *partial_balance;

    partial_balance = gsb_data_partial_balance_get_structure ( partial_balance_number );

    if ( !partial_balance )
        return FALSE;

    partial_balance -> currency = currency;

    return TRUE;
}


gchar *gsb_data_partial_balance_get_marked_balance ( gint partial_balance_number )
{
    struct_partial_balance *partial_balance;
    gsb_real solde = null_real;
    gchar **tab;
    gint i;

    partial_balance = gsb_data_partial_balance_get_structure ( partial_balance_number );

    if ( !partial_balance )
        return 0;

    tab = g_strsplit ( partial_balance -> liste_cptes, ";", 0 );
    for ( i = 0; tab[i]; i++ )
    {
        solde = gsb_real_add ( solde,
                        gsb_data_account_get_marked_balance (
                        utils_str_atoi ( tab[i] ) ) );
    }
    return gsb_real_get_string_with_currency (solde, partial_balance -> currency, TRUE);
}


gchar *gsb_data_partial_balance_get_current_balance ( gint partial_balance_number )
{
    struct_partial_balance *partial_balance;
    gsb_real solde = null_real;
    gchar **tab;
    gint i;

    partial_balance = gsb_data_partial_balance_get_structure ( partial_balance_number );

    if ( !partial_balance )
        return 0;

    tab = g_strsplit ( partial_balance -> liste_cptes, ";", 0 );
    for ( i = 0; tab[i]; i++ )
    {
        solde = gsb_real_add ( solde,
                        gsb_data_account_get_current_balance (
                        utils_str_atoi ( tab[i] ) ) );
    }
    return gsb_real_get_string_with_currency (solde, partial_balance -> currency, TRUE);
}


gint gsb_data_partial_balance_cmp_func ( struct_partial_balance *partial_balance_1,
                        struct_partial_balance *partial_balance_2 )
{
    if ( partial_balance_1 -> partial_balance_number 
        < partial_balance_2 -> partial_balance_number )
        return -1;
    else if ( partial_balance_1 -> partial_balance_number 
        == partial_balance_2 -> partial_balance_number )
        return 0;
    else
        return 1;
}
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
