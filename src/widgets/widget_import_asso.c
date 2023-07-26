/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2020 Pierre Biava (grisbi@pierre.biava.name)                    */
/*          https://www.grisbi.org/                                              */
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

#include <errno.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "widget_import_asso.h"
#include "dialog.h"
#include "gsb_combo_box.h"
#include "gsb_data_payee.h"
#include "gsb_file.h"
#include "gsb_form_widget.h"
#include "gtk_combofix.h"
#include "import.h"
#include "structures.h"
#include "tiers_onglet.h"
#include "utils.h"
#include "utils_prefs.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _WidgetImportAssoPrivate   WidgetImportAssoPrivate;

struct _WidgetImportAssoPrivate
{
	GtkWidget *			vbox_import_asso;
	GtkWidget *			treeview_import_asso;
    GtkWidget *			grid_import_asso_details;
	GtkWidget *			combo_import_asso_payee;
	GtkWidget *			entry_import_asso_search_string;
    GtkWidget *         button_import_asso_add;
    GtkWidget *         button_import_asso_remove;
	GtkWidget *			checkbutton_import_asso_case_insensitive;
	GtkWidget *			checkbutton_import_asso_use_regex;
};

G_DEFINE_TYPE_WITH_PRIVATE (WidgetImportAsso, widget_import_asso, GTK_TYPE_BOX)

static GtkTreePath *path_selected;

enum AssoListColumns
{
	ASSO_LIST_PAYEE_NAME,
	ASSO_LIST_SEARCH_STR,
	ASSO_PAYEE_NUMBER,
	ASSO_BACKGROUND_COLOR,
	ASSO_REAL_PAYEE_NAME,
	ASSO_REAL_SEARCH_STR,
	ASSO_MODEL_N_COLUMNS
};

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * Appellée lorsqu'on coche la case "Automatic filling transactions from payee"
 *
 * \param
 * \param
 *
 * \return
 **/
static void widget_import_asso_checkbutton_import_asso_case_insensitive_toggle (GtkWidget *checkbutton,
																					WidgetImportAsso *page)
{
	GtkWidget *entry;
    gint payee_number;
 	WidgetImportAssoPrivate *priv;

	priv = widget_import_asso_get_instance_private (page);
	entry = gtk_combofix_get_entry (GTK_COMBOFIX (priv->combo_import_asso_payee));
    payee_number = gsb_data_payee_get_number_by_name (gtk_editable_get_chars
													  (GTK_EDITABLE (entry), 0, -1),
													  FALSE);
	gsb_data_payee_set_ignore_case (payee_number,
									gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkbutton)));
}

/**
 * Select the row which show the payee with number passed in data
 *
 * \param
 * \param
 * \param
 * \param
 *
 * \return TRUE if find
 **/
static gboolean widget_import_asso_path_selected (GtkTreeModel *model,
												   	  GtkTreePath *path,
												  	  GtkTreeIter *iter,
												  	  gpointer data)
{
	gint payee_number;
	gint tmp_payee_number;

	payee_number = GPOINTER_TO_INT (data);
	gtk_tree_model_get (model, iter, ASSO_PAYEE_NUMBER, &tmp_payee_number, -1);

	if (tmp_payee_number == payee_number)
	{
		if (path_selected)
		{
			gtk_tree_path_free (path_selected);
			path_selected = NULL;
		}

		path_selected = gtk_tree_path_copy (path);
		return TRUE;
	}

	return FALSE;
}

