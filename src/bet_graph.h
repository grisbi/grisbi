#ifndef _BET_GRAPH_H
#define _BET_GRAPH_H 1

#include <gtk/gtk.h>

/* START_INCLUDE_H */
#include "utils_buttons.h"
/* END_INCLUDE_H */

/* Definition de la taille mini de la boite de dialogue */
#define PAGE_WIDTH 750
#define PAGE_HEIGHT 550

/* Definition du maximum de segments par camembert */
#define MAX_SEGMENT_CAMEMBERT 30
/* Definition du maximum de segments par camembert */
#define MAX_POINTS_GRAPHIQUE 370

/* Definition de la taille maximum d'un libelle */
#define TAILLE_MAX_LIBELLE 50


/* START_DECLARATION */
GtkToolItem *	bet_graph_button_menu_new 				(GtkWidget *toolbar,
														 const gchar *type_graph,
														 GCallback callback,
														 GtkWidget *tree_view);
gchar *			bet_graph_get_configuration_string 		(gint origin_tab);
void 			bet_graph_line_graph_new 				(GtkWidget *button,
														 GtkTreeView *tree_view);
void 			bet_graph_montly_graph_new 				(GtkWidget *button,
														 GtkTreeView *tree_view);
void 			bet_graph_sectors_graph_new 			(GtkWidget *button,
														 GtkTreeView *tree_view);
void 			bet_graph_set_configuration_variables 	(const gchar *string);
void 			struct_free_bet_graph_prefs 			(void);
/* END_DECLARATION */


#endif /*_BET_GRAPH */
