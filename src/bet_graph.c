/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C) 2011-2021 Pierre Biava (grisbi@pierre.biava.name)        */
/*                   2011 Guillaume Verger (guillaume.verger@laposte.net)     */
/*          https://www.grisbi.org/                                           */
/*                                                                            */
/*  This program is free software; you can redistribute it and/or modify      */
/*  it under the terms of the GNU General Public License as published by      */
/*  the Free Software Foundation; either version 2 of the License, or         */
/*  (at your option) any later version.                                       */
/*                                                                            */
/*  This program is distributed in the hope that it will be useful,           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*  GNU General Public License for more details.                              */
/*                                                                            */
/*  You should have received a copy of the GNU General Public License         */
/*  along with this program; if not, write to the Free Software               */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                            */
/* ************************************************************************** */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gdk/gdk.h>
#include <glib/gi18n.h>
#include <strings.h>

#ifdef HAVE_GOFFICE
#include <goffice/goffice.h>

/*START_INCLUDE*/
#include "bet_graph.h"
#include "bet_data.h"
#include "bet_hist.h"
#include "bet_tab.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "gsb_automem.h"
#include "gsb_data_account.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_fyear.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "navigation.h"
#include "structures.h"
#include "utils.h"
#include "utils_gtkbuilder.h"
#include "utils_dates.h"
#include "utils_real.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

typedef struct _BetGraphDataStruct   BetGraphDataStruct;
typedef struct _BetGraphButtonStruct BetGraphButtonStruct;
typedef struct _BetGraphPrefsStruct  BetGraphPrefsStruct;

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
    gchar 					tab_libelle[MAX_POINTS_GRAPHIQUE][TAILLE_MAX_LIBELLE+1];/* données de type string */
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

/*START_STATIC*/
static GogPlot *bet_graph_create_graph_page  (BetGraphDataStruct *self,
											  gboolean add_page);

static gboolean bet_graph_right_button_press (GtkWidget *widget,
											  GdkEventButton  *event,
											  BetGraphDataStruct *self);

/* GtkBuilder construction de l'interface graphique */
static GtkBuilder *				bet_graph_builder = NULL;

/* variables statiques pour les différents types de graphiques */
static BetGraphPrefsStruct *	prefs_prev = NULL;		/* for forecast graph */
static BetGraphPrefsStruct *	prefs_hist = NULL;		/* for monthly graph */

/* mois sous la forme abrégée */
static const gchar *			short_str_months[] = {
    N_("Jan"), N_("Feb"), N_("Mar"), N_("Apr"),
    N_("May"), N_("Jun"), N_("Jul"), N_("Aug"),
    N_("Sep"), N_("Oct"), N_("Nov"), N_("Dec")
};

/* mois sous la forme longue */
static const gchar *			long_str_months[] = {
    N_("January"), N_("February"), N_("March"), N_("April"),
    N_("May"), N_("June"), N_("July"), N_("August"),
    N_("September"), N_("October"), N_("November"), N_("December")
};


/*END_STATIC*/

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 *
 *
 * \param
 *
 * \return TRUE
 **/
static BetGraphButtonStruct *struct_initialise_bet_graph_button (void)
{
    BetGraphButtonStruct *self;

    self = g_new0 (BetGraphButtonStruct, 1);

   return self;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void struct_free_bet_graph_button (BetGraphButtonStruct *self)
{
    g_free (self->name);
    g_free (self->filename);
    g_free (self->service_id);

	g_free (self);
}

/**
 *
 *
 * \param
 *
 * \return TRUE
 **/
static BetGraphDataStruct *struct_initialise_bet_graph_data (void)
{
    BetGraphDataStruct *self;
    gint i;

    self = g_new0 (BetGraphDataStruct, 1);

    self->service_id = NULL;
    self->title = NULL;

    self->tab_vue_libelle = g_malloc (MAX_POINTS_GRAPHIQUE * sizeof (gchar*));

    for (i = 0; i < MAX_POINTS_GRAPHIQUE; i++)
    {
        self->tab_vue_libelle[i] = self->tab_libelle[i];
    }

    self->title_Y = NULL;
    self->title_Y2 = NULL;

   return self;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void struct_free_bet_graph_data (BetGraphDataStruct *self)
{
    g_free (self->title);
    g_free (self->service_id);
    g_free (self->tab_vue_libelle);
    g_free (self->title_Y);
    g_free (self->title_Y2);

    g_free (self);
}

/**
 *
 *
 * \param
 *
 * \return TRUE
 **/
static BetGraphPrefsStruct *struct_initialise_bet_graph_prefs (void)
{
    BetGraphPrefsStruct *self;

    self = g_new0 (BetGraphPrefsStruct, 1);

    self->major_tick_out = TRUE;
    self->major_tick_labeled = TRUE;
    self->position = 2;
    self->new_axis_line = FALSE;
    self->degrees = 90;
    self->gap_spinner = 50;
    self->before_grid = TRUE;

   return self;
}

/**
 * Crée un builder et récupère les widgets du fichier bet_graph.ui
 *
 * \param   none
 *
 * \return TRUE if OK FALSE otherwise
 **/
static gboolean bet_graph_initialise_builder (void)
{
    /* Creation d'un nouveau GtkBuilder */
    bet_graph_builder = gtk_builder_new ();
    if (bet_graph_builder == NULL)
        return FALSE;

    /* récupère les widgets */
    if (utils_gtkbuilder_merge_ui_data_in_builder (bet_graph_builder, "bet_graph.ui"))
        return TRUE;
    else
        return FALSE;

}

/**
 * remplit les structures avec les données des sous divisions.
 *
 * \param BetGraphDataStruct     structure data du parent
 * \param gint                      numéro de division
 *
 * \return FALSE
 **/
static gboolean bet_graph_populate_sectors_by_sub_divisions (BetGraphDataStruct *self,
															 gint div_number)
{
    GtkTreeModel *model = NULL;
    GtkTreeIter iter;

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (self->tree_view));
    if (model == NULL)
        return FALSE;

    if (gtk_tree_model_get_iter_first (model, &iter))
    {
        gint account_number;
        gchar *libelle_division = self->tab_libelle[0];
        gdouble *tab_montant_division = self->tab_Y;

        /* test du numero de compte */
        gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
                        SPP_HISTORICAL_ACCOUNT_NUMBER, &account_number,
                        -1);
        if (account_number != self->account_number)
		{
			if (gsb_data_account_get_bet_hist_use_data_in_account (account_number))
			{
				gint main_account_number;

				main_account_number = bet_data_transfert_get_main_account_from_card (account_number);
				if (main_account_number != self->account_number)
					return FALSE;
			}
			else
				return FALSE;
		}

        do
        {
            gint div;

            gtk_tree_model_get (model, &iter, SPP_HISTORICAL_DIV_NUMBER, &div, -1);

            if (div == div_number)
            {
                GtkTreeIter child_iter;
                gchar *desc;
                gchar *amount;
                gint nbre_elemnts;
                gint i;

                nbre_elemnts = gtk_tree_model_iter_n_children (model, &iter);
                if (nbre_elemnts > MAX_SEGMENT_CAMEMBERT)
                    nbre_elemnts = MAX_SEGMENT_CAMEMBERT;

                for (i = 0; i < nbre_elemnts; i++)
                {
                    if (gtk_tree_model_iter_nth_child (model, &child_iter, &iter, i))
                    {
                        gtk_tree_model_get (model,
                                    &child_iter,
                                    SPP_HISTORICAL_DESC_COLUMN, &desc,
                                    SPP_HISTORICAL_BALANCE_AMOUNT, &amount,
                                    SPP_HISTORICAL_SUB_DIV_NUMBER, &div,
                                    -1);
                        strncpy (&libelle_division[self->nbre_elemnts * TAILLE_MAX_LIBELLE],
                                    desc, TAILLE_MAX_LIBELLE);
                        tab_montant_division[self->nbre_elemnts] = utils_str_strtod (
                                    (amount == NULL) ? "0" : amount, NULL);

                        if (tab_montant_division[self->nbre_elemnts] < 0)
                            self->montant += -tab_montant_division[self->nbre_elemnts];
                        else
                            self->montant += tab_montant_division[self->nbre_elemnts];

                        self->nbre_elemnts++;
                        g_free (desc);
                        g_free (amount);
                    }
                }

                break;
            }
        }
        while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));

        if (self->nbre_elemnts)
            return TRUE;
    }

    /* return */
    return FALSE;
}

/**
 * affiche un graphique de type camembert joint
 *
 * \param BetGraphDataStruct     structure qui contient les données à afficher
 *
 * \return TRUE if OK FALSE otherwise
 **/
static gboolean bet_graph_affiche_camemberts (BetGraphDataStruct *self)
{
    GogPlot *cur_plot;
    GogSeries *series;
    GOData *data;
    GError * error = NULL;

    cur_plot = self->plot;

    gog_plot_clear_series (GOG_PLOT (cur_plot));
    series = GOG_SERIES (gog_plot_new_series (GOG_PLOT (cur_plot)));

    data = go_data_vector_str_new ((const char * const*) self->tab_vue_libelle, self->nbre_elemnts, NULL);
    gog_series_set_dim (series, 0, data, &error);
    if (error != NULL)
    {
        g_error_free (error);
        error = NULL;
        return FALSE;
    }

    data = go_data_vector_val_new (self->tab_Y, self->nbre_elemnts, NULL);
    gog_series_set_dim (series, 1, data, &error);
    if (error != NULL)
    {
        g_error_free (error);
        error = NULL;
        return FALSE;
    }

    return TRUE;
}

/**
 * affiche les sous divisions d'un camembert
 *
 * \param parent        BetGraphDataStruct du parent
 * \param div_number    numéro de la division concernée
 * \param bet_hist_data type de données : catégorie ou IB
 * \param div_name      nom de la catégorie ou de l'IB
 *
 * \return
 **/
