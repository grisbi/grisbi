#ifndef _BET_FINANCE_UI_H
#define _BET_FINANCE_UI_H

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */



/* START_DECLARATION */
GtkWidget *bet_finance_create_account_page ( void );
GtkWidget *bet_finance_create_page ( void );
GtkWidget *bet_finance_get_capital_entry ( void );
gdouble bet_finance_get_number_from_string ( GtkWidget *parent, const gchar *name );
void bet_finance_switch_simulator_page ( void );
void bet_finance_ui_update_amortization_tab ( gint account_number );
/* END_DECLARATION */


#endif /*_BET_FINANCE_UI_H*/
