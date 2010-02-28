#ifndef _BALANCE_ESTIMATE_DATA_H
#define _BALANCE_ESTIMATE_DATA_H

#include <gtk/gtk.h>

/* START_INCLUDE_H */
#include "./gsb_real.h"
/* END_INCLUDE_H */


typedef struct _bet_range           SBR;
typedef struct _historical          SH;
typedef struct _hist_div            struct_hist_div;


struct _bet_range
{
    gboolean first_pass;
    GDate *min_date;
    GDate *max_date;
    gsb_real min_balance;
    gsb_real max_balance;
    gsb_real current_balance;
};

struct _historical
{
    gint div;
	gint account_nb;
    SBR	*sbr;
	GHashTable *list_sub_div;
};


struct _hist_div
{
    gint account_nb;
    gint div_number;
    gboolean div_edited;
    GHashTable *sub_div_list;
    gsb_real amount;
};


/* START_DECLARATION */
gboolean bet_data_add_div_hist ( gint account_nb,
                        gint div_number,
                        gint sub_div_nb );
gsb_real bet_data_get_div_amount ( gint account_nb, gint div_number, gint sub_div_nb );
gint bet_data_get_div_children ( gint account_nb, gint div_number );
gboolean bet_data_get_div_edited ( gint account_nb, gint div_number, gint sub_div_nb );
gchar *bet_data_get_div_name (gint div_num,
                        gint sub_div,
                        const gchar *return_value_error );
gint bet_data_get_div_number ( gint transaction_number, gboolean is_transaction );
gboolean bet_data_get_maj ( void );
gint bet_data_get_selected_currency ( void );
GPtrArray *bet_data_get_strings_to_save ( void );
gint bet_data_get_sub_div_nb ( gint transaction_number, gboolean is_transaction );
gboolean bet_data_init_variables ( void );
void bet_data_insert_div_hist ( struct_hist_div *shd, struct_hist_div *sub_shd );
gboolean bet_data_populate_div ( gint transaction_number,
                        gboolean is_transaction,
                        GHashTable  *list_div );
gboolean bet_data_remove_div_hist ( gint account_nb, gint div_number, gint sub_div_nb );
gboolean bet_data_search_div_hist ( gint account_nb, gint div_number, gint sub_div_nb );
gboolean bet_data_set_div_amount ( gint account_nb,
                        gint div_number,
                        gint sub_div_nb,
                        gsb_real amount );
gboolean bet_data_set_div_edited ( gint account_nb,
                        gint div_number,
                        gint sub_div_nb,
                        gboolean edited );
gboolean bet_data_set_div_ptr ( gint type_div );
void bet_data_set_maj ( gboolean maj );
void bet_data_synchronise_hist_div_list ( GHashTable  *list_div );
SBR *initialise_struct_bet_range ( void );
SH *initialise_struct_historical ( void );
struct_hist_div *initialise_struct_hist_div ( void );
void free_struct_bet_range ( SBR *sbr );
void free_struct_historical ( SH *sh );

/* END_DECLARATION */


#endif