static void bet_graph_affiche_sub_divisions (BetGraphDataStruct *parent,
											 gint div_number,
											 gint bet_hist_data,
											 gchar *div_name,
											 gchar *total_div)
{
    GtkWidget *dialog;
    GtkWidget *label;
    GtkWidget *notebook;
    GtkWidget *box_pie;
    gchar *title;
    BetGraphDataStruct *self;

    /* initialisation de la structure des données */
    self = struct_initialise_bet_graph_data ();
    self->tree_view = parent->tree_view;
    self->account_number = parent->account_number;
    self->currency_number = parent->currency_number;
    self->service_id = g_strdup (parent->service_id);
    self->type_infos = parent->type_infos;
    self->title = g_strconcat (_("Total amount: "), total_div, NULL);
    self->is_legend = TRUE;

    dialog = GTK_WIDGET (gtk_builder_get_object (bet_graph_builder, "bet_graph_sub_div_dialog"));

    if (gtk_window_get_transient_for (GTK_WINDOW (dialog)) == NULL)
    {
        GtkWidget *parent_widget;

        parent_widget = g_object_get_data (G_OBJECT (parent->notebook), "dialog");
        gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (parent_widget));
    }
    gtk_widget_set_size_request (dialog, 600, 400);
    g_signal_connect (G_OBJECT (dialog),
                        "destroy",
                        G_CALLBACK (gtk_widget_destroy),
                        NULL);

    /* set the title */
    if (bet_hist_data)
        title = g_markup_printf_escaped (_("<span weight=\"bold\">"
                        "Display subdivisions items of budgetary: %s</span>"),
                        div_name);

    else
        title = g_markup_printf_escaped (_("<span weight=\"bold\" size=\"large\">"
                        "Display subdivisions of category: %s</span>"),
                        div_name);


    label = GTK_WIDGET (gtk_builder_get_object (bet_graph_builder, "label_sub_div"));
    gtk_label_set_markup (GTK_LABEL (label), title);
    g_free (title);

    /* initialise les pages pour les graphiques camembert */
    notebook = GTK_WIDGET (gtk_builder_get_object (bet_graph_builder, "notebook_sub_div"));
    gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook), FALSE);

    box_pie = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), box_pie, gtk_label_new (_("Graph")));

    /* Set the graph */
    self->notebook = GTK_NOTEBOOK (notebook);
    self->plot = bet_graph_create_graph_page (self, FALSE);

    /* on interdit le clic droit */
    g_signal_handlers_block_by_func (self->widget,
                        G_CALLBACK (bet_graph_right_button_press),
                        self);

    /* populate the data */
    self->valid_data = bet_graph_populate_sectors_by_sub_divisions (self, div_number);

    if (self->valid_data)
        bet_graph_affiche_camemberts (self);

    /* show or hide widgets */
    gtk_widget_show_all (dialog);

    gtk_dialog_run (GTK_DIALOG (dialog));

    /* free the data */
    struct_free_bet_graph_data (self);
    gtk_notebook_remove_page (GTK_NOTEBOOK (notebook), 0);
    gtk_widget_hide (dialog);
}

/**
 * fonction appelée quand on fait un click droit sur un graphique
 *
 * \param
 * \param
 * \param
 *
 * \return TRUE
 **/
static gboolean bet_graph_right_button_press (GtkWidget *widget,
											  GdkEventButton  *event,
											  BetGraphDataStruct *self)
{

    if (event->type == GDK_BUTTON_PRESS
     &&
     event->button == 3
     &&
     strcmp (self->service_id, "GogPiePlot") == 0)
    {
        GogRenderer *rend = NULL;
        GogView *graph_view = NULL;
        GogView *view = NULL;
        GogSeries *series;
        gchar *total_div;
        gint index;
        gint nbre_elemnts = 0;
        gint div_number = 0;
        gint bet_hist_data;

        rend = go_graph_widget_get_renderer (GO_GRAPH_WIDGET (self->widget));
        g_object_get (G_OBJECT (rend), "view", &graph_view, NULL);
        view = gog_view_find_child_view (graph_view, GOG_OBJECT (self->plot));

        index = gog_plot_view_get_data_at_point (GOG_PLOT_VIEW (view), event->x, event->y, &series);

        if (index == -1)
            return TRUE;

        total_div = g_strdup_printf ("%s", utils_real_get_string_with_currency_from_double (
                        self->tab_Y[index], self->currency_number));

        bet_hist_data = gsb_data_account_get_bet_hist_data (self->account_number);
        if (bet_hist_data)
        {
            div_number = gsb_data_budget_get_number_by_name (self->tab_vue_libelle[index], FALSE, 0);
            nbre_elemnts = gsb_data_budget_get_sub_budget_list_length (div_number);
        }
        else
        {
            div_number = gsb_data_category_get_number_by_name (self->tab_vue_libelle[index], FALSE, 0);
            nbre_elemnts = gsb_data_category_get_sub_category_list_length (div_number);
        }

        if (nbre_elemnts)
            bet_graph_affiche_sub_divisions (self, div_number, bet_hist_data,
                        self->tab_vue_libelle[index], total_div);
        else
        {
            GtkWidget *dialog;
            GtkWidget *parent_widget;
            gchar *msg;

            if (bet_hist_data)
                msg = g_strdup (_("The selected budget item has no subitems"));
            else
                msg = g_strdup (_("The selected category has no subcategory"));

            parent_widget = g_object_get_data (G_OBJECT (self->notebook), "dialog");
            dialog = gtk_message_dialog_new (GTK_WINDOW (parent_widget),
                        GTK_DIALOG_DESTROY_WITH_PARENT,
                        GTK_MESSAGE_INFO,
                        GTK_BUTTONS_CLOSE,
                        "%s", msg);

            gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (parent_widget));
            gtk_dialog_run (GTK_DIALOG (dialog));
            gtk_widget_destroy (dialog);

            g_free (msg);
        }
    }

    /* return */
    return TRUE;
}

/**
 * callback qui gère l'affichage des tip
 *
 * \param
 * \param
 * \param
 *
 * \return TRUE to stop the signal emission
 **/
static gboolean bet_graph_on_motion (GtkWidget *event_box,
									 GdkEventMotion *event,
									 BetGraphDataStruct *self)
{
    GogRenderer *rend = NULL;
    GogView *graph_view = NULL;
    GogView *view = NULL;
    GogChartMap *map = NULL;
    GogSeries *series;
    gchar *buf = NULL;
    gint index;

    rend = go_graph_widget_get_renderer (GO_GRAPH_WIDGET (self->widget));
    g_object_get (G_OBJECT (rend), "view", &graph_view, NULL);
    view = gog_view_find_child_view (graph_view, GOG_OBJECT (self->plot));

    if (strcmp (self->service_id, "GogBarColPlot") == 0)
    {
        index = gog_plot_view_get_data_at_point (GOG_PLOT_VIEW (view), event->x, event->y, &series);
        if (index != -1)
        {
            double const *x;
            double const *y;

            gog_series_get_xy_data (series, &x, &y);
            buf = g_strdup_printf (_("date %s : value %s"), self->tab_vue_libelle[index],
                        utils_real_get_string_with_currency_from_double (
                        y[index], self->currency_number));
        }
    }
    else if (strcmp (self->service_id, "GogLinePlot")  == 0)
    {
        GogAxis *x_axis, *y_axis;

        /* correction manque fonction _get_data_at_point () pour les graphes lignes */
        if (self->double_axe)
        {
            gtk_widget_set_tooltip_text (GTK_WIDGET (self->widget), "");
            return TRUE;
        }

        x_axis = GOG_AXIS (gog_object_get_child_by_name (GOG_OBJECT (self->chart), "X-Axis"));
        y_axis = GOG_AXIS (gog_object_get_child_by_name (GOG_OBJECT (self->chart), "Y-Axis"));

        map = gog_chart_map_new (self->chart, &(view->allocation), x_axis, y_axis, NULL, FALSE);

        if (gog_chart_map_is_valid (map)
         &&
         event->x >= view->allocation.x && event->x < view->allocation.x + view->allocation.w
         &&
        event->y >= view->allocation.y && event->y < view->allocation.y + view->allocation.h)
        {
            GogAxisMap *x_map;

            x_map = gog_chart_map_get_axis_map (map, 0);
            index = gog_axis_map_from_view (x_map, event->x);

            buf = g_strdup_printf (_("date %s : value %s"), self->tab_vue_libelle[index-1],
                        utils_real_get_string_with_currency_from_double (
                        self->tab_Y[index-1], self->currency_number));
        }
        gog_chart_map_free (map);
    }
    else if (strcmp (self->service_id, "GogPiePlot") == 0)
    {
        index = gog_plot_view_get_data_at_point (GOG_PLOT_VIEW (view), event->x, event->y, &series);
        if (index != -1)
        {
            double const *x;
            double const *y;

            gog_series_get_xy_data (series, &x, &y);
            buf = g_strdup_printf (
                    "<markup><span font_weight=\"bold\">%s</span>\n%s (%.2f%%)</markup>",
                    self->tab_vue_libelle[index],
                    utils_real_get_string_with_currency_from_double (y[index], self->currency_number),
                    100*y[index]/self->montant
                   );
        }
    }

    gtk_widget_set_tooltip_markup (GTK_WIDGET (self->widget), buf);
    g_free (buf);

    return TRUE;
}

/**
 * Création de la page pour le graphique initialisée
 *
 * \param
 * \param   add_page
 *
 * \return GogPlot
 **/
