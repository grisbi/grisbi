#ifndef _UTILS_EDITABLES_H
#define _UTILS_EDITABLES_H (1)
/* START_INCLUDE_H */
#include "utils_editables.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
void entry_set_value ( GtkWidget * entry, gchar ** value );
GtkWidget *gsb_editable_new_text_area ( gchar *value,
					GCallback hook,
					gpointer data,
					GCallback default_func,
					gint number_for_func );
GtkWidget *gsb_editable_new_text_entry ( gchar *value,
					 GCallback hook,
					 gpointer data,
					 GCallback default_func,
					 gint number_for_func );
void gsb_editable_set_value ( GtkWidget *entry,
			      gchar *value,
			      gint number_for_func );
void gsb_editable_text_area_set_value ( GtkWidget *text_view,
					gchar *value,
					gint number_for_func );
gchar *gsb_editable_text_view_get_content ( GtkWidget *text_view );
GtkWidget *gsb_editable_text_view_new ( gchar *value );
gsb_real gsb_utils_edit_calculate_entry ( GtkWidget *entry );
void increment_decrement_champ ( GtkWidget *entry, gint increment );
GtkWidget * new_text_area ( gchar ** value, GCallback hook );
GtkWidget * new_text_entry ( gchar ** value, GCallback hook, gpointer data );
/* END_DECLARATION */
#endif
