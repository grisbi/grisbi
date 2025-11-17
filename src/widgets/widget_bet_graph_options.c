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
/*     along with this program; if not, write to the Free Software               */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                               */
/* *******************************************************************************/

#include "config.h"

#ifdef HAVE_GOFFICE

#include <errno.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

#include <goffice/goffice.h>

/*START_INCLUDE*/
#include "widget_bet_graph_options.h"
#include "bet_graph.h"
#include "gsb_file.h"
#include "utils_real.h"
#include "structures.h"
#include "widget_bet_graph_others.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _WidgetBetGraphOptionsPrivate	WidgetBetGraphOptionsPrivate;

struct _WidgetBetGraphOptionsPrivate
{
	GtkWidget *			vbox_bet_graph_options;
	GtkWidget *			vbox_options_col;
	GtkWidget *			vbox_rot_align;

	GtkWidget *			checkbutton_major_tick_out;
	GtkWidget *			checkbutton_major_tick_in;
	GtkWidget *			checkbutton_major_tick_labeled;

	GtkWidget *			radiobutton_axis_low;
	GtkWidget *			radiobutton_axis_high;
	GtkWidget *			radiobutton_axis_cross;
	GtkWidget *			checkbutton_new_axis_line;

	GtkWidget *			checkbutton_before_grid;
	GtkWidget *			spinbutton_gap_spinner;

	GtkWidget *			checkbutton_major_grid_y;
	GtkWidget *			checkbutton_minor_grid_y;
};

G_DEFINE_TYPE_WITH_PRIVATE (WidgetBetGraphOptions, widget_bet_graph_options, GTK_TYPE_BOX)
/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * callback
 *
 * \param
 * \param
 *
 * \return
 **/