static GogPlot *bet_graph_create_graph_page  (BetGraphDataStruct *self,
											  gboolean add_page)
{
    GtkWidget *child;
    GtkWidget *w;
    GogGraph *graph;
    GogLabel *label;
    GogPlot *plot;
    GOStyle *style;
    GOData *data;
    PangoFontDescription *desc;

    if (add_page)
    {
        /* Set the new page */
        child = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
        gtk_notebook_prepend_page (self->notebook, child, gtk_label_new (_("Graph")));
    }
    else
        child = gtk_notebook_get_nth_page (GTK_NOTEBOOK (self->notebook), 0);

    /* Set the graph widget */
    w = go_graph_widget_new (NULL);
    g_signal_connect (G_OBJECT (w),
                        "motion-notify-event",
                        G_CALLBACK (bet_graph_on_motion),
                        self);

    g_signal_connect (G_OBJECT (w),
                        "button-press-event",
                        G_CALLBACK (bet_graph_right_button_press),
                        self);
    gtk_box_pack_end (GTK_BOX (child), w, TRUE, TRUE, 0);

    self->widget = w;

    /* Get the embedded graph */
    graph = go_graph_widget_get_graph (GO_GRAPH_WIDGET (w));

    /* set the title */
    if (self->title && strlen (self->title))
    {
        label = (GogLabel *) g_object_new (GOG_TYPE_LABEL, NULL);
        data = go_data_scalar_str_new (self->title, FALSE);
        gog_dataset_set_dim (GOG_DATASET (label), 0, data, NULL);
        gog_object_add_by_name (GOG_OBJECT (graph), "Title", GOG_OBJECT (label));

        /* Change the title font */
        style = go_styled_object_get_style (GO_STYLED_OBJECT (label));
        desc = pango_font_description_from_string ("Sans bold 12");
        go_style_set_font_desc (style, desc);
    }

    /* Get the chart created by the widget initialization */
    self->chart = go_graph_widget_get_chart (GO_GRAPH_WIDGET (w));

    /* Create a pie plot and add it to the chart */
    plot = (GogPlot *) gog_plot_new_by_name (self->service_id);
    gog_object_add_by_name (GOG_OBJECT (self->chart), "Plot", GOG_OBJECT (plot));

    /* Add a legend to the chart */
    if (self->is_legend)
        gog_object_add_by_name (GOG_OBJECT (self->chart), "Legend", NULL);

    return plot;
}

/**
 *
 *
 * \param
 *
 * \return TRUE if OK FALSE otherwise
 **/
static gboolean bet_graph_affiche_XY_line (BetGraphDataStruct *self)
{
    GogPlot *cur_plot;
    GogSeries *series;
    GOData *data;
    GOData *name_src;
    GOStyle *style;
    GogObject *axis;
    GogObject *axis_line = NULL;
    GError *error = NULL;
    const gchar *position;
    BetGraphPrefsStruct *prefs;

    prefs = self->prefs;

    /* Afficher données dans le graphique */
    cur_plot = self->plot;

    /* on met en vertical les libellés de l'axe X */
    axis = gog_object_get_child_by_name (GOG_OBJECT (self->chart), "X-Axis");
    style = go_styled_object_get_style (GO_STYLED_OBJECT (axis));
    go_style_set_text_angle (style, prefs->degrees);

    /* on positionne l'axe des x sur le 0 par défaut */
    switch (prefs->position)
    {
    case 0:
        position = "low";
        break;
    case 1:
        position = "high";
        break;
    case 2:
        position = "cross";
        break;
    default :
        position = "cross";
    }
    g_object_set (G_OBJECT (axis),
                        "pos-str", position,
                        "cross-axis-id", gog_object_get_id (GOG_OBJECT (axis)),
                        NULL);

    g_object_set (G_OBJECT (axis),
                        "major-tick-in", prefs->major_tick_in,
                        "major-tick-out", prefs->major_tick_out,
                        "minor-tick-out", prefs->major_tick_out,
                        "major-tick-labeled", prefs->major_tick_labeled,
                        NULL);

    /* add a line which crosses the Y axis at 0 */
    if (prefs->new_axis_line)
    {
        axis_line = gog_object_add_by_name (axis, "AxisLine", NULL);
        g_object_set (G_OBJECT (axis_line),
                        "pos-str", "cross",
                        "cross-axis-id", gog_object_get_id (GOG_OBJECT (axis_line)),
                        "major-tick-out", FALSE,
                        "major-tick-labeled", FALSE,
                        NULL);
    }

    if (prefs->type_graph == 0)
    {
        /* affichage de la grille au dessus ou en dessous des barres */
        /* sert aussi à afficher les étiquettes au dessus des barres à cause d'un bug de goffice */
        if (prefs->before_grid)
            g_object_set (G_OBJECT (self->plot),
                        "before-grid", TRUE,
                        NULL);

        /* modification du ratio de largeur des barres. 0 pas d'espace entre deux barres */
        g_object_set (G_OBJECT (self->plot),
                        "gap-percentage", prefs->gap_spinner,
                        NULL);
    }

    /* on ajoute une grille pour l'axe Y si necessaire */
    if (self->show_grid)
    {
        if (prefs->major_grid_y || self->show_grid)
        {
            axis = gog_object_get_child_by_name (GOG_OBJECT (self->chart), "Y-Axis");

            gog_object_add_by_name (GOG_OBJECT (axis), "MajorGrid", NULL);
            if (prefs->minor_grid_y)
                gog_object_add_by_name (GOG_OBJECT (axis), "MinorGrid", NULL);
        }
    }

    /* on met à jour les données à afficher */
    gog_plot_clear_series (GOG_PLOT (cur_plot));
    series = GOG_SERIES (gog_plot_new_series (GOG_PLOT (cur_plot)));

    data = go_data_vector_str_new ((const char * const*) self->tab_vue_libelle, self->nbre_elemnts, NULL);
    gog_series_set_dim (series, 0, data, &error);
    if (error != NULL)
    {
        g_error_free (error);
        error = NULL;
        return FALSE;
    }

    data = go_data_vector_val_new (self->tab_Y, self->nbre_elemnts, NULL);
    gog_series_set_dim (series, 1, data, &error);
    if (error != NULL)
    {
        g_error_free (error);
        error = NULL;
        return FALSE;
    }

    if (self->double_axe)
    {
        GogSeries *series2;

        /* on fixe le nom de la première série */
        name_src = go_data_scalar_str_new (self->title_Y, FALSE);
        gog_series_set_name (series, GO_DATA_SCALAR (name_src), NULL);

        series2 = GOG_SERIES (gog_plot_new_series (GOG_PLOT (cur_plot)));
        name_src = go_data_scalar_str_new (self->title_Y2, FALSE);
        gog_series_set_name (series2, GO_DATA_SCALAR (name_src), NULL);

        data = go_data_vector_val_new (self->tab_Y2, self->nbre_elemnts, NULL);
        gog_series_set_dim (series2, 1, data, &error);
        if (error != NULL)
        {
            g_error_free (error);
            error = NULL;
            return FALSE;
        }
    }

    /* return value */
    return TRUE;
}

/**
 *
 *
 * \param
 *
 * \return TRUE
 **/
static void bet_graph_update_graph (BetGraphDataStruct *self)
{
    gtk_notebook_remove_page (self->notebook, 0);

    self->plot = bet_graph_create_graph_page (self, TRUE);
    gtk_widget_show_all (GTK_WIDGET (self->notebook));

    bet_graph_affiche_XY_line (self);
}

/**
 * callback
 *
 * \param
 * \param
 *
 * \return
 **/
static void bet_graph_gap_spinner_changed (GtkSpinButton *spinbutton,
										   BetGraphDataStruct *self)
{
    BetGraphPrefsStruct *prefs;

    prefs = self->prefs;
    prefs->gap_spinner = gtk_spin_button_get_value (spinbutton);

    /* on met à jour le graph */
    bet_graph_update_graph (self);

    gsb_file_set_modified (TRUE);
}

/**
 * callback
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
static void bet_graph_rotation_changed (GORotationSel *rotation,
										int angle,
										BetGraphDataStruct *self)
{
    BetGraphPrefsStruct *prefs;

    prefs = self->prefs;

    prefs->degrees = angle;

    /* on met à jour le graph */
    bet_graph_update_graph (self);

    gsb_file_set_modified (TRUE);
}

/**
 * callback
 *
 * \param
 * \param
 *
 * \return
 **/
static void bet_graph_show_grid_button_changed (GtkToggleButton *togglebutton,
												BetGraphDataStruct *self)
{
    gboolean active;

    active = gtk_toggle_button_get_active (togglebutton);

    self->show_grid = active;
    if (active)
        gtk_button_set_label (GTK_BUTTON (self->button_show_grid), _("Hide grid"));
    else
        gtk_button_set_label (GTK_BUTTON (self->button_show_grid), _("Show grid"));

    /* on met à jour le graph */
    bet_graph_update_graph (self);

    gtk_notebook_set_current_page (self->notebook, 0);
}

/**
 * on positionne les caractéristiques du bouton
 *
 *\param active : 1 = on veut montrer la grille 0 = on veut cacher la grille -1 = sans effet
 *\param hide : 1 = on cache le bouton 0 = on montre le bouton -1 = sans effet
 *
 *\return
 **/
static void bet_graph_show_grid_button_configure (BetGraphDataStruct *self,
												  gint active,
												  gint hide)
{
    if (active != -1)
    {
        /* on bloque le signal */
        g_signal_handlers_block_by_func (G_OBJECT (self->button_show_grid),
                        bet_graph_show_grid_button_changed,
                        self);

        self->show_grid = active;
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (self->button_show_grid), active);

        if (active) /* la grille est visible */
            gtk_button_set_label (GTK_BUTTON (self->button_show_grid), _("Hide grid"));
        else
            gtk_button_set_label (GTK_BUTTON (self->button_show_grid), _("Show grid"));

        /* on débloque le signal */
        g_signal_handlers_unblock_by_func (G_OBJECT (self->button_show_grid),
                        bet_graph_show_grid_button_changed,
                        self);
    }

    if (active != -1)
    {
        if (hide)
            gtk_widget_hide (GTK_WIDGET (self->button_show_grid));
        else
            gtk_widget_show (GTK_WIDGET (self->button_show_grid));
    }
}

