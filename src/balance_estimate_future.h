#ifndef _BALANCE_ESTIMATE_FUTURE_H
#define _BALANCE_ESTIMATE_FUTURE_H

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean bet_future_modify_line ( gint account_number,
                        gint number,
                        gint mother_row );
gboolean bet_future_new_line_dialog ( GtkTreeModel *tab_model,
                        gchar *str_date );
/* END_DECLARATION */

#endif /*_BALANCE_ESTIMATE_FUTURE_H*/

