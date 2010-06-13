#ifndef _TRANSACTION_MODEL_H
#define _TRANSACTION_MODEL_H (1)


/* START_INCLUDE_H */
#include "custom_list.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
gboolean transaction_model_get_iter (GtkTreeIter  *iter,
				     GtkTreePath  *path);
CustomList *transaction_model_get_model (void);
gboolean transaction_model_get_transaction_iter ( GtkTreeIter *iter,
						  gint transaction_number,
						  gint line_in_transaction );
void transaction_model_initialize ( void );
gboolean transaction_model_iter_next (GtkTreeIter *iter);
void transaction_model_set_model ( CustomList *new_custom_list );
/* END_DECLARATION */
#endif
