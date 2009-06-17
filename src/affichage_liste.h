#ifndef _AFFICHAGE_LISTE_H
#define _AFFICHAGE_LISTE_H (1)

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean display_mode_check_line ( gint line_in_transaction,
                        gint visibles_lines );
GtkWidget *onglet_affichage_operations ( void );
GtkWidget *onglet_diverse_form_and_lists ( void );
GtkWidget *onglet_form_completion ( void );
void recuperation_noms_colonnes_et_tips ( void );
/* END_DECLARATION */
#endif
