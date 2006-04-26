#ifndef _OPERATIONS_LISTE_H
#define _OPERATIONS_LISTE_H (1)


/* define the no of column in the transactions list */

#define TRANSACTION_COL_NB_CHECK 0
#define TRANSACTION_COL_NB_DATE 1
#define TRANSACTION_COL_NB_PARTY 2
#define TRANSACTION_COL_NB_PR 3
#define TRANSACTION_COL_NB_DEBIT 4
#define TRANSACTION_COL_NB_CREDIT 5
#define TRANSACTION_COL_NB_BALANCE 6

#define TRANSACTION_LIST_COL_NB 7	 /*< must be the last of the list */

/* define the columns in the store
 * as the data are filled above, the number here
 * begin at TRANSACTION_LIST_COL_NB */

#define TRANSACTION_COL_NB_BACKGROUND 7		/*< color of the background */
#define TRANSACTION_COL_NB_AMOUNT_COLOR 8	/*< color of the amount */
#define TRANSACTION_COL_NB_TRANSACTION_ADDRESS 9
#define TRANSACTION_COL_NB_SAVE_BACKGROUND 10	/*< when selection, save of the normal color of background */
#define TRANSACTION_COL_NB_FONT 11		/*< PangoFontDescription if used */
#define TRANSACTION_COL_NB_TRANSACTION_LINE 12	/*< the line in the transaction (1, 2, 3 or 4) */
#define TRANSACTION_COL_NB_VISIBLE 13		/*< whether that transaction is visible or not */

#define TRANSACTION_COL_NB_TOTAL 14


#define TRANSACTION_LIST_ROWS_NB 4

/* constantes qui dÃ©finissent le no associÃ© au paramÃštre
 * dans la liste d'opÃ© */
# define TRANSACTION_LIST_DATE 1
# define TRANSACTION_LIST_VALUE_DATE 2
# define TRANSACTION_LIST_PARTY 3
# define TRANSACTION_LIST_BUDGET 4
# define TRANSACTION_LIST_CREDIT 5
# define TRANSACTION_LIST_DEBIT 6
# define TRANSACTION_LIST_BALANCE 7
# define TRANSACTION_LIST_AMOUNT 8
# define TRANSACTION_LIST_TYPE 9
# define TRANSACTION_LIST_RECONCILE_NB 10
# define TRANSACTION_LIST_EXERCICE 11
# define TRANSACTION_LIST_CATEGORY 12
# define TRANSACTION_LIST_MARK 13
# define TRANSACTION_LIST_VOUCHER 14 
# define TRANSACTION_LIST_NOTES 15
# define TRANSACTION_LIST_BANK 16
# define TRANSACTION_LIST_NO 17
# define TRANSACTION_LIST_CHQ 18

/* Constantes pour la fonction gtk_tree_view_column_set_alignment() (GTK2) */
#define COLUMN_LEFT 0.0
#define COLUMN_CENTER 0.5
#define COLUMN_RIGHT 1.0

/* START_INCLUDE_H */
#include "gsb_transactions_list.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean affichage_traits_liste_operation ( void );
gboolean assert_selected_transaction ();
gint cherche_ligne_operation ( gint transaction_number,
			       gint no_account );
gpointer cherche_operation_from_ligne ( gint ligne,
					gint no_account );
void clone_selected_transaction ();
GtkWidget *creation_fenetre_operations ( void );
void creation_titres_tree_view ( void );
void demande_mise_a_jour_tous_comptes ( void );
gint find_balance_col ( void );
gint find_balance_line ( void );
gint find_p_r_col ();
gint find_p_r_line ();
gboolean gsb_gui_change_cell_content ( GtkWidget * item, gint number );
GtkWidget * gsb_gui_create_cell_contents_menu ( int x, int y );
gboolean gsb_gui_update_row_foreach ( GtkTreeModel *model, GtkTreePath *path,
				      GtkTreeIter *iter, gint coords[2] );
gchar *gsb_transactions_get_category_real_name ( gint transaction_number );
gboolean gsb_transactions_list_append_transaction ( gint transaction_number,
						    GtkTreeStore *store );
gint gsb_transactions_list_append_white_line ( gint mother_transaction_number,
					       GtkTreeStore *store );
gboolean gsb_transactions_list_button_press ( GtkWidget *tree_view,
					      GdkEventButton *ev );
void gsb_transactions_list_change_expanders ( gint only_current_account );
gboolean gsb_transactions_list_change_sort_type ( GtkWidget *menu_item,
						  gint *no_column );
gboolean gsb_transactions_list_check_mark ( gpointer transaction );
gint gsb_transactions_list_clone_transaction ( gint transaction_number );
void gsb_transactions_list_convert_sorted_iter_to_tree_iter ( GtkTreeIter *tree_iter,
							      GtkTreeIter *sorted_iter );
