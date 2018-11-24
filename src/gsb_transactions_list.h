#ifndef _OPERATIONS_LISTE_H
#define _OPERATIONS_LISTE_H (1)

#include <gtk/gtk.h>

/* the element number for each showable in the list */
enum
{
    ELEMENT_DATE = 1,
    ELEMENT_VALUE_DATE,
    ELEMENT_PARTY,
    ELEMENT_BUDGET,
    ELEMENT_DEBIT,
    ELEMENT_CREDIT,
    ELEMENT_BALANCE,
    ELEMENT_AMOUNT,
    ELEMENT_PAYMENT_TYPE,
    ELEMENT_RECONCILE_NB,
    ELEMENT_EXERCICE,
    ELEMENT_CATEGORY,
    ELEMENT_MARK,
    ELEMENT_VOUCHER,
    ELEMENT_NOTES,
    ELEMENT_BANK,
    ELEMENT_NO,
    ELEMENT_CHQ
};

/* START_INCLUDE_H */
#include "gsb_real.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean	change_aspect_liste											(gint demande);
gboolean	clone_selected_transaction									(GtkWidget *menu_item,
																		 gpointer null);
GtkWidget *	creation_fenetre_operations									(void);
gint		find_element_col											(gint element_number);
gint		find_element_col_for_archive								(void);
gint		find_element_col_split										(gint element_number);
gint		find_element_line											(gint element_number);
void		gsb_gui_transaction_toolbar_set_style						(gint toolbar_style);
gchar *		gsb_transactions_get_category_real_name						(gint transaction_number);
gboolean	gsb_transactions_list_add_transactions_from_archive			(gint archive_number,
																		 gint account_number,
																		 gboolean show_warning);
gboolean	gsb_transactions_list_append_new_transaction				(gint transaction_number,
																		 gboolean update_tree_view);
gboolean	gsb_transactions_list_clone_template						(GtkWidget *menu_item,
																		 gpointer null);
gboolean	gsb_transactions_list_delete_transaction					(gint transaction_number,
																		 gint show_warning);
gboolean	gsb_transactions_list_delete_transaction_from_tree_view		(gint transaction_number);
gboolean	gsb_transactions_list_edit_current_transaction				(void);
gboolean	gsb_transactions_list_edit_transaction						(gint transaction_number);
gboolean	gsb_transactions_list_edit_transaction_by_pointer			(gint *transaction_number);
gboolean	gsb_transactions_list_fill_archive_store					(void);
gint		gsb_transactions_list_get_current_tree_view_width 			(void);
gfloat		gsb_transactions_list_get_row_align							(void);
GtkWidget *	gsb_transactions_list_get_toolbar							(void);
gchar *		gsb_transaction_list_get_titre_colonne_liste_ope			(gint element);
GtkWidget *	gsb_transactions_list_get_tree_view							(void);
gchar *		gsb_transactions_list_grep_cell_content						(gint transaction_number,
																		 gint cell_content_number);
GtkWidget *	gsb_transactions_list_make_gui_list							(void);
gboolean	gsb_transactions_list_restore_archive						(gint archive_number,
																		 gboolean show_warning);
void		gsb_transactions_list_selection_changed						(gint new_selected_transaction);
gboolean	gsb_transactions_list_set_largeur_col						(void);
gboolean	gsb_transactions_list_set_row_align							(gfloat row_align);
void		gsb_transaction_list_set_visible_archived_button			(gboolean visible);
void		gsb_transactions_list_set_visible_rows_number				(gint rows_number);
void		gsb_transactions_list_show_archives_lines					(gboolean show_l);
void		gsb_transactions_list_splitted_to_scheduled					(gint transaction_number,
																		 gint scheduled_number);
gboolean	gsb_transactions_list_switch_expander						(gint transaction_number);
gboolean	gsb_transactions_list_transaction_visible					(gpointer transaction_ptr,
																		 gint account_number,
																		 gint line_in_transaction,
																		 gint what_is_line);
gboolean	gsb_transactions_list_update_transaction					(gint transaction_number);
void		gsb_transactions_list_update_tree_view						(gint account_number,
																		 gboolean keep_selected_transaction);
void		mise_a_jour_affichage_r										(gboolean show_r);
void		move_selected_operation_to_account_nb						(gint source_account,
																		 gint target_account);
gboolean	new_transaction												(void);
void		remove_transaction											(void);
void		schedule_selected_transaction								(void);
GsbReal		solde_debut_affichage										(gint account_number,
																		 gint floating_point);
void		update_titres_tree_view										(void);

/* END_DECLARATION */
#endif
