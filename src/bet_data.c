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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "bet_data.h"
#include "bet_future.h"
#include "bet_hist.h"
#include "bet_tab.h"
#include "dialog.h"
#include "fenetre_principale.h"
#include "gsb_data_account.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_mix.h"
#include "gsb_data_partial_balance.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_transaction.h"
#include "gsb_file.h"
#include "gsb_file_save.h"
#include "gsb_scheduler_list.h"
#include "gsb_transactions_list.h"
#include "navigation.h"
#include "structures.h"
#include "traitement_variables.h"
#include "utils_dates.h"
#include "utils_real.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/


/*START_STATIC*/
static GDate *bet_data_futur_get_next_date ( FuturData *scheduled,
                        const GDate *date,
                        const GDate *date_max );
static FuturData *bet_data_future_copy_struct ( FuturData *scheduled );
static void bet_data_future_set_max_number ( gint number );
static gchar *bet_data_get_key ( gint account_number, gint div_number );
static gboolean bet_data_update_div ( BetHist *sh,
                        gint transaction_number,
                        gint sub_div,
                        gint type_de_transaction,
                        gsb_real amount );
static void struct_free_bet_future ( FuturData *scheduled );
static void struct_free_bet_range ( BetRange *sbr );
static void struct_free_bet_transfert ( TransfertData *transfert );
static void struct_free_hist_div ( HistDiv *shd );
static BetHist *struct_initialise_bet_historical ( void );
/*END_STATIC*/


/*START_EXTERN*/
/*END_EXTERN*/


/* pointeurs définis en fonction du type de données catégories ou IB */
gint (*ptr_div) ( gint transaction_num, gboolean is_transaction );
gint (*ptr_sub_div) ( gint transaction_num, gboolean is_transaction );
gint (*ptr_type) ( gint no_div );
gchar* (*ptr_div_name) ( gint div_num, gint sub_div, const gchar *return_value_error );


/* liste des div et sub_div cochées dans la vue des divisions */
static GHashTable *bet_hist_div_list;

/** the hashtable which contains all the bet_future structures */
static GHashTable *bet_future_list;
static gint future_number;

/** the hashtable for account_balance */
static GHashTable *bet_transfert_list;
static gint transfert_number;


/**
 * Sélectionne les onglets du module gestion budgétaire en fonction du type de compte
 *
 */
void bet_data_select_bet_pages ( gint account_number )
{
    GtkWidget *page;
    GtkWidget *account_page;
    gint current_page;
    BetTypeOnglets bet_show_onglets;

    devel_debug_int ( account_number );
    bet_show_onglets = gsb_data_account_get_bet_show_onglets ( account_number );
    account_page = gsb_gui_get_account_page ();
    current_page = gtk_notebook_get_current_page ( GTK_NOTEBOOK ( account_page ) );

    switch ( bet_show_onglets )
    {
    case BET_ONGLETS_PREV:
        page = gtk_notebook_get_nth_page ( GTK_NOTEBOOK ( account_page ), GSB_ESTIMATE_PAGE );
        gtk_widget_show ( page );
        page = gtk_notebook_get_nth_page ( GTK_NOTEBOOK ( account_page ), GSB_HISTORICAL_PAGE );
        gtk_widget_show ( page );
        page = gtk_notebook_get_nth_page ( GTK_NOTEBOOK ( account_page ), GSB_FINANCE_PAGE );
        gtk_widget_hide ( page );
        bet_historical_g_signal_unblock_tree_view ( );
        gsb_data_account_set_bet_maj ( account_number, BET_MAJ_ALL );
        if ( current_page < GSB_PROPERTIES_PAGE
         &&
         gsb_data_account_get_bet_use_budget ( account_number ) == FALSE )
            gtk_notebook_set_current_page ( GTK_NOTEBOOK ( account_page ), GSB_TRANSACTIONS_PAGE );
        if ( current_page == GSB_FINANCE_PAGE )
            gtk_notebook_set_current_page ( GTK_NOTEBOOK ( account_page ), GSB_ESTIMATE_PAGE );
        break;
    case BET_ONGLETS_HIST:
        page = gtk_notebook_get_nth_page ( GTK_NOTEBOOK ( account_page ), GSB_ESTIMATE_PAGE );
        gtk_widget_hide ( page );
        page = gtk_notebook_get_nth_page ( GTK_NOTEBOOK ( account_page ), GSB_HISTORICAL_PAGE );
        gtk_widget_show ( page );
        page = gtk_notebook_get_nth_page ( GTK_NOTEBOOK ( account_page ), GSB_FINANCE_PAGE );
        gtk_widget_hide ( page );
        if ( current_page == GSB_ESTIMATE_PAGE || current_page == GSB_FINANCE_PAGE )
            gtk_notebook_set_current_page ( GTK_NOTEBOOK ( account_page ), GSB_TRANSACTIONS_PAGE );
        bet_historical_g_signal_block_tree_view ( );
        gsb_data_account_set_bet_maj ( account_number, BET_MAJ_HISTORICAL );
        break;
    case BET_ONGLETS_CAP:
        page = gtk_notebook_get_nth_page ( GTK_NOTEBOOK ( account_page ), GSB_ESTIMATE_PAGE );
        gtk_widget_hide ( page );
        page = gtk_notebook_get_nth_page ( GTK_NOTEBOOK ( account_page ), GSB_HISTORICAL_PAGE );
        gtk_widget_hide ( page );
        page = gtk_notebook_get_nth_page ( GTK_NOTEBOOK ( account_page ), GSB_FINANCE_PAGE );
        gtk_widget_show ( page );
        if ( current_page == GSB_ESTIMATE_PAGE || current_page == GSB_HISTORICAL_PAGE )
            gtk_notebook_set_current_page ( GTK_NOTEBOOK ( account_page ), GSB_TRANSACTIONS_PAGE );
        break;
    default:
        if ( current_page < GSB_PROPERTIES_PAGE )
            gtk_notebook_set_current_page ( GTK_NOTEBOOK ( account_page ), GSB_TRANSACTIONS_PAGE );
        page = gtk_notebook_get_nth_page ( GTK_NOTEBOOK ( account_page ), GSB_ESTIMATE_PAGE );
        gtk_widget_hide ( page );
        page = gtk_notebook_get_nth_page ( GTK_NOTEBOOK ( account_page ), GSB_HISTORICAL_PAGE );
        gtk_widget_hide ( page );
        page = gtk_notebook_get_nth_page ( GTK_NOTEBOOK ( account_page ), GSB_FINANCE_PAGE );
        gtk_widget_hide ( page );
        break;
    }
}