/**
 * callback
 *
 * \param
 * \param
 *
 * \return
 **/
static void bet_graph_toggle_button_changed (GtkToggleButton *togglebutton,
											 BetGraphDataStruct *self)
{
    GtkWidget *button;
    gint rang;
    gboolean active;
    BetGraphPrefsStruct *prefs;

    prefs = self->prefs;

    active = gtk_toggle_button_get_active (togglebutton);

    rang = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (togglebutton), "rang"));

    switch (rang)
    {
    case 0:
        prefs->major_tick_out = active;
        break;
    case 1:
        prefs->major_tick_in = active;
        break;
    case 2:
        prefs->major_tick_labeled = active;
        break;
    case 3:
        prefs->position = 0;
        break;
    case 4:
        prefs->position = 1;
        break;
    case 5:
        prefs->position = 2;
        button = g_object_get_data (G_OBJECT (togglebutton), "other_axis");
        gtk_widget_set_sensitive (button, !active);
        break;
    case 6:
        prefs->new_axis_line = active;
        button = g_object_get_data (G_OBJECT (togglebutton), "other_axis");
        gtk_widget_set_sensitive (button, !active);
        break;
    case 7:
        prefs->before_grid = active;
        break;
    case 8:
        prefs->major_grid_y = active;

        /* on sensibilise ou désensibilise le bouton minor_grid_y */
        button = g_object_get_data (G_OBJECT (togglebutton), "grid_y");
        gtk_widget_set_sensitive (button, active);

        /* si le graphe est dans la bonne configuration on sort */
        if (prefs->major_grid_y == self->show_grid)
        {
            gsb_file_set_modified (TRUE);
            return;
        }
        /* on positionne le bouton self->button_show_grid */
        if (active)
            bet_graph_show_grid_button_configure (self, TRUE, TRUE);
        else
            bet_graph_show_grid_button_configure (self, FALSE, TRUE);
        break;
    case 9:
        prefs->minor_grid_y = active;

        /* on positionne le bouton self->button_show_grid */
        if (active)
            bet_graph_show_grid_button_configure (self, TRUE, TRUE);
        else
            bet_graph_show_grid_button_configure (self, FALSE, TRUE);
        break;
    }

    /* on met à jour le graph */
    bet_graph_update_graph (self);

    gsb_file_set_modified (TRUE);
}

/**
 * Création de la boite de gestion des axes du graphique
 *
 * \param
 *
 * \return boite de gestion des axes du graphique
 **/
static GtkWidget *bet_graph_create_line_preferences (BetGraphDataStruct *self)
{
    GtkWidget *box_prefs;
    GtkWidget *box_options_col;
    GtkWidget *rot_align;
    GtkWidget *widget;
    GtkWidget *button_1;
    GtkWidget *button_2;
    GtkWidget *button_3;
    GtkWidget *rotation;
    BetGraphPrefsStruct *prefs;

    prefs = self->prefs;

    box_prefs = GTK_WIDGET (gtk_builder_get_object (bet_graph_builder, "box_prefs_line"));
    box_options_col = GTK_WIDGET (gtk_builder_get_object (bet_graph_builder, "box_options_col"));
    rot_align = GTK_WIDGET (gtk_builder_get_object (bet_graph_builder, "rot_align"));

    /* définition du type de graphique */
    if (strcmp (self->service_id, "GogLinePlot") == 0)
        prefs->type_graph = 1;
    else if (strcmp (self->service_id, "GogPiePlot") == 0)
        prefs->type_graph = -1;
    else
        prefs->type_graph = 0;

    /* configuration de l'axe X */
    /* configure les options d'affichage de l'axe X */
    widget = GTK_WIDGET (gtk_builder_get_object (bet_graph_builder, "major_tick_out"));
    if (prefs->major_tick_out)
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), TRUE);

    g_object_set_data (G_OBJECT (widget), "rang", GINT_TO_POINTER (0));
    g_signal_connect (widget,
                        "toggled",
                        G_CALLBACK (bet_graph_toggle_button_changed),
                        self);

    widget = GTK_WIDGET (gtk_builder_get_object (bet_graph_builder, "major_tick_in"));
    if (prefs->major_tick_in)
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), TRUE);

    g_object_set_data (G_OBJECT (widget), "rang", GINT_TO_POINTER (1));
    g_signal_connect (widget,
                        "toggled",
                        G_CALLBACK (bet_graph_toggle_button_changed),
                        self);

    widget = GTK_WIDGET (gtk_builder_get_object (bet_graph_builder, "major_tick_labeled"));
    if (prefs->major_tick_labeled)
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), TRUE);

    g_object_set_data (G_OBJECT (widget), "rang", GINT_TO_POINTER (2));
    g_signal_connect (widget,
                        "toggled",
                        G_CALLBACK (bet_graph_toggle_button_changed),
                        self);

    /* Configuration de la position de l'axe des X */
    button_1 = GTK_WIDGET (gtk_builder_get_object (bet_graph_builder, "axis_low"));
    if (prefs->position == 0)
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button_1), TRUE);
    g_object_set_data (G_OBJECT (button_1), "rang", GINT_TO_POINTER (3));

    button_2 = GTK_WIDGET (gtk_builder_get_object (bet_graph_builder, "axis_high"));
    if (prefs->position == 1)
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button_2), TRUE);
    g_object_set_data (G_OBJECT (button_2), "rang", GINT_TO_POINTER (4));

    button_3 = GTK_WIDGET (gtk_builder_get_object (bet_graph_builder, "axis_cross"));
    g_object_set_data (G_OBJECT (button_3), "rang", GINT_TO_POINTER (5));

    widget = GTK_WIDGET (gtk_builder_get_object (bet_graph_builder, "new_axis_line"));
    g_object_set_data (G_OBJECT (widget), "rang", GINT_TO_POINTER (6));

    /* on lie les deux boutons pour les retrouver plus tard */
    g_object_set_data (G_OBJECT (widget), "other_axis", button_3);
    g_object_set_data (G_OBJECT (button_3), "other_axis", widget);

    if (prefs->new_axis_line)
    {
        gtk_widget_set_sensitive (button_3, FALSE);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), TRUE);
    }

    if (prefs->position == 2)
    {
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button_3), TRUE);
        gtk_widget_set_sensitive (widget, FALSE);
    }

    /* set the signal */
    g_signal_connect (button_1,
                        "toggled",
                        G_CALLBACK (bet_graph_toggle_button_changed),
                        self);

    g_signal_connect (button_2,
                        "toggled",
                        G_CALLBACK (bet_graph_toggle_button_changed),
                        self);

    g_signal_connect (button_3,
                        "toggled",
                        G_CALLBACK (bet_graph_toggle_button_changed),
                        self);

    g_signal_connect (widget,
                        "toggled",
                        G_CALLBACK (bet_graph_toggle_button_changed),
                        self);

    /* configure l'orientation des étiquettes de l'axe X */
    rotation = go_rotation_sel_new ();
    go_rotation_sel_set_rotation (GO_ROTATION_SEL (rotation), prefs->degrees);
    g_signal_connect (G_OBJECT (rotation),
                        "rotation-changed",
                        G_CALLBACK (bet_graph_rotation_changed),
                        self);

    gtk_container_add (GTK_CONTAINER (rot_align), rotation);

    if (prefs->type_graph == 0)
    {
        /* configure la visibilité de la grille et provisoirement des étiquettes de l'axe X (bug goffice) */
        widget = GTK_WIDGET (gtk_builder_get_object (bet_graph_builder, "before_grid"));
        g_object_set_data (G_OBJECT (widget), "rang", GINT_TO_POINTER (7));
        if (prefs->before_grid)
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), TRUE);
        g_signal_connect (widget,
                        "toggled",
                        G_CALLBACK (bet_graph_toggle_button_changed),
                        self);

        /* configure la valeur de la largeur des colonnes du graph */
        widget = GTK_WIDGET (gtk_builder_get_object (bet_graph_builder, "gap_spinner"));
        if (prefs->gap_spinner)
            gtk_spin_button_set_value (GTK_SPIN_BUTTON (widget), prefs->gap_spinner);
        g_signal_connect (widget,
                        "value-changed",
                        G_CALLBACK (bet_graph_gap_spinner_changed),
                        self);
    }
    else
        gtk_widget_hide (box_options_col);

    /* configuration de l'axe Y */
    /* configuration de la grille */
    button_1 = GTK_WIDGET (gtk_builder_get_object (bet_graph_builder, "major_grid_y"));
    g_object_set_data (G_OBJECT (button_1), "rang", GINT_TO_POINTER (8));

    button_2 = GTK_WIDGET (gtk_builder_get_object (bet_graph_builder, "minor_grid_y"));
    if (prefs->minor_grid_y)
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button_2), TRUE);

    g_object_set_data (G_OBJECT (button_2), "rang", GINT_TO_POINTER (9));
    /* on lie les deux boutons pour les retrouver plus tard */
    g_object_set_data (G_OBJECT (button_1), "grid_y", button_2);

    if (prefs->major_grid_y)
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button_1), TRUE);
    else
        gtk_widget_set_sensitive (button_2, FALSE);

    /* set the signal */
    g_signal_connect (button_1,
                        "toggled",
                        G_CALLBACK (bet_graph_toggle_button_changed),
                        self);
    g_signal_connect (button_2,
                        "toggled",
                        G_CALLBACK (bet_graph_toggle_button_changed),
                        self);

    return box_prefs;
}

/**
 * Création de la page des préférences pour le graphique si nécessaire.
 *
 * \param BetGraphDataStruct
 *
 * \return
 **/
