#ifndef _CLASSEMENT_OPERATIONS_H
#define _CLASSEMENT_OPERATIONS_H (1)
/* START_INCLUDE_H */
#include "classement_operations.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
gint gsb_strcasecmp ( gchar *string_1,
		      gchar *string_2 );
gint gsb_transactions_list_sort_column ( GtkTreeModel *model,
					 GtkTreeIter *iter_1,
					 GtkTreeIter *iter_2,
					 gint *column_number );
/* END_DECLARATION */
#endif
