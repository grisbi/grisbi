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
/* END_DECLARATION */
#endif
