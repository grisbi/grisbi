/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2021 Pierre Biava (grisbi@pierre.biava.name)                    */
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
#include "widget_list_accounts.h"
#include "dialog.h"
#include "etats_config.h"
#include "etats_prefs.h"
#include "gsb_data_account.h"
#include "gsb_data_report.h"
#include "gsb_file.h"
#include "structures.h"
#include "utils.h"
#include "utils_buttons.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

/*START_STATIC*/
#ifdef OS_OSX
static const gchar *label_search_help = N_("Command-click\nto add to the selection");
#endif /* OS_OSX */
/*END_STATIC*/

typedef struct _WidgetListAccountsPrivate   WidgetListAccountsPrivate;

struct _WidgetListAccountsPrivate
{
	GtkWidget *			vbox_list_accounts;

    GtkWidget *         button_bank_accounts;
    GtkWidget *         button_cash_accounts;
    GtkWidget *         button_assets_accounts;
    GtkWidget *         button_liabilities_accounts;
    GtkWidget *         label_search_help_accounts;
    GtkWidget *         togglebutton_select_all_accounts;
	GtkWidget *         treeview_list_accounts;
};

G_DEFINE_TYPE_WITH_PRIVATE (WidgetListAccounts, widget_list_accounts, GTK_TYPE_BOX)
/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * sélectionne une partie de la liste des comptes
 *
 * \param
 * \param
 *
 * \return
 * */
static void widget_list_accounts_select_partie_liste_comptes (GtkWidget *tree_view,
															  gint type_compte)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreeSelection *selection;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
	if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
		return;

	do
	{
		gint account_number;

		gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 1, &account_number, -1);
		if (gsb_data_account_get_kind (account_number) == type_compte)
			gtk_tree_selection_select_iter (selection, &iter);
	}
	while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
}

/**
 * select or unselect all the rows of treeview
 *
 * \param toggle_button
 * \param tree_view
 *
 * \return
 **/
static void widget_list_accounts_select_unselect (GtkToggleButton *togglebutton,
												  GtkWidget *tree_view)
{
    GtkTreeSelection *selection;
    gchar *label;
    gboolean toggle;
    gint type_compte;

    type_compte = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (togglebutton), "type_compte"));
    toggle = gtk_toggle_button_get_active (togglebutton);
    if ((toggle))
        label = g_strdup (_("Unselect all"));
    else
        label = g_strdup (_("Select all"));

    gtk_button_set_label (GTK_BUTTON (togglebutton), label);
	g_free (label);

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    switch (type_compte)
    {
		case -9:
			utils_togglebutton_select_unselect_all_rows (togglebutton, tree_view);
			break;
		default:
			gtk_tree_selection_unselect_all (selection);
			if (toggle)
				widget_list_accounts_select_partie_liste_comptes (tree_view, type_compte);
			utils_togglebutton_change_label_select_unselect (GTK_TOGGLE_BUTTON (togglebutton), toggle);
			break;
    }
}

/**
 * Initialise les boutons de sélection des comptes
 *
 * \param suffixe du bouton
 * \param tree_view
 *
 * \return
 **/
static void widget_list_accounts_init_buttons_selection (WidgetListAccountsPrivate *priv)
{
	/* bouton select unselect all*/
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->togglebutton_select_all_accounts), FALSE);
    g_object_set_data (G_OBJECT (priv->togglebutton_select_all_accounts), "type_compte", GINT_TO_POINTER (-9));
    g_signal_connect (G_OBJECT (priv->togglebutton_select_all_accounts),
					  "toggled",
					  G_CALLBACK (widget_list_accounts_select_unselect),
					  priv->treeview_list_accounts);

    /* bouton_bank */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->button_bank_accounts), FALSE);
    g_object_set_data (G_OBJECT (priv->button_bank_accounts), "type_compte", GINT_TO_POINTER (0));
    g_signal_connect (G_OBJECT  (priv->button_bank_accounts),
					  "toggled",
					  G_CALLBACK (widget_list_accounts_select_unselect),
					  priv->treeview_list_accounts);

    /* bouton_cash */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->button_cash_accounts), FALSE);
    g_object_set_data (G_OBJECT (priv->button_cash_accounts), "type_compte", GINT_TO_POINTER (1));
    g_signal_connect (G_OBJECT  (priv->button_cash_accounts),
					  "toggled",
					  G_CALLBACK (widget_list_accounts_select_unselect),
					  priv->treeview_list_accounts);

    /* priv->_liabilities */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->button_liabilities_accounts), FALSE);
    g_object_set_data (G_OBJECT (priv->button_liabilities_accounts), "type_compte", GINT_TO_POINTER (2));
    g_signal_connect (G_OBJECT  (priv->button_liabilities_accounts),
					  "toggled",
					  G_CALLBACK (widget_list_accounts_select_unselect),
					  priv->treeview_list_accounts);

    /* "priv->_assets */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->button_assets_accounts), FALSE);
    g_object_set_data (G_OBJECT (priv->button_assets_accounts), "type_compte", GINT_TO_POINTER (3));
    g_signal_connect (G_OBJECT  (priv->button_assets_accounts),
					  "toggled",
					  G_CALLBACK (widget_list_accounts_select_unselect),
					  priv->treeview_list_accounts);
}

/**
 * retourne la liste des comptes dans un GtkTreeModel
 *
 * \param
 *
 * \return model
 */
