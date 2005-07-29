#ifndef _OPERATIONS_LISTE_H
#define _OPERATIONS_LISTE_H (1)


/* define the no of column in the transactions list */

#define TRANSACTION_COL_NB_IS_EXPANDER 0
#define TRANSACTION_COL_NB_CHECK 1
#define TRANSACTION_COL_NB_DATE 2
#define TRANSACTION_COL_NB_PARTY 3
#define TRANSACTION_COL_NB_PR 4
#define TRANSACTION_COL_NB_DEBIT 5
#define TRANSACTION_COL_NB_CREDIT 6
#define TRANSACTION_COL_NB_BALANCE 7

#define TRANSACTION_LIST_COL_NB 8	 /*< must be the last of the list */

/* define the columns in the store
 * as the data are filled above, the number here
 * begin at TRANSACTION_LIST_COL_NB */

#define TRANSACTION_COL_NB_BACKGROUND 8		/*< color of the background */
#define TRANSACTION_COL_NB_AMOUNT_COLOR 9	/*< color of the amount */
#define TRANSACTION_COL_NB_TRANSACTION_ADDRESS 10
#define TRANSACTION_COL_NB_SAVE_BACKGROUND 11	/*< when selection, save of the normal color of background */
#define TRANSACTION_COL_NB_FONT 12		/*< PangoFontDescription if used */
#define TRANSACTION_COL_NB_TRANSACTION_LINE 13	/*< the line in the transaction (1, 2, 3 or 4) */
#define TRANSACTION_COL_NB_VISIBLE 14		/*< whether that transaction is visible or not */
#define TRANSACTION_COL_NB_PIXBUF_CLOSED 15
#define TRANSACTION_COL_NB_PIXBUF_OPEN 16
#define TRANSACTION_COL_NB_IS_EXPANDED 17
#define TRANSACTION_COL_NB_IS_NOT_BREAKDOWN 18

#define TRANSACTION_COL_NB_TOTAL 19


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
#include "operations_liste.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean affichage_traits_liste_operation ( void );
gboolean changement_taille_liste_ope ( GtkWidget *tree_view,
				       GtkAllocation *allocation );
gint cherche_ligne_operation ( gint transaction_number,
			       gint no_account );
void clone_selected_transaction ();
GtkWidget *creation_fenetre_operations ( void );
void demande_mise_a_jour_tous_comptes ( void );
gint find_p_r_col ();
gboolean gsb_data_account_list_set_breakdowns_visible ( gint mother_transaction_number,
						   gint visible );
gchar *gsb_transactions_get_category_real_name ( gint transaction_number );
gboolean gsb_transactions_list_append_transaction ( gint transaction_number,
						    GtkListStore *store );
gboolean gsb_transactions_list_append_white_line ( gint mother_transaction_number,
						   GtkListStore *store );
gboolean gsb_transactions_list_button_press ( GtkWidget *tree_view,
					      GdkEventButton *ev );
gint gsb_transactions_list_clone_transaction ( gint transaction_number );
gboolean gsb_transactions_list_delete_transaction ( gint transaction_number );
gboolean gsb_transactions_list_edit_current_transaction ( void );
gboolean gsb_transactions_list_fill_row ( gint transaction_number,
					  GtkTreeIter *iter,
					  GtkListStore *store,
					  gint line_in_transaction );
GtkTreeIter *gsb_transactions_list_get_iter_from_transaction ( gint transaction_number );
GtkListStore *gsb_transactions_list_get_store (void);
GtkWidget *gsb_transactions_list_get_tree_view (void);
gboolean gsb_transactions_list_key_press ( GtkWidget *widget,
					   GdkEventKey *ev );
GtkWidget *gsb_transactions_list_make_gui_list ( void );
gboolean gsb_transactions_list_move_to_current_transaction ( gint no_account );
gboolean gsb_transactions_list_set_adjustment_value ( gint account_number );
gboolean gsb_transactions_list_set_background_color ( gint no_account );
gboolean gsb_transactions_list_set_current_transaction ( gint transaction_number,
							 gint mother_transaction_number );
gboolean gsb_transactions_list_set_transactions_balances ( gint no_account );
gboolean gsb_transactions_list_set_visibles_rows_on_account ( gint no_account );
gboolean gsb_transactions_list_set_visibles_rows_on_transaction ( gint transaction_number );
void mise_a_jour_affichage_lignes ( gint nb_lignes );
void mise_a_jour_affichage_r ( gint affichage_r );
void mise_a_jour_labels_soldes ( void );
void move_selected_operation_to_account_nb ( gint *account );
void new_transaction () ;
gint recupere_hauteur_ligne_tree_view ( GtkWidget *tree_view );
void remove_transaction ();
void remplissage_liste_operations ( gint compte );
void update_titres_tree_view ( void );
/* END_DECLARATION */
#endif
