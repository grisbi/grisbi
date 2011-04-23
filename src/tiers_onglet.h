#ifndef _TIERS_ONGLET_H
#define _TIERS_ONGLET_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
gint classement_alphabetique_tree ( GtkWidget *tree,
                        GtkCListRow *ligne_1,
                        GtkCListRow *ligne_2 );
GtkWidget *gsb_payee_get_toolbar ( void );
GtkTreeStore *gsb_payee_get_tree_store ( void );
GtkWidget *gsb_payee_get_tree_view ( void );
gboolean gsb_payee_update_combofix ( void );
GtkWidget *onglet_tiers ( void );
void payee_fill_tree ( void );
gboolean payee_hold_position_set_expand ( gboolean expand );
gboolean payee_hold_position_set_path ( GtkTreePath *path );
void appui_sur_ajout_payee ( GtkTreeModel * model );
void appui_sur_manage_tiers ( void );
gboolean edit_payee ( GtkTreeView * view );
gboolean popup_payee_view_mode_menu ( GtkWidget * button );
gboolean payee_remove_unused ( GtkWidget *button,
                        gpointer null );
/* END_DECLARATION */
#endif
