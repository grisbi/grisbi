#ifndef _GSB_COMBO_BOX_H
#define _GSB_COMBO_BOX_H (1)


/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
gint gsb_combo_box_get_index ( GtkWidget *combo_box );
GtkWidget *gsb_combo_box_new_with_index ( gchar **string,
					  GCallback func,
					  gpointer data );
GtkWidget *gsb_combo_box_new_with_index_by_list ( GSList *list,
						  GCallback func,
						  gpointer data );
gint gsb_combo_box_set_index ( GtkWidget *combo_box,
			       gint index );
/* END_DECLARATION */
#endif
