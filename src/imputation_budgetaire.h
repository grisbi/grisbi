#ifndef _IMPUTATION_BUDGETAIRE_H
#define _IMPUTATION_BUDGETAIRE_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
GtkWidget *budgetary_lines_create_list ( void );
void budgetary_lines_fill_list ( void );
void budgetary_lines_exporter_list ( void );
void budgetary_lines_importer_list ( void );
void budgetary_lines_init_variables_list ( void );
void budgetary_lines_update_toolbar_list ( void );
GtkTreeStore *budgetary_lines_get_tree_store ( void );
GtkWidget *budgetary_lines_get_tree_view ( void );

void budgetary_lines_new_budgetary_line ( void );
void budgetary_lines_delete_budgetary_line ( void );
void budgetary_lines_edit_budgetary_line ( void );

GtkTreePath *budgetary_hold_position_get_path ( void );
gboolean budgetary_hold_position_set_expand ( gboolean expand );
gboolean budgetary_hold_position_set_path ( GtkTreePath *path );
gboolean gsb_budget_update_combofix ( gboolean force );
/* END_DECLARATION */
#endif
