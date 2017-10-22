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
#include "dialog.h"
#include "import_csv.h"
#include "structures.h"
#include "utils_prefs.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

static CSVImportRule *csv_rule;

typedef struct _CsvTemplateRulePrivate	CsvTemplateRulePrivate;

struct _CsvTemplateRulePrivate
{
	GtkWidget *		assistant;

	GtkWidget *		dialog_csv_template;

	GtkWidget *		entry_csv_rule_name;

	GtkWidget *		checkbutton_csv_account_id;
	GtkWidget *		eventbox_csv_account_id;
	GtkWidget *		spinbutton_csv_account_id_col;
	GtkWidget *		spinbutton_csv_account_id_row;
	GtkWidget *		checkbutton_csv_header_col;
	GtkWidget *		eventbox_csv_header_col;
	GtkWidget *		spinbutton_csv_first_line;

	GtkWidget *		checkbutton_csv_spec;
	GtkWidget *		combobox_csv_spec_col_data;
	GtkWidget *		combobox_csv_spec_action;
	GtkWidget *		combobox_csv_spec_col_montant;
	GtkWidget *		entry_csv_spec_text;
};

G_DEFINE_TYPE_WITH_PRIVATE (CsvTemplateRule, csv_template_rule, GTK_TYPE_DIALOG)

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 *
 *
 * \param
 * \param
 * \param
 * \param
 *
 * \return
 **/
static void csv_template_rule_spec_conf_entry_deleted (GtkEditable *entry,
													   guint position,
													   guint n_chars,
													   GtkWidget *dialog)
{
	devel_debug (NULL);

	if (gtk_entry_get_text_length (GTK_ENTRY (entry)) == 1)
		gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_APPLY, FALSE);
}

/**
 *
 *
 * \param
 * \param
 * \param
 * \param
 * \param
 *
 * \return
 **/
static void csv_template_rule_spec_conf_entry_inserted (GtkEditable *entry,
													    guint position,
													    gchar *chars,
													    guint n_chars,
													    GtkWidget *dialog)
{
	CsvTemplateRulePrivate *priv;

	devel_debug (NULL);

	priv = csv_template_rule_get_instance_private ((CsvTemplateRule *) dialog);

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->checkbutton_csv_spec)))
	{
		if (gtk_entry_get_text_length (GTK_ENTRY (priv->entry_csv_rule_name)) > 1
			&&
			gtk_entry_get_text_length (GTK_ENTRY (priv->entry_csv_spec_text)) > 1)
		{
			gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_APPLY, TRUE);
		}
	}
	else
	{
		if (gtk_entry_get_text_length (GTK_ENTRY (priv->entry_csv_rule_name)) > 1)
		{
			gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_APPLY, TRUE);
		}
	}
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void csv_template_rule_spec_conf_set_actions (GtkComboBoxText *combobox)
{
	gtk_combo_box_text_append ((GtkComboBoxText *) combobox, NULL, "Skip lines");
	gtk_combo_box_text_append ((GtkComboBoxText *) combobox, NULL, "Invert the amount");
	gtk_combo_box_set_active ((GtkComboBox *) combobox,0);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void csv_template_rule_checkbutton_account_changed (GtkToggleButton *checkbutton,
														   CsvTemplateRule *template_rule)
{
	gboolean checked;
	CsvTemplateRulePrivate *priv;

	devel_debug (NULL);

	priv = csv_template_rule_get_instance_private (template_rule);

	checked = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkbutton));

	if (checked)
	{
		gtk_widget_set_sensitive (GTK_WIDGET (priv->spinbutton_csv_account_id_col), TRUE);
		gtk_widget_set_sensitive (GTK_WIDGET (priv->spinbutton_csv_account_id_row), TRUE);
	}
	else
	{
		gtk_widget_set_sensitive (GTK_WIDGET (priv->spinbutton_csv_account_id_col), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET (priv->spinbutton_csv_account_id_row), FALSE);
	}
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void csv_template_rule_checkbutton_spec_conf_changed (GtkToggleButton *checkbutton,
															 CsvTemplateRule *template_rule)
{
	CsvTemplateRulePrivate *priv;

	devel_debug (NULL);

	priv = csv_template_rule_get_instance_private (template_rule);

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkbutton)))
	{
		gtk_widget_set_sensitive (GTK_WIDGET (priv->combobox_csv_spec_action), TRUE);
		gtk_widget_set_sensitive (GTK_WIDGET (priv->combobox_csv_spec_col_data), TRUE);
		gtk_widget_set_sensitive (GTK_WIDGET (priv->entry_csv_spec_text), TRUE);

		/* changement d'état du bouton Ajouter règle*/
		if (gtk_entry_get_text_length (GTK_ENTRY (priv->entry_csv_spec_text)) == 0)
			gtk_dialog_set_response_sensitive (GTK_DIALOG (template_rule), GTK_RESPONSE_APPLY, FALSE);
	}
	else
	{
		gtk_widget_set_sensitive (GTK_WIDGET (priv->combobox_csv_spec_action), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET (priv->combobox_csv_spec_col_data), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET (priv->entry_csv_spec_text), FALSE);
		if (gtk_entry_get_text_length (GTK_ENTRY (priv->entry_csv_rule_name)) > 0)
			gtk_dialog_set_response_sensitive (GTK_DIALOG (template_rule), GTK_RESPONSE_APPLY, TRUE);
	}
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void csv_template_rule_combobox_spec_conf_action_changed (GtkComboBox *widget,
																 CsvTemplateRule *template_rule)
{
	gint action;
	CsvTemplateRulePrivate *priv;

	devel_debug (NULL);

	priv = csv_template_rule_get_instance_private (template_rule);
	action = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
	if (action == 1)
	{
		gtk_widget_set_sensitive (GTK_WIDGET (priv->combobox_csv_spec_col_montant), TRUE);
	}
	else
	{
		gtk_widget_set_sensitive (GTK_WIDGET (priv->combobox_csv_spec_col_montant), FALSE);
	}
}

