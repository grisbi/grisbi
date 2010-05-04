#ifndef _BALANCE_ESTIMATE_FUTURE_H
#define _BALANCE_ESTIMATE_FUTURE_H

#include <gtk/gtk.h>

/* START_INCLUDE_H */
#include "././gsb_real.h"
#include "././balance_estimate_data.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
void bet_future_initialise_dialog ( void );
gboolean bet_future_modify_line ( gint account_number,
                        gint number,
                        gint mother_row );
gboolean bet_future_new_line_dialog ( GtkTreeModel *tab_model,
                        gchar *str_date );
gboolean bet_transfert_modify_line ( gint account_number, gint number );
gboolean bet_transfert_new_line_dialog ( GtkTreeModel *tab_model,
                        gchar *str_date );
gchar *gsb_transfert_get_str_amount ( struct_transfert_data *transfert, gsb_real amount );
/* END_DECLARATION */

#endif /*_BALANCE_ESTIMATE_FUTURE_H*/