/**
 * fonction itérative pour sélectionner une association dans la liste
 * suite à création ou suite à sélection par le combobox des tiers
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
static void widget_import_asso_select_row (GtkWidget* tree_view,
											   GtkTreeModel *model,
											   gint payee_number)
{
	GtkTreeSelection *selection;

	/* set the path to selected */
	gtk_tree_model_foreach (model,
							(GtkTreeModelForeachFunc) widget_import_asso_path_selected,
							GINT_TO_POINTER (payee_number));

	/* select the good row */
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
	gtk_tree_selection_select_path (selection, path_selected);
	gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (tree_view), path_selected, NULL, FALSE, 0,0);
	gtk_tree_path_free (path_selected);
	path_selected = NULL;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static gboolean widget_import_asso_check_add_button (WidgetImportAsso *page)
{
    gboolean sensitive = TRUE;
	WidgetImportAssoPrivate *priv;

	priv = widget_import_asso_get_instance_private (page);

	if (priv->combo_import_asso_payee)
    {
        const gchar *content;

        content = gtk_combofix_get_text (GTK_COMBOFIX (priv->combo_import_asso_payee));
        if (!content || ! strlen (content))
            sensitive = FALSE;
    }

    if (priv->entry_import_asso_search_string)
    {
		const gchar *content;

		content= gtk_entry_get_text (GTK_ENTRY (priv->entry_import_asso_search_string));
		if (!content || !strlen(content))
			sensitive = FALSE;
    }

    gtk_widget_set_sensitive (GTK_WIDGET (priv->button_import_asso_add), sensitive);

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
static void widget_import_asso_combo_changed (GtkEditable *editable,
												  WidgetImportAsso *page)
{
	GtkTreeSelection *selection;
    const gchar *tmp_str;
    gint payee_number;
	WidgetImportAssoPrivate *priv;

	priv = widget_import_asso_get_instance_private (page);

	/* on supprime la sélection */
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_import_asso));
	gtk_tree_selection_unselect_all (selection);

	/* on récupère le nom du tiers entré et son numero */
    payee_number = gsb_data_payee_get_number_by_name (gtk_editable_get_chars (editable, 0, -1), FALSE);

	if (payee_number == 0)
	{
		/* on est en phase de création d'un nouveau tiers */
		return;
	}
    tmp_str = gsb_data_payee_get_search_string (payee_number);
    gtk_entry_set_text (GTK_ENTRY (priv->entry_import_asso_search_string), tmp_str);
    gtk_widget_set_sensitive (priv->entry_import_asso_search_string, TRUE);

    if (g_utf8_strlen (tmp_str, -1) > 0)
    {
		GtkTreeModel *model;

		model = gtk_tree_view_get_model (GTK_TREE_VIEW (priv->treeview_import_asso));
		widget_import_asso_select_row (priv->treeview_import_asso,
										   model,
										   payee_number);
    }

    /* on empeche la suppression par inadvertance d'une association */
    gtk_widget_set_sensitive (priv->button_import_asso_remove, FALSE);

    widget_import_asso_check_add_button (page);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static gboolean widget_import_asso_select_asso (GtkTreeSelection *selection,
													WidgetImportAsso *page)
{
    GtkTreeIter iter;
    gboolean good;
	WidgetImportAssoPrivate *priv;

	priv = widget_import_asso_get_instance_private (page);
	good = gtk_tree_selection_get_selected (selection, NULL, &iter);

    if (good)
    {
		GtkWidget *entry;
		GtkTreeModel *model;
		gint payee_number;
		gchar *payee_str;
		gchar *search_str;

		entry = gtk_combofix_get_entry (GTK_COMBOFIX (priv->combo_import_asso_payee));
		model = gtk_tree_view_get_model (GTK_TREE_VIEW (priv->treeview_import_asso));
		gtk_tree_model_get (model,
							&iter,
							ASSO_REAL_PAYEE_NAME, &payee_str,
							ASSO_REAL_SEARCH_STR, &search_str,
							ASSO_PAYEE_NUMBER, &payee_number,
							-1);

		g_signal_handlers_block_by_func (G_OBJECT (entry),
										 G_CALLBACK (widget_import_asso_combo_changed),
										 page);
		gtk_combofix_set_text (GTK_COMBOFIX (priv->combo_import_asso_payee), payee_str);
		g_signal_handlers_unblock_by_func (G_OBJECT (entry),
										   G_CALLBACK (widget_import_asso_combo_changed),
										   page);
		gtk_entry_set_text (GTK_ENTRY (priv->entry_import_asso_search_string), search_str);
		gtk_widget_set_sensitive (priv->button_import_asso_remove, TRUE);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_import_asso_case_insensitive),
									  gsb_data_payee_get_ignore_case (payee_number));
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_import_asso_use_regex),
									  gsb_data_payee_get_use_regex (payee_number));

	}

    return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void widget_import_asso_fill_model (GtkListStore *list_store)
{
    GSList *list_tmp;
    GtkTreeIter iter;

	list_tmp = gsb_import_associations_get_liste_associations ();
    gtk_list_store_clear (GTK_LIST_STORE (list_store));

    while (list_tmp)
    {
		const gchar *payee_name;
        gchar *tmp_str1;
        gchar *tmp_str2;
        struct ImportPayeeAsso *assoc;

        assoc = list_tmp->data;
        payee_name = gsb_data_payee_get_name (assoc->payee_number, TRUE);
		tmp_str1 = utils_str_break_form_name_field (payee_name, TRUNC_FORM_FIELD);
		if (assoc->search_str)
			tmp_str2 = utils_str_break_form_name_field (assoc->search_str, TRUNC_FORM_FIELD);
		else
			tmp_str2 = NULL;
        gtk_list_store_append (GTK_LIST_STORE (list_store), &iter);
        gtk_list_store_set (GTK_LIST_STORE (list_store),
							&iter,
							ASSO_LIST_PAYEE_NAME, tmp_str1,
							ASSO_LIST_SEARCH_STR, tmp_str2,
							ASSO_PAYEE_NUMBER, assoc->payee_number,
							ASSO_REAL_PAYEE_NAME, payee_name,
							ASSO_REAL_SEARCH_STR, assoc->search_str,
							-1);
		if (tmp_str1)
			g_free (tmp_str1);
		if (tmp_str2)
			g_free (tmp_str2);
        list_tmp = list_tmp->next;
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
static void widget_import_asso_add_button_clicked (GtkWidget *button,
													   WidgetImportAsso *page)
{
	gchar *hint;
	WidgetImportAssoPrivate *priv;

	devel_debug (NULL);
	priv = widget_import_asso_get_instance_private (page);

	hint = g_strdup (_("You are creating a new payee with a rule"));
	if (dialogue_yes_no (_("If you continue, you will use the payee management module to create "
						   "a new third party and its rule.\n"
						   "Continue anyway?"),
						 hint,
						 GTK_RESPONSE_YES))
	{
		GtkTreeModel *model;
    	gint payee_number;

		payees_manage_payees ();
		payee_number = gsb_import_associations_get_last_payee_number ();
		model = gtk_tree_view_get_model (GTK_TREE_VIEW (priv->treeview_import_asso));

		/* Fill the model */
		g_signal_handlers_block_by_func (G_OBJECT (gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_import_asso))),
										 G_CALLBACK (widget_import_asso_select_asso),
										 page);

		widget_import_asso_fill_model (GTK_LIST_STORE (model));
		g_signal_handlers_unblock_by_func (G_OBJECT (gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_import_asso))),
										   G_CALLBACK (widget_import_asso_select_asso),
										   page);

		/*colorize the tree_view */
		utils_set_list_store_background_color (priv->treeview_import_asso, ASSO_BACKGROUND_COLOR);

		/* select the new ruls */
		widget_import_asso_select_row (priv->treeview_import_asso, model, payee_number);
	}
	g_free (hint);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void widget_import_asso_del_assoc (GtkWidget *button,
											  WidgetImportAsso *page)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gint payee_number;
	WidgetImportAssoPrivate *priv;

	priv = widget_import_asso_get_instance_private (page);

    if (!gtk_tree_selection_get_selected (gtk_tree_view_get_selection
										  (GTK_TREE_VIEW (priv->treeview_import_asso)),
										  NULL,
										  &iter))
	{
		return;
	}

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (priv->treeview_import_asso));
    gtk_tree_model_get (model, &iter, ASSO_PAYEE_NUMBER, &payee_number, -1);
    if (payee_number > 0)
    {
		gsb_import_associations_remove_assoc (payee_number);
		gtk_combofix_set_text (GTK_COMBOFIX (priv->combo_import_asso_payee), "");
		gtk_entry_set_text (GTK_ENTRY (priv->entry_import_asso_search_string), "");
		widget_import_asso_fill_model (GTK_LIST_STORE (model));
		utils_set_list_store_background_color (priv->treeview_import_asso, ASSO_BACKGROUND_COLOR);
		utils_prefs_gsb_file_set_modified ();
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
static void widget_import_asso_add_assoc (GtkWidget *button,
											  WidgetImportAsso *page)
{
    gchar *payee = NULL;
    gchar *search_str = NULL;
    gint payee_number;
	gint ignore_case = 0;
	gint use_regex = 0;
	gboolean result = FALSE;
	WidgetImportAssoPrivate *priv;

	priv = widget_import_asso_get_instance_private (page);

	/* on autorise l'édition du combobox tiers */
	payee = g_strstrip (g_strdup (gtk_combofix_get_text (GTK_COMBOFIX (priv->combo_import_asso_payee))));
    search_str = g_strstrip (g_strdup (gtk_entry_get_text (GTK_ENTRY (priv->entry_import_asso_search_string))));

	/* crée si besoin le tiers */
    payee_number = gsb_data_payee_get_number_by_name (payee, FALSE);
	if (payee_number == 0)
	{
		gchar *hint;
		gchar *text;
		gchar *tmp_markup_str;

		hint = g_strdup (_("You will be redirected to the payee management module"));
		text = g_strdup (_("You cannot directly create a new payee and its rule like this.\n"
						   "If you continue, you will use the payee management module "
						   "to do so.\n"
						   "Continue anyway?"));

		tmp_markup_str = g_markup_printf_escaped ("<span size=\"larger\" weight=\"bold\">%s</span>\n\n", hint);
		if (dialogue_yes_no (text, tmp_markup_str, GTK_RESPONSE_YES))
		{
			payees_manage_payees_with_rule (search_str);
			payee_number = gsb_import_associations_get_last_payee_number ();
			result = TRUE;
		}
		g_free (hint);
		g_free (text);
		g_free (tmp_markup_str);
	}
	else
		result = gsb_import_associations_add_assoc (payee_number, search_str, ignore_case, use_regex);

	g_free(payee);
	g_free(search_str);

	if (result)
	{
		GtkTreeModel *model;

		/* on remplit le modèle et on se positionne sur la nouvelle association */
		model = gtk_tree_view_get_model (GTK_TREE_VIEW (priv->treeview_import_asso));
		g_signal_handlers_block_by_func (G_OBJECT (gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_import_asso))),
										 G_CALLBACK (widget_import_asso_select_asso),
										 page);

		widget_import_asso_fill_model (GTK_LIST_STORE (model));
		g_signal_handlers_unblock_by_func (G_OBJECT (gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_import_asso))),
										   G_CALLBACK (widget_import_asso_select_asso),
										   page);
		widget_import_asso_select_row (priv->treeview_import_asso,
										   model,
										   payee_number);
		utils_set_list_store_background_color (priv->treeview_import_asso, ASSO_BACKGROUND_COLOR);

		utils_prefs_gsb_file_set_modified ();
	}
}

