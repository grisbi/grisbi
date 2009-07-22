#ifndef _IMPUTATION_BUDGETAIRE_H
#define _IMPUTATION_BUDGETAIRE_H (1)
/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean gsb_budget_update_combofix ( void );
GtkWidget *onglet_imputations ( void );
void remplit_arbre_imputation ( void );
gboolean budgetary_hold_position_set_expand ( gboolean expand );
gboolean budgetary_hold_position_set_path ( GtkTreePath *path );
/* END_DECLARATION */
#endif
