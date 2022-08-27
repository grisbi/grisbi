#ifndef _GSB_RECONCILE_H
#define _GSB_RECONCILE_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
gint 			gsb_reconcile_get_last_scheduled_transaction		(void);
gboolean		gsb_reconcile_set_last_scheduled_transaction		(gint scheduled_transaction);
const GDate *	gsb_reconcile_get_pointed_transactions_max_date		(gint account_number);
/* END_DECLARATION */
#endif
