/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2021 Pierre Biava (grisbi@pierre.biava.name)                    */
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

#include <errno.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "widget_list_metatree.h"
#include "dialog.h"
#include "etats_config.h"
#include "etats_prefs.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_report.h"
#include "gsb_file.h"
#include "structures.h"
#include "utils_buttons.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _WidgetListMetatreePrivate   WidgetListMetatreePrivate;

struct _WidgetListMetatreePrivate
{
	GtkWidget *			vbox_list_metatree;

	GtkWidget *			button_income_metatree;
	GtkWidget *			button_outgoing_metatree;
	GtkWidget *			hbox_toggle_collapse_metatree;
	GtkWidget *			hbox_toggle_expand_metatree;
	GtkWidget *			label_income_metatree;
	GtkWidget *			label_outgoing_metatree;
	GtkWidget *			togglebutton_expand_metatree;
	GtkWidget *			togglebutton_select_all_metatree;
	GtkWidget *			treeview_list_metatree;

	/* sert à generer un message dans la fonction appellante */
	gboolean			all_selected;
};

G_DEFINE_TYPE_WITH_PRIVATE (WidgetListMetatree, widget_list_metatree, GTK_TYPE_BOX)

/* the def of the columns in the categ and budget list to filter by categ and budget */
enum
{
	GSB_ETAT_CATEG_BUDGET_LIST_NAME = 0,
	GSB_ETAT_CATEG_BUDGET_LIST_ACTIVE,
	GSB_ETAT_CATEG_BUDGET_LIST_ACTIVATABLE,
	GSB_ETAT_CATEG_BUDGET_LIST_NUMBER,
	GSB_ETAT_CATEG_BUDGET_LIST_SUB_NUMBER,
	GSB_ETAT_CATEG_BUDGET_LIST_NB
};

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * initialise le bouton expand collapse all
 *
 * \param priv structure
 *
 * \return
 **/
static void widget_list_metatree_init_toggle_button_expand (WidgetListMetatreePrivate *priv)
{
	/* set data for calbback function */
	g_object_set_data (G_OBJECT (priv->togglebutton_expand_metatree),
					   "hbox_expand",
					   priv->hbox_toggle_expand_metatree);
	g_object_set_data (G_OBJECT (priv->togglebutton_expand_metatree),
					   "hbox_collapse",
					   priv->hbox_toggle_collapse_metatree);

	/* set signal */
	g_signal_connect (G_OBJECT (priv->togglebutton_expand_metatree),
					  "clicked",
					  G_CALLBACK (utils_togglebutton_collapse_expand_all_rows),
					  priv->treeview_list_metatree);
}

/**
 * check or uncheck all the budgets or categories in the model
 *
 * \param model         the model to fill (is model_categ or model_budget
 * \param select_ptr    TRUE or FALSE to select/unselect all
 *
 * \return FALSE
 **/
static void widget_list_metatree_check_uncheck_all (GtkTreeModel *model,
													gboolean toggle_value)
{
	GtkTreeIter parent_iter;

	if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &parent_iter))
		return;

	do
	{
		GtkTreeIter iter_children;

		gtk_tree_store_set (GTK_TREE_STORE (model),
							&parent_iter,
							GSB_ETAT_CATEG_BUDGET_LIST_ACTIVE, toggle_value,
							-1);

		if (gtk_tree_model_iter_children (GTK_TREE_MODEL (model), &iter_children, &parent_iter))
		{
			/* we are on the children */
			do
				gtk_tree_store_set (GTK_TREE_STORE (model),
									&iter_children,
									GSB_ETAT_CATEG_BUDGET_LIST_ACTIVE, toggle_value,
									-1);
			while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter_children));
		}
	}
	while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &parent_iter));
}

/**
 * coche ou décoche toutes les cases du tree_view
 *
 * \param toggle_button
 * \param tree_view
 *
 * \return
 **/
