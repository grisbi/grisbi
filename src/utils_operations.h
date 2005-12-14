#ifndef _UTILS_OPERATIONS_H
#define _UTILS_OPERATIONS_H (1)
/* START_INCLUDE_H */
#include "utils_operations.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
gpointer operation_par_cheque ( gint no_cheque,
				gint no_compte );
gpointer operation_par_id ( gchar *no_id,
					       gint no_compte );
void update_transaction_in_trees ( gint transaction_number );
/* END_DECLARATION */
#endif
