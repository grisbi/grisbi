#ifndef _BALANCE_ESTIMATE_HIST_H
#define _BALANCE_ESTIMATE_HIST_H

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
GtkWidget *		bet_historical_create_page 							(void);
GtkTreeModel *	bet_historical_get_bet_fyear_model_filter			(void);
gboolean		bet_historical_fyear_create_combobox_store			(void);
void			bet_historical_fyear_hide_present_futures_fyears	(void);
gint 			bet_historical_get_fyear_from_combobox 				(GtkWidget *combo_box);
gchar *			bet_historical_get_hist_source_name 				(gint account_number);
GHashTable *	bet_historical_get_list_trans_current_fyear 		(void);
GDate *			bet_historical_get_start_date_current_fyear 		(void);
GtkWidget *		bet_historical_get_toolbar							(void);
void 			bet_historical_g_signal_block_tree_view 			(void);
void 			bet_historical_g_signal_unblock_tree_view 			(void);
void 			bet_historical_populate_data 						(gint account_number);
void 			bet_historical_refresh_data 						(GtkTreeModel *tab_model,
																	 GDate *date_min,
																	 GDate *date_max);
void 			bet_historical_set_fyear_from_combobox 				(GtkWidget *combo_box,
																	 gint fyear);
void 			bet_historical_set_page_title 						(gint account_number);
void 			bet_historical_update_toolbar 						(gint toolbar_style);



/* END_DECLARATION */

#endif /*_BALANCE_ESTIMATE_HIST_H*/

