#ifndef _BALANCE_ESTIMATE_HIST_H
#define _BALANCE_ESTIMATE_HIST_H

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */

enum bet_historical_data_columns {
    SPP_HISTORICAL_SELECT_COLUMN,
    SPP_HISTORICAL_DESC_COLUMN,
    SPP_HISTORICAL_CURRENT_COLUMN,  /* Accumulation of the current year */
    SPP_HISTORICAL_BALANCE_COLUMN,
    SPP_HISTORICAL_AVERAGE_COLUMN,
    SPP_HISTORICAL_AVERAGE_AMOUNT,  /* average column without currency */
    SPP_HISTORICAL_RETAINED_COLUMN,
    SPP_HISTORICAL_RETAINED_AMOUNT, /* retenaid column without currency */
    SPP_HISTORICAL_BALANCE_COLOR,
    SPP_HISTORICAL_ACCOUNT_NUMBER,
    SPP_HISTORICAL_DIV_NUMBER,
    SPP_HISTORICAL_SUB_DIV_NUMBER,
    SPP_HISTORICAL_EDITED_COLUMN,
    SPP_HISTORICAL_BACKGROUND_COLOR,
    SPP_HISTORICAL_NUM_COLUMNS
};

/* START_DECLARATION */
GtkWidget *bet_historical_create_page ( void );
gint bet_historical_get_fyear_from_combobox ( GtkWidget *combo_box );
void bet_historical_g_signal_block_tree_view ( void );
void bet_historical_g_signal_unblock_tree_view ( void );
void bet_historical_populate_data ( gint account_number );
void bet_historical_refresh_data ( GtkTreeModel *tab_model,
                        GDate *date_min,
                        GDate *date_max );
void bet_historical_set_fyear_from_combobox ( GtkWidget *combo_box, gint fyear );
void bet_historical_set_page_title ( gint account_number );
/* END_DECLARATION */

#endif /*_BALANCE_ESTIMATE_HIST_H*/

