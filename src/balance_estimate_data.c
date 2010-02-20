/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C) 2007 Dominique Parisot                                   */
/*          zionly@free.org                                                   */
/*          2008-2010 Pierre Biava (grisbi@pierre.biava.name)                 */
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

#include "include.h"
#include <config.h>

#ifdef ENABLE_BALANCE_ESTIMATE

/*START_INCLUDE*/
#include "balance_estimate_data.h"
#include "./dialog.h"
#include "./utils_dates.h"
#include "./gsb_data_account.h"
#include "./gsb_data_budget.h"
#include "./gsb_data_category.h"
#include "./gsb_data_fyear.h"
#include "./gsb_data_mix.h"
#include "./gsb_data_payee.h"
#include "./gsb_data_scheduled.h"
#include "./gsb_data_transaction.h"
#include "./gsb_fyear.h"
#include "./gsb_scheduler.h"
#include "./gsb_transactions_list_sort.h"
#include "./main.h"
#include "./include.h"
#include "./structures.h"
#include "./traitement_variables.h"
#include "./erreur.h"
#include "./utils.h"
/*END_INCLUDE*/

typedef struct _hist_div struct_hist_div;


struct _hist_div
{
    gint account_nb;
    gint div_number;
    gboolean div_full;
    gint sub_div_nb;
    GHashTable *sub_div_list;
    gsb_real amount;
};


/*START_STATIC*/
static gboolean bet_data_update_div ( SH *sh, gint transaction_number,
                        gint sub_div );
static struct_hist_div *initialise_struct_hist_div ( void );
static void free_struct_hist_div ( struct_hist_div *bet_hist_div );
/*END_STATIC*/


/*START_EXTERN*/
extern gboolean balances_with_scheduled;
extern gsb_real null_real;
extern GtkWidget *window;
/*END_EXTERN*/


/* pointeurs définis en fonction du type de données catégories ou IB */
gint (*ptr_div) ( gint transaction_num, gboolean is_transaction );
gint (*ptr_sub_div) ( gint transaction_num, gboolean is_transaction );
gchar* (*ptr_div_name) ( gint div_num, gint sub_div, const gchar *return_value_error );


/* liste des div et sub_div cochées dans la vue des divisions */
static GHashTable *bet_hist_div_list;


/**
 * used when we init all the global variables
 * 
 * \param
 * 
 * \return FALSE
 * */
gboolean bet_data_init_variables ( void )
{
    bet_hist_div_list = g_hash_table_new_full ( g_str_hash,
                        g_str_equal,
                        (GDestroyNotify) g_free,
                        (GDestroyNotify) free_struct_hist_div );
    return FALSE;

}


/**
 *
 *
 *
 *
 * */
gboolean bet_data_add_div_hist ( gint account_nb,
                        gint div_number,
                        gint sub_div_nb,
                        gsb_real amount )
{
    gchar *key;
    gchar *sub_key;
    struct_hist_div *shd;
devel_debug_int (account_nb);
    if ( account_nb == 0 )
        key = g_strconcat ("0:", utils_str_itoa ( div_number ), NULL );
    else
        key = g_strconcat ( utils_str_itoa ( account_nb ), ":",
                        utils_str_itoa ( div_number ), NULL );

    if ( ( shd = g_hash_table_lookup ( bet_hist_div_list, key ) ) )
    {
        if ( sub_div_nb > 0 )
        {
            sub_key = utils_str_itoa ( sub_div_nb );
            if (  !g_hash_table_lookup ( bet_hist_div_list, sub_key ) )
            {
                struct_hist_div *sub_shd;

                sub_shd = initialise_struct_hist_div ( );
                if ( !sub_shd )
                {
                    dialogue_error_memory ( );
                    return FALSE;
                }
                sub_shd -> sub_div_nb = sub_div_nb;
                sub_shd -> amount = amount;
                g_hash_table_insert ( shd -> sub_div_list, sub_key, sub_shd );
            }
        }
    }
    else
    {
        shd = initialise_struct_hist_div ( );
        if ( !shd )
        {
            dialogue_error_memory ( );
            return 0;
        }
        shd -> account_nb = account_nb;
        shd -> div_number = div_number;
        shd -> sub_div_nb = sub_div_nb;
        if ( sub_div_nb > 0 )
        {
            struct_hist_div *sub_shd;

            sub_shd = initialise_struct_hist_div ( );
            if ( !sub_shd )
            {
                dialogue_error_memory ( );
                return FALSE;
            }
            sub_key = utils_str_itoa ( sub_div_nb );
            sub_shd -> sub_div_nb = sub_div_nb;
            sub_shd -> amount = amount;
            g_hash_table_insert ( shd -> sub_div_list, sub_key, sub_shd );
        }
        g_hash_table_insert ( bet_hist_div_list, key, shd );
    }

    return TRUE;
}


