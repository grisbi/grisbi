#ifndef GSB_ETATS_CONFIG_H
#define GSB_ETATS_CONFIG_H (1)

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
void gsb_etats_config_personnalisation_etat ( void );

void gsb_etats_config_onglet_affichage_devises_make_combobox ( void );
GtkTreeModel *gsb_etats_config_onglet_categ_budget_get_model ( gboolean is_categ );
gboolean gsb_etats_config_onglet_categ_budget_select_partie_liste ( GtkWidget *button,
                        GdkEventButton *event,
                        GtkWidget *tree_view );
gboolean gsb_etats_config_onglet_categ_budget_toggled ( GtkCellRendererToggle *radio_renderer,
                        gchar *path_str,
                        GtkTreeStore *store );
void gsb_etats_config_onglet_categ_budget_tree_model_check_uncheck_all ( GtkTreeModel *model,
                        gboolean toggle_value );
gchar *gsb_etats_config_onglet_data_grouping_get_string ( gint type_data,
                        gint pos );
void gsb_etats_config_onglet_data_grouping_move_in_list ( gint src_pos,
                        gint src_type_data,
                        gint dst_pos );
GtkTreeModel *gsb_etats_config_onglet_periode_get_model_exercices ( void );
void gsb_etats_config_onglet_periode_make_calendar_entry ( void );
void gsb_etats_config_onglet_data_separation_combo_changed ( GtkComboBox *combo,
                        GtkWidget *widget );

GtkTreeModel *gsb_etats_config_onglet_get_liste_comptes ( void );
GtkTreeModel *gsb_etats_config_onglet_get_liste_tiers ( void );
GtkTreeModel *gsb_etats_config_onglet_mode_paiement_get_model ( void );
void gsb_etats_config_onglet_select_partie_liste_comptes ( GtkWidget *tree_view,
                        gint type_compte );
/* END_DECLARATION */


#endif /* GSB_ETATS_CONFIG_H */
