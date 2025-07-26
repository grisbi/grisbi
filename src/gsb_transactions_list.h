#ifndef _OPERATIONS_LISTE_H
#define _OPERATIONS_LISTE_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
#include "gsb_real.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
void		gsb_gui_transaction_toolbar_set_style						(gint toolbar_style);
gboolean	gsb_transactions_list_change_aspect_liste					(gint demande);
void		gsb_transactions_list_clone_selected_transaction			(GtkWidget *menu_item,
																		 gpointer null);
GtkWidget *	gsb_transactions_list_creation_fenetre_operations			(void);
gint		gsb_transactions_list_find_element_col						(gint element_number);
gint		gsb_transactions_list_find_element_line						(gint element_number);
gboolean	gsb_transactions_list_add_transactions_from_archive			(gint archive_number,
																		 gint account_number,
																		 gboolean show_warning);
gboolean	gsb_transactions_list_append_new_transaction				(gint transaction_number,
																		 gboolean update_tree_view);
void		gsb_transactions_list_clone_template						(GtkWidget *menu_item,
																		 gpointer null);
void		gsb_transactions_list_convert_transaction_to_sheduled		(void);
gboolean	gsb_transactions_list_delete_transaction					(gint transaction_number,
																		 gint show_warning);
gboolean	gsb_transactions_list_delete_transaction_from_tree_view		(gint transaction_number);
gboolean	gsb_transactions_list_edit_current_transaction				(void);
gboolean	gsb_transactions_list_edit_transaction						(gint transaction_number);
void		gsb_transactions_list_edit_transaction_by_pointer			(gint *transaction_number);
gboolean	gsb_transactions_list_fill_archive_store					(void);
void 		gsb_transactions_list_free_titles_tips_col_list_ope			(void);
gint 		gsb_transactions_list_get_element_tab_affichage_ope			(gint dim_1,
																		 gint dim_2);
gchar *		gsb_transactions_list_get_column_title						(gint dim_1,
																		 gint dim_2);
gchar *		gsb_transactions_list_get_column_title_from_element			(gint element);
gint		gsb_transactions_list_get_current_tree_view_width 			(void);
gfloat		gsb_transactions_list_get_row_align							(void);
GsbReal		gsb_transactions_list_get_solde_debut_affichage				(gint account_number,
																		 gint floating_point);
gint *		gsb_transactions_list_get_tab_affichage_ope					(void);
gchar *		gsb_transactions_list_get_tab_affichage_ope_to_string		(void);
gint *		gsb_transactions_list_get_tab_align_col_treeview			(void);
gchar *		gsb_transactions_list_get_tab_align_col_treeview_to_string	(void);
gint *		gsb_transactions_list_get_tab_width_col_treeview			(void);
gchar *		gsb_transactions_list_get_tab_width_col_treeview_to_string	(void);
GtkWidget *	gsb_transactions_list_get_toolbar							(void);
GtkWidget *	gsb_transactions_list_get_tree_view							(void);
gchar *		gsb_transactions_list_grep_cell_content						(gint transaction_number,
																		 gint cell_content_number);
void		gsb_transactions_list_init_tab_affichage_ope				(const gchar *description);
void 		gsb_transactions_list_init_tab_align_col_treeview			(const gchar *description);
void 		gsb_transactions_list_init_tab_width_col_treeview			(const gchar *description);
GtkWidget *	gsb_transactions_list_make_gui_list							(void);
void		gsb_transactions_list_move_transaction_to_account_from_menu (gint source_account,
																		 gint target_account);
void		gsb_transactions_list_remove_transaction					(void);
gboolean	gsb_transactions_list_restore_archive						(gint archive_number,
																		 gboolean show_warning);
void		gsb_transactions_list_selection_changed						(gint new_selected_transaction);
void		gsb_transactions_list_set_current_tree_view_width 			(gint new_tree_view_width);
void 		gsb_transactions_list_set_element_tab_affichage_ope			(gint element_number,
																		 gint dim_1,
																		 gint dim_2);
gboolean	gsb_transactions_list_set_largeur_col						(void);
gboolean	gsb_transactions_list_set_row_align							(gfloat row_align);
void		gsb_transactions_list_set_titles_tips_col_list_ope			(void);
void		gsb_transactions_list_set_visible_archived_button			(gboolean visible);
void		gsb_transactions_list_set_visible_rows_number				(gint rows_number);
void		gsb_transactions_list_show_archives_lines					(gboolean show_l);
void		gsb_transactions_list_show_menu_import_rule					(gint account_number);
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
void		gsb_transactions_list_mise_a_jour_affichage_r				(gboolean show_r);
void		gsb_transactions_list_search								(GtkWidget *menu_item,
																		 gint *transaction_number);
void		gsb_transactions_list_select_new_transaction				(void);
void		gsb_transactions_list_update_titres_tree_view				(void);
/* END_DECLARATION */
#endif
