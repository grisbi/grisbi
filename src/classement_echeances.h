#ifndef _CLASSEMENT_ECHEANCES_H
#define _CLASSEMENT_ECHEANCES_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
gint	classement_gslist_echeance_par_date 					(gpointer scheduled_number_1,
																 gpointer scheduled_number_2,
																 gpointer manual);
gint 	classement_gslist_transactions_par_date 				(gpointer transaction_pointer_1,
																 gpointer transaction_pointer_2);
gint 	classement_gslist_transactions_par_date_decroissante 	(gpointer transaction_pointer_1,
																 gpointer transaction_pointer_2);
gint	classement_gslist_transactions_par_number				(gpointer number_pointer_1,
																 gpointer number_pointer_2,
																 GtkSortType sens);
/* END_DECLARATION */
#endif
