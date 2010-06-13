#ifndef _GSB_DATA_ARCHIVE_STORE_H
#define _GSB_DATA_ARCHIVE_STORE_H (1)


/* START_INCLUDE_H */
#include "gsb_real.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
void gsb_data_archive_store_create_list ( void );
gint gsb_data_archive_store_get_account_number ( gint archive_store_number );
gint gsb_data_archive_store_get_archive_number ( gint archive_store_number );
gsb_real gsb_data_archive_store_get_archives_balance ( gint account_number );
GSList *gsb_data_archive_store_get_archives_list ( void );
gsb_real gsb_data_archive_store_get_balance ( gint archive_store_number );
gint gsb_data_archive_store_get_number ( gpointer archive_ptr );
gpointer gsb_data_archive_store_get_structure ( gint archive_store_number );
gint gsb_data_archive_store_get_transactions_number ( gint archive_store_number );
gboolean gsb_data_archive_store_init_variables ( void );
gboolean gsb_data_archive_store_remove ( gint archive_store_number );
gboolean gsb_data_archive_store_remove_by_archive ( gint archive_number );
/* END_DECLARATION */
#endif
