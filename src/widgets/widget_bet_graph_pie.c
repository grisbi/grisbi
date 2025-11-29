/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2022 Pierre Biava (grisbi@pierre.biava.name)                    */
/*          https://www.grisbi.org                                                */
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
/*     along with this program; if not, see <https://www.gnu.org/licenses/>.     */
/*                                                                               */
/* *******************************************************************************/

#include "config.h"

#ifdef HAVE_GOFFICE

#include <glib/gi18n.h>

#include <goffice/goffice.h>

/*START_INCLUDE*/
#include "widget_bet_graph_pie.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "gsb_data_account.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_file.h"
#include "structures.h"
#include "utils_real.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

/*START_STATIC*/
 static gboolean widget_bet_graph_pie_right_button_press (GtkWidget *dialog,
														 GdkEventButton  *event,
														 BetGraphDataStruct *self);
/*END_STATIC*/

typedef struct _WidgetBetGraphPiePrivate   WidgetBetGraphPiePrivate;

struct _WidgetBetGraphPiePrivate
{
	GtkWidget *			vbox_bet_graph_pie;
	GtkWidget *			label_bet_graph_pie;
	GtkWidget *			notebook_bet_graph_pie;
};

G_DEFINE_TYPE_WITH_PRIVATE (WidgetBetGraphPie, widget_bet_graph_pie, GTK_TYPE_DIALOG)
/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * affiche un graphique de type camembert joint
 *
 * \param BetGraphDataStruct	structure qui contient les données à afficher
 *
 * \return 						TRUE if OK FALSE otherwise
 **/
static gboolean widget_bet_graph_pie_affiche_camemberts (BetGraphDataStruct *self)
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
 * callback qui gère l'affichage des tip
 *
 * \param
 * \param
 * \param
 *
 * \return TRUE to stop the signal emission
 **/
static gboolean widget_bet_graph_pie_motion_notify_event (GtkWidget *event_box,
														  GdkEventMotion *event,
														  BetGraphDataStruct *self)
{
	GogRenderer *rend = NULL;
	GogView *graph_view = NULL;
	GogView *view = NULL;
	GogSeries *series;
	gchar *buf = NULL;
	gint index;

	rend = go_graph_widget_get_renderer (GO_GRAPH_WIDGET (self->widget));
	g_object_get (G_OBJECT (rend), "view", &graph_view, NULL);
	view = gog_view_find_child_view (graph_view, GOG_OBJECT (self->plot));

	index = gog_plot_view_get_data_at_point (GOG_PLOT_VIEW (view), event->x, event->y, &series);
	if (index != -1)
	{
		double const *x;
		double const *y;
		gchar *tmp_str;

		gog_series_get_xy_data (series, &x, &y);
		tmp_str = utils_real_get_string_with_currency_from_double (y[index], self->currency_number);
		buf = g_strdup_printf ("<markup><span font_weight=\"bold\">%s</span>\n%s (%.2f%%)</markup>",
							   self->tab_vue_libelle[index],
							   tmp_str,
							   100*y[index]/self->montant);
		g_free(tmp_str);
	}

	gtk_widget_set_tooltip_markup (GTK_WIDGET (self->widget), buf);
	g_free (buf);

	return TRUE;
}

/**
 * affiche les sous divisions d'un camembert
 *
 * \param parent			BetGraphDataStruct du parent
 * \param div_number		numéro de la division concernée
 * \param bet_hist_data		type de données : catégorie ou IB
 * \param div_name			nom de la catégorie ou de l'IB
 *
 * \return
 **/