/**
 *
 *
 *
 *
 * */
gboolean bet_data_remove_div_hist ( gint account_nb, gint div_number, gint sub_div_nb )
{
    gchar *key;
    char *sub_key;
    struct_hist_div *shd;
devel_debug_int (account_nb);
    if ( account_nb == 0 )
        key = g_strconcat ("0:", utils_str_itoa ( div_number ), NULL );
    else
        key = g_strconcat ( utils_str_itoa ( account_nb ), ":",
                        utils_str_itoa ( div_number ), NULL );

    if ( ( shd = g_hash_table_lookup ( bet_hist_div_list, key ) ) )
    {
        if ( sub_div_nb > 0 )
        {
            sub_key = utils_str_itoa ( sub_div_nb );
            g_hash_table_remove ( shd -> sub_div_list, sub_key );
        }
        if ( g_hash_table_size ( shd -> sub_div_list ) == 0 )
            g_hash_table_remove ( bet_hist_div_list, key );
    }
    else
        return FALSE;

    return TRUE;
}


/**
 *
 *
 *
 *
 * */
gboolean bet_data_search_div_hist ( gint account_nb, gint div_number, gint sub_div_nb )
{
    gchar *key;
    gchar *sub_key;
    struct_hist_div *shd;
    
    if ( account_nb == 0 )
        key = g_strconcat ("0:", utils_str_itoa ( div_number ), NULL );
    else
        key = g_strconcat ( utils_str_itoa ( account_nb ), ":",
                        utils_str_itoa ( div_number ), NULL );

    if ( ( shd = g_hash_table_lookup ( bet_hist_div_list, key ) ) )
    {
        if ( g_hash_table_size ( shd -> sub_div_list ) == 0 )
        {
            g_free ( key );
            return TRUE;
        }
        else if ( sub_div_nb > 0 );
        {
            sub_key = utils_str_itoa ( sub_div_nb );
            if (  g_hash_table_lookup ( shd -> sub_div_list, sub_key ) )
            {
                g_free ( key );
                g_free ( sub_key );
                return TRUE;
            }
            g_free ( sub_key );
        }
    }

    g_free ( key );
    return FALSE;
}


/**
 *
 *
 *
 *
 * */
/**
 *
 *
 *
 *
 * */
gboolean bet_data_set_div_ptr ( gint type_div )
{
    if ( type_div == 0 )
    {
        ptr_div = &gsb_data_mix_get_category_number;
        ptr_sub_div = &gsb_data_mix_get_sub_category_number;
        ptr_div_name = &gsb_data_category_get_name;
    }
    else
    {
        ptr_div = &gsb_data_mix_get_budgetary_number;
        ptr_sub_div = &gsb_data_mix_get_sub_budgetary_number;
        ptr_div_name = &gsb_data_budget_get_name;
    }

    return FALSE;
}
/**
 *
 *
 *
 *
 * */
gint bet_data_get_div_number ( gint transaction_number, gboolean is_transaction )
{
    return ptr_div ( transaction_number, is_transaction );
}


/**
 *
 *
 *
 *
 * */
gint bet_data_get_sub_div_nb ( gint transaction_number, gboolean is_transaction )
{
    return ptr_sub_div ( transaction_number, is_transaction );
}


/**
 *
 *
 *
 *
 * */
gchar *bet_data_get_div_name ( gint div_num,
                        gint sub_div,
                        const gchar *return_value_error )
{
    return g_strdup ( ptr_div_name ( div_num, sub_div, FALSE ) );
}


