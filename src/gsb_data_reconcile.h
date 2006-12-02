#ifndef _GSB_DATA_RECONCILE_H
#define _GSB_DATA_RECONCILE_H (1)


/* START_INCLUDE_H */
#include "gsb_data_reconcile.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
const gchar *gsb_data_reconcile_get_name ( gint reconcile_number );
gint gsb_data_reconcile_get_no_reconcile ( gpointer reconcile_ptr );
gint gsb_data_reconcile_get_number_by_name ( const gchar *name );
GSList *gsb_data_reconcile_get_reconcile_list ( void );
gboolean gsb_data_reconcile_init_variables ( void );
gint gsb_data_reconcile_new ( const gchar *name );
gboolean gsb_data_reconcile_set_name ( gint reconcile_number,
				       const gchar *name );
gint gsb_data_reconcile_set_new_number ( gint reconcile_number,
					 gint new_no_reconcile );
/* END_DECLARATION */
#endif
