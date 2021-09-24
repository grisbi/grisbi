#ifndef _UTILS_WIDGETS_H
#define _UTILS_WIDGETS_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
/*COMMON_FUNCTIONS*/
void			utils_widget_gtk_callback					(GtkWidget *widget,
															 gpointer null);
/* DURATION WIDGET */
GtkWidget *		utils_widget_get_duration_widget 			(gint origin);
gboolean 		utils_widget_duration_button_released 		(GtkWidget *togglebutton,
															 GdkEventButton	*event,
															 GtkWidget *spin_button);
void			utils_widget_duration_number_changed		(GtkWidget *spin_button,
															 gpointer data);

/* HISTORICAL_DATA WIDGET */
GtkWidget *		utils_widget_get_hist_data_widget			(GtkWidget *container,
															 GtkWidget *notebook);
/* ORIGIN_DATA WIDGET */
GtkWidget *		utils_widget_origin_data_new				(GtkWidget *notebook,
						                                     gint type);
void			utils_widget_origin_fyear_clicked			(GtkWidget *combo,
								                             gpointer data);
/* END_DECLARATION */
#endif