static void bet_graph_create_prefs_page  (BetGraphDataStruct *self)
{
    GtkWidget *child;
    GtkWidget *label;

    label = gtk_label_new (_("Options"));
    child = bet_graph_create_line_preferences (self);
    gtk_notebook_append_page (GTK_NOTEBOOK (self->notebook), child, label);
}

/**
 *
 *
 * \param
 *
 * \return TRUE if OK FALSE otherwise
 **/
static gboolean bet_graph_populate_sectors_by_hist_data (BetGraphDataStruct *self)
{
    GtkTreeModel *model = NULL;
    GtkTreeIter iter;

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (self->tree_view));
    if (model == NULL)
        return FALSE;

    if (gtk_tree_model_get_iter_first (model, &iter))
    {
        gint account_number;
        gchar *libelle_division = self->tab_libelle[0];
        gdouble *tab_montant_division = self->tab_Y;

        /* test du numero de compte */
        gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
                        SPP_HISTORICAL_ACCOUNT_NUMBER, &account_number,
                        -1);
        if (account_number != self->account_number)
		{
			if (gsb_data_account_get_bet_hist_use_data_in_account (account_number))
			{
				gint main_account_number;

				main_account_number = bet_data_transfert_get_main_account_from_card (account_number);
				if (main_account_number != self->account_number)
					return FALSE;
			}
			else
				return FALSE;
		}

        do
        {
            gchar *desc = NULL;
            gchar *amount = NULL;
            gint div;
            gint type_infos;

            gtk_tree_model_get (GTK_TREE_MODEL(model),
                        &iter,
                        SPP_HISTORICAL_DESC_COLUMN, &desc,
                        SPP_HISTORICAL_BALANCE_AMOUNT, &amount,
                        SPP_HISTORICAL_DIV_NUMBER, &div,
                        -1);

            type_infos = bet_data_get_div_type (div);
            if (desc && (self->type_infos == -1 || type_infos == self->type_infos))
            {
                strncpy (&libelle_division[self->nbre_elemnts * TAILLE_MAX_LIBELLE], desc, TAILLE_MAX_LIBELLE);
                tab_montant_division[self->nbre_elemnts] = utils_str_strtod ((amount == NULL) ? "0" : amount, NULL);

                if (tab_montant_division[self->nbre_elemnts] < 0)
                    self->montant += -tab_montant_division[self->nbre_elemnts];
                else
                    self->montant += tab_montant_division[self->nbre_elemnts];

                self->nbre_elemnts++;
            }
            g_free (desc);
            g_free (amount);

            if (self->nbre_elemnts >= MAX_SEGMENT_CAMEMBERT)
                break;
        }
        while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));

        if (self->nbre_elemnts)
            return TRUE;
    }

    return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return TRUE if OK FALSE otherwise
 **/
static gboolean bet_graph_populate_lines_by_forecast_data (BetGraphDataStruct *self)
{
    GtkTreeModel *model = NULL;
    GtkTreeIter iter;
    gdouble prev_montant = 0.0;

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (self->tree_view));
    if (model == NULL)
        return FALSE;

    if (gtk_tree_model_get_iter_first (model, &iter))
    {
        gchar *libelle_axe_x = self->tab_libelle[0];
        gdouble *tab_Y = self->tab_Y;
        gdouble montant = 0.;
        GDate *first_date;
        GDate *last_date;
        GDate *date_courante = NULL;
        GDateDay day_courant = G_DATE_BAD_DAY;
        GDateMonth month_courant = G_DATE_BAD_MONTH;
        gint nbre_iterations = 0;

        do
        {
            gchar *amount;
            gchar *str_date;
            GValue date_value = G_VALUE_INIT;
            GDate *date;
            GDateDay day;
            GDateMonth month;
            gint diff_jours;
            gint i;

			if (G_IS_VALUE (&date_value))
				g_value_init (&date_value, G_TYPE_DATE);

            gtk_tree_model_get_value (model,
                        &iter,
                        SPP_ESTIMATE_TREE_SORT_DATE_COLUMN, &date_value);

            gtk_tree_model_get (GTK_TREE_MODEL(model),
                        &iter,
                        SPP_ESTIMATE_TREE_AMOUNT_COLUMN, &amount,
                        -1);
            date = g_value_get_boxed (&date_value);

            montant += (gdouble) utils_str_strtod ((amount == NULL) ? "0" : amount, NULL);

            if (self->nbre_elemnts == 0)
            {
                /* on ajoute 1 jour pour passer au 1er du mois */
                g_date_add_days (date, 1);

                /* on calcule le nombre maxi d'itération pour une année */
                first_date = gsb_date_copy (date);
                last_date = gsb_date_copy (date);
                g_date_add_years (last_date, 1);
                nbre_iterations = g_date_days_between (first_date, last_date);

                date_courante = gsb_date_copy (date);
                day_courant = g_date_get_day (date);
                month_courant = g_date_get_month (date);

                str_date = gsb_format_gdate (date_courante);
                strncpy (&libelle_axe_x[self->nbre_elemnts * TAILLE_MAX_LIBELLE], str_date, TAILLE_MAX_LIBELLE);

                self->nbre_elemnts++;
                g_free (str_date);
                g_date_free (first_date);
                g_date_free (last_date);
            }
            else
            {
                day = g_date_get_day (date);
                month = g_date_get_month (date);
                if (day != day_courant || month != month_courant)
                {
                    /* nombre de jours manquants */
                    diff_jours = g_date_days_between (date_courante, date);
                    for (i = diff_jours; i > 0; i--)
                    {
                        g_date_add_days (date_courante, 1);
                        str_date = gsb_format_gdate (date_courante);

                        strncpy (&libelle_axe_x[self->nbre_elemnts * TAILLE_MAX_LIBELLE],
                                    str_date, TAILLE_MAX_LIBELLE);
                        tab_Y[self->nbre_elemnts-1] = prev_montant;
                        self->nbre_elemnts++;

                        /* on dépasse d'un jour pour obtenir le solde du dernier jour */
                        if (self->nbre_elemnts > nbre_iterations)
                        {
                            self->nbre_elemnts = nbre_iterations + 1;
                            break;
                        }

                        g_free (str_date);
                    }
                    day_courant = day;
                    if (g_date_is_first_of_month (date))
                        month_courant = g_date_get_month (date);
                }
            }
            prev_montant = montant;

            if (self->nbre_elemnts > nbre_iterations)
            {
                self->nbre_elemnts = nbre_iterations;
                dialogue_hint (_("You can not exceed one year of visualization"), _("Overflow"));

                break;
            }
			g_value_unset (&date_value);
        }
        while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));

        tab_Y[self->nbre_elemnts-1] = prev_montant;

        g_date_free (date_courante);

        return TRUE;
    }

    return FALSE;
}

/**
 * callback
 *
 * \param
 *
 * \return TRUE
 **/
static gboolean bet_graph_notebook_change_page (GtkNotebook *notebook,
												gpointer npage,
												gint page,
												BetGraphDataStruct *self)
{
    if (page == 0)
        gtk_widget_show (self->button_show_grid);
    else
        gtk_widget_hide (self->button_show_grid);

    return FALSE;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void bet_graph_popup_choix_graph_activate (GtkMenuItem *menuitem,
												  BetGraphButtonStruct *self)
{
	GtkWidget *toolbar;
    GtkToolItem *item;
    gchar *tmp_str;
    gint nbre_elemnts;
    BetGraphPrefsStruct *prefs;

    prefs = self->prefs;

    /* on définit l'origine du bouton */
    if (self->origin_tab == BET_ONGLETS_PREV)
    {
		toolbar = bet_array_get_toolbar ();
        tmp_str = g_strdup ("forecast_graph");
        nbre_elemnts = gtk_toolbar_get_n_items (GTK_TOOLBAR (toolbar)) -1;
    }
    else
    {
		toolbar = bet_hist_get_toolbar ();
        tmp_str = g_strdup ("hist_graph");
        nbre_elemnts = gtk_toolbar_get_n_items (GTK_TOOLBAR (toolbar)) -1;
    }

    /* on change le type de graphique */
    prefs->type_graph = !prefs->type_graph;

    item = gtk_toolbar_get_nth_item (GTK_TOOLBAR (toolbar), nbre_elemnts);
    if (item)
        gtk_container_remove (GTK_CONTAINER (toolbar), GTK_WIDGET (item));

    item = bet_graph_button_menu_new (toolbar,
                        tmp_str,
                        G_CALLBACK (self->callback),
                        self->tree_view);

    g_free (tmp_str);

    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);

    gtk_widget_show_all (toolbar);

    /* on lance le graphique */
    g_signal_emit_by_name(item, "clicked", self->tree_view, NULL);
}

/**
 * construit le menu popup du bouton
 *
 * \param button
 * \param
 *
 * \return FALSE
 **/
static void bet_graph_popup_choix_graph_menu (GtkWidget *button,
											  GList *liste)
{
    GtkWidget *menu;
    GtkWidget *menu_item;
    GList *tmp_list;

    menu = gtk_menu_new ();

    tmp_list = liste;

    while (tmp_list)
    {
        BetGraphButtonStruct *self;

        self = tmp_list->data;

        menu_item = gtk_menu_item_new_with_label (self->name);

        g_signal_connect (G_OBJECT (menu_item),
                        "activate",
                        G_CALLBACK (bet_graph_popup_choix_graph_activate),
                        self);
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

        /* on rend insensible le bouton visible */
        gtk_widget_set_sensitive (menu_item, !self->is_visible);

        tmp_list = tmp_list->next;
    }

    gtk_menu_tool_button_set_menu (GTK_MENU_TOOL_BUTTON (button), menu);
    gtk_widget_show_all (menu);
}

/**
 * récupère les données historiques sélectionnées
 *
 * \param BetGraphDataStruct
 *
 * \return TRUE or FALSE
 **/