/**
 * callback pour la fermeture de la fenêtre
 *
 * \param GtkDialog		dialog
 * \param gint			result_id
 *
 * \return
 **/
static void csv_template_dialog_response  (GtkDialog *dialog,
										   gint result_id)
{
	CsvTemplateRulePrivate *priv;

	devel_debug_int (result_id);
	if (!dialog)
	{
		return;
	}

	priv = csv_template_rule_get_instance_private ((CsvTemplateRule *) dialog);
	switch (result_id)
	{
		case GTK_RESPONSE_APPLY:
			csv_rule = g_malloc0 (sizeof (CSVImportRule));

			/* name of rule */
			csv_rule->csv_rule_name = g_strdup (gtk_entry_get_text (GTK_ENTRY (priv->entry_csv_rule_name)));

			/* account number */
			if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->checkbutton_csv_account_id)))
			{
				csv_rule->csv_account_id_col = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (priv->spinbutton_csv_account_id_col));
				csv_rule->csv_account_id_row = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (priv->spinbutton_csv_account_id_row));
			}

			/* data of CSV file */
			csv_rule->csv_headers_present = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->checkbutton_csv_header_col));
			csv_rule->csv_first_line_data = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (priv->spinbutton_csv_first_line));

			/* specific configuration */
			if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->checkbutton_csv_spec)))
			{
				GtkWidget *widget;
				SpecConfData *spec_conf_data;

				spec_conf_data = g_malloc0 (sizeof (SpecConfData));
				widget = priv->combobox_csv_spec_action;
				spec_conf_data->combobox_action = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
				if (spec_conf_data->combobox_action == 1)
				{
					widget = priv->combobox_csv_spec_col_montant;
					spec_conf_data->combobox_amount_col = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
				}
				widget = priv->combobox_csv_spec_col_data;
				spec_conf_data->combobox_text_col = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
				widget = priv->entry_csv_spec_text;
				spec_conf_data->entry_text_str = g_strdup (gtk_entry_get_text (GTK_ENTRY (widget)));
				csv_rule->csv_spec_conf_data = spec_conf_data;
			}
				g_object_set_data (G_OBJECT (dialog), "csv-import-rule", csv_rule);
			break;
		case GTK_RESPONSE_CANCEL:
			break;
	}
}

/**
 * Création de la page de définition de la nouvelle règle
 *
 * \param CsvTemplateRule		object
 * \param GtkWidget 			assistant
 *
 * \return
 */