static void widget_list_metatree_button_uncheck_all_toggled (GtkToggleButton *togglebutton,
															 GtkWidget *tree_view)
{
	GtkTreeModel *model;
	gchar *label;
	gboolean toggle;

	toggle = gtk_toggle_button_get_active (togglebutton);
	model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

	widget_list_metatree_check_uncheck_all (model, toggle);

	if ((toggle))
		label = g_strdup (_("Unselect all"));
	else
		label = g_strdup (_("Select all"));

	gtk_button_set_label (GTK_BUTTON (togglebutton), label);
	g_free (label);
}

/**
 * retourne la liste des lignes sélectionnées
 *
 * \param priv structure
 *
 * \return a GSList of CategBudgetSel
 **/
static GSList *widget_list_metatree_get_selected (WidgetListMetatreePrivate *priv)
{
	GtkTreeModel *model;
	GtkTreeIter parent_iter;
	GSList *tmp_list = NULL;

	/* on récupère le modèle */
	model = gtk_tree_view_get_model (GTK_TREE_VIEW (priv->treeview_list_metatree));

	if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &parent_iter))
		return NULL;

	priv->all_selected = TRUE;
	do
	{
		gint div_number;
		gboolean active;
		CategBudgetSel *categ_budget_struct;
		GtkTreeIter iter_children;

		gtk_tree_model_get (GTK_TREE_MODEL (model),
							&parent_iter,
							GSB_ETAT_CATEG_BUDGET_LIST_NUMBER, &div_number,
							GSB_ETAT_CATEG_BUDGET_LIST_ACTIVE, &active,
							-1);
		if (!active)
		{
			priv->all_selected = FALSE;
			continue;
		}

		/* ok, we are on a selected category/budget, create and fill the structure */
		categ_budget_struct = g_malloc0 (sizeof (CategBudgetSel));

		categ_budget_struct->div_number = div_number;
		tmp_list = g_slist_append (tmp_list, categ_budget_struct);

		/* check the children */
		if (gtk_tree_model_iter_children (GTK_TREE_MODEL (model), &iter_children, &parent_iter))
		{
			/* we are on the children */
			do
			{
				gint sub_div_number;

				gtk_tree_model_get (GTK_TREE_MODEL (model),
									&iter_children,
									GSB_ETAT_CATEG_BUDGET_LIST_SUB_NUMBER, &sub_div_number,
									GSB_ETAT_CATEG_BUDGET_LIST_ACTIVE, &active,
									-1);
				if (active)
					categ_budget_struct->sub_div_numbers = g_slist_append (categ_budget_struct->sub_div_numbers,
																		   GINT_TO_POINTER (sub_div_number));
				else
					priv->all_selected = FALSE;
			}
			while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter_children));
		}
	}
	while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &parent_iter));

	return tmp_list;
}

/**
 *
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
static gboolean widget_list_metatree_select_partie_liste (GtkWidget *button,
														  GdkEventButton *event,
														  GtkWidget *tree_view)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gboolean is_categ;
	gboolean type_div;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
	if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
		return FALSE;

	widget_list_metatree_check_uncheck_all (model, FALSE);
	is_categ = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (button), "is_categ"));
	type_div = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (button), "type_div"));

	do
	{
		gint div_number;

		gtk_tree_model_get (GTK_TREE_MODEL (model),
							&iter,
							GSB_ETAT_CATEG_BUDGET_LIST_NUMBER, &div_number,
							-1);

		/* we always select without categories/budget ?? set yes by default */
		if (!div_number
		 ||
		 (is_categ && (gsb_data_category_get_type (div_number) == type_div))
		 ||
		 (!is_categ && (gsb_data_budget_get_type (div_number) == type_div)))
		{
			GtkTreeIter iter_children;

			gtk_tree_store_set (GTK_TREE_STORE (model),
								&iter,
								GSB_ETAT_CATEG_BUDGET_LIST_ACTIVE, TRUE,
								-1);

			if (gtk_tree_model_iter_children (GTK_TREE_MODEL (model), &iter_children, &iter))
			{
				/* we are on the children */
				do
					gtk_tree_store_set (GTK_TREE_STORE (model),
										&iter_children,
										GSB_ETAT_CATEG_BUDGET_LIST_ACTIVE, TRUE,
										-1);

				while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter_children));
			}
		}
	}
	while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));

	return FALSE;
}