/**
 * update une association existante
 *
 * \param text		nouvelle version de la règle.
 * \param
 *
 * \return
 **/
static void widget_import_asso_update_assoc (const gchar *rule,
												 WidgetImportAsso *page)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gint payee_number;
	WidgetImportAssoPrivate *priv;

	priv = widget_import_asso_get_instance_private (page);

    if (!gtk_tree_selection_get_selected (gtk_tree_view_get_selection
										  (GTK_TREE_VIEW (priv->treeview_import_asso)),
										  NULL,
										  &iter))
	{
		/* on est dans une nouvelle association pour un tiers existant */
		widget_import_asso_add_assoc (NULL, page);

		return;
	}

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (priv->treeview_import_asso));
    gtk_tree_model_get (model, &iter, ASSO_PAYEE_NUMBER, &payee_number, -1);
    if (payee_number > 0)
    {
    	GSList *list_tmp;
		gchar *tmp_str;
		GrisbiWinRun *w_run;

		w_run = grisbi_win_get_w_run ();

		tmp_str = utils_str_break_form_name_field (rule, TRUNC_FORM_FIELD);
		gtk_list_store_set (GTK_LIST_STORE (model),
							&iter,
							ASSO_LIST_SEARCH_STR, tmp_str,
							ASSO_REAL_SEARCH_STR, rule,
							-1);
		g_free (tmp_str);

		list_tmp = gsb_import_associations_get_liste_associations ();
	    while (list_tmp)
		{
			struct ImportPayeeAsso *assoc;

			assoc = list_tmp->data;
			if (assoc->payee_number == payee_number)
			{
				if (assoc->search_str)
					g_free (assoc->search_str);
				assoc->search_str = g_strdup (rule);
				assoc->ignore_case = w_run->import_asso_case_insensitive;
				assoc->use_regex = w_run->import_asso_use_regex;
				break;
			}
	        list_tmp = list_tmp->next;
		}
		gsb_data_payee_set_search_string (payee_number, rule);
		gsb_data_payee_set_ignore_case (payee_number, w_run->import_asso_case_insensitive);
		gsb_data_payee_set_use_regex (payee_number, w_run->import_asso_use_regex);

		utils_prefs_gsb_file_set_modified ();
    }
}

