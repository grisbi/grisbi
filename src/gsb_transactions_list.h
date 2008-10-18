#ifndef _OPERATIONS_LISTE_H
#define _OPERATIONS_LISTE_H (1)



/* the element number for each showable in the list */
enum
{
    ELEMENT_DATE = 1,
    ELEMENT_VALUE_DATE ,
    ELEMENT_PARTY ,
    ELEMENT_BUDGET ,
    ELEMENT_DEBIT ,
    ELEMENT_CREDIT ,
    ELEMENT_BALANCE ,
    ELEMENT_AMOUNT ,
    ELEMENT_TYPE ,
    ELEMENT_RECONCILE_NB ,
    ELEMENT_EXERCICE ,
    ELEMENT_CATEGORY ,
    ELEMENT_MARK ,
    ELEMENT_VOUCHER ,
    ELEMENT_NOTES ,
    ELEMENT_BANK ,
    ELEMENT_NO ,
    ELEMENT_CHQ ,
};


/* used for gtk_tree_view_column_set_alignment() (GTK2) */
#define COLUMN_LEFT 0.0
#define COLUMN_CENTER 0.5
#define COLUMN_RIGHT 1.0

/* START_INCLUDE_H */
#include "././gsb_real.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
void clone_selected_transaction ();
GtkWidget *creation_fenetre_operations ( void );
gint find_element_col ( gint element_number );
gint find_element_col_split ( gint element_number );
gint find_element_line ( gint element_number );
gchar *gsb_transactions_get_category_real_name ( gint transaction_number );
gboolean gsb_transactions_list_append_new_transaction ( gint transaction_number,
							gboolean update_tree_view );
gboolean gsb_transactions_list_delete_transaction ( gint transaction_number,
						    gint show_warning );
gboolean gsb_transactions_list_delete_transaction_from_tree_view ( gint transaction_number );
gboolean gsb_transactions_list_edit_current_transaction ( void );
gboolean gsb_transactions_list_edit_transaction ( gint transaction_number );
gboolean gsb_transactions_list_edit_transaction_by_pointer ( gint *transaction_number );
gfloat gsb_transactions_list_get_row_align ( void );
GtkWidget *gsb_transactions_list_get_tree_view (void);
gchar *gsb_transactions_list_grep_cell_content ( gint transaction_number,
						 gint cell_content_number );
gboolean gsb_transactions_list_key_press ( GtkWidget *widget,
					   GdkEventKey *ev );
GtkWidget *gsb_transactions_list_make_gui_list ( void );
gboolean gsb_transactions_list_restore_archive ( gint archive_number,
						 gboolean show_warning );
void gsb_transactions_list_selection_changed ( gint new_selected_transaction );
gboolean gsb_transactions_list_set_row_align ( gfloat row_align );
void gsb_transactions_list_set_visible_rows_number ( gint rows_number );
gboolean gsb_transactions_list_switch_expander ( gint transaction_number );
gboolean gsb_transactions_list_transaction_visible ( gpointer transaction_ptr,
						     gint account_number,
						     gint line_in_transaction,
						     gint what_is_line );
gboolean gsb_transactions_list_update_transaction ( gint transaction_number );
void gsb_transactions_list_update_tree_view ( gint account_number,
					      gboolean keep_selected_transaction );
void mise_a_jour_affichage_r ( gboolean show_r );
void move_selected_operation_to_account_nb ( gint *account );
gboolean new_transaction () ;
void remove_transaction ();
void schedule_selected_transaction ();
gsb_real solde_debut_affichage ( gint account_number,
				 gint floating_point);
/* END_DECLARATION */
#endif
