#ifndef _DEVISES_H
#define _DEVISES_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
#include "gsb_real.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
void 		gsb_currency_check_for_change 				(gint transaction_number);
void 		gsb_currency_exchange_dialog 				(gint account_currency_number,
														 gint transaction_currency_number ,
														 gboolean link_currency,
														 GsbReal exchange_rate,
														 GsbReal exchange_fees,
														 gboolean force);
gint 		gsb_currency_get_currency_from_combobox 	(GtkWidget *combo_box);
GsbReal 	gsb_currency_get_current_exchange 			(void);
GsbReal		gsb_currency_get_current_exchange_fees		(void);
void 		gsb_currency_init_exchanges 				(void);
void 		gsb_currency_init_variables 				(void);
GtkWidget *	gsb_currency_make_combobox 					(gboolean set_name);
gboolean 	gsb_currency_set_combobox_history 			(GtkWidget *combo_box,
														 gint currency_number);
gboolean 	gsb_currency_set_current_exchange 			(GsbReal exchange);
gboolean 	gsb_currency_set_current_exchange_fees 		(GsbReal fees);
gboolean 	gsb_currency_update_combobox_currency_list 	(void);
/* END_DECLARATION */
#endif