static void widget_bet_graph_options_button_toggled (GtkToggleButton *togglebutton,
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
			if (active == FALSE)
			{
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), FALSE);
			}

			/* si le graphe est dans la bonne configuration on sort */
		if (prefs->major_grid_y == self->show_grid)
		{
			gsb_file_set_modified (TRUE);

			return;
		}
		/* on positionne le bouton self->button_show_grid */
		if (active)
			widget_bet_graph_others_show_grid_button_configure (self, TRUE, TRUE);
		else
			widget_bet_graph_others_show_grid_button_configure (self, FALSE, TRUE);
		break;
	case 9:
		prefs->minor_grid_y = active;
		break;
	}

	/* on met à jour le graph */
	widget_bet_graph_others_graph_update (self);

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
static void widget_bet_graph_options_spinbutton_value_changed (GtkSpinButton *spinbutton,
																BetGraphDataStruct *self)
{
	BetGraphPrefsStruct *prefs;

	prefs = self->prefs;
	prefs->gap_spinner = gtk_spin_button_get_value (spinbutton);

	/* on met à jour le graph */
	widget_bet_graph_others_graph_update (self);

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
static void widget_bet_graph_options_rotation_changed (GORotationSel *rotation,
													   int angle,
													   BetGraphDataStruct *self)
{
	BetGraphPrefsStruct *prefs;

	prefs = self->prefs;

	prefs->degrees = angle;

	/* on met à jour le graph */
	widget_bet_graph_others_graph_update (self);

	gsb_file_set_modified (TRUE);
}

/**
 * Création du widget bet_graph_options
 *
 * \param
 * \return
 **/
static void widget_bet_graph_options_setup_widget (WidgetBetGraphOptions *widget,
												   BetGraphDataStruct *self)
{
	GtkWidget *rotation;
	BetGraphPrefsStruct *prefs;
	WidgetBetGraphOptionsPrivate *priv;

	devel_debug (NULL);
	priv = widget_bet_graph_options_get_instance_private (widget);

	prefs = self->prefs;

	/* définition du type de graphique */
	if (strcmp (self->service_id, "GogLinePlot") == 0)
		prefs->type_graph = 1;
	else if (strcmp (self->service_id, "GogPiePlot") == 0)
		prefs->type_graph = -1;
	else
		prefs->type_graph = 0;

	/* configuration de l'axe X */
	/* configure les options d'affichage de l'axe X */
	if (prefs->major_tick_out)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_major_tick_out), TRUE);

	g_object_set_data (G_OBJECT (priv->checkbutton_major_tick_out), "rang", GINT_TO_POINTER (0));
	g_signal_connect (priv->checkbutton_major_tick_out,
					  "toggled",
					  G_CALLBACK (widget_bet_graph_options_button_toggled),
					  self);

	if (prefs->major_tick_in)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_major_tick_in), TRUE);

	g_object_set_data (G_OBJECT (priv->checkbutton_major_tick_in), "rang", GINT_TO_POINTER (1));
	g_signal_connect (priv->checkbutton_major_tick_in,
					  "toggled",
					  G_CALLBACK (widget_bet_graph_options_button_toggled),
					  self);

	if (prefs->major_tick_labeled)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_major_tick_labeled), TRUE);

	g_object_set_data (G_OBJECT (priv->checkbutton_major_tick_labeled), "rang", GINT_TO_POINTER (2));
	g_signal_connect (priv->checkbutton_major_tick_labeled,
					  "toggled",
					  G_CALLBACK (widget_bet_graph_options_button_toggled),
					  self);

	/* Configuration de la position de l'axe des X */
	if (prefs->position == 0)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radiobutton_axis_low), TRUE);
	g_object_set_data (G_OBJECT (priv->radiobutton_axis_low), "rang", GINT_TO_POINTER (3));

	if (prefs->position == 1)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radiobutton_axis_high), TRUE);
	g_object_set_data (G_OBJECT (priv->radiobutton_axis_high), "rang", GINT_TO_POINTER (4));

	if (prefs->position == 2)
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radiobutton_axis_cross), TRUE);
		gtk_widget_set_sensitive (priv->checkbutton_new_axis_line, FALSE);
	}
	g_object_set_data (G_OBJECT (priv->radiobutton_axis_cross), "rang", GINT_TO_POINTER (5));

	if (prefs->new_axis_line)
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_new_axis_line), TRUE);
		gtk_widget_set_sensitive (priv->radiobutton_axis_cross, FALSE);
	}
	g_object_set_data (G_OBJECT (priv->checkbutton_new_axis_line), "rang", GINT_TO_POINTER (6));

	/* on lie les boutons priv->checkbutton_new_axis_line et priv->radiobutton_axis_cross */
	/* pour les retrouver plus tard */
	g_object_set_data (G_OBJECT (priv->checkbutton_new_axis_line), "other_axis", priv->radiobutton_axis_cross);
	g_object_set_data (G_OBJECT (priv->radiobutton_axis_cross), "other_axis", priv->checkbutton_new_axis_line);

	/* set the signal */
	g_signal_connect (priv->radiobutton_axis_low,
					  "toggled",
					  G_CALLBACK (widget_bet_graph_options_button_toggled),
					  self);

	g_signal_connect (priv->radiobutton_axis_high,
					  "toggled",
					  G_CALLBACK (widget_bet_graph_options_button_toggled),
					  self);

	g_signal_connect (priv->radiobutton_axis_cross,
					  "toggled",
					  G_CALLBACK (widget_bet_graph_options_button_toggled),
					  self);

	g_signal_connect (priv->checkbutton_new_axis_line,
					  "toggled",
					  G_CALLBACK (widget_bet_graph_options_button_toggled),
					  self);

	/* configure l'orientation des étiquettes de l'axe X */
	rotation = go_rotation_sel_new ();
	go_rotation_sel_set_rotation (GO_ROTATION_SEL (rotation), prefs->degrees);
	g_signal_connect (G_OBJECT (rotation),
					  "rotation-changed",
					  G_CALLBACK (widget_bet_graph_options_rotation_changed),
					  self);

	gtk_container_add (GTK_CONTAINER (priv->vbox_rot_align), rotation);

	if (prefs->type_graph == 0)
	{
        /* affichage de la grille au dessus ou en dessous des barres */
        /* sert aussi à afficher les étiquettes au dessus des barres à cause d'un bug de goffice */
		g_object_set_data (G_OBJECT (priv->checkbutton_before_grid), "rang", GINT_TO_POINTER (7));
        if (prefs->before_grid)
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_before_grid), TRUE);
		g_signal_connect (priv->checkbutton_before_grid,
						  "toggled",
						  G_CALLBACK (widget_bet_graph_options_button_toggled),
						  self);

		/* configure la valeur de la largeur des colonnes du graph */
		if (prefs->gap_spinner)
			gtk_spin_button_set_value (GTK_SPIN_BUTTON (priv->spinbutton_gap_spinner), prefs->gap_spinner);
		g_signal_connect (priv->spinbutton_gap_spinner,
						  "value-changed",
						  G_CALLBACK (widget_bet_graph_options_spinbutton_value_changed),
						  self);
		gtk_widget_show (priv->vbox_options_col);
	}
	else
		gtk_widget_hide (priv->vbox_options_col);

    /* configuration de l'axe Y */
    /* configuration de la grille */
    if (prefs->major_grid_y)
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_major_grid_y), TRUE);
    else
        gtk_widget_set_sensitive (priv->checkbutton_minor_grid_y, FALSE);
    g_object_set_data (G_OBJECT (priv->checkbutton_major_grid_y), "rang", GINT_TO_POINTER (8));

    if (prefs->minor_grid_y)
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_minor_grid_y), TRUE);
    g_object_set_data (G_OBJECT (priv->checkbutton_minor_grid_y), "rang", GINT_TO_POINTER (9));

	/* on lie les deux boutons pour les retrouver plus tard */
    g_object_set_data (G_OBJECT (priv->checkbutton_major_grid_y), "grid_y", priv->checkbutton_minor_grid_y);

    /* set the signal */
    g_signal_connect (priv->checkbutton_major_grid_y,
					  "toggled",
					  G_CALLBACK (widget_bet_graph_options_button_toggled),
					  self);
    g_signal_connect (priv->checkbutton_minor_grid_y,
					  "toggled",
					  G_CALLBACK (widget_bet_graph_options_button_toggled),
					  self);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void widget_bet_graph_options_init (WidgetBetGraphOptions *widget)
{
	gtk_widget_init_template (GTK_WIDGET (widget));
}

