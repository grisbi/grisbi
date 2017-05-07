/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2017 Pierre Biava (grisbi@pierre.biava.name)                    */
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "csv_template_rule.h"
#include "structures.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _CsvTemplateRulePrivate   CsvTemplateRulePrivate;

struct _CsvTemplateRulePrivate
{
	GtkWidget *			vbox_csv_template;
};

G_DEFINE_TYPE_WITH_PRIVATE (CsvTemplateRule, csv_template_rule, GTK_TYPE_DIALOG)

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * Création de la page de gestion des xxxxx
 *
 * \param prefs
 *
 * \return
 */
static void csv_template_rule_setup_dialog (CsvTemplateRule *template_rule,
											GtkWidget *assistant)
{
	//~ GtkWidget *head_page;
	CsvTemplateRulePrivate *priv;

	devel_debug (NULL);

	priv = csv_template_rule_get_instance_private (template_rule);

	gtk_widget_show (priv->vbox_csv_template);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void csv_template_rule_init (CsvTemplateRule *template_rule)
{
	gtk_widget_init_template (GTK_WIDGET (template_rule));
}

static void csv_template_rule_dispose (GObject *object)
{
	G_OBJECT_CLASS (csv_template_rule_parent_class)->dispose (object);
}

static void csv_template_rule_class_init (CsvTemplateRuleClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = csv_template_rule_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/ui/csv_template_rule.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), CsvTemplateRule, vbox_csv_template);
	//~ gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), CsvTemplateRule, checkbutton_);
	//~ gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), CsvTemplateRule, eventbox_);
	//~ gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), CsvTemplateRule, spinbutton_);
	//~ gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), CsvTemplateRule, filechooserbutton_);
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
CsvTemplateRule *csv_template_rule_new (GtkWidget *assistant)
{
	CsvTemplateRule *template_rule;

	template_rule = g_object_new (CSV_TEMPLATE_RULE_TYPE, "transient-for", GTK_WINDOW (assistant), NULL);
	gtk_window_set_modal (GTK_WINDOW (template_rule), TRUE);

	csv_template_rule_setup_dialog (template_rule, assistant);

	return template_rule;
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

