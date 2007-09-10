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

#define TRANSACTION_COL_NB_BACKGROUND 7		/*< color of the background (a GdkColor) */
#define TRANSACTION_COL_NB_AMOUNT_COLOR 8	/*< color of the amount (a string like "red" or NULL)*/
#define TRANSACTION_COL_NB_TRANSACTION_ADDRESS 9
#define TRANSACTION_COL_NB_WHAT_IS_LINE 10	/*< on what the address point to ? IS_TRANSACTION, IS_ARCHIVE (see below) */
#define TRANSACTION_COL_NB_SAVE_BACKGROUND 11	/*< when selection, save of the normal color of background (a GdkColor) */
#define TRANSACTION_COL_NB_FONT 12		/*< PangoFontDescription if used */
#define TRANSACTION_COL_NB_TRANSACTION_LINE 13	/*< the line in the transaction (1, 2, 3 or 4) */
#define TRANSACTION_COL_NB_VISIBLE 14		/*< whether that transaction is visible or not */

#define TRANSACTION_COL_NB_TOTAL 15

/* definition of the number of max rows for a line, for now limit to 4 */
#define TRANSACTION_LIST_ROWS_NB 4

/* possible values to TRANSACTION_COL_NB_WHAT_IS_LINE */
#define IS_TRANSACTION 0
#define IS_ARCHIVE 1

/* the element number for each showable in the list */
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

/* used for gtk_tree_view_column_set_alignment() (GTK2) */
#define COLUMN_LEFT 0.0
#define COLUMN_CENTER 0.5
#define COLUMN_RIGHT 1.0

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean affichage_traits_liste_operation ( void );
gint cherche_ligne_operation ( gint transaction_number,
			       gint account_number );
void clone_selected_transaction ();
GtkWidget *creation_fenetre_operations ( void );
void demande_mise_a_jour_tous_comptes ( void );
gchar *gsb_transactions_get_category_real_name ( gint transaction_number );
gboolean gsb_transactions_list_append_archive ( gint archive_number );
gboolean gsb_transactions_list_append_new_transaction ( gint transaction_number );
gint gsb_transactions_list_clone_transaction ( gint transaction_number );
gboolean gsb_transactions_list_delete_transaction ( gint transaction_number );
gboolean gsb_transactions_list_delete_transaction_from_tree_view ( gint transaction_number );
gboolean gsb_transactions_list_edit_current_transaction ( void );
gboolean gsb_transactions_list_edit_transaction ( gint transaction_number );
gboolean gsb_transactions_list_edit_transaction_by_pointer ( gint *transaction_number );
GtkTreeModel *gsb_transactions_list_get_filter (void);
GtkTreeModel *gsb_transactions_list_get_sortable (void);
GtkTreeStore *gsb_transactions_list_get_store (void);
GtkWidget *gsb_transactions_list_get_tree_view (void);
gboolean gsb_transactions_list_key_press ( GtkWidget *widget,
					   GdkEventKey *ev );
gboolean gsb_transactions_list_load_marked_r ( void );
GtkWidget *gsb_transactions_list_make_gui_list ( void );
gboolean gsb_transactions_list_move_to_current_transaction ( gint account_number );
gboolean gsb_transactions_list_select ( gint transaction_number );
gboolean gsb_transactions_list_set_adjustment_value ( gint account_number );
gboolean gsb_transactions_list_set_background_color ( gint account_number );
void gsb_transactions_list_set_store (GtkTreeStore *store);
gboolean gsb_transactions_list_set_transactions_balances ( gint account_number );
void gsb_transactions_list_set_visible_rows_number ( gint rows_number );
gboolean gsb_transactions_list_set_visibles_rows_on_account ( gint account_number );
gboolean gsb_transactions_list_update_transaction ( gint transaction_number );
gboolean gsb_transactions_list_update_transaction_value ( gint element_number );
void mise_a_jour_affichage_r ( gint affichage_r );
void move_selected_operation_to_account_nb ( gint *account );
gboolean new_transaction () ;
gint recupere_hauteur_ligne_tree_view ( GtkWidget *tree_view );
void remove_transaction ();
void schedule_selected_transaction ();
/* END_DECLARATION */
#endif
