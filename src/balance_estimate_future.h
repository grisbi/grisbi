#ifndef _BALANCE_ESTIMATE_FUTURE_H
#define _BALANCE_ESTIMATE_FUTURE_H

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean bet_future_configure_form_to_future ( void );
gboolean bet_future_configure_form_to_transaction ( void );
GtkWidget * bet_future_create_page ( void );
gboolean bet_future_new_line_dialog ( GtkWidget *tree_view,
                        GtkTreeModel *tab_model,
                        gchar *str_date );
/* END_DECLARATION */

#endif /*_BALANCE_ESTIMATE_FUTURE_H*/

