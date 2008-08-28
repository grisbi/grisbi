#ifndef _GSB_LIST_MODEL_H
#define _GSB_LIST_MODEL_H (1)

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean transaction_list_select ( gint transaction_number );
gboolean transaction_list_select_down ( gboolean into_children );
gint transaction_list_select_get ( void );
GtkTreePath *transaction_list_select_get_path ( gint line_in_transaction );
void transaction_list_select_unselect (void);
gboolean transaction_list_select_up ( gboolean into_children );
/* END_DECLARATION */

#endif