/*
 * Met à jour les données à afficher dans les différentes vues du module
 *
 *
 */
void bet_data_update_bet_module ( gint account_number, gint page )
{
    gint type_maj;

    devel_debug_int ( account_number );

    /* test account number */
    if ( account_number == -1 )
        return;

    type_maj = gsb_data_account_get_bet_maj ( account_number );
    if ( type_maj == BET_MAJ_FALSE )
        return;

    if ( page == -1 && gsb_gui_navigation_get_current_page () == GSB_ACCOUNT_PAGE )
        page = gtk_notebook_get_current_page ( GTK_NOTEBOOK ( gsb_gui_get_account_page () ) );

    switch ( page )
    {
        case GSB_ESTIMATE_PAGE:
            bet_array_update_estimate_tab ( account_number, type_maj );
            gsb_data_account_set_bet_maj ( account_number, BET_MAJ_FALSE );
            break;
        case GSB_HISTORICAL_PAGE:
            if ( type_maj == BET_MAJ_ALL )
            {
                bet_historical_populate_data ( account_number );
                gsb_data_account_set_bet_maj ( account_number, BET_MAJ_ESTIMATE );
            }
            else if ( type_maj ==  BET_MAJ_HISTORICAL )
            {
                bet_historical_populate_data ( account_number );
                gsb_data_account_set_bet_maj ( account_number, BET_MAJ_FALSE );
            }
            bet_historical_set_page_title ( account_number );
            break;
        default:
            break;
    }
}


/**
 * used when we init all the global variables
 *
 * \param
 *
 * \return FALSE
 * */
gboolean bet_data_init_variables ( void )
{
    if ( bet_hist_div_list )
        g_hash_table_destroy ( bet_hist_div_list );
    bet_hist_div_list = g_hash_table_new_full ( g_str_hash,
                        g_str_equal,
                        (GDestroyNotify) g_free,
                        (GDestroyNotify) struct_free_hist_div );

    if ( bet_future_list )
        g_hash_table_destroy ( bet_future_list );
    bet_future_list = g_hash_table_new_full ( g_str_hash,
                        g_str_equal,
                        (GDestroyNotify) g_free,
                        (GDestroyNotify) struct_free_bet_future );
    future_number = 0;

    if ( bet_transfert_list )
        g_hash_table_destroy ( bet_transfert_list );
    bet_transfert_list = g_hash_table_new_full ( g_str_hash,
                        g_str_equal,
                        (GDestroyNotify) g_free,
                        (GDestroyNotify) struct_free_bet_transfert );
    transfert_number = 0;

    return FALSE;
}


/**
 * used to free the global variables
 *
 * \param
 *
 * \return
 * */
void bet_data_free_variables ( void )
{
    g_hash_table_destroy ( bet_hist_div_list );
    g_hash_table_destroy ( bet_future_list );
    g_hash_table_destroy ( bet_transfert_list );
}


/**
 *
 *
 *
 *
 * */
