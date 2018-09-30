#ifndef _GSB_COMBO_BOX_H
#define _GSB_COMBO_BOX_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
gchar *			gsb_combo_box_get_active_text 				(GtkComboBox *combo_box);
gint 			gsb_combo_box_get_index 					(GtkWidget *combo_box);
GtkListStore *	gsb_combo_box_list_store_new_from_array		(gchar **array);
GtkWidget *		gsb_combo_box_new_with_index 				(gchar **string,
															 GCallback func,
															 gpointer data);
GtkWidget *		gsb_combo_box_new_with_index_from_list 		(GSList *list,
															 GCallback func,
															 gpointer data);
gint 			gsb_combo_box_set_index 					(GtkWidget *combo_box,
															 gint index);
/* COMBO_BOX WITH COMPLETION */
void			gsb_combo_form_box_append_text 				(GtkComboBox *combo,
															 const gchar *text);
void			gsb_combo_form_box_block_unblock_by_func	(GtkWidget *combo_box,
															 gboolean block);
GtkWidget *		gsb_combo_form_box_get_entry				(GtkComboBox *combo_box);
GtkWidget *		gsb_combo_form_box_new 						(GSList *list,
															 GCallback func,
															 gpointer data);
void			gsb_combo_form_box_remove_text				(GtkComboBox *combo,
															 const gchar *text);
gboolean 		gsb_combo_form_box_set_list 				(GtkComboBox *combo_box,
															 GSList *list);
/* END_DECLARATION */
#endif
