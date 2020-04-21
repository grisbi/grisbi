#ifndef _GSB_CURRENCY_CONFIG_H
#define _GSB_CURRENCY_CONFIG_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
#include "gsb_real.h"
/* END_INCLUDE_H */

/** This structure holds information needed for exchange rates
  cache. */
struct CachedExchangeRate {
    gint currency1_number;	/** First currency */
    gint currency2_number;	/** Second currency */
    GsbReal rate;			/** Exchange rate between currency1 and currency 2 */
    GsbReal fees;			/** Fees associated with exchange rate */
};


/* START_DECLARATION */
gboolean 	gsb_currency_config_add_currency 						(GtkWidget *button,
																	 GtkWidget *tree_view);
gboolean 	gsb_currency_config_add_currency_set_combobox 			(GtkWidget *button,
																	 GtkWidget *combobox);
GtkWidget *	gsb_currency_config_create_box_popup 					(GCallback select_callback);
GtkWidget * gsb_currency_config_new_combobox						(gint *value,
																	 GCallback hook);
gboolean 	gsb_currency_config_select_default 						(GtkTreeModel *tree_model,
																	 GtkTreePath *path,
																	 GtkTreeIter *iter,
																	 GtkTreeView *tree_view);
/* END_DECLARATION */
#endif
