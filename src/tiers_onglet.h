#ifndef _TIERS_ONGLET_H
#define _TIERS_ONGLET_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
GtkWidget *payees_create_list ( void );
void payees_fill_list ( void );
void payees_update_toolbar_list ( void );
GtkWidget *payees_get_tree_view ( void );
GtkTreeStore *payees_get_tree_store ( void );

void payees_new_payee ( void );
void payees_delete_payee ( void );
void payees_edit_payee ( void );

void payees_manage_payees ( void );
void payees_remove_unused_payees ( void );

gboolean payees_hold_position_set_expand ( gboolean expand );
gboolean payees_hold_position_set_path ( GtkTreePath *path );
/* END_DECLARATION */
#endif
