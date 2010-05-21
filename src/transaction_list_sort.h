#ifndef _TRANSACTION_LIST_SORT_H
#define _TRANSACTION_LIST_SORT_H (1)


/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
void transaction_list_sort (void);
void transaction_list_sort_get_column ( gint *sort_col,
                        GtkSortType *sort_order );
gboolean transaction_list_sort_get_reconcile_sort ( void );
void transaction_list_sort_set_column ( gint new_sort_col,
                        GtkSortType new_sort_order );
void transaction_list_sort_set_reconcile_sort ( gboolean use_reconcile_sort );
/* END_DECLARATION */
#endif