/**
 * initialisation des boutons de sélection des catégories
 *
 * \param
 * \param
 *
 * \return
 **/
static void widget_list_metatree_init_buttons_select_unselect (WidgetListMetatreePrivate *priv,
															   gint type_page)
{
	devel_debug_int (type_page);
	if (type_page == BUDGET_PAGE_TYPE)
	{
		gtk_label_set_text (GTK_LABEL (priv->label_income_metatree), _("Income budget lines"));
		gtk_label_set_text (GTK_LABEL (priv->label_outgoing_metatree), _("Outgoing budget lines"));
		g_object_set_data (G_OBJECT (priv->button_income_metatree), "is_categ", GINT_TO_POINTER (FALSE));
		g_object_set_data (G_OBJECT (priv->button_income_metatree), "type_div", GINT_TO_POINTER (FALSE));
		g_object_set_data (G_OBJECT (priv->button_outgoing_metatree), "is_categ", GINT_TO_POINTER (FALSE));
		g_object_set_data (G_OBJECT (priv->button_outgoing_metatree), "type_div", GINT_TO_POINTER (TRUE));
	}
	else
	{
		gtk_label_set_text (GTK_LABEL (priv->label_income_metatree), _("Income categories"));
		gtk_label_set_text (GTK_LABEL (priv->label_outgoing_metatree), _("Outgoing categories"));
		g_object_set_data (G_OBJECT (priv->button_income_metatree), "is_categ", GINT_TO_POINTER (TRUE));
		g_object_set_data (G_OBJECT (priv->button_income_metatree), "type_div", GINT_TO_POINTER (FALSE));
		g_object_set_data (G_OBJECT (priv->button_outgoing_metatree), "is_categ", GINT_TO_POINTER (TRUE));
		g_object_set_data (G_OBJECT (priv->button_outgoing_metatree), "type_div", GINT_TO_POINTER (TRUE));
	}

	g_signal_connect (G_OBJECT  (priv->togglebutton_select_all_metatree),
					  "toggled",
					  G_CALLBACK (widget_list_metatree_button_uncheck_all_toggled),
					  priv->treeview_list_metatree);

	g_signal_connect (G_OBJECT  (priv->button_income_metatree),
					  "button-press-event",
					  G_CALLBACK (widget_list_metatree_select_partie_liste),
					  priv->treeview_list_metatree);

	g_signal_connect (G_OBJECT  (priv->button_outgoing_metatree),
					  "button-press-event",
					  G_CALLBACK (widget_list_metatree_select_partie_liste),
					  priv->treeview_list_metatree);
}

/**
 * callback if we toggle a checkbox in the category/budget list
 * if we toggle a div, toggle all the sub-div
 * if we toggle a sub-div, toggle also the div
 *
 * \param radio_renderer
 * \param path          the string of path
 * \param store         the GtkTreeStore of categ/budget
 *
 * \return FALSE
 **/