static gboolean bet_graph_populate_lines_by_hist_line (BetGraphDataStruct *self)
{
    GtkTreeSelection *selection;
    GtkTreeModel *model = NULL;
    GtkTreeIter iter;
    GHashTable *list_transactions;
    GHashTableIter hash_iter;
    gpointer key;
    gpointer value;
    GDate *start_current_fyear;
    GDateMonth date_month = G_DATE_BAD_MONTH;
    GDateMonth today_month = G_DATE_BAD_MONTH;
    gchar *desc;
    gchar *str_amount = NULL;
    gchar *libelle_axe_x = self->tab_libelle[0];
    gdouble *tab_Y = self->tab_Y;
    gdouble *tab_Y2 = self->tab_Y2;
    gint div_number;
    gint sub_div_nb;
    gint fyear_number;
    gint i;
    GsbReal tab[12];
    GsbReal tab2[12];

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (self->tree_view));

    if (!gtk_tree_selection_get_selected (GTK_TREE_SELECTION (selection), &model, &iter))
        return FALSE;

    gtk_tree_model_get (GTK_TREE_MODEL (model),
                        &iter,
                        SPP_HISTORICAL_CURRENT_COLUMN, &str_amount,
                        SPP_HISTORICAL_DIV_NUMBER, &div_number,
                        SPP_HISTORICAL_SUB_DIV_NUMBER, &sub_div_nb,
                        -1);

    fyear_number = gsb_data_account_get_bet_hist_fyear (self->account_number);

    /* on calcule les montants par mois en premier */
    list_transactions = bet_hist_get_list_trans_current_fyear ();
    if (g_hash_table_size (list_transactions) == 0)
        return FALSE;

    /* on initialise les tableaux des montants */
    for (i = 0; i < 12; i++)
    {
        tab[i] = null_real;
        if (fyear_number > 0)
            tab2[i] = null_real;
    }

    g_hash_table_iter_init (&hash_iter, list_transactions);
    while (g_hash_table_iter_next (&hash_iter, &key, &value))
    {
        TransactionCurrentFyear *tcf = (TransactionCurrentFyear*) value;

        if (tcf->div_nb == div_number)
        {
            date_month = g_date_get_month (tcf->date);
            if (sub_div_nb > 0)
            {
                if (tcf->sub_div_nb == sub_div_nb)
                {
                    switch (tcf->type_de_transaction)
                    {
                        case 0:
                            if (fyear_number > 0)
                                tab[date_month-1] = gsb_real_add (tab[date_month-1], tcf->amount);
                            break;
                        case 1:
                            if (fyear_number > 0)
                                tab2[date_month-1] = gsb_real_add (tab2[date_month-1], tcf->amount);
                            else
                                tab[date_month-1] = gsb_real_add (tab[date_month-1], tcf->amount);
                            break;
                        case 2:
                            tab[date_month-1] = gsb_real_add (tab[date_month-1], tcf->amount);
                            if (fyear_number > 0)
                                tab2[date_month-1] = gsb_real_add (tab2[date_month-1], tcf->amount);
                            break;
                    }
                }
            }
            else
            {
                switch (tcf->type_de_transaction)
                {
                    case 0:
                        if (fyear_number > 0)
                            tab[date_month-1] = gsb_real_add (tab[date_month-1], tcf->amount);
                        break;
                    case 1:
                        if (fyear_number > 0)
                            tab2[date_month-1] = gsb_real_add (tab2[date_month-1], tcf->amount);
                        else
                            tab[date_month-1] = gsb_real_add (tab[date_month-1], tcf->amount);
                        break;
                    case 2:
                        tab[date_month-1] = gsb_real_add (tab[date_month-1], tcf->amount);
                        if (fyear_number > 0)
                            tab2[date_month-1] = gsb_real_add (tab2[date_month-1], tcf->amount);
                        break;
                }
            }
        }
    }

    /* On commence par le début de l'exercice courant puis on balaie les douze mois */
    start_current_fyear = bet_hist_get_start_date_current_fyear ();
    date_month = g_date_get_month (start_current_fyear);
    today_month = g_date_get_month (gdate_today ());

    for (i = 0; i < 12; i++)
    {
        if (fyear_number > 0)
            desc = g_strdup_printf ("%s", gettext (long_str_months[date_month-1]));
        else
            desc = g_strdup_printf ("%s %d", gettext (short_str_months[date_month-1]),
                        g_date_get_year (start_current_fyear));
        strncpy (&libelle_axe_x[self->nbre_elemnts * TAILLE_MAX_LIBELLE], desc, TAILLE_MAX_LIBELLE);

        /* Pour un graphique line on n'affiche pas 0 comme donnée des mois futurs */
        if (strcmp (self->service_id, "GogLinePlot") == 0)
        {
            if (i < (gint) today_month)
            {
                tab_Y[self->nbre_elemnts] = gsb_real_real_to_double (tab[date_month-1]);
                if (fyear_number > 0)
                    tab_Y2[self->nbre_elemnts] = gsb_real_real_to_double (tab2[date_month-1]);
            }
            else
            {
                if (fyear_number > 0)
                {
                    tab_Y[self->nbre_elemnts] = gsb_real_real_to_double (tab[date_month-1]);
                    tab_Y2[self->nbre_elemnts] = go_nan;
                }
                else
                    tab_Y[self->nbre_elemnts] = go_nan;
            }
        }
        else
        {
            tab_Y[self->nbre_elemnts] = gsb_real_real_to_double (tab[date_month-1]);
            if (fyear_number > 0)
                tab_Y2[self->nbre_elemnts] = gsb_real_real_to_double (tab2[date_month-1]);
        }

        self->nbre_elemnts++;
        g_date_add_months (start_current_fyear, 1);
        date_month = g_date_get_month (start_current_fyear);
    }

    /* on met la division sous division comme titre */
    self->title = g_strconcat (bet_data_get_div_name (div_number, sub_div_nb, NULL), " = ", str_amount, NULL);

    g_date_free (start_current_fyear);

    /* return value */
    return TRUE;
}

/**
 * retourne la date de début pour les graphiques mensuels
 *
 * \param   none
 *
 * \return  date_debut_periode
 **/
static GDate *bet_graph_get_date_debut_periode (void)
{
    GDate *date_debut_periode;
    GDate *date_jour;
    gint fyear_number;

    date_jour = gdate_today ();
    fyear_number = gsb_data_fyear_get_from_date (date_jour);
    if (!fyear_number)
        date_debut_periode = g_date_new_dmy (1, 1, g_date_get_year (date_jour));
    else
        date_debut_periode = gsb_date_copy (gsb_data_fyear_get_beginning_date (fyear_number));

    g_date_free (date_jour);

    return date_debut_periode;
}

static void bet_graph_button_menu_destroy (GtkWidget *button,
										   GList *liste)
{
    GList *tmp_list;

	tmp_list = liste;
	while (tmp_list)
	{
		BetGraphButtonStruct *self;

		self = tmp_list->data;
		struct_free_bet_graph_button (self);

		tmp_list = tmp_list->next;
	}
	g_list_free (liste);
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 * retourne un bouton pour choisir entre un graph ligne ou colonne
 *
 * \param type_graph    forecast_graph or historical_graph
 * \param callback
 * \paramtree_view
 *
 * \return item
 **/
GtkToolItem *bet_graph_button_menu_new (GtkWidget *toolbar,
										const gchar *type_graph,
										GCallback callback,
										GtkWidget *tree_view)
{
    GtkToolItem *item = NULL;
    GList *liste = NULL;
    gchar *tooltip = NULL;
    gint origin_tab = 0;
    BetGraphButtonStruct *self;
    BetGraphPrefsStruct *prefs = NULL;

    /* initialisation des préférences */
    if (strcmp (type_graph, "forecast_graph") == 0)
    {
        if (prefs_prev == NULL)
            prefs_prev = struct_initialise_bet_graph_prefs ();
        prefs = prefs_prev;
        origin_tab = BET_ONGLETS_PREV;
        tooltip = g_strdup (_("Display the graph of forecast"));
    }
    else if (strcmp (type_graph, "hist_graph") == 0)
    {
        if (prefs_hist == NULL)
            prefs_hist = struct_initialise_bet_graph_prefs ();
        prefs = prefs_hist;
        origin_tab = BET_ONGLETS_HIST;
        tooltip = g_strdup (_("Display the monthly graph"));
    }

    /* initialisation des données du premier bouton */
    self = struct_initialise_bet_graph_button ();

    self->name = g_strdup (_("Column"));
    self->filename = g_strdup ("gsb-graph-histo-24.png");
    self->service_id = g_strdup ("GogBarColPlot");
    self->callback = callback;
    self->tree_view = tree_view;
    self->origin_tab = origin_tab;
    self->prefs = prefs;

    /* si ce boutton est celui par défaut on l'affiche */
    if (prefs && prefs->type_graph == 0)
    {
        self->is_visible = TRUE;
        item = utils_buttons_tool_menu_new_from_image_label (self->filename, self->name);
        self->button = item;
        g_object_set_data (G_OBJECT (self->button), "service_id", self->service_id);
        g_object_set_data (G_OBJECT (self->button), "origin_tab", GINT_TO_POINTER (origin_tab));
        g_signal_connect (G_OBJECT (self->button),
                        "clicked",
                        self->callback,
                        self->tree_view);
    }
    liste = g_list_append (liste, self);

    /* initialisation des données du deuxième bouton */
    self = struct_initialise_bet_graph_button ();

    self->name = g_strdup (_("Line"));
    self->filename = g_strdup ("gsb-graph-line-24.png");
    self->service_id = g_strdup ("GogLinePlot");
    self->callback = callback;
    self->tree_view = tree_view;
    self->origin_tab = origin_tab;
    self->prefs = prefs;

    /* si ce boutton est celui par défaut on l'affiche */
    if (prefs && prefs->type_graph == 1)
    {
        self->is_visible = TRUE;
        item = utils_buttons_tool_menu_new_from_image_label (self->filename, self->name);
        self->button = item;
        g_object_set_data (G_OBJECT (self->button), "service_id", self->service_id);
        g_object_set_data (G_OBJECT (self->button), "origin_tab", GINT_TO_POINTER (origin_tab));
        g_signal_connect (G_OBJECT (self->button),
                        "clicked",
                        self->callback,
                        self->tree_view);
    }
    liste = g_list_append (liste, self);

    if (tooltip)
    {
        gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (item), tooltip);
        g_free (tooltip);
    }

    gtk_menu_tool_button_set_menu (GTK_MENU_TOOL_BUTTON (item), gtk_menu_new ());

	g_signal_connect (G_OBJECT (item),
                        "show-menu",
                        G_CALLBACK (bet_graph_popup_choix_graph_menu),
                        liste);

    g_signal_connect (G_OBJECT (item),
                        "destroy",
                        G_CALLBACK (bet_graph_button_menu_destroy),
                        liste);

    return item;
}