static void widget_bet_graph_pie_affiche_sub_divisions (BetGraphDataStruct *self_parent,
														gint div_number,
														gint bet_hist_data,
														gchar *div_name,
														gchar *total_div)
{
	GtkWidget *dialog;
	GtkWidget *notebook;
	gchar *title;
	BetGraphDataStruct *self;

	/* get the title */
	if (bet_hist_data)
		title = g_markup_printf_escaped (_("Display subdivisions items of budgetary: %s"), div_name);
	else
		title = g_markup_printf_escaped (_("Display subdivisions of category: %s"), div_name);

	/* initialisation de la structure des données */
	self = struct_initialise_bet_graph_data ();
	self->notebook = GTK_NOTEBOOK (self_parent->notebook);
	self->tree_view = self_parent->tree_view;
	self->account_number = self_parent->account_number;
	self->currency_number = self_parent->currency_number;
	self->service_id = g_strdup (self_parent->service_id);
	self->type_infos = self_parent->type_infos;
	self->title = g_strconcat (_("Total amount: "), total_div, NULL);
	self->is_legend = TRUE;

	/* get dialog window */
	dialog = GTK_WIDGET (widget_bet_graph_pie_new (self, title));

	/* on recupere le notebook pour plus tard */
	notebook = widget_bet_graph_pie_get_notebook (dialog);
	self->notebook = GTK_NOTEBOOK (notebook);

	/* create and display graph for credit */
	widget_bet_graph_pie_display_graph (self, TRUE, div_number);

	/* on interdit le clic droit */
	g_signal_handlers_block_by_func (self->widget,
									 G_CALLBACK (widget_bet_graph_pie_right_button_press),
									 self);

	/* show widgets */
	gtk_widget_show_all (dialog);

	gtk_dialog_run (GTK_DIALOG (dialog));

	/* free the data */
	struct_free_bet_graph_data (self);

	gtk_widget_destroy (dialog);
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
static gboolean widget_bet_graph_pie_right_button_press (GtkWidget *dialog,
														 GdkEventButton  *event,
														 BetGraphDataStruct *self)
{

	if (event->type == GDK_BUTTON_PRESS
		&& event->button == 3 )
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

		total_div = utils_real_get_string_with_currency_from_double (self->tab_Y[index],
																	 self->currency_number);

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
			widget_bet_graph_pie_affiche_sub_divisions (self,
														div_number,
														bet_hist_data, self->tab_vue_libelle[index],
														total_div);
		else
		{
			GtkWidget *msg_dialog;
			GtkWidget *parent_dialog;
			gchar *msg;

			if (bet_hist_data)
				msg = g_strdup (_("The selected budget item has no subitems"));
			else
				msg = g_strdup (_("The selected category has no subcategory"));

			parent_dialog = g_object_get_data (G_OBJECT (self->notebook), "dialog");
			msg_dialog = gtk_message_dialog_new (GTK_WINDOW (parent_dialog),
												 GTK_DIALOG_DESTROY_WITH_PARENT,
												 GTK_MESSAGE_INFO,
												 GTK_BUTTONS_CLOSE,
												 "%s",
												 msg);

			gtk_window_set_transient_for (GTK_WINDOW (msg_dialog), GTK_WINDOW (parent_dialog));
			gtk_dialog_run (GTK_DIALOG (msg_dialog));
			gtk_widget_destroy (msg_dialog);

			g_free (msg);
		}
		g_free (total_div);
	}

	/* return */
	return TRUE;
}

/**
 * Création du widget bet_graph_pie
 *
 * \param
 * \return
 **/
static void widget_bet_graph_pie_setup_widget (WidgetBetGraphPie *dialog,
											   BetGraphDataStruct *self,
											   const gchar *title)
{
	gchar *tmp_str;
	WidgetBetGraphPiePrivate *priv;

	devel_debug (NULL);
	priv = widget_bet_graph_pie_get_instance_private (dialog);

	/* set title */
	tmp_str = dialogue_make_pango_attribut ("weight=\"bold\" size=\"x-large\"", title);
	gtk_label_set_markup (GTK_LABEL (priv->label_bet_graph_pie), tmp_str);
	g_free (tmp_str);

	/* attache dialog au notebook */
	g_object_set_data (G_OBJECT (priv->notebook_bet_graph_pie), "dialog", dialog);

	/* set signal */
	g_signal_connect (G_OBJECT (dialog),
					  "destroy",
					  G_CALLBACK (gtk_widget_destroy),
					  NULL);

	/* si self est définie c'est qu'on affiche des sous-catégories ou des sous-IB */
	if (self)
	{
		GtkWidget *dialog_parent;

		dialog_parent = g_object_get_data (G_OBJECT (self->notebook), "dialog");
		gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (dialog_parent));

		gtk_widget_set_size_request (GTK_WIDGET (dialog), 600, 400);
	}
	else
	{
		gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (grisbi_app_get_active_window (NULL)));
		gtk_widget_set_size_request (GTK_WIDGET (dialog), PAGE_WIDTH, PAGE_HEIGHT);
	}
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void widget_bet_graph_pie_init (WidgetBetGraphPie *dialog)
{
	gtk_widget_init_template (GTK_WIDGET (dialog));
}

