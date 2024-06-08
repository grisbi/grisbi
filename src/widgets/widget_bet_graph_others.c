/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2022 Pierre Biava (grisbi@pierre.biava.name)                    */
/*          http://www.grisbi.org                                                */
/*                                                                               */
/*     This program is free software; you can redistribute it and/or modify      */
/*     it under the terms of the GNU General Public License as published by      */
/*     the Free Software Foundation; either version 2 of the License, or         */
/*     (at your option) any later version.                                       */
/*                                                                               */
/*     This program is distributed in the hope that it will be useful,           */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*     GNU General Public License for more details.                              */
/*                                                                               */
/*     You should have received a copy of the GNU General Public License         */
/*     along with this program; if not, write to the Free Software               */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                               */
/* *******************************************************************************/

#include "config.h"

#include <errno.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

#include <goffice/goffice.h>

/*START_INCLUDE*/
#include "widget_bet_graph_others.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "gsb_data_account.h"
#include "gsb_file.h"
#include "structures.h"
#include "utils_real.h"
#include "widget_bet_graph_options.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _WidgetBetGraphOthersPrivate   WidgetBetGraphOthersPrivate;

struct _WidgetBetGraphOthersPrivate
{
	GtkWidget *			vbox_bet_graph_others;
	GtkWidget *			label_bet_graph_others;
	GtkWidget *			notebook_bet_graph_others;
	GtkWidget *			button_show_grid;
};

G_DEFINE_TYPE_WITH_PRIVATE (WidgetBetGraphOthers, widget_bet_graph_others, GTK_TYPE_DIALOG)
/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * callback qui gère l'affichage des tip
 *
 * \param
 * \param
 * \param
 *
 * \return TRUE to stop the signal emission
 **/
static gboolean widget_bet_graph_options_motion_notify_event (GtkWidget *event_box,
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
			gchar *tmp_str;

			gog_series_get_xy_data (series, &x, &y);
			tmp_str = utils_real_get_string_with_currency_from_double (y[index], self->currency_number);
			buf = g_strdup_printf (_("date %s : value %s"), self->tab_vue_libelle[index], tmp_str);
			g_free(tmp_str);
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
			gchar *tmp_str;

			x_map = gog_chart_map_get_axis_map (map, 0);
			index = gog_axis_map_from_view (x_map, event->x);

			tmp_str = utils_real_get_string_with_currency_from_double (self->tab_Y[index-1], self->currency_number);
			buf = g_strdup_printf (_("date %s : value %s"), self->tab_vue_libelle[index-1], tmp_str);
			g_free(tmp_str);
		}
		gog_chart_map_free (map);
	}

	gtk_widget_set_tooltip_markup (GTK_WIDGET (self->widget), buf);
	g_free (buf);

	return TRUE;
}

/**
 *
 *
 * \param
 *
 * \return TRUE if OK FALSE otherwise
 **/
static gboolean widget_bet_graph_others_display_XY_line (BetGraphDataStruct *self)
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
			g_object_set (G_OBJECT (self->plot), "before-grid", TRUE, NULL);

		/* modification du ratio de largeur des barres. 0 pas d'espace entre deux barres */
		g_object_set (G_OBJECT (self->plot), "gap-percentage", prefs->gap_spinner, NULL);
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
 * Création de la page pour le graphique initialisée
 *
 * \param
 * \param   add_page
 *
 * \return GogPlot
 **/
