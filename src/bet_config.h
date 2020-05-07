#ifndef _BALANCE_ESTIMATE_CONFIG_H
#define _BALANCE_ESTIMATE_CONFIG_H

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
void 		bet_config_duration_button_clicked 				(GtkWidget *togglebutton,
															 GdkEventButton	*event,
															 GtkWidget *spin_button);
gboolean 	bet_config_duration_number_changed 				(GtkWidget *spin_button,
															 gpointer data);
void 		bet_config_fyear_clicked 						(GtkWidget *combo,
															 gpointer data);
GtkWidget *	bet_config_get_duration_widget 					(gint origin);
gdouble		bet_config_get_prev_month_max					(void);
GtkWidget *	bet_config_get_select_historical_data 			(GtkWidget *container,
															 GtkWidget *notebook);
GtkWidget *	bet_config_get_select_labels_widget 			(GtkWidget *container);
void 		bet_config_initialise_duration_widget 			(gint account_number,
															 GtkWidget *notebook);
void 		bet_config_initialise_select_historical_data 	(gint account_number,
															 GtkWidget *notebook );
void 		bet_config_origin_data_clicked 					(GtkWidget *togglebutton,
															 GdkEventButton *event,
															 gpointer data);
/* END_DECLARATION */
#endif