GtkTreeStore *gsb_transactions_list_create_store ( void );
GtkWidget *gsb_transactions_list_create_tree_view ( GtkTreeModel *model );
void gsb_transactions_list_create_tree_view_columns ( void );
gboolean gsb_transactions_list_current_transaction_down ( gint no_account );
gboolean gsb_transactions_list_current_transaction_up ( gint no_account );
gboolean gsb_transactions_list_delete_transaction ( gint transaction_number );
gboolean gsb_transactions_list_delete_transaction_from_tree_view ( gpointer transaction );
gboolean gsb_transactions_list_edit_current_transaction ( void );
gboolean gsb_transactions_list_expand_row ( GtkTreeView *tree_view,
					    GtkTreeIter *iter_in_sort,
					    GtkTreePath *path,
					    gpointer null );
gboolean gsb_transactions_list_fill_row ( gint transaction_number,
					  GtkTreeIter *iter,
					  GtkTreeStore *store,
					  gint line_in_transaction );
gboolean gsb_transactions_list_fill_store ( GtkTreeStore *store );
gpointer gsb_transactions_list_find_white_breakdown ( gpointer *breakdown_mother );
GtkTreeModel *gsb_transactions_list_get_filter (void);
GtkTreeIter *gsb_transactions_list_get_iter_from_transaction ( gint transaction_number,
							       gint line_in_transaction );
GtkTreePath *gsb_transactions_list_get_list_path_from_sorted_path ( GtkTreePath *path_sorted );
GtkTreePath *gsb_transactions_list_get_path_from_transaction ( gint transaction_number );
GtkTreeModel *gsb_transactions_list_get_sortable (void);
GtkTreePath *gsb_transactions_list_get_sorted_path_from_list_path ( GtkTreePath *path,
								    gint no_account );
GtkTreeStore *gsb_transactions_list_get_store (void);
gint gsb_transactions_list_get_transaction_from_path ( GtkTreePath *path );
gint gsb_transactions_list_get_transaction_next ( gint transaction_number,
						  gint child_transaction );
GtkWidget *gsb_transactions_list_get_tree_view (void);
gchar *gsb_transactions_list_grep_cell_content ( gint transaction_number,
						 gint cell_content_number );
gboolean gsb_transactions_list_key_press ( GtkWidget *widget,
					   GdkEventKey *ev );
GtkWidget *gsb_transactions_list_make_gui_list ( void );
gboolean gsb_transactions_list_move_to_current_transaction ( gint no_account );
gboolean gsb_transactions_list_set_adjustment_value ( gint account_number );
gboolean gsb_transactions_list_set_background_color ( gint no_account );
gboolean gsb_transactions_list_set_current_transaction ( gint transaction_number );
void gsb_transactions_list_set_filter (GtkTreeModel *filter_model);
GtkTreeModel *gsb_transactions_list_set_filter_store ( GtkTreeStore *store );
void gsb_transactions_list_set_sortable (GtkTreeModel *sortable_model);
GtkTreeModel *gsb_transactions_list_set_sorting_store ( GtkTreeModel *filter_model );
void gsb_transactions_list_set_store (GtkTreeStore *store);
gboolean gsb_transactions_list_set_transactions_balances ( gint no_account );
void gsb_transactions_list_set_tree_view (GtkWidget *tree_view);
void gsb_transactions_list_set_visible_rows_number ( gint rows_number );
gboolean gsb_transactions_list_set_visibles_rows_on_account ( gint no_account );
gboolean gsb_transactions_list_set_visibles_rows_on_transaction ( gint transaction_number );
gboolean gsb_transactions_list_sort_column_changed ( GtkTreeViewColumn *tree_view_column );
void gsb_transactions_list_swap_children ( GtkTreeIter *new_mother_iter,
					   GtkTreeIter *last_mother_iter );
gboolean gsb_transactions_list_title_column_button_press ( GtkWidget *button,
							   GdkEventButton *ev,
							   gint *no_column );
void mise_a_jour_affichage_r ( gint affichage_r );
void mise_a_jour_labels_soldes ( void );
gboolean move_operation_to_account ( gint transaction_number,
				     gint target_account );
void move_selected_operation_to_account ( GtkMenuItem * menu_item );
void move_selected_operation_to_account_nb ( gint *account );
void new_transaction () ;
void p_press (void);
void popup_transaction_context_menu ( gboolean full, int x, int y );
void r_press (void);
gint recupere_hauteur_ligne_tree_view ( GtkWidget *tree_view );
void remove_transaction ();
void remplissage_liste_operations ( gint compte );
void schedule_selected_transaction ();
gint schedule_transaction ( gint transaction_number );
gsb_real solde_debut_affichage ( gint no_account,
				 gint floating_point);
void update_titres_tree_view ( void );
/* END_DECLARATION */
#endif