/**
 *  sérialise les préférences des graphes
 *
 * \param   none
 *
 * \return a newly allocated string which must be freed when no more used.
 **/
gchar *bet_graph_get_configuration_string (gint origin_tab)
{
    gchar *new_str = NULL;
    const gchar *tmp_str = NULL;
    BetGraphPrefsStruct *prefs = NULL;

    if (origin_tab == BET_ONGLETS_PREV)
    {
        prefs = prefs_prev;
        tmp_str = "forecast_prefs";
    }
    else
    {
        prefs = prefs_hist;
        tmp_str = "hist_prefs";
    }

	if (NULL == prefs)
		return NULL;

    new_str = g_markup_printf_escaped ("\t<Bet_graph prefs=\"%s:%d:%d:%d:%d:%d:%d:%d:%.0f:%d:%d:%d:%d\" />\n",
                        tmp_str,
                        prefs->type_graph,
                        prefs->major_tick_out,
                        prefs->major_tick_in,
                        prefs->major_tick_labeled,
                        prefs->position,
                        prefs->new_axis_line,
                        prefs->cross_entry,
                        prefs->degrees,
                        prefs->gap_spinner,
                        prefs->before_grid,
                        prefs->major_grid_y,
                        prefs->minor_grid_y);

    return new_str;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
void bet_graph_line_graph_new (GtkWidget *button,
							   GtkTreeView *tree_view)
{
    GtkWidget *dialog;
    GtkWidget *label;
    gchar *title;
    gchar *service_id;
    gint result;
    gint account_number;
    gint currency_number;
    gint origin_tab;
    BetGraphDataStruct *self = NULL;
    BetGraphPrefsStruct *prefs = NULL;

    devel_debug (NULL);

    /* Initialisation d'un nouveau GtkBuilder */
    if (!bet_graph_initialise_builder ())
        return;

    account_number = gsb_gui_navigation_get_current_account ();
    currency_number = gsb_data_account_get_currency (account_number);
    service_id = g_object_get_data (G_OBJECT (button), "service_id");

    /* initialisation des préférences */
    origin_tab = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (button), "origin_tab"));
    if (origin_tab == BET_ONGLETS_HIST)
        prefs = prefs_hist;
    else
        prefs = prefs_prev;

    /* Initialisations des données */
    self = struct_initialise_bet_graph_data ();
    self->tree_view = tree_view;
    self->account_number = account_number;
    self->currency_number = currency_number;
    self->service_id = g_strdup (service_id);
    self->prefs = prefs;

    /* Création de la fenêtre de dialogue pour le graph */
    dialog = GTK_WIDGET (gtk_builder_get_object (bet_graph_builder, "bet_graph_dialog"));
    gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (grisbi_app_get_active_window (NULL)));
    gtk_widget_set_size_request (dialog, PAGE_WIDTH+30, PAGE_HEIGHT+70);
    g_signal_connect (G_OBJECT (dialog),
                        "destroy",
                        G_CALLBACK (gtk_widget_destroy),
                        NULL);

    /* initialise le bouton show_grid avec la préférence "Major_grid" */
    self->button_show_grid = GTK_WIDGET (gtk_builder_get_object (bet_graph_builder, "button_show_grid"));
    gtk_button_set_image (GTK_BUTTON (self->button_show_grid),
                        gtk_image_new_from_file (g_build_filename (gsb_dirs_get_pixmaps_dir (),
                        "gsb-grille-16.png", NULL)));
    if (prefs->major_grid_y)
        bet_graph_show_grid_button_configure (self, TRUE, -1);
    g_signal_connect (self->button_show_grid,
                        "toggled",
                        G_CALLBACK (bet_graph_show_grid_button_changed),
                        self);

    /* set the title */
    label = GTK_WIDGET (g_object_get_data (G_OBJECT (tree_view), "label_title"));
    title = dialogue_make_pango_attribut ("weight=\"bold\" size=\"x-large\"",
                    gtk_label_get_text (GTK_LABEL (label)));

    label = GTK_WIDGET (gtk_builder_get_object (bet_graph_builder, "label_canvas"));
    gtk_label_set_markup (GTK_LABEL (label), title);
    g_free (title);

    /* initialise les pages pour les graphiques ligne et barre */
    self->notebook = GTK_NOTEBOOK (gtk_builder_get_object (bet_graph_builder, "notebook"));
    g_signal_connect_after (self->notebook,
                        "switch-page",
                        G_CALLBACK (bet_graph_notebook_change_page),
                        self);

    /* set the graphique page */
    self->plot = bet_graph_create_graph_page (self, TRUE);

    /* set the preferences page */
    bet_graph_create_prefs_page  (self);

    /* populate data */
    self->valid_data = bet_graph_populate_lines_by_forecast_data (self);
    if (!self->valid_data)
        return;

    /* affiche les données */
    result = bet_graph_affiche_XY_line (self);
    if (!result)
        return;

    gtk_widget_show_all (dialog);
    gtk_notebook_set_current_page (self->notebook, 0);

    gtk_dialog_run (GTK_DIALOG (dialog));

    /* free the data */
    struct_free_bet_graph_data (self);
    g_object_unref (G_OBJECT (bet_graph_builder));
    gtk_widget_destroy (dialog);
}

/**
 *
 *
 * \param
 *
 * \return TRUE
 **/
void bet_graph_montly_graph_new (GtkWidget *button,
								 GtkTreeView *tree_view)
{
    GtkWidget *dialog;
    GtkWidget *label;
    gchar *title = NULL;
    gchar *tmp_str;
    gint result;
    gint origin_tab;
    gint fyear_number;
    BetGraphDataStruct *self;

    devel_debug (NULL);

    /* Initialisation d'un nouveau GtkBuilder */
    if (!bet_graph_initialise_builder ())
        return;

    /* Initialisations des données */
    self = struct_initialise_bet_graph_data ();
    self->tree_view = tree_view;
    self->account_number = gsb_gui_navigation_get_current_account ();
    self->currency_number = gsb_data_account_get_currency (self->account_number);
    self->service_id = g_strdup (g_object_get_data (G_OBJECT (button), "service_id"));

    origin_tab = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (button), "origin_tab"));
    if (origin_tab == BET_ONGLETS_HIST)
    {
        self->prefs = prefs_hist;
        fyear_number = gsb_data_account_get_bet_hist_fyear (self->account_number);
        if (fyear_number > 0)
        {
            GDate *start_current_fyear;

            self->double_axe = TRUE;
            self->is_legend = TRUE;

            /* set the titles */
            start_current_fyear = bet_hist_get_start_date_current_fyear ();
            self->title_Y = g_strdup (gsb_data_fyear_get_name (fyear_number));
            self->title_Y2 = g_strdup_printf ("%d", g_date_get_year (start_current_fyear));
            title = g_strdup_printf (_("Amounts %s - %s for the account: '%s'"),
                            self->title_Y,
                            self->title_Y2,
                            gsb_data_account_get_name (self->account_number));
            g_date_free (start_current_fyear);
        }
        else
        {
            GDate *date_debut_periode;

            /* set the title */
            date_debut_periode = bet_graph_get_date_debut_periode ();
            tmp_str = gsb_format_gdate (date_debut_periode);
            title = g_strdup_printf (_("Monthly amounts since %s for the account: '%s'"),
                            tmp_str,
                            gsb_data_account_get_name (self->account_number));
            g_date_free (date_debut_periode);
            g_free (tmp_str);
        }
    }
    else
        self->prefs = prefs_prev;

    /* Création de la fenêtre de dialogue pour le graph */
    dialog = GTK_WIDGET (gtk_builder_get_object (bet_graph_builder, "bet_graph_dialog"));
    gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (grisbi_app_get_active_window (NULL)));
    gtk_widget_set_size_request (dialog, PAGE_WIDTH+30, PAGE_HEIGHT+70);
    g_signal_connect (G_OBJECT (dialog),
                        "destroy",
                        G_CALLBACK (gtk_widget_destroy),
                        NULL);

    /* initialise le bouton show_grid avec la préférence "Major_grid" */
    self->button_show_grid = GTK_WIDGET (gtk_builder_get_object (bet_graph_builder, "button_show_grid"));
    gtk_button_set_image (GTK_BUTTON (self->button_show_grid),
                        gtk_image_new_from_file (g_build_filename (gsb_dirs_get_pixmaps_dir (),
                        "gsb-grille-16.png", NULL)));
    if (self->prefs->major_grid_y)
        bet_graph_show_grid_button_configure (self, TRUE, -1);
    g_signal_connect (self->button_show_grid,
                        "toggled",
                        G_CALLBACK (bet_graph_show_grid_button_changed),
                        self);

    title = dialogue_make_pango_attribut ("weight=\"bold\" size=\"x-large\"", title);

    label = GTK_WIDGET (gtk_builder_get_object (bet_graph_builder, "label_canvas"));
    gtk_label_set_markup (GTK_LABEL (label), title);

    g_free (title);

    /* initialise les pages pour les graphiques ligne et barre */
    self->notebook = GTK_NOTEBOOK (gtk_builder_get_object (bet_graph_builder, "notebook"));
    g_signal_connect_after (self->notebook,
                        "switch-page",
                        G_CALLBACK (bet_graph_notebook_change_page),
                        self);

    /* populate data */
    self->valid_data = bet_graph_populate_lines_by_hist_line (self);
    if (!self->valid_data)
        return;

    /* set the graphique page */
    self->plot = bet_graph_create_graph_page (self, TRUE);

    /* set the preferences page */
    bet_graph_create_prefs_page  (self);

    /* affiche les données */
    result = bet_graph_affiche_XY_line (self);
    if (!result)
        return;

    gtk_widget_show_all (dialog);
    gtk_notebook_set_current_page (self->notebook, 0);

    gtk_dialog_run (GTK_DIALOG (dialog));

    /* free the data */
    g_object_unref (G_OBJECT (bet_graph_builder));

    gtk_widget_destroy (dialog);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
