#ifndef _GSB_DATA_RECONCILE_H
#define _GSB_DATA_RECONCILE_H (1)


/* START_INCLUDE_H */
#include "gsb_real.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
gint gsb_data_reconcile_get_account ( gint reconcile_number );
gint gsb_data_reconcile_get_account_last_number ( gint account_number );
gsb_real gsb_data_reconcile_get_final_balance ( gint reconcile_number );
const GDate *gsb_data_reconcile_get_final_date ( gint reconcile_number );
gsb_real gsb_data_reconcile_get_init_balance ( gint reconcile_number );
const GDate *gsb_data_reconcile_get_init_date ( gint reconcile_number );
const gchar *gsb_data_reconcile_get_name ( gint reconcile_number );
gint gsb_data_reconcile_get_no_reconcile ( gpointer reconcile_ptr );
gint gsb_data_reconcile_get_number_by_date ( const GDate *date,
                        gint account_number );
gint gsb_data_reconcile_get_number_by_name ( const gchar *name );
GList *gsb_data_reconcile_get_reconcile_list ( void );
GList *gsb_data_reconcile_get_sort_reconcile_list ( gint account_number );
gboolean gsb_data_reconcile_init_variables ( void );
gint gsb_data_reconcile_max_number ( void );
gint gsb_data_reconcile_new ( const gchar *name );
gboolean gsb_data_reconcile_remove ( gint reconcile_number );
gboolean gsb_data_reconcile_set_account ( gint reconcile_number,
                        gint account_number );
gboolean gsb_data_reconcile_set_final_balance ( gint reconcile_number,
						gsb_real amount );
gboolean gsb_data_reconcile_set_final_date ( gint reconcile_number,
                        const GDate *date );
gboolean gsb_data_reconcile_set_init_balance ( gint reconcile_number,
					       gsb_real amount );
gboolean gsb_data_reconcile_set_init_date ( gint reconcile_number,
					    const GDate *date );
gboolean gsb_data_reconcile_set_name ( gint reconcile_number,
                        const gchar *name );
gint gsb_data_reconcile_set_new_number ( gint reconcile_number,
                        gint new_no_reconcile );
/* END_DECLARATION */
#endif
