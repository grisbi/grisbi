#ifndef _BALANCE_ESTIMATE_DATA_H
#define _BALANCE_ESTIMATE_DATA_H

#include <gtk/gtk.h>

/* START_INCLUDE_H */
#include "gsb_real.h"
/* END_INCLUDE_H */

/* nombre de colonnes du tableau des prévisions */
#define BET_ARRAY_COLUMNS 5

typedef struct _bet_range           SBR;
typedef struct _historical          SH;
typedef struct _hist_div            struct_hist_div;
typedef struct _future_data         struct_futur_data;
typedef struct _transfert_data      struct_transfert_data;

struct _bet_range
{
    gboolean first_pass;
    GDate *min_date;
    GDate *max_date;
    gsb_real current_fyear;
    gsb_real current_balance;
};

/* utilisée pour gérer la liste des données historiques dans la vue historique */
struct _historical
{
    gint div;
	gint account_nb;
    SBR	*sbr;
	GHashTable *list_sub_div;
};

/* utilisée pour gérer la liste des données historiques dans le tableau des prévisions */
struct _hist_div
{
    gint account_nb;
    gint div_number;
    gint origin;
    gboolean div_edited;
    GHashTable *sub_div_list;
    gsb_real amount;
};


struct _future_data
{
    gint number;
    gint account_number;
    GDate *date;
    gsb_real amount;
    guint fyear_number;
    gint payment_number;
    gint party_number;
    gint category_number;
    gint sub_category_number;
    gint budgetary_number;
    gint sub_budgetary_number;
    gchar *notes;

    gboolean is_transfert;
    gint account_transfert;

    gint frequency;                     /*  0=once, 1=week, 2=month, 5=year, 6=perso */
    gint user_interval;					/*  0=days, 1= week 2=month, 3=years */
    gint user_entry;
    GDate *limit_date;
    gint mother_row;                    /* if frequency > 0 */
};


struct _transfert_data
{
    gint number;
    gint account_number;
    gint type;
    gint replace_account;       /* Account number or partial balance concerned */
    gint replace_transaction;
    gint auto_inc_month;
    GDate *date;
    gint category_number;
    gint sub_category_number;
    gint budgetary_number;
    gint sub_budgetary_number;
};


/* START_DECLARATION */
GDate *bet_data_array_get_date_max ( gint account_number );
gboolean bet_data_future_add_lines ( struct_futur_data *scheduled );
GHashTable *bet_data_future_get_list ( void );
struct_futur_data *bet_data_future_get_struct ( gint account_number, gint number );
gboolean bet_data_future_modify_lines ( struct_futur_data *scheduled );
gboolean bet_data_future_remove_line ( gint account_number, gint number, gboolean maj );
gboolean bet_data_future_remove_lines ( gint account_number,
                        gint number,
                        gint mother_row );
gboolean bet_data_future_set_lines_from_file ( struct_futur_data *scheduled );
gboolean bet_data_get_div_edited ( gint account_number, gint div_number, gint sub_div_nb );
gchar *bet_data_get_div_name ( gint div_num,
                        gint sub_div,
                        const gchar *return_value_error );
gint bet_data_get_div_number ( gint transaction_number, gboolean is_transaction );
gint bet_data_get_div_type ( gint div_number );
gint bet_data_get_selected_currency ( void );
gchar *bet_data_get_str_amount_in_account_currency ( gsb_real amount,
                        gint account_number,
                        gint line_number,
                        gint origin );
GPtrArray *bet_data_get_strings_to_save ( void );
gint bet_data_get_sub_div_nb ( gint transaction_number, gboolean is_transaction );
gboolean bet_data_hist_add_div ( gint account_number,
                        gint div_number,
                        gint sub_div_nb );
gsb_real bet_data_hist_get_div_amount ( gint account_nb, gint div_number, gint sub_div_nb );
gboolean bet_data_init_variables ( void );
void bet_data_insert_div_hist ( struct_hist_div *shd, struct_hist_div *sub_shd );
gboolean bet_data_populate_div ( gint transaction_number,
                        gboolean is_transaction,
                        GHashTable  *list_div,
                        gint type_de_transaction );
gboolean bet_data_remove_all_bet_data ( gint account_number );
gboolean bet_data_remove_div_hist ( gint account_number, gint div_number, gint sub_div_nb );
gboolean bet_data_search_div_hist ( gint account_number, gint div_number, gint sub_div_nb );
void bet_data_select_bet_pages ( gint account_number );
gboolean bet_data_set_div_amount ( gint account_nb,
                        gint div_number,
                        gint sub_div_nb,
                        gsb_real amount );
gboolean bet_data_set_div_edited ( gint account_nb,
                        gint div_number,
                        gint sub_div_nb,
                        gboolean edited );
gboolean bet_data_set_div_ptr ( gint type_div );
void bet_data_set_maj ( gint account_number, gint type_maj );
void bet_data_synchronise_hist_div_list ( GHashTable  *list_div );
gboolean bet_data_transfert_add_line ( struct_transfert_data *transfert );
GHashTable *bet_data_transfert_get_list ( void );
gboolean bet_data_transfert_modify_line ( struct_transfert_data *transfert );
gboolean bet_data_transfert_remove_line ( gint account_number, gint number );
gboolean bet_data_transfert_set_line_from_file ( struct_transfert_data *transfert );
void bet_data_transfert_update_date_if_necessary ( struct_transfert_data *transfert );
void bet_data_update_bet_module ( gint account_number, guint page );
void struct_free_bet_historical ( SH *sh );
struct_futur_data *struct_initialise_bet_future ( void );
SBR *struct_initialise_bet_range ( void );
struct_transfert_data *struct_initialise_bet_transfert ( void );
struct_hist_div *struct_initialise_hist_div ( void );
/* END_DECLARATION */


#endif