void bet_graph_sectors_graph_new (GtkWidget *button,
								  GtkTreeView *tree_view)
{
    GtkWidget *dialog;
    GtkWidget *label;
    GtkWidget *notebook;
    GtkWidget *box_pie;
    GtkWidget *button_grid;
    gchar *title;
    gint result_credit = FALSE;
    gint result_debit = FALSE;
    gint account_number;
    gint currency_number;
    BetGraphDataStruct *self_credit;
    BetGraphDataStruct *self_debit;
	GrisbiWinEtat *w_etat;

    devel_debug (NULL);
	w_etat = grisbi_win_get_w_etat ();

    /* Initialisation d'un nouveau GtkBuilder */
    if (!bet_graph_initialise_builder ())
        return;

    /* Création de la fenêtre de dialogue pour le graph */
    dialog = GTK_WIDGET (gtk_builder_get_object (bet_graph_builder, "bet_graph_dialog"));
    gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (grisbi_app_get_active_window (NULL)));
    gtk_widget_set_size_request (dialog, PAGE_WIDTH+30, PAGE_HEIGHT+70);
    g_signal_connect (G_OBJECT (dialog),
                        "destroy",
                        G_CALLBACK (gtk_widget_destroy),
                        NULL);

    /* set the title */
    label = GTK_WIDGET (g_object_get_data (G_OBJECT (tree_view), "label_title"));
    title = dialogue_make_pango_attribut ("weight=\"bold\" size=\"x-large\"",
                    gtk_label_get_text (GTK_LABEL (label)));

    label = GTK_WIDGET (gtk_builder_get_object (bet_graph_builder, "label_canvas"));
    gtk_label_set_markup (GTK_LABEL (label), title);
    g_free (title);

    /* initialise les pages pour les graphiques camembert */
    notebook = GTK_WIDGET (gtk_builder_get_object (bet_graph_builder, "notebook"));
    gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook), FALSE);
    g_object_set_data (G_OBJECT (notebook), "dialog", dialog);

    box_pie = GTK_WIDGET (gtk_builder_get_object (bet_graph_builder, "box_pie"));
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), box_pie, gtk_label_new (_("Graph")));

    /* initialisation des structures de données */
    account_number = gsb_gui_navigation_get_current_account ();
    currency_number = gsb_data_account_get_currency (account_number);

    /* Set the graph for debit */
    self_debit = struct_initialise_bet_graph_data ();
    self_debit->notebook = GTK_NOTEBOOK (notebook);
    self_debit->tree_view = tree_view;
    self_debit->account_number = account_number;
    self_debit->currency_number = currency_number;
    self_debit->type_infos = 1;
	if (w_etat->metatree_assoc_mode)
		self_debit->title = g_strdup ("Charges");
	else
		self_debit->title = g_strdup (_("Expenses"));
    self_debit->service_id = g_strdup ("GogPiePlot");
    self_debit->is_legend = TRUE;

    self_debit->plot = bet_graph_create_graph_page (self_debit, FALSE);

    /* populate the data for debit */
    if (self_debit->valid_data == FALSE)
        self_debit->valid_data = bet_graph_populate_sectors_by_hist_data (self_debit);

    /* show the data for debit */
    if (self_debit->valid_data)
        result_debit = bet_graph_affiche_camemberts (self_debit);

    /* Set the graph for credit */
    self_credit = struct_initialise_bet_graph_data ();
    self_credit->notebook = GTK_NOTEBOOK (notebook);
    self_credit->tree_view = tree_view;
    self_credit->account_number = account_number;
    self_credit->currency_number = currency_number;
    self_credit->type_infos = 0;
	if (w_etat->metatree_assoc_mode)
		self_credit->title = g_strdup ("Produits");
	else
		self_credit->title = g_strdup (_("Incomes"));

    self_credit->service_id = g_strdup ("GogPiePlot");
    self_credit->is_legend = TRUE;

    self_credit->plot = bet_graph_create_graph_page (self_credit, FALSE);

    /* populate the data for credit */
    if (self_credit->valid_data == FALSE)
        self_credit->valid_data = bet_graph_populate_sectors_by_hist_data (self_credit);

    /* show the data for credit */
    if (self_credit->valid_data)
        result_credit = bet_graph_affiche_camemberts (self_credit);

    /* show or hide widgets */
    gtk_widget_show_all (dialog);

    /* cache le bouton show_grid inutile ici */
    button_grid = GTK_WIDGET (gtk_builder_get_object (bet_graph_builder, "button_show_grid"));
    gtk_widget_hide (button_grid);

    if (!result_credit)
        gtk_widget_hide (self_credit->widget);
    if (!result_debit)
        gtk_widget_hide (self_debit->widget);

    gtk_dialog_run (GTK_DIALOG (dialog));

    /* free the data */
    struct_free_bet_graph_data (self_debit);
    struct_free_bet_graph_data (self_credit);
    g_object_unref (G_OBJECT (bet_graph_builder));
    gtk_widget_destroy (dialog);
}

/**
 *
 *
 * \param
 *
 * \return TRUE
 **/
void bet_graph_set_configuration_variables (const gchar *string)
{
    gchar **tab;
    BetGraphPrefsStruct *prefs = NULL;

	if (string == NULL)
	{
		if (prefs_prev == NULL)
		{
            prefs_prev = struct_initialise_bet_graph_prefs ();
		}
		else
		{
			g_free (prefs_prev);
			prefs_prev =struct_initialise_bet_graph_prefs ();
		}

		if (prefs_hist == NULL)
		{
			prefs_hist = struct_initialise_bet_graph_prefs ();
		}
		else
		{
			g_free (prefs_hist);
			prefs_hist = struct_initialise_bet_graph_prefs ();
		}

		return;
	}

    tab = g_strsplit (string, ":", 0);

    if (strcmp (tab[0], "forecast_prefs") == 0)
    {
        if (prefs_prev == NULL)
            prefs_prev = struct_initialise_bet_graph_prefs ();
        prefs = prefs_prev;
    }
    else
    {
        if (prefs_hist == NULL)
            prefs_hist = struct_initialise_bet_graph_prefs ();
        prefs = prefs_hist;
    }

    /* initialisation des variables */
    prefs->type_graph = utils_str_atoi (tab[1]);
	if (prefs->type_graph > 1)
		prefs->type_graph = 1;
    prefs->major_tick_out = utils_str_atoi (tab[2]);
	if (prefs->major_tick_out > 1)
		prefs->major_tick_out = TRUE;
    prefs->major_tick_in = utils_str_atoi (tab[3]);
	if (prefs->major_tick_in > 1)
		prefs->major_tick_in = FALSE;
    prefs->major_tick_labeled = utils_str_atoi (tab[4]);
	if (prefs->major_tick_labeled > 1)
		prefs->major_tick_labeled = TRUE;
    prefs->position = utils_str_atoi (tab[5]);
	if (prefs->position > 2)
		prefs->position = 2;
    prefs->new_axis_line = utils_str_atoi (tab[6]);
	if (prefs->new_axis_line > 1)
		prefs->new_axis_line = FALSE;
    prefs->cross_entry = utils_str_atoi (tab[7]);
	if (prefs->cross_entry > 1)
		prefs->cross_entry = 0;
    prefs->degrees = utils_str_atoi (tab[8]);
	if (prefs->degrees > 180)
		prefs->degrees = 90;
    prefs->gap_spinner = utils_str_atoi (tab[9]);
	if (prefs->gap_spinner > 100)
		prefs->gap_spinner = 50;
    prefs->before_grid = utils_str_atoi (tab[10]);
	if (prefs->before_grid > 1)
		prefs->before_grid = TRUE;
    prefs->major_grid_y = utils_str_atoi (tab[11]);
	if (prefs->major_grid_y > 1)
		prefs->major_grid_y = FALSE;
    prefs->minor_grid_y = utils_str_atoi (tab[12]);
	if (prefs->minor_grid_y > 1)
		prefs->minor_grid_y = FALSE;

    g_strfreev (tab);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void struct_free_bet_graph_prefs (void)
{
    if (prefs_prev)
    {
        g_free (prefs_prev);
        prefs_prev = NULL;
    }
    if (prefs_hist)
    {
        g_free (prefs_hist);
        prefs_hist = NULL;
    }
}

/**
 *
 *
 * \param
 *
 * \return TRUE
 **/
void bet_graph_free_configuration_variables (void)
{
	if (prefs_prev)
		g_free (prefs_prev);
	if (prefs_hist)
		g_free (prefs_hist);
}

#endif /* HAVE_GOFFICE */
/**
 *
 *
 * \param
 *
 * \return
 **/
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
