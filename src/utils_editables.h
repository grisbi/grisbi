#ifndef _UTILS_EDITABLES_H
#define _UTILS_EDITABLES_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean gsb_editable_erase_text_view ( GtkWidget *button,
					GtkWidget *text_view );
gchar *gsb_editable_text_view_get_content ( GtkWidget *text_view );
GtkWidget *gsb_editable_text_view_new ( const gchar *value );
void increment_decrement_champ ( GtkWidget *entry, gint increment );
/* END_DECLARATION */
#endif