static void widget_bet_graph_options_dispose (GObject *object)
{
	G_OBJECT_CLASS (widget_bet_graph_options_parent_class)->dispose (object);
}

static void widget_bet_graph_options_class_init (WidgetBetGraphOptionsClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = widget_bet_graph_options_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/widgets/widget_bet_graph_options.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  WidgetBetGraphOptions,
												  vbox_bet_graph_options);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetBetGraphOptions, vbox_options_col);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetBetGraphOptions, vbox_rot_align);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  WidgetBetGraphOptions,
												  checkbutton_major_tick_out);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  WidgetBetGraphOptions,
												  checkbutton_major_tick_in);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  WidgetBetGraphOptions,
												  checkbutton_major_tick_labeled);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  WidgetBetGraphOptions,
												  radiobutton_axis_low);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  WidgetBetGraphOptions,
												  radiobutton_axis_high);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  WidgetBetGraphOptions,
												  radiobutton_axis_cross);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  WidgetBetGraphOptions,
												  checkbutton_new_axis_line);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  WidgetBetGraphOptions,
												  checkbutton_before_grid);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  WidgetBetGraphOptions,
												  spinbutton_gap_spinner);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  WidgetBetGraphOptions,
												  checkbutton_major_grid_y);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  WidgetBetGraphOptions,
												  checkbutton_minor_grid_y);
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
WidgetBetGraphOptions *widget_bet_graph_options_new (BetGraphDataStruct *self)
{
	WidgetBetGraphOptions *widget;

	widget = g_object_new (WIDGET_BET_GRAPH_OPTIONS_TYPE, NULL);
	widget_bet_graph_options_setup_widget (widget, self);

	return widget;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void widget_bet_graph_options_set_button_major_grid_y (GtkWidget *widget,
													   BetGraphDataStruct *self,
													   gboolean active)
{
	WidgetBetGraphOptionsPrivate *priv;

	priv = widget_bet_graph_options_get_instance_private (WIDGET_BET_GRAPH_OPTIONS (widget));

	/* on bloque le signal */
	g_signal_handlers_block_by_func (G_OBJECT (priv->checkbutton_major_grid_y),
									 widget_bet_graph_options_button_toggled,
									 self);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_major_grid_y), active);
	if (!active)
	{
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->checkbutton_minor_grid_y)))
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_minor_grid_y), FALSE);
		gtk_widget_set_sensitive (priv->checkbutton_minor_grid_y, FALSE);
	}

		/* on débloque le signal */
		g_signal_handlers_unblock_by_func (G_OBJECT (priv->checkbutton_major_grid_y),
										   widget_bet_graph_options_button_toggled,
										   self);

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