/**
 * called when we press a key in the search str entry
 *
 * \param widget	entry for search string
 * \param ev
 * \param page
 *
 * \return FALSE
 **/
static gboolean widget_import_asso_key_press_event (GtkWidget *widget,
														GdkEventKey *ev,
												 		WidgetImportAsso *page)
{
	const gchar *tmp_str;

	switch (ev->keyval)
	{
		case GDK_KEY_KP_Enter :
		case GDK_KEY_Return :
			tmp_str = gtk_entry_get_text (GTK_ENTRY (widget));
			if (strlen (tmp_str) == 0)
				return FALSE;
			else
				widget_import_asso_update_assoc (tmp_str, page);
			return TRUE;
			break;

		default:
			break;
	}
    return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void widget_import_asso_setup_treeview_asso (WidgetImportAsso *page)
{
	WidgetImportAssoPrivate *priv;
    GtkListStore *list_store;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
	GtkCellRenderer *search_cell;
    GtkTreeSelection *selection;

	priv = widget_import_asso_get_instance_private (page);

	gtk_widget_set_name (priv->treeview_import_asso, "tree_view");

	/* create the model */
    list_store = gtk_list_store_new (ASSO_MODEL_N_COLUMNS,
									 G_TYPE_STRING,			/* ASSO_LIST_PAYEE_NAME */
									 G_TYPE_STRING,			/* ASSO_LIST_SEARCH_STR */
									 G_TYPE_INT,			/* ASSO_PAYEE_NUMBER */
									 GDK_TYPE_RGBA,			/* ASSO_BACKGROUND_COLOR */
									 G_TYPE_STRING,			/* ASSO_REAL_PAYEE_NAME */
									 G_TYPE_STRING);		/* ASSO_REAL_SEARCH_STR */

    /* create the treeview */
	gtk_tree_view_set_model (GTK_TREE_VIEW (priv->treeview_import_asso), GTK_TREE_MODEL (list_store));

	/* set the payee name column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Payee name"),
													   cell,
													   "text", ASSO_LIST_PAYEE_NAME,
													   "cell-background-rgba", ASSO_BACKGROUND_COLOR,
													   NULL);
    gtk_tree_view_column_set_expand (column, TRUE);
    gtk_tree_view_column_set_alignment (column, 0.5);
    gtk_tree_view_column_set_sort_column_id (column, 0);
    gtk_tree_view_append_column (GTK_TREE_VIEW (priv->treeview_import_asso), column);

    /* set the Search string column */
    search_cell = gtk_cell_renderer_text_new ();

    column = gtk_tree_view_column_new_with_attributes (_("Search string"),
													   search_cell,
													   "text", ASSO_LIST_SEARCH_STR,
													   "cell-background-rgba", ASSO_BACKGROUND_COLOR,
													   NULL);
    gtk_tree_view_column_set_expand (column, TRUE);
    gtk_tree_view_column_set_alignment (column, 0.5);
    gtk_tree_view_column_set_sort_column_id (column, 1);
    gtk_tree_view_append_column (GTK_TREE_VIEW (priv->treeview_import_asso), column);

	/* fill the list */
    widget_import_asso_fill_model (list_store);
	g_object_unref (list_store);

    utils_set_list_store_background_color (priv->treeview_import_asso, ASSO_BACKGROUND_COLOR);

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_import_asso));
	g_signal_connect (selection,
                      "changed",
					  G_CALLBACK (widget_import_asso_select_asso),
					  page);

	/* select the first item */
	path_selected = gtk_tree_path_new_first ();
	gtk_tree_selection_select_path (selection, path_selected);
	gtk_tree_path_free (path_selected);
	path_selected = NULL;
}

