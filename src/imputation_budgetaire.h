#ifndef _IMPUTATION_BUDGETAIRE_H
#define _IMPUTATION_BUDGETAIRE_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
GtkTreePath *budgetary_hold_position_get_path ( void );
gboolean budgetary_hold_position_set_expand ( gboolean expand );
gboolean budgetary_hold_position_set_path ( GtkTreePath *path );
void budgetary_line_exporter_ib ( void );
void budgetary_line_importer_ib ( void );
GtkTreeStore *budgetary_line_get_tree_store ( void );
GtkWidget *budgetary_line_get_tree_view ( void );
void budgetary_line_new_imputation ( void );
gboolean gsb_budget_update_combofix ( gboolean force );
void gsb_gui_update_budgetary_toolbar ( void );
GtkWidget *onglet_imputations ( void );
void remplit_arbre_imputation ( void );
/* END_DECLARATION */
#endif
