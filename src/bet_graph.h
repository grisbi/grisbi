#ifndef _BET_GRAPH_H
#define _BET_GRAPH_H 1

#ifdef HAVE_GOFFICE

#include <gtk/gtk.h>

/* Definition de la taille mini de la boite de dialogue */
#define PAGE_WIDTH 750
#define PAGE_HEIGHT 550

/* Definition du maximum de segments par camembert */
#define MAX_SEGMENT_CAMEMBERT 30
/* Definition du maximum de segments par camembert */
#define MAX_POINTS_HISTOGRAM 300

/* Definition de la taille maximum d'un libelle */
#define TAILLE_MAX_LIBELLE 50


/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
void bet_graph_line_graph_new ( GtkWidget *menu_item, GtkTreeView *tree_view );
void bet_graph_sectors_graph_new ( GtkWidget *menu_item, GtkTreeView *tree_view );
GtkWidget *bet_graph_sectors_graph_new_new ( void );
/* END_DECLARATION */

#endif /* HAVE_GOFFICE */

#endif /*_BET_GRAPH */
