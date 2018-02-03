/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2018 Pierre Biava (grisbi@pierre.biava.name)                    */
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
#include "gsb_data_import_rule.h"
#include "import_csv.h"
#include "structures.h"
#include "utils_buttons.h"
#include "utils_prefs.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

static CSVImportRule *csv_rule;

typedef struct _CsvTemplateRulePrivate	CsvTemplateRulePrivate;
typedef struct _SpecWidgetLine			SpecWidgetLine;

struct _CsvTemplateRulePrivate
{
	GtkWidget *		dialog_csv_template;

	GtkWidget *		entry_csv_rule_name;
	GtkWidget *		checkbutton_csv_account_id;
	GtkWidget *		eventbox_csv_account_id;
	GtkWidget *		spinbutton_csv_account_id_col;
	GtkWidget *		spinbutton_csv_account_id_row;
	GtkWidget *		checkbutton_csv_header_col;
	GtkWidget *		eventbox_csv_header_col;
	GtkWidget *		spinbutton_csv_first_line;

	GtkWidget *		button_csv_spec_add_line;			/* button add a new condition */
	GtkWidget *		notebook_csv_spec;

	GSList *		list_spec_lines;					/* Liste des lignes des actions spécifiques */
	gchar *			combobox_cols_name;					/* noms des colonnes du fichier importé */
	gboolean		edit_modif;							/* Vrai si le mode edit a été modifié */
};

G_DEFINE_TYPE_WITH_PRIVATE (CsvTemplateRule, csv_template_rule, GTK_TYPE_DIALOG)

/* contient les widgets des lignes d'actions spécifiques */
struct _SpecWidgetLine
{
	gint			index;
	GtkWidget *		grid;
	GtkWidget *		checkbutton;
	GtkWidget *		combobox_action;
	GtkWidget *		combobox_used_data;
	GtkWidget *		combobox_action_data;
	GtkWidget *		entry_used_text;
};

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * autorise le changement du bouton modifier après la première modification
 *
 * \param
 * \param
 *
 * \return
 **/
static void csv_template_rule_spec_conf_edit_widget_changed (GtkWidget *widget,
															 CsvTemplateRule *template_rule)
{
	CsvTemplateRulePrivate *priv;

	priv = csv_template_rule_get_instance_private ((CsvTemplateRule *) template_rule);
	if (!priv->edit_modif)
	{
		priv->edit_modif = TRUE;
		gtk_dialog_set_response_sensitive (GTK_DIALOG (template_rule), GSB_RESPONSE_EDIT, TRUE);
	}
}

/**
 * consiste à remettre en phase l'index de la structure avec la position des onglets
 *
 * \param
 * \param
 *
 * \return
 **/