gboolean bet_data_hist_add_div ( gint account_number,
                        gint div_number,
                        gint sub_div_nb )
{
    gchar *key = NULL;
    gchar *sub_key = NULL;
    HistDiv *shd;

    key = bet_data_get_key ( account_number, div_number );

    if ( ( shd = g_hash_table_lookup ( bet_hist_div_list, key ) ) )
    {
        g_free ( key );
        shd -> div_edited = FALSE;
        shd -> amount = null_real;

        if ( sub_div_nb > 0 )
        {
            sub_key = utils_str_itoa ( sub_div_nb );
            if (  !g_hash_table_lookup ( shd -> sub_div_list, sub_key ) )
            {
                HistDiv *sub_shd;

                sub_shd = struct_initialise_hist_div ( );
                if ( !sub_shd )
                {
                    dialogue_error_memory ( );
                    g_free ( sub_key );
                    return FALSE;
                }
                sub_shd -> div_number = sub_div_nb;
                g_hash_table_insert ( shd -> sub_div_list, sub_key, sub_shd );
            }
            else
            {
                shd -> div_edited = FALSE;
                shd -> amount = null_real;
                g_free ( sub_key );
            }
        }
    }
    else
    {
        shd = struct_initialise_hist_div ( );
        if ( !shd )
        {
            dialogue_error_memory ( );
            return FALSE;
        }
        shd -> account_nb = account_number;
        shd -> origin = gsb_data_account_get_bet_hist_data ( account_number );
        shd -> div_number = div_number;
        if ( sub_div_nb > 0 )
        {
            HistDiv *sub_shd;

            sub_shd = struct_initialise_hist_div ( );
            if ( !sub_shd )
            {
                dialogue_error_memory ( );
                struct_free_hist_div  ( shd );
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
void bet_data_insert_div_hist ( HistDiv *shd, HistDiv *sub_shd )
{
    gchar *key;
    gchar *sub_key;
    HistDiv *tmp_shd;

    key = bet_data_get_key ( shd -> account_nb, shd -> div_number );

    if ( ( tmp_shd = g_hash_table_lookup ( bet_hist_div_list, key ) ) )
    {
        g_free ( key );
        tmp_shd -> div_edited = shd -> div_edited;
        tmp_shd -> amount = shd -> amount;

        if ( sub_shd )
        {
            sub_key = utils_str_itoa ( sub_shd -> div_number );
            g_hash_table_insert ( tmp_shd -> sub_div_list, sub_key, sub_shd );
        }
		struct_free_hist_div (shd);
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
gboolean bet_data_remove_div_hist ( gint account_number, gint div_number, gint sub_div_nb )
{
    gchar *key;
    char *sub_key;
    HistDiv *shd;
    gboolean return_val = FALSE;

    key = bet_data_get_key ( account_number, div_number );

    if ( ( shd = g_hash_table_lookup ( bet_hist_div_list, key ) ) )
    {
        return_val = TRUE;
        if ( sub_div_nb > 0 )
        {
            sub_key = utils_str_itoa ( sub_div_nb );
            g_hash_table_remove ( shd -> sub_div_list, sub_key );
            g_free ( sub_key );
        }
        if ( g_hash_table_size ( shd -> sub_div_list ) == 0 )
            g_hash_table_remove ( bet_hist_div_list, key );
    }
    g_free ( key );

    return return_val;
}


/**
 * return TRUE si la division et sous division existe.
 *
 *
 *
 * */
gboolean bet_data_search_div_hist ( gint account_number, gint div_number, gint sub_div_nb )
{
    gchar *key;
    gchar *sub_key;
    gint origin;
    HistDiv *shd;
    gboolean return_val = FALSE;

    key = bet_data_get_key ( account_number, div_number );

    origin = gsb_data_account_get_bet_hist_data ( account_number );

    if ( ( shd = g_hash_table_lookup ( bet_hist_div_list, key ) )
     && shd -> origin == origin )
    {
        if ( sub_div_nb == 0 )
            return_val = TRUE;
        else if ( sub_div_nb > 0 )
        {
            sub_key = utils_str_itoa ( sub_div_nb );
            if ( g_hash_table_lookup ( shd -> sub_div_list, sub_key ) )
                return_val = TRUE;
            g_free ( sub_key );
        }
    }
    g_free ( key );

    return return_val;
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
        ptr_type = &gsb_data_category_get_type;
        ptr_div_name = &gsb_data_category_get_name;
    }
    else
    {
        ptr_div = &gsb_data_mix_get_budgetary_number;
        ptr_sub_div = &gsb_data_mix_get_sub_budgetary_number;
        ptr_type = &gsb_data_budget_get_type;
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
gint bet_data_get_div_type ( gint div_number )
{
    return ptr_type ( div_number );
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
gboolean bet_data_get_div_edited ( gint account_number, gint div_number, gint sub_div_nb )
{
    gchar *key;
    gint origin;
    HistDiv *shd;
    gboolean edited;

    key = bet_data_get_key ( account_number, div_number );

    origin = gsb_data_account_get_bet_hist_data ( account_number );

    if ( ( shd = g_hash_table_lookup ( bet_hist_div_list, key ) )
     && shd -> origin == origin )
    {
        if ( sub_div_nb == 0 )
            edited = shd -> div_edited;
        else
        {
            gchar *sub_key;
            HistDiv *sub_shd;

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
    HistDiv *shd;

    key = bet_data_get_key ( account_nb, div_number );

    if ( ( shd = g_hash_table_lookup ( bet_hist_div_list, key ) ) )
    {
        if ( sub_div_nb == 0 )
            shd -> div_edited = edited;
        else
        {
            gchar *sub_key;
            HistDiv *sub_shd;

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
gsb_real bet_data_hist_get_div_amount ( gint account_nb, gint div_number, gint sub_div_nb )
{
    gchar *key;
    HistDiv *shd;
    gsb_real amount;

    key = bet_data_get_key ( account_nb, div_number );

    if ( ( shd = g_hash_table_lookup ( bet_hist_div_list, key ) ) )
    {
        if ( sub_div_nb == 0 )
            amount = shd -> amount;
        else
        {
            gchar *sub_key;
            HistDiv *sub_shd;

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
    HistDiv *shd;

    key = bet_data_get_key ( account_nb, div_number );

    if ( ( shd = g_hash_table_lookup ( bet_hist_div_list, key ) ) )
    {
        if ( sub_div_nb == 0 )
            shd -> amount = amount;
        else
        {
            gchar *sub_key;
            HistDiv *sub_shd;

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
                        GHashTable  *list_div,
                        gint type_de_transaction,
                        TransactionCurrentFyear *tcf )
{
    gint div = 0;
    gint sub_div = 0;
    gchar *key;
    BetHist *sh = NULL;
    gsb_real amount;

    div = ptr_div ( transaction_number, is_transaction );
    if ( div > 0 )
        sub_div = ptr_sub_div ( transaction_number, is_transaction );
    else
        return FALSE;

    amount = gsb_data_transaction_get_amount ( transaction_number );
    /* on complète la structure tcf pour les graphiques */
    if ( tcf )
    {
        tcf->div_nb = div;
        tcf->sub_div_nb = sub_div;
        tcf->amount = amount;
    }

    key = utils_str_itoa ( div );
    if ( (sh = g_hash_table_lookup ( list_div, key ) ) )
    {
        bet_data_update_div ( sh, transaction_number, sub_div, type_de_transaction, amount );
        g_free ( key );
    }
    else
    {
        sh = struct_initialise_bet_historical ( );
        sh -> div = div;
        sh -> account_nb = gsb_data_transaction_get_account_number ( transaction_number );
        bet_data_update_div ( sh, transaction_number, sub_div, type_de_transaction, amount );
        g_hash_table_insert ( list_div, key, sh );
    }

    /* return value */
    return FALSE;
}


/**
 * Ajout des données à la division et création de la sous division si elle
 * n'existe pas.
 *
 *
 * */
gboolean bet_data_update_div ( BetHist *sh,
                        gint transaction_number,
                        gint sub_div,
                        gint type_de_transaction,
                        gsb_real amount )
{
    BetRange *sbr = ( BetRange*) sh -> sbr;
    gchar *key;
    BetHist *tmp_sh = NULL;

    switch ( type_de_transaction )
    {
        case 0:
            sbr-> current_balance = gsb_real_add ( sbr -> current_balance, amount );
            break;
        case 1:
            sbr-> current_fyear = gsb_real_add ( sbr -> current_fyear, amount );
            break;
        case 2:
            sbr-> current_balance = gsb_real_add ( sbr -> current_balance, amount );
            sbr-> current_fyear = gsb_real_add ( sbr -> current_fyear, amount );
            break;
    }

    if ( sub_div < 1 )
        return FALSE;

    key = utils_str_itoa ( sub_div );
    if ( ( tmp_sh = g_hash_table_lookup ( sh -> list_sub_div, key ) ) )
    {
        bet_data_update_div ( tmp_sh, transaction_number, -1, type_de_transaction, amount );
        g_free ( key );
    }
    else
    {
        tmp_sh = struct_initialise_bet_historical ( );
        tmp_sh -> div = sub_div;
        tmp_sh -> account_nb = gsb_data_transaction_get_account_number ( transaction_number );
        bet_data_update_div ( tmp_sh, transaction_number, -1, type_de_transaction, amount );
        g_hash_table_insert ( sh -> list_sub_div, key, tmp_sh );
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
    gint index = 0;

    if ( g_hash_table_size ( bet_hist_div_list ) == 0
     &&
        g_hash_table_size ( bet_future_list ) == 0
     &&
        g_hash_table_size ( bet_transfert_list ) == 0 )
        return NULL;

    tab = g_ptr_array_new ( );

    g_hash_table_iter_init ( &iter, bet_hist_div_list );
    while ( g_hash_table_iter_next ( &iter, &key, &value ) )
    {
        HistDiv *shd = ( HistDiv* ) value;

        if ( g_hash_table_size ( shd -> sub_div_list ) == 0 )
        {
            tmp_str = g_markup_printf_escaped ( "\t<Bet_historical Nb=\"%d\" Ac=\"%d\" "
                        "Ori=\"%d\" Div=\"%d\" Edit=\"%d\" Damount=\"%s\" SDiv=\"%d\" "
                        "SEdit=\"%d\" SDamount=\"%s\" />\n",
                        tab -> len + 1,
                        shd -> account_nb,
                        shd -> origin,
                        shd -> div_number,
                        shd -> div_edited,
                        gsb_real_safe_real_to_string ( shd -> amount,
                        gsb_data_account_get_currency_floating_point ( shd -> account_nb ) ),
                        0, 0, "0.00" );

            g_ptr_array_add ( tab, tmp_str );
        }
        else
        {
            GHashTableIter new_iter;

            g_hash_table_iter_init ( &new_iter, shd -> sub_div_list );
            while ( g_hash_table_iter_next ( &new_iter, &key, &value ) )
            {
                HistDiv *sub_shd = ( HistDiv* ) value;
                gint floating_point;

                floating_point = gsb_data_account_get_currency_floating_point ( shd -> account_nb );
                tmp_str = g_markup_printf_escaped ( "\t<Bet_historical Nb=\"%d\" Ac=\"%d\" "
                        "Ori=\"%d\" Div=\"%d\" Edit=\"%d\" Damount=\"%s\" SDiv=\"%d\" "
                        "SEdit=\"%d\" SDamount=\"%s\" />\n",
                        tab -> len + 1,
                        shd -> account_nb,
                        shd -> origin,
                        shd -> div_number,
                        shd -> div_edited,
                        gsb_real_safe_real_to_string ( shd -> amount, floating_point ),
                        sub_shd -> div_number,
                        sub_shd -> div_edited,
                        gsb_real_safe_real_to_string ( sub_shd -> amount, floating_point ) );

                g_ptr_array_add ( tab, tmp_str );
            }
        }
    }

    g_hash_table_iter_init ( &iter, bet_future_list );
    while ( g_hash_table_iter_next ( &iter, &key, &value ) )
    {
        FuturData *scheduled = ( FuturData* ) value;
        gchar *amount;
        gchar *date;
        gchar *limit_date;

        /* set the real */
        amount = gsb_real_safe_real_to_string ( scheduled -> amount,
                        gsb_data_account_get_currency_floating_point ( scheduled -> account_number ) );

        /* set the dates */
        date = gsb_format_gdate_safe ( scheduled -> date );
        limit_date = gsb_format_gdate_safe ( scheduled -> limit_date );

        tmp_str = g_markup_printf_escaped ( "\t<Bet_future Nb=\"%d\" Dt=\"%s\" Ac=\"%d\" "
                        "Am=\"%s\" Pa=\"%d\" IsT=\"%d\" Tra=\"%d\" Ca=\"%d\" Sca=\"%d\" "
                        "Pn=\"%d\" Fi=\"%d\" Bu=\"%d\" Sbu=\"%d\" No=\"%s\" Au=\"0\" "
                        "Pe=\"%d\" Pei=\"%d\" Pep=\"%d\" Dtl=\"%s\" Mo=\"%d\" />\n",
                        scheduled -> number,
                        my_safe_null_str ( date ),
                        scheduled -> account_number,
                        my_safe_null_str ( amount ),
                        scheduled -> party_number,
                        scheduled -> is_transfert,
                        scheduled -> account_transfert,
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

    g_hash_table_iter_init ( &iter, bet_transfert_list );
    index = 0;
    while ( g_hash_table_iter_next ( &iter, &key, &value ) )
    {
        TransfertData *transfert = ( TransfertData* ) value;
        gchar *date;
        gchar *date_bascule;

        /* set the dates */
        date = gsb_format_gdate_safe ( transfert->date );
        date_bascule = gsb_format_gdate_safe ( transfert->date_bascule );

        tmp_str = g_markup_printf_escaped ( "\t<Bet_transfert Nb=\"%d\" Dt=\"%s\" Ac=\"%d\" "
                        "Ty=\"%d\" Ra=\"%d\" Rt=\"%d\" Dd=\"%d\" Dtb=\"%s\" Mlbd=\"%d\" "
                        "Pa=\"%d\" Pn=\"%d\" Ca=\"%d\" Sca=\"%d\" Bu=\"%d\" Sbu=\"%d\" "
                        "CPa=\"%d\" CCa=\"%d\" CSca=\"%d\" CBu=\"%d\" CSbu=\"%d\" />\n",
                        ++index,
                        my_safe_null_str ( date ),
                        transfert->account_number,
                        transfert->type,
                        transfert->replace_account,
                        transfert->replace_transaction,
                        transfert->direct_debit,
                        my_safe_null_str ( date_bascule ),
                        transfert->main_last_banking_date,
                        transfert->main_payee_number,
                        transfert->main_payment_number,
                        transfert->main_category_number,
                        transfert->main_sub_category_number,
                        transfert->main_budgetary_number,
                        transfert->main_sub_budgetary_number,
                        transfert->card_payee_number,
                        transfert->card_category_number,
                        transfert->card_sub_category_number,
                        transfert->card_budgetary_number,
                        transfert->card_sub_budgetary_number );

        g_ptr_array_add ( tab, tmp_str );

        g_free (date);
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
    BetHist *sh = NULL;

    g_hash_table_iter_init ( &iter, bet_hist_div_list );
    while ( g_hash_table_iter_next ( &iter, &key, &value ) )
    {
        HistDiv *shd = ( HistDiv* ) value;
        GHashTableIter new_iter;

        sh = g_hash_table_lookup ( list_div, utils_str_itoa ( shd -> div_number ) );
        if ( sh == NULL )
            bet_data_remove_div_hist ( shd -> account_nb, shd -> div_number, 0 );
        else
        {
            g_hash_table_iter_init ( &new_iter, shd -> sub_div_list );
            while ( g_hash_table_iter_next ( &new_iter, &key, &value ) )
            {
                HistDiv *sub_shd = ( HistDiv* ) value;

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
BetRange *struct_initialise_bet_range ( void )
{
	BetRange	*sbr;

	sbr = g_malloc0 ( sizeof ( BetRange ) );
    sbr -> first_pass = TRUE;
    sbr -> current_fyear = null_real;
    sbr -> current_balance = null_real;

	return sbr;
}


/**
 *
 *
 *
 *
 * */
void struct_free_bet_range ( BetRange *sbr )
{
    g_free ( sbr );
}


/**
 *
 *
 *
 *
 * */
BetHist *struct_initialise_bet_historical ( void )
{
	BetHist	*sh;

	sh = g_malloc0 ( sizeof (BetHist) );
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
void struct_free_bet_historical (BetHist *sh )
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
HistDiv *struct_initialise_hist_div ( void )
{
    HistDiv *shd;

    shd = g_malloc0 ( sizeof ( HistDiv ) );
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
void struct_free_hist_div ( HistDiv *shd )
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
FuturData *struct_initialise_bet_future ( void )
{
    FuturData *sfd;

    sfd = g_malloc0 ( sizeof ( FuturData ) );

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
void struct_free_bet_future ( FuturData *scheduled )
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
 * add lines creates in the bet_future_list
 *
 *
 *
 * */
gboolean bet_data_future_add_lines ( FuturData *scheduled )
{
    gchar *key;

    future_number ++;

    if ( scheduled -> frequency == 0 )
    {
        key = bet_data_get_key ( scheduled -> account_number, future_number );

        scheduled -> number = future_number;
        g_hash_table_insert ( bet_future_list, key, scheduled );
    }
    else
    {
        GDate *date;
        GDate *date_max;
        gint mother_row;
        FuturData *new_sch = NULL;

        mother_row = future_number;

        date_max = bet_data_array_get_date_max ( scheduled -> account_number );

        /* we don't change the initial date */
        date = gsb_date_copy ( scheduled -> date );
        while ( date != NULL && g_date_valid ( date ) )
        {
            key = bet_data_get_key ( scheduled -> account_number, future_number );

            if ( mother_row == future_number )
                new_sch = scheduled;
            else
                new_sch -> mother_row = mother_row;

            new_sch -> number = future_number;
            g_hash_table_insert ( bet_future_list, key, new_sch );

            date = bet_data_futur_get_next_date ( new_sch, date, date_max );
            if ( date == NULL )
                break;
            future_number ++;
            new_sch = bet_data_future_copy_struct ( scheduled );
            new_sch -> date = date;
        }
        g_date_free ( date_max );
    }

    gsb_file_set_modified ( TRUE );

    return TRUE;
}


/**
 * add lines from file
 *
 *
 *
 * */
gboolean bet_data_future_set_lines_from_file ( FuturData *scheduled )
{
    gchar *key;

    key = bet_data_get_key ( scheduled -> account_number, scheduled -> number );

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
 * \param FuturData
 * \param date the current date, we want the next one after that one
 *
 * \return a newly allocated date, the next date or NULL if over the limit
 * */
GDate *bet_data_futur_get_next_date ( FuturData *scheduled,
                        const GDate *date,
                        const GDate *date_max )
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

    if ( scheduled -> limit_date )
    {

	    if ( g_date_compare ( return_date, scheduled -> limit_date ) > 0 )
        {
            g_date_free (return_date);
            return_date = NULL;
        }
    }
    else
    {
        if ( g_date_compare ( return_date, date_max ) > 0 )
        {
            g_date_free (return_date);
            return_date = NULL;
        }
    }

    return ( return_date );
}


/**
 *
 *
 *
 *
 * */
FuturData *bet_data_future_copy_struct ( FuturData *scheduled )
{
    FuturData *new_scheduled;

    new_scheduled = struct_initialise_bet_future ( );

    if ( !new_scheduled )
    {
        dialogue_error_memory ();
        return NULL;
    }

    new_scheduled ->  number = scheduled -> number;
    new_scheduled ->  account_number = scheduled -> account_number;

    if ( g_date_valid ( scheduled -> date ) )
        new_scheduled -> date = gsb_date_copy ( scheduled -> date );
    else
        new_scheduled -> date = NULL;

    new_scheduled -> amount = gsb_real_new ( scheduled -> amount.mantissa,
                        scheduled -> amount.exponent );
    new_scheduled -> fyear_number = scheduled -> fyear_number;
    new_scheduled -> payment_number = scheduled -> payment_number;

    new_scheduled -> party_number = scheduled -> party_number;
    new_scheduled -> is_transfert = scheduled -> is_transfert;
    new_scheduled -> account_transfert = scheduled -> account_transfert;
    new_scheduled -> category_number = scheduled -> category_number;
    new_scheduled -> sub_category_number = scheduled -> sub_category_number;
    new_scheduled -> budgetary_number = scheduled -> budgetary_number;
    new_scheduled -> sub_budgetary_number = scheduled -> sub_budgetary_number;
    new_scheduled -> notes = g_strdup ( scheduled -> notes );

    new_scheduled -> frequency = scheduled -> frequency;
    new_scheduled -> user_interval = scheduled -> user_interval;
    new_scheduled -> user_entry = scheduled -> user_entry;

    if ( scheduled -> limit_date && g_date_valid ( scheduled -> limit_date ) )
        new_scheduled -> limit_date = gsb_date_copy ( scheduled -> limit_date );
    else
        new_scheduled -> limit_date = NULL;

    new_scheduled -> mother_row = scheduled -> mother_row;

    return new_scheduled;
}
/**
 * supprime l'occurence sélectionnée.
 *
 * \param maj TRUE force la mise à jour
 *
 * */
gboolean bet_data_future_remove_line ( gint account_number, gint number, gboolean maj )
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init ( &iter, bet_future_list );
    while (g_hash_table_iter_next ( &iter, &key, &value ) )
    {
        FuturData *scheduled = ( FuturData *) value;

        if ( number != scheduled -> number )
            continue;

        if ( account_number != scheduled -> account_number )
        {
            if ( scheduled -> is_transfert == 0
             || ( scheduled -> is_transfert && account_number != scheduled -> account_transfert ) )
                continue;
        }

        g_hash_table_iter_remove ( &iter );

        break;
    }

    gsb_file_set_modified ( TRUE );

    if ( maj )
    {
        gsb_data_account_set_bet_maj ( account_number, BET_MAJ_ESTIMATE );
        bet_data_update_bet_module ( account_number, GSB_ESTIMATE_PAGE );
    }

    return FALSE;
}


/**
 * supprime touts les occurences de la ligne sélectionnée
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
        FuturData *scheduled = ( FuturData *) value;

        if ( account_number != scheduled -> account_number )
        {
            if ( scheduled -> is_transfert == 0
             || ( scheduled -> is_transfert && account_number != scheduled -> account_transfert ) )
                continue;
        }

        if ( number == scheduled -> number )
            g_hash_table_iter_remove ( &iter );
        else if ( number == scheduled -> mother_row )
            g_hash_table_iter_remove ( &iter );
        else if ( mother_row > 0 && mother_row == scheduled -> number )
            g_hash_table_iter_remove ( &iter );
        else if ( mother_row > 0 && mother_row == scheduled -> mother_row )
            g_hash_table_iter_remove ( &iter );
    }

    gsb_file_set_modified ( TRUE );

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
    GDate *date_tmp;

    date_min = gsb_data_account_get_bet_start_date ( account_number );

    date_max = gsb_date_copy ( date_min );
    date_tmp = date_max;

    if ( g_date_get_day ( date_min ) == 1 )
    {
        g_date_add_months (date_max, gsb_data_account_get_bet_months ( account_number ) - 1 );
        date_max = gsb_date_get_last_day_of_month ( date_tmp );
        g_date_free ( date_tmp );
    }
    else
    {
        g_date_add_months (date_max, gsb_data_account_get_bet_months ( account_number ) );
        g_date_subtract_days ( date_max, 1 );
    }

    g_date_free ( date_min );

    return date_max;
}


/**
 * modify futures data lines
 *
 *
 *
 * */
gboolean bet_data_future_modify_lines ( FuturData *scheduled )
{
    gchar *key;

    key = bet_data_get_key ( scheduled -> account_number, scheduled -> number );

    g_hash_table_replace ( bet_future_list, key, scheduled );

    gsb_file_set_modified ( TRUE );

    return TRUE;
}


/**
 *
 *
 *
 *
 * */
FuturData *bet_data_future_get_struct ( gint account_number, gint number )
{
    gchar *key;
    FuturData *scheduled;

    key = bet_data_get_key ( account_number, number );

    if ( ( scheduled = g_hash_table_lookup ( bet_future_list, key ) ) )
        return scheduled;
    else
        return NULL;
}


/**
 *
 *
 *
 *
 * */
TransfertData *struct_initialise_bet_transfert ( void )
{
    TransfertData *transfert;

    transfert =  g_malloc0 ( sizeof ( TransfertData ) );

    transfert->date = NULL;
    transfert->date_bascule = NULL;

    return transfert;
}


/**
 *
 *
 *
 *
 * */
void struct_free_bet_transfert ( TransfertData *transfert )
{
    if ( transfert->date )
        g_date_free ( transfert->date );
    if ( transfert->date_bascule )
        g_date_free ( transfert->date_bascule );

    g_free ( transfert );
}


/**
 *
 *
 *
 *
 * */
GHashTable *bet_data_transfert_get_list ( void )
{
    return bet_transfert_list;
}


/**
 * add line in the bet_transfer_list
 *
 *
 *
 * */
gboolean bet_data_transfert_add_line ( TransfertData *transfert )
{
    gchar *key;

    transfert_number ++;

    key = bet_data_get_key ( transfert -> account_number, transfert_number );

    transfert -> number = transfert_number;
    g_hash_table_insert ( bet_transfert_list, key, transfert );

    gsb_file_set_modified ( TRUE );

    return TRUE;
}


/**
 *
 *
 *
 *
 * */
gboolean bet_data_transfert_remove_line ( gint account_number, gint number )
{
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init ( &iter, bet_transfert_list );
    while (g_hash_table_iter_next ( &iter, &key, &value ) )
    {
        TransfertData *transfert = ( TransfertData *) value;

        if ( account_number != transfert -> account_number
         ||
         number != transfert -> number)
            continue;

        g_hash_table_iter_remove ( &iter );

        break;
    }

    gsb_file_set_modified ( TRUE );

    gsb_data_account_set_bet_maj ( account_number, BET_MAJ_ESTIMATE );
    bet_data_update_bet_module ( account_number, GSB_ESTIMATE_PAGE );

    return FALSE;
}


/**
 *
 *
 *
 *
 * */
gboolean bet_data_transfert_set_line_from_file ( TransfertData *transfert )
{
    gchar *key;

    key = bet_data_get_key ( transfert -> account_number, transfert -> number );

    if ( transfert -> number >  transfert_number )
        transfert_number = transfert -> number;

    g_hash_table_insert ( bet_transfert_list, key, transfert );

    return TRUE;
}


/**
 * modify transfert line
 *
 *
 *
 * */
gboolean bet_data_transfert_modify_line ( TransfertData *transfert )
{
    gchar *key;

    key = bet_data_get_key ( transfert -> account_number, transfert -> number );

    g_hash_table_replace ( bet_transfert_list, key, transfert );

    gsb_file_set_modified ( TRUE );

    return TRUE;
}


/**
 * crée la ou les transactions de remise à zéro des comptes cartes concernés
 *
 * \param struct transfert
 *
 * \return
 * */
static void bet_data_transfert_create_reset_credit_card ( TransfertData *transfert )
{
    gint transaction_number;
    GDate *date;
    gsb_real amount;

    date = gsb_date_copy ( transfert->date_bascule );
    /* on enlève 1 jour pour la date de l'opération de remise à 0 du compte */
    g_date_subtract_days ( date, 1 );

    /* replace_account is an account */
    if ( transfert -> type == 0 )
    {
        amount = gsb_data_account_get_balance_at_date ( transfert->replace_account, date );
        transaction_number = gsb_data_transaction_new_transaction ( transfert->replace_account );
        gsb_data_transaction_set_date ( transaction_number, date );
        gsb_data_transaction_set_amount ( transaction_number, gsb_real_opposite ( amount ) );

        /* set the currency */
        gsb_data_transaction_set_currency_number ( transaction_number,
                        gsb_data_account_get_currency ( transfert->replace_account ) );

        /* set the payement mode */
        if ( amount.mantissa < 0 )
            gsb_data_transaction_set_method_of_payment_number ( transaction_number,
                        gsb_data_account_get_default_debit ( transfert->replace_account ) );
        else
            gsb_data_transaction_set_method_of_payment_number ( transaction_number,
                        gsb_data_account_get_default_credit ( transfert->replace_account ) );

        /* set the payee */
        gsb_data_transaction_set_party_number ( transaction_number, transfert->main_payee_number );

        /* set the category sub_category */
        if ( transfert->card_category_number )
        {
            gsb_data_transaction_set_category_number ( transaction_number, transfert->card_category_number );
            if ( transfert->card_sub_category_number )
                gsb_data_transaction_set_sub_category_number ( transaction_number,
                        transfert->card_sub_category_number );
        }

        /* set the IB sub_IB */
        if ( transfert->card_budgetary_number )
        {
            gsb_data_transaction_set_budgetary_number ( transaction_number, transfert->card_budgetary_number );
            if ( transfert->card_sub_category_number )
                gsb_data_transaction_set_sub_budgetary_number ( transaction_number,
                        transfert->card_sub_budgetary_number );
        }

        /* append the transaction in list */
        gsb_transactions_list_append_new_transaction ( transaction_number, TRUE );
    }
    else
    {
        GPtrArray *balances;
        gchar **tab;
        gint i;

        tab = g_strsplit ( gsb_data_partial_balance_get_liste_cptes ( transfert->replace_account ), ";", 0 );

        /* on calcule la balance de tous les comptes du pseudo compte */
        balances = gsb_data_partial_balance_calculate_balances_at_date ( transfert->replace_account, date );

        for ( i = 0; tab[i]; i++ )
        {
            gint account_number;
            gsb_real *balance;

            account_number = utils_str_atoi ( tab[i] );
            balance = (gsb_real *) g_ptr_array_index ( balances, i );
            amount.mantissa = balance->mantissa;
            amount.exponent = balance->exponent;

            transaction_number = gsb_data_transaction_new_transaction ( account_number );
            gsb_data_transaction_set_date ( transaction_number, date );
            gsb_data_transaction_set_amount ( transaction_number, gsb_real_opposite ( amount ) );

            /* set the currency */
            gsb_data_transaction_set_currency_number ( transaction_number,
                        gsb_data_account_get_currency ( account_number ) );

            /* set the payement mode */
            if ( amount.mantissa < 0 )
                gsb_data_transaction_set_method_of_payment_number ( transaction_number,
                        gsb_data_account_get_default_debit ( account_number ) );
            else
                gsb_data_transaction_set_method_of_payment_number ( transaction_number,
                        gsb_data_account_get_default_credit ( account_number ) );

            /* set the payee */
            gsb_data_transaction_set_party_number ( transaction_number, transfert->main_payee_number );

            /* set the category sub_category */
            if ( transfert->card_category_number )
            {
                gsb_data_transaction_set_category_number ( transaction_number, transfert->card_category_number );
                if ( transfert->card_sub_category_number )
                    gsb_data_transaction_set_sub_category_number ( transaction_number,
                        transfert->card_sub_category_number );
            }

            /* set the IB sub_IB */
            if ( transfert->card_budgetary_number )
            {
                gsb_data_transaction_set_budgetary_number ( transaction_number, transfert->card_budgetary_number );
                if ( transfert->card_sub_category_number )
                    gsb_data_transaction_set_sub_budgetary_number ( transaction_number,
                        transfert->card_sub_budgetary_number );
            }

            /* append the transaction in list */
            gsb_transactions_list_append_new_transaction ( transaction_number, TRUE );
        }
        g_strfreev ( tab );
        g_ptr_array_free ( balances, TRUE );
    }
    g_date_free ( date );
}


/**
 * update la date de la ligne transfert lorsque la date de bascule est atteinte
 *
 * \param struct transfert contenant les données
 *
 * \return
 * */
void bet_data_transfert_update_date_if_necessary ( TransfertData *transfert )
{
    GDate *date_jour;
    GDate *tmp_date;

    if ( transfert->date_bascule == NULL )
        return;

    date_jour = gdate_today ( );

    if ( g_date_compare ( date_jour, transfert -> date_bascule ) >= 0 )
    {
        gchar *msg;
        const gchar *tmp_str;

        if ( transfert -> type == 0 )
            tmp_str = gsb_data_account_get_name ( transfert->replace_account );
        else
            tmp_str = gsb_data_partial_balance_get_name ( transfert->replace_account );

        msg = g_strdup_printf (
                        _("Warning: the start date of the period of deferred debit card account (%s) "
                          "is reached or exceeded. All operations of this card account must be entered "
                          "or they will not be taken into account in the period.\n\n"
                          "If you have entered all transactions, confirm with \"Yes\".\n"
                          "If you have yet to enter into transactions on behalf deferred debit card finish "
                          "with \"No\" and update the account before you start."),
                        tmp_str );
        if (!question_yes_no ( msg, _("Confirmation of the change of period"), GTK_RESPONSE_CANCEL ))
        {
            g_free ( msg );
            return;
        }
            g_free ( msg );

        /* on crée la transaction dans le compte principal */
        if ( transfert->direct_debit )
            bet_array_create_transaction_from_transfert (transfert);

        /* on remet à zéro les comptes cartes */
        bet_data_transfert_create_reset_credit_card ( transfert );

        /* on incrémente la date de prélèvement */
        tmp_date = gsb_date_copy ( transfert->date );

        g_date_free ( transfert->date );
        g_date_add_months ( tmp_date, 1 );

        if ( transfert->main_last_banking_date )
        {
            transfert->date = gsb_date_get_last_banking_day_of_month ( tmp_date );
            g_date_free ( tmp_date );
        }
        else
        {
            transfert->date = tmp_date;
        }

        /* on incrémente la date de bascule */
        tmp_date = gsb_date_copy ( transfert->date_bascule );

        g_date_free ( transfert->date_bascule );
        g_date_add_months ( tmp_date, 1 );
        transfert->date_bascule = tmp_date;

        gsb_data_account_set_bet_maj ( transfert->account_number, BET_MAJ_ESTIMATE );
        gsb_file_set_modified ( TRUE );
    }

    g_date_free ( date_jour );
}

/**
 * supprime toutes les données du module pour le compte passé en paramètre.
 *
 *
 *
 * */
gboolean bet_data_remove_all_bet_data ( gint account_number )
{
    GHashTable *tmp_list;
    GHashTableIter iter;
    gpointer key, value;

    tmp_list = bet_hist_div_list;

    g_hash_table_iter_init ( &iter, tmp_list );
    while ( g_hash_table_iter_next ( &iter, &key, &value ) )
    {
        HistDiv *shd = ( HistDiv *) value;

        if ( account_number != shd -> account_nb )
            continue;

        if ( g_hash_table_size ( shd -> sub_div_list ) == 0 )
        {
            g_hash_table_iter_remove ( &iter );
            g_hash_table_iter_init ( &iter, tmp_list );
        }
        else
        {
            GHashTableIter new_iter;

            g_hash_table_iter_init ( &new_iter, shd -> sub_div_list );
            while ( g_hash_table_iter_next ( &new_iter, &key, &value ) )
            {
                g_hash_table_iter_remove ( &new_iter );
                g_hash_table_iter_init ( &new_iter, shd -> sub_div_list );

                if ( g_hash_table_size ( shd -> sub_div_list ) == 0 )
                    break;
            }
        }

        if ( g_hash_table_size ( tmp_list ) == 0 )
            break;
    }

    tmp_list = bet_future_list;

    g_hash_table_iter_init ( &iter, tmp_list );
    while ( g_hash_table_iter_next ( &iter, &key, &value ) )
    {
        FuturData *scheduled = ( FuturData *) value;

        if ( account_number != scheduled -> account_number )
            continue;

        bet_data_future_remove_lines ( account_number, scheduled -> number,
                        scheduled -> mother_row );
        g_hash_table_iter_init ( &iter, tmp_list );

        if ( g_hash_table_size ( tmp_list ) == 0 )
            break;
    }

     tmp_list = bet_transfert_list;

    g_hash_table_iter_init ( &iter, tmp_list );
    while ( g_hash_table_iter_next ( &iter, &key, &value ) )
    {
        TransfertData *transfert = ( TransfertData *) value;

        if ( account_number != transfert -> account_number )
            continue;

        g_hash_table_iter_remove ( &iter );
        g_hash_table_iter_init ( &iter, tmp_list );

        if ( g_hash_table_size ( tmp_list ) == 0 )
            return TRUE;
    }

    return TRUE;
}


/**
 * retourne la clef de recherche de la division passée en paramètre.
 *
 *
 *
 * */
gchar *bet_data_get_key ( gint account_number, gint div_number )
{
    gchar *key;
    gchar *div_number_str, *account_number_str; /* only to avoid memory leaks */

    div_number_str = utils_str_itoa ( div_number );

    if ( account_number == 0 )
        key = g_strconcat ("0:", div_number_str, NULL );
    else
    {
        account_number_str = utils_str_itoa ( account_number );
        key = g_strconcat ( account_number_str, ":", div_number_str, NULL );

        g_free ( account_number_str );
    }

    g_free ( div_number_str );

    return key;
}


/**
 *
 *
 *
 *
 * */
gchar *bet_data_get_str_amount_in_account_currency ( gsb_real amount,
                        gint account_number,
                        gint line_number,
                        gint origin )
{
    gchar *str_amount = NULL;
    gint account_currency;
    gint floating_point;
    gsb_real new_amount;

    account_currency = gsb_data_account_get_currency ( account_number );
    floating_point = gsb_data_account_get_currency_floating_point ( account_number );

    switch ( origin )
    {
        case SPP_ORIGIN_TRANSACTION :
            new_amount = gsb_data_transaction_get_adjusted_amount_for_currency ( line_number,
                                    account_currency,
                                    floating_point );
        break;
        case SPP_ORIGIN_SCHEDULED :
            new_amount = gsb_data_scheduled_get_adjusted_amount_for_currency ( line_number,
                                    account_currency,
                                    floating_point );

        break;
        case SPP_ORIGIN_ACCOUNT :
            if ( account_currency == line_number || amount.mantissa == 0 )
            {
                new_amount.mantissa = amount.mantissa;
                new_amount.exponent = amount.exponent;
            }
            else
                new_amount = gsb_transfert_get_str_amount ( amount,
                                    account_currency,
                                    line_number,
                                    floating_point );

        break;
    }

    str_amount = utils_real_get_string ( new_amount );

    return str_amount;
}


/**
 * Création effective de la transaction dans le compte qui comporte le suivi de la carte
 * à débit différé
 *
 * \param structure transfert
 *
 * \return
 * */
void bet_data_transfert_create_new_transaction ( TransfertData *transfert )
{
    gint transaction_number;
    GDate *date;
    gsb_real amount;

    date = gsb_date_copy ( transfert->date_bascule );
    /* on enlève 1 jour pour la date de l'opération de remise à 0 du compte */
    g_date_subtract_days ( date, 1 );

    transaction_number = gsb_data_transaction_new_transaction ( transfert->account_number );

    /* set the date */
    gsb_data_transaction_set_date ( transaction_number, transfert->date );

    /* set the amount soit celui du compte carte soit celui du solde partiel */
    if ( transfert -> type == 0 )
    {
        amount = gsb_data_account_get_balance_at_date ( transfert->replace_account, date );
    }
    else
    {
        amount = gsb_data_partial_balance_get_balance_at_date ( transfert -> replace_account, date );
    }
    gsb_data_transaction_set_amount ( transaction_number, amount );

    /* set the currency */
    gsb_data_transaction_set_currency_number ( transaction_number,
                        gsb_data_account_get_currency ( transfert->account_number ) );

    /* set the payement mode */
    gsb_data_transaction_set_method_of_payment_number ( transaction_number, transfert->main_payment_number );

    /* set the payee */
    gsb_data_transaction_set_party_number ( transaction_number, transfert->main_payee_number );

    /* set the category sub_category */
    if ( transfert->main_category_number )
    {
        gsb_data_transaction_set_category_number ( transaction_number, transfert->main_category_number );
        if ( transfert->main_sub_category_number )
            gsb_data_transaction_set_sub_category_number ( transaction_number, transfert->main_sub_category_number );
    }

    /* set the IB sub_IB */
    if ( transfert->main_budgetary_number )
    {
        gsb_data_transaction_set_budgetary_number ( transaction_number, transfert->main_budgetary_number );
        if ( transfert->main_sub_budgetary_number )
            gsb_data_transaction_set_sub_budgetary_number ( transaction_number, transfert->main_sub_budgetary_number );
    }

    /* append the transaction in list */
    gsb_transactions_list_append_new_transaction ( transaction_number, TRUE );

    g_date_free ( date );
}


/**
 * initialise la structure
 *
 * \param
 *
 * \return
 * */
TransactionCurrentFyear *struct_initialise_transaction_current_fyear ( void )
{
    TransactionCurrentFyear *self;

    self = g_malloc0 ( sizeof ( TransactionCurrentFyear ) );

    self -> date = NULL;
    self -> amount = null_real;

    return self;
}


/**
 *
 *
 * \param
 *
 * \return
 * */
void struct_free_bet_transaction_current_fyear ( TransactionCurrentFyear *self )
{
    if ( self->date )
        g_date_free ( self->date );

    g_free ( self );
}


/**
 * Sets to 0 all the amounts selected for historical data (ou "archival")
 *
 * \param account_number
 *
 * \return
 * */
void bet_data_hist_reset_all_amounts ( gint account_number )
{
    GHashTableIter iter;
    gpointer key, value;

    if ( g_hash_table_size ( bet_hist_div_list ) == 0 )
        return;

    g_hash_table_iter_init ( &iter, bet_hist_div_list );
    while ( g_hash_table_iter_next ( &iter, &key, &value ) )
    {
        HistDiv *shd = ( HistDiv* ) value;

        if ( g_hash_table_size ( shd -> sub_div_list ) == 0 )
        {
            if ( shd -> div_edited )
                bet_data_set_div_amount ( account_number, shd -> div_number, 0, null_real );
        }
        else
        {
            GHashTableIter new_iter;

            g_hash_table_iter_init ( &new_iter, shd -> sub_div_list );
            while ( g_hash_table_iter_next ( &new_iter, &key, &value ) )
            {
                HistDiv *sub_shd = ( HistDiv* ) value;

                if ( sub_shd->div_edited )
                    bet_data_set_div_amount ( account_number, shd->div_number, sub_shd->div_number, null_real );
            }
        }
    }
}


/**
 *
 *
 * \param
 *
 * \return
 * */

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
