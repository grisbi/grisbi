#ifndef _EXERCICE_H
#define _EXERCICE_H (1)
/* START_INCLUDE_H */
#include "exercice.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
void affiche_exercice_par_date ( GtkWidget *entree_date,
				 GtkWidget *option_menu_exercice );
gint cherche_no_menu_exercice ( gint no_demande,
				GtkWidget *option_menu );
GtkWidget *creation_menu_exercices ( gint origine );
gint gsb_financial_year_get_number_from_option_menu ( GtkWidget *option_menu );
GtkWidget *onglet_exercices ( void );
/* END_DECLARATION */
#endif
