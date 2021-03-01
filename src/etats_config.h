#ifndef ETATS_CONFIG_H
#define ETATS_CONFIG_H

#include <gtk/gtk.h>

#define TEXT_NORMAL (0<<0)
#define TEXT_BOLD   (1<<0)
#define TEXT_ITALIC (1<<1)
#define TEXT_HUGE   (1<<2)
#define TEXT_LARGE  (1<<3)
#define TEXT_SMALL  (1<<4)


/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
void 			etats_config_onglet_affichage_devises_make_combobox 			(void);
GtkTreeModel *	etats_config_onglet_categ_budget_get_model 						(gboolean is_categ);
gboolean 		etats_config_onglet_categ_budget_select_partie_liste 			(GtkWidget *button,
																				 GdkEventButton *event,
																				 GtkWidget *tree_view);
gboolean 		etats_config_onglet_categ_budget_toggled 						(GtkCellRendererToggle *radio_renderer,
																				 gchar *path_str,
																				 GtkTreeStore *store);
void 			etats_config_onglet_categ_budget_tree_model_check_uncheck_all	(GtkTreeModel *model,
																				 gboolean toggle_value);
gchar *			etats_config_onglet_data_grouping_get_string 					(gint type_data,
																				 gint pos);
void 			etats_config_onglet_data_grouping_move_in_list 					(gint src_pos,
																				 gint src_type_data,
																				 gint dst_pos);
void 			etats_config_onglet_data_separation_combo_changed 				(GtkComboBox *combo,
																				 GtkWidget *widget);
GtkTreeModel *	etats_config_onglet_get_liste_tiers 							(void);

GtkTreeModel *	etats_config_onglet_mode_paiement_get_model 					(void);
gint			etats_config_personnalisation_etat								(void);
/* END_DECLARATION */


#endif /* ETATS_CONFIG_H */
