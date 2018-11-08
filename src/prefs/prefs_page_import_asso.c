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

#include <errno.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "prefs_page_import_asso.h"
#include "dialog.h"
#include "gsb_combo_box.h"
#include "gsb_data_payee.h"
#include "gsb_file.h"
#include "gsb_form_widget.h"
#include "gtk_combofix.h"
#include "import.h"
#include "structures.h"
#include "utils.h"
#include "utils_prefs.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _PrefsPageImportAssoPrivate   PrefsPageImportAssoPrivate;

struct _PrefsPageImportAssoPrivate
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

G_DEFINE_TYPE_WITH_PRIVATE (PrefsPageImportAsso, prefs_page_import_asso, GTK_TYPE_BOX)

static GtkTreePath *path_selected;

#define ASSO_BACKGROUND_COLOR 3
/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
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
static gboolean prefs_page_import_asso_select_row (GtkTreeModel *model,
												   GtkTreePath *path,
												   GtkTreeIter *iter,
												   gpointer data)
{
	gint payee_number;
	gint tmp_payee_number;

	payee_number = GPOINTER_TO_INT (data);
	gtk_tree_model_get (model, iter, 2, &tmp_payee_number, -1);
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
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void prefs_page_import_asso_foreach_asso (GtkTreeModel *model,
												 gint payee_number)
{
	gtk_tree_model_foreach (model,
							(GtkTreeModelForeachFunc) prefs_page_import_asso_select_row,
							GINT_TO_POINTER (payee_number));
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
static void prefs_page_import_asso_cell_edited (GtkCellRendererText *cell,
												const gchar *path_string,
												const gchar *new_text,
												PrefsPageImportAsso *page)
{
    GtkTreeModel *model;
    GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
    GtkTreeIter iter;
    GSList *list_tmp;
    gchar *search_str;
    gint payee_number;
	PrefsPageImportAssoPrivate *priv;

	priv = prefs_page_import_asso_get_instance_private (page);

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (priv->treeview_import_asso));
    gtk_tree_model_get_iter (model, &iter, path);
    gtk_tree_model_get (model, &iter, 1, &search_str, 2, &payee_number, -1);
    if (g_utf8_collate (search_str, new_text) != 0)
    {
        devel_debug (new_text);
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, 1, new_text, -1);

		list_tmp = gsb_import_associations_get_liste_associations ();
        while (list_tmp)
        {
            struct ImportPayeeAsso *assoc;

            assoc = list_tmp -> data;
            if (assoc -> payee_number == payee_number)
            {
                if (assoc -> search_str && strlen (assoc -> search_str))
                    g_free (assoc -> search_str);
                assoc -> search_str = g_strdup (new_text);
                gtk_entry_set_text (GTK_ENTRY (priv->entry_import_asso_search_string), new_text);
                break;
            }
            list_tmp = list_tmp -> next;
        }
        gsb_data_payee_set_search_string (payee_number, new_text);
    }
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static gboolean prefs_page_import_asso_check_add_button (PrefsPageImportAsso *page)
{
    gboolean sensitive = TRUE;
	PrefsPageImportAssoPrivate *priv;

	priv = prefs_page_import_asso_get_instance_private (page);

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
static void prefs_page_import_asso_combo_changed (GtkEditable *editable,
												  PrefsPageImportAsso *page)
{
    const gchar *tmp_str;
    gint payee_number;
	PrefsPageImportAssoPrivate *priv;

	priv = prefs_page_import_asso_get_instance_private (page);

    payee_number = gsb_data_payee_get_number_by_name (gtk_editable_get_chars
													  (editable, 0, -1),
													  FALSE);
    tmp_str = gsb_data_payee_get_search_string (payee_number);

    if (g_utf8_strlen (tmp_str, -1) == 0)
    {
        gtk_entry_set_text (GTK_ENTRY (priv->entry_import_asso_search_string), tmp_str);
        gtk_widget_set_sensitive (priv->entry_import_asso_search_string, TRUE);
    }
    else
    {
        gchar *str;

        str = g_strdup_printf (_("You cannot choose this payee because it "
								  "already has an association"));
        dialogue_warning (str);
        gtk_editable_delete_text (editable, 0, -1);
        if (strlen (gtk_entry_get_text (GTK_ENTRY (priv->entry_import_asso_search_string))))
        {
            gtk_entry_set_text (GTK_ENTRY (priv->entry_import_asso_search_string), "");
        }
        g_free (str);
    }

    /* on empeche la suppression par inadvertance d'une association */
    gtk_widget_set_sensitive (priv->button_import_asso_remove, FALSE);

    prefs_page_import_asso_check_add_button (page);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static gboolean prefs_page_import_asso_select_asso (GtkTreeSelection *selection,
													PrefsPageImportAsso *page)
{
    GtkTreeIter iter;
    gboolean good;
	PrefsPageImportAssoPrivate *priv;

	priv = prefs_page_import_asso_get_instance_private (page);
	good = gtk_tree_selection_get_selected (selection, NULL, &iter);

    if (good)
    {
		GtkWidget *entry;
		GtkTreeModel *model;
		gchar *payee_str;
		gchar *search_str;

		entry = gtk_combofix_get_entry (GTK_COMBOFIX (priv->combo_import_asso_payee));
		model = gtk_tree_view_get_model (GTK_TREE_VIEW (priv->treeview_import_asso));
		gtk_tree_model_get (model, &iter, 0, &payee_str, 1, &search_str, -1);

		g_signal_handlers_block_by_func (G_OBJECT (entry),
										 G_CALLBACK (prefs_page_import_asso_combo_changed),
										 page);
		gtk_combofix_set_text (GTK_COMBOFIX (priv->combo_import_asso_payee), payee_str);
		g_signal_handlers_unblock_by_func (G_OBJECT (entry),
										   G_CALLBACK (prefs_page_import_asso_combo_changed),
										   page);
		gtk_entry_set_text (GTK_ENTRY (priv->entry_import_asso_search_string), search_str);
		gtk_widget_set_sensitive (priv->entry_import_asso_search_string, FALSE);
		gtk_widget_set_sensitive (priv->button_import_asso_remove, TRUE);
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
static void prefs_page_import_asso_fill_model (GtkListStore *list_store)
{
    GSList *list_tmp;
    GtkTreeIter iter;

	list_tmp = gsb_import_associations_get_liste_associations ();
    gtk_list_store_clear (GTK_LIST_STORE (list_store));

    while (list_tmp)
    {
        gchar *tmp_str;
        struct ImportPayeeAsso *assoc;

        assoc = list_tmp->data;
        tmp_str = g_strdup (gsb_data_payee_get_name (assoc->payee_number, TRUE));
        gtk_list_store_append (GTK_LIST_STORE (list_store), &iter);
        gtk_list_store_set (GTK_LIST_STORE (list_store),
							&iter,
							0, tmp_str,
							1, assoc->search_str,
							2, assoc->payee_number,
							-1);
        g_free (tmp_str);
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
static void prefs_page_import_asso_del_assoc (GtkWidget *button,
											  PrefsPageImportAsso *page)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gint payee_number;
	PrefsPageImportAssoPrivate *priv;

	priv = prefs_page_import_asso_get_instance_private (page);

    if (!gtk_tree_selection_get_selected (gtk_tree_view_get_selection
										  (GTK_TREE_VIEW (priv->treeview_import_asso)),
										  NULL,
										  &iter))
	{
		return;
	}

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (priv->treeview_import_asso));
    gtk_tree_model_get (model, &iter, 2, &payee_number, -1);
    if (payee_number > 0)
    {
		gsb_import_associations_remove_assoc (payee_number);
		gtk_combofix_set_text (GTK_COMBOFIX (priv->combo_import_asso_payee), "");
		gtk_entry_set_text (GTK_ENTRY (priv->entry_import_asso_search_string), "");
		prefs_page_import_asso_fill_model (GTK_LIST_STORE (model));
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
static void prefs_page_import_asso_add_assoc (GtkWidget *button,
											  PrefsPageImportAsso *page)
{
    gchar *payee = NULL;
    gchar *search_str = NULL;
    gint payee_number;
	gint ignore_case = 0;
	gint use_regex = 0;
	gboolean result = FALSE;
	static gboolean etat = FALSE;
	PrefsPageImportAssoPrivate *priv;

	priv = prefs_page_import_asso_get_instance_private (page);

	payee = g_strstrip (g_strdup (gtk_combofix_get_text (GTK_COMBOFIX (priv->combo_import_asso_payee))));
    search_str = g_strstrip (g_strdup (gtk_entry_get_text (GTK_ENTRY (priv->entry_import_asso_search_string))));
    if (!etat && (strlen (payee) > 0 || strlen (search_str) > 0))
	{
		gtk_combofix_set_text (GTK_COMBOFIX (priv->combo_import_asso_payee), "");
		gtk_entry_set_text (GTK_ENTRY (priv->entry_import_asso_search_string), "");
		etat = TRUE;

        return;
	}

	etat = FALSE;

    payee_number = gsb_data_payee_get_number_by_name  (payee, TRUE);
	g_free(payee);

	result = gsb_import_associations_add_assoc (payee_number, search_str, ignore_case, use_regex);
	if (result)
	{
		GtkTreeModel *model;
		GtkTreeSelection *selection;

		model = gtk_tree_view_get_model (GTK_TREE_VIEW (priv->treeview_import_asso));
        prefs_page_import_asso_fill_model (GTK_LIST_STORE (model));
		prefs_page_import_asso_foreach_asso (model, payee_number);
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_import_asso));
		gtk_tree_selection_select_path (selection, path_selected);
		gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (priv->treeview_import_asso), path_selected, NULL, FALSE, 0,0);
		gtk_tree_path_free (path_selected);
		path_selected = NULL;
		utils_set_list_store_background_color (priv->treeview_import_asso, ASSO_BACKGROUND_COLOR);

		utils_prefs_gsb_file_set_modified ();
	}
	g_free(search_str);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void prefs_page_import_asso_setup_treeview_asso (PrefsPageImportAsso *page)
{
	PrefsPageImportAssoPrivate *priv;
    GtkListStore *list_store;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkTreeSelection *selection;

	priv = prefs_page_import_asso_get_instance_private (page);

	gtk_widget_set_name (priv->treeview_import_asso, "tree_view");

	/* create the model */
    list_store = gtk_list_store_new (4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, GDK_TYPE_RGBA);

    /* create the treeview */
	gtk_tree_view_set_model (GTK_TREE_VIEW (priv->treeview_import_asso), GTK_TREE_MODEL (list_store));
    g_object_unref (list_store);

	/* set the payee name column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Payee name"),
													   cell,
													   "text", 0,
													   "cell-background-rgba", ASSO_BACKGROUND_COLOR,
													   NULL);
    gtk_tree_view_column_set_expand (column, TRUE);
    gtk_tree_view_column_set_alignment (column, 0.5);
    gtk_tree_view_column_set_sort_column_id (column, 0);
    gtk_tree_view_append_column (GTK_TREE_VIEW (priv->treeview_import_asso), column);

    /* set the Search string column */
    cell = gtk_cell_renderer_text_new ();
    g_object_set (cell, "editable", TRUE, NULL);
    g_signal_connect (cell,
					  "edited",
					  G_CALLBACK (prefs_page_import_asso_cell_edited),
					  page);

    column = gtk_tree_view_column_new_with_attributes (_("Search string"),
													   cell,
													   "text", 1,
													   "cell-background-rgba", ASSO_BACKGROUND_COLOR,
													   NULL);
    gtk_tree_view_column_set_expand (column, TRUE);
    gtk_tree_view_column_set_alignment (column, 0.5);
    gtk_tree_view_column_set_sort_column_id (column, 1);
    gtk_tree_view_append_column (GTK_TREE_VIEW (priv->treeview_import_asso), column);

	/* fill the list */
    prefs_page_import_asso_fill_model (list_store);

    utils_set_list_store_background_color (priv->treeview_import_asso, ASSO_BACKGROUND_COLOR);

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_import_asso));
	g_signal_connect (selection,
                      "changed",
					  G_CALLBACK (prefs_page_import_asso_select_asso),
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
static void prefs_page_import_asso_setup_import_asso_page (PrefsPageImportAsso *page)
{
	GtkWidget *entry;
	GSList *tmp_list;
	GrisbiWinRun *w_run;
	PrefsPageImportAssoPrivate *priv;

	devel_debug (NULL);

	priv = prefs_page_import_asso_get_instance_private (page);
	w_run = grisbi_win_get_w_run ();

    /* set signal button "Add" */
    g_signal_connect (G_OBJECT (priv->button_import_asso_add),
                      "clicked",
                      G_CALLBACK  (prefs_page_import_asso_add_assoc),
                      page);

    /* set signal button "Remove" */
    g_signal_connect (G_OBJECT (priv->button_import_asso_remove),
                       "clicked",
                       G_CALLBACK (prefs_page_import_asso_del_assoc),
                       page);
    gtk_widget_set_sensitive (priv->button_import_asso_remove, FALSE);

	/* Create entry liste des tiers */
	tmp_list = gsb_data_payee_get_name_and_report_list();
	priv->combo_import_asso_payee = gtk_combofix_new_with_properties (tmp_list,
														   etat.combofix_force_payee,
														   etat.combofix_max_item,
														   !w_run->import_asso_case_insensitive,
														   FALSE,
														   METATREE_PAYEE);
	gsb_data_payee_free_name_and_report_list (tmp_list);
    gsb_form_widget_combo_entry_set_text (priv->combo_import_asso_payee, "");
    gtk_widget_set_hexpand (priv->combo_import_asso_payee, TRUE);
    gtk_grid_attach (GTK_GRID (priv->grid_import_asso_details), priv->combo_import_asso_payee, 1, 0, 1, 1);

	/* Connect signal combo_import_asso_payee entry */
	entry = gtk_combofix_get_entry (GTK_COMBOFIX (priv->combo_import_asso_payee));
    g_signal_connect (G_OBJECT (entry),
					  "changed",
					  G_CALLBACK (prefs_page_import_asso_combo_changed),
					  page);

    /* init entry search string */
    gtk_entry_set_text (GTK_ENTRY (priv->entry_import_asso_search_string), "");
    g_signal_connect_swapped (priv->entry_import_asso_search_string,
							  "changed",
							  G_CALLBACK (prefs_page_import_asso_check_add_button),
							  page);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_import_asso_case_insensitive),
								  w_run->import_asso_case_insensitive);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_import_asso_use_regex),
								  w_run->import_asso_use_regex);

	/* Connect signal checkbutton_import_asso_case_insensitive */
    g_signal_connect (priv->checkbutton_import_asso_case_insensitive,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &w_run->import_asso_case_insensitive);

	/* Connect signal checkbutton_import_asso_use_regex */
    g_signal_connect (priv->checkbutton_import_asso_use_regex,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &w_run->import_asso_use_regex);

	/* setup treeview_associations */
	prefs_page_import_asso_setup_treeview_asso (page);

	prefs_page_import_asso_check_add_button (page);
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
static void prefs_page_import_asso_init (PrefsPageImportAsso *page)
{
	path_selected = NULL;

	gtk_widget_init_template (GTK_WIDGET (page));

	prefs_page_import_asso_setup_import_asso_page (page);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void prefs_page_import_asso_dispose (GObject *object)
{
	G_OBJECT_CLASS (prefs_page_import_asso_parent_class)->dispose (object);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void prefs_page_import_asso_class_init (PrefsPageImportAssoClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = prefs_page_import_asso_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/ui/prefs_page_import_asso.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageImportAsso, vbox_import_asso);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageImportAsso, treeview_import_asso);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageImportAsso, grid_import_asso_details);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageImportAsso, entry_import_asso_search_string);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageImportAsso, button_import_asso_add);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageImportAsso, button_import_asso_remove);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageImportAsso, checkbutton_import_asso_case_insensitive);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageImportAsso, checkbutton_import_asso_use_regex);
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
PrefsPageImportAsso *prefs_page_import_asso_new (GrisbiPrefs *win)
{
  return g_object_new (PREFS_PAGE_IMPORT_ASSO_TYPE, NULL);
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

