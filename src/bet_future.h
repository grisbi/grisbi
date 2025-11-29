#ifndef _BALANCE_ESTIMATE_FUTURE_H
#define _BALANCE_ESTIMATE_FUTURE_H

#include <gtk/gtk.h>

/* START_INCLUDE_H */
#include "gsb_real.h"
#include "bet_data.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
/* BET_FORM */
gboolean	bet_form_entry_get_focus		(GtkWidget *entry,
											 GdkEvent *ev,
											 GtkWidget *dialog);



/* BET_FUTURE */
void 		bet_future_initialise_dialog 	(gboolean cleanup);
gboolean 	bet_future_modify_line 			(gint account_number,
											 gint number,
											 gint mother_row);
gboolean 	bet_future_new_line_dialog 		(GtkTreeModel *tab_model,
											 gchar *str_date);


/* BET_TRANSFERT */
void	 	bet_transfert_modify_line 		(gint account_number,
											 gint number);
gboolean 	bet_transfert_new_line_dialog 	(GtkTreeModel *tab_model,
											 gchar *str_date);
GsbReal 	gsb_transfert_get_str_amount 	(GsbReal amount,
											 gint account_currency,
											 gint replace_currency,
											 gint floating_point);


/* BET_UTILS */
void	bet_utils_get_budget_data			(GtkWidget *widget,
											 gint struct_type,
											 gpointer *value);
void 	bet_utils_get_category_data			(GtkWidget *widget,
											 gint struct_type,
											 gpointer *value);
void	bet_utils_get_payee_data			(GtkWidget *widget,
											 gint struct_type,
											 gpointer *value);
void	bet_utils_get_payment_data			(GtkWidget *widget,
											 gint struct_type,
											 gpointer *value);
/* END_DECLARATION */

#endif /*_BALANCE_ESTIMATE_FUTURE_H*/

