#ifndef _COMPTES_TRAITEMENTS_H
#define _COMPTES_TRAITEMENTS_H (1)
/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
GtkWidget * creation_option_menu_comptes ( GtkSignalFunc func, 
					   gboolean activate_currrent,
					   gboolean include_closed );
gboolean delete_account ( void );
gint demande_type_nouveau_compte ( void );
gboolean gsb_account_create_name_tree_model ( GtkWidget *combo_box,
					      GCallback func, 
					      gboolean include_closed );
gint gsb_account_get_number_tree_model ( GtkWidget *combo_box );
gboolean gsb_account_update_name_tree_model ( GtkWidget *combo_box,
					      gboolean include_closed );
gboolean new_account ( void );
/* END_DECLARATION */
#endif
