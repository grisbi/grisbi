#ifndef _CATEGORIES_ONGLET_H
#define _CATEGORIES_ONGLET_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
GtkWidget *categories_create_list ( void );
void categories_exporter_list ( void );
void categories_fill_list ( void );
void categories_importer_list ( void );
void categories_init_variables_list ( void );
GtkTreeStore *categories_get_tree_store ( void );
GtkWidget *categories_get_tree_view ( void );

void categories_new_category ( void );
void categories_delete_category ( void );
void categories_edit_category ( void );

GtkTreePath *categories_hold_position_get_path ( void );
gboolean categories_hold_position_set_expand ( gboolean expand );
gboolean categories_hold_position_set_path ( GtkTreePath *path );
void gsb_gui_categories_toolbar_set_style ( gint toolbar_style );
/* END_DECLARATION */
#endif