static gboolean widget_list_metatree_radio_button_toggled (GtkCellRendererToggle *radio_renderer,
														   gchar *path_str,
														   GtkTreeStore *store)
{
	GtkTreePath *path;
	GtkTreeIter iter;
	GtkTreeIter iter_children;
	gboolean toggle_value;

	g_return_val_if_fail (path_str != NULL, FALSE);
	g_return_val_if_fail (store != NULL && GTK_IS_TREE_STORE (store), FALSE);

	/* first get the iter and the value of the checkbutton */
	path = gtk_tree_path_new_from_string (path_str);
	gtk_tree_model_get_iter (GTK_TREE_MODEL (store), &iter, path);
	gtk_tree_model_get (GTK_TREE_MODEL (store),
						&iter,
						GSB_ETAT_CATEG_BUDGET_LIST_ACTIVE, &toggle_value,
						-1);

	/* ok, we invert the button */
	toggle_value = !toggle_value;

	gtk_tree_store_set (GTK_TREE_STORE (store),
						&iter,
						GSB_ETAT_CATEG_BUDGET_LIST_ACTIVE, toggle_value,
						-1);

	/* if we are on a mother, we set the same value to all the children */
	if (gtk_tree_model_iter_children (GTK_TREE_MODEL (store), &iter_children, &iter))
	{
		/* we are on the children */
		do
			gtk_tree_store_set (GTK_TREE_STORE (store),
								&iter_children,
								GSB_ETAT_CATEG_BUDGET_LIST_ACTIVE, toggle_value,
								-1);
		while (gtk_tree_model_iter_next (GTK_TREE_MODEL (store), &iter_children));
	}

	/* if we are activating a child, activate the mother */
	if (toggle_value
		&&
		gtk_tree_model_iter_parent (GTK_TREE_MODEL (store), &iter_children, &iter))
		gtk_tree_store_set (GTK_TREE_STORE (store),
							&iter_children,
							GSB_ETAT_CATEG_BUDGET_LIST_ACTIVE, toggle_value,
							-1);

	gtk_tree_path_free (path);

	return FALSE;
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
static gint widget_list_metatree_sort_function (GtkTreeModel *model,
												GtkTreeIter *iter_1,
												GtkTreeIter *iter_2,
												gpointer ptr)
{
	gchar *name_1;
	gchar *name_2;
	gint number_1;
	gint number_2;
	gint sub_number_1;
	gint sub_number_2;
	gint return_value = 0;

	gtk_tree_model_get (model,
						iter_1,
						GSB_ETAT_CATEG_BUDGET_LIST_NAME, &name_1,
						GSB_ETAT_CATEG_BUDGET_LIST_NUMBER, &number_1,
						GSB_ETAT_CATEG_BUDGET_LIST_SUB_NUMBER, &sub_number_1,
						-1);

	gtk_tree_model_get (model,
						iter_2,
						GSB_ETAT_CATEG_BUDGET_LIST_NAME, &name_2,
						GSB_ETAT_CATEG_BUDGET_LIST_NUMBER, &number_2,
						GSB_ETAT_CATEG_BUDGET_LIST_SUB_NUMBER, &sub_number_2,
						-1);

	/* on compare d'abord les divisions */
	if (number_1 != -1 && number_2 != -1 && number_1 - number_2)
	{
		if (number_1 == 0)
			return_value = -1;
		else if (number_2 == 0)
			return_value = 1;
		else
			return_value =  strcmp (g_utf8_collate_key (name_1, -1), g_utf8_collate_key (name_2, -1));

		if (name_1)
		{
			g_free (name_1);
			name_1 = NULL;
		}
		if (name_2)
		{
			g_free (name_2);
			name_2 = NULL;
		}
	}

	if (return_value)
		return return_value;

	/* puis les sous-divisions */
	if (sub_number_1 == 0)
		return_value = -1;
	else if (sub_number_2 == 0)
		return_value = 1;
	else
		return_value =  strcmp (g_utf8_collate_key (name_1, -1), g_utf8_collate_key (name_2, -1));

	if (name_1) g_free (name_1);
	if (name_2) g_free (name_2);

	return return_value;
}

/**
 * fill the categories selection list for report
 *
 * \param model
 * \param type_page		BUDGET_PAGE_TYPE,CATEGORY_PAGE_TYPE
 *
 * \return FALSE
 **/
static void widget_list_metatree_fill_model (GtkTreeModel *model,
											 gint type_page)
{
	GtkTreeIter parent_iter;
	GtkTreeIter child_iter;
	GSList *list_tmp;
	gchar *without_name;
	gchar *without_sub_name;

	if (type_page == BUDGET_PAGE_TYPE)
	{
		list_tmp = gsb_data_budget_get_budgets_list ();
	    gtk_tree_store_clear (GTK_TREE_STORE (model));

		without_name = _("No budgetary line");
		without_sub_name = _("No sub-budgetary line");
	}
	else if (type_page == CATEGORY_PAGE_TYPE)
	{
		list_tmp = gsb_data_category_get_categories_list ();
		gtk_tree_store_clear (GTK_TREE_STORE (model));

		without_name = _("No category");
		without_sub_name = _("No subcategory");
	}
	else
		return;

	while (list_tmp)
	{
		gchar *name;
		gint div_number;
		GSList *tmp_list_sub_div;

		/* get div to append */
		if (type_page == BUDGET_PAGE_TYPE)
		{
			div_number = gsb_data_budget_get_no_budget (list_tmp-> data);
			name = gsb_data_budget_get_name (div_number, 0, NULL);
			tmp_list_sub_div = gsb_data_budget_get_sub_budget_list (div_number);
		}
		else
		{
			div_number = gsb_data_category_get_no_category (list_tmp-> data);
			name = gsb_data_category_get_name (div_number, 0, NULL);
			tmp_list_sub_div = gsb_data_category_get_sub_category_list (div_number);
		}

		if (name)
		{
			/* append to the model */
				gtk_tree_store_append (GTK_TREE_STORE (model), &parent_iter, NULL);
				gtk_tree_store_set (GTK_TREE_STORE (model),
									&parent_iter,
									GSB_ETAT_CATEG_BUDGET_LIST_NAME, name,
									GSB_ETAT_CATEG_BUDGET_LIST_NUMBER, div_number,
									GSB_ETAT_CATEG_BUDGET_LIST_SUB_NUMBER, -1,
									GSB_ETAT_CATEG_BUDGET_LIST_ACTIVATABLE, TRUE,
									-1);

			g_free (name);

			/* append sub_div */
			while (tmp_list_sub_div)
			{
				gint sub_div_number;

				if (type_page == BUDGET_PAGE_TYPE)
				{
					sub_div_number = gsb_data_budget_get_no_sub_budget (tmp_list_sub_div-> data);
					name = gsb_data_budget_get_sub_budget_name (div_number, sub_div_number, NULL);
				}
				else
				{
					sub_div_number = gsb_data_category_get_no_sub_category (tmp_list_sub_div-> data);
					name = gsb_data_category_get_sub_category_name (div_number, sub_div_number, NULL);
				}

				if (!name)
					name = g_strdup (_("Not available"));

				/* append to the model */
				gtk_tree_store_append (GTK_TREE_STORE (model), &child_iter, &parent_iter);
				gtk_tree_store_set (GTK_TREE_STORE (model),
									&child_iter,
									GSB_ETAT_CATEG_BUDGET_LIST_NAME, name,
									GSB_ETAT_CATEG_BUDGET_LIST_NUMBER, -1,
									GSB_ETAT_CATEG_BUDGET_LIST_SUB_NUMBER, sub_div_number,
									GSB_ETAT_CATEG_BUDGET_LIST_ACTIVATABLE, TRUE,
									-1);

				g_free (name);

				tmp_list_sub_div = tmp_list_sub_div-> next;
			}
		}
		/* append without sub-div */
			gtk_tree_store_append (GTK_TREE_STORE (model), &child_iter, &parent_iter);
			gtk_tree_store_set (GTK_TREE_STORE (model),
								&child_iter,
								GSB_ETAT_CATEG_BUDGET_LIST_NAME, without_sub_name,
								GSB_ETAT_CATEG_BUDGET_LIST_NUMBER, -1,
								GSB_ETAT_CATEG_BUDGET_LIST_SUB_NUMBER, 0,
								GSB_ETAT_CATEG_BUDGET_LIST_ACTIVATABLE, TRUE,
								-1);

		list_tmp = list_tmp-> next;
	}

	/* append without div and sub-div*/
	gtk_tree_store_append (GTK_TREE_STORE (model), &parent_iter, NULL);
	gtk_tree_store_set (GTK_TREE_STORE (model),
						&parent_iter,
						GSB_ETAT_CATEG_BUDGET_LIST_NAME, without_name,
						GSB_ETAT_CATEG_BUDGET_LIST_NUMBER, 0,
						GSB_ETAT_CATEG_BUDGET_LIST_SUB_NUMBER, -1,
						GSB_ETAT_CATEG_BUDGET_LIST_ACTIVATABLE, TRUE,
						GSB_ETAT_CATEG_BUDGET_LIST_ACTIVE, TRUE,
						-1);

	gtk_tree_store_append (GTK_TREE_STORE (model), &child_iter, &parent_iter);
	gtk_tree_store_set (GTK_TREE_STORE (model),
						&child_iter,
						GSB_ETAT_CATEG_BUDGET_LIST_NAME, without_sub_name,
						GSB_ETAT_CATEG_BUDGET_LIST_NUMBER, -1,
						GSB_ETAT_CATEG_BUDGET_LIST_SUB_NUMBER, 0,
						GSB_ETAT_CATEG_BUDGET_LIST_ACTIVATABLE, TRUE,
						GSB_ETAT_CATEG_BUDGET_LIST_ACTIVE, TRUE,
						-1);
}

/**
 * création du model
 *
 * \param type_page
 *
 * \return a GtkWidget : the GtkTreeView
 **/
static GtkTreeModel *widget_list_metatree_get_model (gint type_page)
{
	GtkTreeStore *store;

	store = gtk_tree_store_new (GSB_ETAT_CATEG_BUDGET_LIST_NB,
								G_TYPE_STRING,			/* GSB_ETAT_CATEG_BUDGET_LIST_NAME */
								G_TYPE_BOOLEAN,			/* GSB_ETAT_CATEG_BUDGET_LIST_ACTIVE */
								G_TYPE_BOOLEAN,			/* GSB_ETAT_CATEG_BUDGET_LIST_ACTIVATABLE */
								G_TYPE_INT,				/* GSB_ETAT_CATEG_BUDGET_LIST_NUMBER */
								G_TYPE_INT);			/* GSB_ETAT_CATEG_BUDGET_LIST_SUB_NUMBER */

	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (store),
										  GSB_ETAT_CATEG_BUDGET_LIST_NAME,
										  GTK_SORT_ASCENDING);

	if (type_page == BUDGET_PAGE_TYPE)
	{
		gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (store),
										 GSB_ETAT_CATEG_BUDGET_LIST_NAME,
										 (GtkTreeIterCompareFunc) widget_list_metatree_sort_function,
										 GINT_TO_POINTER (FALSE),
										 NULL);
		widget_list_metatree_fill_model (GTK_TREE_MODEL (store), BUDGET_PAGE_TYPE);
	}
	else if (type_page == CATEGORY_PAGE_TYPE)
	{
		gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (store),
										 GSB_ETAT_CATEG_BUDGET_LIST_NAME,
										 (GtkTreeIterCompareFunc) widget_list_metatree_sort_function,
										 GINT_TO_POINTER (TRUE),
										 NULL);

		widget_list_metatree_fill_model (GTK_TREE_MODEL (store), CATEGORY_PAGE_TYPE);
	}
	else
		return NULL;

	/* return */
	return GTK_TREE_MODEL (store);
}