static GogPlot *widget_bet_graph_others_create_graph_page  (BetGraphDataStruct *self,
															gboolean add_page)
{
	GtkWidget *child;
	GtkWidget *w;
	GogGraph *graph;
	GogLabel *label;
	GogPlot *plot = NULL;
	GOStyle *style;
	GOData *data;
	PangoFontDescription *desc;

	/* Set the new page */
	child = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_notebook_prepend_page (self->notebook, child, gtk_label_new (_("Graph")));

	/* Set the graph widget */
	w = go_graph_widget_new (NULL);
	g_signal_connect (G_OBJECT (w),
					  "motion-notify-event",
					  G_CALLBACK (widget_bet_graph_options_motion_notify_event),
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
 * \return TRUE
 **/
void widget_bet_graph_others_graph_update (BetGraphDataStruct *self)
{
	gtk_notebook_remove_page (self->notebook, 0);

	self->plot = widget_bet_graph_others_create_graph_page (self, TRUE);
	gtk_widget_show_all (GTK_WIDGET (self->notebook));

	widget_bet_graph_others_display_XY_line (self);
}

/**
 * callback
 *
 * \param
 * \param
 *
 * \return
 **/
static void widget_bet_graph_others_show_grid_button_toggled (GtkToggleButton *togglebutton,
															  BetGraphDataStruct *self)
{
	GtkWidget *widget_options;
	gboolean active;

	active = gtk_toggle_button_get_active (togglebutton);

	self->show_grid = active;
	if (active)
		gtk_button_set_label (GTK_BUTTON (self->button_show_grid), _("Hide grid"));
	else
		gtk_button_set_label (GTK_BUTTON (self->button_show_grid), _("Show grid"));

	/* on update éventuellement major_grid_y */
	widget_options = gtk_notebook_get_nth_page (GTK_NOTEBOOK (self->notebook), 1);
	widget_bet_graph_options_set_button_major_grid_y (widget_options, self, active);

	/* on met à jour le graph */
	widget_bet_graph_others_graph_update (self);

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
void widget_bet_graph_others_show_grid_button_configure (BetGraphDataStruct *self,
																gint active,
																gint hide)
{
	if (active != -1)
	{
		/* on bloque le signal */
		g_signal_handlers_block_by_func (G_OBJECT (self->button_show_grid),
										 widget_bet_graph_others_show_grid_button_toggled,
										 self);

		self->show_grid = active;
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (self->button_show_grid), active);

		if (active) /* la grille est visible */
			gtk_button_set_label (GTK_BUTTON (self->button_show_grid), _("Hide grid"));
		else
			gtk_button_set_label (GTK_BUTTON (self->button_show_grid), _("Show grid"));

		/* on débloque le signal */
		g_signal_handlers_unblock_by_func (G_OBJECT (self->button_show_grid),
										   widget_bet_graph_others_show_grid_button_toggled,
										   self);
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
 *
 * \return TRUE
 **/
static gboolean widget_bet_graph_others_notebook_switch_page (GtkNotebook *notebook,
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
 * Création du widget bet_graph_others
 *
 * \param
 * \return
 **/
static void widget_bet_graph_others_setup_widget (WidgetBetGraphOthers *dialog,
												  BetGraphDataStruct *self,
												  const gchar *title,
												  gint origin_tab)
{
	gchar *tmp_str;
	GtkWidget *child;
	GtkWidget *label;
	gboolean result = FALSE;
	BetGraphPrefsStruct *prefs;
	WidgetBetGraphOthersPrivate *priv;

	devel_debug (NULL);
	priv = widget_bet_graph_others_get_instance_private (dialog);

	/* init position et dimension */
	gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (grisbi_app_get_active_window (NULL)));
	gtk_widget_set_size_request (GTK_WIDGET (dialog), PAGE_WIDTH, PAGE_HEIGHT);

	/* set title of dialog window */
	if (g_strcmp0 (self->service_id, "GogBarColPlot") == 0)
		gtk_window_set_title (GTK_WINDOW (dialog), _("Graph: Bars"));
	else
		gtk_window_set_title (GTK_WINDOW (dialog), _("Graph: Lines"));

	/* set title of graph */
	tmp_str = dialogue_make_pango_attribut ("weight=\"bold\" size=\"x-large\"", title);
	gtk_label_set_markup (GTK_LABEL (priv->label_bet_graph_others), tmp_str);
	g_free (tmp_str);

	/* attache dialog au notebook */
	g_object_set_data (G_OBJECT (priv->notebook_bet_graph_others), "dialog", dialog);

	/* set signal */
	g_signal_connect (G_OBJECT (dialog),
					  "destroy",
					  G_CALLBACK (gtk_widget_destroy),
					  NULL);

	/* get prefs */
	prefs = self->prefs;

	/* initialise le bouton show_grid et la préférence "Major_grid" */
	self->button_show_grid = priv->button_show_grid;
	if (prefs->major_grid_y)
		widget_bet_graph_others_show_grid_button_configure (self, TRUE, -1);
	g_signal_connect (priv->button_show_grid,
					  "toggled",
					  G_CALLBACK (widget_bet_graph_others_show_grid_button_toggled),
					  self);

	/* initialise les pages pour les graphiques ligne et barre */
	self->notebook = GTK_NOTEBOOK (priv->notebook_bet_graph_others);
	g_signal_connect_after (self->notebook,
							"switch-page",
							G_CALLBACK (widget_bet_graph_others_notebook_switch_page),
							self);

	/* set the graphique page */
	self->plot = widget_bet_graph_others_create_graph_page (self, TRUE);

	/* add the preferences page */
	label = gtk_label_new (_("Options"));
	child = GTK_WIDGET (widget_bet_graph_options_new (self));
	gtk_notebook_append_page (GTK_NOTEBOOK (self->notebook), child, label);

	/* populate data */
	if (origin_tab == BET_ONGLETS_HIST)
	{
		self->valid_data = bet_graph_populate_lines_by_hist_line (self);
		if (!self->valid_data)
			return;
	}
	else if (origin_tab == BET_ONGLETS_PREV)
	{
		self->valid_data = bet_graph_populate_lines_by_forecast_data (self);
		if (!self->valid_data)
			return;
	}

	/* affiche les données */
	result = widget_bet_graph_others_display_XY_line (self);
	if (!result)
		gtk_widget_hide (self->widget);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void widget_bet_graph_others_init (WidgetBetGraphOthers *dialog)
{
	gtk_widget_init_template (GTK_WIDGET (dialog));
}

static void widget_bet_graph_others_dispose (GObject *object)
{
	G_OBJECT_CLASS (widget_bet_graph_others_parent_class)->dispose (object);
}

static void widget_bet_graph_others_class_init (WidgetBetGraphOthersClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = widget_bet_graph_others_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/widgets/widget_bet_graph_others.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetBetGraphOthers, vbox_bet_graph_others);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetBetGraphOthers, label_bet_graph_others);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetBetGraphOthers, notebook_bet_graph_others);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetBetGraphOthers, button_show_grid);
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 *
 *
 * \param
 *
 * \return
 **/
WidgetBetGraphOthers *widget_bet_graph_others_new (BetGraphDataStruct *self,
												   const gchar *title,
												   gint origin_tab)
{
	WidgetBetGraphOthers *dialog;

	dialog = g_object_new (WIDGET_BET_GRAPH_OTHERS_TYPE, NULL);
	widget_bet_graph_others_setup_widget (dialog, self, title, origin_tab);

	return dialog;
}

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

