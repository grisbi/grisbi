#ifndef _TIERS_ONGLET_H
#define _TIERS_ONGLET_H (1)
/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
gint classement_alphabetique_tree ( GtkWidget *tree,
				    GtkCListRow *ligne_1,
				    GtkCListRow *ligne_2 );
void mise_a_jour_combofix_tiers ( void );
GtkWidget *onglet_tiers ( void );
void remplit_arbre_tiers ( void );
/* END_DECLARATION */
#endif
