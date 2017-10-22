#ifndef _ACCUEIL_H
#define _ACCUEIL_H (1)

#include <gtk/gtk.h>

/* Constantes pour la fonction gtk_misc_set_alignment() (GTK1) */
#define MISC_TOP 0
#define MISC_VERT_CENTER 0.5
#define MISC_BOTTOM 1
#define MISC_LEFT 0
#define MISC_HORIZ_CENTER 0.5
#define MISC_RIGHT 1



/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
void        affiche_dialogue_soldes_minimaux                        (void);
GtkWidget * creation_onglet_accueil                                 (void);
GtkWidget *	gsb_main_page_get_logo_accueil 							(void);
gboolean 	gsb_main_page_set_logo_accueil 							(GtkWidget *logo);
gboolean    gsb_main_page_update_finished_scheduled_transactions    (gint scheduled_number);
void        gsb_main_page_update_homepage_title                     (const gchar *title);
void        mise_a_jour_accueil                                     (gboolean force);
/* END_DECLARATION */
#endif
