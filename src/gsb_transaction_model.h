#ifndef _GSB_TRANSACTION_MODEL_H
#define _GSB_TRANSACTION_MODEL_H (1)


/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
void gsb_transaction_model_convert_sorted_iter_to_model_iter ( GtkTreeIter *tree_iter,
							       GtkTreeIter *sorted_iter );
gint gsb_transaction_model_get_archive_store_from_sorted_path ( GtkTreePath *path_sorted );
GtkTreeIter *gsb_transaction_model_get_iter_from_transaction ( gint transaction_number,
							       gint line_in_transaction );
GtkTreePath *gsb_transaction_model_get_model_path_from_sorted_path ( GtkTreePath *path_sorted );
gint gsb_transaction_model_get_transaction_from_sorted_path ( GtkTreePath *path_sorted );
GtkTreePath *gsb_transactions_list_get_sorted_path ( gint transaction_number,
						     gint line_in_transaction );
/* END_DECLARATION */
#endif
