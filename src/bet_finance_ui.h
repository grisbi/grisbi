#ifndef _BET_FINANCE_UI_H
#define _BET_FINANCE_UI_H

#include <gtk/gtk.h>

/* START_INCLUDE_H */
#include "bet_data_finance.h"
/* END_INCLUDE_H */



/* START_DECLARATION */
GsbReal					bet_finance_get_loan_amount_at_date					(gint scheduled_number,
																			 gint transfer_account,
																			 const GDate *date,
																			 gboolean maj_s_loan_capital_du);
AmortissementStruct *	bet_finance_get_echeance_first 						(LoanStruct *s_loan,
																			 GDate *first_date);
gdouble 				bet_finance_get_number_from_string 					(GtkWidget *parent,
																			 const gchar *name);
void 					bet_finance_update_amortization_tab 				(gint account_number);
void 					bet_finance_update_amortization_tab_with_data 		(gint account_number,
																			 GtkWidget *parent,
																			 LoanStruct *s_loan);
GtkWidget *				bet_finance_ui_create_account_amortization_page 	(void);
GtkWidget *				bet_finance_ui_create_amortization_heading			(GtkWidget *parent);
GtkWidget *				bet_finance_ui_create_amortization_tree_view 		(GtkWidget *container,
																			 gint origin);
GtkWidget *				bet_finance_ui_create_loan_simulator 				(void);
GtkWidget *				bet_finance_ui_create_simulator_toolbar 			(GtkWidget *parent,
																			 GtkWidget *tree_view,
																			 gboolean simulator,
																			 gboolean amortization);
GtkWidget *				bet_finance_ui_get_capital_entry 					(void);
void 					bet_finance_ui_switch_simulator_page 				(void);
void 					bet_finance_ui_update_all_finance_toolbars 			(gint toolbar_style);
/* END_DECLARATION */

#endif /*_BET_FINANCE_UI_H*/