static void csv_template_rule_notebook_tab_renumber_pages (GtkWidget *notebook,
														   gint page_removed,
														   CsvTemplateRule *template_rule)
{
	GSList *list;
	gint index = 1;
	CsvTemplateRulePrivate *priv;

	priv = csv_template_rule_get_instance_private (template_rule);

	list = priv->list_spec_lines;
	while (list)
	{
		SpecWidgetLine *line_struct;

		line_struct = (SpecWidgetLine *) list->data;
		if (line_struct->index > page_removed + 1)
		{
			GtkWidget *tab_label;
			gpointer pointer;
			gchar *tmp_str;

			/* set the new index */
			line_struct->index = index;
			pointer = GINT_TO_POINTER (line_struct->index);
			g_object_set_data (G_OBJECT (line_struct->checkbutton), "index", pointer);
			g_object_set_data (G_OBJECT (line_struct->entry_used_text), "index", pointer);
			g_object_set_data (G_OBJECT (line_struct->combobox_action), "index", pointer);

			/* update tab_label */
			tab_label = gtk_notebook_get_tab_label (GTK_NOTEBOOK (priv->notebook_csv_spec), line_struct->grid);
			tmp_str = g_strdup_printf("Condition %d", line_struct->index);
			gtk_label_set_text (GTK_LABEL (tab_label), tmp_str);
			g_free (tmp_str);
		}
		index++;
		list = list->next;
	}
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void csv_template_rule_notebook_tab_close_button_clicked (GtkButton *button,
																 CsvTemplateRule *template_rule)
{
	gint page_num;
	gint nbre_pages;
	gpointer line_struct;
	CsvTemplateRulePrivate *priv;

	priv = csv_template_rule_get_instance_private (template_rule);
	page_num = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (button), "page_num"));
	line_struct = g_slist_nth_data (priv->list_spec_lines, page_num);
	if (line_struct)
		priv->list_spec_lines = g_slist_remove (priv->list_spec_lines, line_struct);
	nbre_pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (priv->notebook_csv_spec));
	gtk_notebook_remove_page (GTK_NOTEBOOK (priv->notebook_csv_spec), page_num);
	gtk_widget_set_sensitive (GTK_WIDGET (priv->button_csv_spec_add_line), TRUE);

	/* on renumerote les lignes si ce n'est pas le dernier onglet qui est supprimé */
	if (page_num < nbre_pages - 1)
		csv_template_rule_notebook_tab_renumber_pages (priv->notebook_csv_spec, page_num, template_rule);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static GtkWidget *csv_template_rule_notebook_tab_label_new (const gchar *label_text,
															gint page_num,
															CsvTemplateRule *template_rule)
{
	GtkWidget *hbox;
	GtkWidget *button;
	GtkWidget *label;
    GtkWidget *image;

	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	label = gtk_label_new (label_text);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

    image = gtk_image_new_from_icon_name ("gtk-close", GTK_ICON_SIZE_MENU);
    button = gtk_button_new ();
	g_object_set_data (G_OBJECT (button), "page_num", GINT_TO_POINTER (page_num));
	gtk_container_add (GTK_CONTAINER (button), image);
	gtk_container_set_border_width (GTK_CONTAINER (button), 0);
	gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);

	gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, MARGIN_BOX);
	gtk_widget_show_all (hbox);

	g_signal_connect (button,
					  "clicked",
					  G_CALLBACK (csv_template_rule_notebook_tab_close_button_clicked),
					  template_rule);

	return hbox;
}

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
	if (gtk_entry_get_text_length (GTK_ENTRY (entry)) == 1)
	{
		gint rule_number;
		CsvTemplateRulePrivate *priv;

		priv = csv_template_rule_get_instance_private ((CsvTemplateRule *) dialog);
		rule_number = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (dialog), "rule_number"));
		if (rule_number)
			gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GSB_RESPONSE_EDIT, FALSE);
		else
			gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_APPLY, FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET (priv->button_csv_spec_add_line), FALSE);
	}
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
	gint index;
	gint rule_number;
	SpecWidgetLine *line_struct;
	CsvTemplateRulePrivate *priv;

	priv = csv_template_rule_get_instance_private ((CsvTemplateRule *) dialog);
	index = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (entry), "index"));
	rule_number = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (dialog), "rule_number"));
	if (index == 0)
		index = 1;						/* il y a toujours une ligne de configuration spéciale */

	line_struct = (SpecWidgetLine *) g_slist_nth_data (priv->list_spec_lines, index-1);
	if (!line_struct)
		return;

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (line_struct->checkbutton)))
	{
		if (gtk_entry_get_text_length (GTK_ENTRY (priv->entry_csv_rule_name)) > 1
			&&
			gtk_entry_get_text_length (GTK_ENTRY (line_struct->entry_used_text)) > 1)
		{
			if (rule_number)
				gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GSB_RESPONSE_EDIT, TRUE);
			else
				gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_APPLY, TRUE);

			gtk_widget_set_sensitive (GTK_WIDGET (priv->button_csv_spec_add_line), TRUE);
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
	//~ gtk_combo_box_text_append ((GtkComboBoxText *) combobox, NULL, "transfer to account");

	gtk_combo_box_set_active ((GtkComboBox *) combobox,0);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void csv_template_rule_spec_conf_set_cols_name (GtkWidget *combobox,
													   const gchar *cols_name)
{
	gchar **tab;
    gint i=0;

	if (!cols_name)
		return;

	tab = g_strsplit (cols_name, ";", 0);
    while ( tab[i] )
    {
		gtk_combo_box_text_append ((GtkComboBoxText *) combobox, NULL, tab[i]);
		i++;
    }

    g_strfreev ( tab );
	gtk_combo_box_set_active ((GtkComboBox *) combobox,0);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void csv_template_rule_checkbutton_account_changed (GtkToggleButton *checkbutton,
														   CsvTemplateRule *template_rule)
{
	gboolean checked;
	gint rule_number;
	CsvTemplateRulePrivate *priv;

	priv = csv_template_rule_get_instance_private (template_rule);
	rule_number = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (template_rule), "rule_number"));

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
	if (rule_number)
		gtk_dialog_set_response_sensitive (GTK_DIALOG (template_rule), GSB_RESPONSE_EDIT, TRUE);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void csv_template_rule_spec_conf_checkbutton_changed (GtkToggleButton *checkbutton,
															 CsvTemplateRule *template_rule)
{
	gint index;
	gint rule_number;
	SpecWidgetLine *line_struct;
	CsvTemplateRulePrivate *priv;

	priv = csv_template_rule_get_instance_private (template_rule);
	index = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (checkbutton), "index"));
	rule_number = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (template_rule), "rule_number"));

	line_struct = (SpecWidgetLine *) g_slist_nth_data (priv->list_spec_lines, index-1);

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkbutton)))
	{
		gtk_widget_set_sensitive (GTK_WIDGET (line_struct->combobox_action), TRUE);
		gtk_widget_set_sensitive (GTK_WIDGET (line_struct->combobox_used_data), TRUE);
		gtk_widget_set_sensitive (GTK_WIDGET (line_struct->entry_used_text), TRUE);
		gtk_widget_set_sensitive (GTK_WIDGET (priv->button_csv_spec_add_line), TRUE);

		/* changement d'état du bouton Ajouter règle*/
		if (gtk_entry_get_text_length (GTK_ENTRY (line_struct->entry_used_text)) == 0)
		{
			if (rule_number)
				gtk_dialog_set_response_sensitive (GTK_DIALOG (template_rule), GSB_RESPONSE_EDIT, FALSE);
			else
				gtk_dialog_set_response_sensitive (GTK_DIALOG (template_rule), GTK_RESPONSE_APPLY, FALSE);
			gtk_widget_set_sensitive (GTK_WIDGET (priv->button_csv_spec_add_line), FALSE);
		}
	}
	else
	{
		gtk_widget_set_sensitive (GTK_WIDGET (line_struct->combobox_action), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET (line_struct->combobox_used_data), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET (line_struct->combobox_action_data), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET (line_struct->entry_used_text), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET (priv->button_csv_spec_add_line), FALSE);

		if (gtk_entry_get_text_length (GTK_ENTRY (priv->entry_csv_rule_name)) > 0)
		{
			if (rule_number)
				gtk_dialog_set_response_sensitive (GTK_DIALOG (template_rule), GSB_RESPONSE_EDIT, TRUE);
			else
				gtk_dialog_set_response_sensitive (GTK_DIALOG (template_rule), GTK_RESPONSE_APPLY, TRUE);
		}
	}
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void csv_template_rule_spec_conf_combobox_action_changed (GtkComboBox *combobox,
																 CsvTemplateRule *template_rule)
{
	gint index;
	gint action;
	gint rule_number;
	SpecWidgetLine *line_struct = NULL;
	CsvTemplateRulePrivate *priv;

	priv = csv_template_rule_get_instance_private (template_rule);
	index = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (combobox), "index"));
	rule_number = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (template_rule), "rule_number"));

	line_struct = (SpecWidgetLine *) g_slist_nth_data (priv->list_spec_lines, index-1);
	if (!line_struct)
		return;

	action = gtk_combo_box_get_active (GTK_COMBO_BOX (combobox));
	if (action == 1)
	{
		gtk_widget_set_sensitive (GTK_WIDGET (line_struct->combobox_action_data), TRUE);
	}
	else
	{
		gtk_widget_set_sensitive (GTK_WIDGET (line_struct->combobox_action_data), FALSE);
	}
	if (rule_number)
		gtk_dialog_set_response_sensitive (GTK_DIALOG (template_rule), GSB_RESPONSE_EDIT, TRUE);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static SpecWidgetLine *csv_template_rule_spec_conf_new_line (CsvTemplateRule *template_rule,
															 gint index)
{
	GtkWidget *grid;
	GtkWidget *label;
	SpecWidgetLine *line_struct;

	/* set the grid */
	grid = gtk_grid_new ();
	gtk_grid_set_column_spacing (GTK_GRID (grid), MARGIN_BOX);
	gtk_grid_set_row_spacing (GTK_GRID (grid), MARGIN_BOX);

	label = gtk_label_new ("    ");
	gtk_grid_attach (GTK_GRID (grid), label, 0, 0, 1, 1);
	label = gtk_label_new (_("Data to be used"));
	gtk_grid_attach (GTK_GRID (grid), label, 2, 0, 1, 1);
	label = gtk_label_new (_("Value"));
	gtk_grid_attach (GTK_GRID (grid), label, 3, 0, 1, 1);
	label = gtk_label_new (_("Action"));
	gtk_grid_attach (GTK_GRID (grid), label, 4, 0, 1, 1);
	label = gtk_label_new (_("Data of action"));
	gtk_grid_attach (GTK_GRID (grid), label, 5, 0, 1, 1);

	line_struct = g_malloc0 (sizeof (SpecWidgetLine));
	line_struct->index = index;

	line_struct->checkbutton = gtk_check_button_new ();
	gtk_grid_attach (GTK_GRID (grid), line_struct->checkbutton, 1, 1, 1, 1);
	g_object_set_data (G_OBJECT (line_struct->checkbutton), "index", GINT_TO_POINTER (index));

	line_struct->combobox_used_data = gtk_combo_box_text_new ();
	gtk_grid_attach (GTK_GRID (grid), line_struct->combobox_used_data, 2, 1, 1, 1);

	line_struct->entry_used_text = gtk_entry_new ();
	gtk_grid_attach (GTK_GRID (grid), line_struct->entry_used_text, 3, 1, 1, 1);
	g_object_set_data (G_OBJECT (line_struct->entry_used_text), "index", GINT_TO_POINTER (index));

	line_struct->combobox_action = gtk_combo_box_text_new ();
	gtk_grid_attach (GTK_GRID (grid), line_struct->combobox_action, 4, 1, 1, 1);
	g_object_set_data (G_OBJECT (line_struct->combobox_action), "index", GINT_TO_POINTER (index));

	line_struct->combobox_action_data = gtk_combo_box_text_new ();
	gtk_grid_attach (GTK_GRID (grid), line_struct->combobox_action_data, 5, 1, 1, 1);

	gtk_widget_set_sensitive (GTK_WIDGET (line_struct->combobox_used_data), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (line_struct->entry_used_text), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (line_struct->combobox_action), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (line_struct->combobox_action_data), FALSE);

	/* Connect signal */
	g_signal_connect (line_struct->checkbutton,
					  "toggled",
					  G_CALLBACK (csv_template_rule_spec_conf_checkbutton_changed),
					  template_rule);
	g_signal_connect (line_struct->combobox_action,
					  "changed",
					  G_CALLBACK (csv_template_rule_spec_conf_combobox_action_changed),
					  template_rule);

	g_signal_connect (line_struct->entry_used_text,
					  "insert-text",
					  G_CALLBACK (csv_template_rule_spec_conf_entry_inserted),
					  template_rule);
	g_signal_connect (line_struct->entry_used_text,
					  "delete-text",
					  G_CALLBACK (csv_template_rule_spec_conf_entry_deleted),
					  template_rule);

	gtk_widget_show_all (grid);
	line_struct->grid = grid;

	return line_struct;
}

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
static void csv_template_rule_notebook_switch_page (GtkNotebook *notebook,
													GtkWidget *page,
													guint page_num,
													CsvTemplateRule *template_rule)
{
	GSList *list;
	gboolean page_removed = FALSE;
	CsvTemplateRulePrivate *priv;

	//~ devel_debug_int (page_num);
	priv = csv_template_rule_get_instance_private (template_rule);

	g_signal_handlers_block_by_func (GTK_NOTEBOOK (priv->notebook_csv_spec),
									 csv_template_rule_notebook_switch_page,
									 template_rule);
	list = priv->list_spec_lines;
	while (list)
	{
		SpecWidgetLine *line_struct;

		line_struct = (SpecWidgetLine *) list->data;
		list = list->next;

		if (line_struct->index > 0)
		{
			if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (line_struct->checkbutton)) == FALSE)
			{
				/* remove the page */
				gtk_notebook_remove_page (GTK_NOTEBOOK (priv->notebook_csv_spec), line_struct->index -1);
				priv->list_spec_lines = g_slist_remove (priv->list_spec_lines, (gpointer) line_struct);
				page_removed = TRUE;
			}
		}
	}
	if (page_removed)
	{
		csv_template_rule_notebook_tab_renumber_pages (priv->notebook_csv_spec, 0, template_rule);
	}
	g_signal_handlers_unblock_by_func (GTK_NOTEBOOK (priv->notebook_csv_spec),
									   csv_template_rule_notebook_switch_page,
									   template_rule);

}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void csv_template_rule_button_add_line_clicked (GtkButton *button,
													   CsvTemplateRule *template_rule)
{
	GtkWidget *tab_label;
	gchar *tmp_str;
	gint index;
	SpecWidgetLine *line_struct;
	CsvTemplateRulePrivate *priv;

	priv = csv_template_rule_get_instance_private (template_rule);

	/* invalider le button */
	gtk_widget_set_sensitive (GTK_WIDGET (priv->button_csv_spec_add_line), FALSE);

	/* création de la ligne, initialisation et ajout dans la liste */
	index = g_slist_length (priv->list_spec_lines);
	line_struct = csv_template_rule_spec_conf_new_line (template_rule, index + 1);

	csv_template_rule_spec_conf_set_cols_name (line_struct->combobox_used_data, priv->combobox_cols_name);
	csv_template_rule_spec_conf_set_cols_name (line_struct->combobox_action_data, priv->combobox_cols_name);
	csv_template_rule_spec_conf_set_actions (GTK_COMBO_BOX_TEXT (line_struct->combobox_action));
	priv->list_spec_lines = g_slist_append (priv->list_spec_lines, line_struct);

	gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook_csv_spec), line_struct->grid, NULL);
	tmp_str = g_strdup_printf("Condition %d", index + 1);
	tab_label = csv_template_rule_notebook_tab_label_new (tmp_str, index, template_rule);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (priv->notebook_csv_spec), line_struct->grid, tab_label);
	g_free (tmp_str);

	/* on bloque le signal car le checkbutton de la nouvelle ligne est décochée par défaut */
	g_signal_handlers_block_by_func (GTK_NOTEBOOK (priv->notebook_csv_spec),
									 csv_template_rule_notebook_switch_page,
									 template_rule);
	gtk_notebook_set_current_page (GTK_NOTEBOOK (priv->notebook_csv_spec), index);
	/* on débloque le signal */
	g_signal_handlers_unblock_by_func (GTK_NOTEBOOK (priv->notebook_csv_spec),
									   csv_template_rule_notebook_switch_page,
									   template_rule);
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
	GSList *list;
	gint rule_number;
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
			csv_rule->csv_cols_name = g_strdup (priv->combobox_cols_name);
			list = priv->list_spec_lines;
			while (list)
			{
				SpecWidgetLine *line_struct;

				line_struct = (SpecWidgetLine *) list->data;

				if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (line_struct->checkbutton)))
				{
					GtkWidget *widget;
					SpecConfData *spec_conf_data;

					spec_conf_data = g_malloc0 (sizeof (SpecConfData));
					widget = line_struct->combobox_action;
					spec_conf_data->csv_spec_conf_action = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
					if (spec_conf_data->csv_spec_conf_action == 1)
					{
						widget = line_struct->combobox_action_data;
						spec_conf_data->csv_spec_conf_action_data = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
					}
					widget = line_struct->combobox_used_data;
					spec_conf_data->csv_spec_conf_used_data = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
					widget = line_struct->entry_used_text;
					spec_conf_data->csv_spec_conf_used_text = g_strdup (gtk_entry_get_text (GTK_ENTRY (widget)));
					csv_rule->csv_spec_lines_list = g_slist_append (csv_rule->csv_spec_lines_list, spec_conf_data);
				}
				list = list->next;
			}
				g_object_set_data (G_OBJECT (dialog), "csv-import-rule", csv_rule);
			break;
		case GSB_RESPONSE_EDIT:			/* EDIT rule */
			rule_number = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (dialog), "rule_number"));
			if (rule_number)
			{
				GSList * new_csv_spec_lines_list;
				const gchar *tmp_str;

				tmp_str = gtk_entry_get_text (GTK_ENTRY (priv->entry_csv_rule_name));
				gsb_data_import_rule_set_name (rule_number, tmp_str);
				gsb_data_import_rule_set_csv_account_id_col (rule_number,
															 gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
																							   (priv->spinbutton_csv_account_id_col)));
				gsb_data_import_rule_set_csv_account_id_row (rule_number,
															 gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
																							   (priv->spinbutton_csv_account_id_row)));
				gsb_data_import_rule_set_csv_first_line_data (rule_number,
															  gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
																								(priv->spinbutton_csv_first_line)));
				gsb_data_import_rule_set_csv_headers_present (rule_number,
															  gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
																							(priv->checkbutton_csv_header_col)));
				/* specific configuration */
				/* on libère la liste des lignes spéciales de la règle */
				gsb_data_import_rule_free_csv_spec_lines_list (rule_number);
				list = priv->list_spec_lines;
				while (list)
				{
					SpecWidgetLine *line_struct;

					line_struct = (SpecWidgetLine *) list->data;

					if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (line_struct->checkbutton)))
					{
						GtkWidget *widget;
						SpecConfData *spec_conf_data;

						spec_conf_data = g_malloc0 (sizeof (SpecConfData));
						widget = line_struct->combobox_action;
						spec_conf_data->csv_spec_conf_action = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
						if (spec_conf_data->csv_spec_conf_action == 1)
						{
							widget = line_struct->combobox_action_data;
							spec_conf_data->csv_spec_conf_action_data = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
						}
						widget = line_struct->combobox_used_data;
						spec_conf_data->csv_spec_conf_used_data = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
						widget = line_struct->entry_used_text;
						spec_conf_data->csv_spec_conf_used_text = g_strdup (gtk_entry_get_text (GTK_ENTRY (widget)));
						new_csv_spec_lines_list = g_slist_append (new_csv_spec_lines_list, spec_conf_data);
					}
					list = list->next;
				}
				utils_prefs_gsb_file_set_modified ();
				gsb_data_import_rule_set_csv_spec_lines_list (rule_number, new_csv_spec_lines_list);
			}
			gtk_widget_destroy (GTK_WIDGET (dialog));
			break;
		case GTK_RESPONSE_CANCEL:
			gtk_widget_destroy (GTK_WIDGET (dialog));
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
	GtkWidget *button;
	GtkWidget *tab_label;
	GSList *csv_file_columns;
	GSList *list;
	gchar *tmp_str;
	gint csv_first_line_data;
	SpecWidgetLine *line_struct;
	CsvTemplateRulePrivate *priv;

	devel_debug (NULL);

	priv = csv_template_rule_get_instance_private (template_rule);

	/* window title */
	gtk_window_set_title (GTK_WINDOW (template_rule), _("Create an import rule for CSV file"));

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
	line_struct = csv_template_rule_spec_conf_new_line (template_rule, 1);
	gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook_csv_spec), line_struct->grid, NULL);

	/* set the tab_label */
	tmp_str = g_strdup_printf(_("Condition %d"), 1);
	tab_label = gtk_label_new (tmp_str);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (priv->notebook_csv_spec), line_struct->grid, tab_label);
	g_free (tmp_str);

	/* set the notebook action widget */
	button = utils_buttons_button_new_from_image ("gtk-add-16.png");
	gtk_widget_show (button);
	gtk_widget_set_sensitive (GTK_WIDGET (button), FALSE);
	gtk_notebook_set_action_widget (GTK_NOTEBOOK (priv->notebook_csv_spec), button, GTK_PACK_START);
	priv->button_csv_spec_add_line = button;

	/* init line_struct */
	csv_file_columns = csv_import_get_columns_list (assistant);
	list = csv_file_columns;
	while (list)
	{
		gchar *string_to_free = NULL;

		if (priv->combobox_cols_name == NULL)
			priv->combobox_cols_name = g_strdup (list->data);
		else
		{
			priv->combobox_cols_name = g_strconcat ((string_to_free = priv->combobox_cols_name),
													";", list->data, NULL);
			g_free (string_to_free);
		}
		list = list->next;
	}
	csv_template_rule_spec_conf_set_cols_name (line_struct->combobox_used_data, priv->combobox_cols_name);
	csv_template_rule_spec_conf_set_cols_name (line_struct->combobox_action_data, priv->combobox_cols_name);
	csv_template_rule_spec_conf_set_actions (GTK_COMBO_BOX_TEXT (line_struct->combobox_action));

	priv->list_spec_lines = g_slist_append (priv->list_spec_lines, line_struct);

	/* set signals */
	g_signal_connect (priv->notebook_csv_spec,
					  "switch-page",
					  G_CALLBACK (csv_template_rule_notebook_switch_page),
					  template_rule);

	g_signal_connect (priv->button_csv_spec_add_line,
					  "clicked",
					  G_CALLBACK (csv_template_rule_button_add_line_clicked),
					  template_rule);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void csv_template_rule_edit_dialog (CsvTemplateRule *template_rule,
										   gint rule_number)
{
	GtkWidget *button;
	GtkWidget *tab_label;
	const gchar *tmp_str;
	gint index = 0;
	gboolean checked;
	CsvTemplateRulePrivate *priv;

	devel_debug (NULL);

	priv = csv_template_rule_get_instance_private (template_rule);

	/* window title */
	gtk_window_set_title (GTK_WINDOW (template_rule), _("Edit an import rule for CSV file"));

	/* set the edit button */
	button = gtk_dialog_get_widget_for_response (GTK_DIALOG (template_rule), GTK_RESPONSE_APPLY);
	gtk_widget_hide (button);
	button = gtk_dialog_get_widget_for_response (GTK_DIALOG (template_rule), GSB_RESPONSE_EDIT);
	gtk_widget_show (button);
	gtk_widget_set_sensitive (button, FALSE);
	g_object_set_data (G_OBJECT (template_rule), "rule_number", GINT_TO_POINTER (rule_number));

	/* set the notebook action widget */
	button = utils_buttons_button_new_from_image ("gtk-add-16.png");
	gtk_widget_show (button);
	gtk_notebook_set_action_widget (GTK_NOTEBOOK (priv->notebook_csv_spec), button, GTK_PACK_START);
	priv->button_csv_spec_add_line = button;

	/* init general data */
	tmp_str = gsb_data_import_rule_get_name (rule_number);
	gtk_entry_set_text (GTK_ENTRY (priv->entry_csv_rule_name), tmp_str);

	index = gsb_data_import_rule_get_csv_account_id_col (rule_number);
	if (index)
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_csv_account_id), TRUE);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (priv->spinbutton_csv_account_id_col), (gdouble) index);
		index = gsb_data_import_rule_get_csv_account_id_row (rule_number);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (priv->spinbutton_csv_account_id_row), (gdouble) index);
	}

	index = gsb_data_import_rule_get_csv_first_line_data (rule_number);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (priv->spinbutton_csv_first_line), (gdouble) index);
	checked = gsb_data_import_rule_get_csv_headers_present (rule_number);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_csv_header_col), checked);

	/* Connect signal */
    g_signal_connect (priv->eventbox_csv_account_id,
					  "button-press-event",
					  G_CALLBACK (utils_prefs_page_eventbox_clicked),
					  priv->checkbutton_csv_account_id);

	g_signal_connect (priv->checkbutton_csv_account_id,
					  "toggled",
					  G_CALLBACK (csv_template_rule_checkbutton_account_changed),
					  template_rule);

    g_signal_connect (priv->eventbox_csv_header_col,
					  "button-press-event",
					  G_CALLBACK (utils_prefs_page_eventbox_clicked),
					  priv->checkbutton_csv_header_col);

	g_signal_connect_after (priv->spinbutton_csv_account_id_col,
							"value-changed",
							G_CALLBACK (csv_template_rule_spec_conf_edit_widget_changed),
							template_rule);

	g_signal_connect_after (priv->spinbutton_csv_account_id_row,
							"value-changed",
							G_CALLBACK (csv_template_rule_spec_conf_edit_widget_changed),
							template_rule);

	g_signal_connect_after (priv->spinbutton_csv_first_line,
							"value-changed",
							G_CALLBACK (csv_template_rule_spec_conf_edit_widget_changed),
							template_rule);

	g_signal_connect_after (priv->checkbutton_csv_header_col,
							"toggled",
							G_CALLBACK (csv_template_rule_spec_conf_edit_widget_changed),
							template_rule);

	/* on regarde si il y a un traitement spécial */
	index = 1;
	if (gsb_data_import_rule_get_csv_spec_nbre_lines (rule_number))
	{
		GSList *list;
		SpecConfData *spec_conf_data;

		list = gsb_data_import_rule_get_csv_spec_lines_list (rule_number);
		spec_conf_data = (SpecConfData *) list->data;
		while (list)
		{
			gchar *label_str;
			SpecWidgetLine *line_struct;

			spec_conf_data = (SpecConfData *) list->data;

			/* set the spec line */
			line_struct = csv_template_rule_spec_conf_new_line (template_rule, index);
			gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook_csv_spec), line_struct->grid, NULL);
			/* set the tab_label */
			label_str = g_strdup_printf(_("Condition %d"), index);
			tab_label = gtk_label_new (label_str);
			gtk_notebook_set_tab_label (GTK_NOTEBOOK (priv->notebook_csv_spec), line_struct->grid, tab_label);
			g_free (label_str);
			/* init line_struct */
			priv->combobox_cols_name = g_strdup (gsb_data_import_rule_get_csv_spec_cols_name (rule_number));
			csv_template_rule_spec_conf_set_cols_name (line_struct->combobox_used_data, priv->combobox_cols_name);
			csv_template_rule_spec_conf_set_cols_name (line_struct->combobox_action_data, priv->combobox_cols_name);
			csv_template_rule_spec_conf_set_actions (GTK_COMBO_BOX_TEXT (line_struct->combobox_action));

			/* add line */
			priv->list_spec_lines = g_slist_append (priv->list_spec_lines, line_struct);

			/* set widgets */
			/* on bloque le signal car le checkbutton de la nouvelle ligne est décochée par défaut */
			g_signal_handlers_block_by_func (G_OBJECT (line_struct->checkbutton),
											 csv_template_rule_spec_conf_checkbutton_changed,
											 template_rule);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (line_struct->checkbutton), TRUE);
			gtk_combo_box_set_active (GTK_COMBO_BOX (line_struct->combobox_action), spec_conf_data->csv_spec_conf_action);
			gtk_combo_box_set_active (GTK_COMBO_BOX (line_struct->combobox_action_data),
									  spec_conf_data->csv_spec_conf_action_data);
			gtk_combo_box_set_active (GTK_COMBO_BOX (line_struct->combobox_used_data),
									  spec_conf_data->csv_spec_conf_used_data);
			gtk_entry_set_text (GTK_ENTRY (line_struct->entry_used_text), spec_conf_data->csv_spec_conf_used_text);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (line_struct->checkbutton), TRUE);
			gtk_widget_set_sensitive (line_struct->entry_used_text, TRUE);
			gtk_widget_set_sensitive (line_struct->combobox_action, TRUE);
			gtk_widget_set_sensitive (line_struct->combobox_used_data, TRUE);

			/* unblock the signal */
			g_signal_handlers_unblock_by_func (G_OBJECT (line_struct->checkbutton),
											   csv_template_rule_spec_conf_checkbutton_changed,
											   template_rule);

			index++;
			list = list->next;
		};
	}
	else
	{
		gchar *label_str;
		SpecWidgetLine *line_struct;

		/* set the spec line */
		line_struct = csv_template_rule_spec_conf_new_line (template_rule, index);
		gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook_csv_spec), line_struct->grid, NULL);
		/* set the tab_label */
		label_str = g_strdup_printf(_("Condition %d"), index);
		tab_label = gtk_label_new (label_str);
		gtk_notebook_set_tab_label (GTK_NOTEBOOK (priv->notebook_csv_spec), line_struct->grid, tab_label);
		g_free (label_str);
		/* init line_struct */
		priv->combobox_cols_name = g_strdup (gsb_data_import_rule_get_csv_spec_cols_name (rule_number));
		csv_template_rule_spec_conf_set_cols_name (line_struct->combobox_used_data, priv->combobox_cols_name);
		csv_template_rule_spec_conf_set_cols_name (line_struct->combobox_action_data, priv->combobox_cols_name);
		csv_template_rule_spec_conf_set_actions (GTK_COMBO_BOX_TEXT (line_struct->combobox_action));
	}
	/* set signals */
	g_signal_connect (priv->notebook_csv_spec,
					  "switch-page",
					  G_CALLBACK (csv_template_rule_notebook_switch_page),
					  template_rule);

	g_signal_connect (priv->button_csv_spec_add_line,
					  "clicked",
					  G_CALLBACK (csv_template_rule_button_add_line_clicked),
					  template_rule);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void csv_template_rule_init (CsvTemplateRule *template_rule)
{
	CsvTemplateRulePrivate *priv;

	priv = csv_template_rule_get_instance_private ((CsvTemplateRule *) template_rule);
	gtk_widget_init_template (GTK_WIDGET (template_rule));

	/* initialisation des variables */
	priv->combobox_cols_name = NULL;
	priv->list_spec_lines = NULL;

	/* Add action buttons */
	gtk_dialog_set_response_sensitive (GTK_DIALOG (template_rule), GTK_RESPONSE_APPLY, FALSE);
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

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), CsvTemplateRule, checkbutton_csv_account_id);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), CsvTemplateRule, checkbutton_csv_header_col);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), CsvTemplateRule, entry_csv_rule_name);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), CsvTemplateRule, eventbox_csv_account_id);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), CsvTemplateRule, spinbutton_csv_account_id_col);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), CsvTemplateRule, spinbutton_csv_account_id_row);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), CsvTemplateRule, eventbox_csv_header_col);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), CsvTemplateRule, spinbutton_csv_first_line);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), CsvTemplateRule, notebook_csv_spec);

	/* signaux */
    gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (klass), csv_template_dialog_response);
	gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (klass), csv_template_rule_spec_conf_entry_deleted);
	gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (klass), csv_template_rule_spec_conf_entry_inserted);
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
CsvTemplateRule *csv_template_rule_edit (GtkWindow *parent,
										 gint rule_number)
{
	CsvTemplateRule *template_rule;

	template_rule = g_object_new (CSV_TEMPLATE_RULE_TYPE, "transient-for", GTK_WINDOW (parent), NULL);
	gtk_window_set_modal (GTK_WINDOW (template_rule), TRUE);

	csv_template_rule_edit_dialog (template_rule, rule_number);

	return template_rule;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void csv_template_rule_csv_import_rule_struct_free	(CSVImportRule *csv_rule)
{
	GSList *list;

	g_free (csv_rule->csv_rule_name);
	g_free (csv_rule->csv_cols_name);
	list = csv_rule->csv_spec_lines_list;
	while (list)
	{
		SpecConfData *spec_conf_data;

		spec_conf_data = list->data;
		if (spec_conf_data)
		{
			g_free (spec_conf_data->csv_spec_conf_used_text);
		}
		list = list->next;
	}
	g_slist_free (csv_rule->csv_spec_lines_list);
	g_free (csv_rule);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void csv_template_rule_spec_conf_data_struct_copy (SpecConfData *spec_conf_data)
{
	SpecConfData *new_struct;

	new_struct = g_malloc0 (sizeof (SpecConfData));
	new_struct->csv_spec_conf_action = spec_conf_data->csv_spec_conf_action;
	new_struct->csv_spec_conf_action_data = spec_conf_data->csv_spec_conf_action_data;
	new_struct->csv_spec_conf_used_data = spec_conf_data->csv_spec_conf_used_data;
	new_struct->csv_spec_conf_used_text = g_strdup (spec_conf_data->csv_spec_conf_used_text);
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