/**
 * force la selection du premier item de la liste
 *
 * \param model
 *
 * \return
 **/
static void widget_list_metatree_select_without_metatree (GtkTreeModel *model)
{
	GtkTreeIter parent_iter;

	if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &parent_iter))
		return;

	gtk_tree_store_set (GTK_TREE_STORE (model),
						&parent_iter,
						GSB_ETAT_CATEG_BUDGET_LIST_ACTIVE, TRUE,
						-1);
}

/**
 * Initialisation du tree_view
 *
 * \param treeview_name
 * \param tmp_list
 *
 * \return
 **/
static void widget_list_metatree_init_treeview (GtkWidget *tree_view,
												GSList *tmp_list)
{
	GtkTreeModel *model;
	GtkTreeIter parent_iter;

	/* on récupère le modèle */
	model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

	widget_list_metatree_check_uncheck_all (model, FALSE);

	while (tmp_list)
	{
		CategBudgetSel *categ_budget_struct = tmp_list->data;

		if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &parent_iter))
			return;
		do
		{
			gint div_number;

			gtk_tree_model_get (GTK_TREE_MODEL (model),
								&parent_iter,
								GSB_ETAT_CATEG_BUDGET_LIST_NUMBER, &div_number,
								-1);

			/* we check the children only if the category is selected */
			if (div_number == categ_budget_struct->div_number)
			{
				gtk_tree_store_set (GTK_TREE_STORE (model),
								&parent_iter,
								GSB_ETAT_CATEG_BUDGET_LIST_ACTIVE, TRUE,
								-1);

				if (categ_budget_struct->sub_div_numbers)
				{
					GtkTreeIter iter_child;

					if (gtk_tree_model_iter_children (GTK_TREE_MODEL (model), &iter_child, &parent_iter))
					{
						do
						{
							gint sub_div_number;

							gtk_tree_model_get (GTK_TREE_MODEL (model),
												&iter_child,
												GSB_ETAT_CATEG_BUDGET_LIST_SUB_NUMBER, &sub_div_number,
												-1);

							if  (g_slist_find (categ_budget_struct->sub_div_numbers,
							 GINT_TO_POINTER (sub_div_number)))
								gtk_tree_store_set (GTK_TREE_STORE (model),
												&iter_child,
												GSB_ETAT_CATEG_BUDGET_LIST_ACTIVE, TRUE,
												-1);
						}
						while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter_child));
					}
				}
				/* we have found the category, can stop here */
				break;
			}
		}
		while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &parent_iter));

		tmp_list = tmp_list->next;
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
static void widget_list_metatree_setup_treeview (WidgetListMetatree *widget,
												 gint type_page)
{
	GtkCellRenderer *cell_renderer;
	GtkCellRenderer *radio_renderer;
	GtkTreeModel *model = NULL;
	GtkTreeSelection *selection;
	GtkTreeViewColumn *column;
	WidgetListMetatreePrivate *priv;

	priv = widget_list_metatree_get_instance_private (widget);

	/* set le model */
	model = widget_list_metatree_get_model (type_page);

	if (!model)
		return;

	/* on sélectionne par défaut le premier item */
	widget_list_metatree_select_without_metatree (model);
	gtk_tree_view_set_model (GTK_TREE_VIEW (priv->treeview_list_metatree), model);
	g_object_unref (G_OBJECT (model));

	/* set the color of selected row */
	gtk_widget_set_name (priv->treeview_list_metatree, "tree_view");

	/* create the column */
	column = gtk_tree_view_column_new ();

	/* create the toggle button part */
	radio_renderer = gtk_cell_renderer_toggle_new ();
	g_object_set (G_OBJECT (radio_renderer), "xalign", 0.0, NULL);
	gtk_tree_view_column_pack_start (column, radio_renderer, FALSE);
	gtk_tree_view_column_set_attributes (column, radio_renderer, "active", 1, "activatable", 2, NULL);
	g_signal_connect (G_OBJECT (radio_renderer),
					  "toggled",
					  G_CALLBACK (widget_list_metatree_radio_button_toggled),
					  model);

	/* create the text part */
	cell_renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (cell_renderer), "xalign", 0.0, NULL);
	gtk_tree_view_column_pack_start (column, cell_renderer, TRUE);
	gtk_tree_view_column_set_attributes (column, cell_renderer, "text", 0, NULL);

	/* append the column */
	gtk_tree_view_append_column (GTK_TREE_VIEW (priv->treeview_list_metatree), column);
	gtk_tree_view_column_set_resizable (column, TRUE);

	/* set selection */
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_list_metatree));
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);
}

