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
#include "./utils_dates.h"
#include "./gsb_data_account.h"
#include "./gsb_data_budget.h"
#include "./gsb_data_category.h"
#include "./gsb_data_fyear.h"
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


/*START_STATIC*/
static gboolean bet_data_update_div ( SH *sh, gint transaction_number,
                        gint sub_div );
/*END_STATIC*/

/*START_EXTERN*/
extern gboolean balances_with_scheduled;
extern gsb_real null_real;
extern GtkWidget *window;
/*END_EXTERN*/


/* pointeurs définis en fonction du type de données catégories ou IB */
gint (*ptr_div) ( gint transaction_num );
gint (*ptr_sub_div) ( gint transaction_num );
gchar* (*ptr_div_name) ( gint div_num, gint sub_div, const gchar *return_value_error );


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
        ptr_div = &gsb_data_transaction_get_category_number;
        ptr_sub_div = &gsb_data_transaction_get_sub_category_number;
        ptr_div_name = &gsb_data_category_get_name;
    }
    else
    {
        ptr_div = &gsb_data_transaction_get_budgetary_number;
        ptr_sub_div = &gsb_data_transaction_get_sub_budgetary_number;
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
gchar *bet_data_get_div_name (gint div_num,
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
gboolean bet_data_populate_div ( gint transaction_number,
                        GHashTable  *list_div )
{
    gint div = 0;
    gint sub_div = 0;
    SH *sh = NULL;

    div = ptr_div ( transaction_number );
    if ( div > 0 )
        sub_div = ptr_sub_div ( transaction_number );
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
 *
 *
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

    if ( sub_div == -1 )
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
    sh -> list_sub_div = g_hash_table_new_full ( g_int_hash,
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
void free_struct_historical (SH *sh)
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
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
#endif /* ENABLE_BALANCE_ESTIMATE */
