#ifndef GSB_ETATS_CONFIG_H
#define GSB_ETATS_CONFIG_H (1)

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
void gsb_etats_config_personnalisation_etat ( void );
GtkTreeModel *gsb_etats_config_onglet_get_liste_comptes ( void );
GtkTreeModel *gsb_etats_config_onglet_get_liste_exercices ( void );
GtkTreeModel *gsb_etats_config_onglet_get_liste_tiers ( void );
void gsb_etats_config_onglet_select_partie_liste_comptes ( GtkWidget *tree_view,
                        gint type_compte );


void gsb_etats_config_tree_model_check_uncheck_all ( GtkTreeModel *model,
                        gboolean toggle_value );

/* END_DECLARATION */


#endif /* GSB_ETATS_CONFIG_H */