/**
 * Création de la page de gestion des list_metatree
 *
 * \param
 *
 * \return
 **/
static void widget_list_metatree_setup_widget (WidgetListMetatree *widget,
											   gint type_page)
{
	WidgetListMetatreePrivate *priv;

	devel_debug (NULL);
	priv = widget_list_metatree_get_instance_private (widget);

	/* init tree_view */
	widget_list_metatree_setup_treeview (widget, type_page);

	/* on met la connection pour déplier replier les catégories */
	widget_list_metatree_init_toggle_button_expand (priv);

	/* on met la connection pour (dé)sélectionner tout ou partie des divisions */
	widget_list_metatree_init_buttons_select_unselect (priv, type_page);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void widget_list_metatree_init (WidgetListMetatree *widget)
{
	gtk_widget_init_template (GTK_WIDGET (widget));
}

static void widget_list_metatree_dispose (GObject *object)
{
	G_OBJECT_CLASS (widget_list_metatree_parent_class)->dispose (object);
}

static void widget_list_metatree_class_init (WidgetListMetatreeClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = widget_list_metatree_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/widgets/widget_list_metatree.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetListMetatree, vbox_list_metatree);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetListMetatree, button_income_metatree);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetListMetatree, button_outgoing_metatree);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetListMetatree, hbox_toggle_collapse_metatree);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetListMetatree, hbox_toggle_expand_metatree);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetListMetatree, label_income_metatree);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetListMetatree, label_outgoing_metatree);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetListMetatree, treeview_list_metatree);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetListMetatree, togglebutton_expand_metatree);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetListMetatree, togglebutton_select_all_metatree);
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 *
 *
 * \param page		category page or budgetary page
 * \param			BUDGETARY_PAGE_TYPE or CATEGORY_PAGE_TYPE
 *
 * \return
 **/
