#ifndef _PARAMETRES_H
#define _PARAMETRES_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/*START_DECLARATION*/
GtkWidget * gsb_config_date_format_chosen               (GtkWidget *parent,
                                                         gint sens);
gboolean    gsb_config_metatree_re_sort_divisions       (GtkWidget *checkbutton,
                                                         GdkEventButton *event,
                                                         gpointer data);
GtkWidget * gsb_config_number_format_chosen             (GtkWidget *parent,
                                                         gint sens);
/* PROVISOIRE */
GtkWidget *	onglet_accueil 								(void);
GtkWidget * onglet_delete_messages 						(void);
GtkWidget *	onglet_messages_and_warnings 				(void);
GtkWidget * onglet_metatree 							(void);
gboolean 	gsb_gui_encryption_toggled 					(GtkWidget *checkbox,
														 gpointer data);
/*END_DECLARATION*/

#endif