static void csv_template_rule_setup_dialog (CsvTemplateRule *template_rule,
											GtkWidget *assistant)
{
	GSList *csv_file_columns;
	GSList *list;
	gint csv_first_line_data;
	CsvTemplateRulePrivate *priv;

	devel_debug (NULL);

	priv = csv_template_rule_get_instance_private (template_rule);

	priv->assistant = assistant;

	/* Rule_name */
	gtk_widget_set_name (GTK_WIDGET (priv->entry_csv_rule_name), "entry_csv_rule_name");

	/* account number*/
	gtk_widget_set_sensitive (GTK_WIDGET (priv->spinbutton_csv_account_id_col), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (priv->spinbutton_csv_account_id_row), FALSE);

	/* Connect signal */
    g_signal_connect (priv->eventbox_csv_account_id,
					  "button-press-event",
					  G_CALLBACK (utils_prefs_page_eventbox_clicked),
					  priv->checkbutton_csv_account_id);

	g_signal_connect (priv->checkbutton_csv_account_id,
					  "toggled",
					  G_CALLBACK (csv_template_rule_checkbutton_account_changed),
					  template_rule);

	/* spinbutton_csv_first_line */
	csv_first_line_data =  GPOINTER_TO_INT (g_object_get_data (G_OBJECT(assistant), "csv_first_line_data"));
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (priv->spinbutton_csv_first_line), csv_first_line_data);

	/* Connect signal */
    g_signal_connect (priv->eventbox_csv_header_col,
					  "button-press-event",
					  G_CALLBACK (utils_prefs_page_eventbox_clicked),
					  priv->checkbutton_csv_header_col);

	/* specific configuration */
	csv_file_columns = csv_import_get_columns_list (assistant);
	list = csv_file_columns;
	while (list)
	{
		//~ printf ("colonne = %s\n", (gchar *) list->data);
		gtk_combo_box_text_append ((GtkComboBoxText *) priv->combobox_csv_spec_col_data,
								   NULL,
								   (const gchar *) list->data);
		gtk_combo_box_text_append ((GtkComboBoxText *) priv->combobox_csv_spec_col_montant,
								   NULL,
								   (const gchar *) list->data);

		list = list->next;
	}
	gtk_combo_box_set_active ((GtkComboBox *) priv->combobox_csv_spec_col_data, 0);

	g_object_set_data (G_OBJECT (priv->checkbutton_csv_spec), "spec-conf", GINT_TO_POINTER (0));
	g_signal_connect (priv->checkbutton_csv_spec,
					  "toggled",
					  G_CALLBACK (csv_template_rule_checkbutton_spec_conf_changed),
					  template_rule);

	csv_template_rule_spec_conf_set_actions ((GtkComboBoxText *) priv->combobox_csv_spec_action);
	g_signal_connect (priv->combobox_csv_spec_action,
					  "changed",
					  G_CALLBACK (csv_template_rule_combobox_spec_conf_action_changed),
					  template_rule);
	gtk_combo_box_set_active ((GtkComboBox *) priv->combobox_csv_spec_col_montant, 0);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void csv_template_rule_init (CsvTemplateRule *template_rule)
{
	gtk_widget_init_template (GTK_WIDGET (template_rule));

	/* Add action buttons */
	gtk_dialog_set_response_sensitive (GTK_DIALOG (template_rule), GTK_RESPONSE_APPLY, FALSE);
}

static void csv_template_rule_dispose (GObject *object)
{
	//~ CsvTemplateRulePrivate *priv;

	//~ priv = csv_template_rule_get_instance_private ((CsvTemplateRule *) object);

	G_OBJECT_CLASS (csv_template_rule_parent_class)->dispose (object);
}

static void csv_template_rule_class_init (CsvTemplateRuleClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = csv_template_rule_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/ui/csv_template_rule.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), CsvTemplateRule, checkbutton_csv_account_id);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), CsvTemplateRule, checkbutton_csv_header_col);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), CsvTemplateRule, entry_csv_rule_name);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), CsvTemplateRule, eventbox_csv_account_id);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), CsvTemplateRule, spinbutton_csv_account_id_col);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), CsvTemplateRule, spinbutton_csv_account_id_row);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), CsvTemplateRule, eventbox_csv_header_col);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), CsvTemplateRule, spinbutton_csv_first_line);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), CsvTemplateRule, checkbutton_csv_spec);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), CsvTemplateRule, combobox_csv_spec_action);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), CsvTemplateRule, combobox_csv_spec_col_data);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), CsvTemplateRule, combobox_csv_spec_col_montant);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), CsvTemplateRule, entry_csv_spec_text);

	/* signaux */
    gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (klass), csv_template_dialog_response);
	gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (klass), csv_template_rule_spec_conf_entry_deleted);
	gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (klass), csv_template_rule_spec_conf_entry_inserted);
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
void csv_template_rule_csv_rule_struct_free	(CSVImportRule *csv_rule)
{
	SpecConfData *spec_conf_data;

	g_free (csv_rule->csv_rule_name);

	spec_conf_data = csv_rule->csv_spec_conf_data;
	if (spec_conf_data)
	{
		g_free (spec_conf_data->entry_text_str);
		g_free (spec_conf_data);
	}

	g_free (csv_rule);
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