/**
 *
 *
 *
 *
 * */
gsb_real bet_data_get_div_amount ( gint account_nb, gint div_number, gint sub_div_nb )
{
    gchar *key;
    struct_hist_div *shd;
    gsb_real amount;

    if ( account_nb == 0 )
        key = g_strconcat ("0:", utils_str_itoa ( div_number ), NULL );
    else
        key = g_strconcat ( utils_str_itoa ( account_nb ), ":",
                        utils_str_itoa ( div_number ), NULL );

    if ( ( shd = g_hash_table_lookup ( bet_hist_div_list, key ) ) )
    {
        if ( sub_div_nb == 0 )
            amount = shd -> amount;
        else
        {
            gchar *sub_key;
            struct_hist_div *sub_shd;

            sub_key = utils_str_itoa ( sub_div_nb );
            if ( ( sub_shd = g_hash_table_lookup ( shd -> sub_div_list, sub_key ) ) )
                amount = sub_shd -> amount;
            else
                amount = null_real;
            g_free ( sub_key );
        }
    }
    else
        amount = null_real;
    g_free ( key );

    return amount;
}


/**
 *
 *
 *
 *
 * */
gboolean bet_data_set_div_amount ( gint account_nb,
                        gint div_number,
                        gint sub_div_nb,
                        gsb_real amount )
{
    gchar *key;
    struct_hist_div *shd;

    if ( account_nb == 0 )
        key = g_strconcat ("0:", utils_str_itoa ( div_number ), NULL );
    else
        key = g_strconcat ( utils_str_itoa ( account_nb ), ":",
                        utils_str_itoa ( div_number ), NULL );

    if ( ( shd = g_hash_table_lookup ( bet_hist_div_list, key ) ) )
    {
        if ( sub_div_nb == 0 )
            shd -> amount = amount;
        else
        {
            gchar *sub_key;
            struct_hist_div *sub_shd;

            sub_key = utils_str_itoa ( sub_div_nb );
            if ( ( sub_shd = g_hash_table_lookup ( shd -> sub_div_list, sub_key ) ) )
                sub_shd -> amount = amount;
            g_free ( sub_key );
        }
    }

    g_free ( key );

    return FALSE;
}


/**
 *
 *
 *
 *
 * */
gboolean bet_data_get_div_full ( gint account_nb, gint div_number )
{
    gchar *key;
    struct_hist_div *shd;

    if ( account_nb == 0 )
        key = g_strconcat ("0:", utils_str_itoa ( div_number ), NULL );
    else
        key = g_strconcat ( utils_str_itoa ( account_nb ), ":",
                        utils_str_itoa ( div_number ), NULL );

    if ( ( shd = g_hash_table_lookup ( bet_hist_div_list, key ) ) )
        return shd -> div_full;
    else
        return FALSE;
}


/**
 *
 *
 *
 *
 * */
gboolean bet_data_set_div_full ( gint account_nb, gint div_number, gboolean full )
{
    gchar *key;
    struct_hist_div *shd;
devel_debug_int (full);
    if ( account_nb == 0 )
        key = g_strconcat ("0:", utils_str_itoa ( div_number ), NULL );
    else
        key = g_strconcat ( utils_str_itoa ( account_nb ), ":",
                        utils_str_itoa ( div_number ), NULL );

    if ( ( shd = g_hash_table_lookup ( bet_hist_div_list, key ) ) )
        shd -> div_full = full;

    return FALSE;
}



/**
 * Ajoute les données de la transaction à la division et la sous division
 * création des nouvelles divisions et si existantes ajout des données
 * par appel à bet_data_update_div ( )
 *
 * */
gboolean bet_data_populate_div ( gint transaction_number,
                        gboolean is_transaction,
                        GHashTable  *list_div )
{
    gint div = 0;
    gint sub_div = 0;
    SH *sh = NULL;

    div = ptr_div ( transaction_number, is_transaction );
    if ( div > 0 )
        sub_div = ptr_sub_div ( transaction_number, is_transaction );
    else
        return FALSE;
    
    if ( (sh = g_hash_table_lookup ( list_div, utils_str_itoa ( div ) ) ) )
        bet_data_update_div ( sh, transaction_number, sub_div );
    else
    {
        sh = initialise_struct_historical ( );
        sh -> div = div;
        sh -> account_nb = gsb_data_transaction_get_account_number ( transaction_number );
        bet_data_update_div ( sh, transaction_number, sub_div );
        g_hash_table_insert ( list_div, utils_str_itoa ( div ), sh );
    }

    return FALSE;
}


/**
 * Ajout des données à la division et création de la sous division si elle 
 * n'existe pas.
 *
 *
 * */
gboolean bet_data_update_div ( SH *sh, gint transaction_number, gint sub_div )
{
    SBR *sbr = ( SBR*) sh -> sbr;
    gsb_real amount;
    SH *tmp_sh = NULL;

    amount = gsb_data_transaction_get_amount ( transaction_number );
    sbr-> current_balance = gsb_real_add ( sbr -> current_balance, amount );

    if ( sub_div < 1 )
        return FALSE;

    if ( ( tmp_sh = g_hash_table_lookup ( sh -> list_sub_div, utils_str_itoa ( sub_div ) ) ) )
        bet_data_update_div ( tmp_sh, transaction_number, -1 );
    else
    {
        tmp_sh = initialise_struct_historical ( );
        tmp_sh -> div = sub_div;
        tmp_sh -> account_nb = gsb_data_transaction_get_account_number ( transaction_number );
        bet_data_update_div ( tmp_sh, transaction_number, -1 );
        g_hash_table_insert ( sh -> list_sub_div, utils_str_itoa ( sub_div ), tmp_sh );
    }

    return FALSE;
}


/**
 *
 *
 *
 *
 * */
SBR *initialise_struct_bet_range ( void )
{
	SBR	*sbr;
	
	sbr = g_malloc ( sizeof ( SBR ) );
    sbr -> first_pass = TRUE;
    sbr -> min_date = NULL;
    sbr -> max_date = NULL;
    sbr -> min_balance = null_real;
    sbr -> max_balance = null_real;
    sbr -> current_balance = null_real;

	return sbr;
}


/**
 *
 *
 *
 *
 * */
void free_struct_bet_range ( SBR *sbr )
{
    if ( sbr-> min_date )
        g_date_free ( sbr-> min_date );
    if ( sbr-> max_date )
        g_date_free ( sbr-> max_date );

    g_free ( sbr );
}


SH *initialise_struct_historical ( void )
{
	SH	*sh;

	sh = g_malloc ( sizeof ( SH ) );
    sh -> sbr = initialise_struct_bet_range ( );
    sh -> list_sub_div = g_hash_table_new_full ( g_str_hash,
                        g_str_equal,
                        NULL,
                        (GDestroyNotify) free_struct_historical );
	return sh;
}


/**
 *
 *
 *
 *
 * */
void free_struct_historical ( SH *sh )
{

    if ( sh -> sbr )
        free_struct_bet_range ( sh -> sbr );
    if ( sh -> list_sub_div )
        g_hash_table_remove_all ( sh -> list_sub_div );

    g_free ( sh );
}


/**
 *
 *
 *
 *
 * */
struct_hist_div *initialise_struct_hist_div ( void )
{
    struct_hist_div *shd;

    shd = g_malloc ( sizeof ( struct_hist_div ) );
    shd -> account_nb = 0;
    shd -> div_number = 0;
    shd -> div_full = FALSE;
    shd -> sub_div_nb = -1;
    shd -> sub_div_list = g_hash_table_new_full ( g_str_hash,
                        g_str_equal,
                        (GDestroyNotify) g_free,
                        (GDestroyNotify) free_struct_hist_div );
    shd -> amount = null_real;

    return shd;
}


/**
 *
 *
 *
 *
 * */
void free_struct_hist_div ( struct_hist_div *shd )
{
    if ( shd -> sub_div_list )
        g_hash_table_remove_all ( shd -> sub_div_list );

    g_free ( shd );
}

/**
 *
 *
 *
 *
 * */


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
#endif /* ENABLE_BALANCE_ESTIMATE */