WidgetListMetatree *widget_list_metatree_new (GtkWidget *page,
											  gint type_page)
{
	WidgetListMetatree *widget;

	widget = g_object_new (WIDGET_LIST_METATREE_TYPE, NULL);
	widget_list_metatree_setup_widget (WIDGET_LIST_METATREE (widget), type_page);

	return widget;
}

/**
 *
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
void widget_list_metatree_init_liste (GtkWidget *widget,
									  gint report_number,
									  gint type_page)
{
	GSList *tmp_list;
	WidgetListMetatreePrivate *priv;

	devel_debug (NULL);
	priv = widget_list_metatree_get_instance_private (WIDGET_LIST_METATREE (widget));

	if (type_page == BUDGET_PAGE_TYPE)
	{
		tmp_list = gsb_data_report_get_budget_struct_list (report_number);
	}
	else if (type_page == CATEGORY_PAGE_TYPE)
	{
		tmp_list = gsb_data_report_get_category_struct_list (report_number);
	}
	else
		tmp_list = NULL;

	widget_list_metatree_init_treeview (priv->treeview_list_metatree, tmp_list);
	if (g_slist_length (tmp_list))
		utils_togglebutton_set_label_position_unselect (priv->togglebutton_select_all_metatree,
														G_CALLBACK (widget_list_metatree_button_uncheck_all_toggled),
														priv->treeview_list_metatree);
}

/**
 *
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
gboolean widget_list_metatree_get_info (GtkWidget *widget,
										gint report_number,
										gint type_page)
{
	GSList *list;
	gboolean result = FALSE;
	WidgetListMetatreePrivate *priv;

	devel_debug (NULL);
	priv = widget_list_metatree_get_instance_private (WIDGET_LIST_METATREE (widget));

	list = widget_list_metatree_get_selected (priv);

	/* if all is selected, erase the new list and set an info message */
	if (priv->all_selected)
	{
		widget_list_metatree_button_uncheck_all_toggled (GTK_TOGGLE_BUTTON (priv->togglebutton_select_all_metatree),
														 priv->treeview_list_metatree);
		gsb_data_report_free_categ_budget_struct_list (list);

		list = NULL;
		result = TRUE;
	}

	if (type_page == BUDGET_PAGE_TYPE)
	{
		gsb_data_report_set_budget_struct_list (report_number, list);
	}
	else if (type_page == CATEGORY_PAGE_TYPE)
	{
		gsb_data_report_set_category_struct_list (report_number, list);
	}

	return result;
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

