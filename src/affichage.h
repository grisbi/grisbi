#ifndef _AFFICHAGE_H
#define _AFFICHAGE_H (1)
/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
GtkWidget *onglet_display_addresses ( void );
GtkWidget * onglet_display_fonts ( void );
GtkWidget *onglet_display_transaction_form ( void );
GtkWidget *tab_display_toolbar ( void );
gboolean update_transaction_form ( GtkWidget * checkbox, gpointer data );
/* END_DECLARATION */
#endif
