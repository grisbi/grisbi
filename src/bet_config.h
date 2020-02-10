#ifndef _BALANCE_ESTIMATE_CONFIG_H
#define _BALANCE_ESTIMATE_CONFIG_H

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
GtkWidget *	bet_config_account_create_account_page 			(void);
GtkWidget *	bet_config_account_get_finance_data 			(gchar *title);
void 		bet_config_duration_button_clicked 				(GtkWidget *togglebutton,
															 GdkEventButton	*event,
															 GtkWidget *spin_button);
gboolean 	bet_config_duration_number_changed 				(GtkWidget *spin_button,
															 gpointer data);
void 		bet_config_fyear_clicked 						(GtkWidget *combo,
															 gpointer data);
GtkWidget *	bet_config_general_create_general_page 			(void);
GtkWidget *	bet_config_get_duration_widget 					(gint origin);
gdouble		bet_config_get_prev_month_max					(void);
GtkWidget *	bet_config_get_select_historical_data 			(GtkWidget *container,
															 GtkWidget *notebook);
GtkWidget *	bet_config_get_select_labels_widget 			(GtkWidget *container);
void 		bet_config_initialise_duration_widget 			(gint account_number,
															 GtkWidget *notebook);
void 		bet_config_initialise_finance_widget 			(gint account_number,
															 GtkWidget *notebook);
void 		bet_config_initialise_select_historical_data 	(gint account_number,
															 GtkWidget *notebook );
void 		bet_config_initialise_select_bank_card 			(gint account_number);
void 		bet_config_origin_data_clicked 					(GtkWidget *togglebutton,
															 GdkEventButton *event,
															 gpointer data);
void 		bet_config_select_bank_card_toggle 				(GtkToggleButton *button,
															 GtkWidget *combo);
/* END_DECLARATION */
#endif
