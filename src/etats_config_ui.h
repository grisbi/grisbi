#ifndef GSB_ETATS_CONFIG_UI_H
#define GSB_ETATS_CONFIG_UI_H (1)

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
void etats_config_ui_free_all_var ( void );
GtkWidget *etats_config_ui_create_dialog ( void );

void etats_config_ui_onglet_categ_budget_check_uncheck_all ( GtkToggleButton *togglebutton,
                        GtkWidget *tree_view );
void etats_config_ui_onglet_comptes_select_unselect ( GtkToggleButton *togglebutton,
                        GtkWidget *tree_view );
GSList *etats_config_ui_onglet_mode_paiement_get_list_rows_selected ( const gchar *treeview_name );
void etats_config_ui_onglet_mode_paiement_select_rows_from_list ( GSList *liste,
                        const gchar *treeview_name );
void etats_config_ui_onglet_periode_date_interval_sensitive ( gboolean show );

gint etats_config_ui_buttons_radio_get_active_index ( const gchar *button_name );
void etats_config_ui_buttons_radio_set_active_index ( const gchar *button_name,
                        gint index );

GSList *etats_config_ui_tree_view_get_list_rows_selected ( const gchar *treeview_name );
gint etats_config_ui_tree_view_get_single_row_selected ( const gchar *treeview_name );
void etats_config_ui_tree_view_select_rows_from_list ( GSList *liste,
                        const gchar *treeview_name,
                        gint column );
void etats_config_ui_tree_view_select_single_row ( const gchar *treeview_name,
                        gint numero );

gboolean etats_config_ui_widget_get_actif ( const gchar *widget_name );
GtkWidget *etats_config_ui_widget_get_widget_by_name ( const gchar *parent_name,
                        const gchar *child_name );
gboolean etats_config_ui_widget_set_actif ( const gchar *widget_name,
                        gboolean actif );
/* END_DECLARATION */


#endif /* GSB_ETATS_CONFIG_UI_H */