/**
 * Création de la page de gestion des import_asso
 *
 * \param prefs
 *
 * \return
 */
static void widget_import_asso_setup_import_asso_page (WidgetImportAsso *page)
{
	GtkWidget *entry;
	GSList *tmp_list;
	GrisbiWinEtat *w_etat;
	GrisbiWinRun *w_run;
	WidgetImportAssoPrivate *priv;

	devel_debug (NULL);
	priv = widget_import_asso_get_instance_private (page);
	w_etat = grisbi_win_get_w_etat ();
	w_run = grisbi_win_get_w_run ();

	/* set sensitive button_import_asso_remove */
    gtk_widget_set_sensitive (priv->button_import_asso_remove, FALSE);

	/* Create entry liste des tiers */
	tmp_list = gsb_data_payee_get_name_and_report_list();
	priv->combo_import_asso_payee = gtk_combofix_new_with_properties (tmp_list,
														   w_etat->combofix_force_payee,
														   !w_run->import_asso_case_insensitive,
														   FALSE,
														   METATREE_PAYEE);
	gsb_data_payee_free_name_and_report_list (tmp_list);
    gtk_combofix_set_text (GTK_COMBOFIX (priv->combo_import_asso_payee), "");
    gtk_widget_set_hexpand (priv->combo_import_asso_payee, TRUE);
    gtk_grid_attach (GTK_GRID (priv->grid_import_asso_details), priv->combo_import_asso_payee, 1, 0, 1, 1);

	/* Get combo_import_asso_payee entry */
	entry = gtk_combofix_get_entry (GTK_COMBOFIX (priv->combo_import_asso_payee));

	/* init entry search string */
    gtk_entry_set_text (GTK_ENTRY (priv->entry_import_asso_search_string), "");

	/* init check buttons case_insensitive and use regex */
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_import_asso_case_insensitive),
								  w_run->import_asso_case_insensitive);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_import_asso_use_regex),
								  w_run->import_asso_use_regex);
	gtk_widget_set_sensitive (priv->checkbutton_import_asso_use_regex, FALSE); /* unused */

	/* setup treeview_associations */
	widget_import_asso_setup_treeview_asso (page);
	widget_import_asso_check_add_button (page);

    /* set signal button "Add" */
    g_signal_connect (G_OBJECT (priv->button_import_asso_add),
                      "clicked",
                      G_CALLBACK  (widget_import_asso_add_button_clicked),
                      page);

    /* set signal button "Remove" */
    g_signal_connect (G_OBJECT (priv->button_import_asso_remove),
                       "clicked",
                       G_CALLBACK (widget_import_asso_del_assoc),
                       page);

    /* set signal combo_import_asso_payee entry */
    g_signal_connect (G_OBJECT (entry),
					  "changed",
					  G_CALLBACK (widget_import_asso_combo_changed),
					  page);

	/* set signal search string entry */
	g_signal_connect (G_OBJECT (priv->entry_import_asso_search_string),
					  "key-press-event",
					  G_CALLBACK (widget_import_asso_key_press_event),
					  page);

	/* Connect signal checkbutton_import_asso_case_insensitive */
    g_signal_connect (priv->checkbutton_import_asso_case_insensitive,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &w_run->import_asso_case_insensitive);
    g_signal_connect_after (priv->checkbutton_import_asso_case_insensitive,
							"toggled",
							G_CALLBACK (widget_import_asso_checkbutton_import_asso_case_insensitive_toggle),
							page);

	/* Connect signal checkbutton_import_asso_use_regex */
    g_signal_connect (priv->checkbutton_import_asso_use_regex,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &w_run->import_asso_use_regex);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
/**
 *
 *
 * \param
 *
 * \return
 **/
static void widget_import_asso_init (WidgetImportAsso *page)
{
	path_selected = NULL;

	gtk_widget_init_template (GTK_WIDGET (page));

	widget_import_asso_setup_import_asso_page (page);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void widget_import_asso_dispose (GObject *object)
{
	G_OBJECT_CLASS (widget_import_asso_parent_class)->dispose (object);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void widget_import_asso_class_init (WidgetImportAssoClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = widget_import_asso_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/widgets/widget_import_asso.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetImportAsso, vbox_import_asso);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetImportAsso, treeview_import_asso);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetImportAsso, grid_import_asso_details);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetImportAsso, entry_import_asso_search_string);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetImportAsso, button_import_asso_add);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetImportAsso, button_import_asso_remove);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetImportAsso, checkbutton_import_asso_case_insensitive);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetImportAsso, checkbutton_import_asso_use_regex);
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
WidgetImportAsso *widget_import_asso_new (GrisbiPrefs *win)
{
  return g_object_new (WIDGET_IMPORT_ASSO_TYPE, NULL);
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