static void widget_bet_graph_pie_dispose (GObject *object)
{
	G_OBJECT_CLASS (widget_bet_graph_pie_parent_class)->dispose (object);
}

static void widget_bet_graph_pie_class_init (WidgetBetGraphPieClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = widget_bet_graph_pie_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/widgets/widget_bet_graph_pie.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetBetGraphPie, vbox_bet_graph_pie);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetBetGraphPie, label_bet_graph_pie);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetBetGraphPie, notebook_bet_graph_pie);
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 * c'est juste un container pour un graphique de type camembert
 *
 * \param
 *
 * \return
 **/
WidgetBetGraphPie *widget_bet_graph_pie_new (BetGraphDataStruct *self,
											 const gchar *title)
{
	WidgetBetGraphPie *dialog;

	dialog = g_object_new (WIDGET_BET_GRAPH_PIE_TYPE, NULL);
	widget_bet_graph_pie_setup_widget (dialog, self, title);

	return dialog;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
GtkWidget *widget_bet_graph_pie_get_notebook (GtkWidget *dialog)
{
	WidgetBetGraphPiePrivate *priv;

	priv = widget_bet_graph_pie_get_instance_private (WIDGET_BET_GRAPH_PIE (dialog));

	return priv->notebook_bet_graph_pie;
}

/**
 * Création de la page pour le graphique initialisé
 *
 * \param
 * \param display_sub_div	TRUE si affichage des sous-divisions
 *
 * \return
 **/
 void widget_bet_graph_pie_display_graph (BetGraphDataStruct *self,
										  gboolean display_sub_div,
										  gint div_number)
{
	GtkWidget *child;
	GtkWidget *w;
	GogGraph *graph;
	GogLabel *label;
	GOStyle *style;
	GOData *data;
	gboolean result = FALSE;
	PangoFontDescription *desc;

	child = gtk_notebook_get_nth_page (GTK_NOTEBOOK (self->notebook), 0);

	/* Set the graph widget */
	w = go_graph_widget_new (NULL);
	g_signal_connect (G_OBJECT (w),
					  "motion-notify-event",
					  G_CALLBACK (widget_bet_graph_pie_motion_notify_event),
					  self);

	g_signal_connect (G_OBJECT (w),
					  "button-press-event",
					  G_CALLBACK (widget_bet_graph_pie_right_button_press),
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
	self->plot = (GogPlot *) gog_plot_new_by_name (self->service_id);
	gog_object_add_by_name (GOG_OBJECT (self->chart), "Plot", GOG_OBJECT (self->plot));

	/* Add a legend to the chart */
	if (self->is_legend)
		gog_object_add_by_name (GOG_OBJECT (self->chart), "Legend", NULL);

	/* populate the data */
	if (self->valid_data == FALSE)
	 {
		if (display_sub_div)
			self->valid_data = bet_graph_populate_sectors_by_sub_divisions (self, div_number);
		else
			self->valid_data = bet_graph_populate_sectors_by_hist_data (self);
	}

	 /* show the data */
	if (self->valid_data)
		result = widget_bet_graph_pie_affiche_camemberts (self);

	if (!result)
		gtk_widget_hide (self->widget);
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

#endif
