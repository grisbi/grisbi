#ifndef _UTILS_H
#define _UTILS_H (1)
/* START_INCLUDE_H */
#include "utils.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean assert_account_loaded ();
void desensitive_widget ( GtkWidget *widget );
gboolean lance_navigateur_web ( const gchar *url );
gboolean met_en_normal ( GtkWidget *event_box,
			 GdkEventMotion *event,
			 gpointer pointeur );
gboolean met_en_prelight ( GtkWidget *event_box,
			   GdkEventMotion *event,
			   gpointer pointeur );
GtkWidget *new_paddingbox_with_title (GtkWidget * parent, gboolean fill, gchar * title);
GtkWidget *new_vbox_with_title_and_icon ( gchar * title,
					  gchar * image_filename);
void sens_desensitive_pointeur ( GtkWidget *bouton,
				 GtkWidget *widget );
void sensitive_widget ( GtkWidget *widget );
void update_ecran ( void );
/* END_DECLARATION */
#endif
