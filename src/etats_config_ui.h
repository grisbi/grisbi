#ifndef GSB_ETATS_CONFIG_UI_H
#define GSB_ETATS_CONFIG_UI_H (1)

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
void etats_config_ui_free_builder ( void );
GtkWidget *etats_config_ui_create_dialog ( void );
void etats_config_ui_onglet_periode_date_interval_sensitive ( gboolean show );
void etats_config_ui_onglet_periode_select_exer_from_list ( GSList *liste );
GSList *etats_config_ui_tree_view_get_list_item_selected ( const gchar *treeview_name );
gint etats_config_ui_tree_view_get_single_item_selected ( const gchar *treeview_name );
void etats_config_ui_tree_view_select_single_item ( const gchar *treeview_name,
                        gint numero );
gboolean etats_config_ui_widget_get_actif ( const gchar *widget_name );
GtkWidget *etats_config_ui_widget_get_widget_by_name ( const gchar *parent_name,
                        const gchar *child_name );
gboolean etats_config_ui_widget_set_actif ( const gchar *widget_name,
                        gboolean actif );
/* END_DECLARATION */


#endif /* GSB_ETATS_CONFIG_UI_H */
