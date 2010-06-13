#ifndef _GSB_RECONCILE_LIST_H
#define _GSB_RECONCILE_LIST_H (1)


/* START_INCLUDE_H */
#include "custom_list.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
gboolean gsb_reconcile_list_button_clicked ( GtkWidget *button,
					     gpointer null );
gint gsb_reconcile_list_sort_func (CustomRecord **a,
				   CustomRecord **b,
				   CustomList *custom_list);
/* END_DECLARATION */
#endif
