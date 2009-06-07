#ifndef _AFFICHAGE_H
#define _AFFICHAGE_H (1)

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
GtkWidget *onglet_display_addresses ( void );
GtkWidget * onglet_display_fonts ( void );
GtkWidget *tab_display_toolbar ( void );
gboolean update_homepage_title (GtkEntry *entry, gchar *value,
                        gint length, gint * position);
/* END_DECLARATION */
#endif
