#ifndef _CATEGORIES_ONGLET_H
#define _CATEGORIES_ONGLET_H (1)


/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
GtkTreePath *category_hold_position_get_path ( void );
gboolean category_hold_position_set_expand ( gboolean expand );
gboolean category_hold_position_set_path ( GtkTreePath *path );
GtkWidget *onglet_categories ( void );
void remplit_arbre_categ ( void );
/* END_DECLARATION */
#endif
