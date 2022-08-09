#ifndef _BET_GRAPH_H
#define _BET_GRAPH_H 1

#include <gtk/gtk.h>
#include <goffice/goffice.h>

/* START_INCLUDE_H */
#include "utils_buttons.h"
/* END_INCLUDE_H */

/* Definition de la taille mini de la boite de dialogue */
#define PAGE_WIDTH 800
#define PAGE_HEIGHT 650

/* Definition du maximum de segments par camembert */
#define MAX_SEGMENT_CAMEMBERT 30
/* Definition du maximum de segments par camembert */
#define MAX_POINTS_GRAPHIQUE 370

/* Definition de la taille maximum d'un libelle */
#define TAILLE_MAX_LIBELLE 50

typedef struct _BetGraphDataStruct		BetGraphDataStruct;
typedef struct _BetGraphButtonStruct	BetGraphButtonStruct;
typedef struct _BetGraphPrefsStruct		BetGraphPrefsStruct;

struct _BetGraphDataStruct
{
    /* données générales */
    GtkTreeView *			tree_view;			/* tree_view contenant les données à traiter */
    GtkNotebook *			notebook;			/* notebook pour l'affichage du graphique */
    GtkWidget *				button_show_grid;	/* bouton pour cacher/montrer la grille du graphique  */
    gint					account_number;		/* compte concerné */
    gint					currency_number;	/* devise du compte */

    /* Données pour le graphique */
    GtkWidget *				widget;
    GogChart *				chart;
    GogPlot *				plot;
    gchar *					title;
    gchar *					service_id;			/* définit le type de graphique : GogPiePlot, GogLinePlot, GogBarColPlot = défaut */
    gboolean 				is_legend;
    gboolean				valid_data;			/* empêche le recalcul des données pendant la durée de vie du graph */
    gboolean				show_grid;			/* FALSE par défaut */

    /* données communes aux axes*/
    gint					nbre_elemnts;

    /* données pour l'axe X */
    gdouble 				tab_X[MAX_POINTS_GRAPHIQUE];							/* données de type gdouble */
    gchar 					tab_libelle[MAX_POINTS_GRAPHIQUE][TAILLE_MAX_LIBELLE];	/* données de type string */
    gchar **				tab_vue_libelle;										/* tableau associé à celui ci-dessus */

    /* données pour l'axe Y */
    gdouble					tab_Y[MAX_POINTS_GRAPHIQUE];	/* série 1 données de type gdouble */
    gdouble					tab_Y2[MAX_POINTS_GRAPHIQUE];	/* série 2 données de type gdouble */
    gboolean				double_axe;						/* TRUE if two axes */
    gchar *					title_Y;						/* titre de la série 1 */
    gchar *					title_Y2;						/* titre de la série 2 */

    /* données pour les camemberts */
    gint 					type_infos;			/* 0 type crédit ou < 0, 1 type débit ou >= 0, -1 tous types */
    gdouble 				montant;			/* montant annuel toutes catégories. sert au calcul de pourcentage */

    /* préférences pour le graphique */
    BetGraphPrefsStruct *	prefs;
};

struct _BetGraphButtonStruct
{
    gchar *					name;
    gchar *					filename;
    gchar *					service_id;			/* définit le type de graphique */
    GCallback				callback;			/* fonction de callback */
    gboolean 				is_visible;			/* TRUE si le bouton est visible dans la barre d'outils */
    gint 					origin_tab;			/* BET_ONGLETS_PREV ou BET_ONGLETS_HIST */
    GtkToolItem *			button;
    GtkWidget *				tree_view;
    BetGraphPrefsStruct *	prefs;				/* préférences pour le graphique */
};

struct _BetGraphPrefsStruct
{
    gint					type_graph;				/* type de graphique : -1 secteurs, 1 ligne, 0 barres par défaut */
    gboolean				major_tick_out;			/* TRUE par défaut */
    gboolean				major_tick_in;			/* FALSE par défaut */
    gboolean				major_tick_labeled;		/* affichage des libellés de l'axe X. TRUE par défaut*/
    gint					position;				/* position de l'axe des X. En bas par défaut*/
    gboolean				new_axis_line;			/* ligne supplémentaire. Croise l'axe des Y à 0. TRUE par défaut*/
    gint					cross_entry;			/* position du croisement avec l'axe Y. 0 par défaut */
    gdouble					degrees;				/* rotation des étiquettes de l'axe X en degrés. 90° par défaut */
    gint					gap_spinner;			/* espace entre deux barres en %. 50 par défaut*/
    gboolean				before_grid;			/* les étiquettes sont cachées par les barres par défaut */
    gboolean				major_grid_y;			/* ajoute une grille principale sur l'axe Y */
    gboolean				minor_grid_y;			/* ajoute une grille secondaire sur l'axe Y */
};

/* START_DECLARATION */
BetGraphDataStruct *	struct_initialise_bet_graph_data		(void);
GtkToolItem *	bet_graph_button_menu_new 						(GtkWidget *toolbar,
																 const gchar *type_graph,
																 GCallback callback,
																 GtkWidget *tree_view);
void 			bet_graph_free_configuration_variables			(void);
gchar *			bet_graph_get_configuration_string				(gint origin_tab);
void 			bet_graph_line_graph_new						(GtkWidget *button,
																 GtkTreeView *tree_view);
void 			bet_graph_montly_graph_new						(GtkWidget *button,
																 GtkTreeView *tree_view);
gboolean 		bet_graph_populate_sectors_by_hist_data			(BetGraphDataStruct *self);
gboolean 		bet_graph_populate_sectors_by_sub_divisions		(BetGraphDataStruct *self,
																 gint div_number);
void		 	bet_graph_sectors_graph_new						(GtkWidget *button,
																 GtkTreeView *tree_view);
void 			bet_graph_set_configuration_variables			(const gchar *string);
void 			bet_graph_show_grid_button_configure			(BetGraphDataStruct *self,
																 gint active,
																 gint hide);
void 			bet_graph_update_graph							(BetGraphDataStruct *self);
void 			struct_free_bet_graph_prefs						(void);
void			struct_free_bet_graph_data						(BetGraphDataStruct *self);

/* END_DECLARATION */

#endif /*_BET_GRAPH */
