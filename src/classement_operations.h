#ifndef _CLASSEMENT_OPERATIONS_H
#define _CLASSEMENT_OPERATIONS_H (1)
/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
gint classement_liste_par_no_ope_ventil ( GtkWidget *liste,
					  GtkCListRow *ligne_1,
					  GtkCListRow *ligne_2 );
gint gsb_strcasecmp ( gchar *string_1,
		      gchar *string_2 );
gint gsb_transactions_list_sort_column_0 ( GtkTreeModel *model,
					   GtkTreeIter *iter_1,
					   GtkTreeIter *iter_2,
					   gint *no_account );
gint gsb_transactions_list_sort_column_1 ( GtkTreeModel *model,
					   GtkTreeIter *iter_1,
					   GtkTreeIter *iter_2,
					   gint *no_account );
gint gsb_transactions_list_sort_column_2 ( GtkTreeModel *model,
					   GtkTreeIter *iter_1,
					   GtkTreeIter *iter_2,
					   gint *no_account );
gint gsb_transactions_list_sort_column_3 ( GtkTreeModel *model,
					   GtkTreeIter *iter_1,
					   GtkTreeIter *iter_2,
					   gint *no_account );
gint gsb_transactions_list_sort_column_4 ( GtkTreeModel *model,
					   GtkTreeIter *iter_1,
					   GtkTreeIter *iter_2,
					   gint *no_account );
gint gsb_transactions_list_sort_column_5 ( GtkTreeModel *model,
					   GtkTreeIter *iter_1,
					   GtkTreeIter *iter_2,
					   gint *no_account );
gint gsb_transactions_list_sort_column_6 ( GtkTreeModel *model,
					   GtkTreeIter *iter_1,
					   GtkTreeIter *iter_2,
					   gint *no_account );
/* END_DECLARATION */
#endif
