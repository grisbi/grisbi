#ifndef _UTILS_EDITABLES_H
#define _UTILS_EDITABLES_H (1)
/* START_INCLUDE_H */
#include "gsb_real.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean gsb_editable_erase_text_view ( GtkWidget *button,
					GtkWidget *text_view );
gchar *gsb_editable_text_view_get_content ( GtkWidget *text_view );
GtkWidget *gsb_editable_text_view_new ( const gchar *value );
gsb_real gsb_utils_edit_calculate_entry ( GtkWidget *entry );
void increment_decrement_champ ( GtkWidget *entry, gint increment );
/* END_DECLARATION */
#endif