static GtkTreeModel *widget_list_accounts_get_liste_comptes (void)
{
    GtkListStore *list_store;
    GSList *list_tmp;

    list_store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);

    /* on remplit la liste des exercices */
    list_tmp = gsb_data_account_get_list_accounts ();

    while (list_tmp)
    {
        GtkTreeIter iter;
        gchar *name;
        gint account_number;

        account_number = gsb_data_account_get_no_account (list_tmp -> data);

        name = my_strdup (gsb_data_account_get_name (account_number));

        gtk_list_store_append (list_store, &iter);
        gtk_list_store_set (list_store, &iter, 0, name, 1, account_number, -1);

        if (name)
            g_free (name);

        list_tmp = list_tmp -> next;
    }

    return GTK_TREE_MODEL (list_store);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void widget_list_accounts_setup_treeview (WidgetListAccounts *widget)
{
    GtkCellRenderer *cell;
	GtkTreeModel *model;
    GtkTreeSelection *selection;
    GtkTreeViewColumn *column;
	WidgetListAccountsPrivate *priv;

	priv = widget_list_accounts_get_instance_private (widget);

    /* set le model */
    model = widget_list_accounts_get_liste_comptes ();
	gtk_tree_view_set_model (GTK_TREE_VIEW (priv->treeview_list_accounts), model);
    g_object_unref (G_OBJECT (model));

    /* set the color of selected row */
	gtk_widget_set_name (priv->treeview_list_accounts, "tree_view");

    /* set the column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (NULL, cell, "text", 0, NULL);
    gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_append_column (GTK_TREE_VIEW (priv->treeview_list_accounts), GTK_TREE_VIEW_COLUMN (column));
    gtk_tree_view_column_set_resizable (column, TRUE);

	/* set selection */
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_list_accounts));
    gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);
}

/**
 * Création de la widget de gestion des list_accounts
 *
 * \param
 * \return
 **/
static void widget_list_accounts_setup_widget (WidgetListAccounts *widget)
{
	WidgetListAccountsPrivate *priv;

	devel_debug (NULL);
	priv = widget_list_accounts_get_instance_private (widget);

	/* on adapte le label pour Mac_OSX */
#ifdef OS_OSX
	gtk_label_set_text (GTK_LABEL (priv->label_search_help_accounts), label_search_help);
	gtk_label_set_justify (GTK_LABEL (priv->label_search_help_accounts), GTK_JUSTIFY_CENTER);
#endif /* OS_OSX */

	/* init tree_view */
	widget_list_accounts_setup_treeview (widget);

    /* on initialise les boutons pour sélectionner tout ou partie des comptes */
	widget_list_accounts_init_buttons_selection (priv);

}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void widget_list_accounts_init (WidgetListAccounts *widget)
{
	gtk_widget_init_template (GTK_WIDGET (widget));

	widget_list_accounts_setup_widget (widget);
}

static void widget_list_accounts_dispose (GObject *object)
{
	G_OBJECT_CLASS (widget_list_accounts_parent_class)->dispose (object);
}

static void widget_list_accounts_class_init (WidgetListAccountsClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = widget_list_accounts_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/widgets/widget_list_accounts.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetListAccounts, vbox_list_accounts);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetListAccounts, button_bank_accounts);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetListAccounts, button_cash_accounts);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetListAccounts, button_liabilities_accounts);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetListAccounts, button_assets_accounts);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetListAccounts, label_search_help_accounts);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetListAccounts, togglebutton_select_all_accounts);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetListAccounts, treeview_list_accounts);
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
WidgetListAccounts *widget_list_accounts_new (GtkWidget *page)
{
  return g_object_new (WIDGET_LIST_ACCOUNTS_TYPE, NULL);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
void widget_list_accounts_init_liste (GtkWidget *widget,
									  gint report_number,
									  gint type_page)
{
	GSList *list;
	WidgetListAccountsPrivate *priv;

	devel_debug (NULL);
	priv = widget_list_accounts_get_instance_private (WIDGET_LIST_ACCOUNTS (widget));

	/* select list */
	if (type_page == ACCOUNT_PAGE_TYPE)
		list = gsb_data_report_get_account_numbers_list (report_number);
	else
		list = gsb_data_report_get_transfer_account_numbers_list (report_number);
	new_etats_prefs_tree_view_select_rows_from_list (list, priv->treeview_list_accounts, 1);
	if (g_slist_length (list))
		utils_togglebutton_set_label_position_unselect (priv->togglebutton_select_all_accounts,
														G_CALLBACK (widget_list_accounts_select_unselect),
														priv->treeview_list_accounts);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean widget_list_accounts_get_info (GtkWidget *widget,
										gint report_number,
										gint type_page)
{
	WidgetListAccountsPrivate *priv;

	devel_debug (NULL);
	priv = widget_list_accounts_get_instance_private (WIDGET_LIST_ACCOUNTS (widget));

	if (type_page == ACCOUNT_PAGE_TYPE)
	{
		if (utils_tree_view_all_rows_are_selected (GTK_TREE_VIEW (priv->treeview_list_accounts)))
		{
			gchar *text;
			gchar *hint;

			hint = g_strdup (_("Performance issue."));
			text = g_strdup (_("All accounts have been selected.  Grisbi will run "
							   "faster without the \"Detail accounts used\" option activated"));

			dialogue_hint (text, hint);
			gsb_data_report_set_account_use_chosen (report_number, FALSE);

			g_free (text);
			g_free (hint);

			return TRUE;
		}
		else
			gsb_data_report_set_account_numbers_list (report_number,
													  new_etats_prefs_tree_view_get_list_rows_selected
													  (priv->treeview_list_accounts));
	}
	else
	{
		gsb_data_report_set_transfer_account_numbers_list (report_number,
														   new_etats_prefs_tree_view_get_list_rows_selected
														   (priv->treeview_list_accounts));
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
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */

