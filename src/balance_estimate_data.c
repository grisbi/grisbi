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

/* ./configure --with-balance-estimate */

#include "include.h"
#include <config.h>

#ifdef ENABLE_BALANCE_ESTIMATE

/*START_INCLUDE*/
#include "balance_estimate_data.h"
#include "./balance_estimate_tab.h"
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
#include "./gsb_file_save.h"
#include "./gsb_fyear.h"
#include "./gsb_scheduler.h"
#include "./gsb_scheduler_list.h"
#include "./gsb_transactions_list_sort.h"
#include "./main.h"
#include "./navigation.h"
#include "./include.h"
#include "./structures.h"
#include "./traitement_variables.h"
#include "./erreur.h"
#include "./utils.h"
/*END_INCLUDE*/


/*START_STATIC*/
static struct_futur_data *bet_data_future_copy_struct ( struct_futur_data *scheduled );
static void bet_data_future_set_max_number ( gint number );
static gboolean bet_data_update_div ( SH *sh, gint transaction_number,
                        gint sub_div );
static void struct_free_hist_div ( struct_hist_div *bet_hist_div );
/*END_STATIC*/


/*START_EXTERN*/
extern gboolean balances_with_scheduled;
extern GtkWidget *notebook_general;
extern gsb_real null_real;
extern GtkWidget *window;
/*END_EXTERN*/


/* pointeurs définis en fonction du type de données catégories ou IB */
gint (*ptr_div) ( gint transaction_num, gboolean is_transaction );
gint (*ptr_sub_div) ( gint transaction_num, gboolean is_transaction );
gchar* (*ptr_div_name) ( gint div_num, gint sub_div, const gchar *return_value_error );


/* liste des div et sub_div cochées dans la vue des divisions */
static GHashTable *bet_hist_div_list;

/** the hashtable which contains all the bet_future structures */
static GHashTable *bet_future_list;

static gint future_number;

/* force la mise à jour des données */
static gboolean bet_maj = FALSE;


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
                        (GDestroyNotify) struct_free_hist_div );
    bet_future_list = g_hash_table_new_full ( g_str_hash,
                        g_str_equal,
                        (GDestroyNotify) g_free,
                        (GDestroyNotify) struct_free_bet_future );
    future_number = 0;

    return FALSE;
}


/**
 *
 *
 *
 *
 * */
gboolean bet_data_get_maj ( void )
{
    return bet_maj;
}


/**
 *
 *
 *
 *
 * */
void bet_data_set_maj ( gboolean maj )
{
    bet_maj = maj;
}


/**
 *
 *
 *
 *
 * */
gboolean bet_data_hist_add_div ( gint account_nb,
                        gint div_number,
                        gint sub_div_nb )
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
        shd -> div_edited = FALSE;
        shd -> amount = null_real;

        if ( sub_div_nb > 0 )
        {
            sub_key = utils_str_itoa ( sub_div_nb );
            if (  !g_hash_table_lookup ( shd -> sub_div_list, sub_key ) )
            {
                struct_hist_div *sub_shd;

                sub_shd = struct_initialise_hist_div ( );
                if ( !sub_shd )
                {
                    dialogue_error_memory ( );
                    return FALSE;
                }
                sub_shd -> div_number = sub_div_nb;
                g_hash_table_insert ( shd -> sub_div_list, sub_key, sub_shd );
            }
            else
            {
                shd -> div_edited = FALSE;
                shd -> amount = null_real;
            }
        }
    }
    else
    {
        shd = struct_initialise_hist_div ( );
        if ( !shd )
        {
            dialogue_error_memory ( );
            return 0;
        }
        shd -> account_nb = account_nb;
        shd -> div_number = div_number;
        if ( sub_div_nb > 0 )
        {
            struct_hist_div *sub_shd;

            sub_shd = struct_initialise_hist_div ( );
            if ( !sub_shd )
            {
                dialogue_error_memory ( );
                return FALSE;
            }
            sub_key = utils_str_itoa ( sub_div_nb );
            sub_shd -> div_number = sub_div_nb;
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
void bet_data_insert_div_hist ( struct_hist_div *shd, struct_hist_div *sub_shd )
{
    gchar *key;
    gchar *sub_key;
    struct_hist_div *tmp_shd;

    if ( shd -> account_nb == 0 )
        key = g_strconcat ("0:", utils_str_itoa ( shd -> div_number ), NULL );
    else
        key = g_strconcat ( utils_str_itoa ( shd -> account_nb ), ":",
                        utils_str_itoa ( shd -> div_number ), NULL );

    if ( ( tmp_shd = g_hash_table_lookup ( bet_hist_div_list, key ) ) )
    {
        tmp_shd -> div_edited = shd -> div_edited;
        tmp_shd -> amount = shd -> amount;

        if ( sub_shd )
        {
            sub_key = utils_str_itoa ( sub_shd -> div_number );
            g_hash_table_insert ( tmp_shd -> sub_div_list, sub_key, sub_shd );
        }
    }
    else
    {
        if ( sub_shd )
        {
            sub_key = utils_str_itoa ( sub_shd -> div_number );
            g_hash_table_insert ( shd -> sub_div_list, sub_key, sub_shd );
        }
        g_hash_table_insert ( bet_hist_div_list, key, shd );
    }
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
    
    //~ devel_debug ( g_strdup_printf ("account_nb = %d div_number = %d sub_div_nb = %d",
                                    //~ account_nb, div_number, sub_div_nb));
    
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
 * return TRUE si la division et sous division existe.
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
        if ( sub_div_nb == 0 )
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
    return g_strdup ( ptr_div_name ( div_num, sub_div, NULL ) );
}


/**
 *
 *
 *
 *
 * */
gboolean bet_data_get_div_edited ( gint account_nb, gint div_number, gint sub_div_nb )
{
    gchar *key;
    struct_hist_div *shd;
    gboolean edited;

    if ( account_nb == 0 )
        key = g_strconcat ("0:", utils_str_itoa ( div_number ), NULL );
    else
        key = g_strconcat ( utils_str_itoa ( account_nb ), ":",
                        utils_str_itoa ( div_number ), NULL );

    if ( ( shd = g_hash_table_lookup ( bet_hist_div_list, key ) ) )
    {
        if ( sub_div_nb == 0 )
            edited = shd -> div_edited;
        else
        {
            gchar *sub_key;
            struct_hist_div *sub_shd;

            sub_key = utils_str_itoa ( sub_div_nb );
            if ( ( sub_shd = g_hash_table_lookup ( shd -> sub_div_list, sub_key ) ) )
                edited = sub_shd -> div_edited;
            else
                edited = FALSE;
            g_free ( sub_key );
        }
    }
    else
        edited = FALSE;
    g_free ( key );

    return edited;
}


/**
 *
 *
 *
 *
 * */
gboolean bet_data_set_div_edited ( gint account_nb,
                        gint div_number,
                        gint sub_div_nb,
                        gboolean edited )
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
            shd -> div_edited = edited;
        else
        {
            gchar *sub_key;
            struct_hist_div *sub_shd;

            sub_key = utils_str_itoa ( sub_div_nb );
            if ( ( sub_shd = g_hash_table_lookup ( shd -> sub_div_list, sub_key ) ) )
                sub_shd -> div_edited = edited;
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
gint bet_data_get_div_children ( gint account_nb, gint div_number )
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
        return g_hash_table_size ( shd -> sub_div_list );
    }
    else
        return 0;
}


/**
 *
 *
 *
 *
 * */
gsb_real bet_data_hist_get_div_amount ( gint account_nb, gint div_number, gint sub_div_nb )
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
gint bet_data_get_selected_currency ( void )
{
    gint selected_account;
    gint currency_number;

    selected_account = gsb_gui_navigation_get_current_account ( );
    if ( selected_account == -1 )
        return 0;

    currency_number = gsb_data_account_get_currency ( selected_account );

    return currency_number;
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
        sh = struct_initialise_bet_historical ( );
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
        tmp_sh = struct_initialise_bet_historical ( );
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
GPtrArray *bet_data_get_strings_to_save ( void )
{
    GPtrArray *tab = NULL;
    gchar *tmp_str = NULL;
    GHashTableIter iter;
    gpointer key, value;

    if ( g_hash_table_size ( bet_hist_div_list ) == 0 )
        return NULL;

    tab = g_ptr_array_new ( );

    g_hash_table_iter_init ( &iter, bet_hist_div_list );
    while ( g_hash_table_iter_next ( &iter, &key, &value ) )
    {
        struct_hist_div *shd = ( struct_hist_div* ) value;

        if ( g_hash_table_size ( shd -> sub_div_list ) == 0 )
        {
            tmp_str = g_markup_printf_escaped ( "\t<Bet_historical Nb=\"%d\" Ac=\"%d\" "
                        "Div=\"%d\" Edit=\"%d\" Damount=\"%s\" SDiv=\"%d\" "
                        "SEdit=\"%d\" SDamount=\"%s\" />\n",
                        tab -> len + 1,
                        shd -> account_nb,
                        shd -> div_number,
                        shd -> div_edited,
                        gsb_real_save_real_to_string ( shd -> amount, 2 ),
                        0, 0, "0.00" );

            g_ptr_array_add ( tab, tmp_str );
        }
        else
        {
            GHashTableIter new_iter;

            g_hash_table_iter_init ( &new_iter, shd -> sub_div_list );
            while ( g_hash_table_iter_next ( &new_iter, &key, &value ) )
            {
                struct_hist_div *sub_shd = ( struct_hist_div* ) value;

                tmp_str = g_markup_printf_escaped ( "\t<Bet_historical Nb=\"%d\" Ac=\"%d\" "
                        "Div=\"%d\" Edit=\"%d\" Damount=\"%s\" SDiv=\"%d\" "
                        "SEdit=\"%d\" SDamount=\"%s\" />\n",
                        tab -> len + 1,
                        shd -> account_nb,
                        shd -> div_number,
                        shd -> div_edited,
                        gsb_real_save_real_to_string ( shd -> amount, 2 ),
                        sub_shd -> div_number,
                        sub_shd -> div_edited,
                        gsb_real_save_real_to_string ( sub_shd -> amount, 2 ) );

                g_ptr_array_add ( tab, tmp_str );
            }
        }
    }

    g_hash_table_iter_init ( &iter, bet_future_list );
    while ( g_hash_table_iter_next ( &iter, &key, &value ) )
    {
        struct_futur_data *scheduled = ( struct_futur_data* ) value;
        gchar *amount;
        gchar *date;
        gchar *limit_date;

        /* set the real */
        amount = gsb_real_save_real_to_string ( scheduled -> amount, 2 );

        /* set the dates */
        date = gsb_format_gdate_safe ( scheduled -> date );
        limit_date = gsb_format_gdate_safe ( scheduled -> limit_date );

        tmp_str = g_markup_printf_escaped ( "\t<Bet_future Nb=\"%d\" Dt=\"%s\" Ac=\"%d\" "
                        "Am=\"%s\" Pa=\"%d\" Ca=\"%d\" Sca=\"%d\" Pn=\"%d\" "
                        "Fi=\"%d\" Bu=\"%d\" Sbu=\"%d\" No=\"%s\" Au=\"0\" "
                        "Pe=\"%d\" Pei=\"%d\" Pep=\"%d\" Dtl=\"%s\" Mo=\"%d\" />\n",
					    scheduled -> number,
					    my_safe_null_str ( date ),
					    scheduled -> account_number,
					    my_safe_null_str ( amount ),
					    scheduled -> party_number,
					    scheduled -> category_number,
					    scheduled -> sub_category_number,
					    scheduled -> payment_number,
					    scheduled -> fyear_number,
					    scheduled -> budgetary_number,
					    scheduled -> sub_budgetary_number,
					    my_safe_null_str ( scheduled -> notes ),
					    scheduled -> frequency,
					    scheduled -> user_interval,
					    scheduled -> user_entry,
					    my_safe_null_str ( limit_date ),
                        scheduled -> mother_row );

        g_ptr_array_add ( tab, tmp_str );

        g_free (amount);
        g_free (date);
        g_free (limit_date);
    }

    return tab;
}


/**
 * supprime de la liste bet_hist_div_list les divisions sous divisions
 * inexistantes dans list_div.
 *
 *
 * */
void bet_data_synchronise_hist_div_list ( GHashTable  *list_div )
{
    GHashTableIter iter;
    gpointer key, value;
    SH *sh = NULL;

    g_hash_table_iter_init ( &iter, bet_hist_div_list );
    while ( g_hash_table_iter_next ( &iter, &key, &value ) ) 
    {
        struct_hist_div *shd = ( struct_hist_div* ) value;
        GHashTableIter new_iter;

        sh = g_hash_table_lookup ( list_div, utils_str_itoa ( shd -> div_number ) );
        if ( sh == NULL )
            bet_data_remove_div_hist ( shd -> account_nb, shd -> div_number, 0 );
        else
        {
            g_hash_table_iter_init ( &new_iter, shd -> sub_div_list );
            while ( g_hash_table_iter_next ( &new_iter, &key, &value ) )
            {
                struct_hist_div *sub_shd = ( struct_hist_div* ) value;

                if ( !g_hash_table_lookup ( sh -> list_sub_div, utils_str_itoa (
                 sub_shd -> div_number ) ) )
                {
                    bet_data_remove_div_hist ( shd -> account_nb,
                                shd -> div_number,
                                sub_shd -> div_number );
                    g_hash_table_iter_init ( &new_iter, shd -> sub_div_list );
                }
            }
        }
    }
}
/**
 *
 *
 *
 *
 * */
SBR *struct_initialise_bet_range ( void )
{
	SBR	*sbr;
	
	sbr = g_malloc0 ( sizeof ( SBR ) );
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
void struct_free_bet_range ( SBR *sbr )
{
    if ( sbr-> min_date )
        g_date_free ( sbr-> min_date );
    if ( sbr-> max_date )
        g_date_free ( sbr-> max_date );

    g_free ( sbr );
}


/**
 *
 *
 *
 *
 * */
SH *struct_initialise_bet_historical ( void )
{
	SH	*sh;

	sh = g_malloc0 ( sizeof ( SH ) );
    sh -> sbr = struct_initialise_bet_range ( );
    sh -> list_sub_div = g_hash_table_new_full ( g_str_hash,
                        g_str_equal,
                        NULL,
                        (GDestroyNotify) struct_free_bet_historical );
	return sh;
}


/**
 *
 *
 *
 *
 * */
void struct_free_bet_historical ( SH *sh )
{

    if ( sh -> sbr )
        struct_free_bet_range ( sh -> sbr );
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
struct_hist_div *struct_initialise_hist_div ( void )
{
    struct_hist_div *shd;

    shd = g_malloc0 ( sizeof ( struct_hist_div ) );
    shd -> account_nb = 0;
    shd -> div_number = 0;
    shd -> div_edited = FALSE;
    shd -> amount = null_real;
    shd -> sub_div_list = g_hash_table_new_full ( g_str_hash,
                        g_str_equal,
                        (GDestroyNotify) g_free,
                        (GDestroyNotify) struct_free_hist_div );

    return shd;
}


/**
 *
 *
 *
 *
 * */
void struct_free_hist_div ( struct_hist_div *shd )
{
    if ( shd -> sub_div_list )
        g_hash_table_remove_all ( shd -> sub_div_list );

    g_free ( shd );
}

/**
 * Sélectionne les onglets du module gestion budgétaire en fonction du type de compte
 *
 */
void bet_data_select_bet_pages ( gint account_number )
{
    GtkWidget *notebook;
    GtkWidget *page;
    kind_account kind;
    gint current_page;

    notebook = g_object_get_data ( G_OBJECT ( notebook_general ), "account_notebook");
    kind = gsb_data_account_get_kind ( account_number );
    current_page = gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook ) );

    switch ( kind )
    {
    case GSB_TYPE_BANK:
    case GSB_TYPE_CASH:
        page = gtk_notebook_get_nth_page ( GTK_NOTEBOOK ( notebook ), 1 );
        gtk_widget_show ( page );
        page = gtk_notebook_get_nth_page ( GTK_NOTEBOOK ( notebook ), 2 );
        gtk_widget_show ( page );
        bet_array_update_estimate_tab ( );
        break;
    case GSB_TYPE_LIABILITIES:
        page = gtk_notebook_get_nth_page ( GTK_NOTEBOOK ( notebook ), 1 );
        gtk_widget_show ( page );
        if ( current_page == 2 )
            gtk_notebook_set_current_page ( GTK_NOTEBOOK ( notebook ), 1 );
        page = gtk_notebook_get_nth_page ( GTK_NOTEBOOK ( notebook ), 2 );
        gtk_widget_hide ( page );
        bet_array_update_estimate_tab ( );
        break;
    case GSB_TYPE_ASSET:
        if ( current_page < 2 )
            gtk_notebook_set_current_page ( GTK_NOTEBOOK ( notebook ), 0 );
        page = gtk_notebook_get_nth_page ( GTK_NOTEBOOK ( notebook ), 1 );
        gtk_widget_hide ( page );
        page = gtk_notebook_get_nth_page ( GTK_NOTEBOOK ( notebook ), 2 );
        gtk_widget_hide ( page );
        break;
    }
}


/**
 *
 *
 *
 *
 * */
struct_futur_data *struct_initialise_bet_future ( void )
{
    struct_futur_data *sfd;

    sfd = g_malloc0 ( sizeof ( struct_futur_data ) );

    sfd -> date = NULL;
    sfd -> amount = null_real;
    sfd -> notes = NULL;
    sfd -> limit_date = NULL;

    return sfd;
}


/**
 *
 *
 *
 *
 * */
void struct_free_bet_future ( struct_futur_data *scheduled )
{
    if ( scheduled -> date )
        g_date_free ( scheduled -> date );
    if ( scheduled -> limit_date )
        g_date_free ( scheduled -> limit_date );
    if ( scheduled -> notes )
        g_free ( scheduled -> notes );

    g_free ( scheduled );
}


/**
 * add lines creates in the tab_array
 *
 *
 *
 * */
gboolean bet_data_future_add_lines ( struct_futur_data *scheduled )
{
    gchar *key;
    
    future_number ++;

    if ( scheduled -> frequency == 0 )
    {
        if ( scheduled -> account_number == 0 )
            key = g_strconcat ("0:", utils_str_itoa ( future_number ), NULL );
        else
            key = g_strconcat ( utils_str_itoa ( scheduled -> account_number ), ":",
                        utils_str_itoa ( future_number ), NULL );

        scheduled -> number = future_number;
        g_hash_table_insert ( bet_future_list, key, scheduled );
    }
    else
    {
        GDate *date;
        GDate *date_max;
        gint mother_row;
        struct_futur_data *new_sch = NULL;

        mother_row = future_number;

        date_max = bet_data_array_get_date_max ( scheduled -> account_number );
        /* we don't change the initial date */
        date = gsb_date_copy ( scheduled -> date );
        while ( date != NULL && g_date_valid ( date ) && g_date_compare ( date, date_max ) <= 0 )
        {
            if ( scheduled -> account_number == 0 )
                key = g_strconcat ("0:", utils_str_itoa ( future_number ), NULL );
            else
                key = g_strconcat ( utils_str_itoa ( scheduled -> account_number ), ":",
                        utils_str_itoa ( future_number ), NULL );

            if ( mother_row == future_number )
                new_sch = scheduled;
            else
                new_sch -> mother_row = mother_row;

            new_sch -> number = future_number;
            g_hash_table_insert ( bet_future_list, key, new_sch );

            date = bet_data_futur_get_next_date ( new_sch, date );
            if ( date == NULL )
                break;
            future_number ++;
            new_sch = bet_data_future_copy_struct ( scheduled );
            new_sch -> date = date;
        }
        g_date_free ( date_max );
    }

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );

    return TRUE;
}


/**
 * add lines from file
 *
 *
 *
 * */
gboolean bet_data_future_set_lines_from_file ( struct_futur_data *scheduled )
{
    gchar *key;

        if ( scheduled -> account_number == 0 )
            key = g_strconcat ("0:", utils_str_itoa ( scheduled -> number ), NULL );
        else
            key = g_strconcat ( utils_str_itoa ( scheduled -> account_number ), ":",
                        utils_str_itoa ( scheduled -> number ), NULL );

        bet_data_future_set_max_number ( scheduled -> number );

        g_hash_table_insert ( bet_future_list, key, scheduled );

    return TRUE;
}


/**
 *
 *
 *
 *
 * */
void bet_data_future_set_max_number ( gint number )
{
    if ( number >  future_number )
        future_number = number;
}


/**
 *
 *
 *
 *
 * */
GHashTable *bet_data_future_get_list ( void )
{
    return bet_future_list;
}


/**
 * find and return the next date after the given date for the given futur data
 * 
 *
 * \param struct_futur_data 
 * \param date the current date, we want the next one after that one
 *
 * \return a newly allocated date, the next date or NULL if over the limit
 * */
GDate *bet_data_futur_get_next_date ( struct_futur_data *scheduled,
				     const GDate *date )
{
    GDate *return_date;

    if ( !scheduled
	 ||
	 !scheduled -> frequency
	 ||
	 !date
	 ||
	 !g_date_valid ( date ) )
	return NULL;

    /* we don't change the initial date */
    return_date = gsb_date_copy (date);

    switch ( scheduled -> frequency )
    {
	case SCHEDULER_PERIODICITY_ONCE_VIEW:
	    return NULL;
	    break;

	case SCHEDULER_PERIODICITY_WEEK_VIEW:
	    g_date_add_days ( return_date, 7 );
	    break;

	case SCHEDULER_PERIODICITY_MONTH_VIEW:
	    g_date_add_months ( return_date, 1 );
	    break;

	case SCHEDULER_PERIODICITY_TWO_MONTHS_VIEW:
	    g_date_add_months ( return_date, 2 );
	    break;

	case SCHEDULER_PERIODICITY_TRIMESTER_VIEW:
	    g_date_add_months ( return_date, 3 );
	    break;

	case SCHEDULER_PERIODICITY_YEAR_VIEW:
	    g_date_add_years ( return_date, 1 );
	    break;

	case SCHEDULER_PERIODICITY_CUSTOM_VIEW:
	    if ( scheduled -> user_entry <= 0 )
	    {
            g_date_free (return_date);
            return NULL;
	    }

	    switch ( scheduled -> user_interval )
	    {
		case PERIODICITY_DAYS:
		    g_date_add_days ( return_date, scheduled -> user_entry);
		    break;

		case PERIODICITY_WEEKS:
		    g_date_add_days ( return_date, scheduled -> user_entry * 7 );
		    break;

		case PERIODICITY_MONTHS:
		    g_date_add_months ( return_date, scheduled -> user_entry );
		    break;

		case PERIODICITY_YEARS:
		    g_date_add_years ( return_date, scheduled -> user_entry );
		    break;
	    }
	    break;
    }

    if ( scheduled -> limit_date
	 &&
	 g_date_compare ( return_date, scheduled -> limit_date ) > 0 )
    {
        g_date_free (return_date);
        return_date = NULL;
    }
    
    return ( return_date );
}


/**
 *
 *
 *
 *
 * */
struct_futur_data *bet_data_future_copy_struct ( struct_futur_data *scheduled )
{
    struct_futur_data *new_sch;

    new_sch = struct_initialise_bet_future ( );

    new_sch ->  number = scheduled -> number;
    new_sch ->  account_number = scheduled -> account_number;

    if ( g_date_valid ( scheduled -> date ) )
        new_sch -> date = gsb_date_copy ( scheduled -> date );
    else
        new_sch -> date = NULL;

    new_sch -> amount = gsb_real_new ( scheduled -> amount.mantissa,
                        scheduled -> amount.exponent );
    new_sch -> fyear_number = scheduled -> fyear_number;
    new_sch -> payment_number = scheduled -> payment_number;

    new_sch -> party_number = scheduled -> party_number;
    new_sch -> category_number = scheduled -> category_number;
    new_sch -> sub_category_number = scheduled -> sub_category_number;
    new_sch -> budgetary_number = scheduled -> budgetary_number;
    new_sch -> sub_budgetary_number = scheduled -> sub_budgetary_number;
    new_sch -> notes = g_strdup ( scheduled -> notes );

    new_sch -> frequency = scheduled -> frequency;
    new_sch -> user_interval = scheduled -> user_interval;
    new_sch -> user_entry = scheduled -> user_entry;

    if ( scheduled -> limit_date && g_date_valid ( scheduled -> limit_date ) )
        new_sch -> limit_date = gsb_date_copy ( scheduled -> limit_date );
    else
        new_sch -> limit_date = NULL;

    new_sch -> mother_row = scheduled -> mother_row;

    return new_sch;
}
/**
 *
 *
 *
 *
 * */
gboolean bet_data_future_remove_line ( gint account_number, gint number )
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init ( &iter, bet_future_list );
    while (g_hash_table_iter_next ( &iter, &key, &value ) ) 
    {
        struct_futur_data *scheduled = ( struct_futur_data *) value;

        if ( account_number != scheduled -> account_number 
         ||
         number != scheduled -> number)
            continue;

        g_hash_table_iter_remove ( &iter );

        break;
    }

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );

    return FALSE;
}


/**
 *
 *
 *
 *
 * */
gboolean bet_data_future_remove_lines ( gint account_number,
                        gint number,
                        gint mother_row )
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init ( &iter, bet_future_list );
    while (g_hash_table_iter_next ( &iter, &key, &value ) ) 
    {
        struct_futur_data *scheduled = ( struct_futur_data *) value;

        if ( account_number != scheduled -> account_number )
            continue;

        if ( number == scheduled -> number )
            g_hash_table_iter_remove ( &iter );
        else if ( number == scheduled -> mother_row )
            g_hash_table_iter_remove ( &iter );
        else if ( mother_row > 0 && mother_row == scheduled -> number )
            g_hash_table_iter_remove ( &iter );
        else if ( mother_row > 0 && mother_row == scheduled -> mother_row )
            g_hash_table_iter_remove ( &iter );
    }

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );

    return FALSE;
}


/**
 * retourne la date max d'interrogation pour les prévisions
 *
 *
 *
 * */
GDate *bet_data_array_get_date_max ( gint account_number )
{
    GDate *date_min;
    GDate *date_max;

    date_min = gsb_data_account_get_bet_start_date ( account_number );

    date_max = gsb_date_copy ( date_min );

    if ( g_date_get_day ( date_min ) == 1 )
    {
        g_date_add_months (date_max, gsb_data_account_get_bet_months ( account_number ) - 1 );
        date_max = gsb_date_get_last_day_of_month ( date_max );
    }
    else
    {
        g_date_add_months (date_max, gsb_data_account_get_bet_months ( account_number ) );
        g_date_subtract_days ( date_max, 1 );
    }

    return date_max;
}


/**
 * modify futures data lines 
 *
 *
 *
 * */
gboolean bet_data_future_modify_lines ( struct_futur_data *scheduled )
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init ( &iter, bet_future_list );
    while ( g_hash_table_iter_next ( &iter, &key, &value ) )
    {
        struct_futur_data *sch = ( struct_futur_data *) value;

        if ( scheduled -> number == sch -> number )
        {
            if ( scheduled -> account_number == 0 )
                key = g_strconcat ("0:", utils_str_itoa ( scheduled -> number ), NULL );
            else
                key = g_strconcat ( utils_str_itoa ( scheduled -> account_number ), ":",
                                utils_str_itoa ( scheduled -> number ), NULL );

            g_hash_table_replace ( bet_future_list, key, scheduled );
        }
    }

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );

    return TRUE;
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
#endif /* ENABLE_BALANCE_ESTIMATE */
