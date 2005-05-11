#ifndef _ACCUEIL_H
#define _ACCUEIL_H (1)

/* Constantes pour la fonction gtk_misc_set_alignment() (GTK1) */
#define MISC_TOP 0
#define MISC_VERT_CENTER 0.5
#define MISC_BOTTOM 1
#define MISC_LEFT 0
#define MISC_HORIZ_CENTER 0.5
#define MISC_RIGHT 1



/* START_INCLUDE_H */
#include "structures.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
void affiche_dialogue_soldes_minimaux ( void );
GtkWidget *creation_onglet_accueil ( void );
gboolean gsb_main_page_update_finished_scheduled_transactions ( struct operation_echeance *scheduled_transaction );
void mise_a_jour_accueil ( void );
/* END_DECLARATION */
#endif
