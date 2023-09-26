#ifndef _UTILS_WIDGETS_H
#define _UTILS_WIDGETS_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
/*COMMON_FUNCTIONS*/
void			utils_widget_gtk_callback						(GtkWidget *widget,
																 gpointer null);
/* DURATION WIDGET */
GtkWidget *		utils_widget_get_duration_widget 				(gint origin);
gboolean 		utils_widget_duration_button_released 			(GtkWidget *togglebutton,
																 GdkEventButton	*event,
																 GtkWidget *spin_button);
void			utils_widget_duration_number_changed			(GtkWidget *spin_button,
																 gpointer data);

/* IMPORT CSV_OPTIONS WIDGET */
void			utils_widget_import_csv_options_widget_new		(GtkWidget *parent,
																 gboolean fill,
																 GtkWidget *assistant);
void 			utils_widget_import_csv_options_set_combo_order	(GtkWidget *assistant,
																 gint import_format_order);
/* HISTORICAL_DATA WIDGET */
GtkWidget *		utils_widget_get_hist_data_widget				(GtkWidget *container,
																 GtkWidget *notebook);
/* ORIGIN_DATA WIDGET */
GtkWidget *		utils_widget_origin_data_new					(GtkWidget *notebook,
																 gint type);
void			utils_widget_origin_fyear_clicked				(GtkWidget *combo,
																 gpointer data);
/* FORMAT DATE AND NUMBER */
GtkWidget *		utils_widget_date_format_chosen					(GtkWidget *parent,
																 gint sens);
GtkWidget *		utils_widget_number_format_chosen				(GtkWidget *parent,
																 gint sens);
/* END_DECLARATION */
#endif
