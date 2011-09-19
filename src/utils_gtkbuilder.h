#ifndef _H_UTILS_GTKBUILDER
#define _H_UTILS_GTKBUILDER 1

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
gchar *utils_gtkbuilder_get_full_path ( const gchar *name );
GtkWidget *utils_gtkbuilder_get_widget_by_name ( GtkBuilder *builder,
                        const gchar *gtk_builder_name,
                        const gchar *optional_name );
gint utils_gtkbuilder_merge_ui_data_in_builder ( GtkBuilder *builder,
                    const gchar *ui_name );
/* END_DECLARATION */

#endif /* _H_UTILS_GTKBUILDER */
