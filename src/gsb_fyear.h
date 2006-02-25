#ifndef _GSB_FYEAR_H
#define _GSB_FYEAR_H (1)


/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
void affiche_exercice_par_date ( GtkWidget *entree_date,
				 GtkWidget *option_menu_exercice );
gint cherche_no_menu_exercice ( gint no_demande,
				GtkWidget *option_menu );
GtkWidget *creation_menu_exercices ( gint origine );
gint gsb_financial_year_get_number_from_option_menu ( GtkWidget *option_menu );
gboolean update_financial_year_menus ();
/* END_DECLARATION */
#endif
