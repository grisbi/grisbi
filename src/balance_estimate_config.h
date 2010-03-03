#ifndef _BALANCE_ESTIMATE_CONFIG_H
#define _BALANCE_ESTIMATE_CONFIG_H

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
GtkWidget *bet_config_create_page ( void );
void bet_config_duration_button_clicked ( GtkWidget *togglebutton,
                        GtkWidget *spin_button );
gboolean bet_config_duration_number_changed ( GtkWidget *spin_button,
                        GtkWidget *togglebutton );
void bet_config_duration_period_clicked ( GtkWidget *togglebutton,
                        GtkWidget *button );
/* END_DECLARATION */


#endif
