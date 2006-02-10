#ifndef _DEVISES_H
#define _DEVISES_H (1)


/* START_INCLUDE_H */
#include "gsb_currency.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
void gsb_currency_check_for_change ( gint transaction_number );
void gsb_currency_exchange_dialog ( gint account_currency_number,
				    gint transaction_currency_number ,
				    gint link_currency,
				    gdouble exchange_rate,
				    gdouble exchange_fees,
				    gboolean force );
gint gsb_currency_get_currency_from_combobox ( GtkWidget *combo_box );
gdouble gsb_currency_get_current_exchange (void);
gdouble gsb_currency_get_current_exchange_fees (void);
void gsb_currency_init_variables ( void );
GtkWidget *gsb_currency_make_combobox ( gboolean set_name );
gboolean gsb_currency_set_combobox_history ( GtkWidget *combo_box,
					     gint currency_number );
gboolean gsb_currency_update_currency_list ( void );
/* END_DECLARATION */
#endif
