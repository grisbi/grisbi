#ifndef _AFFICHAGE_H
#define _AFFICHAGE_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
GtkWidget *onglet_display_addresses ( void );
GtkWidget * onglet_display_fonts ( void );
GtkWidget *tab_display_toolbar ( void );
void update_homepage_title (GtkEditable *entry,
							gpointer data);
/* END_DECLARATION */
#endif
