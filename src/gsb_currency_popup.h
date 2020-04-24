#ifndef _GSB_CURRENCY_POPUP_H
#define _GSB_CURRENCY_POPUP_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
//~ #include "gsb_real.h"
/* END_INCLUDE_H */

/** This structure holds information needed for exchange rates
  cache. */
/* START_DECLARATION */
void			gsb_popup_list_fill_model							(GtkTreeView *tree_view,
																	 gboolean include_obsolete);
gboolean 		gsb_popup_list_select_default 						(GtkTreeModel *tree_model,
																	 GtkTreePath *path,
																	 GtkTreeIter *iter,
																	 GtkTreeView *tree_view);
gboolean		gsb_popup_list_selected_currency_new				(GtkTreeSelection *selection);
void			gsb_popup_list_selection_changed					(GtkTreeSelection *selection,
									   								 GtkWidget *w_currency_details);
GtkTreeModel *	gsb_popup_list_store_new							(void);
GtkWidget *		gsb_popup_list_iso_4217_new							(GtkWidget *parent,
																	 GCallback select_callback);
/* END_DECLARATION */
#endif
